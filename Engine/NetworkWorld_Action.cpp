#include "stdafx.h"
#include "NetworkWorld.h"
#include "GameObjectBase.h"
#include "TimeManager.h"

// ---------------------------
// Client -> Host : SendAction
// ---------------------------
void NetworkWorld::SendActionMove(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId, uint8_t dir, float dist)
{
    ActionMove mv{};
    mv.dir = dir;
    mv.dist = dist;

    ActionHeader h{};
    h.turn = turn;
    h.actorPeerId = actorPeerId;
    h.actorNetId = actorNetId;
    h.targetNetId = 0;
    h.kind = (uint16_t)ActionKind::Move;
    h.payloadSize = (uint16_t)sizeof(ActionMove);
    h.seq = ++s_localActionSeq;

    ByteBuffer buf;
    buf.WritePod(h);
    buf.WritePod(mv);

    auto& net = NetManager::GetInstance();
    net.SendRaw(MSG_ACTION, buf.data);
}

void NetworkWorld::SendActionEndTurn(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId)
{
    printf("[SEND] EndTurn peer=%u turn=%u\n", actorPeerId, turn);
    ActionHeader h{};
    h.turn = turn;
    h.actorPeerId = actorPeerId;
    h.actorNetId = 0;
    h.targetNetId = 0;
    h.kind = (uint16_t)ActionKind::EndTurn;
    h.payloadSize = 0;
    h.seq = ++s_localActionSeq;

    ByteBuffer buf;
    buf.WritePod(h);

    auto& net = NetManager::GetInstance();
    net.SendRaw(MSG_ACTION, buf.data);
}

void NetworkWorld::QueueActionMove(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId, uint8_t dir, float dist)
{
    if (actorNetId == 0) return;

    // 네트워크로 보내지 않고, "SendActionMove와 동일한 payload"를 만들어 버킷에 넣는다.
    ActionMove mv{};
    mv.dir = dir;
    mv.dist = dist;

    ActionHeader h{};
    h.turn = turn;
    h.actorPeerId = actorPeerId;
    h.actorNetId = actorNetId;
    h.targetNetId = 0;
    h.kind = (uint16_t)ActionKind::Move;
    h.payloadSize = (uint16_t)sizeof(ActionMove);
    h.seq = ++s_localActionSeq;

    ByteBuffer buf;
    buf.WritePod(h);
    buf.WritePod(mv);

    // turn bucket에 저장
    s_turnBuckets[turn].actionsByActor[actorNetId].push_back(std::move(buf.data));
}

void NetworkWorld::QueueActionEndTurn(uint32_t turn, uint32_t actorPeerId, uint32_t peerId)
{
    if (peerId == 0) return;
    s_turnBuckets[turn].endedPeers.insert(peerId);
    printf("[HOST] QueueEndTurn peer=%u turn=%u\n", peerId, turn);
}

// ---------------------------
// Host : OnAction (판정)
// ---------------------------
void NetworkWorld::OnAction(const NetManager::NetEvent& ev)
{
    auto& net = NetManager::GetInstance();
    if (!net.IsHost()) return;

    ByteReader rd(ev.payload.data(), ev.payload.size());

    ActionHeader h{};
    if (!rd.ReadPod(h)) return;
    if (rd.Remaining() != h.payloadSize) return;

    TurnBucket& bucket = s_turnBuckets[h.turn];

    switch ((ActionKind)h.kind)
    {
    case ActionKind::Move:
        // payload 전체를 저장(나중에 ProcessTurn에서 다시 파싱)
        bucket.actionsByActor[h.actorNetId].push_back(ev.payload);
        break;

    case ActionKind::EndTurn:
        
        if (h.payloadSize != 0) return;
        bucket.endedPeers.insert(h.actorPeerId);
        printf("[HOST] EndTurn recv actor=%u turn=%u\n", h.actorPeerId, h.turn);
        break;

    default:
        return;
    }//  여기서 HandleMoveAction 호출하지 않는다!
}

void NetworkWorld::SendTurnSync(uint32_t turn)
{
    auto& net = NetManager::GetInstance();
    if (!net.IsHost()) return;

    TurnSync t{};
    t.currentTurn = turn;

    ByteBuffer buf;
    buf.WritePod(t);

    net.SendRaw(MSG_TURN, buf.data);
}

void NetworkWorld::OnTurn(const NetManager::NetEvent& ev)
{
    ByteReader rd(ev.payload.data(), ev.payload.size());

    TurnSync t{};
    if (!rd.ReadPod(t)) return;

    s_clientTurn = t.currentTurn;

}

bool NetworkWorld::IsTurnReady(uint32_t turn)
{
    if (s_expectedPeers.empty()) return false;

    auto it = s_turnBuckets.find(turn);
    if (it == s_turnBuckets.end()) return false;

    const auto& ended = it->second.endedPeers;

    for (uint32_t peer : s_expectedPeers)
    {
        if (!ended.contains(peer)) return false;
    }
    return true;
}

void NetworkWorld::HostTick()
{
    auto& net = NetManager::GetInstance();
    if (!net.IsHost()) return;

    if (IsTurnReady(s_currentTurn))
    {
        ProcessTurn(s_currentTurn);
        s_currentTurn++;
        SendTurnSync(s_currentTurn);
    }
}

void NetworkWorld::ProcessTurn(uint32_t turn)
{
    auto it = s_turnBuckets.find(turn);
    if (it == s_turnBuckets.end()) return;

    TurnBucket bucket = std::move(it->second);
    s_turnBuckets.erase(it);

    // 모든 액션을 모아 header.seq로 정렬
    struct Pending { ActionHeader h; std::vector<uint8_t> bytes; };
    std::vector<Pending> all;

    for (auto& [actor, list] : bucket.actionsByActor)
    {
        for (auto& bytes : list)
        {
            ByteReader rd(bytes.data(), bytes.size());
            ActionHeader h{};
            if (!rd.ReadPod(h)) continue;
            all.push_back({ h, std::move(bytes) });
        }
    }

    std::sort(all.begin(), all.end(),
        [](const Pending& a, const Pending& b)
        {
            if (a.h.seq != b.h.seq) return a.h.seq < b.h.seq;
            return a.h.actorNetId < b.h.actorNetId;
        });

    // 순서대로 파싱 후 기존 HandleXXX 호출
    for (auto& p : all)
    {
        ByteReader rd(p.bytes.data(), p.bytes.size());
        ActionHeader h{};
        if (!rd.ReadPod(h)) continue;
        if (rd.Remaining() != h.payloadSize) continue;

        switch ((ActionKind)h.kind)
        {
        case ActionKind::Move:
        {
            if (h.payloadSize != sizeof(ActionMove)) break;
            ActionMove mv{};
            if (!rd.ReadPod(mv)) break;
            HandleMoveAction(h, mv);
            break;
        }
        default:
            break;
        }
    }

    // TODO: 호스트기준 월드로 동기화
    // 턴 종료 처리(필요하면 여기서)
    // SendDirtyStatesToAll(); 등 결과 전파를 여기서 일괄로 해도 됨
}

// ---------------------------
// Host : Apply rules
// ---------------------------
void NetworkWorld::HandleMoveAction(const ActionHeader& h, const ActionMove& mv)
{

    auto* obj = Find(h.actorNetId);
    if (!obj)
    {
        printf("[HOST] Find failed netId=%u. keys=", h.actorNetId);
        for (auto& [id, _] : s_objects) printf("%u ", id);
        printf("\n");
        return;
    }

    // 이동 벡터 계산
    float deltaTime = TimeManager::GetInstance().GetDeltaTime();

    float moveSpeed = 10.0f;

    DirectX::XMVECTOR delta = DirectX::XMVectorZero();

    const float dist = mv.dist;
    const float step = moveSpeed * deltaTime * dist;

    switch ((MoveDir)mv.dir)
    {
    case MoveDir::Up:
        delta = DirectX::XMVectorSet(0.0f, 0.0f, step, 0.0f);
        break;
    case MoveDir::Down:
        delta = DirectX::XMVectorSet(0.0f, 0.0f, -step, 0.0f);
        break;
    case MoveDir::Left:
        delta = DirectX::XMVectorSet(-step, 0.0f, 0.0f, 0.0f);
        break;
    case MoveDir::Right:
        delta = DirectX::XMVectorSet(step, 0.0f, 0.0f, 0.0f);
        break;
    default:
        break;
    }

    // 현재 Transform 가져오기
    auto pos = obj->GetPosition();
    auto rot = obj->GetRotation();
    auto scale = obj->GetScale();

    // 위치 갱신
    pos = DirectX::XMVectorAdd(pos, delta);

    // Host 월드에 적용
    obj->SetPosition(pos);

    // 결과 State 전송 -> Client
    SendState(h.actorNetId, pos, rot, scale);

}

void NetworkWorld::HandleEndTurnAction(const ActionHeader& h)
{
    // TODO: TurnManager.Advance()
}

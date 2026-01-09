#include "stdafx.h"
#include "NetworkWorld.h"
#include "GameObjectBase.h"
#include "TimeManager.h"

// ---------------------------
// Client -> Host : SendAction
// ---------------------------
void NetworkWorld::SendActionMove(uint32_t turn, uint32_t actorNetId, uint8_t dir, float dist)
{
    ActionMove mv{};
    mv.dir = dir;
    mv.dist = dist;

    ActionHeader h{};
    h.turn = turn;
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

void NetworkWorld::SendActionEndTurn(uint32_t turn, uint32_t actorNetId)
{
    ActionHeader h{};
    h.turn = turn;
    h.actorNetId = actorNetId;
    h.targetNetId = 0;
    h.kind = (uint16_t)ActionKind::EndTurn;
    h.payloadSize = 0;
    h.seq = ++s_localActionSeq;

    ByteBuffer buf;
    buf.WritePod(h);

    auto& net = NetManager::GetInstance();
    net.SendRaw(MSG_ACTION, buf.data);
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

    switch ((ActionKind)h.kind)
    {
    case ActionKind::Move:
    {
        if (h.payloadSize != sizeof(ActionMove)) return;
        ActionMove mv{};
        if (!rd.ReadPod(mv)) return;
        HandleMoveAction(h, mv);
        break;
    }
    case ActionKind::EndTurn:
    {
        if (h.payloadSize != 0) return;
        HandleEndTurnAction(h);
        break;
    }
    default:
        return;
    }

    // 결과를 전파
    // TODO: Dirty 시스템 만들기
    // SendDirtyStatesToAll();

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

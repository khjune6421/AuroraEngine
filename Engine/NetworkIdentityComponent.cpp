#include "stdafx.h"
#include "NetworkIdentityComponent.h"
#include "NetworkWorld.h"
#include "GameObjectBase.h"
#include "TimeManager.h"
#include "InputManager.h"   
REGISTER_TYPE(NetworkIdentityComponent)

void NetworkIdentityComponent::Initialize()
{
    GameObjectBase* owner = static_cast<GameObjectBase*>(GetOwner());
    if (!owner) return;

    if (m_typeName.empty())
        if (m_typeName.empty()) m_typeName = owner->GetType();

    if (m_netId != 0)
        NetworkWorld::Register(m_netId, owner);

    // Authority면 스폰을 보내서 상대에 동일 오브젝트가 생기게 함
    if (m_isAuthority && m_autoSpawn && !m_spawnSent && m_netId != 0 && !m_typeName.empty())
    {
        SetNetId(m_netId);
        NetworkWorld::SendSpawn(static_cast<GameObjectBase*>(m_owner), m_netId, m_typeName);
        m_spawnSent = true;
    }
}

void NetworkIdentityComponent::Update()
{
    //if (!m_isAuthority) return;

    if (m_isAuthority)
    {
        GameObjectBase* owner = static_cast<GameObjectBase*>(GetOwner());
        if (!owner) return;

        if (m_netId == 0 && NetManager::GetInstance().IsHost()) { m_netId = NetworkWorld::AllocateNetId(); return; }

        if (m_netId != 0 && s_localClaimedNetId == 0)
        {
            // 이 머신에서 첫 번째로 netId가 생긴 NetworkIdentity를 내 조종 대상으로 삼는다
            s_localClaimedNetId = m_netId;
            m_isLocalPlayer = true;
            m_isPlayerPawn = true; // 테스트용: 이게 actor
        }
        else
        {
            // 내 것이 아니면 입력 금지
            m_isLocalPlayer = (m_netId == s_localClaimedNetId);
            m_isPlayerPawn = (m_netId == s_localClaimedNetId);
        }

        if (!m_registeredActor && NetManager::GetInstance().IsHost() && m_isPlayerPawn)
        {
            uint32_t myPeerId = NetManager::GetInstance().GetSelfPeerId();
            NetworkWorld::HostRegisterExpectedPeer(myPeerId);
            m_registeredActor = true;
        }

        if (m_autoSpawn && !m_spawnSent && !m_typeName.empty())
        {
            SetNetId(m_netId);
            NetworkWorld::SendSpawn(owner, m_netId, m_typeName);
            m_spawnSent = true;
        }
    }

    //TODO:임시 테스트용 입력송신을 담당해서 묶을 함수 하나 만들기 UpdateClientInput
    auto& net = NetManager::GetInstance();
    if (!net.IsConnected()) return;
    if (m_netId == 0) return;

    if (!m_isLocalPlayer) return;

    uint32_t turn = NetworkWorld::GetLocalTurn(); // 아래에서 만들기
    uint32_t myPeerId = NetManager::GetInstance().GetSelfPeerId();
    InputManager& input = InputManager::GetInstance();

    if (input.GetKey(KeyCode::Left))
    {
        if (net.IsHost()) NetworkWorld::QueueActionMove(turn, myPeerId, m_netId, (uint8_t)MoveDir::Left, 1.0f);
        else             NetworkWorld::SendActionMove(turn, myPeerId, m_netId, (uint8_t)MoveDir::Left, 1.0f);
    }
    if (input.GetKey(KeyCode::Right))
    {
        if (net.IsHost()) NetworkWorld::QueueActionMove(turn, myPeerId, m_netId, (uint8_t)MoveDir::Right, 1.0f);
        else             NetworkWorld::SendActionMove(turn, myPeerId, m_netId, (uint8_t)MoveDir::Right, 1.0f);
    }

    if (input.GetKeyDown(KeyCode::Enter))
    {
        
        //printf("[CLIENT] EndTurn send myPeerId=%u netId=%u\n", myPeerId, m_netId);

        //if (net.IsHost()) NetworkWorld::QueueActionEndTurn(turn, myPeerId, m_netId);
        //else             NetworkWorld::SendActionEndTurn(turn, myPeerId, m_netId);
    }
}

void NetworkIdentityComponent::Finalize()
{
    if (m_netId != 0)
        NetworkWorld::Unregister(m_netId);
}

void NetworkIdentityComponent::RenderImGui()
{
    ImGui::SeparatorText("Network Identity");

    ImGui::InputScalar("NetId", ImGuiDataType_U32, &m_netId);
    ImGui::Checkbox("Authority", &m_isAuthority);
    ImGui::Checkbox("Auto Spawn", &m_autoSpawn);

    char buf[256];
    strcpy_s(buf, m_typeName.c_str());
    if (ImGui::InputText("Type Name (for spawn)", buf, sizeof(buf)))
        m_typeName = buf;

    ImGui::InputFloat("Send Interval", &m_sendInterval);
    //if (m_sendInterval < 0.01f) m_sendInterval = 0.01f;

    if (ImGui::Button("Register"))
    {
        GameObjectBase* owner = static_cast<GameObjectBase*>(GetOwner());
        if (owner && m_netId != 0)
            NetworkWorld::Register(m_netId, owner);
    }

    ImGui::SameLine();
    if (ImGui::Button("Send Spawn Now"))
    {
        GameObjectBase* owner = static_cast<GameObjectBase*>(GetOwner());
        if (owner && m_netId != 0 && !m_typeName.empty())
        {
            SetNetId(m_netId);
            NetworkWorld::SendSpawn(static_cast<GameObjectBase*>(m_owner), m_netId, m_typeName);
            m_spawnSent = true;
        }
    }
}

nlohmann::json NetworkIdentityComponent::Serialize()
{
    nlohmann::json j;
    j["netId"] = m_netId;
    j["authority"] = m_isAuthority;
    j["autoSpawn"] = m_autoSpawn;
    j["sendInterval"] = m_sendInterval;
    j["typeName"] = m_typeName;
    return j;
}

void NetworkIdentityComponent::Deserialize(const nlohmann::json& jsonData)
{
    if (jsonData.contains("netId"))        m_netId = jsonData["netId"].get<uint32_t>();
    if (jsonData.contains("authority"))    m_isAuthority = jsonData["authority"].get<bool>();
    if (jsonData.contains("autoSpawn"))    m_autoSpawn = jsonData["autoSpawn"].get<bool>();
    if (jsonData.contains("sendInterval")) m_sendInterval = jsonData["sendInterval"].get<float>();
    if (jsonData.contains("typeName"))     m_typeName = jsonData["typeName"].get<std::string>();

    //if (m_sendInterval < 0.01f) m_sendInterval = 0.01f;
}

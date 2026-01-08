#include "stdafx.h"
#include "NetworkIdentityComponent.h"
#include "GameObjectBase.h"
#include "TimeManager.h"

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
        NetworkWorld::SendSpawn(static_cast<GameObjectBase*>(m_owner), m_netId, m_typeName);
        m_spawnSent = true;
    }
}

void NetworkIdentityComponent::Update()
{
    if (!m_isAuthority) return;

    GameObjectBase* owner = static_cast<GameObjectBase*>(GetOwner());
    if (!owner) return;

    // typeName이 세팅되어 있고, netId가 정상일 때만 동작
    if (m_netId == 0) return;

    // 아직 스폰을 안 보냈는데 조건이 맞으면 보냄
    if (m_autoSpawn && !m_spawnSent && !m_typeName.empty())
    {
        NetworkWorld::SendSpawn(static_cast<GameObjectBase*>(m_owner), m_netId, m_typeName);
        m_spawnSent = true;
    }

    // 상태 송신(테스트용)
    m_sendAccum += TimeManager::GetInstance().GetDeltaTime();
    if (m_sendAccum >= m_sendInterval)
    {
        m_sendAccum = 0.0f;
        NetworkWorld::SendState(
            m_netId,
            owner->GetPosition(),
            owner->GetRotation(),
            owner->GetScale()
        );
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
    if (m_sendInterval < 0.01f) m_sendInterval = 0.01f;

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

    if (m_sendInterval < 0.01f) m_sendInterval = 0.01f;
}

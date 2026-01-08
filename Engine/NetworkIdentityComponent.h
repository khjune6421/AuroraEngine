#pragma once
#include "ComponentBase.h"
#include "NetworkWorld.h"

void ForceLink_NetworkIdentityComponent();

class NetworkIdentityComponent : public ComponentBase
{
public:
    NetworkIdentityComponent() = default;
    ~NetworkIdentityComponent() override = default;

    bool NeedsUpdate() const override { return true; }
    bool NeedsRender() const override { return false; }

    void SetNetId(uint32_t id) { m_netId = id; }
    uint32_t GetNetId() const { return m_netId; }

    void SetAuthority(bool a) { m_isAuthority = a; }
    bool IsAuthority() const { return m_isAuthority; }

    void SetAutoSpawn(bool a) { m_autoSpawn = a; }

private:
    void Initialize() override;
    void Update() override;
    void RenderImGui() override;
    void Finalize() override;

    nlohmann::json Serialize() override;
    void Deserialize(const nlohmann::json& jsonData) override;

private:
    // 공유 오브젝트 식별자
    uint32_t m_netId = 0;

    // 내가 이 오브젝트의 상태를 송신할 권한이 있는가(Host 또는 소유 Client)
    bool m_isAuthority = false;

    // Initialize 시 자동으로 Spawn 보내기 (테스트 편의)
    bool m_autoSpawn = true;

    // Spawn 1회만 보내기
    bool m_spawnSent = false;

    // 상태 전송 주기(테스트용: 20Hz)
    float m_sendAccum = 0.0f;
    float m_sendInterval = 0.05f;

    std::vector<std::string> m_replicateComponents = { "ModelComponent" };
    // 스폰할 타입 이름(상대가 TypeRegistry로 생성할 때 필요)
    // 보통은 owner->GetType() 같은 게 있으면 그걸 쓰는 게 맞음
    std::string m_typeName = "";
};

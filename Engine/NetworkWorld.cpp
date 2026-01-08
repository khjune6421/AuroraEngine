#include "stdafx.h"
#include "NetworkWorld.h"

#include "SceneBase.h"
#include "GameObjectBase.h"

#include "NetworkIdentityComponent.h"
#include "ModelComponent.h"

void NetworkWorld::Initialize()
{
    if (s_inited) return;
    s_inited = true;
    RegisterHandlers();
    printf("[NET] RegisterHandler SPAWN=%u STATE=%u\n", MSG_SPAWN, MSG_STATE);

}

void NetworkWorld::Finalize()
{
    if (!s_inited) return;
    UnregisterHandlers();
    Clear();
    s_inited = false;
}

void NetworkWorld::SetScene(SceneBase* scene)
{
    s_scene = scene;
    // 씬 바뀔 때 기존 맵은 날리는 게 안전 (씬 오브젝트 포인터가 무효화될 수 있음)
    Clear();
}

void NetworkWorld::Clear()
{
    s_objects.clear();
}

void NetworkWorld::Register(uint32_t netId, GameObjectBase* obj)
{
    if (!obj) return;
    s_objects[netId] = obj;
}

void NetworkWorld::Unregister(uint32_t netId)
{
    s_objects.erase(netId);
}

GameObjectBase* NetworkWorld::Find(uint32_t netId)
{
    auto it = s_objects.find(netId);
    if (it == s_objects.end()) return nullptr;
    return it->second;
}

void NetworkWorld::RegisterHandlers()
{
    auto& net = NetManager::GetInstance();

    net.RegisterHandler(MSG_SPAWN, [](const NetManager::NetEvent& ev)
        {
            NetworkWorld::OnSpawn(ev);
        });

    net.RegisterHandler(MSG_STATE, [](const NetManager::NetEvent& ev)
        {
            NetworkWorld::OnState(ev);
        });
}

void NetworkWorld::UnregisterHandlers()
{
    auto& net = NetManager::GetInstance();
    net.UnregisterHandler(MSG_SPAWN);
    net.UnregisterHandler(MSG_STATE);
}

// ---------------------
// inbound handlers
// ---------------------

void NetworkWorld::OnSpawn(const NetManager::NetEvent& ev)
{
    printf("[NET] OnSpawn ENTER msgId=%u payloadSize=%zu\n",
        (unsigned)ev.msgId, ev.payload.size());

    auto j = NetManager::BytesToJson(ev.payload);
    if (j.is_discarded()) { printf("[NET] OnSpawn JSON discard\n"); return; }

    printf("[NET] OnSpawn JSON=%s\n", j.dump().c_str());

    if (!j.contains("netId")) { printf("[NET] OnSpawn no netId\n"); return; }
    if (!j.contains("type")) { printf("[NET] OnSpawn no type\n"); return; }

    const uint32_t netId = j["netId"].get<uint32_t>();
    const std::string typeName = j["type"].get<std::string>();

    // 이미 존재하면 무시(중복 스폰 방지)
    if (Find(netId))
        return;

    GameObjectBase* obj = s_scene->CreateRootGameObject(typeName);
    if (!obj) return;

    // transform (optional)
    if (j.contains("p")) obj->SetPosition(JsonToVec3(j["p"], 1.0f));
    if (j.contains("r")) obj->SetRotation(JsonToVec3(j["r"], 0.0f)); // euler
    if (j.contains("s")) obj->SetScale(JsonToVec3(j["s"], 1.0f));

    //Register(netId, obj);

    auto* idc = obj->CreateComponent<NetworkIdentityComponent>();
    idc->SetNetId(netId);
    idc->SetAuthority(false);

    // components 복원
    if (j.contains("components") && j["components"].is_array())
    {
        for (auto& c : j["components"])
        {
            std::string ct = c.value("type", "");
            if (ct.empty() || !c.contains("data")) continue;

            // 지금은 ModelComponent만 처리(1단계)
            if (ct == "ModelComponent")
            {
                obj->CreateComponent<ModelComponent>();
                auto* model = obj->GetComponent<ModelComponent>();
                printf("[NET] After CreateComponent(ModelComponent): %p\n", model);
                if (Base* model = obj->GetComponent<ModelComponent>())
                {
                    model->BaseDeserialize(c["data"]);
                }
            }
            // 나중에 다른 컴포넌트도 여기에 추가
        }
    }

    Register(netId, obj);
    printf("[NET] Spawned object. netId=%u type=%s\n", netId, typeName.c_str());
}

void NetworkWorld::OnState(const NetManager::NetEvent& ev)
{
    auto j = NetManager::BytesToJson(ev.payload);
    if (j.is_discarded())
        return;

    if (!j.contains("netId"))
        return;

    const uint32_t netId = j["netId"].get<uint32_t>();
    GameObjectBase* obj = Find(netId);
    if (!obj) return;

    if (j.contains("p")) obj->SetPosition(JsonToVec3(j["p"], 1.0f));
    if (j.contains("r")) obj->SetRotation(JsonToVec3(j["r"], 0.0f));
    if (j.contains("s")) obj->SetScale(JsonToVec3(j["s"], 1.0f));
}

void NetworkWorld::SendSpawn(GameObjectBase* obj, uint32_t netId, const std::string& typeName)
{
    if (!obj) return;

    nlohmann::json j;
    j["netId"] = netId;
    j["type"] = typeName;
    j["p"] = Vec3ToJson(obj->GetPosition());
    j["r"] = Vec3ToJson(obj->GetRotation());
    j["s"] = Vec3ToJson(obj->GetScale());

    j["components"] = nlohmann::json::array();

    // ModelComponent 복제 (지금은 1단계로 이것만)
    if (Base* model = obj->GetComponent<ModelComponent>())
    {
        nlohmann::json c;
        c["type"] = "ModelComponent";
        c["data"] = model->BaseSerialize();   // Serialize 결과(json)
        j["components"].push_back(c);
    }

    NetManager::GetInstance().SendJson(MSG_SPAWN, j);
}

void NetworkWorld::SendState(uint32_t netId,
    const DirectX::XMVECTOR& pos,
    const DirectX::XMVECTOR& rotEuler,
    const DirectX::XMVECTOR& scale)
{
    nlohmann::json j;
    j["netId"] = netId;
    j["p"] = Vec3ToJson(pos);
    j["r"] = Vec3ToJson(rotEuler);
    j["s"] = Vec3ToJson(scale);
    j["components"] = nlohmann::json::array();

    NetManager::GetInstance().SendJson(MSG_STATE, j);
}

// ---------------------
// json helpers
// ---------------------

nlohmann::json NetworkWorld::Vec3ToJson(const DirectX::XMVECTOR& v)
{
    DirectX::XMFLOAT3 f;
    DirectX::XMStoreFloat3(&f, v);
    return nlohmann::json::array({ f.x, f.y, f.z });
}

DirectX::XMVECTOR NetworkWorld::JsonToVec3(const nlohmann::json& a, float w)
{
    if (!a.is_array() || a.size() < 3) return DirectX::XMVectorSet(0, 0, 0, w);
    return DirectX::XMVectorSet(a[0].get<float>(), a[1].get<float>(), a[2].get<float>(), w);
}

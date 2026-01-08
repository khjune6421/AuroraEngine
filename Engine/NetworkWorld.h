#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>

#include "NetManager.h"

class SceneBase;
class GameObjectBase;

class NetworkWorld
{
public:
    // msg id
    static constexpr NetManager::MsgId MSG_SPAWN = 100;
    static constexpr NetManager::MsgId MSG_STATE = 101; // 다음 단계(상태 동기화)

public:
    // lifecycle
    static void Initialize();          // 핸들러 등록(한 번만)
    static void Finalize();            // 핸들러 해제 + 맵 정리

    // scene binding
    static void SetScene(SceneBase* scene); // 씬 시작/전환 시 호출
    static void Clear();                    // 씬 종료 시 호출(또는 SetScene 내부에서)

    // registry
    static void Register(uint32_t netId, GameObjectBase* obj);
    static void Unregister(uint32_t netId);
    static GameObjectBase* Find(uint32_t netId);

    static void SendSpawn(GameObjectBase* obj, uint32_t netId, const std::string& typeName);

    static void SendState(uint32_t netId,
        const DirectX::XMVECTOR& pos,
        const DirectX::XMVECTOR& rotEuler,
        const DirectX::XMVECTOR& scale);

private:
    // handlers
    static void RegisterHandlers();
    static void UnregisterHandlers();

    static void OnSpawn(const NetManager::NetEvent& ev);
    static void OnState(const NetManager::NetEvent& ev);

    // json helpers
    static nlohmann::json Vec3ToJson(const DirectX::XMVECTOR& v);
    static DirectX::XMVECTOR JsonToVec3(const nlohmann::json& a, float w);

private:
    static inline bool s_inited = false;
    static inline SceneBase* s_scene = nullptr;

    static inline std::unordered_map<uint32_t, GameObjectBase*> s_objects;
};

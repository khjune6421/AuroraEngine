#pragma once
#include "NetManager.h"
#include "NetAction.h"

class SceneBase;
class GameObjectBase;

class NetworkWorld
{
public:
    // msg id
    static constexpr NetManager::MsgId MSG_SPAWN    = 100;
    static constexpr NetManager::MsgId MSG_STATE    = 101; // 다음 단계(상태 동기화)
    static constexpr NetManager::MsgId MSG_ACTION   = 200;
    static constexpr NetManager::MsgId MSG_TURN     = 201;
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

    // Client -> Host send wrappers
    static void SendActionMove(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId, uint8_t dir, float dist);
    static void SendActionEndTurn(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId);

    // Host turn loop
    static void HostTick();                 // 매 프레임 호스트에서 호출
    static bool IsTurnReady(uint32_t turn); // 해당 턴 처리 가능?
    static void ProcessTurn(uint32_t turn); // 해당 턴 액션 일괄 처리

    // Listen server local enqueue
    static void QueueActionMove(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId, uint8_t dir, float dist);
    static void QueueActionEndTurn(uint32_t turn, uint32_t actorPeerId, uint32_t actorNetId);

    //key 저장보장
    static uint32_t AllocateNetId();

    //입력송신 용 turn
    static uint32_t GetLocalTurn();

    static void HostRegisterExpectedActor(uint32_t actorNetId);
    static void HostRegisterExpectedPeer(uint32_t actorNetId);
private:
    // handlers
    static void RegisterHandlers();
    static void UnregisterHandlers();

    static void OnSpawn(const NetManager::NetEvent& ev);
    static void OnState(const NetManager::NetEvent& ev);

    //  Receive 
    static void OnAction(const NetManager::NetEvent& ev);
    static void SendTurnSync(uint32_t turn);
    static void OnTurn(const NetManager::NetEvent& ev);

    //  Host only: apply game rule 
    static void HandleMoveAction(const ActionHeader& h, const ActionMove& mv);
    static void HandleEndTurnAction(const ActionHeader& h);

    // json helpers
    static nlohmann::json Vec3ToJson(const DirectX::XMVECTOR& v);
    static DirectX::XMVECTOR JsonToVec3(const nlohmann::json& a, float w);

private:
    static inline bool s_inited = false;
    static inline SceneBase* s_scene = nullptr;

    static inline uint32_t s_localActionSeq = 0;

    static inline std::unordered_map<uint32_t, GameObjectBase*> s_objects;

    struct TurnBucket
    {
        // actorNetId -> actions (순서 보장용으로 header.seq로 정렬 가능)
        std::unordered_map<uint32_t, std::vector<std::vector<uint8_t>>> actionsByActor;
        std::unordered_set<uint32_t> endedActors; // EndTurn 제출한 actor들
        std::unordered_set<uint32_t> endedPeers;
    };

    static inline std::unordered_map<uint32_t, TurnBucket> s_turnBuckets;
    static inline uint32_t s_currentTurn = 0;
    static inline std::unordered_set<uint32_t> s_expectedActors;
    static inline std::unordered_set<uint32_t> s_expectedPeers;
    static inline int s_expectedLimit = 2;

    //클라가 알고 있는 현재 턴
    static inline uint32_t s_clientTurn = 0;
};

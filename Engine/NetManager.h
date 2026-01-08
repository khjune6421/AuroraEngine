#pragma once

// NetManager 내부에서 사용하는 boost::asio 타입들
#include <boost/asio.hpp>
#include <boost/asio/executor_work_guard.hpp>

// json
#include <nlohmann/json.hpp>

// std 타입들
#include <array>            // std::array
#include <atomic>           // std::atomic
#include <cstdint>          // uint16_t, uint32_t, uint8_t
#include <functional>       // std::function
#include <memory>           // std::unique_ptr
#include <mutex>            // std::mutex
#include <optional>         // std::optional
#include <queue>            // std::queue
#include <string>           // std::string
#include <thread>           // std::thread
#include <unordered_map>    // std::unordered_map
#include <utility>          // std::move (SetOnConnected 등에서)
#include <vector>           // std::vector
///////////////////////////////////////////////////////////////////////
#include "Singleton.h"

class NetManager : public Singleton<NetManager>
{
	friend class Singleton<NetManager>;

public:
    using MsgId = uint16_t;
    using PeerId = uint32_t;

    struct NetEvent
    {
        enum class Type
        {
            Connected,
            Disconnected,
            Message,
            Error
        };

        Type type = Type::Message;
        PeerId peerId = 0;
        MsgId msgId = 0;

        std::vector<uint8_t> payload;
        std::string errorMessage;
    };

    using Handler = std::function<void(const NetEvent&)>;

public:

    // ----- lifecycle (엔진 루프에서 호출) -----
    void Initialize();      // io_context 준비
    void Update();          // 메인 스레드에서 이벤트 처리(중요)
    void Finalize();        // 스레드 정리 및 연결 종료

    // ----- host/client -----
    bool StartHost(uint16_t port);                 // Listen server
    bool Connect(const std::string& host, uint16_t port); // Client connect
    void Disconnect();                              // 연결 종료(클라 기준)
    bool IsRunning() const { return m_running.load(); }
    bool IsConnected() const { return m_connected.load(); }

    // ----- send -----
    // 간단 JSON 송신 (턴제에 매우 편함)
    bool SendJson(MsgId msgId, const nlohmann::json& j);

    // raw payload 송신 (msgId + payload)
    bool SendRaw(MsgId msgId, const void* data, size_t size);
    bool SendRaw(MsgId msgId, const std::vector<uint8_t>& bytes);

    // ----- message dispatch -----
    void RegisterHandler(MsgId msgId, Handler handler);
    void UnregisterHandler(MsgId msgId);

    // 기본 이벤트(Connected/Disconnected/Error)도 받고 싶으면 msgId 없이 별도 콜백 두는 게 편함
    void SetOnConnected(std::function<void(PeerId)> cb) { m_onConnected = std::move(cb); }
    void SetOnDisconnected(std::function<void(PeerId)> cb) { m_onDisconnected = std::move(cb); }
    void SetOnError(std::function<void(const std::string&)> cb) { m_onError = std::move(cb); }

    // payload json 헬퍼
    static std::vector<uint8_t> JsonToBytes(const nlohmann::json& j);
    static nlohmann::json BytesToJson(const std::vector<uint8_t>& bytes);
private:
    NetManager() = default;
	~NetManager() { Finalize(); }

    NetManager(const NetManager&) = delete;
    NetManager& operator=(const NetManager&) = delete;
    NetManager(NetManager&&) = delete;
    NetManager& operator=(NetManager&&) = delete;

private:
    // ----- internal session (단일 연결 기준) -----
    void StartIoThread();
    void StopIoThread();

    void DoAccept();    // host용
    void DoConnect(const std::string& host, uint16_t port);

    void BeginReadHeader();
    void BeginReadBody(std::size_t bodyLen);
    void PushEvent(NetEvent&& ev);

    void DoWrite(std::vector<uint8_t>&& framedPacket);

    // 프레임: [u32 length][u16 msgId][payload...]
    static std::vector<uint8_t> BuildFrame(MsgId msgId, const uint8_t* payload, size_t payloadSize);

private:
    // asio core
    boost::asio::io_context m_io;

    using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    std::optional<WorkGuard> m_workGuard;
    std::thread m_ioThread{};

    // host acceptor
    std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor{};

    // single session socket (LAN 턴제면 이것만으로 시작 가능)
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket{};

    // 상태
    std::atomic<bool> m_running{ false };
    std::atomic<bool> m_connected{ false };
    bool m_isHost = false;
    PeerId m_peerIdSelf = 1; // 일단 1로 두고, 나중에 핸드셰이크로 배정

    // read buffers
    std::array<uint8_t, 4> m_readLenBuf{};    // length (u32)
    std::vector<uint8_t> m_readBodyBuf{};     // body = [msgId][payload]

    // write queue (asio 스레드에서 직렬화)
    std::mutex m_writeMtx;
    std::queue<std::vector<uint8_t>> m_writeQueue;
    bool m_writing = false;

    // event queue (asio -> main thread)
    std::mutex m_eventMtx;
    std::queue<NetEvent> m_events;

    // handlers (main thread에서만 접근)
    std::unordered_map<MsgId, Handler> m_handlers;

    // callbacks
    std::function<void(PeerId)> m_onConnected;
    std::function<void(PeerId)> m_onDisconnected;
    std::function<void(const std::string&)> m_onError;
};
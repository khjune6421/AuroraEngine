#include "stdafx.h"
#include "NetManager.h"

using boost::asio::ip::tcp;

static uint32_t ReadU32LE(const uint8_t* p)
{
    uint32_t v = 0;
    v |= uint32_t(p[0]);
    v |= uint32_t(p[1]) << 8;
    v |= uint32_t(p[2]) << 16;
    v |= uint32_t(p[3]) << 24;
    return v;
}
static void WriteU32LE(uint8_t* p, uint32_t v)
{
    p[0] = uint8_t(v & 0xFF);
    p[1] = uint8_t((v >> 8) & 0xFF);
    p[2] = uint8_t((v >> 16) & 0xFF);
    p[3] = uint8_t((v >> 24) & 0xFF);
}
static uint16_t ReadU16LE(const uint8_t* p)
{
    return uint16_t(p[0]) | (uint16_t(p[1]) << 8);
}
static void WriteU16LE(uint8_t* p, uint16_t v)
{
    p[0] = uint8_t(v & 0xFF);
    p[1] = uint8_t((v >> 8) & 0xFF);
}

//ReadU32LE, WriteU32LE, ReadU16LE, WriteU16LE      <- 리틀엔디안으로 변경하는 함수(memcpy 엔디안 문제 방지용)

void NetManager::Initialize()
{
    if (m_running.load()) return;

    m_workGuard.emplace(boost::asio::make_work_guard(m_io)); // 생성(대입 아님)
    m_running.store(true);

    StartIoThread();
}

void NetManager::Finalize()
{
    Disconnect();
    StopIoThread();

    m_handlers.clear();
    m_workGuard.reset();
    m_acceptor.reset();
    m_socket.reset();
}

void NetManager::StartIoThread()
{   
    m_ioThread = std::thread([this]()//네트워크용 별도 스레드 분리
        {
            try
            {
                m_io.run();
            }
            catch (const std::exception& e)
            {
                PushEvent(NetEvent{
                    .type = NetEvent::Type::Error,
                    .errorMessage = std::string("io_context exception: ") + e.what()
                    });
            }
        });
}

void NetManager::StopIoThread()
{
    if (!m_running.exchange(false)) return;

    m_workGuard.reset();   // run이 끝날 조건을 만든다
    m_io.stop();           // 대기중인 비동기 작업 깨워서 종료

    if (m_ioThread.joinable())//스레드가 끝날때까지 기다린 후 정리
        m_ioThread.join();

    m_io.restart(); // 재사용 가능하게
}


bool NetManager::StartHost(uint16_t port)
{
    Initialize();
    m_isHost = true;

    try
    {
        tcp::endpoint ep(tcp::v4(), port);
        m_acceptor = std::make_unique<tcp::acceptor>(m_io);
        m_acceptor->open(ep.protocol());
        m_acceptor->set_option(tcp::acceptor::reuse_address(true));
        m_acceptor->bind(ep);
        m_acceptor->listen();

        printf("[NET] Hosting started. Listening on port %u...\n", port);
        //서버 오픈 및 연결 준비
        DoAccept();
        return true;
    }
    catch (const std::exception& e)
    {
        PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = e.what() });
        return false;
    }
}

void NetManager::DoAccept()
{
    if (!m_acceptor) return;

    auto sock = std::make_shared<tcp::socket>(m_io);
    m_acceptor->async_accept(*sock, [this, sock](const boost::system::error_code& ec)
        {
            if (ec)
            {
                PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = ec.message() });
                return;
            }

            // 단일 연결만 지원: 기존 연결이 있으면 끊고 교체
            m_socket = std::make_unique<tcp::socket>(std::move(*sock));
            m_connected.store(true);

            PushEvent(NetEvent{ .type = NetEvent::Type::Connected, .peerId = 2 });
            BeginReadHeader();

            // 다음 연결도 계속 받으려면:
            // DoAccept();
        });
}

bool NetManager::Connect(const std::string& host, uint16_t port)
{
    Initialize();
    m_isHost = false;

    try
    {
        DoConnect(host, port);
        return true;
    }
    catch (const std::exception& e)
    {
        PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = e.what() });
        return false;
    }
}

void NetManager::DoConnect(const std::string& host, uint16_t port)
{
    auto resolver = std::make_shared<tcp::resolver>(m_io);
    auto sock = std::make_shared<tcp::socket>(m_io);

    resolver->async_resolve(host, std::to_string(port),
        [this, resolver, sock](const boost::system::error_code& ec,
            tcp::resolver::results_type results)
        {
            if (ec)
            {
                PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = ec.message() });
                return;
            }

            boost::asio::async_connect(*sock, results,
                [this, sock](const boost::system::error_code& ec2, const tcp::endpoint&)
                {
                    if (ec2)
                    {
                        PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = ec2.message() });
                        return;
                    }

                    m_socket = std::make_unique<tcp::socket>(std::move(*sock));
                    m_connected.store(true);

                    PushEvent(NetEvent{ .type = NetEvent::Type::Connected, .peerId = 1 });
                    BeginReadHeader();
                });
        });
}

void NetManager::Disconnect()
{
    m_connected.store(false);

    boost::asio::post(m_io, [this]()
        {
            if (m_socket)
            {
                boost::system::error_code ec;
                m_socket->shutdown(tcp::socket::shutdown_both, ec);
                m_socket->close(ec);
                m_socket.reset();
            }

            if (m_acceptor)
            {
                boost::system::error_code ec;
                m_acceptor->close(ec);
            }
        });
}

void NetManager::BeginReadHeader()
{
    if (!m_socket) return;

    boost::asio::async_read(*m_socket, boost::asio::buffer(m_readLenBuf),
        [this](const boost::system::error_code& ec, std::size_t)
        {
            if (ec)
            {
                m_connected.store(false);
                PushEvent(NetEvent{ .type = NetEvent::Type::Disconnected, .peerId = 0, .errorMessage = ec.message() });
                return;
            }

            uint32_t bodyLen = ReadU32LE(m_readLenBuf.data());
            if (bodyLen < 2 || bodyLen >(1024u * 1024u)) // 최소 msgId 2바이트, 1MB 제한(임의)
            {
                m_connected.store(false);
                PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = "Invalid body length" });
                return;
            }

            BeginReadBody(bodyLen);
        });
}
/*
BeginReadHeader()
4바이트(length) 읽는다
읽고 나서 bodyLen 계산
bodyLen 검증(최소 2, 최대 1MB)
BeginReadBody(bodyLen) 호출
*/
void NetManager::BeginReadBody(std::size_t bodyLen)
{
    if (!m_socket) return;

    m_readBodyBuf.resize(bodyLen);
    boost::asio::async_read(*m_socket, boost::asio::buffer(m_readBodyBuf),
        [this](const boost::system::error_code& ec, std::size_t)
        {
            if (ec)
            {
                m_connected.store(false);
                PushEvent(NetEvent{ .type = NetEvent::Type::Disconnected, .peerId = 0, .errorMessage = ec.message() });
                return;
            }

            if (m_readBodyBuf.size() < 2)
            {
                PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = "Body too small" });
                return;
            }

            MsgId msgId = ReadU16LE(m_readBodyBuf.data());
            std::vector<uint8_t> payload;
            payload.assign(m_readBodyBuf.begin() + 2, m_readBodyBuf.end());

            PushEvent(NetEvent{
                .type = NetEvent::Type::Message,
                .peerId = 0,
                .msgId = msgId,
                .payload = std::move(payload)
                });

            BeginReadHeader(); // 계속 읽기
        });
}
/*
BeginReadBody(bodyLen)
bodyLen 바이트 읽는다
첫 2바이트에서 msgId 파싱
나머지를 payload로 분리
NetEvent(Message)를 큐에 push
다시 BeginReadHeader() 호출해서 다음 메시지 받음
*/

//콜백체인
void NetManager::PushEvent(NetEvent&& ev)
{
    std::lock_guard lock(m_eventMtx); //mutex로 보호해서 큐에 push
    m_events.push(std::move(ev));
}

void NetManager::Update()
{
    //네트워크 업데이트 디버깅
    //static int tick = 0;
    //if (++tick % 60 == 0) printf("[NET] NetManager::Update tick\n");
    // 메인 스레드에서 호출
    std::queue<NetEvent> local;
    {
        std::lock_guard lock(m_eventMtx);
        std::swap(local, m_events);     //큐 통째로 빼서 락 풀고 계산
    }

    while (!local.empty())
    {
        const NetEvent& ev = local.front();

        switch (ev.type)
        {
        case NetEvent::Type::Connected:
            if (m_onConnected) m_onConnected(ev.peerId);
            break;
        case NetEvent::Type::Disconnected:
            if (m_onDisconnected) m_onDisconnected(ev.peerId);
            break;
        case NetEvent::Type::Error:
            if (m_onError) m_onError(ev.errorMessage);
            break;
        case NetEvent::Type::Message:
        {
            auto it = m_handlers.find(ev.msgId);
            if (it != m_handlers.end())
                it->second(ev);
        }
        break;
        }
        
        local.pop();
    }
}

void NetManager::RegisterHandler(MsgId msgId, Handler handler)
{
    m_handlers[msgId] = std::move(handler);
}

void NetManager::UnregisterHandler(MsgId msgId)
{
    m_handlers.erase(msgId);
}

std::vector<uint8_t> NetManager::BuildFrame(MsgId msgId, const uint8_t* payload, size_t payloadSize)
{
    // [u32 len][u16 msgId][payload...] <- 길이, ID타입 , 실제데이터
    uint32_t bodyLen = uint32_t(2 + payloadSize);
    std::vector<uint8_t> buf;
    buf.resize(4 + bodyLen);

    WriteU32LE(buf.data(), bodyLen);
    WriteU16LE(buf.data() + 4, msgId);

    if (payloadSize > 0 && payload)
        std::memcpy(buf.data() + 6, payload, payloadSize);

    return buf;
}

void NetManager::DoWrite(std::vector<uint8_t>&& framedPacket)
{
    if (!m_socket) return;

    std::lock_guard lock(m_writeMtx);
    m_writeQueue.push(std::move(framedPacket));

    if (m_writing) return;
    m_writing = true;

    auto& front = m_writeQueue.front();
    boost::asio::async_write(*m_socket, boost::asio::buffer(front),
        [this](const boost::system::error_code& ec, std::size_t)
        {
            std::lock_guard lock(m_writeMtx);

            if (ec)
            {
                m_writing = false;
                PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = ec.message() });
                return;
            }

            m_writeQueue.pop();
            if (m_writeQueue.empty())
            {
                m_writing = false;
                return;
            }

            auto& next = m_writeQueue.front();
            boost::asio::async_write(*m_socket, boost::asio::buffer(next),
                [this](const boost::system::error_code& ec2, std::size_t)
                {
                    if (ec2)
                    {
                        std::lock_guard lock2(m_writeMtx);
                        m_writing = false;
                        PushEvent(NetEvent{ .type = NetEvent::Type::Error, .errorMessage = ec2.message() });
                    }
                });
        });
}

bool NetManager::SendRaw(MsgId msgId, const void* data, size_t size)
{
    if (!m_connected.load() || !m_socket) return false;

    auto frame = BuildFrame(msgId, reinterpret_cast<const uint8_t*>(data), size);
    boost::asio::post(m_io, [this, frame = std::move(frame)]() mutable
        {
            DoWrite(std::move(frame));
        });
    return true;
}

bool NetManager::SendRaw(MsgId msgId, const std::vector<uint8_t>& bytes)
{
    return SendRaw(msgId, bytes.data(), bytes.size());
}

std::vector<uint8_t> NetManager::JsonToBytes(const nlohmann::json& j)
{
    std::string s = j.dump();
    return std::vector<uint8_t>(s.begin(), s.end());
}

nlohmann::json NetManager::BytesToJson(const std::vector<uint8_t>& bytes)
{
    if (bytes.empty()) return nlohmann::json{};
    std::string s(bytes.begin(), bytes.end());
    return nlohmann::json::parse(s, nullptr, false);
}

bool NetManager::SendJson(MsgId msgId, const nlohmann::json& j)
{
    auto bytes = JsonToBytes(j);
    return SendRaw(msgId, bytes);
}
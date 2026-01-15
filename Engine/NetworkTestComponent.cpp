#include "stdafx.h"
#include "NetworkTestComponent.h"
#include "NetworkWorld.h"

REGISTER_TYPE(NetworkTestComponent)

#pragma pack(push, 1)
struct AssignPeerId
{
    NetManager::PeerId assignedId;
};
#pragma pack(pop)

void NetworkTestComponent::Initialize()
{
	// 핸들러 등록
    auto& net = NetManager::GetInstance();

    RegisterHandlers();

    if (m_autoConnect)
    {
        auto& net = NetManager::GetInstance();
        net.Initialize();

        if (m_isHost) net.StartHost(static_cast<uint16_t>(m_port));
        else          net.Connect(m_hostIp, static_cast<uint16_t>(m_port));
    }
}

void NetworkTestComponent::Finalize()
{
    UnregisterHandlers();
}

void NetworkTestComponent::UnregisterHandlers()
{
    if (!m_handlersRegistered) return;
    m_handlersRegistered = false;

    auto& net = NetManager::GetInstance();
    net.UnregisterHandler(MSG_HELLO);
    net.UnregisterHandler(MSG_ASSIGN_ID);
}

void NetworkTestComponent::RegisterHandlers()
{
    if (m_handlersRegistered) return;
    m_handlersRegistered = true;

    auto& net = NetManager::GetInstance();

    // client -> host: hello 받으면 assign 보내기
    net.RegisterHandler(MSG_HELLO, [&](const NetManager::NetEvent& ev)
        {
            if (!net.IsHost()) return;

            AssignPeerId msg{ 2 }; // 단일 클라 테스트용 고정
            NetworkWorld::HostRegisterExpectedPeer(msg.assignedId);
            net.SendRaw(MSG_ASSIGN_ID, &msg, sizeof(msg));
            
            printf("[HOST] assign peerId=%u (from peer=%u)\n", msg.assignedId, ev.peerId);
        });

    // host -> client: assign 받으면 self peerId 설정
    net.RegisterHandler(MSG_ASSIGN_ID, [&](const NetManager::NetEvent& ev)
        {
            if (ev.payload.size() != sizeof(AssignPeerId)) return;

            AssignPeerId msg{};
            memcpy(&msg, ev.payload.data(), sizeof(msg));

            net.SetSelfPeerId(msg.assignedId);
            printf("[CLIENT] received assigned peerId=%u\n", msg.assignedId);
        });

    net.SetOnConnected([&](NetManager::PeerId)
        {
            printf("[NET] Connected! host=%d\n", (int)net.IsHost());

            if (net.IsHost())
            {
                net.SetSelfPeerId(1);
                NetworkWorld::HostRegisterExpectedPeer(net.GetSelfPeerId());
            }
            else
            {
                SendHello();
            }
        });

    net.SetOnDisconnected([](NetManager::PeerId)
        {
            printf("[NET] Disconnected!\n");
        });

    net.SetOnError([](const std::string& e)
        {
            printf("[NET] Error: %s\n", e.c_str());
        });
}


void NetworkTestComponent::SendHello()
{
    auto& net = NetManager::GetInstance();
    if (!net.IsConnected()) return;

    uint8_t dummy = 0;
    net.SendRaw(MSG_HELLO, &dummy, sizeof(dummy));

    printf("[CLIENT] SendHello\n");
}

void NetworkTestComponent::RenderImGui()
{
    auto& net = NetManager::GetInstance();

    ImGui::SeparatorText("Network Test");

    ImGui::Checkbox("Auto Connect on Initialize", &m_autoConnect);
    ImGui::Checkbox("Host (Listen Server)", &m_isHost);

	// IP / Port 입력
    char ipBuf[256];
    strcpy_s(ipBuf, m_hostIp.c_str());
    if (ImGui::InputText("Host IP", ipBuf, sizeof(ipBuf)))
        m_hostIp = ipBuf;

    ImGui::InputInt("Port", &m_port);
    if (m_port < 1) m_port = 1;
    if (m_port > 65535) m_port = 65535;

	// 상태 표시
    ImGui::Text("Running: %s", net.IsRunning() ? "true" : "false");
    ImGui::Text("Connected: %s", net.IsConnected() ? "true" : "false");
    ImGui::Text("SelfPeerId: %u", net.GetSelfPeerId());

	// 버튼들
    if (ImGui::Button("Initialize Net"))
    {
        net.Initialize();
    }
    ImGui::SameLine();
    if (ImGui::Button("Disconnect"))
    {
        net.Disconnect();
    }

    if (ImGui::Button("Start Host"))
    {
        net.Initialize();
        net.StartHost(static_cast<uint16_t>(m_port));
    }
    ImGui::SameLine();
    if (ImGui::Button("Connect"))
    {
        net.Initialize();
        net.Connect(m_hostIp, static_cast<uint16_t>(m_port));
    }

    if (ImGui::Button("Send HELLO"))
    {
        SendHello();
    }
}

nlohmann::json NetworkTestComponent::Serialize()
{
    nlohmann::json j;
    j["autoConnect"] = m_autoConnect;
    j["isHost"] = m_isHost;
    j["hostIp"] = m_hostIp;
    j["port"] = m_port;
    return j;
}

void NetworkTestComponent::Deserialize(const nlohmann::json& jsonData)
{
    if (jsonData.contains("autoConnect")) m_autoConnect = jsonData["autoConnect"].get<bool>();
    if (jsonData.contains("isHost"))      m_isHost = jsonData["isHost"].get<bool>();
    if (jsonData.contains("hostIp"))      m_hostIp = jsonData["hostIp"].get<std::string>();
    if (jsonData.contains("port"))        m_port = jsonData["port"].get<int>();

    if (m_port < 1) m_port = 1;
    if (m_port > 65535) m_port = 65535;
}

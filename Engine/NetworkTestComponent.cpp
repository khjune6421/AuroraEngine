#include "stdafx.h"
#include "NetworkTestComponent.h"

REGISTER_TYPE(NetworkTestComponent)

void NetworkTestComponent::Initialize()
{
	// 핸들러 등록
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

void NetworkTestComponent::RegisterHandlers()
{
    if (m_handlersRegistered) return;
    m_handlersRegistered = true;

    auto& net = NetManager::GetInstance();

    net.RegisterHandler(MSG_HELLO, [](const NetManager::NetEvent& ev)
        {
            auto j = NetManager::BytesToJson(ev.payload);
            printf("[NET] MSG_HELLO received: %s\n", j.dump().c_str());
        });

    net.SetOnConnected([](NetManager::PeerId)
        {
            printf("[NET] Connected!\n");
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

void NetworkTestComponent::UnregisterHandlers()
{
    if (!m_handlersRegistered) return;
    m_handlersRegistered = false;

    auto& net = NetManager::GetInstance();
    net.UnregisterHandler(MSG_HELLO);
}

void NetworkTestComponent::SendHello()
{
    nlohmann::json j;
    j["hello"] = "ping";
    j["time"] = static_cast<int>(time(nullptr));

    NetManager::GetInstance().SendJson(MSG_HELLO, j);
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

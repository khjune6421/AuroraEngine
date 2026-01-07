/// LineComponent.cpp의 시작
#include "stdafx.h"
#include "LineComponent.h"

#include "Renderer.h"
#include "ResourceManager.h"

REGISTER_TYPE(LineComponent)

Vertex_Pos g_line_vertices[] =
{
	{{-0.5f, 0.0f, 0.0f, 1.0f}}, 
	{{0.5f, 0.0f, 0.0f, 1.0f}}
};

void LineComponent::Initialize()
{
	m_deviceContext = Renderer::GetInstance().GetDeviceContext();

	ResourceManager& resourceManager = ResourceManager::GetInstance();

	CreateShaders();
}

void LineComponent::Render()
{
	constexpr UINT stride = sizeof(Vertex_Pos);
	constexpr UINT offset = 0;

	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	m_deviceContext->IASetInputLayout(m_vertexShaderAndInputLayout.second.Get());
	m_deviceContext->VSSetShader(m_vertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	m_deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	m_deviceContext->Draw(2, 0);
}

void LineComponent::RenderImGui()
{
	char vsShaderNameBuffer[256];
	strcpy_s(vsShaderNameBuffer, m_vsShaderName.c_str());
	if (ImGui::InputText("Vertex Shader Name", vsShaderNameBuffer, sizeof(vsShaderNameBuffer))) m_vsShaderName = vsShaderNameBuffer;

	char psShaderNameBuffer[256];
	strcpy_s(psShaderNameBuffer, m_psShaderName.c_str());
	if (ImGui::InputText("Pixel Shader Name", psShaderNameBuffer, sizeof(psShaderNameBuffer))) m_psShaderName = psShaderNameBuffer;

	if (ImGui::Button("Load"))
	{
		//m_model = ResourceManager::GetInstance().LoadModel(m_modelFileName);
		CreateShaders();
	}
}

void LineComponent::CreateShaders()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();
	m_vertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout(m_vsShaderName, m_inputElements);
	m_pixelShader = resourceManager.GetPixelShader(m_psShaderName);
	m_vertexBuffer = resourceManager.CreateVertexBuffer(g_line_vertices, sizeof(Vertex_Pos), 2, false);
}
/// LineComponent.cpp의 끝
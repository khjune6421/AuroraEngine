/// ModelComponent.cpp의 시작
#include "stdafx.h"
#include "ModelComponent.h"

#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;

REGISTER_TYPE(ModelComponent)

void ForceLink_ModelComponent() {}

void ModelComponent::Initialize()
{
	m_deviceContext = Renderer::GetInstance().GetDeviceContext();

	ResourceManager& resourceManager = ResourceManager::GetInstance();

	CreateShaders();

	m_materialConstantBuffer = resourceManager.GetConstantBuffer(PSConstBuffers::MaterialFactor);
	m_model = resourceManager.LoadModel(m_modelFileName);
}

void ModelComponent::Render()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();
	resourceManager.SetBlendState(m_blendState);
	resourceManager.SetRasterState(m_rasterState);

	m_deviceContext->IASetInputLayout(m_vertexShaderAndInputLayout.second.Get());
	m_deviceContext->VSSetShader(m_vertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	for (const auto& mesh : m_model->meshes)
	{
		// 나중에 메쉬별로 설정 가능하게 변경
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 메쉬 버퍼 설정
		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0;
		m_deviceContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		m_deviceContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// 재질 상수 버퍼 셰이더에 설정
		m_deviceContext->UpdateSubresource(m_materialConstantBuffer.Get(), 0, nullptr, &mesh.materialFactor, 0, 0);

		// 재질 텍스처 셰이더에 설정
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Albedo), 1, mesh.materialTexture.albedoTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::ORM), 1, mesh.materialTexture.ORMTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Normal), 1, mesh.materialTexture.normalTextureSRV.GetAddressOf());

		m_deviceContext->DrawIndexed(mesh.indexCount, 0, 0);
	}
}

void ModelComponent::RenderImGui()
{
	char modelFileNameBuffer[256];
	strcpy_s(modelFileNameBuffer, m_modelFileName.c_str());
	if (ImGui::InputText("Model File Name", modelFileNameBuffer, sizeof(modelFileNameBuffer))) m_modelFileName = modelFileNameBuffer;

	char vsShaderNameBuffer[256];
	strcpy_s(vsShaderNameBuffer, m_vsShaderName.c_str());
	if (ImGui::InputText("Vertex Shader Name", vsShaderNameBuffer, sizeof(vsShaderNameBuffer))) m_vsShaderName = vsShaderNameBuffer;

	char psShaderNameBuffer[256];
	strcpy_s(psShaderNameBuffer, m_psShaderName.c_str());
	if (ImGui::InputText("Pixel Shader Name", psShaderNameBuffer, sizeof(psShaderNameBuffer))) m_psShaderName = psShaderNameBuffer;

	if (ImGui::Button("Load"))
	{
		m_model = ResourceManager::GetInstance().LoadModel(m_modelFileName);
		CreateShaders();
	}

	ImGui::Separator();
	int blendStateInt = static_cast<int>(m_blendState);
	if (ImGui::Combo("Blend State", &blendStateInt, "Opaque\0AlphaBlend\0AlphaToCoverage\0"))
	{
		m_blendState = static_cast<BlendState>(blendStateInt);
		ResourceManager::GetInstance().SetBlendState(m_blendState);
	}
	int rasterStateInt = static_cast<int>(m_rasterState);
	if (ImGui::Combo("Raster State", &rasterStateInt, "BackBuffer\0Solid\0Wireframe\0"))
	{
		m_rasterState = static_cast<RasterState>(rasterStateInt);
		ResourceManager::GetInstance().SetRasterState(m_rasterState);
	}
}

nlohmann::json ModelComponent::Serialize()
{
	nlohmann::json jsonData;

	jsonData["vsShaderName"] = m_vsShaderName;
	jsonData["psShaderName"] = m_psShaderName;
	jsonData["modelFileName"] = m_modelFileName;

	jsonData["blendState"] = static_cast<int>(m_blendState);
	jsonData["rasterState"] = static_cast<int>(m_rasterState);

	return jsonData;
}

void ModelComponent::Deserialize(const nlohmann::json& jsonData)
{
	m_vsShaderName = jsonData["vsShaderName"].get<string>();
	m_psShaderName = jsonData["psShaderName"].get<string>();
	m_modelFileName = jsonData["modelFileName"].get<string>();

	m_blendState = static_cast<BlendState>(jsonData["blendState"].get<int>());
	m_rasterState = static_cast<RasterState>(jsonData["rasterState"].get<int>());
}

void ModelComponent::CreateShaders()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();
	m_vertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout(m_vsShaderName, m_inputElements);
	m_pixelShader = resourceManager.GetPixelShader(m_psShaderName);
}
/// ModelComponent.cpp의 끝
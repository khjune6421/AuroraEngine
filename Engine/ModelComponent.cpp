/// ModelComponent.cpp의 시작
#include "stdafx.h"
#include "ModelComponent.h"

#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;

REGISTER_TYPE(ModelComponent)

void ModelComponent::Initialize()
{
	m_deviceContext = Renderer::GetInstance().GetDeviceContext();

	ResourceManager& resourceManager = ResourceManager::GetInstance();

	m_materialConstantBuffer = resourceManager.GetConstantBuffer(sizeof(MaterialFactor)); // TODO: 매번 재질 상수 버퍼 생성하지 말고 공유하도록 변경
	m_model = resourceManager.LoadModel(m_modelFileName);

	CreateShaders();
}

void ModelComponent::Render()
{
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;

	m_deviceContext->IASetInputLayout(m_vertexShaderAndInputLayout.second.Get());
	m_deviceContext->VSSetShader(m_vertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	for (const auto& mesh : m_model->meshes)
	{
		m_deviceContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		m_deviceContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// 나중에 메쉬별로 설정 가능하게 변경
		m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 재질 상수 버퍼 셰이더에 설정
		m_deviceContext->UpdateSubresource(m_materialConstantBuffer.Get(), 0, nullptr, &mesh.materialFactor, 0, 0);
		m_deviceContext->PSSetConstantBuffers(static_cast<UINT>(PSConstBuffers::Material), 1, m_materialConstantBuffer.GetAddressOf());

		// 재질 텍스처 셰이더에 설정
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Albedo), 1, mesh.materialTexture.albedoTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Normal), 1, mesh.materialTexture.normalTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Metallic), 1, mesh.materialTexture.metallicTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Roughness), 1, mesh.materialTexture.roughnessTextureSRV.GetAddressOf());
		m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::AmbientOcclusion), 1, mesh.materialTexture.ambientOcclusionTextureSRV.GetAddressOf());

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
}

nlohmann::json ModelComponent::Serialize()
{
	nlohmann::json jsonData;

	jsonData["modelFileName"] = m_modelFileName;
	jsonData["vsShaderName"] = m_vsShaderName;
	jsonData["psShaderName"] = m_psShaderName;

	return jsonData;
}

void ModelComponent::Deserialize(const nlohmann::json& jsonData)
{
	m_modelFileName = jsonData["modelFileName"].get<string>();
	m_vsShaderName = jsonData["vsShaderName"].get<string>();
	m_psShaderName = jsonData["psShaderName"].get<string>();
}

void ModelComponent::CreateShaders()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();
	m_vertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout(m_vsShaderName, m_inputElements);
	m_pixelShader = resourceManager.GetPixelShader(m_psShaderName);
}
/// ModelComponent.cpp의 끝
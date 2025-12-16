#include "stdafx.h"
#include "ModelComponent.h"

#include "GameObjectBase.h"
#include "Renderer.h"
#include "RenderResourceManager.h"

void ModelComponent::Render()
{
	const com_ptr<ID3D11DeviceContext> deviceContext = Renderer::GetInstance().GetDeviceContext();

	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetInputLayout(m_vertexShaderAndInputLayout.second.Get());
	deviceContext->VSSetShader(m_vertexShaderAndInputLayout.first.Get(), nullptr, 0);
	deviceContext->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	for (const auto& mesh : m_model->meshes)
	{
		deviceContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		deviceContext->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		deviceContext->DrawIndexed(mesh.indexCount, 0, 0);
	}
}

void ModelComponent::Begin()
{
	m_model = RenderResourceManager::GetInstance().LoadModel(m_modelFileName);

	CreateShaders();
}

void ModelComponent::CreateShaders()
{
	RenderResourceManager& resourceManager = RenderResourceManager::GetInstance();
	m_vertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout(m_vsShaderName, m_inputElements);
	m_pixelShader = resourceManager.GetPixelShader(m_psShaderName);
}
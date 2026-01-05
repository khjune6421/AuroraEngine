///SceneBase.h의 시작
#pragma once
#include "Base.h"
#include "TypeRegistry.h"
#include "GameObjectBase.h"
#ifdef _DEBUG
#include "DebugCamera.h"
#endif

class SceneBase : public Base
{
	#ifdef _DEBUG
	std::unique_ptr<DebugCamera> m_debugCamera = nullptr; // 디버그 카메라 게임 오브젝트
	#endif

	com_ptr<ID3D11DeviceContext> m_deviceContext = nullptr; // 디바이스 컨텍스트 포인터

	std::vector<std::unique_ptr<Base>> m_gameObjects = {}; // 게임 오브젝트 배열
	std::vector<Base*> m_gameObjectsToRemove = {}; // 제거할 게임 오브젝트 배열

	struct ViewProjectionBuffer // 뷰-투영 상수 버퍼 구조체
	{
		DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity(); // 뷰 행렬
		DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixIdentity(); // 투영 행렬
		DirectX::XMMATRIX VPMatrix = DirectX::XMMatrixIdentity(); // VP 행렬
	};
	ViewProjectionBuffer m_viewProjectionData = {}; // 뷰-투영 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_viewProjectionConstantBuffer = nullptr; // 뷰-투영 상수 버퍼

	DirectX::XMVECTOR m_cameraPosition = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f); // 카메라 위치
	com_ptr<ID3D11Buffer> m_cameraPositionConstantBuffer = nullptr; // 카메라 위치 상수 버퍼

	struct DirectionalLightBuffer // 방향광 상수 버퍼 구조체
	{
		DirectX::XMVECTOR lightDirection = DirectX::XMVectorSet(-0.5f, -1.0f, -0.5f, 0.0f); // 방향광 방향
		DirectX::XMFLOAT4 lightColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 방향광 색상
	};
	DirectionalLightBuffer m_directionalLightData = {}; // 방향광 상수 버퍼 데이터
	com_ptr<ID3D11Buffer> m_directionalLightConstantBuffer = nullptr; // 방향광 상수 버퍼

	std::string m_environmentMapFileName = "Skybox.dds"; // 환경 맵 파일 이름
	com_ptr<ID3D11ShaderResourceView> m_environmentMapSRV = nullptr; // 환경 맵 셰이더 리소스 뷰

	std::pair<com_ptr<ID3D11VertexShader>, com_ptr<ID3D11InputLayout>> m_skyboxVertexShaderAndInputLayout = {}; // 스카이박스 정점 셰이더
	com_ptr<ID3D11PixelShader> m_skyboxPixelShader = nullptr; // 스카이박스 픽셀 셰이더
	com_ptr<ID3D11DepthStencilState> m_skyboxDepthStencilState = nullptr; // 스카이박스 깊이버퍼 상태

protected:
	class CameraComponent* m_mainCamera = nullptr; // 메인 카메라 컴포넌트 포인터
	DirectX::XMVECTOR m_directionalLightDirection = DirectX::XMVectorSet(-0.5f, -1.0f, -0.5f, 0.0f); // 방향광 방향
	DirectX::XMFLOAT4 m_lightColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 환경광, 방향광

public:
	SceneBase();
	virtual ~SceneBase() = default;
	SceneBase(const SceneBase&) = delete; // 복사 금지
	SceneBase& operator=(const SceneBase&) = delete; // 복사 대입 금지
	SceneBase(SceneBase&&) = delete; // 이동 금지
	SceneBase& operator=(SceneBase&&) = delete; // 이동 대입 금지

	// 루트 게임 오브젝트 생성 // 포인터 반환 안함
	void CreateRootGameObject(std::string typeName);

	template<typename T> requires std::derived_from<T, GameObjectBase>
	T* CreateRootGameObject(); // 루트 게임 오브젝트 생성 // 포인터 반환

	template<typename T> requires std::derived_from<T, GameObjectBase>
	T* CreateRootGameObject(std::string typeName); // 루트 게임 오브젝트 생성 // 포인터 반환

	// 루트 게임 오브젝트 제거 // 제거 배열에 추가
	void RemoveGameObject(GameObjectBase* gameObject) { m_gameObjectsToRemove.push_back(gameObject); }

protected:
	// 메인 카메라 게임 오브젝트 설정
	virtual GameObjectBase* CreateCameraObject();

private:
	// 씬 초기화 // 씬 사용 전 반드시 호출해야 함
	void BaseInitialize() override;
	// 씬 업데이트 // 씬 매니저가 호출
	void BaseUpdate() override;
	// 씬 렌더링 // 씬 매니저가 호출
	void BaseRender() override;
	// ImGui 렌더링
	void BaseRenderImGui() override;
	// 씬 종료 // 씬 매니저가 씬을 교체할 때 호출
	void BaseFinalize() override;

	// 씬 직렬화
	nlohmann::json BaseSerialize() override;
	// 씬 역직렬화
	void BaseDeserialize(const nlohmann::json& jsonData) override;

	// 리소스 매니저에서 필요한 리소스 얻기
	void GetResources();
	// 상수 버퍼 업데이트
	void UpdateConstantBuffers();
	// 스카이박스 렌더링
	void RenderSkybox();
	// 제거할 게임 오브젝트 제거 // Update에서 호출
	void RemovePendingGameObjects();
};

template<typename T> requires std::derived_from<T, GameObjectBase>
inline T* SceneBase::CreateRootGameObject()
{
	std::unique_ptr<Base> gameObject = std::make_unique<T>();

	gameObject->BaseInitialize();
	T* gameObjectPtr = static_cast<T*>(gameObject.get());
	m_gameObjects.push_back(std::move(gameObject));

	return gameObjectPtr;
}

template<typename T> requires std::derived_from<T, GameObjectBase>
inline T* SceneBase::CreateRootGameObject(std::string typeName)
{
	std::unique_ptr<Base> gameObject = TypeRegistry::GetInstance().CreateGameObject(typeName);

	gameObject->BaseInitialize();
	T* gameObjectPtr = static_cast<T*>(gameObject.get());
	m_gameObjects.push_back(std::move(gameObject));

	return gameObjectPtr;
}
///SceneBase.h의 끝
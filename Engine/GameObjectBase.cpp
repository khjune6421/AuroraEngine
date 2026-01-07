/// GameObjectBase.cpp의 시작
#include "stdafx.h"
#include "GameObjectBase.h"

#include "Renderer.h"
#include "ResourceManager.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(GameObjectBase)

GameObjectBase::GameObjectBase()
{
	static UINT idIndex = 0;
	m_id = idIndex++;

	m_deviceContext = Renderer::GetInstance().GetDeviceContext();
}

void GameObjectBase::MoveDirection(float distance, Direction direction)
{
	XMVECTOR directionVector = GetDirectionVector(direction);
	XMVECTOR deltaPosition = XMVectorScale(directionVector, distance);

	MovePosition(deltaPosition);
}

void GameObjectBase::SetRotation(const XMVECTOR& rotation)
{
	m_euler = rotation;
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(ToRadians(m_euler)); // 라디안으로 변환

	SetDirty();
}

void GameObjectBase::Rotate(const XMVECTOR& deltaRotation)
{
	m_euler = XMVectorAdd(m_euler, deltaRotation);
	m_quaternion = XMQuaternionRotationRollPitchYawFromVector(ToRadians(m_euler)); // 라디안으로 변환

	SetDirty();
}

void GameObjectBase::LookAt(const XMVECTOR& targetPosition, const XMVECTOR& upDirection)
{
	XMVECTOR direction = XMVector3Normalize(XMVectorSubtract(targetPosition, UpdateWorldMatrix().r[3]));
	XMVECTOR right = XMVector3Cross(upDirection, direction);
	XMVECTOR up = XMVector3Cross(direction, right);

	m_quaternion = XMQuaternionRotationMatrix({ right, up, direction, { 0.0f, 0.0f, 0.0f, 1.0f } });
	m_euler = ToDegrees(static_cast<XMVECTOR>(static_cast<SimpleMath::Quaternion>(m_quaternion).ToEuler())); // 도 단위로 변환

	SetDirty();
}

XMVECTOR GameObjectBase::GetDirectionVector(Direction direction) const
{
	switch (direction)
	{
	case Direction::Left:
		return XMVector3Rotate({ -1.0f, 0.0f, 0.0f, 0.0f }, m_quaternion);
	case Direction::Right:
		return XMVector3Rotate({ 1.0f, 0.0f, 0.0f, 0.0f }, m_quaternion);

	case Direction::Up:
		return XMVector3Rotate({ 0.0f, 1.0f, 0.0f, 0.0f }, m_quaternion);
	case Direction::Down:
		return XMVector3Rotate({ 0.0f, -1.0f, 0.0f, 0.0f }, m_quaternion);

	case Direction::Forward:
		return XMVector3Rotate({ 0.0f, 0.0f, 1.0f, 0.0f }, m_quaternion);
	case Direction::Backward:
		return XMVector3Rotate({ 0.0f, 0.0f, -1.0f, 0.0f }, m_quaternion);

	default:
		return XMVectorZero();
	}
}

XMVECTOR GameObjectBase::GetWorldDirectionVector(Direction direction)
{
	XMMATRIX worldMatrix = UpdateWorldMatrix();

	switch (direction)
	{
	case Direction::Left:
		return XMVector3Normalize(worldMatrix.r[0] * -1.0f);
	case Direction::Right:
		return XMVector3Normalize(worldMatrix.r[0]);

	case Direction::Up:
		return XMVector3Normalize(worldMatrix.r[1]);
	case Direction::Down:
		return XMVector3Normalize(worldMatrix.r[1] * -1.0f);

	case Direction::Forward:
		return XMVector3Normalize(worldMatrix.r[2]);
	case Direction::Backward:
		return XMVector3Normalize(worldMatrix.r[2] * -1.0f);

	default:
		return XMVectorZero();
	}
}

void GameObjectBase::CreateComponent(string typeName)
{
	unique_ptr<ComponentBase> component = TypeRegistry::GetInstance().CreateComponent(typeName);

	if (m_components[type_index(typeid(*component))])
	{
		#ifdef _DEBUG
		cerr << "오류: 게임 오브젝트 '" << m_name << "'에 이미 컴포넌트 '" << typeName << "'가 존재합니다." << endl;
		#else
		MessageBoxA(nullptr, ("오류: 게임 오브젝트 '" + m_name + "'에 이미 컴포넌트 '" + typeName + "'가 존재합니다.").c_str(), "GameObjectBase Error", MB_OK | MB_ICONERROR);
		#endif
		return;
	}

	component->SetOwner(this);

	if (component->NeedsUpdate()) m_updateComponents.push_back(component.get());
	if (component->NeedsRender()) m_renderComponents.push_back(component.get());

	static_cast<Base*>(component.get())->BaseInitialize();

	m_components[type_index(typeid(*component))] = move(component);
}

void GameObjectBase::CreateChildGameObject(string typeName)
{
	unique_ptr<GameObjectBase> childGameObjectPtr = TypeRegistry::GetInstance().CreateGameObject(typeName);
	childGameObjectPtr->m_parent = this;
	childGameObjectPtr->BaseInitialize();

	m_childrens.push_back(move(childGameObjectPtr));
}

void GameObjectBase::BaseInitialize()
{
	m_type = GetTypeName(*this);
	m_name = m_type + "_" + to_string(m_id);

	m_worldMatrixConstantBuffer = ResourceManager::GetInstance().GetConstantBuffer(VSConstBuffers::WorldNormal);

	#ifdef NDEBUG
	Initialize();
	#endif
}

void GameObjectBase::BaseUpdate()
{
	#ifdef NDEBUG
	Update();
	#endif

	// 월드 행렬 업데이트
	UpdateWorldMatrix();

	// 제거할 컴포넌트 및 자식 게임 오브젝트 제거
	RemovePending();

	// 컴포넌트 업데이트
	for (Base*& component : m_updateComponents) component->BaseUpdate();

	// 제거할 자식 게임 오브젝트 제거;
	// 자식 게임 오브젝트 업데이트
	for (auto& child : m_childrens) child->BaseUpdate();
}

void GameObjectBase::BaseRender()
{
	#ifdef NDEBUG
	Render();
	#endif

	// 컴포넌트 렌더링
	if (!m_renderComponents.empty())
	{
		// 월드 및 WVP 행렬 상수 버퍼 업데이트 및 셰이더에 설정
		m_worldData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
		m_worldData.normalMatrix = XMMatrixTranspose(m_inverseScaleSquareMatrix * m_worldMatrix);

		m_deviceContext->UpdateSubresource(m_worldMatrixConstantBuffer.Get(), 0, nullptr, &m_worldData, 0, 0);

		for (Base*& component : m_renderComponents) component->BaseRender();
	}

	// 자식 게임 오브젝트 렌더링
	for (auto& child : m_childrens) child->BaseRender();
}

void GameObjectBase::BaseRenderImGui()
{
	ImGui::PushID(this);

	if (ImGui::Button("Remove")) SetAlive(false);

	ImGui::SameLine();
	if (ImGui::TreeNode(m_name.c_str()))
	{
		// 위치
		if (ImGui::DragFloat3("Position", &m_position.m128_f32[0], 0.05f))  SetDirty();
		// 회전
		if (ImGui::DragFloat3("Rotation", &m_euler.m128_f32[0], 0.5f))
		{
			m_quaternion = XMQuaternionRotationRollPitchYawFromVector(ToRadians(m_euler)); // 라디안으로 변환
			SetDirty();
		};
		// 크기
		if (ImGui::DragFloat3("Scale", &m_scale.m128_f32[0], 0.01f)) SetDirty();

		RenderImGui();

		// 컴포넌트 ImGui 렌더링
		if (!m_components.empty())
		{
			ImGui::Separator();
			ImGui::Text("Components:");
			for (auto& [typeIndex, component] : m_components) component->BaseRenderImGui();
		}
		ImGui::Separator();
		if (ImGui::Button("Add Component")) ImGui::OpenPopup("Select Component Type");
		if (ImGui::BeginPopup("Select Component Type"))
		{
			for (const auto& [typeName, factory] : TypeRegistry::GetInstance().m_componentRegistry)
			{
				if (ImGui::Selectable(typeName.c_str()))
				{
					CreateComponent(typeName);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndPopup();
		}

		// 자식 게임 오브젝트 ImGui 렌더링
		if (!m_childrens.empty())
		{
			ImGui::Separator();
			ImGui::Text("Children:");
			for (unique_ptr<GameObjectBase>& child : m_childrens) child->BaseRenderImGui();
		}
		ImGui::Separator();
		if (ImGui::Button("Add GameObject")) ImGui::OpenPopup("Select GameObject Type");
		if (ImGui::BeginPopup("Select GameObject Type"))
		{
			for (const auto& [typeName, factory] : TypeRegistry::GetInstance().m_gameObjectRegistry)
			{
				if (ImGui::Selectable(typeName.c_str()))
				{
					CreateChildGameObject(typeName);
					ImGui::CloseCurrentPopup();
				}
			}

			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}

void GameObjectBase::BaseFinalize()
{
	#ifdef NDEBUG
	Finalize();
	#endif

	// 컴포넌트 종료
	for (auto& [typeIndex, component] : m_components) component->BaseFinalize();

	// 자식 게임 오브젝트 종료
	for (auto& child : m_childrens) child->BaseFinalize();
}

nlohmann::json GameObjectBase::BaseSerialize()
{
	nlohmann::json jsonData;

	// 게임 오브젝트 타입 저장
	jsonData["type"] = m_type;

	// 기본 게임 오브젝트 데이터 저장
	jsonData["name"] = m_name;
	jsonData["position"] = { m_position.m128_f32[0], m_position.m128_f32[1], m_position.m128_f32[2], m_position.m128_f32[3] };
	jsonData["rotation"] = { m_quaternion.m128_f32[0], m_quaternion.m128_f32[1], m_quaternion.m128_f32[2], m_quaternion.m128_f32[3] };
	jsonData["scale"] = { m_scale.m128_f32[0], m_scale.m128_f32[1], m_scale.m128_f32[2], m_scale.m128_f32[3] };

	// 파생 클래스의 직렬화 호출
	nlohmann::json derivedData = Serialize();
	if (!derivedData.is_null() && derivedData.is_object()) jsonData.merge_patch(derivedData);

	// 컴포넌트들 저장
	nlohmann::json componentsData = nlohmann::json::array();
	for (auto& [typeIndex, component] : m_components) componentsData.push_back(component->BaseSerialize());
	jsonData["components"] = componentsData;

	// 자식 게임 오브젝트들 저장
	nlohmann::json childrenData = nlohmann::json::array();
	for (auto& child : m_childrens) childrenData.push_back(child->BaseSerialize());
	jsonData["childGameObjects"] = childrenData;

	return jsonData;
}

void GameObjectBase::BaseDeserialize(const nlohmann::json& jsonData)
{
	// 기본 게임 오브젝트 데이터 로드
	m_name = jsonData["name"].get<string>();

	m_position = XMVectorSet
	(
		jsonData["position"][0].get<float>(),
		jsonData["position"][1].get<float>(),
		jsonData["position"][2].get<float>(),
		jsonData["position"][3].get<float>()
	);
	m_quaternion = XMVectorSet
	(
		jsonData["rotation"][0].get<float>(),
		jsonData["rotation"][1].get<float>(),
		jsonData["rotation"][2].get<float>(),
		jsonData["rotation"][3].get<float>()
	);
	m_scale = XMVectorSet
	(
		jsonData["scale"][0].get<float>(),
		jsonData["scale"][1].get<float>(),
		jsonData["scale"][2].get<float>(),
		jsonData["scale"][3].get<float>()
	);

	// 파생 클래스의 데이터 로드
	Deserialize(jsonData);

	// 컴포넌트들 로드
	for (const auto& componentData : jsonData["components"])
	{
		string typeName = componentData["type"].get<string>();
		unique_ptr<ComponentBase> componentPtr = TypeRegistry::GetInstance().CreateComponent(typeName);

		componentPtr->SetOwner(this);
		if (componentPtr->NeedsUpdate()) m_updateComponents.push_back(componentPtr.get());
		if (componentPtr->NeedsRender()) m_renderComponents.push_back(componentPtr.get());

		Base* basePtr = static_cast<Base*>(componentPtr.get());
		basePtr->BaseDeserialize(componentData);
		basePtr->BaseInitialize();

		m_components[type_index(typeid(*componentPtr))] = move(componentPtr);
	}
	
	// 자식 게임 오브젝트들 로드
	for (const auto& childData : jsonData["childGameObjects"])
	{
		string typeName = childData["type"].get<string>();
		unique_ptr<GameObjectBase> childGameObjectPtr = TypeRegistry::GetInstance().CreateGameObject(typeName);

		childGameObjectPtr->m_parent = this;
		childGameObjectPtr->BaseDeserialize(childData);
		childGameObjectPtr->BaseInitialize();

		m_childrens.push_back(move(childGameObjectPtr));
	}

	SetDirty();
}

void GameObjectBase::RemovePending()
{
	// 제거할 자식 게임 오브젝트 제거
	erase_if
	(
		m_childrens, [](const unique_ptr<GameObjectBase>& gameObject)
		{
			if (!gameObject->GetAlive())
			{
				gameObject->BaseFinalize();
				return true;
			}
			return false;
		}
	);
	// 제거할 컴포넌트 제거
	erase_if
	(
		m_components, [&](const auto& component)
		{
			if (!component.second->GetAlive())
			{
				component.second->BaseFinalize();
				
				// 업데이트 및 렌더링 목록에서 제거
				ComponentBase* compBasePtr = dynamic_cast<ComponentBase*>(component.second.get());
				if (compBasePtr->NeedsUpdate()) erase_if(m_updateComponents, [&](Base* updateComponent) { return updateComponent == compBasePtr; });
				if (compBasePtr->NeedsRender()) erase_if(m_renderComponents, [&](Base* renderComponent) { return renderComponent == compBasePtr; });

				return true;
			}
			return false;
		}
	);
}

void GameObjectBase::SetDirty()
{
	m_isDirty = true;
	for (auto& child : m_childrens) child->SetDirty();
}

const XMMATRIX& GameObjectBase::UpdateWorldMatrix()
{
	if (m_isDirty)
	{
		m_positionMatrix = XMMatrixTranslationFromVector(m_position);
		m_rotationMatrix = XMMatrixRotationQuaternion(m_quaternion);
		m_scaleMatrix = XMMatrixScalingFromVector(m_scale);

		m_worldMatrix = m_scaleMatrix * m_rotationMatrix * m_positionMatrix;

		XMVECTOR scaleSquared = XMVectorMultiply(m_scale, m_scale);
		XMVECTOR invScaleSquared = XMVectorReciprocal(scaleSquared);
		m_inverseScaleSquareMatrix = XMMatrixScalingFromVector(invScaleSquared);

		if (m_parent)
		{
			m_worldMatrix *= m_parent->UpdateWorldMatrix();
			m_inverseScaleSquareMatrix *= m_parent->m_inverseScaleSquareMatrix;
		}

		m_isDirty = false;
	}

	return m_worldMatrix;
}
/// GameObjectBase.cpp의 끝
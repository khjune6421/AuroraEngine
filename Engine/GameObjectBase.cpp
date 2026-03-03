#include "stdafx.h"
#include "GameObjectBase.h"

#include "SceneManager.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(GameObjectBase)

GameObjectBase* GameObjectBase::s_selectedObject = nullptr;

GameObjectBase::GameObjectBase()
{
	static UINT idIndex = 0;
	m_id = idIndex++;
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

ComponentBase* GameObjectBase::CreateComponent(const string& typeName)
{
	unique_ptr<ComponentBase> component = TypeRegistry::GetInstance().CreateComponent(typeName);
	ComponentBase* componentPtr = component.get();

	if (m_components[type_index(typeid(*component))])
	{
		LOG_ERROR("오류: 게임 오브젝트 '" << m_name << "'에 이미 컴포넌트 '" << typeName << "'가 존재합니다.");
		return nullptr;
	}

	component->SetOwner(this);

	if (component->NeedsFixedUpdate()) m_fixedUpdateComponents.push_back(component.get());
	if (component->NeedsUpdate()) m_updateComponents.push_back(component.get());
	if (component->NeedsRender()) m_renderComponents.push_back(component.get());

	static_cast<Base*>(componentPtr)->BaseInitialize();
	m_components[type_index(typeid(*component))] = move(component);

	return componentPtr;
}

GameObjectBase* GameObjectBase::CreateChildGameObject(const string& typeName)
{
	unique_ptr<GameObjectBase> childGameObject = TypeRegistry::GetInstance().CreateGameObject(typeName);
	GameObjectBase* childGameObjectPtr = childGameObject.get();

	childGameObject->m_parent = this;
	childGameObject->BaseInitialize();

	m_childrens.push_back(move(childGameObject));

	return childGameObjectPtr;
}

GameObjectBase* GameObjectBase::CreatePrefabChildGameObject(const string& prefabFileName)
{
	return CreateFromJson(*SceneManager::GetInstance().GetPrefabData(prefabFileName));
}

GameObjectBase* GameObjectBase::CreateFromJson(const nlohmann::json& jsonData)
{
	unique_ptr<GameObjectBase> childGameObject = TypeRegistry::GetInstance().CreateGameObject(jsonData["type"].get<string>());
	GameObjectBase* childGameObjectPtr = childGameObject.get();

	childGameObject->m_parent = this;
	childGameObject->BaseDeserialize(jsonData);
	childGameObject->BaseInitialize();

	m_childrens.push_back(move(childGameObject));

	return childGameObjectPtr;
}

GameObjectBase* GameObjectBase::GetChildGameObject(const string& name)
{
	for (auto& child : m_childrens) if (child->m_name == name) return child.get();

	return nullptr;
}

GameObjectBase* GameObjectBase::GetGameObjectRecursive(const string& name)
{
	for (auto& child : m_childrens)
	{
		if (child->m_name == name) return child.get();
		GameObjectBase* found = child->GetGameObjectRecursive(name);
		if (found) return found;
	}
	return nullptr;
}

void GameObjectBase::BaseInitialize()
{
	m_type = GetTypeName(*this);
	if (m_name.empty()) m_name = m_type + "_" + to_string(m_id);

	#ifdef NDEBUG
	Initialize();
	#endif

	// 컴포넌트와 자식 오브젝트 초기화
	for (auto& [typeIndex, component] : m_components) static_cast<Base*>(component.get())->BaseInitialize();
	for (auto& child : m_childrens) child->BaseInitialize();
}

void GameObjectBase::BaseFixedUpdate()
{
	#ifdef NDEBUG
	FixedUpdate();
	#endif

	// 월드 행렬 업데이트
	UpdateWorldMatrix();

	// 제거할 컴포넌트 및 자식 게임 오브젝트 제거
	RemovePending();

	// 컴포넌트 고정 업데이트
	for (Base*& component : m_fixedUpdateComponents) component->BaseFixedUpdate();

	// 자식 게임 오브젝트 고정 업데이트
	for (auto& child : m_childrens) child->BaseFixedUpdate();
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
	for (Base*& component : m_renderComponents) component->BaseRender();

	// 자식 게임 오브젝트 렌더링
	for (auto& child : m_childrens) child->BaseRender();
}

#ifdef _DEBUG
void GameObjectBase::BaseRenderImGui()
{
	ImGui::PushID(this);

	if (ImGui::Button("Remove")) SetAlive(false);
	ImGui::SameLine();

	if (ImGui::Button("Save As Prefab")) SaveAsPrefab();
	ImGui::SameLine();

	static array<char, 256> nameBuffer = {};
	strcpy_s(nameBuffer.data(), nameBuffer.size(), m_name.c_str());
	if (ImGui::InputText("", nameBuffer.data(), sizeof(nameBuffer))) m_name = nameBuffer.data();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (s_selectedObject == this) flags |= ImGuiTreeNodeFlags_Selected;

	bool isOpen = ImGui::TreeNodeEx(m_name.c_str(), flags);
	if (ImGui::IsItemClicked()) SetSelectedObject(this);

	if (isOpen)
	{
		// 위치
		if (ImGui::DragFloat3("Position", &m_position.m128_f32[0], 0.05f)) SetDirty();
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
		ImGui::Separator();
		if (!m_components.empty())
		{
			ImGui::Text("Components:");
			for (auto& [typeIndex, component] : m_components) component->BaseRenderImGui();
		}
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
		ImGui::Separator();
		if (!m_childrens.empty())
		{
			ImGui::Text("Children:");
			for (unique_ptr<GameObjectBase>& child : m_childrens) child->BaseRenderImGui();
		}
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
		ImGui::SameLine();

		// prefab으로 자식 게임 오브젝트 생성
		if (ImGui::Button("Add From Prefab")) ImGui::OpenPopup("Select Prefab");
		if (ImGui::BeginPopup("Select Prefab"))
		{
			const filesystem::path prefabDirectory = "../Asset/Prefab/";
			for (const auto& entry : filesystem::directory_iterator(prefabDirectory))
			{
				if (entry.path().extension() == ".json")
				{
					string prefabName = entry.path().stem().string();
					if (ImGui::Selectable(prefabName.c_str()))
					{
						CreatePrefabChildGameObject(prefabName + ".json");
						ImGui::CloseCurrentPopup();
					}
				}
			}
			ImGui::EndPopup();
		}

		ImGui::TreePop();
	}

	ImGui::PopID();
}
#endif

void GameObjectBase::BaseFinalize()
{
	if (s_selectedObject == this) s_selectedObject = nullptr;

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
	if (jsonData.contains("name")) m_name = jsonData["name"].get<string>();

	if (jsonData.contains("position"))
	{
		m_position = XMVectorSet
		(
			jsonData["position"][0].get<float>(),
			jsonData["position"][1].get<float>(),
			jsonData["position"][2].get<float>(),
			jsonData["position"][3].get<float>()
		);
	}
	if (jsonData.contains("rotation"))
	{
		m_quaternion = XMVectorSet
		(
			jsonData["rotation"][0].get<float>(),
			jsonData["rotation"][1].get<float>(),
			jsonData["rotation"][2].get<float>(),
			jsonData["rotation"][3].get<float>()
		);
		m_euler = ToDegrees(static_cast<XMVECTOR>(static_cast<SimpleMath::Quaternion>(m_quaternion).ToEuler()));
	}
	if (jsonData.contains("scale"))
	{
		m_scale = XMVectorSet
		(
			jsonData["scale"][0].get<float>(),
			jsonData["scale"][1].get<float>(),
			jsonData["scale"][2].get<float>(),
			jsonData["scale"][3].get<float>()
		);
	}

	// 파생 클래스의 데이터 로드
	Deserialize(jsonData);

	// 컴포넌트들 로드
	for (const auto& componentData : jsonData["components"])
	{
		string typeName = componentData["type"].get<string>();
		unique_ptr<ComponentBase> component = TypeRegistry::GetInstance().CreateComponent(typeName);

		if (m_components[type_index(typeid(*component))]) LOG_ERROR("오류: 게임 오브젝트 '" << m_name << "'에 이미 컴포넌트 '" << typeName << "'가 존재합니다.");
		component->SetOwner(this);
		if (component->NeedsFixedUpdate()) m_fixedUpdateComponents.push_back(component.get());
		if (component->NeedsUpdate()) m_updateComponents.push_back(component.get());
		if (component->NeedsRender()) m_renderComponents.push_back(component.get());

		Base* basePtr = static_cast<Base*>(component.get());
		basePtr->BaseDeserialize(componentData);
		m_components[type_index(typeid(*component))] = move(component);
	}
	
	// 자식 게임 오브젝트들 로드
	for (const auto& childData : jsonData["childGameObjects"])
	{
		string typeName = childData["type"].get<string>();
		unique_ptr<GameObjectBase> childGameObject = TypeRegistry::GetInstance().CreateGameObject(typeName);

		childGameObject->m_parent = this;
		childGameObject->BaseDeserialize(childData);
		m_childrens.push_back(move(childGameObject));
	}

	SetDirty();
}

void GameObjectBase::SaveAsPrefab()
{
	LOG("게임 오브젝트 '" << m_name << " 저장 중...");
	const filesystem::path prefabFilePath = "../Asset/Prefab/" + m_name + ".json";

	ofstream prefabFile(prefabFilePath);
	prefabFile << BaseSerialize().dump(4);
	prefabFile.close();

	SceneManager::GetInstance().LoadAllPrefabs();

	LOG("게임 오브젝트 '" << m_name << " 저장 완료!");
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
				if (compBasePtr->NeedsFixedUpdate()) erase_if(m_fixedUpdateComponents, [&](Base* fixedUpdateComponent) { return fixedUpdateComponent == compBasePtr; });
				if (compBasePtr->NeedsUpdate()) erase_if(m_updateComponents, [&](Base* updateComponent) { return updateComponent == compBasePtr; });
				if (compBasePtr->NeedsRender()) erase_if(m_renderComponents, [&](Base* renderComponent) { return renderComponent == compBasePtr; });

				return true;
			}
			return false;
		}
	);
}

void GameObjectBase::ApplyWorldMatrix(const XMMATRIX& worldMatrix)
{
	XMMATRIX localMatrix = worldMatrix;
	if (m_parent)
	{
		const XMMATRIX parentWorld = m_parent->UpdateWorldMatrix();
		const XMMATRIX invParent = XMMatrixInverse(nullptr, parentWorld);
		localMatrix = worldMatrix * invParent;
	}

	XMVECTOR scale = XMVectorZero();
	XMVECTOR rotationQuat = XMQuaternionIdentity();
	XMVECTOR translation = XMVectorZero();
	if (XMMatrixDecompose(&scale, &rotationQuat, &translation, localMatrix))
	{
		m_scale = scale;
		m_quaternion = rotationQuat;
		m_euler = ToDegrees(static_cast<XMVECTOR>(static_cast<SimpleMath::Quaternion>(rotationQuat).ToEuler()));
		m_position = translation;
		SetDirty();
	}
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

		if (m_parent && !m_isIgnoreParentTransform)
		{
			m_worldMatrix *= m_parent->UpdateWorldMatrix();
			m_inverseScaleSquareMatrix *= m_parent->m_inverseScaleSquareMatrix;
		}

		m_isDirty = false;

		m_worldData.worldMatrix = XMMatrixTranspose(m_worldMatrix);
		m_worldData.normalMatrix = XMMatrixTranspose(m_inverseScaleSquareMatrix * m_worldMatrix);
	}

	return m_worldMatrix;
}
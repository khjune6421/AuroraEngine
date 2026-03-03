///bof SceneBase.cpp
#include "stdafx.h"
#include "SceneBase.h"

#include "Renderer.h"
#include "CameraComponent.h"
#include "ResourceManager.h"
#include "TimeManager.h"
#include "NavigationManager.h"
#include "WindowManager.h"
#include "ModelComponent.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "ColliderComponent.h"

#include "Button.h"
#include "Slider.h"
#include "Panel.h"
#include "AnimPanel.h"
#include "Text.h"

using namespace std;
using namespace DirectX;

PostProcessingBuffer SceneBase::m_postProcessingData = {};

GameObjectBase* SceneBase::CreateRootGameObject(const string& typeName)
{
	unique_ptr<GameObjectBase> gameObject = TypeRegistry::GetInstance().CreateGameObject(typeName);
	GameObjectBase* gameObjectPtr = gameObject.get();

	static_cast<Base*>(gameObjectPtr)->BaseInitialize();
	m_gameObjects.push_back(move(gameObject));

	return gameObjectPtr;
}

GameObjectBase* SceneBase::CreatePrefabRootGameObject(const string& prefabFileName)
{
	return CreateFromJson(*SceneManager::GetInstance().GetPrefabData(prefabFileName));
}

GameObjectBase* SceneBase::CreateFromJson(const nlohmann::json& jsonData)
{
	unique_ptr<GameObjectBase> gameObject = TypeRegistry::GetInstance().CreateGameObject(jsonData["type"].get<string>());
	GameObjectBase* gameObjectPtr = gameObject.get();

	static_cast<Base*>(gameObjectPtr)->BaseDeserialize(jsonData);
	static_cast<Base*>(gameObjectPtr)->BaseInitialize();

	m_gameObjects.push_back(move(gameObject));

	return gameObjectPtr;
}

GameObjectBase* SceneBase::GetRootGameObject(const string& name)
{
	for (unique_ptr<Base>& gameObject : m_gameObjects)
	{
		GameObjectBase* gameObjectBase = static_cast<GameObjectBase*>(gameObject.get());
		if (gameObjectBase && gameObjectBase->GetName() == name) return gameObjectBase;
	}

	return nullptr;
}

GameObjectBase* SceneBase::GetGameObjectRecursive(const string& name)
{
	for (unique_ptr<Base>& gameObject : m_gameObjects)
	{
		GameObjectBase* gameObjectBase = static_cast<GameObjectBase*>(gameObject.get());
		if (gameObjectBase)
		{
			if (gameObjectBase->GetName() == name) return gameObjectBase;
			GameObjectBase* found = gameObjectBase->GetGameObjectRecursive(name);
			if (found) return found;
		}
	}
	return nullptr;
}

#ifdef _DEBUG
void SceneBase::SaveState()
{
	nlohmann::json sceneData = BaseSerialize();

	if (m_lastSavedSnapshot.is_null() || m_lastSavedSnapshot.empty()) { m_lastSavedSnapshot = sceneData; return; }

	const nlohmann::json forwardDiff = nlohmann::json::diff(m_lastSavedSnapshot, sceneData);
	if (forwardDiff.empty()) return;

	const nlohmann::json inverseDiff = nlohmann::json::diff(sceneData, m_lastSavedSnapshot);
	m_previousStateInversePatches.push_back(inverseDiff);

	if (m_previousStateInversePatches.size() > 1000) m_previousStateInversePatches.pop_front();

	m_lastSavedSnapshot = sceneData;
}

void SceneBase::Undo()
{
	if (m_previousStateInversePatches.empty()) return;

	nlohmann::json inversePatch = move(m_previousStateInversePatches.back());
	m_previousStateInversePatches.pop_back();

	nlohmann::json previousScene = BaseSerialize().patch(inversePatch);

	BaseDeserialize(previousScene);
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseInitialize();

	m_lastSavedSnapshot = previousScene;
}
#endif
void SceneBase::OnResizeEvent(std::pair<float,float> res)
{
	for (auto& m : m_UIList)
	{
		m->OnResize(res);
	}
}

void SceneBase::BaseInitialize()
{
	m_deviceContext = Renderer::GetInstance().GetDeviceContext();

	m_type = GetTypeName(*this);

	#ifdef _DEBUG
	m_debugCamera = make_unique<DebugCamera>();
	static_cast<Base*>(m_debugCamera.get())->BaseInitialize();
	m_debugCamera->Initialize();
	#endif

	const filesystem::path sceneFilePath = "../Asset/Scene/" + m_type + ".json";
	if (filesystem::exists(sceneFilePath))
	{
		ifstream file(sceneFilePath);
		nlohmann::json sceneData = {};
		file >> sceneData;
		file.close();
		BaseDeserialize(sceneData);
	}
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseInitialize();

	GetResources();

	#ifdef _DEBUG
	SaveState();
	#else
	Initialize();
	#endif

}

void SceneBase::BaseFixedUpdate()
{
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseFixedUpdate();
}

void SceneBase::BaseUpdate()
{
	if (SceneManager::GetInstance().IsPaused())
	{
		InputManager& inputManager = InputManager::GetInstance();

		const POINT& mousePosition = inputManager.GetMousePosition();
		const bool isMousePressed = inputManager.GetKey(KeyCode::MouseLeft);
		const bool isMouseClicked = inputManager.GetKeyUp(KeyCode::MouseLeft);
		for (auto it = m_UIList.rbegin(); it != m_UIList.rend(); ++it)
		{
			if (auto* button = dynamic_cast<Button*>(it->get()))
			{
				if (button->CheckInput(mousePosition, isMousePressed, isMouseClicked))
					break;
			}

			if (auto* slider = dynamic_cast<Slider*>(it->get()))
			{
				if (slider->CheckInput(mousePosition, isMousePressed))
					break;
			}
		}
		return;
	}

	#ifdef _DEBUG
	m_debugCamera->Update();
	static_cast<Base*>(m_debugCamera.get())->BaseUpdate();
	#else
	Update();
	#endif

	RemovePending();
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseUpdate();
	
	InputManager& inputManager = InputManager::GetInstance();

	const POINT& mousePosition = inputManager.GetMousePosition();
	const bool isMousePressed = inputManager.GetKey(KeyCode::MouseLeft);
	const bool isMouseClicked = inputManager.GetKeyUp(KeyCode::MouseLeft);
	for (auto it = m_UIList.rbegin(); it != m_UIList.rend(); ++it)
	{
		if (auto* button = dynamic_cast<Button*>(it->get()))
		{
			if (button->CheckInput(mousePosition, isMousePressed, isMouseClicked))
				break;
		}

		if (auto* slider = dynamic_cast<Slider*>(it->get()))
		{
			if (slider->CheckInput(mousePosition, isMousePressed))
				break;
		}
	}
	//for (auto& ui : m_UIList){
	//	ui->Update();
	//}



	#ifdef _DEBUG
	if (inputManager.GetKeyUp(KeyCode::MouseLeft)) SaveState();

	if (m_isNavMeshCreating) NavigationManager::GetInstance().HandlePlaceLink(m_navMeshCreationHeight);

	if (inputManager.GetKey(KeyCode::Control))
	{
		if (inputManager.GetKeyDown(KeyCode::S))
		{
			LOG("[System] Saving Scene: " << m_type << "...");
			const filesystem::path sceneFilePath = "../Asset/Scene/" + m_type + ".json";

			ofstream sceneFile(sceneFilePath);
			sceneFile << BaseSerialize().dump(4);
			sceneFile.close();

			LOG("[System] Scene: " << m_type << " saved successfully!");
		}

		if (inputManager.GetKeyDown(KeyCode::Z)) Undo();
		if (inputManager.GetKeyDown(KeyCode::MouseLeft)) PickObjectDebugCamera();
	}
	#endif
}

void SceneBase::BaseRender()
{
	Renderer& renderer = Renderer::GetInstance();

	renderer.RENDER_FUNCTION(RenderStage::DirectionalLightShadow, BlendState::Opaque).emplace_back
	(
		numeric_limits<float>::lowest(),
		[&]()
		{
			Renderer& renderer = Renderer::GetInstance();
			const CameraComponent& mainCamera = CameraComponent::GetMainCamera();

			renderer.SetViewport(static_cast<FLOAT>(DIRECTIAL_LIGHT_SHADOW_MAP_SIZE), static_cast<FLOAT>(DIRECTIAL_LIGHT_SHADOW_MAP_SIZE));

			const float cameraFarPlane = mainCamera.GetFarZ();

			XMVECTOR lightPosition = m_globalLightData.lightDirection * -250.0f;
			lightPosition = XMVectorSetW(lightPosition, 1.0f);
			renderer.SetRenderSortPoint(lightPosition);

			constexpr XMVECTOR LIGHT_UP = { 0.0f, 1.0f, 0.0f, 0.0f };
			constexpr XMVECTOR LIGHT_TARGET_OFFSET = { 0.0f, 0.0f, 0.0f, 0.0f };
			m_viewProjectionData.viewMatrix = XMMatrixLookAtLH(lightPosition, LIGHT_TARGET_OFFSET, LIGHT_UP);

			const float lightRange = 500.0f; // TODO: 나중에 조금 줄일수도 있음
			m_viewProjectionData.projectionMatrix = XMMatrixOrthographicLH(lightRange, lightRange, 0.1f, lightRange);

			m_viewProjectionData.VPMatrix = XMMatrixTranspose(m_viewProjectionData.viewMatrix * m_viewProjectionData.projectionMatrix);
			m_globalLightData.lightViewProjectionMatrix = m_viewProjectionData.VPMatrix;

			m_deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);

			m_deviceContext->PSSetShader(m_shadowMapPixelShader.Get(), nullptr, 0);
		}
	);

	renderer.RENDER_FUNCTION(RenderStage::Scene, BlendState::Opaque).emplace_back
	(
		numeric_limits<float>::lowest(), 
		[&]()
		{
			Renderer& renderer = Renderer::GetInstance();
			const CameraComponent& mainCamera = CameraComponent::GetMainCamera();

			renderer.SetViewport();

			renderer.SetRenderSortPoint(mainCamera.GetPosition());

			UpdateConstantBuffers();

			m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Skybox), 1, m_skyboxSRV.GetAddressOf());
			m_deviceContext->PSSetShaderResources(static_cast<UINT>(TextureSlots::Environment), 1, m_environmentMapSRV.GetAddressOf());
			RenderSkybox();

			#ifdef _DEBUG
			if (m_isRenderDebugCoordinates) RenderDebugCoordinates();
			#endif
			Render();
		}
	);

	if (m_showFPS)
	{
		renderer.UI_RENDER_FUNCTIONS().emplace_back
		(
			[&]()
			{
				static UINT frameCount = 0;
				static float elapsedTime = 0.0f;
				static UINT FPS = 0;

				frameCount++;

				elapsedTime += TimeManager::GetInstance().GetDeltaTime();

				if (elapsedTime >= 1.0)
				{
					FPS = frameCount * static_cast<UINT>(elapsedTime);
					frameCount = 0;
					elapsedTime = 0.0;
				}

				Renderer::GetInstance().RenderTextScreenPosition((L"FPS: " + to_wstring(FPS)).c_str(), XMFLOAT2(10.0f, 10.0f));
			}
		);
	}

	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseRender();

	for (const unique_ptr<UIBase>& ui : m_UIList) ui->RenderUI(renderer);

	#ifdef _DEBUG
	if (m_isNavMeshCreating) NavigationManager::GetInstance().RenderNavMesh();
	#endif
}

#ifdef _DEBUG
void SceneBase::BaseRenderImGui()
{
	ImGui::Begin("Debug Camera");
	static_cast<Base*>(m_debugCamera.get())->BaseRenderImGui();
	ImGui::End();

	ImGui::Begin(m_type.c_str());

	ImGui::Checkbox("Debug Coordinates", &m_isRenderDebugCoordinates);

	ImGui::Checkbox("NavMesh Creating", &m_isNavMeshCreating);
	ImGui::DragFloat("NavMesh Creation Height", &m_navMeshCreationHeight, 0.1f, 0.1f, 100.0f);

	ImGui::Separator();
	array<char, 256> skyboxFileNameBuffer = {};
	strcpy_s(skyboxFileNameBuffer.data(), skyboxFileNameBuffer.size(), m_skyboxFileName.c_str());
	if (ImGui::InputText("Skybox File Name", skyboxFileNameBuffer.data(), sizeof(skyboxFileNameBuffer))) m_skyboxFileName = skyboxFileNameBuffer.data();
	if (ImGui::Button("Load Skybox")) m_skyboxSRV = ResourceManager::GetInstance().GetTexture(m_skyboxFileName);

	array<char, 256> environmentMapFileNameBuffer = {};
	strcpy_s(environmentMapFileNameBuffer.data(), environmentMapFileNameBuffer.size(), m_environmentMapFileName.c_str());
	if (ImGui::InputText("Environment Map File Name", environmentMapFileNameBuffer.data(), sizeof(environmentMapFileNameBuffer))) m_environmentMapFileName = environmentMapFileNameBuffer.data();
	if (ImGui::Button("Load Environment Map")) m_environmentMapSRV = ResourceManager::GetInstance().GetTexture(m_environmentMapFileName);

	ImGui::Separator();
	ImGui::Text("[Post Processing]");
	
	ImGui::Separator();

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Bloom");
	ImGui::CheckboxFlags("Bloom", &m_postProcessingData.flags, static_cast<UINT>(PostProcessingBuffer::PostProcessingFlag::Bloom));
	ImGui::DragFloat("Bloom Intensity", &m_postProcessingData.bloomIntensity, 0.1f, 0.0f, 100.0f);

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Gamma");
	ImGui::CheckboxFlags("Gamma", &m_postProcessingData.flags, static_cast<UINT>(PostProcessingBuffer::PostProcessingFlag::Gamma));
	ImGui::DragFloat("Gamma Intensity", &m_postProcessingData.gammaIntensity, 0.01f, 0.1f, 5.0f);

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Grayscale");
	ImGui::CheckboxFlags("Grayscale", &m_postProcessingData.flags, static_cast<UINT>(PostProcessingBuffer::PostProcessingFlag::Grayscale));
	ImGui::DragFloat("Grayscale Intensity", &m_postProcessingData.grayScaleIntensity, 0.01f, 0.0f, 1.0f);

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Vignetting");
	ImGui::CheckboxFlags("Vignetting", &m_postProcessingData.flags, static_cast<UINT>(PostProcessingBuffer::PostProcessingFlag::Vignetting));
	ImGui::ColorEdit4("Vignetting Color And Intensity", &m_postProcessingData.vignettingColor.x);
	
	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Radial Blur");
	ImGui::CheckboxFlags("Radial Blur", &m_postProcessingData.flags, static_cast<UINT>(PostProcessingBuffer::PostProcessingFlag::RadialBlur));
	ImGui::DragFloat2("Radial Blur Center", &m_postProcessingData.radialBlurParam.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Radial Blur Dist", &m_postProcessingData.radialBlurParam.z, 0.01f, 0.0f, 10.0f);
	ImGui::DragFloat("Radial Blur Strength", &m_postProcessingData.radialBlurParam.w, 0.01f, 0.0f, 100.0f);

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "LUT");
	const char* lutItems[] = {
	#define X(name) #name,
	LUT_LIST
	#undef X
	};
	int& lut1idx = Renderer::GetInstance().GetSelectedLUTIndex();
	ImGui::Combo("Select LUT", &lut1idx, lutItems, LUTData::COUNT);
	com_ptr<ID3D11ShaderResourceView> lut_texture_1 = nullptr;
	lut_texture_1 = ResourceManager::GetInstance().GetLUT(lut1idx);
	ImGui::Image
	(
		(ImTextureID)lut_texture_1.Get(),
		ImVec2(256, 16)
	);
	int& lut2idx = Renderer::GetInstance().GetSelectedLUT2Index();
	ImGui::CheckboxFlags("LUT CROSSFADE", &m_postProcessingData.flags, static_cast<UINT>(PostProcessingBuffer::PostProcessingFlag::LUT_CROSSFADE));
	ImGui::Combo("Select LUT2", &lut2idx, lutItems, LUTData::COUNT);
	com_ptr<ID3D11ShaderResourceView> lut_texture_2 = nullptr;
	lut_texture_2 = ResourceManager::GetInstance().GetLUT(lut2idx);
	ImGui::Image
	(
		(ImTextureID)lut_texture_2.Get(),
		ImVec2(256, 16)
	);
	ImGui::DragFloat("Lut Lerp Factor", &m_postProcessingData.lutLerpFactor, 0.01f, 0.0f, 1.0f);

	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Light");
	ImGui::DragFloat("IBL Intensity", &m_globalLightData.lightColor.w, 0.001f, 0.0f, 1.0f);
	ImGui::ColorEdit3("Light Color", &m_globalLightData.lightColor.x);

	if (ImGui::DragFloat3("Light Direction", &m_globalLightData.lightDirection.m128_f32[0], 0.001f, -1.0f, 1.0f))
	{
		m_globalLightData.lightDirection = XMVectorSetW(XMVector3Normalize(m_globalLightData.lightDirection), m_globalLightData.lightDirection.m128_f32[3]);
	}
	ImGui::DragFloat("Directional Intensity", &m_globalLightData.lightDirection.m128_f32[3], 0.001f, 0.0f, 100.0f);

	RenderImGui();

	if (ImGui::Button("Bake Colloder")) BakeCollider();

	ImGui::Separator();
	ImGui::Text("Game Objects:");
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseRenderImGui();

	ImGui::Separator();
	if (ImGui::Button("Add GameObject")) ImGui::OpenPopup("Select GameObject Type");
	if (ImGui::BeginPopup("Select GameObject Type"))
	{
		for (const auto& [typeName, factory] : TypeRegistry::GetInstance().m_gameObjectRegistry)
		{
			if (ImGui::Selectable(typeName.c_str()))
			{
				CreateRootGameObject(typeName);
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}
	if (ImGui::Button("Add Prefab")) ImGui::OpenPopup("Select Prefab");
	if (ImGui::BeginPopup("Select Prefab"))
	{
		const filesystem::path prefabDirectory = "../Asset/Prefab/";
		for (const auto& entry : filesystem::directory_iterator(prefabDirectory))
		{
			if (entry.path().extension() == ".json")
			{
				const string prefabFileName = entry.path().stem().string();
				if (ImGui::Selectable(prefabFileName.c_str()))
				{
					CreatePrefabRootGameObject(prefabFileName + ".json");
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::EndPopup();
	}

	ImGui::End();

	GameObjectBase* selectedObject = GameObjectBase::GetSelectedObject();
	if (selectedObject)
	{
		static ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
		static ImGuizmo::MODE gizmoMode = ImGuizmo::LOCAL;
		static bool useSnap = false;
		array<float, 3> snapTranslation = { 0.5f, 0.5f, 0.5f };
		static float snapRotation = 15.0f;
		static float snapScale = 0.1f;

		ImGui::Begin("Gizmo");
		ImGui::Text("Selected: %s", selectedObject->GetName().c_str());
		if (ImGui::RadioButton("Translate", gizmoOperation == ImGuizmo::TRANSLATE)) gizmoOperation = ImGuizmo::TRANSLATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Rotate", gizmoOperation == ImGuizmo::ROTATE)) gizmoOperation = ImGuizmo::ROTATE;
		ImGui::SameLine();
		if (ImGui::RadioButton("Scale", gizmoOperation == ImGuizmo::SCALE)) gizmoOperation = ImGuizmo::SCALE;

		if (gizmoOperation != ImGuizmo::SCALE)
		{
			if (ImGui::RadioButton("Local", gizmoMode == ImGuizmo::LOCAL)) gizmoMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", gizmoMode == ImGuizmo::WORLD)) gizmoMode = ImGuizmo::WORLD;
		}

		ImGui::Checkbox("Snap", &useSnap);
		if (useSnap)
		{
			if (gizmoOperation == ImGuizmo::TRANSLATE) ImGui::InputFloat3("Snap T", snapTranslation.data());
			else if (gizmoOperation == ImGuizmo::ROTATE) ImGui::InputFloat("Snap R", &snapRotation);
			else if (gizmoOperation == ImGuizmo::SCALE) ImGui::InputFloat("Snap S", &snapScale);
		}
		ImGui::End();

		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

		RECT rect = WindowManager::GetInstance().GetClientPosRect();
		ImGuizmo::SetRect(static_cast<float>(rect.left), static_cast<float>(rect.top), static_cast<float>(rect.right), static_cast<float>(rect.bottom));
		
		ImGuizmo::SetOrthographic(false);

		const float* snap = nullptr;
		array<float, 3> snapValues = { 0.0f, 0.0f, 0.0f };
		if (useSnap)
		{
			if (gizmoOperation == ImGuizmo::TRANSLATE)
			{
				memcpy(snapValues.data(), snapTranslation.data(), sizeof(float) * 3);
				snap = snapValues.data();
			}
			else if (gizmoOperation == ImGuizmo::ROTATE)
			{
				snapValues[0] = snapRotation;
				snap = snapValues.data();
			}
			else if (gizmoOperation == ImGuizmo::SCALE)
			{
				snapValues[0] = snapScale;
				snap = snapValues.data();
			}
		}

		const CameraComponent& mainCamera = CameraComponent::GetMainCamera();
		XMMATRIX worldMatrix = selectedObject->GetWorldMatrix();
		ImGuizmo::Manipulate
		(
			&mainCamera.GetViewMatrix().r[0].m128_f32[0],
			&mainCamera.GetProjectionMatrix().r[0].m128_f32[0],
			gizmoOperation,
			gizmoMode,
			&worldMatrix.r[0].m128_f32[0],
			nullptr,
			snap
		);

		if (ImGuizmo::IsUsing()) selectedObject->ApplyWorldMatrix(worldMatrix);
	}

	RenderImGui_UI();
}

void SceneBase::RenderImGui_UI()
{
	ImGui::Begin("UI Editor");

	if (ImGui::Button("Add Panel"))  CreateUI<Panel>()->SetName("New Panel");
	ImGui::SameLine();
	if (ImGui::Button("Add AnimPanel")) CreateUI<AnimPanel>()->SetName("New AnimPanel");
	ImGui::SameLine();
	if (ImGui::Button("Add Button")) CreateUI<Button>()->SetName("New Button");
	ImGui::SameLine();
	if (ImGui::Button("Add Slider")) CreateUI<Slider>()->SetName("New Slider");
	ImGui::SameLine();
	if (ImGui::Button("Add Text")) CreateUI<Text>()->SetName("New Text");

	ImGui::Separator();

	ImGui::Columns(2, "UIEditorColumns", true); 

	ImGui::BeginChild("UI_Hierarchy", ImVec2(0, 0), true);
	ImGui::Text("Hierarchy");
	ImGui::Separator();

	for (const auto& uiPtr : m_UIList) {
		UIBase* ui = uiPtr.get();
		if (ui && ui->GetParent() == nullptr) {
			RenderUITreeNode(ui);
		}
	}

	if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered()) {
		m_selectedUI = nullptr;
	}

	ImVec2 availSize = ImGui::GetContentRegionAvail();
	if (availSize.y < 50.0f) availSize.y = 50.0f; 

	ImGui::Dummy(availSize); 

	if (ImGui::BeginDragDropTarget()) 
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UI_DRAG_DROP")) {
			UIBase* droppedUI = *(UIBase**)payload->Data;
			if (droppedUI) {
				droppedUI->SetParent(nullptr); 
			}
		}
		ImGui::EndDragDropTarget();
	}

	ImGui::EndChild();

	ImGui::NextColumn();

	ImGui::BeginChild("UI_Inspector", ImVec2(0, 0), true);

	if (m_selectedUI) {
		ImGui::TextColored(ImVec4(0, 1, 0, 1), "Inspector: %s", m_selectedUI->GetName().c_str());

		static char nameBuf[128];
		strcpy_s(nameBuf, m_selectedUI->GetName().c_str());
		if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
			m_selectedUI->SetName(nameBuf);
		}

		UIBase* parent = m_selectedUI->GetParent();
		string parentName = parent ? parent->GetName() : "None (Root)";
		ImGui::Text("Parent: %s", parentName.c_str());

		if (parent && ImGui::Button("Detach Parent")) {
			m_selectedUI->SetParent(nullptr);
		}

		bool active = m_selectedUI->GetActive();
		if (ImGui::Checkbox("Active", &active)) m_selectedUI->SetActive(active);

		DirectX::XMFLOAT2 pos = m_selectedUI->GetLocalPosition();
		if (ImGui::DragFloat2("Local Pos", &pos.x, 0.01f)) m_selectedUI->SetLocalPosition(pos);

		float scale = m_selectedUI->GetScale();
		if (ImGui::DragFloat("Scale", &scale, 0.01f, 0.0f, 10.0f)) m_selectedUI->SetScale(scale);
		
		float depth = m_selectedUI->GetDepth();
		if (ImGui::DragFloat("Depth", &depth, 0.01f, 0.0f, 1.0f)) m_selectedUI->SetDepth(depth);

			ImGui::Separator();
		}




		// 텍스처 경로 입력 람다
		auto TexPathInput = [&](const char* label, std::string current, std::string& out) {
			char buf[128]; strcpy_s(buf, current.c_str());
			//if (ImGui::InputText(label, buf, 128, ImGuiInputTextFlags_EnterReturnsTrue)) {
			if (ImGui::InputText(label, buf, 128)) {
				out = buf; return true;
			}
			return false;
			};


		if (auto* btn = dynamic_cast<Button*>(m_selectedUI)) {
			ImGui::TextColored(ImVec4(0.8f, 0.8f, 1.0f, 1.0f), "TYPE: BUTTON");

			// --- Action Key ---
			static char keyBuf[64];
			strcpy_s(keyBuf, btn->GetActionKey().c_str());
			if (ImGui::InputText("Action Key", keyBuf, sizeof(keyBuf))) {
				btn->SetActionKey(keyBuf);
			}
			ImGui::Separator();




			// --- [Idle State] ---
			if (ImGui::TreeNode("State: Idle")) {
				bool pathChanged = TexPathInput("Path##Idle", btn->m_pathIdle, btn->m_pathIdle);
				XMFLOAT4 c_i;
				XMStoreFloat4(&c_i, btn->m_colorIdle);
				ImGui::ColorEdit4("Color##Idle", &c_i.x);
				btn->m_colorIdle = XMLoadFloat4(&c_i);
				ImGui::DragFloat("Scale Multi##Idle", &btn->m_designScale, 0.01f, 0.1f, 3.0f);

				if (pathChanged) btn->SetButtonTextures(btn->m_pathIdle, btn->m_pathHover, btn->m_pathPressed, btn->m_pathClicked);
				ImGui::TreePop();
			}

			// --- [Hover State] ---
			if (ImGui::TreeNode("State: Hover")) {
				bool pathChanged = TexPathInput("Path##Hover", btn->m_pathHover, btn->m_pathHover);
				XMFLOAT4 c_h;
				XMStoreFloat4(&c_h, btn->m_colorHover);
				ImGui::ColorEdit4("Color##Hover", &c_h.x);
				btn->m_colorHover = XMLoadFloat4(&c_h);
				ImGui::DragFloat("Scale Multi##Hover", &btn->m_scaleHover, 0.01f, 0.1f, 3.0f);

				if (pathChanged) btn->SetButtonTextures(btn->m_pathIdle, btn->m_pathHover, btn->m_pathPressed, btn->m_pathClicked);
				ImGui::TreePop();
			}

			// --- [Pressed State] ---
			if (ImGui::TreeNode("State: Pressed")) {
				bool pathChanged = TexPathInput("Path##Pressed", btn->m_pathPressed, btn->m_pathPressed);
				XMFLOAT4 c_p;
				XMStoreFloat4(&c_p, btn->m_colorPressed);
				ImGui::ColorEdit4("Color##Pressed", &c_p.x);
				btn->m_colorPressed = XMLoadFloat4(&c_p);
				ImGui::DragFloat("Scale Multi##Pressed", &btn->m_scalePressed, 0.01f, 0.1f, 3.0f);

				if (pathChanged) btn->SetButtonTextures(btn->m_pathIdle, btn->m_pathHover, btn->m_pathPressed, btn->m_pathClicked);
				ImGui::TreePop();
			}

			// --- [Clicked State] ---
			if (ImGui::TreeNode("State: Clicked")) {
				bool pathChanged = TexPathInput("Path##Clicked", btn->m_pathClicked, btn->m_pathClicked);
				XMFLOAT4 c_c;
				XMStoreFloat4(&c_c, btn->m_colorClicked);
				ImGui::ColorEdit4("Color##Clicked", &c_c.x);
				btn->m_colorClicked = XMLoadFloat4(&c_c);
				ImGui::DragFloat("Scale Multi##Clicked", &btn->m_scaleClicked, 0.01f, 0.1f, 3.0f);

				if (pathChanged) btn->SetButtonTextures(btn->m_pathIdle, btn->m_pathHover, btn->m_pathPressed, btn->m_pathClicked);
				ImGui::TreePop();
			}

		}
		else if (auto* panel = dynamic_cast<Panel*>(m_selectedUI)) {
			ImGui::TextColored(ImVec4(0.6f, 1.0f, 1.0f, 1.0f), "TYPE: PANEL");
			ImGui::Separator();

			bool pathChanged = TexPathInput("Path", panel->m_pathIdle, panel->m_pathIdle);

			XMFLOAT4 c;
			XMStoreFloat4(&c, panel->m_colorIdle);
			ImGui::ColorEdit4("Color", &c.x);
			panel->m_colorIdle = XMLoadFloat4(&c);

			ImGui::DragFloat("Scale Multi", &panel->m_designScale, 0.01f, 0.1f, 3.0f);

			if (pathChanged) panel->SetTextureAndOffset(panel->m_pathIdle);


		}

		else if (auto* text = dynamic_cast<Text*>(m_selectedUI)) {
			ImGui::TextColored(ImVec4(0.6f, 1.0f, 1.0f, 1.0f), "TYPE: TEXT");
			ImGui::Separator();

			static char textBuf[256];
			strcpy_s(textBuf, text->GetText().c_str());
			if (ImGui::InputText("Text", textBuf, sizeof(textBuf))) {
				text->SetText(textBuf);
			}

			static char fontBuf[128];
			strcpy_s(fontBuf, text->GetFontName().c_str());
			if (ImGui::InputText("Font", fontBuf, sizeof(fontBuf))) {
				text->SetFontName(fontBuf);
			}

			XMFLOAT4 color;
			XMStoreFloat4(&color, text->GetColorIdle());
			if (ImGui::ColorEdit4("Color", &color.x)) {
				text->SetColorIdle(XMLoadFloat4(&color));
			}
		}
		else if (auto* slider = dynamic_cast<Slider*>(m_selectedUI)) {
			ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "TYPE: SLIDER");
			ImGui::Separator();

			static char keyBuf[64];
			strcpy_s(keyBuf, slider->GetActionKey().c_str());
			if (ImGui::InputText("Action Key", keyBuf, sizeof(keyBuf))) {
				slider->SetActionKey(keyBuf);
			}

			float min = slider->GetMin();
			float max = slider->GetMax();
			float val = slider->GetValue();

			bool changed = false;
			changed |= ImGui::DragFloat("Min", &min, 0.1f);
			changed |= ImGui::DragFloat("Max", &max, 0.1f);
			changed |= ImGui::DragFloat("Value", &val, 0.1f, min, max);

			if (changed) {
				slider->SetRange(min, max);
				slider->SetValue(val);
			}

			ImGui::Separator();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[ Handle Settings ]");

			if (ImGui::TreeNode("Handle: Idle")) {
				bool pathChanged = TexPathInput("Path##H_Idle", slider->m_handlePathIdle, slider->m_handlePathIdle);

				DirectX::XMFLOAT4 c;
				DirectX::XMStoreFloat4(&c, slider->m_handleColorIdle);
				if (ImGui::ColorEdit4("Color##H_Idle", &c.x)) slider->m_handleColorIdle = DirectX::XMLoadFloat4(&c);

				ImGui::DragFloat("Scale##H_Idle", &slider->m_handleScaleIdle, 0.01f, 0.1f, 3.0f);

				if (pathChanged) slider->SetHandleTextures(slider->m_handlePathIdle, slider->m_handlePathHover, slider->m_handlePathPressed);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Handle: Hover")) {
				bool pathChanged = TexPathInput("Path##H_Hover", slider->m_handlePathHover, slider->m_handlePathHover);

				DirectX::XMFLOAT4 c;
				DirectX::XMStoreFloat4(&c, slider->m_handleColorHover);
				if (ImGui::ColorEdit4("Color##H_Hover", &c.x)) slider->m_handleColorHover = DirectX::XMLoadFloat4(&c);

				ImGui::DragFloat("Scale##H_Hover", &slider->m_handleScaleHover, 0.01f, 0.1f, 3.0f);

				if (pathChanged) slider->SetHandleTextures(slider->m_handlePathIdle, slider->m_handlePathHover, slider->m_handlePathPressed);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Handle: Pressed")) {
				bool pathChanged = TexPathInput("Path##H_Pressed", slider->m_handlePathPressed, slider->m_handlePathPressed);

				DirectX::XMFLOAT4 c;
				DirectX::XMStoreFloat4(&c, slider->m_handleColorPressed);
				if (ImGui::ColorEdit4("Color##H_Pressed", &c.x)) slider->m_handleColorPressed = DirectX::XMLoadFloat4(&c);

				ImGui::DragFloat("Scale##H_Pressed", &slider->m_handleScalePressed, 0.01f, 0.1f, 3.0f);

				if (pathChanged) slider->SetHandleTextures(slider->m_handlePathIdle, slider->m_handlePathHover, slider->m_handlePathPressed);
				ImGui::TreePop();
			}
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "[ Value Text Settings ]");

			ImGui::Checkbox("Show Value Text", &slider->m_showValueText);

			// Font Scale
			ImGui::DragFloat("Text Scale", &slider->m_valueTextScale,
				0.01f, 0.05f, 3.0f);

			// Offset (X,Y)
			float offset[2] =
			{
				slider->m_valueTextOffset.x,
				slider->m_valueTextOffset.y
			};

			if (ImGui::DragFloat2("Text Offset", offset, 1.0f))
			{
				slider->m_valueTextOffset.x = offset[0];
				slider->m_valueTextOffset.y = offset[1];
			}

			// Text Color
			DirectX::XMFLOAT4 tc;
			DirectX::XMStoreFloat4(&tc, slider->m_valueTextColor);

			if (ImGui::ColorEdit4("Text Color", &tc.x))
			{
				slider->m_valueTextColor = DirectX::XMLoadFloat4(&tc);
			}

			ImGui::TextDisabled("Text follows handle automatically.");
		}
		else if (auto* animPanel = dynamic_cast<AnimPanel*>(m_selectedUI)) {
			ImGui::Separator();
			ImGui::TextColored(ImVec4(0.8f, 1.f, .8f, 1.0f), "Animation");

		bool changed = false;
		changed |= ImGui::DragInt("Rows", &animPanel->m_rows, 1, 1, 128);
		changed |= ImGui::DragInt("Columns", &animPanel->m_columns, 1, 1, 128);
		changed |= ImGui::DragInt("Start Frame", &animPanel->m_startFrame, 1, 0, 4095);
		changed |= ImGui::DragInt("End Frame", &animPanel->m_endFrame, 1, 1, 4096);
		changed |= ImGui::DragFloat("FPS", &animPanel->m_framesPerSecond, 0.1f, 0.0f, 120.0f);
		changed |= ImGui::DragFloat("Playback Speed", &animPanel->playback_speed_, 0.1f, 0.1f, 4.0f);
		ImGui::Checkbox("Loop", &animPanel->m_loop);
		ImGui::Checkbox("Auto Play", &animPanel->auto_play_);
		ImGui::Checkbox("Destroy On Finish", &animPanel->destroy_on_finish_);
		if (changed) animPanel->UpdateFlipBookRect();

		ImGui::Text("Current Frame: %d / %d", animPanel->m_currentFrame + 1, animPanel->m_endFrame);

		if (animPanel->m_textureIdle.first) {
			const ImVec2 previewSize(200.0f, 200.0f);
			const ImTextureID textureId = reinterpret_cast<ImTextureID>(animPanel->m_textureIdle.first.Get());
			ImGui::Image(textureId, previewSize);

			const ImVec2 p0 = ImGui::GetItemRectMin();
			const ImVec2 p1 = ImGui::GetItemRectMax();
			const ImVec2 imageSize(p1.x - p0.x, p1.y - p0.y);

			int texW = 1;
			int texH = 1;

			{
				com_ptr<ID3D11Resource> res;
				animPanel->GetTexture()->GetResource(res.GetAddressOf());
				com_ptr<ID3D11Texture2D> tex2D;
				if (res && SUCCEEDED(res.As(&tex2D))) {
					D3D11_TEXTURE2D_DESC desc;
					tex2D->GetDesc(&desc);
					texW = static_cast<int>(desc.Width);
					texH = static_cast<int>(desc.Height);
				}
			}

			const RECT& currentRect = animPanel->m_UIAnimationRect;

			float uvLeft = (float)currentRect.left / (float)texW;
			float uvTop = (float)currentRect.top / (float)texH;
			float uvRight = (float)currentRect.right / (float)texW;
			float uvBottom = (float)currentRect.bottom / (float)texH;

			const ImVec2 rectMin
			{
				p0.x + uvLeft * imageSize.x,
				p0.y + uvTop * imageSize.y
			};
			const ImVec2 rectMax
			{
				p0.x + uvRight * imageSize.x, // rectMin.x + width 대신 right 좌표 사용
				p0.y + uvBottom * imageSize.y
			};

			const ImU32 rectColor = IM_COL32(255, 230, 0, 255);
			ImGui::GetWindowDrawList()->AddRect(rectMin, rectMax, rectColor, 0.0f, 0, 2.0f);
		} else ImGui::Text("Texture preview: (none)");
		ImGui::Separator();


		}


 
	 else {
		ImGui::TextDisabled("Select UI to edit");
	}


	ImGui::EndChild();
	ImGui::Columns(1); // 컬럼 종료
	ImGui::End();
}

void SceneBase::RenderUITreeNode(UIBase* ui)
{
	if (!ui) return;

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
	if (m_selectedUI == ui) flags |= ImGuiTreeNodeFlags_Selected;

	bool hasChildren = false;
	for (const auto& childPtr : m_UIList) {
		if (childPtr && childPtr->GetParent() == ui) {
			hasChildren = true;
			break;
		}
	}
	if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf;

	bool nodeOpen = ImGui::TreeNodeEx((void*)ui, flags, "[%s] %s", ui->GetTypeName().c_str(), ui->GetName().c_str());

	if (ImGui::IsItemClicked()) m_selectedUI = ui;

	if (ImGui::BeginDragDropSource()) {
		ImGui::SetDragDropPayload("UI_DRAG_DROP", &ui, sizeof(UIBase*));
		ImGui::Text("Move %s", ui->GetName().c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget()) {
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("UI_DRAG_DROP")) {
			UIBase* droppedUI = *(UIBase**)payload->Data;
			if (droppedUI != ui) {
				droppedUI->SetParent(ui);
			}
		}
		ImGui::EndDragDropTarget();
	}

	if (ImGui::BeginPopupContextItem()) {
		if (ImGui::MenuItem("Delete")) {
			for (auto& child : m_UIList) {
				if (child && child->GetParent() == ui) child->SetParent(nullptr);
			}

			if (m_selectedUI == ui) m_selectedUI = nullptr;

			erase_if(m_UIList, [ui](const unique_ptr<UIBase>& ptr) { return ptr.get() == ui; });

			ImGui::EndPopup();
			if (nodeOpen) ImGui::TreePop();
			return;
		}
		ImGui::EndPopup();
	}

	if (nodeOpen) {
		for (const auto& childPtr : m_UIList) {
			if (childPtr && childPtr->GetParent() == ui) {
				RenderUITreeNode(childPtr.get());
			}
		}
		ImGui::TreePop();
	}
}


#endif

void SceneBase::BaseFinalize()
{
	#ifdef NDEBUG
	Finalize();
	#endif

	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObject->BaseFinalize();
}

nlohmann::json SceneBase::BaseSerialize()
{
	nlohmann::json jsonData;

	jsonData["lightColor"] =
	{
		m_globalLightData.lightColor.x,
		m_globalLightData.lightColor.y,
		m_globalLightData.lightColor.z,
		m_globalLightData.lightColor.w
	};
	jsonData["lightDirection"] =
	{
		m_globalLightData.lightDirection.m128_f32[0],
		m_globalLightData.lightDirection.m128_f32[1],
		m_globalLightData.lightDirection.m128_f32[2],
		m_globalLightData.lightDirection.m128_f32[3]
	};

	jsonData["postProcessing"] =
	{
		{ "flags", m_postProcessingData.flags },
		{ "bloomIntensity", m_postProcessingData.bloomIntensity },
		{ "gammaIntensity", m_postProcessingData.gammaIntensity },
		{ "grayScaleIntensity", m_postProcessingData.grayScaleIntensity },
		{ "vignettingColor", { m_postProcessingData.vignettingColor.x, m_postProcessingData.vignettingColor.y, m_postProcessingData.vignettingColor.z, m_postProcessingData.vignettingColor.w } },
		{ "radialBlurParam", { m_postProcessingData.radialBlurParam.x, m_postProcessingData.radialBlurParam.y, m_postProcessingData.radialBlurParam.z, m_postProcessingData.radialBlurParam.w } },
		{ "lutLerpFactor", m_postProcessingData.lutLerpFactor}
	};

	jsonData["skyboxFileName"] = m_skyboxFileName;
	jsonData["environmentMapFileName"] = m_environmentMapFileName;

	nlohmann::json navMeshData = NavigationManager::GetInstance().Serialize();
	if (!navMeshData.is_null() && navMeshData.is_object()) jsonData.merge_patch(navMeshData);

	nlohmann::json uiArray = nlohmann::json::array();
	for (size_t i = 0; i < m_UIList.size(); ++i) {
		if (!m_UIList[i]) continue;
		nlohmann::json uiData = m_UIList[i]->Serialize();
		int parentIdx = -1;
		UIBase* parent = m_UIList[i]->GetParent();
		if (parent) {
			for (size_t j = 0; j < m_UIList.size(); ++j) {
				if (m_UIList[j].get() == parent) {
					parentIdx = static_cast<int>(j);
					break;
				}
			}
		}
		uiData["parentIdx"] = parentIdx;
		uiArray.push_back(uiData);
	}

	jsonData["UI"] = uiArray;

	nlohmann::json derivedData = Serialize();
	if (!derivedData.is_null() && derivedData.is_object()) jsonData.merge_patch(derivedData);

	nlohmann::json gameObjectsData = nlohmann::json::array();
	for (unique_ptr<Base>& gameObject : m_gameObjects) gameObjectsData.push_back(gameObject->BaseSerialize());
	jsonData["rootGameObjects"] = gameObjectsData;

	return jsonData;
}

void SceneBase::BaseDeserialize(const nlohmann::json& jsonData)
{
	if (jsonData.contains("lightColor"))
	{
		m_globalLightData.lightColor = XMFLOAT4
		(
			jsonData["lightColor"][0].get<float>(),
			jsonData["lightColor"][1].get<float>(),
			jsonData["lightColor"][2].get<float>(),
			jsonData["lightColor"][3].get<float>()
		);
	}
	if (jsonData.contains("lightDirection"))
	{
		m_globalLightData.lightDirection = XMVectorSet
		(
			jsonData["lightDirection"][0].get<float>(),
			jsonData["lightDirection"][1].get<float>(),
			jsonData["lightDirection"][2].get<float>(),
			jsonData["lightDirection"][3].get<float>()
		);
	}

	if (jsonData.contains("postProcessing"))
	{
		const nlohmann::json& ppData = jsonData["postProcessing"];
		if (ppData.contains("flags")) m_postProcessingData.flags = ppData["flags"].get<UINT>();
		if (ppData.contains("bloomIntensity")) m_postProcessingData.bloomIntensity = ppData["bloomIntensity"].get<float>();
		if (ppData.contains("gammaIntensity")) m_postProcessingData.gammaIntensity = ppData["gammaIntensity"].get<float>();
		if (ppData.contains("grayScaleIntensity")) m_postProcessingData.grayScaleIntensity = ppData["grayScaleIntensity"].get<float>();
		if (ppData.contains("vignettingColor"))
		{
			m_postProcessingData.vignettingColor = XMFLOAT4
			(
				ppData["vignettingColor"][0].get<float>(),
				ppData["vignettingColor"][1].get<float>(),
				ppData["vignettingColor"][2].get<float>(),
				ppData["vignettingColor"][3].get<float>()
			);
		}
		if (ppData.contains("radialBlurParam"))
		{
			m_postProcessingData.radialBlurParam = XMFLOAT4
			(
				ppData["radialBlurParam"][0].get<float>(),
				ppData["radialBlurParam"][1].get<float>(),
				ppData["radialBlurParam"][2].get<float>(),
				ppData["radialBlurParam"][3].get<float>()
			);
		}
		if (ppData.contains("lutLerpFactor")) m_postProcessingData.lutLerpFactor = ppData["lutLerpFactor"].get<float>();
	}

	if (jsonData.contains("skyboxFileName")) m_skyboxFileName = jsonData["skyboxFileName"].get<string>();
	if (jsonData.contains("environmentMapFileName")) m_environmentMapFileName = jsonData["environmentMapFileName"].get<string>();

	NavigationManager::GetInstance().Deserialize(jsonData);

	m_UIList.clear();
	if (jsonData.contains("UI")) {
		const nlohmann::json& uiArray = jsonData["UI"];
		for (const auto& uiItem : uiArray) {
			string type = uiItem["type"].get<string>();
			UIBase* newUI = UIBase::CreateFactory(type);

			if (newUI) {
				newUI->Deserialize(uiItem);
				m_UIList.push_back(unique_ptr<UIBase>(newUI));
			} else {}
		}
		for (size_t i = 0; i < uiArray.size(); ++i) {			
			if (i >= m_UIList.size()) break;
			if (m_UIList[i] == nullptr) continue;
			int parentIdx = uiArray[i].value("parentIdx", -1);
			if (parentIdx >= 0 && parentIdx < static_cast<int>(m_UIList.size())) {
				m_UIList[i]->SetParent(m_UIList[parentIdx].get());
			}
		}
	}
	BindUIActions();

	Deserialize(jsonData);

	GameObjectBase::SetSelectedObject(nullptr);
	BaseFinalize();
	m_gameObjects.clear();

	for (const auto& gameObjectData : jsonData["rootGameObjects"])
	{
		string typeName = gameObjectData["type"].get<string>();
		unique_ptr<Base> gameObjectPtr = TypeRegistry::GetInstance().CreateGameObject(typeName);

		gameObjectPtr->BaseDeserialize(gameObjectData);
		m_gameObjects.push_back(move(gameObjectPtr));
	}
}

void SceneBase::RemovePending()
{
	erase_if
	(
		m_gameObjects, [](const unique_ptr<Base>& gameObject)
		{
			if (!gameObject->GetAlive())
			{
				gameObject->BaseFinalize();
				return true;
			}
			return false;
		}
	);
}

void SceneBase::GetResources()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();

	m_skyboxVertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout("VSSkybox.hlsl");
	m_skyboxPixelShader = resourceManager.GetPixelShader("PSSkybox.hlsl");
	m_shadowMapPixelShader = resourceManager.GetPixelShader("PSShadow.hlsl");

	#ifdef _DEBUG
	m_debugCoordinateVertexShaderAndInputLayout = resourceManager.GetVertexShaderAndInputLayout("VSCoordinateLine.hlsl");
	m_debugCoordinatePixelShader = resourceManager.GetPixelShader("PSColor.hlsl");
	#endif

	m_skyboxSRV = resourceManager.GetTexture(m_skyboxFileName);
	m_environmentMapSRV = resourceManager.GetTexture(m_environmentMapFileName);

	m_viewProjectionConstantBuffer = resourceManager.GetConstantBuffer(VSConstBuffers::ViewProjection);
	#ifdef _DEBUG
	m_viewProjectionForNormalConstantBuffer = resourceManager.GetConstantBuffer(GSConstBuffers::NormalViewProjection);
	#endif
	m_skyboxViewProjectionConstantBuffer = resourceManager.GetConstantBuffer(VSConstBuffers::SkyboxViewProjection);
	m_timeConstantBuffer = resourceManager.GetConstantBuffer(VSConstBuffers::Time);

	m_cameraPositionConstantBuffer = resourceManager.GetConstantBuffer(PSConstBuffers::CameraPosition);
	m_globalLightConstantBuffer = resourceManager.GetConstantBuffer(PSConstBuffers::GlobalLight);

	m_postProcessingConstantBuffer = resourceManager.GetConstantBuffer(PSConstBuffers::PostProcessing);
}

void SceneBase::UpdateConstantBuffers()
{
	const CameraComponent& mainCamera = CameraComponent::GetMainCamera();

	m_viewProjectionData.viewMatrix = mainCamera.GetViewMatrix();
	m_viewProjectionData.projectionMatrix = mainCamera.GetProjectionMatrix();
	m_viewProjectionData.VPMatrix = XMMatrixTranspose(m_viewProjectionData.viewMatrix * m_viewProjectionData.projectionMatrix);
	m_deviceContext->UpdateSubresource(m_viewProjectionConstantBuffer.Get(), 0, nullptr, &m_viewProjectionData, 0, 0);

	#ifdef _DEBUG
	m_viewProjectionForNormalData.viewProjectionMatrix = m_viewProjectionData.VPMatrix;
	m_deviceContext->UpdateSubresource(m_viewProjectionForNormalConstantBuffer.Get(), 0, nullptr, &m_viewProjectionForNormalData, 0, 0);
	#endif

	m_viewProjectionData.viewMatrix.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	m_skyboxViewProjectionData.skyboxVPMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, m_viewProjectionData.viewMatrix * m_viewProjectionData.projectionMatrix));
	m_deviceContext->UpdateSubresource(m_skyboxViewProjectionConstantBuffer.Get(), 0, nullptr, &m_skyboxViewProjectionData, 0, 0);

	m_timeData.totalTime = TimeManager::GetInstance().GetTotalTime();
	m_timeData.deltaTime = TimeManager::GetInstance().GetDeltaTime();
	m_timeData.sinTime = sinf(m_timeData.totalTime);
	m_timeData.cosTime = cosf(m_timeData.totalTime);
	m_deviceContext->UpdateSubresource(m_timeConstantBuffer.Get(), 0, nullptr, &m_timeData, 0, 0);

	m_cameraPositionData.cameraPosition = mainCamera.GetPosition();
	m_deviceContext->UpdateSubresource(m_cameraPositionConstantBuffer.Get(), 0, nullptr, &m_cameraPositionData, 0, 0);

	m_deviceContext->UpdateSubresource(m_globalLightConstantBuffer.Get(), 0, nullptr, &m_globalLightData, 0, 0);

	m_deviceContext->UpdateSubresource(m_postProcessingConstantBuffer.Get(), 0, nullptr, &m_postProcessingData, 0, 0);
}

void SceneBase::RenderSkybox()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();

	resourceManager.SetDepthStencilState(DepthStencilState::Skybox);

	resourceManager.SetRasterState(RasterState::Solid);
	resourceManager.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_deviceContext->IASetInputLayout(m_skyboxVertexShaderAndInputLayout.second.Get());
	m_deviceContext->VSSetShader(m_skyboxVertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);

	constexpr UINT STRIDE = 0;
	constexpr UINT OFFSET = 0;
	constexpr ID3D11Buffer* nullBuffer = nullptr;
	m_deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &STRIDE, &OFFSET);

	m_deviceContext->Draw(3, 0);

	resourceManager.SetDepthStencilState(DepthStencilState::Default);
}

#ifdef _DEBUG
void SceneBase::PickObjectDebugCamera()
{
	const POINT& mouse = InputManager::GetInstance().GetMousePosition();
	pair<XMVECTOR, XMVECTOR> ray = CameraComponent::GetMainCamera().RayCast(static_cast<float>(mouse.x), static_cast<float>(mouse.y));
	GameObjectBase::SetSelectedObject(ModelComponent::CheckCollision(ray.first, ray.second));
}

void SceneBase::RenderDebugCoordinates()
{
	ResourceManager& resourceManager = ResourceManager::GetInstance();

	resourceManager.SetRasterState(RasterState::Solid);
	resourceManager.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	m_deviceContext->IASetInputLayout(m_debugCoordinateVertexShaderAndInputLayout.second.Get());
	m_deviceContext->VSSetShader(m_debugCoordinateVertexShaderAndInputLayout.first.Get(), nullptr, 0);
	m_deviceContext->PSSetShader(m_debugCoordinatePixelShader.Get(), nullptr, 0);

	constexpr UINT STRIDE = 0;
	constexpr UINT OFFSET = 0;
	constexpr ID3D11Buffer* nullBuffer = nullptr;
	m_deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &STRIDE, &OFFSET);

	m_deviceContext->DrawInstanced(2, 2004, 0, 0);
}

void SceneBase::BakeCollider()
{
	for (unique_ptr<Base>& gameObject : m_gameObjects)
	{
		GameObjectBase* gameObjectBase = dynamic_cast<GameObjectBase*>(gameObject.get());
		if (gameObjectBase->GetComponent<ModelComponent>() && !gameObjectBase->GetComponent<ColliderComponent>())
		{
			gameObjectBase->CreateComponent<ColliderComponent>()->LoadFromModelMesh();
		}
	}
}

#endif
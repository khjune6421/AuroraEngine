#include "stdafx.h"
#include "BlenderCameraObject.h"
#include "InputManager.h"

REGISTER_TYPE(BlenderCameraObject)

void BlenderCameraObject::Initialize()
{
	SetPosition({ 0.0f, 10.0f, -20.0f });
	LookAt({ 0.0f, 0.0f, 0.0f });
}

void BlenderCameraObject::Update()
{
	//using enum KeyCode;
	//auto& input = InputManager::GetInstance();
	//float wheel = InputManager::GetInstance().GetMouseWheel();


 //   if (wheel != 0.0f)
 //   {
 //       // 현재 로컬 위치 가져오기 (예: 0, 10, -20)
 //       XMVECTOR pos = GetPosition();

 //       // 줌 속도 조절
 //       float zoomSpeed = 10.0f * deltaTime;

 //       // 앞뒤(Z축)로 이동. (좌표계에 따라 +가 줌인일수도 -가 줌인일수도 있음)
 //       pos.z += wheel * zoomSpeed;

 //       // 너무 가까워지거나 멀어지지 않게 클램핑(선택사항)
 //       // pos.z = std::clamp(pos.z, -50.0f, -2.0f);

 //       SetPosition(pos);
 //   }

 //   // 2. 시선 고정 (필수!)
 //   // 내가 어디로 이동하든, 부모(0,0,0 - 로컬 기준)를 쳐다본다.
 //   LookAt({ 0.0f, 0.0f, 0.0f });
}
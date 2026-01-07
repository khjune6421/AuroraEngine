///LineObject.cpp의 시작
#include "stdafx.h"
#include "LineObject.h"

#include "LineComponent.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(LineObject)

void LineObject::Initialize()
{
	CreateComponent<LineComponent>(); // 기본 생성
	SetScale({ 10.0f, 1.0f, 1.0f });
	//SetPosition({ -5.0f, .0f, .0f });
	//GetComponent<LineComponent>()->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
}

void LineObject::Update()
{

}

///LineObject.cpp의 끝
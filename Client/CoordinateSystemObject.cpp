///CoordinateSystemObject.cpp의 시작
#include "stdafx.h"
#include "CoordinateSystemObject.h"

#include "LineComponent.h"
#include "LineObject.h"

using namespace std;
using namespace DirectX;

REGISTER_TYPE(CoordinateSystemObject)

void CoordinateSystemObject::Initialize()
{

	for (int i = 0; i < 21; i++)
	{
		CreateChildGameObject<LineObject>();
		m_childrens[i]->SetScale({ 100.0f, 1.0f, 1.0f });
		m_childrens[i]->SetPosition({ 0, .0f, -50.0f + 5 * i });
		if(i == 10)
		{
			//블랜더 x축색
			m_childrens[i]->GetComponent<LineComponent>()->SetColor({ 0.965f, 0.212f, 0.322f, 1.0f });
		}
		else
		{
			//블랜더 일반축색
			m_childrens[i]->GetComponent<LineComponent>()->SetColor({ 0.616f, 0.624f, 0.631f, 0.1f });
		}
	}

	for (int i = 0; i < 21; i++)
	{
		CreateChildGameObject<LineObject>();
		m_childrens[i + 21]->SetScale({ 100.0f, 1.0f, 1.0f });
		m_childrens[i + 21]->SetRotation({0, 90 ,0});
		m_childrens[i + 21]->SetPosition({ -50.0f + 5 * i, .0f, 0 });
		if (i == 10)
		{
			//블랜더 z축색
			m_childrens[i + 21]->GetComponent<LineComponent>()->SetColor({ 0.176f, 0.541f, 0.949f, 1.0f });
		}
		else
		{
			//블랜더 일반축색
			m_childrens[i + 21]->GetComponent<LineComponent>()->SetColor({ 0.616f, 0.624f, 0.631f, 0.1f });
		}
	}

	{
		
		CreateChildGameObject<LineObject>();
		//m_childrens[21+21]->SetScale({ 1.0f, 1.0f, 1.0f }); //이게 맞는데
		m_childrens[21+21]->SetScale({ 5.0f, 1.0f, 1.0f }); 
		m_childrens[21+21]->SetRotation({ 0, 0 ,90 });
		//m_childrens[21+21]->SetPosition({ .0f , .0f, 0.5f }); // 이게 맞는데
		m_childrens[21+21]->SetPosition({ .0f , 2.5f, .0f });
		// 블랜더 y축색
		m_childrens[21+21]->GetComponent<LineComponent>()->SetColor({ 0.498f, 0.769f, 0.067f, 1.0f });
	}

}

void CoordinateSystemObject::Update()
{

}

///CoordinateSystemObject.cpp의 끝
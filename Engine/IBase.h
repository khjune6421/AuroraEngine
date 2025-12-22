#pragma once

class IBase
{
public:
	IBase() = default;
	virtual ~IBase() = default;
	IBase(const IBase&) = default; // 복사
	IBase& operator=(const IBase&) = default; // 복사 대입
	IBase(IBase&&) = default; // 이동
	IBase& operator=(IBase&&) = default; // 이동 대입

	virtual void Initialize() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;
	virtual void Finalize() = 0;
};
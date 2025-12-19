#pragma once

class IBase
{
public:
	virtual ~IBase() = default;
	IBase(const IBase&) = delete;
	IBase& operator=(const IBase&) = delete;
	IBase(IBase&&) = delete;
	IBase& operator=(IBase&&) = delete;

	virtual void Initialize(IBase* parent) = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Render() = 0;
	virtual void Finalize() = 0;
};
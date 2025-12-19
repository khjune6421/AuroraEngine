#pragma once

class IBase
{
protected:
	IBase* m_parent = nullptr;
	std::vector<std::unique_ptr<IBase>> m_children = {};

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
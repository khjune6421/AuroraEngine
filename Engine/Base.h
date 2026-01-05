#pragma once

class IBase
{
public:
	IBase() = default;
	virtual ~IBase() = default;
	IBase(const IBase&) = default;
	IBase& operator=(const IBase&) = default;
	IBase(IBase&&) = default;
	IBase& operator=(IBase&&) = default;

	virtual void BaseInitialize() = 0;
	virtual void BaseUpdate() = 0;
	virtual void BaseRender() = 0;
	virtual void BaseRenderImGui() = 0;
	virtual void BaseFinalize() = 0;

	virtual nlohmann::json BaseSerialize() = 0;
	virtual void BaseDeserialize(const nlohmann::json& jsonData) = 0;

	// 소유한 객체나 자원 중 제거 대기중인 항목이 있으면 모두 제거
	virtual void RemovePending() = 0;
};

class Base : public IBase
{
	bool m_isActive = true; // 활성화 여부 // TODO: 기능 추가
	bool m_isAlive = true; // 생존 여부

protected:
	std::string m_type = "Base"; // 타입 이름

public:
	Base() = default;
	virtual ~Base() = default;
	Base(const Base&) = default;
	Base& operator=(const Base&) = default;
	Base(Base&&) = default;
	Base& operator=(Base&&) = default;

	std::string GetType() const { return m_type; }

	void SetActive(bool isActive) { m_isActive = isActive; }
	bool GetActive() const { return m_isActive; }
	void SetAlive(bool isAlive) { m_isAlive = isAlive; }
	bool GetAlive() const { return m_isAlive; }

protected:
	// 파생 클래스의 초기화
	virtual void Initialize() {}
	// 파생 클래스의 업데이트
	virtual void Update() {}
	// 파생 클래스의 렌더링
	virtual void Render() {}
	// 파생 클래스의 ImGui 렌더링
	virtual void RenderImGui() {}
	// 파생 클래스의 종료
	virtual void Finalize() {}

	// 파생 클래스의 직렬화
	virtual nlohmann::json Serialize() { return nlohmann::json(); }
	// 파생 클래스의 역직렬화
	virtual void Deserialize(const nlohmann::json& jsonData) {}
};
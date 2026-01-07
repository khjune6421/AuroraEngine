/// InputManager.h의 시작
/// https://github.com/DragonT-iger/DTEngine/blob/main/DTEngine/InputManager.h
/// 25.12.30 기준 async를 HandleMessage로 바꾸기 위함임 
///
#pragma once

#include "Singleton.h"
#include <array>
#include <cstdint>   
#include "KeyCode.h" 

struct MousePos
{
    int x;
    int y;
};


class InputManager : public Singleton<InputManager>
{
public:
    friend class Singleton<InputManager>;

    using UINT = unsigned int;
    using WPARAM = std::uintptr_t;
    using LPARAM = std::intptr_t;

    void Initialize();
    void Update();

    void HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void EndFrame();

    bool GetKeyDown(KeyCode key) const;
    bool GetKey(KeyCode key) const;
    bool GetKeyUp(KeyCode key) const;

    const MousePos& GetMousePosition() const { return m_mousePos; }
    const MousePos& GetMouseDelta() const { return m_mouseDelta; }
    int GetMouseWheel() const { return m_wheelDelta; }

private:
    InputManager() = default;
    ~InputManager() = default;

    std::array<bool, 256> m_keyState;
    std::array<bool, 256> m_keyDownState;
    std::array<bool, 256> m_keyUpState;

    MousePos m_mousePos = { 0, 0 };
    MousePos m_prevMousePos = { 0, 0 };
    MousePos m_mouseDelta = { 0, 0 };
    int m_wheelDelta = 0;

    int MapKeyCodeToVKey(KeyCode key) const;
};

/// InputManager.h의 끝
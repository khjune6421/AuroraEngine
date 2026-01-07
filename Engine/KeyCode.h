/// KeyCode.h의 시작
/// https://github.com/DragonT-iger/DTEngine/blob/main/DTEngine/KeyCode.h
/// 
#pragma once

enum class KeyCode
{

    None = 0,

    // 마우스
    MouseLeft = 1,
    MouseRight = 2,
    MouseMiddle = 3,

    // 제어 키
    Control,
    Shift,
    Alt,
    Space,
    Enter,
    Escape,
    Tab,
    Backspace,
    Delete,

    // 방향 키
    Left,
    Right,
    Up,
    Down,

    // 문자
    A = 'A',
    B = 'B',
    C = 'C',
    D = 'D',
    E = 'E',
    F = 'F',
    G = 'G',
    H = 'H',
    I = 'I',
    J = 'J',
    K = 'K',
    L = 'L',
    M = 'M',
    N = 'N',
    O = 'O',
    P = 'P',
    Q = 'Q',
    R = 'R',
    S = 'S',
    T = 'T',
    U = 'U',
    V = 'V',
    W = 'W',
    X = 'X',
    Y = 'Y',
    Z = 'Z',

    // 숫자
    Num0 = '0',
    Num1 = '1',
    Num2 = '2',
    Num3 = '3',
    Num4 = '4',
    Num5 = '5',
    Num6 = '6',
    Num7 = '7',
    Num8 = '8',
    Num9 = '9',

    // F 키 (F1 ~ F12)
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};
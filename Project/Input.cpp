#include "Input.h"

void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
  HRESULT result;

#pragma region DirectInputの初期化
  IDirectInput8 *directInput = nullptr;

  result = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
                              (void **)&directInput, nullptr);
  assert(SUCCEEDED(result));
#pragma endregion

#pragma region キーボードの初期化
  IDirectInputDevice8 *keyboard = nullptr;

  result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
  assert(SUCCEEDED(result));

  result = keyboard->SetDataFormat(&c_dfDIKeyboard);
  assert(SUCCEEDED(result));

  result = keyboard->SetCooperativeLevel(
      hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
  assert(SUCCEEDED(result));

#pragma endregion
}

void Input::Update() {}

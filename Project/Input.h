#pragma once
#include <Windows.h>
#include <wrl.h>
#define DRECTINPUT_VERSION 0x0800 // DirectInput version 8.0
#include <cassert>
#include <dinput.h>

class Input {
//public:
//  template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:
  void Initialize(HINSTANCE hInstance, HWND hwnd);
  void Update();

private:
  Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard = nullptr;
};

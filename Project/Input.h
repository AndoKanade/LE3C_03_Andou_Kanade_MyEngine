#pragma once
#include <Windows.h>
#include <wrl.h>
#define DRECTINPUT_VERSION 0x0800 // DirectInput version 8.0
#include <cassert>
#include <dinput.h>

class Input {
public:
  void Initialize(HINSTANCE hInstance, HWND hwnd);
  void Update();
};

#pragma once
#include <Windows.h>
#include <cassert>
#include <wrl.h>
#define DRECTINPUT_VERSION 0x0800 // DirectInput version 8.0
#include <dinput.h>

class Input {
  // public:
  //   template <class T> using Comptr = Microsoft::WRL::ComPtr<T>;

public:
  void Initialize(HINSTANCE hInstance, HWND hwnd);
  void Update();

  bool PushKey(BYTE keyNumber);
  bool TriggerKey(BYTE keyNumber);

private:
  Microsoft::WRL::ComPtr<IDirectInput8> directInput = nullptr;
  Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard = nullptr;

  BYTE key[256] = {};
  BYTE preKey[256] = {};
};

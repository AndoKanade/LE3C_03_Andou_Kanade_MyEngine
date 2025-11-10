#pragma once
#include "WinAPI.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

class DXCommon {
public:
  Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
  Microsoft::WRL::ComPtr<ID3D12Device> device;

  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;

  Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;

  Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;

  uint32_t descriptorSizeSRV = 0;
  uint32_t descriptorSizeRTV = 0;
  uint32_t descriptorSizeDSV = 0;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

  std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> backBuffers;
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

  void Initialize(WinAPI *winApi);

  void InitDevice();
  void InitCommand();
  void CreateSwapChain();
  void CreateDepthBuffer();
  void CreateDescriptorHeaps();
  void InitRenderTargetView();

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
  CreateDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors,
                       bool shaderVisible);

private:
  WinAPI *winApi_ = nullptr;

  static D3D12_CPU_DESCRIPTOR_HANDLE
  GetCPUDescriptorHandle(ID3D12DescriptorHeap *descriptorHeap,
                         uint32_t descriptorSize, uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU =
        descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += descriptorSize * index;
    return handleCPU;
  }

  static D3D12_GPU_DESCRIPTOR_HANDLE
  GetGPUDscriptorHandle(ID3D12DescriptorHeap *descriptorHeap,
                        uint32_t descriptorSize, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU =
        descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += descriptorSize * index;
    return handleGPU;
  }
};

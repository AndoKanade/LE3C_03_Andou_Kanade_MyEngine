#pragma once
#include "WinAPI.h"
#include <array>
#include <d3d12.h>
#include <dxcapi.h>
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
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
  std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> swapChainResources;

  Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
  D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

  uint32_t descriptorSizeSRV = 0;
  uint32_t descriptorSizeRTV = 0;
  uint32_t descriptorSizeDSV = 0;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = nullptr;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = nullptr;
  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap = nullptr;

  std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, 2> backBuffers;
  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

  Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;

  D3D12_VIEWPORT viewport{};
  D3D12_RECT scissorRect{};

  Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils = nullptr;
  Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler = nullptr;
  IDxcIncludeHandler *includeHandler = nullptr;

  D3D12_RESOURCE_BARRIER barrier{};
  uint64_t fenceValue = 0;
  HANDLE fenceEvent = nullptr;

  void Initialize(WinAPI *winApi);

  void InitDevice();
  void InitCommand();
  void CreateSwapChain();
  void CreateDepthBuffer();
  void CreateDescriptorHeaps();
  void InitRenderTargetView();
  void InitDepthStancilView();
  void InitFence();
  void InitViewportRect();
  void InitScissorRect();
  void CreateDXCCompiler();
  void InitImGui();

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
  CreateDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors,
                       bool shaderVisible);

  D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCPUDescriptorHandle(uint32_t index);
  D3D12_GPU_DESCRIPTOR_HANDLE GetSRVGPUDescriptorHandle(uint32_t index);

  void PreDraw();
  void PostDraw();

private:
  WinAPI *winApi_ = nullptr;

  static D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
      const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &descriptorHeap,
      uint32_t descriptorSize, uint32_t index) {
    D3D12_CPU_DESCRIPTOR_HANDLE handleCPU =
        descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    handleCPU.ptr += descriptorSize * index;
    return handleCPU;
  }

  static D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDscriptorHandle(
      const Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> &descriptorHeap,
      uint32_t descriptorSize, uint32_t index) {
    D3D12_GPU_DESCRIPTOR_HANDLE handleGPU =
        descriptorHeap->GetGPUDescriptorHandleForHeapStart();
    handleGPU.ptr += descriptorSize * index;
    return handleGPU;
  }
};

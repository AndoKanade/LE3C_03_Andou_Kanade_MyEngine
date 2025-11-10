#include "DXCommon.h"
#include "Logger.h"
#include "StringUtility.h"
#include <cassert>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace Microsoft::WRL;
using namespace Logger;
using namespace StringUtility;
void DXCommon::Initialize(WinAPI *winApi) {

  assert(winApi);
  this->winApi_ = winApi;
  InitDevice();
  InitCommand();
  CreateSwapChain();
}

void DXCommon::InitDevice() {

  HRESULT hr;

  /// デバッグレイヤーをオン
  Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
  if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
    // デバッグレイヤーを有効にする
    debugController->EnableDebugLayer();

    // GPUでもチェックするようにする
    debugController->SetEnableGPUBasedValidation(TRUE);
  }

  /// DXGIファクトリーの生成

  HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

  assert(SUCCEEDED(hr));

  /// アダプターの選定
  // 使用するアダプターの変数、最初はnullptrを入れておく
  Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;

  // 良い順にアダプターを頼む
  for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(
                       i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                       IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND;
       i++) {

    // アダプターの情報を取得する
    DXGI_ADAPTER_DESC3 adapterDesc{};

    hr = useAdapter->GetDesc3(&adapterDesc);
    assert(SUCCEEDED(hr));

    // ソフトウェアアダプター出なければ採用
    if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
      Log(std::format("Use Adapter : {}\n",
                      ConvertString(adapterDesc.Description)));
      break;
    }
    useAdapter = nullptr;
  }

  assert(useAdapter != nullptr);

  /// D3D12デバイスの生成

  D3D_FEATURE_LEVEL featureLevels[] = {

      D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0};

  const char *featureLevelStrings[] = {"12.2", "12.1", "12.0"};

  // 高い順に生成できるか試していく
  for (size_t i = 0; i < _countof(featureLevels); i++) {
    hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i],
                           IID_PPV_ARGS(&device));

    if (SUCCEEDED(hr)) {
      // 生成できたのでループを抜ける
      Log(std::format("FeatureLevels : {}\n", featureLevelStrings[i]));
      break;
    }
  }

  // 生成がうまくいかなかったので起動しない
  assert(SUCCEEDED(hr));
  Log("Complete create D3D12Device!!\n");
}

void DXCommon::InitCommand() {

  HRESULT hr;
  D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
  hr = device->CreateCommandQueue(&commandQueueDesc,
                                  IID_PPV_ARGS(&commandQueue));

  // コマンドキューの生成に失敗したら起動しない
  assert(SUCCEEDED(hr));

  // コマンドアロケータの生成
  hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                      IID_PPV_ARGS(&commandAllocator));

  // コマンドリストを生成する
  hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                 commandAllocator.Get(), nullptr,
                                 IID_PPV_ARGS(&commandList));
  // コマンドリストの生成に失敗したら起動しない
  assert(SUCCEEDED(hr));
}

void DXCommon::CreateSwapChain() {

  HRESULT hr;
  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
  swapChainDesc.Width = winApi_->kCliantWidth;
  swapChainDesc.Height = winApi_->kCliantHeight;
  swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  swapChainDesc.BufferCount = 2;
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

  // コマンドキュー、ウィンドウハンドル、スワップチェインの設定

  hr = dxgiFactory->CreateSwapChainForHwnd(
      commandQueue.Get(), winApi_->GetHwnd(), &swapChainDesc, nullptr, nullptr,
      reinterpret_cast<IDXGISwapChain1 **>(swapChain.GetAddressOf()));
  // スワップチェインの生成に失敗したら起動しない
  assert(SUCCEEDED(hr));
}

void DXCommon::CreateDepthBuffer() {

  D3D12_RESOURCE_DESC resourceDesc{};

  resourceDesc.Width = winApi_->kCliantWidth;
  resourceDesc.Height = winApi_->kCliantHeight;
  resourceDesc.MipLevels = 1;
  resourceDesc.DepthOrArraySize = 1;
  resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  resourceDesc.SampleDesc.Count = 1;
  resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
  resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

  // 利用するheapの設定
  D3D12_HEAP_PROPERTIES heapProperties{};

  heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

  D3D12_CLEAR_VALUE depthClearValue{};

  depthClearValue.DepthStencil.Depth = 1.0f;
  depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

  HRESULT hr = device->CreateCommittedResource(
      &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
      D3D12_RESOURCE_STATE_COPY_DEST, &depthClearValue,
      IID_PPV_ARGS(&depthStencilResource));
  assert(SUCCEEDED(hr));
}

void DXCommon::CreateDescriptorHeaps() {

  HRESULT hr;

  descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(
      D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

  descriptorSizeRTV =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
  descriptorSizeDSV =
      device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

  rtvDescriptorHeap =
      CreateDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

  srvDescriptorHeap =
      CreateDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 28, true);

  dsvDescriptorHeap =
      CreateDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);
}

void DXCommon::InitRenderTargetView() {

  HRESULT hr;
  const UINT kNumBackBuffers = 2;

  D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
      rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

  D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
  rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
  rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

  for (uint32_t i = 0; i < kNumBackBuffers; ++i) {

    hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
    assert(SUCCEEDED(hr));

    rtvHandles[i] = rtvHandle;

    device->CreateRenderTargetView(backBuffers[i].Get(), &rtvDesc, rtvHandle);

    rtvHandle.ptr += descriptorSizeRTV;
  }
}

ComPtr<ID3D12DescriptorHeap>
DXCommon::CreateDiscriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                               UINT numDescriptors, bool shaderVisible) {

  Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
  D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
  descriptorHeapDesc.Type = heapType;
  descriptorHeapDesc.NumDescriptors = numDescriptors;
  descriptorHeapDesc.Flags = shaderVisible
                                 ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                 : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
  HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc,
                                            IID_PPV_ARGS(&descriptorHeap));
  assert(SUCCEEDED(hr));

  return descriptorHeap;
}

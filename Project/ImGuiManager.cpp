#include "ImGuiManager.h"
#include "WinAPI.h"
#include "DXCommon.h"
#include "SrvManager.h"

// ImGuiのライブラリヘッダー
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx12.h>

// ==========================================================================
// シングルトンインスタンスの取得
// ==========================================================================
ImGuiManager* ImGuiManager::GetInstance(){
    // 関数内でstatic変数を宣言すると、プログラム終了まで生き続ける
    static ImGuiManager instance;
    return &instance;
}

// ==========================================================================
// 初期化処理
// ==========================================================================
void ImGuiManager::Initialize(WinAPI* winApi,DXCommon* dxCommon){
    // 1. メンバ変数への保存
    dxCommon_ = dxCommon;

    // 2. ImGuiコンテキストの作成 (ImGuiの管理領域を作る)
    ImGui::CreateContext();

    // 3. 見た目の設定 (好みに合わせて変更可能。今回はダークモード)
    ImGui::StyleColorsDark();

    // 4. Win32(OS)周りの初期化
    ImGui_ImplWin32_Init(winApi->GetHwnd());

    // 5. SRV(Shader Resource View)ヒープ領域の確保
    // ImGuiは文字を表示するために「フォントテクスチャ」を使うので、SRVヒープに場所が必要
    SrvManager* srvManager = SrvManager::GetInstance();

    // SRVマネージャから空いているインデックスを1つもらう
    srvIndex_ = srvManager->Allocate();

    // 描画時にセットするために、ヒープ自体のポインタも保存しておく
    srvHeap_ = srvManager->GetDescriptorHeap();

    // 6. DirectX12周りの初期化
    ImGui_ImplDX12_InitInfo initInfo = {};

    // 必須情報のセット
    initInfo.Device = dxCommon_->GetDevice();
    initInfo.CommandQueue = dxCommon_->GetCommandQueue(); // ★DXCommonに追加したゲッターを使う
    initInfo.NumFramesInFlight = static_cast<int>(dxCommon_->GetSwapChainResourcesNum());
    initInfo.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    initInfo.DSVFormat = DXGI_FORMAT_UNKNOWN; // ImGuiで深度バッファを使わない場合はUNKNOWNでOK

    // ヒープの設定
    initInfo.SrvDescriptorHeap = srvHeap_;

    // ★重要: 従来の「1個だけ確保して渡す」方式の場合は Legacy~ に代入する
    initInfo.LegacySingleSrvCpuDescriptor = srvManager->GetCPUDescriptorHandle(srvIndex_);
    initInfo.LegacySingleSrvGpuDescriptor = srvManager->GetGPUDescriptorHandle(srvIndex_);

    // 初期化実行
    ImGui_ImplDX12_Init(&initInfo);
}

// ==========================================================================
// フレーム開始処理
// ==========================================================================
void ImGuiManager::Begin(){
    // DirectX12用、Win32用、ImGui本体の順で新しいフレームの開始を通知する
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

// ==========================================================================
// フレーム終了処理 (内部コマンド生成)
// ==========================================================================
void ImGuiManager::End(){
    // ImGuiの描画処理を確定させ、描画データを作成する
    // ※ ここではまだ画面には出ない
    ImGui::Render();
}

// ==========================================================================
// 描画処理 (GPUコマンド発行)
// ==========================================================================
void ImGuiManager::Draw(){
    // コマンドリストを取得
    ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

    // 1. ImGuiが使うディスクリプタヒープをセット
    ID3D12DescriptorHeap* ppHeaps[] = {srvHeap_};
    commandList->SetDescriptorHeaps(_countof(ppHeaps),ppHeaps);

    // 2. 生成された描画データを元に、実際の描画コマンドを積む
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(),commandList);
}

// ==========================================================================
// 終了処理
// ==========================================================================
void ImGuiManager::Finalize(){
    // 初期化と逆の順序で終了処理を行うのが基本
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
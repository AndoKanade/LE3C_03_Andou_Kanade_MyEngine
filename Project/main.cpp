#define _USE_MATH_DEFINES
#define PI 3.14159265f
#include "DXCommon.h"
#include "Input.h"
#include "StringUtility.h"
#include "WinAPI.h"
#include <chrono>
#include <cmath>
#include <dbghelp.h>
#include <dxcapi.h>
#include <dxgidebug.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string.h>
#include <strsafe.h>
#include <vector>
#include <xaudio2.h>
#include "SpriteCommon.h"
#include "Sprite.h"
#include "Math.h"
#include "Obj3DCommon.h"
#include "Obj3D.h"
#include "ModelCommon.h" // ★追加
#include "Model.h"       // ★追加
#include"Logger.h"

#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include <iostream>
#include <map>
#include "TextureManager.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lPalam);

#pragma comment(lib, "Dbghelp.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "xaudio2.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

using namespace DirectX;

LRESULT CALLBACK WindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam){
	if(ImGui_ImplWin32_WndProcHandler(hwnd,msg,wparam,lparam)){
		return true;
	}

	switch(msg){
	case WM_DESTROY:

		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProcW(hwnd,msg,wparam,lparam);
}
struct D3DResourceLeakChecker{
	~D3DResourceLeakChecker(){

		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if(SUCCEEDED(DXGIGetDebugInterface1(0,IID_PPV_ARGS(&debug)))){
			debug->ReportLiveObjects(DXGI_DEBUG_ALL,DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP,DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12,DXGI_DEBUG_RLO_ALL);
		}
	}
};

#pragma region サウンド再生
struct ChunkHeader{
	char id[4];
	int32_t size;
};

struct RiffHeader{
	ChunkHeader chunk;
	char type[4];
};

struct FormatChunk{
	ChunkHeader chunk;
	WAVEFORMATEX fmt;
};

struct SoundData{
	WAVEFORMATEX wfex;
	BYTE* pBUffer;
	unsigned int bufferSize;
	;
};

#pragma endregion

struct WindowData{
	HINSTANCE hInstance;
	HWND hwnd;
};

#pragma region 音声データの読み込み

SoundData SoundLoadWave(const char* filename){

	std::ifstream file;
	file.open(filename,std::ios_base::binary);
	assert(file.is_open());

	RiffHeader riff;
	file.read((char*)&riff,sizeof(riff));

	if(strncmp(riff.chunk.id,"RIFF",4) != 0){
		assert(0);
	}

	if(strncmp(riff.type,"WAVE",4) != 0){
		assert(0);
	}

	FormatChunk format = {};
	file.read((char*)&format,sizeof(ChunkHeader));

	if(strncmp(format.chunk.id,"fmt ",4) != 0){
		assert(0);
	}

	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt,format.chunk.size);

	ChunkHeader data;
	file.read((char*)&data,sizeof(data));

	if(strncmp(data.id,"JUNK",4) == 0){
		file.seekg(data.size,std::ios_base::cur);

		file.read((char*)&data,sizeof(data));
	}

	if(strncmp(data.id,"data",4) != 0){
		assert(0);
	}

	char* pBuffer = new char[data.size];
	file.read(pBuffer,data.size);

	file.close();

	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBUffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

#pragma endregion

#pragma region 音声データの解放
void SoundUnload(SoundData* soundData){
	delete[] soundData->pBUffer;

	soundData->pBUffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

#pragma endregion

#pragma region サウンドの再生

void SoundPlayWave(IXAudio2* xAudio2,const SoundData& soundData){
	HRESULT result;

	IXAudio2SourceVoice* pSourceVoice = nullptr;
	result = xAudio2->CreateSourceVoice(&pSourceVoice,&soundData.wfex);
	assert(SUCCEEDED(result));

	XAUDIO2_BUFFER buf{};

	buf.pAudioData = soundData.pBUffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	result = pSourceVoice->SubmitSourceBuffer(&buf);
	result = pSourceVoice->Start();
}

#pragma endregion

#pragma region ダンプ
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception){

	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = {0};
	CreateDirectory(L"./Dumps",nullptr);
	StringCchPrintfW(filePath,MAX_PATH,L"./Dumps/%04d-%02d%02d-%02d%02d.dmp",
		time.wYear,time.wMonth,time.wDay,time.wHour,
		time.wMinute);
	HANDLE dumpFileHandle =
		CreateFile(filePath,GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ,0,CREATE_ALWAYS,0,0);

	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();

	MINIDUMP_EXCEPTION_INFORMATION minidumpinformation{0};
	minidumpinformation.ThreadId = threadId;
	minidumpinformation.ExceptionPointers = exception;
	minidumpinformation.ClientPointers = TRUE;

	MiniDumpWriteDump(GetCurrentProcess(),processId,dumpFileHandle,
		MiniDumpNormal,&minidumpinformation,nullptr,nullptr);

	return EXCEPTION_EXECUTE_HANDLER;
}
#pragma endregion

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE,LPSTR,int){
	D3DResourceLeakChecker leakCheck;

	WinAPI* winApi = nullptr;
	Input* input = nullptr;

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	SetUnhandledExceptionFilter(ExportDump);

	winApi = new WinAPI();
	winApi->Initialize();

	DXCommon* dxCommon = nullptr;
	dxCommon = new DXCommon();
	dxCommon->Initialize(winApi);

	input = new Input();
	input->Initialize(winApi);

	TextureManager::GetInstance()->Initialize(dxCommon);
	// モデルやスプライトで使うテクスチャの読み込み
	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resource/monsterball.png");

	// スプライト共通部分
	SpriteCommon* spriteCommon = nullptr;
	spriteCommon = new SpriteCommon;
	spriteCommon->Initialize(dxCommon);

	// スプライト生成
	Sprite* sprite = new Sprite();
	sprite->Initialize(spriteCommon,"resource/uvChecker.png");

	Sprite* spriteBall = new Sprite();
	spriteBall->Initialize(spriteCommon,"resource/monsterball.png");
	spriteBall->SetPosition({200.0f,0.0f});

	// ------------------------------------------------------------------
	// 3Dオブジェクトとモデルの初期化 (ここが今回のメイン！)
	// ------------------------------------------------------------------

	// 1. 3D描画共通部分 (RootSignature, PSOなど)
	Obj3dCommon* object3dCommon = nullptr;
	object3dCommon = new Obj3dCommon();
	object3dCommon->Initialize(dxCommon);

	// 2. モデル共通部分 (必要なら作成)
	ModelCommon* modelCommon = new ModelCommon();
	modelCommon->Initialize(dxCommon);

	// 3. モデルデータの生成・読み込み
	// (ここで plane.obj と関連するテクスチャが読み込まれる)
	Model* model = new Model();
	model->Initialize(modelCommon);

	// 4. 3Dオブジェクト(1つ目)の生成とモデルのセット
	Obj3D* object3d = new Obj3D();
	object3d->Initialize(object3dCommon);
	object3d->SetModel(model); // ★モデルをセット！
	object3d->SetTranslate({0.0f, 0.0f, 0.0f});
	object3d->SetScale({1.0f, 1.0f, 1.0f});

	// 5. 3Dオブジェクト(2つ目)の生成 (同じモデルを使い回す！)
	Obj3D* object3d_2 = new Obj3D();
	object3d_2->Initialize(object3dCommon);
	object3d_2->SetModel(model); // ★同じモデルをセット！
	object3d_2->SetTranslate({2.0f, 0.5f, 0.0f}); // 場所をずらす
	object3d_2->SetScale({0.5f, 0.5f, 0.5f});     // 大きさを変える

	// ------------------------------------------------------------------

	SoundData soundData = SoundLoadWave("resource/You_and_Me.wav");

	// XAudio2初期化
	HRESULT result = XAudio2Create(&xAudio2,0,XAUDIO2_DEFAULT_PROCESSOR);
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));


	while(true){

		if(winApi->ProcessMessage()){
			break;
		} else{
			// 更新処理
			input->Update();

			// 3Dオブジェクトの更新
			object3d->Update();
			object3d_2->Update(); // 2つ目も更新

			// スプライトの更新
			sprite->Update();
			spriteBall->Update();

			if(input->TriggerKey(DIK_SPACE)){
				OutputDebugStringA("Trigger space\n");
			}

			// 描画処理
			dxCommon->PreDraw();

			// 3Dモデル描画
			object3dCommon->Draw(); // 共通設定(RootSignatureなど)
			object3d->Draw();       // 1つ目の描画 (DrawCall)
			object3d_2->Draw();     // 2つ目の描画 (DrawCall)

			// スプライト描画
			spriteCommon->Draw();
			sprite->Draw();
			// spriteBall->Draw();

			dxCommon->PostDraw();
		}
	}

	Logger::Log("unkillable demon king\n");

	TextureManager::GetInstance()->Finalize();

	// 解放処理
	delete object3d;
	delete object3d_2;    // 追加分
	delete object3dCommon;

	delete model;         // モデルの解放
	delete modelCommon;   // モデル共通部の解放

	delete spriteBall;
	delete sprite;
	delete spriteCommon;

	delete dxCommon;
	delete input;

	xAudio2.Reset();
	SoundUnload(&soundData);

	winApi->Finalize();
	delete winApi;

	return 0;
}
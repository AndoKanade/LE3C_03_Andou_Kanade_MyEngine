#pragma once

// =========================================================
// インクルード
// =========================================================
// --- 基盤・マネージャ ---
#include "WinAPI.h"
#include "DXCommon.h"
#include "Input.h"
#include "SrvManager.h"
#include "ImGuiManager.h"
#include "SoundManager.h"
#include "CameraManager.h"
#include "ParticleManager.h"
#include "TextureManager.h"
#include "ModelManager.h"

// --- ゲームオブジェクト・共通 ---
#include "SpriteCommon.h"
#include "Obj3DCommon.h"
#include "Obj3D.h"
#include "Sprite.h"
#include "ParticleEmitter.h"

#include <string>

/// <summary>
/// アプリケーション全体の進行を管理するクラス
/// </summary>
class Application{
public: // --- 公開メソッド ---

	// コンストラクタ・デストラクタ
	Application();
	~Application();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ウィンドウメッセージの処理
	/// </summary>
	/// <returns>終了リクエストが来たらtrue</returns>
	bool ProcessMessage();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

private: // --- メンバ変数 ---

	// 基盤システム
	WinAPI* winApi_ = nullptr;
	DXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;

	// 描画共通設定
	SpriteCommon* spriteCommon_ = nullptr;
	Obj3dCommon* object3dCommon_ = nullptr;

	// --- ゲームオブジェクト (デモシーン用) ---
	Obj3D* planeObj_ = nullptr;
	Obj3D* fenceObj_ = nullptr;
	ParticleEmitter* particleEmitter_ = nullptr;

	// --- その他状態変数 ---
	bool isPaused_ = false;                                  // ポーズ状態
	const std::string kBgmPath_ = "resource/You_and_Me.mp3"; // BGMのファイルパス
};
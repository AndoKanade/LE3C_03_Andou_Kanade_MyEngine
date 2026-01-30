#pragma once
#include "Framework.h" // Frameworkを継承

// ゲーム固有のオブジェクト
#include "Obj3D.h"
#include "ParticleEmitter.h"

/// <summary>
/// ゲームの実装を行うクラス
/// </summary>
class Application : public Framework{
public:
	// コンストラクタ・デストラクタ
	Application();
	~Application();

	// オーバーライド (Frameworkの関数を上書き)
	void Initialize() override;
	void Finalize() override;
	void Update() override;
	void Draw() override;

private:
	// --- ゲーム固有のメンバ変数だけを残す ---
	Obj3D* planeObj_ = nullptr;
	Obj3D* fenceObj_ = nullptr;
	ParticleEmitter* particleEmitter_ = nullptr;

	bool isPaused_ = false;
	const std::string kBgmPath_ = "resource/You_and_Me.mp3";
};
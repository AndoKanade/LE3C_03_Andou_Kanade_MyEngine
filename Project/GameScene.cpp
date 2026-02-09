#include "GameScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "SoundManager.h"
#include "ParticleManager.h"
#include "CameraManager.h"
#include "ImGuiManager.h"

// コンストラクタ
GameScene::GameScene() = default;

// デストラクタ
GameScene::~GameScene(){}

// 終了処理
void GameScene::Finalize(){
	// shared_ptr / unique_ptr が自動解放してくれるので空でOK
}

// 初期化
void GameScene::Initialize(Obj3dCommon* object3dCommon,Input* input,SpriteCommon* spriteCommon){
	// メンバ変数に保存
	object3dCommon_ = object3dCommon;
	input_ = input;
	spriteCommon_ = spriteCommon;

	/// -------------------------------------------
	/// 1. リソースのロード
	/// -------------------------------------------
	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resource/monsterball.png");
	TextureManager::GetInstance()->LoadTexture("resource/Circle.png");

	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("fence.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");

	SoundManager::GetInstance()->SoundLoadFile(kBgmPath_);

	// パーティクルグループの作成
	ParticleManager::GetInstance()->CreateParticleGroup("Circle","resource/Circle.png");

	/// -------------------------------------------
	/// 2. オブジェクトの生成と初期化
	/// -------------------------------------------

	// ■ 親: 地面 (Plane)
	// ★変更: make_shared で生成 (weak_ptrで参照される側は shared_ptr 必須)
	planeObj_ = std::make_shared<Obj3D>();
	planeObj_->Initialize(object3dCommon_);
	planeObj_->SetModel("plane.obj");

	// ■ 子: 柵 (Fence)
	// ★変更: make_shared で生成
	fenceObj_ = std::make_shared<Obj3D>();
	fenceObj_->Initialize(object3dCommon_);
	fenceObj_->SetModel("fence.obj");

	// ★追加: 親子付け (EX課題)
	// これで Fence は Plane の動きについていくようになります
	fenceObj_->SetParent(planeObj_);

	// 座標設定 (親である Plane からの相対座標になります)
	fenceObj_->SetTranslate({2.0f, 0.0f, 0.0f});


	// パーティクルエミッタ (ここは親を持たないので unique_ptr のままでOK)
	Transform emitterConfig;
	emitterConfig.scale = {1.0f, 1.0f, 1.0f};
	particleEmitter_ = std::make_unique<ParticleEmitter>("Circle",emitterConfig,10,0.2f);

	/// -------------------------------------------
	/// 3. カメラの設定 (元のコードのまま)
	/// -------------------------------------------
	CameraManager::GetInstance()->CreateCamera("default");
	auto* defaultCamera = CameraManager::GetInstance()->GetCamera("default");
	defaultCamera->SetTranslate({0.0f, 0.0f, -10.0f});
	CameraManager::GetInstance()->SetActiveCamera("default");

	// 3Dオブジェクト共通設定にカメラをセット
	object3dCommon_->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
}

// 更新
void GameScene::Update(){
	// オブジェクトの更新
	if(planeObj_){ planeObj_->Update(); }
	if(fenceObj_){ fenceObj_->Update(); }

	// パーティクルの更新
	if(particleEmitter_){ particleEmitter_->Update(); }

	// BGM再生
	if(input_->TriggerKey(DIK_SPACE)){
		if(!SoundManager::GetInstance()->IsPlaying(kBgmPath_)){
			SoundManager::GetInstance()->PlayAudio(kBgmPath_,0.5f,true);
		}
	}

	// ImGui (デバッグ用)
#ifdef USE_IMGUI
	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();
	if(activeCamera){
		ImGui::Begin("GameScene Debug");

		// カメラ位置調整
		Vector3 translate = activeCamera->GetTranslate();
		ImGui::DragFloat3("Camera Pos",&translate.x,0.1f);
		activeCamera->SetTranslate(translate);

		// ★追加: 親(Plane)を動かして、親子関係を確認できるようにしておく
		if(planeObj_){
			Vector3 pPos = planeObj_->GetTranslate();
			ImGui::DragFloat3("Parent(Plane) Pos",&pPos.x,0.1f);
			planeObj_->SetTranslate(pPos);
		}

		ImGui::End();
	}
#endif
}

// 描画
void GameScene::Draw(){
	// オブジェクトの描画
	if(planeObj_){ planeObj_->Draw(); }
	if(fenceObj_){ fenceObj_->Draw(); }

	// パーティクルの描画が必要ならここに追加
}
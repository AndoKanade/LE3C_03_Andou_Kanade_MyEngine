#include "Application.h"
#include "Logger.h"

// コンストラクタ
Application::Application(){}

// デストラクタ
Application::~Application(){}

// 初期化処理
void Application::Initialize(){
	// ダンプ出力の設定
	SetUnhandledExceptionFilter(Logger::ExportDump);

	// ---------------------------------------------
	// 1. 基盤システム初期化
	// ---------------------------------------------
	winApi_ = new WinAPI();
	winApi_->Initialize();

	dxCommon_ = new DXCommon();
	dxCommon_->Initialize(winApi_);

	input_ = new Input();
	input_->Initialize(winApi_);

	// ---------------------------------------------
	// 2. マネージャ初期化
	// ---------------------------------------------
	SrvManager::GetInstance()->Initialize(dxCommon_);

#ifdef _DEBUG
	ImGuiManager::GetInstance()->Initialize(winApi_,dxCommon_);
#endif

	SoundManager::GetInstance()->Initialize();
	TextureManager::GetInstance()->Initialize(dxCommon_,SrvManager::GetInstance());
	ModelManager::GetInstance()->Initialize(dxCommon_);
	CameraManager::GetInstance()->Initialize();
	ParticleManager::GetInstance()->Initialize(dxCommon_,SrvManager::GetInstance());

	// ---------------------------------------------
	// 3. 描画共通システムの初期化
	// ---------------------------------------------
	spriteCommon_ = new SpriteCommon();
	// spriteCommon_->Initialize(dxCommon_); // 必要に応じて有効化

	object3dCommon_ = new Obj3dCommon();
	object3dCommon_->Initialize(dxCommon_);

	// ---------------------------------------------
	// 4. リソースロード
	// ---------------------------------------------
	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resource/monsterball.png");
	TextureManager::GetInstance()->LoadTexture("resource/Circle.png"); // パーティクル用

	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("fence.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");

	// パーティクルリソース
	ParticleManager::GetInstance()->CreateParticleGroup("Circle","resource/Circle.png");

	// サウンドロード (MP3/WAV)
	SoundManager::GetInstance()->SoundLoadFile(kBgmPath_);

	// ---------------------------------------------
	// 5. ゲームオブジェクト生成 (シーン構築)
	// ---------------------------------------------
	// 床
	planeObj_ = new Obj3D();
	planeObj_->Initialize(object3dCommon_);
	planeObj_->SetModel("plane.obj");
	planeObj_->SetScale({1.0f, 1.0f, 1.0f});

	// 柵
	fenceObj_ = new Obj3D();
	fenceObj_->Initialize(object3dCommon_);
	fenceObj_->SetModel("fence.obj");
	fenceObj_->SetTranslate({2.0f, 0.0f, 0.0f});

	// パーティクルエミッタ
	Transform emitterConfig;
	emitterConfig.scale = {1.0f, 1.0f, 1.0f};
	emitterConfig.translate = {0.0f, 0.0f, 0.0f};
	particleEmitter_ = new ParticleEmitter("Circle",emitterConfig,10,0.2f);

	// ---------------------------------------------
	// 6. カメラ設定
	// ---------------------------------------------
	CameraManager::GetInstance()->CreateCamera("default");
	auto* defaultCamera = CameraManager::GetInstance()->GetCamera("default");
	defaultCamera->SetTranslate({0.0f, 0.0f, -10.0f});
	CameraManager::GetInstance()->SetActiveCamera("default");

	// 3Dオブジェクト共通設定にカメラをセット
	object3dCommon_->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
}

// 更新処理
void Application::Update(){
	input_->Update();

#ifdef _DEBUG
	ImGuiManager::GetInstance()->Begin();
#endif

	// ---------------------------------------------
	// カメラ更新 (ImGuiによるデバッグ操作含む)
	// ---------------------------------------------
	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();
	if(activeCamera){
#ifdef _DEBUG
		Vector3 translate = activeCamera->GetTranslate();
		ImGui::Begin("Camera Control");
		ImGui::DragFloat3("Position",&translate.x,0.1f);
		ImGui::End();
		activeCamera->SetTranslate(translate);
#endif
	}
	CameraManager::GetInstance()->Update();
	object3dCommon_->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());

	// ---------------------------------------------
	// ゲームオブジェクト更新
	// ---------------------------------------------
	planeObj_->Update();
	// fenceObj_->Update();

	// ---------------------------------------------
	// 入力処理 (デバッグ用サウンド操作)
	// ---------------------------------------------
	if(input_->TriggerKey(DIK_SPACE)){
		if(!SoundManager::GetInstance()->IsPlaying(kBgmPath_)){
			SoundManager::GetInstance()->PlayAudio(kBgmPath_,0.5f,true);
		}
	}
	if(input_->TriggerKey(DIK_RETURN)){
		SoundManager::GetInstance()->StopAudio(kBgmPath_);
	}
	if(input_->TriggerKey(DIK_P)){
		if(isPaused_){
			SoundManager::GetInstance()->ResumeAudio(kBgmPath_);
			isPaused_ = false;
		} else{
			SoundManager::GetInstance()->PauseAudio(kBgmPath_);
			isPaused_ = true;
		}
	}

#ifdef _DEBUG
	ImGuiManager::GetInstance()->End();
#endif
}

// 描画処理
void Application::Draw(){
	// 描画前処理
	dxCommon_->PreDraw();
	SrvManager::GetInstance()->PreDraw();

	// ---------------------------------------------
	// 3Dオブジェクト描画
	// ---------------------------------------------
	object3dCommon_->Draw();
	planeObj_->Draw();
	// fenceObj_->Draw();

	// ---------------------------------------------
	// UI描画
	// ---------------------------------------------
#ifdef _DEBUG
	ImGuiManager::GetInstance()->Draw();
#endif

	// 描画後処理
	dxCommon_->PostDraw();
}

// 終了処理
void Application::Finalize(){
	// 1. ゲームオブジェクト解放
	delete particleEmitter_;
	delete planeObj_;
	delete fenceObj_;
	delete object3dCommon_;
	delete spriteCommon_;

	// 2. マネージャ解放
#ifdef _DEBUG
	ImGuiManager::GetInstance()->Finalize();
#endif
	SoundManager::GetInstance()->Finalize();
	ParticleManager::GetInstance()->Finalize();
	ModelManager::GetInstance()->Finalize();
	TextureManager::GetInstance()->Finalize();
	CameraManager::GetInstance()->Finalize();
	SrvManager::GetInstance()->Finalize();

	// 3. 基盤システム解放
	delete input_;
	delete dxCommon_;
	delete winApi_;
}

// ウィンドウメッセージ処理
bool Application::ProcessMessage(){
	return winApi_->ProcessMessage();
}
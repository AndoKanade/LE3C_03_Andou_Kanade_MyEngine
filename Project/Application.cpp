#include "Application.h"

Application::Application(){}
Application::~Application(){}

void Application::Initialize(){
	// ★重要：まず親(Framework)の初期化を呼ぶ
	Framework::Initialize();

	// --- ここからはゲーム固有の処理 ---

	// リソースロード
	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");
	TextureManager::GetInstance()->LoadTexture("resource/monsterball.png");
	TextureManager::GetInstance()->LoadTexture("resource/Circle.png");
	ModelManager::GetInstance()->LoadModel("plane.obj");
	ModelManager::GetInstance()->LoadModel("fence.obj");
	ModelManager::GetInstance()->LoadModel("sphere.obj");

	ParticleManager::GetInstance()->CreateParticleGroup("Circle","resource/Circle.png");
	SoundManager::GetInstance()->SoundLoadFile(kBgmPath_);

	// オブジェクト生成
	planeObj_ = new Obj3D();
	planeObj_->Initialize(object3dCommon_);
	planeObj_->SetModel("plane.obj");

	fenceObj_ = new Obj3D();
	fenceObj_->Initialize(object3dCommon_);
	fenceObj_->SetModel("fence.obj");
	fenceObj_->SetTranslate({2.0f, 0.0f, 0.0f});

	// パーティクル
	Transform emitterConfig;
	emitterConfig.scale = {1.0f, 1.0f, 1.0f};
	particleEmitter_ = new ParticleEmitter("Circle",emitterConfig,10,0.2f);

	// カメラ
	CameraManager::GetInstance()->CreateCamera("default");
	auto* defaultCamera = CameraManager::GetInstance()->GetCamera("default");
	defaultCamera->SetTranslate({0.0f, 0.0f, -10.0f});
	CameraManager::GetInstance()->SetActiveCamera("default");

	object3dCommon_->SetDefaultCamera(CameraManager::GetInstance()->GetActiveCamera());
}

void Application::Update(){
	// ★親の更新処理を呼ぶ (入力更新などはここで行われる)
	Framework::Update();

	// ゲーム固有の更新
	planeObj_->Update();

	// 入力処理 (input_ は親クラスにあるのでそのまま使える)
	if(input_->TriggerKey(DIK_SPACE)){
		if(!SoundManager::GetInstance()->IsPlaying(kBgmPath_)){
			SoundManager::GetInstance()->PlayAudio(kBgmPath_,0.5f,true);
		}
	}

	if(input_->TriggerKey(DIK_RETURN)){
		SoundManager::GetInstance()->StopAudio(kBgmPath_);
	}

#ifdef _DEBUG
	// 例：カメラの座標をいじるウィンドウ
	Camera* activeCamera = CameraManager::GetInstance()->GetActiveCamera();
	if(activeCamera){
		// 1. ウィンドウを作る
		ImGui::Begin("Camera Control");

		// 2. カメラの今の座標を取ってくる
		Vector3 translate = activeCamera->GetTranslate();

		// 3. スライダーを表示して値をいじらせる
		//    (ラベル名, 変数のアドレス, 感度)
		ImGui::DragFloat3("Position",&translate.x,0.1f);

		// 4. ウィンドウを閉じる
		ImGui::End();

		// いじった値をカメラにセットし直す
		activeCamera->SetTranslate(translate);
	}
#endif

#ifdef _DEBUG
	ImGuiManager::GetInstance()->End(); // FrameworkでBeginしてるのでEndが必要
#endif
}

void Application::Draw(){
	// 描画開始
	dxCommon_->PreDraw();
	SrvManager::GetInstance()->PreDraw();

	// 描画コマンド
	object3dCommon_->Draw();
	planeObj_->Draw();

#ifdef _DEBUG
	ImGuiManager::GetInstance()->Draw();
#endif

	// 描画終了
	dxCommon_->PostDraw();
}

void Application::Finalize(){
	// ゲーム固有の解放
	delete particleEmitter_;
	delete planeObj_;
	delete fenceObj_;

	// ★最後に親の終了処理を呼ぶ
	Framework::Finalize();
}
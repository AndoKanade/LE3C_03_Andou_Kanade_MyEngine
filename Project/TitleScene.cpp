#include "TitleScene.h"
#include "SceneManager.h" 
#include "ModelManager.h"
#include "Input.h"
#include "Sprite.h"
#include "SpriteCommon.h"
#include"TextureManager.h"

// デストラクタ
TitleScene::~TitleScene(){
	Finalize();
}

// 終了処理
void TitleScene::Finalize(){
	// メモリ解放 (delete後は nullptr で安全策)
	if(titleObject_){
		delete titleObject_;
		titleObject_ = nullptr;
	}
	if(sprite_){
		delete sprite_;
		sprite_ = nullptr;
	}
}

// 初期化
void TitleScene::Initialize(Obj3dCommon* object3dCommon,Input* input,SpriteCommon* spriteCommon){
	object3dCommon_ = object3dCommon;
	input_ = input;
	spriteCommon_ = spriteCommon;

	// --- リソースのロード ---
	ModelManager::GetInstance()->LoadModel("fence.obj");
	TextureManager::GetInstance()->LoadTexture("resource/uvChecker.png");

	// ★追加: スプライト生成と初期設定
	sprite_ = new Sprite();
	sprite_->Initialize(spriteCommon_,"resource/uvChecker.png");

	// これがないと見えない可能性があります！
	sprite_->SetPosition({0.0f, 0.0f});       // 左上に配置
	sprite_->SetSize({300.0f, 300.0f});       // 見える大きさに設定
	sprite_->SetColor({1.0f, 1.0f, 1.0f, 1.0f}); // 真っ白（不透明）

	// --- オブジェクト生成 ---
	titleObject_ = new Obj3D();
	titleObject_->Initialize(object3dCommon_);
	titleObject_->SetModel("fence.obj");

	// 座標とサイズの設定
	titleObject_->SetTranslate({0.0f, 0.0f, 0.0f});
	titleObject_->SetScale({0.5f, 0.5f, 0.5f});
}

// 更新
void TitleScene::Update(){

	// --- ImGuiの処理 ---
	ImGui::Begin("Sprite Settings"); // ウィンドウの名前

	// 色と透明度をまとめて変更できる便利な機能
	// &spriteColor_.x は、Vector4の先頭アドレス(x)を渡すことで、x,y,z,w(rgba)をまとめて操作できます
	ImGui::ColorEdit4("Color & Alpha",&spriteColor_.x);

	ImGui::End();

	// オブジェクトの更新
	if(titleObject_){
		titleObject_->Update();
	}

	// ★追加: スプライトの更新（これを忘れると描画されません！）
	if(sprite_){
		sprite_->SetColor(spriteColor_);
		sprite_->Update();
	}

	// シーン遷移処理 (スペースキー)
	if(input_->TriggerKey(DIK_SPACE)){
		// 3. マネージャーに切り替え依頼
		SceneManager::GetInstance()->ChangeScene("GAME");
	}
}

// 描画
void TitleScene::Draw(){
	if(titleObject_){
		titleObject_->Draw();
	}

	if(spriteCommon_ && sprite_){
		spriteCommon_->Draw(); // 共通設定をコマンドリストに積む(PreDraw的な役割)

		sprite_->Draw();       // スプライト自身の描画コマンドを積む
	}
}
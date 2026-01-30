#include "Application.h"
#include "D3DResourceLeakChecker.h"

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	D3DResourceLeakChecker leakCheck;
	Framework* game = new Application();

	// 初期化
	game->Initialize();

	// ゲームループ
	while(true){
		if(game->IsEndRequest()){
			break;
		}

		// 更新と描画
		game->Update();
		game->Draw();
	}

	// 終了処理
	game->Finalize();

	delete game;
	return 0;
}
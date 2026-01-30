#include "Application.h"
#include "D3DResourceLeakChecker.h"

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int){
	D3DResourceLeakChecker leakCheck;
	Application* app = new Application();

	app->Initialize();

	while(true){
		if(app->ProcessMessage()){
			break;
		}

		// 更新処理
		app->Update();

		// 描画処理
		app->Draw();
	}

	// 終了処理
	app->Finalize();

	delete app;
	return 0;
}
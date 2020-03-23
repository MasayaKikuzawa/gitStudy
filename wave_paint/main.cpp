/*-----------------------------------------------------------------------------
	ピクチャーボックスへの描画プログラム（サンプル2）

	所々に
	/*******
	(↓に記述)
	********
	という箇所があるのでそこを穴埋めしていく


  　「OK」ボタンが押されるとピクチャーボックス内にウィンドウを作成し，円などを描画．
  　ラジオボタンを選択することで円内の色が変化する．

-----------------------------------------------------------------------------*/
#include <windows.h>		//Windows環境
#include <windowsx.h>		//Windows環境
#include <process.h>		//スレッド用
#include<iostream>
#include<fstream>
#include<string>
#include "header.h"			//ヘッダーファイル
#include "resource.h"		//リソースファイル
using namespace std;

//定数宣言
#define DEF_APP_NAME	TEXT("Waveform Test")
#define DEF_MUTEX_NAME	DEF_APP_NAME			//ミューテックス名


//構造体
typedef struct send{
	HWND	hEdit;
	HWND	hwnd;//ウィンドウハンドル
}SEND_POINTER_STRUCT;

HWND name[2];
streampos pos[2];

//======================================
//ここからダイアログバージョン

//メイン関数(ダイアログバージョン)
//ここは特にいじらなくてOK
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HANDLE hMutex;		

	//多重起動判定
	hMutex = CreateMutex(NULL, TRUE, DEF_MUTEX_NAME);		//ミューテックスオブジェクトの生成
	if(GetLastError() == ERROR_ALREADY_EXISTS){				//2重起動の有無を確認
		MessageBox(NULL, TEXT("既に起動されています．"), NULL, MB_OK|MB_ICONERROR);
		return 0;											//終了
	}

	//ダイアログ起動
	DialogBox(hInstance, MAKEINTRESOURCE(WAVE), NULL, MainDlgProc);

	return FALSE;			//終了
}

//編集するのはここから

//メインプロシージャ（ダイアログ）
/********************************

システム画面（.rcファイル）に配置したボタン等が押されたときや，
初期化，終了時にどのような処理を行うかをここに記述する．

********************************/
BOOL CALLBACK MainDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hPict[2];		//大枠ウィンドウハンドル（PictureBox）
	static HWND hWnd[2];		//子ウィンドウハンドル（正弦波用と矩形波用）
	static HWND st;//開始ボタンのハンドルを獲得する変数
	static HWND ed;//終了ボタンのハンドルを獲得する変数

	static HFONT hFont;				//フォント
	static HANDLE hThread[2];//各子ウィンドウのスレッドを管理する変数
	static UINT thID[2];//各スレッドのハンドルを格納する変数
	static SEND_POINTER_STRUCT Sps[2];

	switch(uMsg){
	case WM_INITDIALOG:		//ダイアログ初期化(exeをダブルクリックした時)
		
		st = GetDlgItem(hDlg, START1);//開始ボタンのハンドルを取得
		ed = GetDlgItem(hDlg,END1);//停止ボタンのハンドルを取得
				return TRUE;

	case WM_COMMAND:		//ボタンが押された時
		/*******
		case WM_COMMAND:では各ボタンが押されたときの処理を記載します．
	   ********/
		switch(LOWORD(wParam)){
		case START1: //開始ボタン
			/***********************
			開始ボタンが押されたときに描画を開始します．
			描画を開始するためにはPictureControlのハンドルを取得する必要があります
			↑を参考にハンドル hPictをGetDlgItem関数を用いて取得しましょう．
			***********************/
			//ここから

			hPict[0] = GetDlgItem(hDlg, WINDOW1);//波形描画領域(上)のハンドルを取得する
			hPict[1] = GetDlgItem(hDlg, WINDOW2);//波形描画領域(下)のハンドルを取得する

			//ここまで	
			WinInitialize(NULL, hDlg, (HMENU)110, "WINDOW1", hPict[0], WndProc, &hWnd[0]); //正弦波用ウィンドウの初期化
			WinInitialize(NULL, hDlg, (HMENU)110, "WINDOW2", hPict[1], WndProc, &hWnd[1]); //矩形波用ウィンドウの初期化
			//WinInitialize関数によって子ウィンドウプロシージャにhPictがhWndとしてセットされました．

			name[0] = hWnd[0];
			name[1] = hWnd[1];

			Sps[0].hwnd = hWnd[0];//正弦波用の画面の構造体（内部はウィンドウハンドルが格納されているだけ）
			Sps[1].hwnd = hWnd[1];//矩形波の画面の構造体（内部はウィンドウハンドルが格納されているだけ）
			
			//スレッドを立てる

			hThread[0] = (HANDLE)_beginthreadex(NULL, 0, TFunc, (PVOID)& Sps[0], 0, &thID[0]);   //正弦波スレッド
			hThread[1] = (HANDLE)_beginthreadex(NULL, 0, TFunc, (PVOID)& Sps[1], 0, &thID[1]);   //矩形波スレッド

			EnableWindow(GetDlgItem(hDlg, START1), FALSE);						//開始ボタン無効化　　　　//EnableWindowで入力を無効または有効にする。

			return TRUE;
		case STOP1:	//停止・再開ボタン

						/*　サスペンドカウンタ　**************************
						　　　実行を許可するまでスレッドを動かさない。
						   　　ResumeThread：　サスペンドカウンタを1減らす
							 　SuspendThread：　サスペンドカウンタを1増やす

							  0のときは実行。それ以外は待機する。
							  **************************************************/


			if (ResumeThread(hThread[0]) == 0 || ResumeThread(hThread[1]) == 0) {					//停止中かを調べる(サスペンドカウントを１減らす)
				SetDlgItemText(hDlg, STOP1, TEXT("再開"));	//再開に変更　　　　　　　　　　　　　　　　　　　//SetDlgItemTextでダイアログ内のテキストなどを変更することができる
				SuspendThread(hThread[0]);						//スレッドの実行を停止(サスペンドカウントを１増やす)
				SuspendThread(hThread[1]);
			}
			else
				SetDlgItemText(hDlg, STOP1, TEXT("停止"));	//停止に変更

			return TRUE;
		//ここまで
		}
		break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);			//ダイアログ終了
        return TRUE;
    }

	

	//オーナー描画後に再描画
	if (uMsg==WM_PAINT){
		InvalidateRect(hWnd[0], NULL, TRUE );	//再描画
		InvalidateRect(hWnd[1], NULL, TRUE);	//再描画
	}

	return FALSE;
}

/********************************

ここでPictureControlの描画を行います．

子ウィンドウプロシージャ中のhWndはPictureControlのハンドルです．

********************************/
//子ウィンドウプロシージャ
HRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	HDC			hdc;				//デバイスコンテキストのハンドル
	PAINTSTRUCT ps;					//(構造体)クライアント領域描画するための情報
	RECT rect;
	HBRUSH		hBrush, hOldBrush;	//ブラシ
	HPEN		hPen, hOldPen, hPen2;		//ペン
	

	
	static int width, height;
	
	switch(uMsg){
	case WM_CREATE:	
	

		color = RGB(0, 0, 0);
		break;

	case WM_PAINT:
		
		//ここでは波形の描画を行う
		hdc = BeginPaint(hWnd, &ps);//デバイスコンテキストを取得


		//ペン，ブラシ生成
		hBrush = CreateSolidBrush(color);				//ブラシ生成
		hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);	//ブラシ設定
		colorPen = RGB(255, 255, 255);
		hPen = CreatePen(PS_SOLID, 2, colorPen);		//ペン生成(軸用)
		hPen2 = CreatePen(PS_SOLID, 2, RGB(255,0,0));	//グラフ描画用
		hOldPen = (HPEN)SelectObject(hdc, hPen);		//ペン設定


		SetBkColor(hdc, RGB(0, 0, 0));//背景色を決める
		SetTextColor(hdc, RGB(255, 255, 255));

		
		static int cnt = 0;
		if (cnt < 2)
		{
			GetClientRect(hWnd, &rect);//現在のクライアントのサイズを取得
			width = rect.right;//幅
			height = rect.bottom;//高さ

			Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

			MoveToEx(hdc, 0, height / 2, NULL);//横軸
			LineTo(hdc, width, height / 2);

			MoveToEx(hdc, width / 8, 0, NULL);//縦軸
			LineTo(hdc, width / 8, height);
			TextOut(hdc, width / 2, height * 6 / 8, TEXT("Times[s]"), 8);
			cnt++;
		}
		
	
		ifstream ifs("data.txt");
		string str;


		if (ifs.fail()) {
			cerr << "File do not exist.\n";
			exit(0);
		}

		double data1, data2;
		static double old_position_y[2] = { height / 2,height / 2 };
		static double old_position_x[2] = { width / 8,width / 8 };

		if (hWnd == name[0])
		{
			double tmp = 0;
			ifs.seekg(pos[0]);
			getline(ifs, str);
	

			sscanf(str.data(), "%lf %lf", &data1, &data2);
			pos[0] = ifs.tellg();

			
			tmp = -data1 * ((double)height / 2 - 2) + (double)(height-1) / 2;


			colorPen = RGB(255, 255, 255);
			hOldPen = (HPEN)SelectObject(hdc, hPen);		//ペン設定

			if (old_position_x[0] + 1 > width)
			{
				hOldPen = (HPEN)SelectObject(hdc, hPen);		//ペン設定
				GetClientRect(hWnd, &rect);//現在のクライアントのサイズを取得
				width = rect.right;//幅
				height = rect.bottom;//高さ
				SetTextColor(hdc, RGB(255, 255, 255));
				
				old_position_x[0] = width / 8;
				old_position_y[0] = tmp;

				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

				MoveToEx(hdc, 0, height / 2, NULL);//横軸
				LineTo(hdc, width, height / 2);

				MoveToEx(hdc, width / 8, 0, NULL);//縦軸
				LineTo(hdc, width / 8, height);
				TextOut(hdc, width / 2, height * 6 / 8, TEXT("Times[s]"), 8);
			}
			else
			{
				hOldPen = (HPEN)SelectObject(hdc, hPen2);		//ペン設定
				MoveToEx(hdc, old_position_x[0], old_position_y[0], NULL);
				LineTo(hdc, old_position_x[0] + 1, tmp);
				old_position_x[0] += 1;
				old_position_y[0] = tmp;
			}

		
		}
		else if(hWnd == name[1])
		{
			double tmp = 0;
			ifs.seekg(pos[1]);
			getline(ifs, str);


			sscanf(str.data(), "%lf %lf", &data1, &data2);
			pos[1] = ifs.tellg();

			SetTextColor(hdc, RGB(0, 0, 0));

			tmp = -data2 * ((double)height / 2 - 2) + (double)(height-1)/ 2;

			if (old_position_x[1] + 1 > width)
			{

				GetClientRect(hWnd, &rect);//現在のクライアントのサイズを取得
				width = rect.right;//幅
				height = rect.bottom;//高さ
				SetTextColor(hdc, RGB(255, 255, 255));
				colorPen = RGB(255, 255, 255);

				old_position_x[1] = width / 8;
				old_position_y[1] = tmp;

				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

				MoveToEx(hdc, 0, height / 2, NULL);//横軸
				LineTo(hdc, width, height / 2);

				MoveToEx(hdc, width / 8, 0, NULL);//縦軸
				LineTo(hdc, width / 8, height);
				TextOut(hdc, width / 2, height * 6 / 8, TEXT("Times[s]"), 8);
			}
			else
			{
				hOldPen = (HPEN)SelectObject(hdc, hPen2);		//ペン設定
				MoveToEx(hdc, old_position_x[1], old_position_y[1], NULL);
				LineTo(hdc, old_position_x[1] + 1, tmp);
				old_position_x[1] += 1;
				old_position_y[1] = tmp;
			}
		}

		EndPaint(hWnd, &ps);//デバイスコンテキスト解放
		//ペン，ブラシ廃棄
		/********************************

		使い終わったペンとブラシは破棄する必要があります．

		********************************/
		SelectObject(hdc, hOldBrush);
		DeleteObject(hBrush);
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);

		break;
	}

	return TRUE;
}

//ここまで

//-----------------------------------------------------------------------------
//子ウィンドウ初期化＆生成
//指定したウィンドウハンドルの領域に子ウィンドウを生成する．
//----------------------------------------------------------
// hInst	: 生成用インスタンスハンドル
// hPaWnd	: 親ウィンドウのハンドル
// chID		: 子ウィンドウのID
// cWinName	: 子ウィンドウ名
// PaintArea: 子ウィンドウを生成する領域のデバイスハンドル
// WndProc	: ウィンドウプロシージャ
// *hWnd	: 子ウィンドウのハンドル（ポインタ）
// 戻り値	: 成功時=true
//-----------------------------------------------------------------------------
BOOL WinInitialize( HINSTANCE hInst, HWND hPaWnd, HMENU chID, char *cWinName, HWND PaintArea, WNDPROC WndProc ,HWND *hWnd)
{
	WNDCLASS wc;			//ウィンドウクラス
	WINDOWPLACEMENT	wplace;	//子ウィンドウ生成領域計算用（画面上のウィンドウの配置情報を格納する構造体）
	RECT WinRect;			//子ウィンドウ生成領域
	ATOM atom;				//アトム

	//ウィンドウクラス初期化
	wc.style		=CS_HREDRAW ^ WS_MAXIMIZEBOX | CS_VREDRAW;	//ウインドウスタイル
	wc.lpfnWndProc	=WndProc;									//ウインドウのメッセージを処理するコールバック関数へのポインタ
	wc.cbClsExtra	=0;											//
	wc.cbWndExtra	=0;
	wc.hCursor		=NULL;										//プログラムのハンドル
	wc.hIcon		=NULL;										//アイコンのハンドル
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);		//ウインドウ背景色
	wc.hInstance	=hInst;										//ウインドウプロシージャがあるインスタンスハンドル
	wc.lpszMenuName	=NULL;										//メニュー名
	wc.lpszClassName=(LPCTSTR)cWinName;									//ウインドウクラス名

	if(!(atom=RegisterClass(&wc))){
		MessageBox(hPaWnd,TEXT("ウィンドウクラスの生成に失敗しました．"),NULL,MB_OK|MB_ICONERROR);
		return false;
	}

	GetWindowPlacement(PaintArea,&wplace);	//描画領域ハンドルの情報を取得(ウィンドウの表示状態を取得)
	WinRect=wplace.rcNormalPosition;		//描画領域の設定

	//ウィンドウ生成
	*hWnd=CreateWindow(
		(LPCTSTR)atom,
		(LPCTSTR)cWinName,
		WS_CHILD | WS_VISIBLE,//| WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME |WS_VISIBLE, 
		WinRect.left,WinRect.top,
		WinRect.right-WinRect.left,WinRect.bottom-WinRect.top,
		hPaWnd,chID,hInst,NULL
	);

	if( *hWnd==NULL ){
		MessageBox(hPaWnd,TEXT("ウィンドウの生成に失敗しました．"),NULL,MB_OK|MB_ICONERROR);
		return false;
	}

	return true;
}


UINT WINAPI TFunc(LPVOID thParam) {

	//ここにはファイルデータの書き込みを行う
	SEND_POINTER_STRUCT* FU = (SEND_POINTER_STRUCT*)thParam;        //構造体のポインタ取得

	for (int i = 0; i < 3000; i++)
	{
		InvalidateRect(FU-> hwnd, NULL, TRUE);	//再描画
		Sleep(1000/61.5);
	}



	return 0;
}
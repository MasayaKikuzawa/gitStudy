//-------------------------------------------------------------------------
//	main.cppファイルのヘッダーファイル
//	
//	関数，変数宣言用
//-------------------------------------------------------------------------

//関数宣言
BOOL CALLBACK MainDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );	//メインダイアログプロシージャ
BOOL WinInitialize( HINSTANCE hInst, HWND hPaWnd, HMENU chID, char *cWinName, HWND PaintArea, WNDPROC WndProc ,HWND *hDC);//子ウィンドウを生成
HRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );		//子ウィンドウプロシージャ
UINT WINAPI TFunc(LPVOID thParam);


static COLORREF	color, colorPen;	//色
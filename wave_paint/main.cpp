/*-----------------------------------------------------------------------------
	�s�N�`���[�{�b�N�X�ւ̕`��v���O�����i�T���v��2�j

	���X��
	/*******
	(���ɋL�q)
	********
	�Ƃ����ӏ�������̂ł����������߂��Ă���


  �@�uOK�v�{�^�����������ƃs�N�`���[�{�b�N�X���ɃE�B���h�E���쐬���C�~�Ȃǂ�`��D
  �@���W�I�{�^����I�����邱�Ƃŉ~���̐F���ω�����D

-----------------------------------------------------------------------------*/
#include <windows.h>		//Windows��
#include <windowsx.h>		//Windows��
#include <process.h>		//�X���b�h�p
#include<iostream>
#include<fstream>
#include<string>
#include "header.h"			//�w�b�_�[�t�@�C��
#include "resource.h"		//���\�[�X�t�@�C��
using namespace std;

//�萔�錾
#define DEF_APP_NAME	TEXT("Waveform Test")
#define DEF_MUTEX_NAME	DEF_APP_NAME			//�~���[�e�b�N�X��


//�\����
typedef struct send{
	HWND	hEdit;
	HWND	hwnd;//�E�B���h�E�n���h��
}SEND_POINTER_STRUCT;

HWND name[2];
streampos pos[2];

//======================================
//��������_�C�A���O�o�[�W����

//���C���֐�(�_�C�A���O�o�[�W����)
//�����͓��ɂ�����Ȃ���OK
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	HANDLE hMutex;		

	//���d�N������
	hMutex = CreateMutex(NULL, TRUE, DEF_MUTEX_NAME);		//�~���[�e�b�N�X�I�u�W�F�N�g�̐���
	if(GetLastError() == ERROR_ALREADY_EXISTS){				//2�d�N���̗L�����m�F
		MessageBox(NULL, TEXT("���ɋN������Ă��܂��D"), NULL, MB_OK|MB_ICONERROR);
		return 0;											//�I��
	}

	//�_�C�A���O�N��
	DialogBox(hInstance, MAKEINTRESOURCE(WAVE), NULL, MainDlgProc);

	return FALSE;			//�I��
}

//�ҏW����̂͂�������

//���C���v���V�[�W���i�_�C�A���O�j
/********************************

�V�X�e����ʁi.rc�t�@�C���j�ɔz�u�����{�^�����������ꂽ�Ƃ���C
�������C�I�����ɂǂ̂悤�ȏ������s�����������ɋL�q����D

********************************/
BOOL CALLBACK MainDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static HWND hPict[2];		//��g�E�B���h�E�n���h���iPictureBox�j
	static HWND hWnd[2];		//�q�E�B���h�E�n���h���i�����g�p�Ƌ�`�g�p�j
	static HWND st;//�J�n�{�^���̃n���h�����l������ϐ�
	static HWND ed;//�I���{�^���̃n���h�����l������ϐ�

	static HFONT hFont;				//�t�H���g
	static HANDLE hThread[2];//�e�q�E�B���h�E�̃X���b�h���Ǘ�����ϐ�
	static UINT thID[2];//�e�X���b�h�̃n���h�����i�[����ϐ�
	static SEND_POINTER_STRUCT Sps[2];

	switch(uMsg){
	case WM_INITDIALOG:		//�_�C�A���O������(exe���_�u���N���b�N������)
		
		st = GetDlgItem(hDlg, START1);//�J�n�{�^���̃n���h�����擾
		ed = GetDlgItem(hDlg,END1);//��~�{�^���̃n���h�����擾
				return TRUE;

	case WM_COMMAND:		//�{�^���������ꂽ��
		/*******
		case WM_COMMAND:�ł͊e�{�^���������ꂽ�Ƃ��̏������L�ڂ��܂��D
	   ********/
		switch(LOWORD(wParam)){
		case START1: //�J�n�{�^��
			/***********************
			�J�n�{�^���������ꂽ�Ƃ��ɕ`����J�n���܂��D
			�`����J�n���邽�߂ɂ�PictureControl�̃n���h�����擾����K�v������܂�
			�����Q�l�Ƀn���h�� hPict��GetDlgItem�֐���p���Ď擾���܂��傤�D
			***********************/
			//��������

			hPict[0] = GetDlgItem(hDlg, WINDOW1);//�g�`�`��̈�(��)�̃n���h�����擾����
			hPict[1] = GetDlgItem(hDlg, WINDOW2);//�g�`�`��̈�(��)�̃n���h�����擾����

			//�����܂�	
			WinInitialize(NULL, hDlg, (HMENU)110, "WINDOW1", hPict[0], WndProc, &hWnd[0]); //�����g�p�E�B���h�E�̏�����
			WinInitialize(NULL, hDlg, (HMENU)110, "WINDOW2", hPict[1], WndProc, &hWnd[1]); //��`�g�p�E�B���h�E�̏�����
			//WinInitialize�֐��ɂ���Ďq�E�B���h�E�v���V�[�W����hPict��hWnd�Ƃ��ăZ�b�g����܂����D

			name[0] = hWnd[0];
			name[1] = hWnd[1];

			Sps[0].hwnd = hWnd[0];//�����g�p�̉�ʂ̍\���́i�����̓E�B���h�E�n���h�����i�[����Ă��邾���j
			Sps[1].hwnd = hWnd[1];//��`�g�̉�ʂ̍\���́i�����̓E�B���h�E�n���h�����i�[����Ă��邾���j
			
			//�X���b�h�𗧂Ă�

			hThread[0] = (HANDLE)_beginthreadex(NULL, 0, TFunc, (PVOID)& Sps[0], 0, &thID[0]);   //�����g�X���b�h
			hThread[1] = (HANDLE)_beginthreadex(NULL, 0, TFunc, (PVOID)& Sps[1], 0, &thID[1]);   //��`�g�X���b�h

			EnableWindow(GetDlgItem(hDlg, START1), FALSE);						//�J�n�{�^���������@�@�@�@//EnableWindow�œ��͂𖳌��܂��͗L���ɂ���B

			return TRUE;
		case STOP1:	//��~�E�ĊJ�{�^��

						/*�@�T�X�y���h�J�E���^�@**************************
						�@�@�@���s��������܂ŃX���b�h�𓮂����Ȃ��B
						   �@�@ResumeThread�F�@�T�X�y���h�J�E���^��1���炷
							 �@SuspendThread�F�@�T�X�y���h�J�E���^��1���₷

							  0�̂Ƃ��͎��s�B����ȊO�͑ҋ@����B
							  **************************************************/


			if (ResumeThread(hThread[0]) == 0 || ResumeThread(hThread[1]) == 0) {					//��~�����𒲂ׂ�(�T�X�y���h�J�E���g���P���炷)
				SetDlgItemText(hDlg, STOP1, TEXT("�ĊJ"));	//�ĊJ�ɕύX�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@//SetDlgItemText�Ń_�C�A���O���̃e�L�X�g�Ȃǂ�ύX���邱�Ƃ��ł���
				SuspendThread(hThread[0]);						//�X���b�h�̎��s���~(�T�X�y���h�J�E���g���P���₷)
				SuspendThread(hThread[1]);
			}
			else
				SetDlgItemText(hDlg, STOP1, TEXT("��~"));	//��~�ɕύX

			return TRUE;
		//�����܂�
		}
		break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);			//�_�C�A���O�I��
        return TRUE;
    }

	

	//�I�[�i�[�`���ɍĕ`��
	if (uMsg==WM_PAINT){
		InvalidateRect(hWnd[0], NULL, TRUE );	//�ĕ`��
		InvalidateRect(hWnd[1], NULL, TRUE);	//�ĕ`��
	}

	return FALSE;
}

/********************************

������PictureControl�̕`����s���܂��D

�q�E�B���h�E�v���V�[�W������hWnd��PictureControl�̃n���h���ł��D

********************************/
//�q�E�B���h�E�v���V�[�W��
HRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{

	HDC			hdc;				//�f�o�C�X�R���e�L�X�g�̃n���h��
	PAINTSTRUCT ps;					//(�\����)�N���C�A���g�̈�`�悷�邽�߂̏��
	RECT rect;
	HBRUSH		hBrush, hOldBrush;	//�u���V
	HPEN		hPen, hOldPen, hPen2;		//�y��
	

	
	static int width, height;
	
	switch(uMsg){
	case WM_CREATE:	
	

		color = RGB(0, 0, 0);
		break;

	case WM_PAINT:
		
		//�����ł͔g�`�̕`����s��
		hdc = BeginPaint(hWnd, &ps);//�f�o�C�X�R���e�L�X�g���擾


		//�y���C�u���V����
		hBrush = CreateSolidBrush(color);				//�u���V����
		hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);	//�u���V�ݒ�
		colorPen = RGB(255, 255, 255);
		hPen = CreatePen(PS_SOLID, 2, colorPen);		//�y������(���p)
		hPen2 = CreatePen(PS_SOLID, 2, RGB(255,0,0));	//�O���t�`��p
		hOldPen = (HPEN)SelectObject(hdc, hPen);		//�y���ݒ�


		SetBkColor(hdc, RGB(0, 0, 0));//�w�i�F�����߂�
		SetTextColor(hdc, RGB(255, 255, 255));

		
		static int cnt = 0;
		if (cnt < 2)
		{
			GetClientRect(hWnd, &rect);//���݂̃N���C�A���g�̃T�C�Y���擾
			width = rect.right;//��
			height = rect.bottom;//����

			Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

			MoveToEx(hdc, 0, height / 2, NULL);//����
			LineTo(hdc, width, height / 2);

			MoveToEx(hdc, width / 8, 0, NULL);//�c��
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
			hOldPen = (HPEN)SelectObject(hdc, hPen);		//�y���ݒ�

			if (old_position_x[0] + 1 > width)
			{
				hOldPen = (HPEN)SelectObject(hdc, hPen);		//�y���ݒ�
				GetClientRect(hWnd, &rect);//���݂̃N���C�A���g�̃T�C�Y���擾
				width = rect.right;//��
				height = rect.bottom;//����
				SetTextColor(hdc, RGB(255, 255, 255));
				
				old_position_x[0] = width / 8;
				old_position_y[0] = tmp;

				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

				MoveToEx(hdc, 0, height / 2, NULL);//����
				LineTo(hdc, width, height / 2);

				MoveToEx(hdc, width / 8, 0, NULL);//�c��
				LineTo(hdc, width / 8, height);
				TextOut(hdc, width / 2, height * 6 / 8, TEXT("Times[s]"), 8);
			}
			else
			{
				hOldPen = (HPEN)SelectObject(hdc, hPen2);		//�y���ݒ�
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

				GetClientRect(hWnd, &rect);//���݂̃N���C�A���g�̃T�C�Y���擾
				width = rect.right;//��
				height = rect.bottom;//����
				SetTextColor(hdc, RGB(255, 255, 255));
				colorPen = RGB(255, 255, 255);

				old_position_x[1] = width / 8;
				old_position_y[1] = tmp;

				Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

				MoveToEx(hdc, 0, height / 2, NULL);//����
				LineTo(hdc, width, height / 2);

				MoveToEx(hdc, width / 8, 0, NULL);//�c��
				LineTo(hdc, width / 8, height);
				TextOut(hdc, width / 2, height * 6 / 8, TEXT("Times[s]"), 8);
			}
			else
			{
				hOldPen = (HPEN)SelectObject(hdc, hPen2);		//�y���ݒ�
				MoveToEx(hdc, old_position_x[1], old_position_y[1], NULL);
				LineTo(hdc, old_position_x[1] + 1, tmp);
				old_position_x[1] += 1;
				old_position_y[1] = tmp;
			}
		}

		EndPaint(hWnd, &ps);//�f�o�C�X�R���e�L�X�g���
		//�y���C�u���V�p��
		/********************************

		�g���I������y���ƃu���V�͔j������K�v������܂��D

		********************************/
		SelectObject(hdc, hOldBrush);
		DeleteObject(hBrush);
		SelectObject(hdc, hOldPen);
		DeleteObject(hPen);

		break;
	}

	return TRUE;
}

//�����܂�

//-----------------------------------------------------------------------------
//�q�E�B���h�E������������
//�w�肵���E�B���h�E�n���h���̗̈�Ɏq�E�B���h�E�𐶐�����D
//----------------------------------------------------------
// hInst	: �����p�C���X�^���X�n���h��
// hPaWnd	: �e�E�B���h�E�̃n���h��
// chID		: �q�E�B���h�E��ID
// cWinName	: �q�E�B���h�E��
// PaintArea: �q�E�B���h�E�𐶐�����̈�̃f�o�C�X�n���h��
// WndProc	: �E�B���h�E�v���V�[�W��
// *hWnd	: �q�E�B���h�E�̃n���h���i�|�C���^�j
// �߂�l	: ������=true
//-----------------------------------------------------------------------------
BOOL WinInitialize( HINSTANCE hInst, HWND hPaWnd, HMENU chID, char *cWinName, HWND PaintArea, WNDPROC WndProc ,HWND *hWnd)
{
	WNDCLASS wc;			//�E�B���h�E�N���X
	WINDOWPLACEMENT	wplace;	//�q�E�B���h�E�����̈�v�Z�p�i��ʏ�̃E�B���h�E�̔z�u�����i�[����\���́j
	RECT WinRect;			//�q�E�B���h�E�����̈�
	ATOM atom;				//�A�g��

	//�E�B���h�E�N���X������
	wc.style		=CS_HREDRAW ^ WS_MAXIMIZEBOX | CS_VREDRAW;	//�E�C���h�E�X�^�C��
	wc.lpfnWndProc	=WndProc;									//�E�C���h�E�̃��b�Z�[�W����������R�[���o�b�N�֐��ւ̃|�C���^
	wc.cbClsExtra	=0;											//
	wc.cbWndExtra	=0;
	wc.hCursor		=NULL;										//�v���O�����̃n���h��
	wc.hIcon		=NULL;										//�A�C�R���̃n���h��
	wc.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);		//�E�C���h�E�w�i�F
	wc.hInstance	=hInst;										//�E�C���h�E�v���V�[�W��������C���X�^���X�n���h��
	wc.lpszMenuName	=NULL;										//���j���[��
	wc.lpszClassName=(LPCTSTR)cWinName;									//�E�C���h�E�N���X��

	if(!(atom=RegisterClass(&wc))){
		MessageBox(hPaWnd,TEXT("�E�B���h�E�N���X�̐����Ɏ��s���܂����D"),NULL,MB_OK|MB_ICONERROR);
		return false;
	}

	GetWindowPlacement(PaintArea,&wplace);	//�`��̈�n���h���̏����擾(�E�B���h�E�̕\����Ԃ��擾)
	WinRect=wplace.rcNormalPosition;		//�`��̈�̐ݒ�

	//�E�B���h�E����
	*hWnd=CreateWindow(
		(LPCTSTR)atom,
		(LPCTSTR)cWinName,
		WS_CHILD | WS_VISIBLE,//| WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME |WS_VISIBLE, 
		WinRect.left,WinRect.top,
		WinRect.right-WinRect.left,WinRect.bottom-WinRect.top,
		hPaWnd,chID,hInst,NULL
	);

	if( *hWnd==NULL ){
		MessageBox(hPaWnd,TEXT("�E�B���h�E�̐����Ɏ��s���܂����D"),NULL,MB_OK|MB_ICONERROR);
		return false;
	}

	return true;
}


UINT WINAPI TFunc(LPVOID thParam) {

	//�����ɂ̓t�@�C���f�[�^�̏������݂��s��
	SEND_POINTER_STRUCT* FU = (SEND_POINTER_STRUCT*)thParam;        //�\���̂̃|�C���^�擾

	for (int i = 0; i < 3000; i++)
	{
		InvalidateRect(FU-> hwnd, NULL, TRUE);	//�ĕ`��
		Sleep(1000/61.5);
	}



	return 0;
}
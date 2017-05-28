// CaptureClient.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "CaptureClient.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND	  hwndDlg;
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

HINSTANCE hScreenCapture;
int(*runFunc)(wchar_t*, int, HWND*) = NULL;

// 이 코드 모듈에 들어 있는 함수의 정방향 선언입니다.
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CAPTURECLIENT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 응용 프로그램 초기화를 수행합니다.
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CAPTURECLIENT));

    MSG msg;

    // 기본 메시지 루프입니다.
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  함수: MyRegisterClass()
//
//  목적: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CAPTURECLIENT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CAPTURECLIENT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   목적: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   설명:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // ShowWindow(hWnd, nCmdShow);
   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   hScreenCapture = LoadLibrary(_T("ScreenCapture.dll"));
   if (hScreenCapture == NULL) {
	   return FALSE;
   }

   runFunc = (int(*)(wchar_t*, int, HWND*))GetProcAddress(hScreenCapture, "ScreenShot");
   if (runFunc == NULL) {
	   return FALSE;
   }

   return TRUE;
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적:  주 창의 메시지를 처리합니다.
//
//  WM_COMMAND  - 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT    - 주 창을 그립니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//

NOTIFYICONDATA data;
HMENU hMenu, hPopupMenu;
BOOL checkURL = TRUE;
BOOL checkClip = FALSE;
#define ID_TRAYICON_NOTIFY (WM_APP+100)

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_INITMENU:
		if (checkURL)
			CheckMenuItem((HMENU)wParam, ID_MEVENT_URL, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem((HMENU)wParam, ID_MEVENT_URL, MF_BYCOMMAND | MF_UNCHECKED);

		if (checkURL) {
			checkClip = FALSE;
			EnableMenuItem((HMENU)wParam, ID_MEVENT_CLIPBOARD, MF_BYCOMMAND | MF_GRAYED);
		}
		else
			EnableMenuItem((HMENU)wParam, ID_MEVENT_CLIPBOARD, MF_BYCOMMAND | MF_ENABLED);

		if (checkClip)
			CheckMenuItem((HMENU)wParam, ID_MEVENT_CLIPBOARD, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem((HMENU)wParam, ID_MEVENT_CLIPBOARD, MF_BYCOMMAND | MF_UNCHECKED);

		if (checkClip) {
			checkURL = FALSE;
			EnableMenuItem((HMENU)wParam, ID_MEVENT_URL, MF_BYCOMMAND | MF_GRAYED);
		}
		else
			EnableMenuItem((HMENU)wParam, ID_MEVENT_URL, MF_BYCOMMAND | MF_ENABLED);
		return 0;

	case WM_CREATE:
		ZeroMemory(&data, sizeof(data));
		data.cbSize = sizeof(NOTIFYICONDATA);
		data.hWnd = hWnd;
		data.uID = 0;
		data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		data.uCallbackMessage = ID_TRAYICON_NOTIFY;
		data.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_CAPTURECLIENT)); // LoadIcon(NULL, IDI_APPLICATION);
		lstrcpy(data.szTip, TEXT("Capture Program"));
		Shell_NotifyIcon(NIM_ADD, &data);

		SetTimer(hWnd, 0, 1, NULL);
		break;

	case ID_TRAYICON_NOTIFY:
		switch (lParam) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN: {
				POINT pos;

				hMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MEVENT));
				hPopupMenu = GetSubMenu(hMenu, 0);
				GetCursorPos(&pos);
				SetForegroundWindow(hWnd);
				TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON |
					TPM_RIGHTBUTTON, pos.x, pos.y, 0, hWnd, NULL);
				SetForegroundWindow(hWnd);
				DestroyMenu(hPopupMenu);
				DestroyMenu(hMenu);
			}
			break;
		}
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 메뉴 선택을 구문 분석합니다.
            switch (wmId)
            {
            case IDM_ABOUT:
				//runFunc(TEXT("."));
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

			// Mouse Menu Event
			// MenuID: LOWORD(wParam)
			case ID_MEVENT_URL:
				checkURL = !checkURL;
				if (hwndDlg != NULL) {
					EndDialog(hwndDlg, 0);
				}
				break;
			case ID_MEVENT_CLIPBOARD:
				checkClip = !checkClip;
				if (hwndDlg != NULL) {
					EndDialog(hwndDlg, 0);
				}
				break;
			case ID_MEVENT_HELP:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case ID_MEVENT_DESCRIPT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DESCRIPT), hWnd, About);
				break;
			case ID_MEVENT_EXIT:
				DestroyWindow(hWnd);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

	case WM_TIMER:
		switch (wParam) {
		case 0: {
				static int run_flag = 0;

				if (run_flag == 0) {
					int type = 0;
					run_flag = 1;
					if (checkURL) {
						type |= 0x01;
					}
					if (checkClip) {
						type |= 0x02;
					}
					hwndDlg = NULL;
					run_flag = runFunc(TEXT("."), type, &hwndDlg);
				}
				break;
			}
		}
		break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다.
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		// destroy TrayIcon
		data.cbSize = sizeof(NOTIFYICONDATA);
		data.hWnd = hWnd;
		data.uID = 0;
		Shell_NotifyIcon(NIM_DELETE, &data);

        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// 정보 대화 상자의 메시지 처리기입니다.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

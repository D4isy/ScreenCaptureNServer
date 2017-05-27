// ScreenCapture.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ScreenCapture.h"

#define TIMER_KEYDOWN 1

HWND *_hwndDlg = NULL;
BOOL isCustomPictureKey = FALSE;
BOOL isSelectPictureKey = FALSE;
BOOL isWindowPictureKey = FALSE;
BOOL isFirstInit = FALSE;

UINT pictureTick = 250;

VOID				CommandFinished(HWND hDlg)
{
	SendMessage(GetParent(hDlg), WM_FINISHED, 0, 0);
}

VOID				CommandDrawRect(HWND hDlg)
{
	SendMessage(GetParent(hDlg), WM_DRAWRECT, 0, 0);
}

VOID				CommandDrawLine(HWND hDlg)
{
	SendMessage(GetParent(hDlg), WM_DRAWLINE, 0, 0);
}

VOID				CommandDrawText(HWND hDlg)
{
	SendMessage(GetParent(hDlg), WM_DRAWTEXT, 0, 0);
}

VOID HotKeyDefaultSet(BOOL isType)
{
	isCustomPictureKey = isType;
	isSelectPictureKey = isType;
	isWindowPictureKey = isType;
	isFirstInit = isType;
}

VOID InitScreenMemory(VOID)
{
	OutputDebugString(L"[Init] release resource\n");
	SelectObject(hdcMem, (HBITMAP)NULL);
	SelectObject(hdcTemp, (HBITMAP)NULL);
	SelectObject(hdcFinish, (HBITMAP)NULL);
	SelectObject(hdcPaint, (HBITMAP)NULL);
	SelectObject(hdcPaint, (HPEN)NULL);
	SAFE_RELEASE(hBmpPaint);
	SAFE_RELEASE(hBitmap);
	SAFE_RELEASE(hBmpTemp);
	SAFE_RELEASE(hBmpFinish);
	SAFE_RELEASE(hdcPaint);
	SAFE_RELEASE(hdcMem);
	SAFE_RELEASE(hdcTemp);
	SAFE_RELEASE(hdcFinish);

	HDC hdcDisplay = CreateDC(L"DISPLAY", NULL, NULL, NULL);

	// È¸»ö È­¸éÀÌ ¶ß°ÔÇÒ ÀÌ¹ÌÁö ¸®¼Ò½º ·Îµå
	ImageFromResource(IDB_LAYER, L"PNG", &pImage);
	hdcMem = CreateCompatibleDC(hdcDisplay);

	// À©µµ¿ì Å©±â ¼³Á¤
	RECT rcShow = { 0, 0, nWidth, nHeight };
	hBitmap = CreateCompatibleBitmap(&rcShow);

	SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdcDisplay, 0, 0, SRCCOPY);

	if (isWindowPictureKey) {
		HDC hdcFound = GetDC(g_hwndFoundWindow);
		BitBlt(hdcMem, ptBegin.x, ptBegin.y, ptEnd.x - ptBegin.x, ptEnd.y - ptBegin.y, hdcFound, 0, 0, SRCCOPY);
	}

	DeleteDC(hdcDisplay);

	// ÀÓ½Ã ÆäÀÎÆ®ÇÒ ¸Þ¸ð¸®°ø°£ ÇÒ
	hdcTemp = CreateCompatibleDC(hdcMem);
	hdcFinish = CreateCompatibleDC(hdcMem);
	hdcPaint = CreateCompatibleDC(hdcMem);

	hBmpTemp = CreateCompatibleBitmap(&rcShow);
	SelectObject(hdcTemp, hBmpTemp);

	hBmpFinish = CreateCompatibleBitmap(&rcShow);
	SelectObject(hdcFinish, hBmpFinish);

	hBmpPaint = CreateCompatibleBitmap(&rcShow);
	SelectObject(hdcPaint, hBmpPaint);

	BitBlt(hdcTemp, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcFinish, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcPaint, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);

	// ÆäÀÎÆ® »ö±ò ¼³Á¤
	hPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
	SelectObject(hdcPaint, hPen);
}

BOOL CheckWindowValidity(HWND hwndDialog, HWND hwndToCheck)
{
	HWND hwndTemp = NULL;
	BOOL bRet = TRUE;

	// The window must not be NULL.
	if (hwndToCheck == NULL)
	{
		bRet = FALSE;
		goto CheckWindowValidity_0;
	}

	// It must also be a valid window as far as the OS is concerned.
	if (IsWindow(hwndToCheck) == FALSE)
	{
		bRet = FALSE;
		goto CheckWindowValidity_0;
	}

	// It also must not be the "Search Window" dialog box itself.
	if (hwndToCheck == hwndDialog)
	{
		bRet = FALSE;
		goto CheckWindowValidity_0;
	}

	// It also must not be one of the dialog box's children...
	hwndTemp = GetParent(hwndToCheck);
	// if ((hwndTemp == hwndDialog) || (hwndTemp == g_hwndMainWnd))
	if (hwndTemp == hwndDialog)
	{
		bRet = FALSE;
		goto CheckWindowValidity_0;
	}

CheckWindowValidity_0:

	return bRet;
}

HWND GetParentWindowFromPoint(POINT p, HWND hDlg)
{
	HWND hWnd = WindowFromPoint(p);	// ÇØ´ç ÁÂÇ¥ÀÇ À©µµ¿ì HWND ¸¦ ±¸ÇÔ
									// HWND hWnd = MyWindowFromPoint(p, hDlg);
	HWND hParent = NULL;

	while (1)
	{
		hParent = GetParent(hWnd);
		if (hParent == NULL) break;
		hWnd = hParent;
	}

	return hWnd;
}

VOID SelectWindow(HWND hWnd)
{
	POINT screenpoint;
	HWND hwndFoundWindow;

	GetCursorPos(&screenpoint);
	hwndFoundWindow = GetParentWindowFromPoint(screenpoint, hWnd);

	if (CheckWindowValidity(hWnd, hwndFoundWindow))
	{
		POINT top, left;
		RECT rect, rect2;

		GetClientRect(hwndFoundWindow, &rect2);
		GetWindowRect(hwndFoundWindow, &rect);
		ptEnd.x = rect.right;
		ptEnd.y = rect.bottom;
		ptBegin.x = rect.left;
		ptBegin.y = rect.top;

		top.x = rect.left;
		top.y = rect.top;

		left.x = rect.right;
		left.y = rect.bottom;
		ScreenToClient(hwndFoundWindow, &top);
		ScreenToClient(hwndFoundWindow, &left);

		ptBegin.x = ptBegin.x - top.x;
		ptBegin.y = ptBegin.y;// -top.y;
		ptEnd.x = ptEnd.x - (left.x - rect2.right);
		ptEnd.y = ptEnd.y - (left.y - rect2.bottom);
	}
}

BOOL GetHotKeyUse(VOID)
{
	if (isCustomPictureKey) {
		return TRUE;
	}
	else if (isSelectPictureKey) {
		return TRUE;
	}
	else if (isWindowPictureKey) {
		return TRUE;
	}

	return FALSE;
}

INT_PTR CALLBACK CommandBarProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{

	case WM_INITDIALOG:
	{
		if (btnList.GetButtonCount() == 0)
		{

			CImageButton btnDrawRect(1, 0, 1, 24, 24, IDB_DRH, IDB_DRN, IDB_DRD, _T("PNG"));
			btnDrawRect.RegisterEventHandle(CommandDrawRect);
			CImageButton btnDrawText(2, 25, 1, 24, 24, IDB_DTH, IDB_DTN, IDB_DTD, _T("PNG"));
			btnDrawText.RegisterEventHandle(CommandDrawText);
			CImageButton btnDrawLine(3, 50, 1, 24, 24, IDB_DLH, IDB_DLN, IDB_DLD, _T("PNG"));
			btnDrawLine.RegisterEventHandle(CommandDrawLine);
			CImageButton btnFinished(4, 75, 1, 54, 24, IDB_FH, IDB_FN, IDB_FD, _T("PNG"));
			btnFinished.RegisterEventHandle(CommandFinished);

			btnList.AddButton(btnFinished);
			btnList.AddButton(btnDrawRect);
			btnList.AddButton(btnDrawText);
			btnList.AddButton(btnDrawLine);
		}
		return (INT_PTR)TRUE;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg, &ps);

		btnList.DispatchEvent(hDlg, message, wParam, lParam);

		EndPaint(hDlg, &ps);
	}
	break;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	{
		btnList.DispatchEvent(hDlg, message, wParam, lParam);
	}
	break;
	default:
		return FALSE;
	}
	return (INT_PTR)TRUE;
}

INT_PTR CALLBACK CaptureProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{

	case WM_INITDIALOG:
	{
		*_hwndDlg = hDlg;
		// DialogBox ¼û±â±â
		SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_HIDEWINDOW);
		SetTimer(hDlg, TIMER_KEYDOWN, 1, NULL);
		return TRUE;
	}
	break;
	case WM_KEYUP:
	{
		if (isFirstInit) {
			switch (wParam) {
			case VK_ESCAPE:
				// EndDialog(hDlg, 0);
				break;
			}
		}
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (isFirstInit) {
			if (GetHotKeyUse()) {
				AbortCapture(hDlg);
			}
		}
	}
	break;
	case WM_PAINT:
	{
		if (isFirstInit) {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hDlg, &ps);

			Draw(hDlg, hdc);

			EndPaint(hDlg, &ps);
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (isFirstInit) {
			if (isWindowPictureKey && !bSelected) {
				UINT tmpParam = ptEnd.x | (ptEnd.y << 16);
				bDown = true;
				OnLButtonUp(hDlg, wParam, tmpParam);
			}
			else if (isCustomPictureKey || isWindowPictureKey) {
				OnLButtonDown(hDlg, wParam, lParam);
			}
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (isFirstInit) {
			if (isCustomPictureKey || (isWindowPictureKey && bSelected)) {
				OnLButtonUp(hDlg, wParam, lParam);
			}
		}
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (isFirstInit) {
			if (isWindowPictureKey && !bSelected) {
				static UINT timetick = 0;
				POINT screenpoint = { LOWORD(lParam), HIWORD(lParam) };
				RECT rt;

				rt.left = ptBegin.x;
				rt.top = ptBegin.y;
				rt.right = ptEnd.x;
				rt.bottom = ptEnd.y;

				if (!PtInRect(&rt, screenpoint) || (rt.left == 0 && rt.top == 0 && rt.right == nWidth && rt.bottom == nHeight)) {
					if (timetick + pictureTick < GetTickCount()) {
						ShowWindow(hDlg, SW_MINIMIZE);
						SelectWindow(hDlg);
						ShowWindow(hDlg, SW_MAXIMIZE);

						HDC hdc = GetDC(hDlg);
						Draw(hDlg, hdc);
						ReleaseDC(hDlg, hdc);
						InvalidateRect(hDlg, NULL, FALSE);
						timetick = GetTickCount();
					}
				}
			}
			else if (isCustomPictureKey || isWindowPictureKey) {
				OnMouseMove(hDlg, wParam, lParam);
			}
		}
	}
	break;
	case WM_LBUTTONDBLCLK:
	case WM_FINISHED:
	{
		if (isFirstInit) {
			if (IsWindowEnabled(hWndEdit))
			{
				DrawText(hDlg, wParam, lParam);
			}
			OnDoubleClick(hDlg, wParam, lParam);
		}
	}
	break;
	case WM_DRAWLINE:
	{
		if (isFirstInit) {
			if (IsWindowEnabled(hWndEdit))
			{
				DrawText(hDlg, wParam, lParam);
			}
			bDrawText = false;
			bDrawRect = false;
			bDrawLine = true;
			bDown = false;
		}
	}

	break;
	case WM_DRAWRECT:
	{
		if (isFirstInit) {
			if (IsWindowEnabled(hWndEdit))
			{
				DrawText(hDlg, wParam, lParam);
			}
			bDrawLine = false;
			bDrawText = false;
			bDrawRect = true;
			bDown = false;
		}
	}
	break;
	case WM_DRAWTEXT:
	{
		if (isFirstInit) {
			bDrawRect = false;
			bDrawText = true;
			bDrawLine = false;
			bDown = false;
		}
	}
	break;
	case WM_TIMER:
	{
		if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_CONTROL) & 0x8000) && GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
			AbortCapture(hDlg);
		}
		if (GetHotKeyUse() && IsWindowVisible(hDlg) && !IsIconic(hDlg)) {
			if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
				AbortCapture(hDlg);
			}
		}
		else if (!GetHotKeyUse() && IsWindowVisible(hDlg) && !IsIconic(hDlg)) {
			if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(0x32) & 0x8000)) { // 2¹ø Å° ´­¸²
				if (!isCustomPictureKey) {
					if (!isFirstInit) {
						isFirstInit = TRUE;
						InitScreenMemory();
					}
					ShowWindow(hDlg, SW_SHOW);
					isCustomPictureKey = TRUE;
					Graphics *pGraphic = new Graphics(hdcTemp);

					if (pImage != NULL)
					{
						// È¸»öÈ­¸é
						pGraphic->DrawImage(pImage, 0, 0, nWidth, nHeight);
					}

					delete pGraphic;
					InitDialog(hDlg);
					InvalidateRect(hDlg, NULL, FALSE);
				}
			}
			else if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(0x33) & 0x8000)) { // 3¹ø Å° ´­¸²
				if (!isSelectPictureKey) {
					if (!isFirstInit) {
						isFirstInit = TRUE;
						InitScreenMemory();
					}
					ShowWindow(hDlg, SW_SHOW);
					InitDialog(hDlg);

					ptBegin.x = 0;
					ptBegin.y = 0;
					ptEnd.x = nWidth;
					ptEnd.y = nHeight;

					InvalidateRect(hDlg, NULL, FALSE);
					OnDoubleClick(hDlg, NULL, NULL);
				}
			}
			else if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(0x34) & 0x8000)) { // 4¹ø Å° ´­¸²
				if (!isWindowPictureKey) {
					if (!isFirstInit) {
						isFirstInit = TRUE;
						InitScreenMemory();
					}
					ShowWindow(hDlg, SW_SHOW);

					isWindowPictureKey = TRUE;
					Graphics *pGraphic = new Graphics(hdcTemp);

					if (pImage != NULL)
					{
						// È¸»öÈ­¸éÀÌ ¶ß°Ô¸¸µå´Â ¿äÀÎ!
						pGraphic->DrawImage(pImage, 0, 0, nWidth, nHeight);
					}

					delete pGraphic;
					InitDialog(hDlg);
					InvalidateRect(hDlg, NULL, FALSE);
				}
			}
		}
	}
	break;
	case WM_DESTROY:
	{
		if (hWndCommandBar != NULL)
		{
			DestroyWindow(hWndCommandBar);
		}
	}

	break;
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;

}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

BOOL SaveHDCToFile(HWND hDlg, HDC hDC, LPRECT lpRect)
{
	BOOL bRet = FALSE;
	int nWidth = lpRect->right - lpRect->left;
	int nHeight = lpRect->bottom - lpRect->top;

	HDC memDC = CreateCompatibleDC(hDC);

	HBITMAP hBmp = CreateCompatibleBitmap(hDC, nWidth, nHeight);

	SelectObject(memDC, hBmp);

	BitBlt(memDC, 0, 0, nWidth, nHeight,
		hDC, lpRect->left, lpRect->top, SRCCOPY);

	CLSID pngClsid;
	GetEncoderClsid(L"image/png", &pngClsid);

	Bitmap *pbmSrc = Bitmap::FromHBITMAP(hBmp, NULL);

	SYSTEMTIME st;
	GetLocalTime(&st);
	srand(GetTickCount());

	wchar_t wszPathTemp[MAX_PATH] = { 0 };
	char path[MAX_PATH] = { 0, };
	int rnd = rand();
	sprintf(path, "%d%02d%02d%02d%02d%02d%d%d.png",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds,
		rnd);
	wsprintf(wszPathTemp, L"%s/%d%02d%02d%02d%02d%02d%d%d.png",
		wszFilePath,
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond,
		st.wMilliseconds,
		rnd);
	OutputDebugStringW(wszPathTemp);

	if (pbmSrc->Save(wszPathTemp, &pngClsid) == Ok)
	{
		bRet = TRUE;
	}

	if (bClip) {
		if (OpenClipboard(hDlg)) {
			HBITMAP hBitmap_copy = CreateBitmap(nWidth, nHeight, 1, 32, NULL);
			HDC newDC = CreateCompatibleDC(hDC);
			HBITMAP newBitmap = (HBITMAP)SelectObject(newDC, hBitmap_copy);

			BitBlt(newDC, 0, 0, nWidth, nHeight, memDC, 0, 0, SRCCOPY);

			SelectObject(newDC, newBitmap);
			DeleteDC(newDC);

			EmptyClipboard();
			SetClipboardData(CF_BITMAP, hBitmap_copy);
			CloseClipboard();
		}
	}

	delete pbmSrc;

	SelectObject(memDC, (HBITMAP)NULL);
	DeleteDC(memDC);
	DeleteObject(hBmp);

	if (bUrl) {
		socket_t servSock;

		FILE* fp;
		int  urlLen = 0;
		char fBuffer[BUF_SIZE];
		// ÆÄÀÏ Å©±â ±¸ÇÏ±â
		int fileSize, readSize, readTotalSize;

		const char IP[] = "220.149.14.83";
		int port = 443;

		servSock = creat_client_socket(IP, port);
		if (servSock == SOCKET_ERROR) { return bRet; }

		fp = fopen(path, "rb");
		if (fp == NULL) {
			puts("fopen() error!");
		}

		// fileÀÇ Å©±â °è»ê 
		fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		readTotalSize = 0;

		send(servSock, (char*)&fileSize, sizeof(int), 0);
		
		while (fileSize > readTotalSize) {
			memset(fBuffer, 0x00, BUF_SIZE);
			readSize = fread(fBuffer, sizeof(char), BUF_SIZE, fp);
			if (send(servSock, fBuffer, readSize, 0) == SOCKET_ERROR) {
				break;
			}
			readTotalSize = readTotalSize + readSize;
		}

		fclose(fp);

		memset(fBuffer, 0x00, BUF_SIZE);
		recv(servSock, (char*)&urlLen, sizeof(int), 0);
		recv(servSock, fBuffer, urlLen, 0);
		closesocket(servSock);

		{
			HANDLE hData = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, urlLen + 1);
			char *pData = (char*)GlobalLock(hData);
			if (pData != NULL) {
				memcpy(pData, fBuffer, urlLen + 1);
				GlobalUnlock(hData);
				if (OpenClipboard(hDlg)) {
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hData);
					CloseClipboard();
				}
			}
		}
	}

	return bRet;
}

void ThreadProc(void *param)
{
	// MessageBox(NULL, L"Ä¸ÃÄ ÈÄ ÀÌ ¸Þ½ÃÁö°¡ ¶å´Ï´Ù. ", L"°øÁö", MB_OK | MB_TOPMOST);
	Sleep(500);
	SetEvent(hEvent);
}

extern "C" EXPORT_API int ScreenShot(wchar_t* wszPath, int type, HWND *hwndDlg)
{
	if (wszPath == NULL)
	{
		return -1;
	}

	if (type & 0x01) {
		bUrl = true;
	}
	if (type & 0x02) {
		bClip = true;
	}
	wcscpy(wszFilePath, wszPath);

	bSelected = false;
	bDrawText = false;
	bDrawRect = false;
	hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	ResetEvent(hEvent);

	_beginthread(ThreadProc, 0, NULL);

	MSG msg;
	while (WaitForSingleObject(hEvent, 0) != WAIT_OBJECT_0)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Sleep(100);
		}
	}

	GdiplusStartup(&gdiplusGdiToken, &gdiplusStartupInput, NULL);

	nWidth = GetSystemMetrics(SM_CXSCREEN);
	nHeight = GetSystemMetrics(SM_CYSCREEN);

	ptBegin.x = 0;
	ptBegin.y = 0;

	ptEnd.x = 0;
	ptEnd.y = 0;

	HDC hdcDisplay = CreateDC(L"DISPLAY", NULL, NULL, NULL);

	// È¸»ö È­¸éÀÌ ¶ß°ÔÇÒ ÀÌ¹ÌÁö ¸®¼Ò½º ·Îµå
	ImageFromResource(IDB_LAYER, L"PNG", &pImage);
	hdcMem = CreateCompatibleDC(hdcDisplay);

	// À©µµ¿ì Å©±â ¼³Á¤
	RECT rcShow = { 0, 0, nWidth, nHeight };
	hBitmap = CreateCompatibleBitmap(&rcShow);

	SelectObject(hdcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, nWidth, nHeight, hdcDisplay, 0, 0, SRCCOPY);

	DeleteDC(hdcDisplay);

	// ÀÓ½Ã ÆäÀÎÆ®ÇÒ ¸Þ¸ð¸®°ø°£ ÇÒ
	hdcTemp = CreateCompatibleDC(hdcMem);
	hdcFinish = CreateCompatibleDC(hdcMem);
	hdcPaint = CreateCompatibleDC(hdcMem);

	hBmpTemp = CreateCompatibleBitmap(&rcShow);
	SelectObject(hdcTemp, hBmpTemp);

	hBmpFinish = CreateCompatibleBitmap(&rcShow);
	SelectObject(hdcFinish, hBmpFinish);

	hBmpPaint = CreateCompatibleBitmap(&rcShow);
	SelectObject(hdcPaint, hBmpPaint);

	BitBlt(hdcTemp, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcFinish, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcPaint, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);

	//Graphics* pGraphic = new Graphics(hdcTemp);

	//if(pImage != NULL)
	//{
	//	// È¸»öÈ­¸éÀÌ ¶ß°Ô¸¸µå´Â ¿äÀÎ!
	//	pGraphic->DrawImage(pImage, 0, 0, nWidth, nHeight);
	//}

	//delete pGraphic;

	// ÆäÀÎÆ® »ö±ò ¼³Á¤
	hPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
	SelectObject(hdcPaint, hPen);

	// ÀÓ½Ã Å×½ºÆ®
	// MoveToEx(hdcMem, 0, 0, NULL);
	// LineTo(hdcMem, nWidth, nHeight);

	// À©µµ¿ì ÇÔ¼ö È£Ãâ!
	_hwndDlg = hwndDlg;
	DialogBox(hInst, MAKEINTRESOURCE(IDD_SHOW), NULL, CaptureProc);

	// ¸Þ¸ð¸® ÇØÁ¦
	OutputDebugString(L"release resource\n");
	SelectObject(hdcMem, (HBITMAP)NULL);
	SelectObject(hdcTemp, (HBITMAP)NULL);
	SelectObject(hdcFinish, (HBITMAP)NULL);
	SelectObject(hdcPaint, (HBITMAP)NULL);
	SelectObject(hdcPaint, (HPEN)NULL);
	SAFE_RELEASE(hBmpPaint);
	SAFE_RELEASE(hBitmap);
	SAFE_RELEASE(hBmpTemp);
	SAFE_RELEASE(hBmpFinish);
	SAFE_RELEASE(hdcPaint);
	SAFE_RELEASE(hdcMem);
	SAFE_RELEASE(hdcTemp);
	SAFE_RELEASE(hdcFinish);
	SAFE_RELEASE(hPen);

	delete pImage;

	GdiplusShutdown(gdiplusGdiToken);
	return 0;
}

HBITMAP	CreateCompatibleBitmap(RECT* rcClient)
{
	BYTE                      * pBits;
	BITMAPINFOHEADER          bmih;
	ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));

	bmih.biSize = sizeof(BITMAPINFOHEADER);
	bmih.biWidth = rcClient->right - rcClient->left;
	bmih.biHeight = rcClient->bottom - rcClient->top;
	bmih.biPlanes = 1;
	bmih.biBitCount = 32;
	bmih.biCompression = BI_RGB;
	bmih.biSizeImage = 0;
	bmih.biXPelsPerMeter = 0;
	bmih.biYPelsPerMeter = 0;
	bmih.biClrUsed = 0;
	bmih.biClrImportant = 0;

	HBITMAP hBitMap = CreateDIBSection(NULL, (BITMAPINFO *)&bmih, 0, (VOID**)&pBits, NULL, 0);
	return hBitMap;
}

VOID ImageFromResource(UINT uID, LPCTSTR pszResType, Image** ppImage)
{
	//HINSTANCE hInst =  GetModuleHandle(L"ScreenCapture.dll");
	HRSRC hRsrc = FindResource(hInst, MAKEINTRESOURCE(uID), pszResType);
	if (!hRsrc) {
		return;
	}

	HGLOBAL hMemRsrc = LoadResource(hInst, hRsrc);
	BYTE * pMemRsrc = (BYTE *)LockResource(hMemRsrc);

	size_t sizeRsrc = SizeofResource(hInst, hRsrc);
	HGLOBAL hMemStrm = GlobalAlloc(GMEM_FIXED, sizeRsrc);
	//ASSERT(m_hMemStrm != NULL);
	if (!hMemStrm) {
		return;
	}

	BYTE * pMemStrm = (BYTE *)GlobalLock(hMemStrm);
	memcpy(pMemStrm, pMemRsrc, sizeRsrc);

	IStream * pStrm = NULL;
	if (CreateStreamOnHGlobal(pMemStrm, TRUE, &pStrm) == S_OK)
	{
		*ppImage = Image::FromStream(pStrm);
	}
	pStrm->Release();
}

VOID InitDialog(HWND hDlg)
{
	hWndEdit = CreateWindow(L"EDIT", L"",
		WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_BORDER | WS_CLIPCHILDREN,
		0, 0, 100, 100, hDlg,
		(HMENU)10000, hInst, 0);

	SendMessage(hWndEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), (LPARAM)TRUE);
	ShowWindow(hWndEdit, SW_HIDE);
	SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, nWidth, nHeight, SWP_SHOWWINDOW);
}

VOID AbortCapture(HWND hDlg)
{
	if (bDown == true || hWndCommandBar != NULL)
	{
		bSelected = false;
		if (hWndCommandBar != NULL)
			DestroyWindow(hWndCommandBar);
		hWndCommandBar = NULL;
		ptBegin.x = 0;
		ptBegin.y = 0;
		ptEnd.x = 0;
		ptEnd.y = 0;
		ptRectBegin.x = 0;
		ptRectBegin.y = 0;
		ptRectEnd.x = 0;
		ptRectEnd.y = 0;
		bDrawRect = false;
		bDrawText = false;
		bDown = false;
		if (IsWindowEnabled(hWndEdit))
		{
			SetWindowText(hWndEdit, L"");
			EnableWindow(hWndEdit, FALSE);
			ShowWindow(hWndEdit, SW_HIDE);
		}
		BitBlt(hdcPaint, 0, 0, nWidth, nHeight, hdcMem, 0, 0, SRCCOPY);
		InvalidateRect(hDlg, NULL, FALSE);
	}
	else
	{
		wcscpy(wszFilePath, L"unknown");
		HotKeyDefaultSet(FALSE);
		EndDialog(hDlg, 0);
	}
}

VOID Draw(HWND hDlg, HDC hdc)
{
	BitBlt(hdcFinish, 0, 0, nWidth, nHeight, hdcTemp, 0, 0, SRCCOPY);

	BitBlt(hdcFinish, ptBegin.x, ptBegin.y, ptEnd.x - ptBegin.x, ptEnd.y - ptBegin.y, hdcMem, ptBegin.x, ptBegin.y, SRCCOPY);

	if (bSelected == true)
	{
		BitBlt(hdcFinish, ptBegin.x, ptBegin.y, ptEnd.x - ptBegin.x, ptEnd.y - ptBegin.y, hdcPaint, ptBegin.x, ptBegin.y, SRCCOPY);
	}
	if (bDrawRect == true && bDown == true)
	{
		Graphics graphics(hdcFinish);
		Pen redPen(Color(255, 255, 0, 0), 2);
		Rect rect(
			ptRectBegin.x < ptRectEnd.x ? ptRectBegin.x : ptRectEnd.x,
			ptRectBegin.y < ptRectEnd.y ? ptRectBegin.y : ptRectEnd.y,
			abs(ptRectEnd.x - ptRectBegin.x),
			abs(ptRectEnd.y - ptRectBegin.y));

		graphics.DrawRectangle(&redPen, rect);
	}
	else if (bDrawText == true)
	{
		RECT rect = {
			ptTextBegin.x ,
			ptTextBegin.y ,
			ptTextBegin.x + 200,
			ptTextBegin.y + 200
		};

		SetBkMode(hdcFinish, TRANSPARENT);
		SelectObject(hdcFinish, GetStockObject(DEFAULT_GUI_FONT));
		SetTextColor(hdcFinish, RGB(255, 0, 0));
		DrawText(hdcFinish, szText, -1, &rect, DT_LEFT);
	}

	BitBlt(hdc, 0, 0, nWidth, nHeight, hdcFinish, 0, 0, SRCCOPY);

	char szPos[1024];
	sprintf(szPos, "X:%d Y:%d Width:%d Hegiht:%d",
		ptBegin.x,
		ptBegin.y,
		abs(ptEnd.x - ptBegin.x),
		abs(ptEnd.y - ptBegin.y));

	RECT rc = {
		ptBegin.x < nWidth - 300 ? ptBegin.x : nWidth - 300,
		ptBegin.y - 20 < 0 ? ptBegin.y : ptBegin.y - 20,
		(ptBegin.x < nWidth - 300 ? ptBegin.x : nWidth - 300) + 300,
		(ptBegin.y - 20 < 0 ? ptBegin.y : ptBegin.y - 20) + 20 };
	if (ptBegin.x != ptEnd.x)
	{
		SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));
		SetTextColor(hdc, RGB(255, 255, 255));
		SetBkColor(hdc, RGB(0, 0, 0));
		DrawTextA(hdc, szPos, -1, &rc, DT_LEFT);
	}
}

VOID OnLButtonDown(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (bSelected == false)
	{
		if (hWndCommandBar != NULL)
		{
			DestroyWindow(hWndCommandBar);
		}
		if (bDown == false)
		{
			bSelected = false;
			ptBegin.x = LOWORD(lParam);
			ptBegin.y = HIWORD(lParam);
		}
	}
	else
	{
		if (bDrawRect == true)
		{
			ptRectBegin.x = LOWORD(lParam);
			ptRectBegin.y = HIWORD(lParam);

		}
		else if (bDrawText == true)
		{
			DrawText(hDlg, wParam, lParam);
		}
		else if (bDrawLine == true)
		{
			ptOld.x = LOWORD(lParam);
			ptOld.y = HIWORD(lParam);
		}
	}
	bDown = true;
}

VOID DrawText(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (IsWindowEnabled(hWndEdit))
	{
		GetWindowText(hWndEdit, szText, 1024);

		SetWindowText(hWndEdit, L"");
		EnableWindow(hWndEdit, FALSE);
		ShowWindow(hWndEdit, SW_HIDE);
	}
	else
	{
		ptTextBegin.x = LOWORD(lParam);
		ptTextBegin.y = HIWORD(lParam);

		szText[0] = 0;
		EnableWindow(hWndEdit, TRUE);
		SetFocus(hWndEdit);
		MoveWindow(hWndEdit, ptTextBegin.x, ptTextBegin.y, 200, 30, TRUE);
		ShowWindow(hWndEdit, SW_SHOW);
	}
	InvalidateRect(hDlg, NULL, FALSE);
	UpdateWindow(hDlg);
	BitBlt(hdcPaint, ptBegin.x, ptBegin.y, ptEnd.x - ptBegin.x, ptEnd.y - ptBegin.y, hdcFinish, ptBegin.x, ptBegin.y, SRCCOPY);
}

VOID DrawLine(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	MoveToEx(hdcPaint, ptOld.x, ptOld.y, NULL);
	LineTo(hdcPaint, LOWORD(lParam), HIWORD(lParam));
	ptOld.x = LOWORD(lParam);
	ptOld.y = HIWORD(lParam);
	InvalidateRect(hDlg, NULL, FALSE);
	UpdateWindow(hDlg);
}

VOID OnLButtonUp(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (bDown == true && ptBegin.x != ptEnd.x)//·ÀÖ¹Êó±êµãµ½¹¤¾ßÌõÉÏÍÏ¶¯²úÉúµÄbug
	{
		if (bSelected == false)//È·¶¨ÇøÓò
		{

			bSelected = true;
			bDrawRect = false;
			bDrawText = false;
			hWndCommandBar = CreateDialog(hInst, MAKEINTRESOURCE(IDD_COMMANDBAR), hDlg, CommandBarProc);
			if (hWndCommandBar != NULL)
			{
				SetWindowPos(hWndCommandBar,
					HWND_TOPMOST,
					(LOWORD(lParam) - 134) < 0 ? 0 : (LOWORD(lParam) - 134),
					HIWORD(lParam) + 2 < nHeight - 26 ? HIWORD(lParam) + 2 : nHeight - 26,
					134, 26,
					SWP_SHOWWINDOW);
				UpdateWindow(hWndCommandBar);
			}
		}
		else//»­¾ØÐÎ»òÎÄ±¾½áÊø
		{
			if (bDrawRect == true)
			{
				BitBlt(hdcPaint, ptBegin.x, ptBegin.y, ptEnd.x - ptBegin.x, ptEnd.y - ptBegin.y, hdcFinish, ptBegin.x, ptBegin.y, SRCCOPY);
			}
		}
		bDown = false;
	}
}

VOID OnDoubleClick(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	OutputDebugString(L"finished\n");
	RECT rc = {
		ptBegin.x < ptEnd.x ? ptBegin.x : ptEnd.x,
		ptBegin.y < ptEnd.y ? ptBegin.y : ptEnd.y,
		ptEnd.x > ptBegin.x ? ptEnd.x : ptBegin.x,
		ptEnd.y > ptBegin.y ? ptEnd.y : ptBegin.y
	};
	SaveHDCToFile(hDlg, hdcFinish, &rc);
	HotKeyDefaultSet(FALSE);
	EndDialog(hDlg, 0);
}

VOID OnMouseMove(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (bDown == true)
	{
		if (bSelected == false)
		{
			ptEnd.x = LOWORD(lParam);
			ptEnd.y = HIWORD(lParam);
			InvalidateRect(hDlg, NULL, FALSE);
		}
		else
		{
			ptRectEnd.x = LOWORD(lParam);
			ptRectEnd.y = HIWORD(lParam);

			ptTextEnd.x = LOWORD(lParam);
			ptTextEnd.y = HIWORD(lParam);

			if (bDrawRect == true)
			{
				InvalidateRect(hDlg, NULL, FALSE);
			}
			else if (bDrawLine == true)
			{
				DrawLine(hDlg, wParam, lParam);
			}
		}
	}
}
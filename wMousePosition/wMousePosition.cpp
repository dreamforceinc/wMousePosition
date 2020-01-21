// wMousePosition.cpp : Show mouse's position and caption of window under cursor

#include "framework.h"
#include "wMousePosition.h"
#include <commctrl.h>
#pragma comment (lib,"comctl32.lib")

#define timerID			35
#define ID_SLIDER		135
#define MAX_LOADSTRING	30

// Global Variables:
TCHAR			szTitle[MAX_LOADSTRING];
TCHAR			szWindowClass[MAX_LOADSTRING];
TCHAR			bufferWin[MAX_PATH] = TEXT("Push mouse!");
TCHAR			bufferPt[MAX_LOADSTRING] = TEXT("xxxxx, yyyyy");
HHOOK			MouseHook = NULL;
HFONT			hFont = NULL;
HWND			hWnd, hStatic0, hStatic1, hStatic2, hStatic3, hSlider;
MSLLHOOKSTRUCT	*lpmhs;
UINT_PTR		uTimer = NULL;
UINT			tbMin = 128, tbMax = 255, tbPos = tbMax;
int				dtWidth = 0, dtHeight = 0;
int				xPos = 0, yPos = 0;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
VOID				ShowError(HINSTANCE, UINT);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	MouseHookProc(int, WPARAM, LPARAM);

int APIENTRY _tWinMain(_In_		HINSTANCE	hInstance,
					   _In_opt_	HINSTANCE	hPrevInstance,
					   _In_		LPTSTR		lpCmdLine,
					   _In_		int			nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WMOUSEPOSITION, szWindowClass, MAX_LOADSTRING);
	if (FindWindow(szWindowClass, NULL))
	{
		ShowError(hInstance, IDS_RUNNING);
		return FALSE;
	}
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return -1;
	}

	// Hook to all available threads
	MouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, hInstance, NULL);
	if (!MouseHook)
	{
		ShowError(hInstance, IDS_ERR_HOOK);
		SendMessage(hWnd, WM_CLOSE, NULL, NULL);
	}

	MSG msg;
	BOOL bRet;

	// Start the message loop.
	while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
	{
		if (bRet == -1)
		{
			ShowError(hInstance, IDS_ERR_MAIN);
			return -1;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WMOUSEPOSITION));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = wcex.hIcon;
	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	INITCOMMONCONTROLSEX icex; // Structure for control initialization.
	icex.dwICC = ICC_BAR_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rc = { 0 };
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rc, FALSE);
	dtWidth = rc.right, dtHeight = rc.bottom;

	int mainWidth = 500, offset = 8, ctrlHeight = 20;
	int mainHeight = 64;
	int static1Width = mainWidth - 2 * offset;
	int static1posY = offset, static2posY = 2 * offset + ctrlHeight;
	int mainX = (dtWidth - mainWidth) / 2;
	int mainY = (dtHeight - mainHeight) / 2;

	HWND hWnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_LAYERED, szWindowClass, szTitle, WS_BORDER,
							   mainX, mainY, mainWidth, mainHeight,
							   nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		ShowError(hInstance, IDS_ERR_WND);
		return FALSE;
	}

	hStatic0 = CreateWindow(TEXT("STATIC"), NULL, WS_CHILD | WS_VISIBLE | SS_BLACKFRAME,
							0, 0, mainWidth, mainHeight,
							hWnd, NULL, hInstance, nullptr);
	hStatic1 = CreateWindow(TEXT("STATIC"), TEXT("Move mouse"), WS_CHILD | SS_CENTER | SS_SUNKEN | WS_VISIBLE | SS_ENDELLIPSIS,
							offset, static1posY, static1Width, ctrlHeight,
							hWnd, NULL, hInstance, nullptr);
	hStatic2 = CreateWindow(TEXT("STATIC"), nullptr, WS_CHILD | SS_CENTER | SS_SUNKEN | WS_VISIBLE,
							offset, static2posY, static1Width / 3, ctrlHeight,
							hWnd, NULL, hInstance, nullptr);
	hSlider = CreateWindow(TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | TBS_NOTICKS | TBS_TOOLTIPS | TBS_BOTH,
						   offset + static1Width / 3 + offset, static2posY, static1Width / 3, ctrlHeight,
						   hWnd, (HMENU)ID_SLIDER, hInstance, nullptr);
	hStatic3 = CreateWindow(TEXT("STATIC"), TEXT("Alt + F4 for Exit"), WS_CHILD | SS_CENTER | SS_SUNKEN | WS_VISIBLE,
							offset + 2 * (static1Width / 3) + 2 * offset, static2posY, static1Width / 3 - 2 * offset, ctrlHeight,
							hWnd, NULL, hInstance, nullptr);
	SendMessage(hStatic1, WM_SETFONT, (WPARAM)hFont, FALSE);
	SendMessage(hStatic3, WM_SETFONT, (WPARAM)hFont, FALSE);
	SendMessage(hSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(tbMin, tbMax));
	SendMessage(hSlider, TBM_SETPOS, (WPARAM)TRUE, tbMax);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		// Loading font
		hFont = CreateFont(16, 0, 0, 0, FW_MEDIUM, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, TEXT("Segoe UI"));

		// Make window semitransparent
		SetLayeredWindowAttributes(hWnd, NULL, tbMax, LWA_ALPHA);

		// Remove caption
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
		uTimer = SetTimer(hWnd, timerID, timerID, NULL);
		break;

	case WM_TIMER:
		SetWindowText(hStatic2, bufferPt);
		SetWindowText(hStatic1, bufferWin);
		break;

	case WM_HSCROLL:
		if (lParam == (LPARAM)hSlider)
		{
			tbPos = SendMessage(hSlider, TBM_GETPOS, NULL, NULL);
			SetLayeredWindowAttributes(hWnd, NULL, tbPos, LWA_ALPHA);
		}
		break;

	case WM_LBUTTONDOWN:
		ReleaseCapture();
		SendMessage(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
		break;

	case WM_DESTROY:
		if (uTimer) KillTimer(hWnd, timerID);
		if (hFont) DeleteObject(hFont);
		if (MouseHook) UnhookWindowsHookEx(MouseHook);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	lpmhs = (MSLLHOOKSTRUCT*)lParam;
	xPos = lpmhs->pt.x;
	yPos = lpmhs->pt.y;

	HWND win = WindowFromPoint(lpmhs->pt);
	if (win)
	{
		HWND pwin = win;
		while (TRUE)
		{
			pwin = GetParent(win);
			if (pwin) win = pwin;
			else break;
		}
		GetWindowText(win, bufferWin, sizeof(bufferWin) - sizeof(TCHAR));
	}
	StringCchPrintf(bufferPt, sizeof(bufferPt) - sizeof(TCHAR), TEXT("%d, %d"), xPos, yPos);

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

VOID ShowError(HINSTANCE hInstance, UINT uID)
{
	WCHAR szErrorText[MAX_LOADSTRING]; // Текст ошибки
	LoadStringW(hInstance, uID, szErrorText, MAX_LOADSTRING);
	MessageBox(hWnd, szErrorText, szTitle, MB_OK | MB_ICONERROR);
}

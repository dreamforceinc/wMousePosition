// wMousePosition.cpp : Show mouse's position and caption of window under cursor

#include "framework.h"
#include "wMousePosition.h"
#include <commctrl.h>
#pragma comment (lib,"comctl32.lib")

#define timerID         35
#define ID_SLIDER       135
#define MAX_LOADSTRING  32

// Global Variables:
WCHAR       szTitle[MAX_LOADSTRING];
WCHAR       szWindowClass[MAX_LOADSTRING];
WCHAR       bufferWin[MAX_PATH] = L"Push mouse!";
WCHAR       bufferPt[MAX_LOADSTRING] = L"xxxxx, yyyyy";
HFONT       hFont = NULL;
HWND        hWnd, hStatic0, hStatic1, hStatic2, hStatic3, hSlider;
UINT_PTR    uTimer = NULL;
UINT        tbMin = 128, tbMax = 255, tbPos = tbMax;
LONG        dtWidth = 0, dtHeight = 0;
POINT       ptPos = { 0 };

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
VOID                ShowError(HINSTANCE, UINT);
VOID                GetPositionAndTitle();
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WMOUSEPOSITION, szWindowClass, MAX_LOADSTRING);
    if (FindWindowW(szWindowClass, NULL)) {
        ShowError(hInstance, IDS_RUNNING);
        return FALSE;
    }
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow)) {
        return -1;
    }

    GetPositionAndTitle();

    MSG msg;
    BOOL bRet;

    // Start the message loop.
    while ((bRet = GetMessageW(&msg, NULL, 0, 0)) != 0) {
        if (bRet == -1) {
            ShowError(hInstance, IDS_ERR_MAIN);
            return -1;
        }
        else {
            GetPositionAndTitle();
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEXW);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_WMOUSEPOSITION));
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = wcex.hIcon;
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    INITCOMMONCONTROLSEX icex = { 0 }; // Structure for control initialization.
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    RECT rc = { 0 };
    SystemParametersInfoW(SPI_GETWORKAREA, NULL, &rc, FALSE);
    dtWidth = rc.right, dtHeight = rc.bottom;

    int mainWidth = 500, offset = 8, ctrlHeight = 20;
    int mainHeight = 64;
    int static1Width = mainWidth - 2 * offset;
    int static1posY = offset, static2posY = 2 * offset + ctrlHeight;
    int mainX = (dtWidth - mainWidth) / 2;
    int mainY = (dtHeight - mainHeight) / 2;

    HWND hWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED, szWindowClass, szTitle, WS_BORDER,
        mainX, mainY, mainWidth, mainHeight,
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        ShowError(hInstance, IDS_ERR_WND);
        return FALSE;
    }

    hStatic0 = CreateWindowExW(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BLACKFRAME, 0, 0, mainWidth, mainHeight, hWnd, NULL, hInstance, nullptr);
    hStatic1 = CreateWindowExW(0, L"STATIC", L"Move mouse", WS_CHILD | SS_CENTER | SS_SUNKEN | WS_VISIBLE | SS_ENDELLIPSIS, offset, static1posY, static1Width, ctrlHeight, hWnd, NULL, hInstance, nullptr);
    hStatic2 = CreateWindowExW(0, L"STATIC", nullptr, WS_CHILD | SS_CENTER | SS_SUNKEN | WS_VISIBLE, offset, static2posY, static1Width / 3, ctrlHeight, hWnd, NULL, hInstance, nullptr);
    hSlider = CreateWindowExW(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | TBS_NOTICKS | TBS_TOOLTIPS | TBS_BOTH, offset + static1Width / 3 + offset, static2posY, static1Width / 3, ctrlHeight, hWnd, (HMENU)ID_SLIDER, hInstance, nullptr);
    hStatic3 = CreateWindowExW(0, L"STATIC", L"Alt + F4 for Exit", WS_CHILD | SS_CENTER | SS_SUNKEN | SS_NOTIFY | WS_VISIBLE, offset + 2 * (static1Width / 3) + 2 * offset, static2posY, static1Width / 3 - 2 * offset, ctrlHeight, hWnd, (HMENU)235, hInstance, nullptr);
    SendMessageW(hStatic1, WM_SETFONT, (WPARAM)hFont, FALSE);
    SendMessageW(hStatic3, WM_SETFONT, (WPARAM)hFont, FALSE);
    SendMessageW(hSlider, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(tbMin, tbMax));
    SendMessageW(hSlider, TBM_SETTIPSIDE, (WPARAM)TBTS_TOP, NULL);
    SendMessageW(hSlider, TBM_SETPOS, (WPARAM)TRUE, tbMax);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_CREATE:
        {
            // Loading font
            hFont = CreateFontW(16, 0, 0, 0, FW_MEDIUM, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, L"Segoe UI");

            // Make window semitransparent
            SetLayeredWindowAttributes(hWnd, NULL, tbMax, LWA_ALPHA);

            // Remove caption
            SetWindowLongW(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & (~WS_CAPTION));
            uTimer = SetTimer(hWnd, timerID, 50, NULL);
        }
        break;

        case WM_TIMER:
        {
            SetWindowTextW(hStatic2, bufferPt);
            SetWindowTextW(hStatic1, bufferWin);
        }
        break;

        case WM_HSCROLL:
        {
            if (lParam == (LPARAM)hSlider) {
                tbPos = SendMessageW(hSlider, TBM_GETPOS, NULL, NULL);
                SetLayeredWindowAttributes(hWnd, NULL, tbPos, LWA_ALPHA);
            }
        }
        break;

        case WM_LBUTTONDOWN:
        {
            ReleaseCapture();
            SendMessageW(hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);
        }
        break;

        case WM_DESTROY:
        {
            if (uTimer) KillTimer(hWnd, timerID);
            if (hFont) DeleteObject(hFont);
            PostQuitMessage(0);
        }
        break;

        default: return DefWindowProcW(hWnd, message, wParam, lParam);
    }
    return 0;
}

VOID GetPositionAndTitle() {
    HWND win = NULL, pwin = NULL;
    GetCursorPos(&ptPos);
    win = WindowFromPoint(ptPos);
    if (win) {
        pwin = win;
        while (pwin) {
            pwin = GetParent(win);
            if (pwin) win = pwin;
            //else break;
        }
        GetWindowTextW(win, bufferWin, ARRAYSIZE(bufferWin));
    }
    StringCchPrintfW(bufferPt, ARRAYSIZE(bufferPt), TEXT("%d, %d"), ptPos.x, ptPos.y);
}

VOID ShowError(HINSTANCE hInstance, UINT uID) {
    WCHAR szErrorText[MAX_LOADSTRING]; // Error text
    LoadStringW(hInstance, uID, szErrorText, MAX_LOADSTRING);
    MessageBoxW(hWnd, szErrorText, szTitle, MB_OK | MB_ICONERROR);
}

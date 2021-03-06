#include "stdafx.h"
#include "RedSpiderKiller.h"

#include "Killer.h"

#define MAX_LOADSTRING 100

// 工具栏的宽高
const int TOOLBAR_WIDTH = 10 + (32 + 10) * 1;
const int TOOLBAR_HEIGHT = 10 + (32 + 10) * 3;

// LOGO
const int LOGO_X = 10;
const int LOGO_Y = 10;
// 切换显示状态图标的位置
const int SWITCH_X = 10;
const int SWITCH_Y = 10 + (32 + 10) * 1;
// 退出图标的位置
const int EXIT_X = 10;
const int EXIT_Y = 10 + (32 + 10) * 2;

std::shared_ptr<Killer> killer = nullptr;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// 创建包含窗口用的函数
WCHAR szWindowClassTopWindow[] = L"TopWindow";
ATOM				MyRegisterClassTopWindow(HINSTANCE hInstance);
LRESULT CALLBACK	WndProcTopWindow(HWND, UINT, WPARAM, LPARAM);

int screenWidth;
int screenHeight;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_REDSPIDERKILLER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	MyRegisterClassTopWindow(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_REDSPIDERKILLER));

	MSG msg;

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_REDSPIDERKILLER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

// 注册包含窗口类
ATOM MyRegisterClassTopWindow(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProcTopWindow;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = nullptr;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = szWindowClassTopWindow;
	wcex.hIconSm = nullptr;

	return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	killer = Killer::Create();

	hInst = hInstance;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	HWND hWnd = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClass, szTitle, WS_POPUP,
		screenWidth - TOOLBAR_WIDTH, screenHeight / 3, TOOLBAR_WIDTH, TOOLBAR_HEIGHT, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	// 创建包含窗口并设置，全屏无边框
	auto topWindow = CreateWindowEx(WS_EX_TOOLWINDOW, szWindowClassTopWindow, szTitle, WS_POPUP | WS_CLIPCHILDREN,
		0, 0, screenWidth, screenHeight, nullptr, nullptr, hInstance, nullptr);
	killer->SetTopWindow(topWindow);

	// 初始化
	if (!killer->Init())
	{
		auto mbresult = MessageBox(hWnd, L"未检测到红蜘蛛，请手动选择文件。", szTitle, MB_OKCANCEL);
		if (mbresult == IDOK)
		{
			// 自动获取失败，手动填写
			static TCHAR szFileName[MAX_PATH];
			static TCHAR szTitleName[MAX_PATH];
			static TCHAR szFilter[] = TEXT("Execute Files(*.EXE)\0*.exe\0\0");

			OPENFILENAME ofn;
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.hInstance = nullptr;
			ofn.lpstrFilter = szFilter;
			ofn.lpstrCustomFilter = nullptr;
			ofn.nMaxCustFilter = 0;
			ofn.nFilterIndex = 0;
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFileTitle = szTitleName;
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrInitialDir = nullptr;
			ofn.lpstrTitle = nullptr;
			ofn.Flags = 0;
			ofn.nFileOffset = 0;
			ofn.nFileExtension = 0;
			ofn.lpstrDefExt = TEXT("exe");
			ofn.lCustData = 0;
			ofn.lpfnHook = nullptr;
			ofn.lpTemplateName = nullptr;
			if (GetOpenFileName(&ofn))
			{
				killer->SetFilePath(std::wstring(szFileName));
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 图标
	static HICON iconLogo = LoadIcon(hInst, MAKEINTRESOURCE(IDI_REDSPIDERKILLER));
	static HICON iconHide = LoadIcon(hInst, MAKEINTRESOURCE(IDI_HIDE));
	static HICON iconShow = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SHOW));
	static HICON iconExit = LoadIcon(hInst, MAKEINTRESOURCE(IDI_EXIT));
	// 窗口的 X 坐标
	static int posX = screenWidth - TOOLBAR_WIDTH;
	// 鼠标是否进入了窗口（不保证没有出去）
	static bool isMouseEntered = true;
	// 是否找到并且设定了红蜘蛛窗口
	static bool spiderWindowSeted = false;
	// 计时，隐藏
	static int timeCount = 0;


	switch (message)
	{
	case WM_CREATE:
	{
		// 所有计时都用一个计时器
		SetTimer(hWnd, 0, 1, nullptr);
		break;
	}

	case WM_MOUSEMOVE:
	{
		isMouseEntered = true;
		timeCount = 0;
		break;
	}

	case WM_TIMER:
	{
		// 查找并设置红蜘蛛窗口（仅全屏生效）
		if (!spiderWindowSeted)
		{
			auto topWindow = GetForegroundWindow();
			if (killer->SetSpiderWindow(topWindow))
			{
				killer->Window();
				killer->Show();
				spiderWindowSeted = true;
			}
		}
		// 工具栏显示与隐藏
		if (isMouseEntered && posX > screenWidth - TOOLBAR_WIDTH)
		{
			posX -= 5;
		}
		if (!isMouseEntered && posX < screenWidth - 5)
		{
			posX += 5;
		}
		if (timeCount++ >= 250)
		{
			timeCount = 0;
			isMouseEntered = false;
		}
		auto swp = SWP_NOSIZE | SWP_NOACTIVATE;
		if (!killer->IsInited())
		{
			swp |= SWP_NOZORDER;
		}
		SetWindowPos(hWnd, HWND_TOPMOST, posX, screenHeight / 3, 0, 0, swp);
		break;
	}

	case WM_LBUTTONUP:
	{
		auto x = LOWORD(lParam);
		auto y = HIWORD(lParam);
		if (x > LOGO_X && y > LOGO_Y && x < LOGO_X + 32 && y < LOGO_Y + 32)
		{
			MessageBox(hWnd, L"RedSpiderKiller v0.1\r\n版权没有 ocrosoft.com", L"关于", MB_OK);
		}
		else if (x > SWITCH_X && y > SWITCH_Y && x < SWITCH_X + 32 && y < SWITCH_Y + 32)
		{
			if (spiderWindowSeted)
			{
				if (killer->IsHide())
				{
					killer->Show();
				}
				else
				{
					killer->Hide();
				}
				InvalidateRect(hWnd, nullptr, TRUE);
			}
			else
			{
				MessageBox(hWnd, L"全屏控制后会自动窗口化，\r\n窗口化后此按钮用于隐藏及显示，\r\n因此现在无效。", L"按钮不可用", MB_OK);
			}
		}
		else if (x > EXIT_X && y > EXIT_Y && x < EXIT_X + 32 && y < EXIT_Y + 32)
		{
			auto mbresult = MessageBox(hWnd, L"关闭后红蜘蛛会恢复到原始状态，确定关闭吗？", L"警告", MB_YESNO);
			if (mbresult == IDYES)
			{
				SendMessage(hWnd, WM_DESTROY, 0, 0);
			}
		}
		break;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		DrawIconEx(hdc, LOGO_X, LOGO_Y, iconLogo, 32, 32, 0, nullptr, DI_NORMAL);
		if (killer->IsHide())
		{
			DrawIconEx(hdc, SWITCH_X, SWITCH_Y, iconShow, 32, 32, 0, nullptr, DI_NORMAL);
		}
		else
		{
			DrawIconEx(hdc, SWITCH_X, SWITCH_Y, iconHide, 32, 32, 0, nullptr, DI_NORMAL);
		}
		DrawIconEx(hdc, EXIT_X, EXIT_Y, iconExit, 32, 32, 0, nullptr, DI_NORMAL);
		EndPaint(hWnd, &ps);
		break;
	}

	case WM_DESTROY:
	{
		KillTimer(hWnd, 0);
		PostQuitMessage(0);
		break;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// 包含窗口的窗口过程，啥都不干
LRESULT CALLBACK WndProcTopWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SETFOCUS:
		SetFocus(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
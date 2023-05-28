#include <Windows.h>
#include <iostream>

using namespace std;

void changeDisplaySettings(DEVMODE dm, DWORD old = 0)
{
    LONG ret = ChangeDisplaySettings(&dm, CDS_UPDATEREGISTRY | CDS_GLOBAL);
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    switch (ret)
    {
    case DISP_CHANGE_SUCCESSFUL:
    {
        char buff[1024];
        sprintf_s(buff, "Refresh rate changed from %d to %d successfully", old, dm.dmDisplayFrequency);
        wchar_t wbuff[1025];
        mbstowcs_s(nullptr, wbuff, 1025, buff, 1024);
        MessageBox(NULL, wbuff, L"VRR", NULL);
        break;
    }
    case DISP_CHANGE_BADPARAM:
        MessageBox(NULL, L"Error: BADPARAM", L"ERROR", NULL);
        break;
    case DISP_CHANGE_FAILED:
        MessageBox(NULL, L"Error: Change failed", L"ERROR", NULL);
        break;
    case DISP_CHANGE_RESTART:
    {
        char buff[1024];
        sprintf_s(buff, "To change refresh rate from %d to %d RESTART your PC", old, dm.dmDisplayFrequency);
        wchar_t wbuff[1025];
        mbstowcs_s(nullptr, wbuff, 1025, buff, 1024);
        MessageBox(NULL, wbuff, L"VRR", NULL);
        break;
    }
    default:
        MessageBox(NULL, L"Unknown error", L"ERROR", NULL);
        break;
    }
}

void checkPoverAndSwitch()
{
    DEVMODE dm = {};
    dm.dmSize = sizeof(dm);

    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

    SYSTEM_POWER_STATUS powerStatus;
    GetSystemPowerStatus(&powerStatus);
    if (powerStatus.ACLineStatus == 1)
    {
        if (dm.dmDisplayFrequency == 165) return;
        DWORD old = dm.dmDisplayFrequency;
        dm.dmDisplayFrequency = 165;
        dm.dmFields = DM_DISPLAYFREQUENCY;
        changeDisplaySettings(dm, old);
    }
    else
    {
        if (dm.dmDisplayFrequency == 60) return;
        DWORD old = dm.dmDisplayFrequency;
        dm.dmDisplayFrequency = 60;
        dm.dmFields = DM_DISPLAYFREQUENCY;
        changeDisplaySettings(dm, old);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        checkPoverAndSwitch();
        break;
    }
    case WM_POWERBROADCAST:
    {
        checkPoverAndSwitch();
        break;
    }
    case WM_DESTROY: 
    {
        PostQuitMessage(0);
        break;
    }
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MSG msg;
    WNDCLASS wc = { 0 };
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.lpszClassName = L"ESVRR";
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;


    RegisterClass(&wc);
    HWND hWnd = CreateWindowW(wc.lpszClassName, L"Energy-saving VRR", WS_OVERLAPPED | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 600, 300, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        MessageBox(NULL,
            L"CreateWindow failed!",
            L"Error",
            NULL);

        return 1;
    }

    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    RegisterPowerSettingNotification(hWnd, &GUID_ACDC_POWER_SOURCE, DEVICE_NOTIFY_WINDOW_HANDLE);

    return (int)msg.wParam;
}
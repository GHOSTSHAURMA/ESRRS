#include <Windows.h>
#include <iostream>
#include "resource.h"

using namespace std;

const int batteryRR = 60, lineRR = 165;
HICON icon_battery_full, icon_plug;
NOTIFYICONDATAA iconData = { };
HMENU icon_popup_menu;

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
        MessageBox(NULL, wbuff, L"ESRRS", NULL);
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
        MessageBox(NULL, wbuff, L"ESRRS", NULL);
        break;
    }
    default:
        MessageBox(NULL, L"Unknown error", L"ERROR", NULL);
        break;
    }
}

void updateIcon(HWND hWnd)
{
    SYSTEM_POWER_STATUS powerStatus;
    GetSystemPowerStatus(&powerStatus);
    iconData.hIcon = powerStatus.ACLineStatus == 1 ? icon_plug : icon_battery_full;

    Shell_NotifyIconA(NIM_MODIFY, &iconData);
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
        if (dm.dmDisplayFrequency == lineRR) return;
        DWORD old = dm.dmDisplayFrequency;
        dm.dmDisplayFrequency = lineRR;
        dm.dmFields = DM_DISPLAYFREQUENCY;
        changeDisplaySettings(dm, old);
    }
    else
    {
        if (dm.dmDisplayFrequency == batteryRR) return;
        DWORD old = dm.dmDisplayFrequency;
        dm.dmDisplayFrequency = batteryRR;
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
        updateIcon(hWnd);
        break;
    }
    case WM_DESTROY: 
    {
        PostQuitMessage(0);
        break;
    }
    case WM_COMMAND:
    {
        switch (wParam)
        {
        case ID_ICON_CLOSE:
        {
            DestroyWindow(hWnd);

            break;
        }
        case ID_ICON_SETTINGS:
        {
            MessageBox(hWnd, L"To be implemented.", L"Settings", NULL);

            break;
        }
        default:
            break;
        }

        break;
    }
    case WM_USER:
    {
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONUP:
        {
            POINT pt;
            GetCursorPos(&pt);
            icon_popup_menu = GetSubMenu(LoadMenuA((HINSTANCE)GetWindowLongA(hWnd, GWL_HINSTANCE), MAKEINTRESOURCEA(IDR_MENU_ICON)), 0);

            TPMPARAMS tpmp = {};
            tpmp.cbSize = sizeof(tpmp);
            tpmp.rcExclude = { 0, 0, 0, 0 };

            TrackPopupMenuEx(icon_popup_menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, pt.x, pt.y, hWnd, &tpmp);

            break;
        }
        default:
            break;
        }
    }
    }

    return DefWindowProcA(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_  LPSTR lpCmdLine, _In_  int nCmdShow)
{
    int lastError;
    icon_battery_full = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_BATTERY_FULL));
    icon_plug = LoadIconA(hInstance, MAKEINTRESOURCEA(IDI_PLUG));

    MSG msg;
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.lpszClassName = L"ESRRS";
    wc.hInstance = hInstance;
    wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
    wc.lpfnWndProc = WndProc;

    RegisterClassEx(&wc);
    HWND hWnd = CreateWindowEx(0, wc.lpszClassName, L"ESRRS", WS_OVERLAPPED | WS_MINIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 600, 300, NULL, NULL, hInstance, NULL);

    if (!hWnd)
    {
        MessageBox(NULL,
            L"CreateWindow failed!",
            L"Error",
            NULL);

        return 1;
    }

    // No need for now
    // Add settings later
    // ShowWindow(hWnd, SHOW_OPENWINDOW);

    iconData.cbSize = sizeof(iconData);
    iconData.hWnd = hWnd;
    iconData.uCallbackMessage = WM_USER;
    iconData.uVersion = NOTIFYICON_VERSION_4;
    iconData.uFlags = NIF_TIP | NIF_MESSAGE | NIF_ICON;
    
    SYSTEM_POWER_STATUS powerStatus;
    GetSystemPowerStatus(&powerStatus);
    iconData.hIcon = powerStatus.ACLineStatus == 1? icon_plug : icon_battery_full;

    memcpy(iconData.szTip, "ESRRS\nEnergy-saving refresh rate switch", 40);

    Shell_NotifyIconA(NIM_ADD, &iconData);

    UpdateWindow(hWnd);

    while (GetMessage(&msg, NULL, 0, 0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIconA(NIM_DELETE, &iconData);

    return (int)msg.wParam;
}
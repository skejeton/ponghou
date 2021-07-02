#ifndef UNICODE
#define UNICODE
#include <unistd.h>
#endif 

#include "game.h"
#include "wminterop.h"
#include <stdio.h>
#include <windows.h>
#include <gdiplus.h>

struct Wmi_Struct wmi_global;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


static HDC hdc;
static HDC memhdc;
static HBITMAP hbitmap;
static int wsizex;
static int wsizey;

static void update_wsize(HWND hwnd) {
	RECT r;
	GetClientRect( hwnd, &r );
    
	wsizex = (int)(r.right - r.left);
	wsizey = (int)(r.bottom - r.top);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    (void)hPrevInstance;
    (void)pCmdLine;
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";
    
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    //GdiplusStartupInput gdiplusStartupInput;
    //ULONG_PTR           gdiplusToken;
       
    // Initialize GDI+.
    // GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Ponghou",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);

    update_wsize(hwnd);
    hdc = GetDC(hwnd);
    memhdc = CreateCompatibleDC(hdc);
    hbitmap = CreateCompatibleBitmap(hdc, wsizex, wsizey);
    SelectObject(memhdc, hbitmap);


    // Run the message loop.
    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        wmi_global = (struct Wmi_Struct) {
            ._vendor = {
                0, 0, 0,
                { 0 },
                { 0 },
                0
            },
            .buf = malloc((size_t)(wsizex*wsizey*4)),
            .current_color = 0,
            .window_size = { wsizex, wsizey }
        };
        game_start(&wmi_global);
        SetTimer(hwnd, 1, 1000/30, NULL);  
        break;
    case WM_SIZE: {
        update_wsize(hwnd);
        hdc = GetDC(hwnd);
        DeleteObject(hbitmap);
        hbitmap = CreateCompatibleBitmap(hdc, wsizex, wsizey);
        SelectObject(memhdc, hbitmap);
        free(wmi_global.buf);
        wmi_global.buf = malloc((size_t)(wsizex*wsizey*4));
        wmi_global.window_size = (struct Wmi_Size) { wsizex, wsizey };
    }
    break;
    case WM_LBUTTONDOWN:
        wmi_global._vendor.lmousedown = true;
        wmi_global._vendor.justlmousedown = true;
        break;
    case WM_LBUTTONUP:
        wmi_global._vendor.lmousedown = false;
        break;
    case WM_KEYDOWN:
        wmi_global._vendor.released[wParam] = false;
        wmi_global._vendor.pressed[wParam] = true;
        wmi_global._vendor.lastkey = wParam;
        break;
    case WM_KEYUP:
        wmi_global._vendor.released[wParam] = true;
        wmi_global._vendor.pressed[wParam] = false;
        break;
    case WM_TIMER:
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_ERASEBKGND:
        return TRUE;
    case WM_PAINT:
        {
            POINT cursor;
            GetCursorPos(&cursor);
            ScreenToClient(hwnd, &cursor);
            wmi_global.mouse_pos = (struct Wmi_Point) { .x = cursor.x, .y = cursor.y };
            game_loop(&wmi_global);
            SetBitmapBits(hbitmap, (UINT)(wsizex*wsizey*4), wmi_global.buf);
            BitBlt(hdc, 0, 0, wsizex, wsizey, memhdc, 0, 0, SRCCOPY);
            ValidateRect(hwnd, NULL);
            wmi_global._vendor.justlmousedown = false;
            wmi_global._vendor.lastkey = 0xFFFF;
        }
        return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

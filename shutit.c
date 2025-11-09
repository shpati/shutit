#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

volatile BOOL cancel = FALSE;
volatile int countdown = 60;

DWORD WINAPI ShutdownThread(LPVOID _) {
    while (countdown > 0 && !cancel) { Sleep(1000); countdown--; }
    if (!cancel) WinExec("shutdown /p /f", SW_HIDE);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND btnOK, btnCancel;
    char buf[32];
    switch (msg) {
    case WM_CREATE: {
        HFONT f = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        btnOK = CreateWindow("BUTTON", "", WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON, 70,30,100,30, hwnd,(HMENU)1,NULL,NULL);
        EnableWindow(btnOK,FALSE); SendMessage(btnOK,WM_SETFONT,(WPARAM)f,TRUE);
        btnCancel = CreateWindow("BUTTON","Cancel",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,180,30,100,30,hwnd,(HMENU)2,NULL,NULL);
        SendMessage(btnCancel,WM_SETFONT,(WPARAM)f,TRUE);
        CreateThread(NULL,0,ShutdownThread,NULL,0,NULL);
        SetTimer(hwnd,1,1000,NULL);
        HICON hIcon = LoadIcon(NULL, IDI_INFORMATION); // Other options: IDI_APPLICATION, IDI_WARNING, IDI_ERROR, IDI_SHIELD
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        break;
    }
    case WM_TIMER: sprintf(buf,"%d",countdown); SetWindowText(btnOK,buf); if(countdown<=0) KillTimer(hwnd,1); break;
    case WM_COMMAND: if(LOWORD(wParam)==2){cancel=TRUE; WinExec("shutdown /a",SW_HIDE); MessageBox(hwnd,"Shutdown cancelled!","CANCELLED",MB_OK|MB_ICONINFORMATION); DestroyWindow(hwnd);} break;
    case WM_CLOSE: cancel=TRUE; WinExec("shutdown /a",SW_HIDE); DestroyWindow(hwnd); break;
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR cmd,int nShow){
    WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=hInst; wc.lpszClassName="ShutdownWnd"; wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    RegisterClass(&wc);
    if(cmd && cmd[0]){int t=atoi(cmd); if(t>=0) countdown=t;}
    HWND hwnd = CreateWindow("ShutdownWnd","Shutdown Timer",WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX),CW_USEDEFAULT,CW_USEDEFAULT,360,120,NULL,NULL,hInst, NULL);
    RECT r; GetWindowRect(hwnd,&r); SetWindowPos(hwnd,NULL,(GetSystemMetrics(SM_CXSCREEN)-(r.right-r.left))/2,(GetSystemMetrics(SM_CYSCREEN)-(r.bottom-r.top))/2,0,0,SWP_NOSIZE|SWP_NOZORDER);
    ShowWindow(hwnd,nShow);
    MSG msg; while(GetMessage(&msg,NULL,0,0)){TranslateMessage(&msg); DispatchMessage(&msg);}
    return 0;
}

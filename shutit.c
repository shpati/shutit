#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

volatile BOOL cancel = FALSE;
volatile int countdown = 60;
char cmdShutdown[128] = "shutdown /s /t 60";
char actionText[64] = "Active command: Shutdown";

DWORD WINAPI ShutdownThread(LPVOID _) {
    while(countdown > 0 && !cancel) { Sleep(1000); countdown--; }
    if(!cancel) WinExec(cmdShutdown, SW_HIDE);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static HWND btnOK, btnCancel, lblInfo;
    char buf[32];
    switch(msg){
    case WM_CREATE: {
        HFONT f = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        btnOK = CreateWindow("BUTTON","",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,50,50,100,30,hwnd,(HMENU)1,NULL,NULL);
        EnableWindow(btnOK,FALSE); SendMessage(btnOK,WM_SETFONT,(WPARAM)f,TRUE);
        btnCancel = CreateWindow("BUTTON","Cancel",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,160,50,100,30,hwnd,(HMENU)2,NULL,NULL);
        SendMessage(btnCancel,WM_SETFONT,(WPARAM)f,TRUE);
        lblInfo = CreateWindow("STATIC",actionText,WS_CHILD|WS_VISIBLE|SS_CENTER,10,10,300,30,hwnd,NULL,NULL,NULL);
        SendMessage(lblInfo,WM_SETFONT,(WPARAM)f,TRUE);

        HICON hIcon = LoadIcon(NULL, IDI_INFORMATION);
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

        CreateThread(NULL,0,ShutdownThread,NULL,0,NULL);
        SetTimer(hwnd,1,1000,NULL);
        break;
    }
    case WM_TIMER:
        sprintf(buf,"%d",countdown);
        SetWindowText(btnOK, buf);
        if(countdown <= 0) KillTimer(hwnd,1);
        break;
    case WM_COMMAND:
        if(LOWORD(wParam)==2){ cancel=TRUE; WinExec("shutdown /a", SW_HIDE);
            MessageBox(hwnd,"Shutdown cancelled!","CANCELLED",MB_OK|MB_ICONINFORMATION); DestroyWindow(hwnd); }
        break;
    case WM_CLOSE: cancel=TRUE; WinExec("shutdown /a", SW_HIDE); DestroyWindow(hwnd); break;
    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hwnd,msg,wParam,lParam);
    }
    return 0;
}

void parseCmdLine(LPSTR cmd){
    int t=0;
    char mode[16] = "";
    if(cmd && cmd[0]){
        while(*cmd==' ') cmd++;
        int i=0; while(*cmd && *cmd!=' ' && i<15) mode[i++]=*cmd++; mode[i]=0;
        while(*cmd==' ') cmd++; if(*cmd) t=atoi(cmd); if(t>0) countdown=t;
    }

    if(_stricmp(mode,"/r")==0) { sprintf(cmdShutdown,"shutdown /r /t %d /f",countdown); strcpy(actionText,"Active command: Restart"); }
    else if(_stricmp(mode,"/h")==0) { sprintf(cmdShutdown,"shutdown /h"); strcpy(actionText,"Active command: Hibernate"); }
    else if(_stricmp(mode,"/sleep")==0) { sprintf(cmdShutdown,"rundll32.exe powrprof.dll,SetSuspendState 0,1,0"); strcpy(actionText,"Active command: Sleep"); }
    else { sprintf(cmdShutdown,"shutdown /s /t %d",countdown); strcpy(actionText,"Active command: Shutdown"); }
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR cmd,int nShow){
    WNDCLASS wc={0}; wc.lpfnWndProc=WndProc; wc.hInstance=hInst; wc.lpszClassName="ShutdownWnd"; wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
    RegisterClass(&wc);
    parseCmdLine(cmd);
    HWND hwnd=CreateWindow("ShutdownWnd","shutit",WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
                           CW_USEDEFAULT,CW_USEDEFAULT,320,150,NULL,NULL,hInst,NULL);

    RECT r; GetWindowRect(hwnd,&r);
    SetWindowPos(hwnd,NULL,(GetSystemMetrics(SM_CXSCREEN)-(r.right-r.left))/2,
                 (GetSystemMetrics(SM_CYSCREEN)-(r.bottom-r.top))/2,0,0,SWP_NOSIZE|SWP_NOZORDER);

    ShowWindow(hwnd,nShow);
    MSG msg; while(GetMessage(&msg,NULL,0,0)){TranslateMessage(&msg); DispatchMessage(&msg);}
    return 0;
}

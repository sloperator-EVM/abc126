#include <stdio.h>
#include "getAbsPos.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "absMove.h"
#include "keyboard.h"
#include "getKeyState.h"

bool is_initilized = false;


void MAIN_INIT(){
    is_initilized = true;
    init_tablet();
    init_virtual_mouse();
    initilize_keyboard();
    init_libinput();
    init_layer_shell();
    pthread_t check_buttons_thread;
    pthread_t cursor_pos_thread;
    //pthread_create(&cursor_pos_thread, NULL, update_cursor_pos, NULL); 
    pthread_create(&check_buttons_thread, NULL, check_buttons, NULL);
    
}
void MAIN_DESTROY(){
    destroy_tablet();
    destroy_layer_shell();
    destroy_virtual_mouse();
    destroy_keyboard();
    destroy_libinput();
}

void SetCursorPos(int x, int y) {
    if (!is_initilized) MAIN_INIT();
    if (tablet.fd < 0) {
        perror("Tablet is not is_initilized \n");
        return;
    }
    send_absolute(x, y); 
}

bool GetCursorPos(POINT *point){
    if (!is_initilized) MAIN_INIT();
    (*point).x = cursor_x;
    (*point).y = cursor_y;
    return 1;
}

UINT SendInput(UINT cInputs, INPUT inputs[], int cbSize){
    if (!is_initilized) MAIN_INIT();
    for (int i = 0; i < cInputs; i++){
        INPUT input = inputs[i];
        switch (input.type){
            case (INPUT_MOUSE):
                switch (input.mi.dwFlags){
                    case (MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE):
                        SetCursorPos(input.mi.dx, input.mi.dy);
                        break;
                    case (MOUSEEVENTF_MOVE):
                        mouseMove(input.mi.dx, input.mi.dy);
                        break;
                    case (MOUSEEVENTF_LEFTDOWN):
                        emit_mouse(EV_KEY, BTN_LEFT, 1);
                        sync_mouse();
                        break;
                    case (MOUSEEVENTF_LEFTUP):
                        emit_mouse(EV_KEY, BTN_LEFT, 0);
                        sync_mouse();
                        break;
                    case (MOUSEEVENTF_RIGHTDOWN):
                        emit_mouse(EV_KEY, BTN_RIGHT, 1);
                        sync_mouse();
                        break;
                    case (MOUSEEVENTF_RIGHTUP):
                        emit_mouse(EV_KEY, BTN_RIGHT, 0);
                        sync_mouse();
                        break;
                    case (MOUSEEVENTF_WHEEL):
                        emit_mouse(EV_REL, REL_WHEEL, input.mi.mouseData);
                        sync_mouse();
                        break;
                    case (MOUSEEVENTF_HWHEEL):
                        emit_mouse(EV_REL, REL_HWHEEL, input.mi.mouseData);
                        sync_mouse();
                        break;
                    
                }
                
                break;
                
            case (INPUT_KEYBOARD):
                bool is_pressed = 0;
                if (input.ki.dwFlags == NULL){
                    is_pressed = 1;
                }
                emit(EV_KEY, winapi_to_linux_key(input.ki.wVk), is_pressed);
                break;
            default:
                return 1;

        }
    }
    return 0;
}
void GetSystemTime(SYSTEMTIME *lpSystemTime){
    time_t rawtime;
    time(&rawtime);
    struct tm *utc_time = gmtime(&rawtime);
    (*lpSystemTime).wDay = utc_time -> tm_mday;
    (*lpSystemTime).wHour = utc_time -> tm_hour;
    (*lpSystemTime).wMinute = utc_time -> tm_min;
    (*lpSystemTime).wSecond = utc_time -> tm_sec;
    (*lpSystemTime).wYear = utc_time -> tm_year + 1900;
    (*lpSystemTime).wMonth = utc_time -> tm_mon + 1;
    (*lpSystemTime).wDayOfWeek = utc_time -> tm_wday;
}

void GetLocalTime(SYSTEMTIME *lpSystemTime){
    time_t rawtime;
    time(&rawtime);
    struct tm *utc_time = localtime(&rawtime);
    (*lpSystemTime).wDay = utc_time -> tm_mday;
    (*lpSystemTime).wHour = utc_time -> tm_hour;
    (*lpSystemTime).wMinute = utc_time -> tm_min;
    (*lpSystemTime).wSecond = utc_time -> tm_sec;
    (*lpSystemTime).wYear = utc_time -> tm_year + 1900;
    (*lpSystemTime).wMonth = utc_time -> tm_mon + 1;
    (*lpSystemTime).wDayOfWeek = utc_time -> tm_wday;
}

UINT MapVirtualKeyA(UINT uCode, UINT uMapType){
    
    switch (uMapType)
    {
    case MAPVK_VK_TO_VSC:
        return winapi_to_linux_key(uCode);
    case MAPVK_VSC_TO_VK:
        return linux_to_winapi_key(uCode);
    case MAPVK_VK_TO_CHAR:
        return uCode;
    case MAPVK_VSC_TO_VK_EX:
        switch(uCode) {
            case 0x1D: 
                return VK_RCONTROL;
                
            case 0x38: 
                return VK_RMENU;
                
            case 0x2A: 
                return VK_LSHIFT;
                
            case 0x36:  
                return VK_RSHIFT;
                
            case 0x5B: 
                return VK_LWIN;
                
            case 0x5C: 
                return VK_RWIN;
                
            case 0x5D:  
                return VK_APPS;
        default:
            return MapVirtualKeyA(uCode, MAPVK_VSC_TO_VK);
    case MAPVK_VK_TO_VSC_EX:
            //todo
            return MapVirtualKeyA(uCode, MAPVK_VK_TO_VSC);
    }
    default:
        return -1;
    }
}

void mouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo){
    if (!is_initilized) MAIN_INIT();
    INPUT inp;
    inp.type = INPUT_MOUSE;
    inp.mi.dwFlags = dwFlags;
    inp.mi.dx = dx;
    inp.mi.dy = dy;
    inp.mi.mouseData = dwData;
    inp.mi.dwExtraInfo = dwExtraInfo;
    INPUT inps[1] = {inp};
    SendInput(1, inps, sizeof(INPUT));
}

void keybd_event(BYTE bVk, BYTE bScan, BYTE dwFlags, ULONG_PTR dwExtraInfo){
    if (!is_initilized) MAIN_INIT();
    INPUT inp;
    inp.type = INPUT_KEYBOARD;
    inp.ki.wVk = bVk;
    inp.ki.wScan = bScan;
    inp.ki.dwFlags = dwFlags;
    inp.ki.dwExtraInfo = dwExtraInfo;
    INPUT inps[1] = {inp};
    SendInput(1, inps, sizeof(INPUT));
}


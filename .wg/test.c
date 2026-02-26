#include <stdio.h>
#include "lib.h"

int main(){
    MAIN_INIT();

    SetCursorPos(500, 200);
 
    POINT p;
    GetCursorPos(&p);
    printf("GetCursorPos coords: %d %d \n", p.x, p.y);

    
    INPUT inputs[2];
    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, inputs, sizeof(INPUT));
    printf("Every End \n");
    MAIN_DESTROY();
    return 0;
    
}
#include <stdio.h>
#include "lib.h"


void* restrict_cursor(void* arg){
    RECT* rect  = (RECT*) arg;
    int right = rect -> right;
    int left = rect -> left;
    int bottom = rect -> bottom;
    int top = (*rect).top;
    printf("top: %d \n", rect->top);
    while (true){
        if (cursor_x > right){
            SetCursorPos(right, cursor_y);
            printf("right \n");
        }
        if (cursor_x < left){
            SetCursorPos(left, cursor_y);
            printf("left \n");
        }
        if (cursor_y < top){
            SetCursorPos(cursor_x, top);
            printf("top \n");
        }
        if (cursor_y > bottom){
            SetCursorPos(cursor_x, bottom);
            printf("bottom \n");
        }
    }
}

bool ClipCursor(RECT rect){
    printf("top1: %d \n", rect.top);
    RECT copy = rect;
    pthread_t clip_cursor_thread;
    pthread_create(&clip_cursor_thread, NULL, restrict_cursor, &copy);
    sleep(1);
    return 0;
}




int main(){
    MAIN_INIT();
    SetCursorPos(100, 200);
    MAIN_DESTROY();
}
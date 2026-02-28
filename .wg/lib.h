#include <stdio.h>
#include "getAbsPos.h"
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "absMove.h"
#include "keyboard.h"
#include "structures.h"

extern void MAIN_INIT(){
    init_tablet();
    init_layer_shell();
    init_virtual_mouse();
    initilize_keyboard();
    pthread_t cursor_pos_thread;
    pthread_create(&cursor_pos_thread, NULL, update_cursor_pos, NULL); 
    
}
extern void MAIN_DESTROY(){
    destroy_tablet();
    destroy_layer_shell();
    destroy_virtual_mouse();
    destroy_keyboard();
}

extern bool GetCursorPos(POINT *point){
    (*point).x = cursor_x;
    (*point).y = cursor_y;
    return 1;
}

extern UINT SendInput(UINT cInputs, INPUT inputs[], int cbSize){
    (void)cbSize;

    for (UINT i = 0; i < cInputs; i++){
        INPUT input = inputs[i];
        switch (input.type){
            case (0):
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
                
            case (1):
                bool is_pressed = 0;
                if (input.ki.dwFlags == 0){
                    is_pressed = 1;
                }
                printf("Pressing");
                emit(EV_KEY, input.ki.wVk, is_pressed);
                sleep(1);
                break;
            default:
                return 1;

        }
        sleep(1);
    }
    return 0;
}
extern void DeleteCriticalSection(void *critical_section){
    (void)critical_section;
}


extern void InitializeCriticalSection(void *critical_section){
    if (critical_section) {
        memset(critical_section, 0, 64);
    }
}

extern void EnterCriticalSection(void *critical_section){
    (void)critical_section;
}

extern void LeaveCriticalSection(void *critical_section){
    (void)critical_section;
}

extern unsigned long GetLastError(void){
    return (unsigned long)errno;
}

extern void *SetUnhandledExceptionFilter(void *filter){
    return filter;
}

extern void Sleep(unsigned long milliseconds){
    usleep((useconds_t)(milliseconds * 1000));
}

static __thread void *g_tls_slots[128];
extern void *TlsGetValue(unsigned long index){
    if (index >= 128) {
        return NULL;
    }
    return g_tls_slots[index];
}

#ifndef PAGE_NOACCESS
#define PAGE_NOACCESS          0x01
#define PAGE_READONLY          0x02
#define PAGE_READWRITE         0x04
#define PAGE_EXECUTE           0x10
#define PAGE_EXECUTE_READ      0x20
#define PAGE_EXECUTE_READWRITE 0x40
#endif

static int page_to_prot(unsigned long flProtect) {
    switch (flProtect & 0xFF) {
        case PAGE_NOACCESS: return PROT_NONE;
        case PAGE_READONLY: return PROT_READ;
        case PAGE_READWRITE: return PROT_READ | PROT_WRITE;
        case PAGE_EXECUTE: return PROT_EXEC;
        case PAGE_EXECUTE_READ: return PROT_READ | PROT_EXEC;
        case PAGE_EXECUTE_READWRITE: return PROT_READ | PROT_WRITE | PROT_EXEC;
        default: return PROT_READ | PROT_WRITE;
    }
}

extern int VirtualProtect(void *address, size_t size, unsigned long new_protect, unsigned long *old_protect){
    if (old_protect) {
        *old_protect = PAGE_READWRITE;
    }
    return mprotect(address, size, page_to_prot(new_protect)) == 0;
}

extern size_t VirtualQuery(const void *address, void *buffer, size_t length){
    (void)address;
    if (!buffer || length == 0) {
        return 0;
    }
    memset(buffer, 0, length);
    return length;
}


extern char **environ;

static int g_ucrt_argc = 0;
static char **g_ucrt_argv = NULL;
static int g_ucrt_fmode = 0;
static int g_ucrt_commode = 0;
static void *g_invalid_parameter_handler = NULL;

extern char ***__p__environ(void){
    return &environ;
}

extern int _set_new_mode(int mode){
    g_ucrt_commode = mode;
    return g_ucrt_commode;
}

extern int _configthreadlocale(int per_thread_locale_type){
    (void)per_thread_locale_type;
    return 0;
}

extern int __setusermatherr(void *handler){
    (void)handler;
    return 0;
}

extern void *__C_specific_handler(void){
    return NULL;
}

extern int *__p___argc(void){
    return &g_ucrt_argc;
}

extern char ***__p___argv(void){
    return &g_ucrt_argv;
}

extern void _cexit(void){
}

extern int _configure_narrow_argv(int mode){
    (void)mode;
    return 0;
}

extern int _crt_atexit(void (*fn)(void)){
    if (!fn) {
        return 0;
    }
    return atexit(fn);
}

extern int _initialize_narrow_environment(void){
    return 0;
}

extern void _set_app_type(int type){
    (void)type;
}

extern void _initterm(void (**start)(void), void (**end)(void)){
    if (!start || !end) {
        return;
    }
    for (void (**it)(void) = start; it < end; ++it) {
        if (*it) {
            (*it)();
        }
    }
}

extern int _initterm_e(int (**start)(void), int (**end)(void)){
    if (!start || !end) {
        return 0;
    }
    for (int (**it)(void) = start; it < end; ++it) {
        if (*it) {
            int rc = (*it)();
            if (rc != 0) {
                return rc;
            }
        }
    }
    return 0;
}

extern void *_set_invalid_parameter_handler(void *handler){
    void *old = g_invalid_parameter_handler;
    g_invalid_parameter_handler = handler;
    return old;
}

extern int *__p__commode(void){
    return &g_ucrt_commode;
}

extern int *__p__fmode(void){
    return &g_ucrt_fmode;
}

extern FILE *__acrt_iob_func(unsigned index){
    switch (index) {
        case 0: return stdin;
        case 1: return stdout;
        case 2: return stderr;
        default: return NULL;
    }
}

extern int __stdio_common_vfprintf(unsigned long long options, FILE *stream, const char *format, void *locale, va_list args){
    (void)options;
    (void)locale;
    if (!stream || !format) {
        errno = EINVAL;
        return -1;
    }
    return vfprintf(stream, format, args);
}

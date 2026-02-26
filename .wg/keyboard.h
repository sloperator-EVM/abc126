#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

int fd_k = -1;
struct uinput_setup usetup;

extern void destroy_keyboard(){
    ioctl(fd_k, UI_DEV_DESTROY);
    close(fd_k);
    return;
}

extern void emit(int type, int code, int val) {
    struct input_event ie;

    ie.type = type;
    ie.code = code;
    ie.value = val;
   
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;

    if (write(fd_k, &ie, sizeof(ie)) < 0) {
        perror("write event");
        exit(1);
    }
    ie.type = EV_SYN;
    ie.code = SYN_REPORT;
    ie.value = 0;
    write(fd_k, &ie, sizeof(ie));
}

extern int initilize_keyboard(){
    fd_k = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd_k < 0) {
        perror("open /dev/uinput");
        return 1;
    }
    if (ioctl(fd_k, UI_SET_EVBIT, EV_KEY) < 0) {
        perror("ioctl set evbit");
        close(fd_k);
        return 1;
    }
    ioctl(fd_k, UI_SET_EVBIT, EV_KEY);
    for (int key = 1; key <= 248; key++) {
        ioctl(fd_k, UI_SET_KEYBIT, key);
    }
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB; 
    usetup.id.vendor = 0x1234;    
    usetup.id.product = 0x5678;   
    strcpy(usetup.name, "Virtual Keyboard");

    if (ioctl(fd_k, UI_DEV_SETUP, &usetup) < 0) {
        perror("ioctl dev setup");
        close(fd_k);
        return 1;
    }
    
    if (ioctl(fd_k, UI_DEV_CREATE) < 0) {
        perror("ioctl dev create");
        close(fd_k);
        return 1;
    }
}

// extern int press_button(int PRESS_KEY){
//     emit(EV_KEY, PRESS_KEY, 1); 
//     emit(EV_SYN, SYN_REPORT, 0);
//     emit(EV_KEY, PRESS_KEY, 0); 
//     emit(EV_SYN, SYN_REPORT, 0); 
//     return 0;
// }
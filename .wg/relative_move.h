// move_cursor_uinput.c - COMPLETE WORKING CODE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <string.h>
#include <errno.h>
#include <time.h>

int fd;
struct input_event ev;

extern int init_virtual_mouse() {
    // Open uinput device
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open /dev/uinput");
        return -1;
    }
    
    // Enable mouse events
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);
    ioctl(fd, UI_SET_KEYBIT, BTN_MIDDLE);
    
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_RELBIT, REL_WHEEL);
    
    // Setup the uinput device
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "Virtual Mouse");
    
    // Create the virtual device
    if (ioctl(fd, UI_DEV_SETUP, &usetup) < 0) {
        perror("UI_DEV_SETUP failed");
        close(fd);
        fd = -1;
        return -1;
    }
    
    if (ioctl(fd, UI_DEV_CREATE) < 0) {
        perror("UI_DEV_CREATE failed");
        close(fd);
        fd = -1;
        return -1;
    }
    
    printf("Virtual mouse created successfully!\n");
    sleep(1);  // Give system time to recognize
    
    return 0;
}

// Clean up virtual mouse
extern void destroy_virtual_mouse() {
    if (fd >= 0) {
        ioctl(fd, UI_DEV_DESTROY);
        close(fd);
        fd = -1;
        printf("Virtual mouse destroyed.\n");
    }
}

int emit_mouse(int type, int code, int val){
    memset(&ev, 0, sizeof(ev));
    ev.type = type;
    ev.code = code;
    ev.value = val;
    write(fd, &ev, sizeof(ev));
    return 0;
}

int sync_mouse(){
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    
    write(fd, &ev, sizeof(ev));
    return 0;
}

// Move cursor using existing virtual mouse
extern void mouseMove(int rel_x, int rel_y) {
    if (fd < 0) {
        printf("Virtual mouse not initialized!\n");
        return;
    }
    printf("Moving cursor by %d, %d\n", rel_x, rel_y);
    
    // Create movement events
    
    // X movement
    emit_mouse(EV_REL, REL_X, rel_x);
    
    // Y movement
    emit_mouse(EV_REL, REL_Y, rel_y);
    
    // Sync event
    sync_mouse();
    // Small delay to ensure events are processed
    sleep(0);
}

extern void mouseClick(int button){
    emit_mouse(EV_KEY, button, 1);
    sync_mouse();
    
    emit_mouse(EV_KEY, button, 0);
    sync_mouse();

}

extern void DrawCircle(int step){
    int k = step;
    int del = 0;
    for (int i = 0; i < 3; i++){
        mouseMove(k, k);
        sleep(del);
        
    }
    for (int i = 0; i < 4; i++){
        mouseMove(k, 0);
        sleep(del);
    }
    for (int i = 0; i < 3; i++){
        mouseMove(k, -k);
        sleep(del);
    }
    for (int i = 0; i < 4; i++){
        mouseMove(0, -k);
        sleep(del);
    }
    for (int i = 0; i < 3; i++){
        mouseMove(-k, -k);
        sleep(del);
    }
    for (int i = 0; i < 4; i++){
        mouseMove(-k, 0);
        sleep(del);
    }
    for (int i = 0; i < 3; i++){
        mouseMove(-k, k);
        sleep(del);
    }
    for (int i = 0; i < 4; i++){
        mouseMove(0, k);
        sleep(del);
    }
    return;
}

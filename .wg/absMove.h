// virtual_tablet.c - Absolute positioning with virtual tablet
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/uinput.h>
#include <string.h>
#include <signal.h>
#include <math.h>

// Screen dimensions
#define SCREEN_WIDTH  1920
#define SCREEN_HEIGHT 1080

typedef struct {
    int fd;
    int screen_width;
    int screen_height;
    int is_pressed;
    int last_x, last_y;
    int pressure;
} Tablet;

Tablet tablet = { .fd = -1, .screen_width = SCREEN_WIDTH, 
                  .screen_height = SCREEN_HEIGHT, .is_pressed = 0,
                  .pressure = 0 };

extern void destroy_tablet() {
    if (tablet.fd >= 0) {
        if (tablet.is_pressed) {
            struct input_event ev;
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_KEY;
            ev.code = BTN_TOUCH;
            ev.value = 0;
            write(tablet.fd, &ev, sizeof(ev));
            
            memset(&ev, 0, sizeof(ev));
            ev.type = EV_SYN;
            ev.code = SYN_REPORT;
            ev.value = 0;
            write(tablet.fd, &ev, sizeof(ev));
        }
        
        ioctl(tablet.fd, UI_DEV_DESTROY);
        close(tablet.fd);
        tablet.fd = -1;
    }
    return;
}

extern int init_tablet() {
    if (tablet.fd >= 0) return 0;
    
    tablet.fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (tablet.fd < 0) {
        perror("Failed to open /dev/uinput");
        return -1;
    }
    
    
    ioctl(tablet.fd, UI_SET_EVBIT, EV_ABS);
    
    ioctl(tablet.fd, UI_SET_ABSBIT, ABS_X);
    struct uinput_abs_setup abs_x;
    memset(&abs_x, 0, sizeof(abs_x));
    abs_x.code = ABS_X;
    abs_x.absinfo.minimum = 0;
    abs_x.absinfo.maximum = tablet.screen_width - 1;
    abs_x.absinfo.resolution = 100;  
    ioctl(tablet.fd, UI_ABS_SETUP, &abs_x);
    
    ioctl(tablet.fd, UI_SET_ABSBIT, ABS_Y);
    struct uinput_abs_setup abs_y;
    memset(&abs_y, 0, sizeof(abs_y));
    abs_y.code = ABS_Y;
    abs_y.absinfo.minimum = 0;
    abs_y.absinfo.maximum = tablet.screen_height - 1;
    abs_y.absinfo.resolution = 100;
    ioctl(tablet.fd, UI_ABS_SETUP, &abs_y);
    
    ioctl(tablet.fd, UI_SET_ABSBIT, ABS_PRESSURE);
    struct uinput_abs_setup abs_pressure;
    memset(&abs_pressure, 0, sizeof(abs_pressure));
    abs_pressure.code = ABS_PRESSURE;
    abs_pressure.absinfo.minimum = 0;
    abs_pressure.absinfo.maximum = 1024;  
    ioctl(tablet.fd, UI_ABS_SETUP, &abs_pressure);
    
    
    ioctl(tablet.fd, UI_SET_EVBIT, EV_KEY);
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_TOUCH);      
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_STYLUS);     
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_STYLUS2);   
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_LEFT);      
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_RIGHT);      
    
    
    ioctl(tablet.fd, UI_SET_EVBIT, EV_SYN);
    
   
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x056A;      
    usetup.id.product = 0x00BC;     
    usetup.id.version = 1;
    strcpy(usetup.name, "Virtual Tablet");
    
    if (ioctl(tablet.fd, UI_DEV_SETUP, &usetup) < 0 ||
        ioctl(tablet.fd, UI_DEV_CREATE) < 0) {
        perror("Failed to create tablet");
        close(tablet.fd);
        tablet.fd = -1;
        return -1;
    }
    
    
    usleep(1000); 
    
    
    tablet.last_x = tablet.screen_width / 2;
    tablet.last_y = tablet.screen_height / 2;
    
    return 0;
}


void send_absolute(int x, int y) {
    if (tablet.fd < 0) return;
    
    struct input_event ev;
    
    
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    write(tablet.fd, &ev, sizeof(ev));
    
    
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    write(tablet.fd, &ev, sizeof(ev));
    
    
    
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(tablet.fd, &ev, sizeof(ev));
    
    tablet.last_x = x;
    tablet.last_y = y;
    
    usleep(1000); 
}










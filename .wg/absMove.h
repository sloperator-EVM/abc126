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

// Cleanup on exit
extern void destroy_tablet() {
    if (tablet.fd >= 0) {
        if (tablet.is_pressed) {
            // Lift pen
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
        printf("\nTablet destroyed\n");
    }
    return;
}

// Initialize virtual tablet
extern int init_tablet() {
    if (tablet.fd >= 0) return 0;
    
    printf("Creating virtual tablet (%dx%d)...\n", 
           tablet.screen_width, tablet.screen_height);
    
    // Open uinput
    tablet.fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (tablet.fd < 0) {
        perror("Failed to open /dev/uinput");
        return -1;
    }
    
    // ===== CRITICAL: Configure as ABSOLUTE TABLET device =====
    
    // 1. Enable absolute positioning
    ioctl(tablet.fd, UI_SET_EVBIT, EV_ABS);
    
    // 2. Setup X axis (0 to screen width)
    ioctl(tablet.fd, UI_SET_ABSBIT, ABS_X);
    struct uinput_abs_setup abs_x;
    memset(&abs_x, 0, sizeof(abs_x));
    abs_x.code = ABS_X;
    abs_x.absinfo.minimum = 0;
    abs_x.absinfo.maximum = tablet.screen_width - 1;
    abs_x.absinfo.resolution = 100;  // 100 units per mm
    ioctl(tablet.fd, UI_ABS_SETUP, &abs_x);
    
    // 3. Setup Y axis (0 to screen height)
    ioctl(tablet.fd, UI_SET_ABSBIT, ABS_Y);
    struct uinput_abs_setup abs_y;
    memset(&abs_y, 0, sizeof(abs_y));
    abs_y.code = ABS_Y;
    abs_y.absinfo.minimum = 0;
    abs_y.absinfo.maximum = tablet.screen_height - 1;
    abs_y.absinfo.resolution = 100;
    ioctl(tablet.fd, UI_ABS_SETUP, &abs_y);
    
    // 4. Setup pressure (for tablet pen)
    ioctl(tablet.fd, UI_SET_ABSBIT, ABS_PRESSURE);
    struct uinput_abs_setup abs_pressure;
    memset(&abs_pressure, 0, sizeof(abs_pressure));
    abs_pressure.code = ABS_PRESSURE;
    abs_pressure.absinfo.minimum = 0;
    abs_pressure.absinfo.maximum = 1024;  // 10-bit pressure
    ioctl(tablet.fd, UI_ABS_SETUP, &abs_pressure);
    
    // 5. Enable buttons
    ioctl(tablet.fd, UI_SET_EVBIT, EV_KEY);
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_TOUCH);      // Pen touch
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_STYLUS);     // Pen tip
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_STYLUS2);    // Pen button
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_LEFT);       // Left click
    ioctl(tablet.fd, UI_SET_KEYBIT, BTN_RIGHT);      // Right click
    
    // 6. Enable synchronization
    ioctl(tablet.fd, UI_SET_EVBIT, EV_SYN);
    
    // Create device
    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x056A;      // Wacom's vendor ID (makes it look real)
    usetup.id.product = 0x00BC;     // Random product ID
    usetup.id.version = 1;
    strcpy(usetup.name, "Virtual Tablet");
    
    if (ioctl(tablet.fd, UI_DEV_SETUP, &usetup) < 0 ||
        ioctl(tablet.fd, UI_DEV_CREATE) < 0) {
        perror("Failed to create tablet");
        close(tablet.fd);
        tablet.fd = -1;
        return -1;
    }
    
    // Wait for device to be recognized
    usleep(1000000);  // 1 second
    
    // Initialize at center
    tablet.last_x = tablet.screen_width / 2;
    tablet.last_y = tablet.screen_height / 2;
    
    return 0;
}

// Send absolute position event
void send_absolute(int x, int y, int pressure) {
    if (tablet.fd < 0) return;
    
    struct input_event ev;
    
    // X position (ABSOLUTE)
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS;
    ev.code = ABS_X;
    ev.value = x;
    write(tablet.fd, &ev, sizeof(ev));
    
    // Y position (ABSOLUTE)
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_ABS;
    ev.code = ABS_Y;
    ev.value = y;
    write(tablet.fd, &ev, sizeof(ev));
    
    // Pressure (optional)
    if (pressure >= 0) {
        memset(&ev, 0, sizeof(ev));
        ev.type = EV_ABS;
        ev.code = ABS_PRESSURE;
        ev.value = pressure;
        write(tablet.fd, &ev, sizeof(ev));
        tablet.pressure = pressure;
    }
    
    // Sync
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(tablet.fd, &ev, sizeof(ev));
    
    tablet.last_x = x;
    tablet.last_y = y;
    
    usleep(1000);  // 10ms delay
}

// Move to absolute position (NO relative calculation!)
extern void SetCursorPos(int x, int y) {
    if (tablet.fd < 0) {
        printf("Tablet not initialized!\n");
        return;
    }
    
    // Validate
    if (x < 0 || x >= tablet.screen_width || 
        y < 0 || y >= tablet.screen_height) {
        printf("Position (%d, %d) out of bounds (0-%d, 0-%d)\n",
               x, y, tablet.screen_width-1, tablet.screen_height-1);
        return;
    }
    
    printf("Moving to ABSOLUTE position: (%d, %d)\n", x, y);
    send_absolute(x, y, -1);  // -1 = keep current pressure
}

// Pen down (start drawing/clicking)
void pen_down(int pressure) {
    if (tablet.fd < 0 || tablet.is_pressed) return;
    
    printf("Pen DOWN at (%d, %d), pressure: %d\n", 
           tablet.last_x, tablet.last_y, pressure);
    
    struct input_event ev;
    
    // Set pen tip button
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = BTN_TOUCH;
    ev.value = 1;
    write(tablet.fd, &ev, sizeof(ev));
    
    // Set stylus
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = BTN_STYLUS;
    ev.value = 1;
    write(tablet.fd, &ev, sizeof(ev));
    
    // Update pressure
    send_absolute(tablet.last_x, tablet.last_y, pressure);
    
    tablet.is_pressed = 1;
    tablet.pressure = pressure;
}

// Pen up (stop drawing/clicking)
void pen_up() {
    if (tablet.fd < 0 || !tablet.is_pressed) return;
    
    printf("Pen UP at (%d, %d)\n", tablet.last_x, tablet.last_y);
    
    struct input_event ev;
    
    // Release pen tip
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = BTN_TOUCH;
    ev.value = 0;
    write(tablet.fd, &ev, sizeof(ev));
    
    // Release stylus
    memset(&ev, 0, sizeof(ev));
    ev.type = EV_KEY;
    ev.code = BTN_STYLUS;
    ev.value = 0;
    write(tablet.fd, &ev, sizeof(ev));
    
    // Zero pressure
    send_absolute(tablet.last_x, tablet.last_y, 0);
    
    tablet.is_pressed = 0;
    tablet.pressure = 0;
}


// Click at absolute position




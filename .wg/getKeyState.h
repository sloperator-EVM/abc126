#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <libinput.h>
#include <libudev.h>
#include <linux/input-event-codes.h>
#include <stdbool.h>
#include "structures.h"

#define KEY_G_CODE 34
bool is_button_pressed[256];
struct udev *udev = NULL;
int fd_linp;




static int running_libinp = 1;
static struct libinput *li = NULL;

void print_curr_pressed_buttons(){
    for (int i = 0; i < 256; i++){
        if (is_button_pressed[i]){
            printf("%d ", i);
        }
    }
    printf("\n");
    return;
}

void signal_handler(int sig) {
    (void)sig;  
    running_libinp = 0;
}

static int open_restricted(const char *path, int flags, void *user_data) {
    (void)user_data;  
    int fd_linp = open(path, flags);
    return fd_linp < 0 ? -fd_linp : fd_linp;
}

static void close_restricted(int fd_linp, void *user_data) {
    (void)user_data;  
    close(fd_linp);
}

static const struct libinput_interface interface = {
    .open_restricted = open_restricted,
    .close_restricted = close_restricted,
};

static void handle_events(struct libinput *li) {
    struct libinput_event *ev;
    
    while ((ev = libinput_get_event(li)) != NULL) {
        enum libinput_event_type type = libinput_event_get_type(ev);
        
        if (type == LIBINPUT_EVENT_KEYBOARD_KEY) {
            struct libinput_event_keyboard *key_ev = libinput_event_get_keyboard_event(ev);
            uint32_t key = libinput_event_keyboard_get_key(key_ev);
            enum libinput_key_state key_state = libinput_event_keyboard_get_key_state(key_ev);
            is_button_pressed[key] = (key_state == LIBINPUT_KEY_STATE_PRESSED);
            //print_curr_pressed_buttons();
        }
        
        libinput_event_destroy(ev);
    }
}

int init_libinput(){

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    udev = udev_new();
    if (!udev) {
        fprintf(stderr, "Failed to initialize udev\n");
        return 1;
    }
    li = libinput_udev_create_context(&interface, NULL, udev);
    if (!li) {
        fprintf(stderr, "Failed to create libinput context\n");
        udev_unref(udev);
        return 1;
    }
    if (libinput_udev_assign_seat(li, "seat0") != 0) {
        fprintf(stderr, "Failed to assign seat\n");
        libinput_unref(li);
        udev_unref(udev);
        return 1;
    }
    fd_linp = libinput_get_fd(li);
    return 0;
}

void destroy_libinput(){
    if (li) {
        libinput_unref(li);
    }
    if (udev) {
        udev_unref(udev);
    }
    return;
}

void *check_buttons(void *arg){
    while (running_libinp) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd_linp, &fds);
        
        struct timeval tv = {1, 0};
        int ret = select(fd_linp + 1, &fds, NULL, NULL, &tv);
        
        if (ret > 0 && FD_ISSET(fd_linp, &fds)) {
            libinput_dispatch(li);
            handle_events(li);
        } else if (ret == 0) {
            continue;
        } else {
            break;
        }
    }
    return 0;
}

short GetKeyState(int nVirtKey){
    short linux_key = winapi_to_linux_key(nVirtKey);
    if (is_button_pressed[linux_key]){
        return 0x8000;
    }
    return 0x0000;
}

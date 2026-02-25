#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client-protocol.h>
#include <wayland-client.h>

#include "single-pixel-buffer-v1.h"
#include "wlr-layer-shell-unstable-v1.h"
#include "wlr-virtual-pointer-unstable-v1.h"
#include "viewporter.h"


// for arguments
#include "arg.h"

int main(){
    zwlr_virtual_pointer_manager_v1_interface;
    printf("You can live for now");
    return 0;
}

#include "winrun/loader.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool env_debug_enabled(void) {
    const char *debug = getenv("WINRUN_DEBUG");
    return debug != NULL && strcmp(debug, "0") != 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s <windows-exe> [args...]\n", argv[0]);
        return 2;
    }

    bool debug = env_debug_enabled();
    install_signal_handlers(debug);

    pe_image image;
    int rc = pe_parse_image(argv[1], &image);
    if (rc != 0) {
        fprintf(stderr, "failed to parse PE image '%s' (rc=%d)\n", argv[1], rc);
        return 1;
    }

    mapped_image mapped;
    rc = map_pe_image(&image, &mapped);
    if (rc != 0) {
        fprintf(stderr, "failed to map image (rc=%d)\n", rc);
        pe_destroy_image(&image);
        return 1;
    }

    rc = apply_relocations(&image, &mapped);
    if (rc != 0 && mapped.actual_base != mapped.preferred_base) {
        fprintf(stderr, "failed to apply relocations (rc=%d)\n", rc);
        unmap_pe_image(&mapped);
        pe_destroy_image(&image);
        return 1;
    }

    void *winapi = dlopen("./lib/libwinapi.so", RTLD_NOW | RTLD_LOCAL);
    if (!winapi) {
        fprintf(stderr, "warning: could not load ./lib/libwinapi.so: %s\n", dlerror());
    }

    import_resolver resolver = {
        .default_winapi_lib = winapi,
        .debug = debug,
    };

    rc = resolve_imports(&image, &mapped, &resolver);
    if (rc != 0) {
        unmap_pe_image(&mapped);
        pe_destroy_image(&image);
        if (winapi) {
            dlclose(winapi);
        }
        return 1;
    }

    rc = initialize_tls(&image, &mapped);
    if (rc != 0) {
        fprintf(stderr, "failed to initialize TLS (rc=%d)\n", rc);
        unmap_pe_image(&mapped);
        pe_destroy_image(&image);
        if (winapi) {
            dlclose(winapi);
        }
        return 1;
    }

    int app_rc = execute_entry_point(&image, &mapped, argc - 1, &argv[1]);

    unmap_pe_image(&mapped);
    pe_destroy_image(&image);
    if (winapi) {
        dlclose(winapi);
    }

    return app_rc;
}

#include "winrun/loader.h"

#include <dlfcn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static bool env_debug_enabled(void) {
    const char *debug = getenv("WINRUN_DEBUG");
    return debug != NULL && strcmp(debug, "0") != 0;
}

static bool file_exists(const char *path) {
    return access(path, R_OK) == 0;
}

static const char *resolve_winapi_lib_path(void) {
    const char *env_path = getenv("WINRUN_WINAPI_LIB");
    if (env_path && env_path[0] != '\0') {
        return env_path;
    }

    static const char *candidates[] = {
        "./lib/libwinapi.so",
        "./.wg/libwinapi.so",
        "./build/lib/libwinapi.so",
    };

    for (size_t i = 0; i < sizeof(candidates) / sizeof(candidates[0]); ++i) {
        if (file_exists(candidates[i])) {
            return candidates[i];
        }
    }

    return "./lib/libwinapi.so";
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

    const char *winapi_lib_path = resolve_winapi_lib_path();
    void *winapi = dlopen(winapi_lib_path, RTLD_NOW | RTLD_LOCAL);
    if (!winapi) {
        fprintf(stderr, "warning: could not load WinAPI library '%s': %s\n", winapi_lib_path, dlerror());
    } else if (debug) {
        fprintf(stderr, "winrun: loaded WinAPI shim from %s\n", winapi_lib_path);
    }

    set_loader_runtime_args(argc - 1, &argv[1]);

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

    const char *strict_prot_env = getenv("WINRUN_STRICT_PROT");
    bool strict_prot = strict_prot_env && strcmp(strict_prot_env, "0") != 0;
    if (strict_prot) {
        rc = protect_pe_image_sections(&image, &mapped);
        if (rc != 0) {
            fprintf(stderr, "failed to apply section protections (rc=%d)\n", rc);
            unmap_pe_image(&mapped);
            pe_destroy_image(&image);
            if (winapi) {
                dlclose(winapi);
            }
            return 1;
        }
    }

    int app_rc = execute_entry_point(&image, &mapped, argc - 1, &argv[1]);

    unmap_pe_image(&mapped);
    pe_destroy_image(&image);
    if (winapi) {
        dlclose(winapi);
    }

    return app_rc;
}

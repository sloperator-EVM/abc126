#include "winrun/loader.h"

#include <stdint.h>
#include <stdio.h>

#define DLL_PROCESS_ATTACH 1

typedef void (*tls_callback_t)(void *dll_handle, uint32_t reason, void *reserved);

int initialize_tls(const pe_image *image, mapped_image *mapped) {
    pe_data_directory dir = image->data_directories[IMAGE_DIRECTORY_ENTRY_TLS];
    if (dir.virtual_address == 0 || dir.size == 0) {
        return 0;
    }

    if (image->is_pe64) {
        pe_tls_directory64 *tls = (pe_tls_directory64 *)(mapped->base + dir.virtual_address);
        uint64_t callbacks_va = tls->address_of_callbacks;
        if (callbacks_va == 0) {
            return 0;
        }

        tls_callback_t *callbacks = (tls_callback_t *)(uintptr_t)callbacks_va;
        while (*callbacks) {
            (*callbacks)((void *)mapped->actual_base, DLL_PROCESS_ATTACH, NULL);
            callbacks++;
        }
    } else {
        pe_tls_directory32 *tls = (pe_tls_directory32 *)(mapped->base + dir.virtual_address);
        uint32_t callbacks_va = tls->address_of_callbacks;
        if (callbacks_va == 0) {
            return 0;
        }

        tls_callback_t *callbacks = (tls_callback_t *)(uintptr_t)callbacks_va;
        while (*callbacks) {
            (*callbacks)((void *)(uintptr_t)mapped->actual_base, DLL_PROCESS_ATTACH, NULL);
            callbacks++;
        }
    }

    return 0;
}

int execute_entry_point(const pe_image *image, mapped_image *mapped, int argc, char **argv) {
    (void)argc;
    (void)argv;

    uint8_t *entry = mapped->base + image->address_of_entry_point;
    int (*entry_fn)(void) = (int (*)(void))(uintptr_t)entry;

    fprintf(stderr, "jumping to entry point: 0x%llx\n",
            (unsigned long long)(mapped->actual_base + image->address_of_entry_point));

    return entry_fn();
}

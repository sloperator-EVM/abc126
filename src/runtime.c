#include "winrun/loader.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#define DLL_PROCESS_ATTACH 1

typedef void (*tls_callback_t)(void *dll_handle, uint32_t reason, void *reserved);

static void *va_to_ptr(const mapped_image *mapped, uint64_t va, size_t size) {
    if (va >= mapped->actual_base && va - mapped->actual_base + size <= mapped->size) {
        return mapped->base + (va - mapped->actual_base);
    }

    if (va + size <= mapped->size) {
        return mapped->base + va;
    }

    return NULL;
}

int initialize_tls(const pe_image *image, mapped_image *mapped) {
    pe_data_directory dir = image->data_directories[IMAGE_DIRECTORY_ENTRY_TLS];
    if (dir.virtual_address == 0 || dir.size == 0) {
        return 0;
    }

    if (image->is_pe64) {
        pe_tls_directory64 *tls = mapped_rva_to_ptr(mapped, dir.virtual_address, sizeof(*tls));
        if (!tls) {
            return -EINVAL;
        }

        if (tls->address_of_callbacks == 0) {
            return 0;
        }

        for (size_t i = 0;; ++i) {
            tls_callback_t *slot = va_to_ptr(mapped, tls->address_of_callbacks + i * sizeof(uint64_t), sizeof(uint64_t));
            if (!slot) {
                return -EINVAL;
            }
            if (*slot == NULL) {
                break;
            }
            (*slot)((void *)(uintptr_t)mapped->actual_base, DLL_PROCESS_ATTACH, NULL);
        }
    } else {
        pe_tls_directory32 *tls = mapped_rva_to_ptr(mapped, dir.virtual_address, sizeof(*tls));
        if (!tls) {
            return -EINVAL;
        }

        if (tls->address_of_callbacks == 0) {
            return 0;
        }

        for (size_t i = 0;; ++i) {
            tls_callback_t *slot = va_to_ptr(mapped, (uint64_t)tls->address_of_callbacks + i * sizeof(uint32_t), sizeof(uint32_t));
            if (!slot) {
                return -EINVAL;
            }
            if (*slot == NULL) {
                break;
            }
            (*slot)((void *)(uintptr_t)mapped->actual_base, DLL_PROCESS_ATTACH, NULL);
        }
    }

    return 0;
}

int execute_entry_point(const pe_image *image, mapped_image *mapped, int argc, char **argv) {
    (void)argc;
    (void)argv;

    if (image->address_of_entry_point >= mapped->size) {
        return -EINVAL;
    }

    uint8_t *entry = mapped->base + image->address_of_entry_point;
    int (*entry_fn)(void) = (int (*)(void))(uintptr_t)entry;

    fprintf(stderr, "jumping to entry point: 0x%llx\n",
            (unsigned long long)(mapped->actual_base + image->address_of_entry_point));

    return entry_fn();
}

#include "winrun/loader.h"

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>

static void *resolve_symbol(import_resolver *resolver, const char *dll, const char *name) {
    (void)dll;
    if (!resolver->default_winapi_lib) {
        return NULL;
    }
    return dlsym(resolver->default_winapi_lib, name);
}

int resolve_imports(const pe_image *image, mapped_image *mapped, import_resolver *resolver) {
    pe_data_directory dir = image->data_directories[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (dir.virtual_address == 0 || dir.size == 0) {
        return 0;
    }

    pe_import_descriptor *desc = (pe_import_descriptor *)(mapped->base + dir.virtual_address);
    for (; desc->name != 0; ++desc) {
        const char *dll_name = (const char *)(mapped->base + desc->name);
        uint64_t thunk_rva = desc->original_first_thunk ? desc->original_first_thunk : desc->first_thunk;

        if (image->is_pe64) {
            uint64_t *lookup = (uint64_t *)(mapped->base + thunk_rva);
            uint64_t *iat = (uint64_t *)(mapped->base + desc->first_thunk);
            for (; *lookup != 0; ++lookup, ++iat) {
                if ((*lookup & (1ULL << 63)) != 0) {
                    continue;
                }
                uint8_t *import_by_name = mapped->base + (uint32_t)(*lookup);
                const char *func_name = (const char *)(import_by_name + 2);
                void *fn = resolve_symbol(resolver, dll_name, func_name);
                if (!fn) {
                    fprintf(stderr, "unresolved import: %s!%s\n", dll_name, func_name);
                    return -1;
                }
                *iat = (uint64_t)(uintptr_t)fn;
            }
        } else {
            uint32_t *lookup = (uint32_t *)(mapped->base + thunk_rva);
            uint32_t *iat = (uint32_t *)(mapped->base + desc->first_thunk);
            for (; *lookup != 0; ++lookup, ++iat) {
                if ((*lookup & (1U << 31)) != 0) {
                    continue;
                }
                uint8_t *import_by_name = mapped->base + *lookup;
                const char *func_name = (const char *)(import_by_name + 2);
                void *fn = resolve_symbol(resolver, dll_name, func_name);
                if (!fn) {
                    fprintf(stderr, "unresolved import: %s!%s\n", dll_name, func_name);
                    return -1;
                }
                *iat = (uint32_t)(uintptr_t)fn;
            }
        }
    }

    return 0;
}

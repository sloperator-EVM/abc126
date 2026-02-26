#include "winrun/loader.h"

#include <dlfcn.h>
#include <errno.h>
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
    if (dir.virtual_address == 0 || dir.size < sizeof(pe_import_descriptor)) {
        return 0;
    }

    pe_import_descriptor *desc = mapped_rva_to_ptr(mapped, dir.virtual_address, dir.size);
    if (!desc) {
        return -EINVAL;
    }

    size_t max_desc = dir.size / sizeof(pe_import_descriptor);
    for (size_t d = 0; d < max_desc && desc[d].name != 0; ++d) {
        pe_import_descriptor *cur = &desc[d];

        const char *dll_name = mapped_rva_to_ptr(mapped, cur->name, 1);
        if (!dll_name) {
            return -EINVAL;
        }

        uint64_t thunk_rva = cur->original_first_thunk ? cur->original_first_thunk : cur->first_thunk;
        if (thunk_rva == 0 || cur->first_thunk == 0) {
            continue;
        }

        if (image->is_pe64) {
            uint64_t *lookup = mapped_rva_to_ptr(mapped, (uint32_t)thunk_rva, sizeof(uint64_t));
            uint64_t *iat = mapped_rva_to_ptr(mapped, cur->first_thunk, sizeof(uint64_t));
            if (!lookup || !iat) {
                return -EINVAL;
            }

            for (size_t i = 0;; ++i) {
                uint64_t *lk = mapped_rva_to_ptr(mapped, (uint32_t)thunk_rva + (uint32_t)(i * sizeof(uint64_t)), sizeof(uint64_t));
                uint64_t *out = mapped_rva_to_ptr(mapped, cur->first_thunk + (uint32_t)(i * sizeof(uint64_t)), sizeof(uint64_t));
                if (!lk || !out) {
                    return -EINVAL;
                }
                if (*lk == 0) {
                    break;
                }
                if ((*lk & (1ULL << 63)) != 0) {
                    continue;
                }

                uint32_t ibn_rva = (uint32_t)(*lk);
                uint8_t *import_by_name = mapped_rva_to_ptr(mapped, ibn_rva, 3);
                if (!import_by_name) {
                    return -EINVAL;
                }

                const char *func_name = (const char *)(import_by_name + 2);
                void *fn = resolve_symbol(resolver, dll_name, func_name);
                if (!fn) {
                    fprintf(stderr, "unresolved import: %s!%s\n", dll_name, func_name);
                    return -ENOENT;
                }
                *out = (uint64_t)(uintptr_t)fn;
            }
        } else {
            for (size_t i = 0;; ++i) {
                uint32_t *lk = mapped_rva_to_ptr(mapped, (uint32_t)thunk_rva + (uint32_t)(i * sizeof(uint32_t)), sizeof(uint32_t));
                uint32_t *out = mapped_rva_to_ptr(mapped, cur->first_thunk + (uint32_t)(i * sizeof(uint32_t)), sizeof(uint32_t));
                if (!lk || !out) {
                    return -EINVAL;
                }
                if (*lk == 0) {
                    break;
                }
                if ((*lk & (1U << 31)) != 0) {
                    continue;
                }

                uint8_t *import_by_name = mapped_rva_to_ptr(mapped, *lk, 3);
                if (!import_by_name) {
                    return -EINVAL;
                }

                const char *func_name = (const char *)(import_by_name + 2);
                void *fn = resolve_symbol(resolver, dll_name, func_name);
                if (!fn) {
                    fprintf(stderr, "unresolved import: %s!%s\n", dll_name, func_name);
                    return -ENOENT;
                }
                *out = (uint32_t)(uintptr_t)fn;
            }
        }
    }

    return 0;
}

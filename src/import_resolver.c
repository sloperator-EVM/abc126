#include "winrun/loader.h"

#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern char **environ;

static int g_loader_argc = 0;
static char **g_loader_argv = NULL;
static int g_loader_fmode = 0;
static int g_loader_commode = 0;

void set_loader_runtime_args(int argc, char **argv) {
    g_loader_argc = argc;
    g_loader_argv = argv;
}

static char ***builtin___p__environ(void) { return &environ; }
static int *builtin___p___argc(void) { return &g_loader_argc; }
static char ***builtin___p___argv(void) { return &g_loader_argv; }
static int *builtin___p__fmode(void) { return &g_loader_fmode; }
static int *builtin___p__commode(void) { return &g_loader_commode; }

static int builtin__set_new_mode(int mode) { g_loader_commode = mode; return g_loader_commode; }
static int builtin__configthreadlocale(int mode) { (void)mode; return 0; }
static int builtin___setusermatherr(void *handler) { (void)handler; return 0; }
static void *builtin___C_specific_handler(void) { return NULL; }
static void builtin__cexit(void) {}
static int builtin__configure_narrow_argv(int mode) { (void)mode; return 0; }
static int builtin__crt_atexit(void (*fn)(void)) { return fn ? atexit(fn) : 0; }
static int builtin__initialize_narrow_environment(void) { return 0; }
static void builtin__set_app_type(int type) { (void)type; }
static void *builtin__set_invalid_parameter_handler(void *handler) { return handler; }

static FILE *builtin___acrt_iob_func(unsigned idx) {
    switch (idx) {
        case 0: return stdin;
        case 1: return stdout;
        case 2: return stderr;
        default: return NULL;
    }
}

static int builtin___stdio_common_vfprintf(unsigned long long options, FILE *stream, const char *format, void *locale, va_list args) {
    (void)options;
    (void)locale;
    if (!stream || !format) {
        errno = EINVAL;
        return -1;
    }
    return vfprintf(stream, format, args);
}

static void *resolve_builtin_symbol(const char *name) {
    if (strcmp(name, "__p__environ") == 0) return (void *)(uintptr_t)builtin___p__environ;
    if (strcmp(name, "__p___argc") == 0) return (void *)(uintptr_t)builtin___p___argc;
    if (strcmp(name, "__p___argv") == 0) return (void *)(uintptr_t)builtin___p___argv;
    if (strcmp(name, "__p__fmode") == 0) return (void *)(uintptr_t)builtin___p__fmode;
    if (strcmp(name, "__p__commode") == 0) return (void *)(uintptr_t)builtin___p__commode;
    if (strcmp(name, "_set_new_mode") == 0) return (void *)(uintptr_t)builtin__set_new_mode;
    if (strcmp(name, "_configthreadlocale") == 0) return (void *)(uintptr_t)builtin__configthreadlocale;
    if (strcmp(name, "__setusermatherr") == 0) return (void *)(uintptr_t)builtin___setusermatherr;
    if (strcmp(name, "__C_specific_handler") == 0) return (void *)(uintptr_t)builtin___C_specific_handler;
    if (strcmp(name, "_cexit") == 0) return (void *)(uintptr_t)builtin__cexit;
    if (strcmp(name, "_configure_narrow_argv") == 0) return (void *)(uintptr_t)builtin__configure_narrow_argv;
    if (strcmp(name, "_crt_atexit") == 0) return (void *)(uintptr_t)builtin__crt_atexit;
    if (strcmp(name, "_initialize_narrow_environment") == 0) return (void *)(uintptr_t)builtin__initialize_narrow_environment;
    if (strcmp(name, "_set_app_type") == 0) return (void *)(uintptr_t)builtin__set_app_type;
    if (strcmp(name, "_set_invalid_parameter_handler") == 0) return (void *)(uintptr_t)builtin__set_invalid_parameter_handler;
    if (strcmp(name, "__acrt_iob_func") == 0) return (void *)(uintptr_t)builtin___acrt_iob_func;
    if (strcmp(name, "__stdio_common_vfprintf") == 0) return (void *)(uintptr_t)builtin___stdio_common_vfprintf;
    return NULL;
}

static void *resolve_symbol(import_resolver *resolver, const char *dll, const char *name) {
    (void)dll;

    if (resolver->default_winapi_lib) {
        void *fn = dlsym(resolver->default_winapi_lib, name);
        if (fn) {
            return fn;
        }
    }

    void *builtin = resolve_builtin_symbol(name);
    if (builtin) {
        return builtin;
    }

    return dlsym(RTLD_DEFAULT, name);
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

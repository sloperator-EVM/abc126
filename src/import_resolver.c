#include "winrun/loader.h"

#include <dlfcn.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__)
#define WINRUN_MS_ABI __attribute__((ms_abi))
#else
#define WINRUN_MS_ABI
#endif

extern char **environ;

static int g_loader_argc = 0;
static char **g_loader_argv = NULL;
static int g_loader_fmode = 0;
static int g_loader_commode = 0;

void set_loader_runtime_args(int argc, char **argv) {
    g_loader_argc = argc;
    g_loader_argv = argv;
}

static char ***WINRUN_MS_ABI builtin___p__environ(void) { return &environ; }
static int *WINRUN_MS_ABI builtin___p___argc(void) { return &g_loader_argc; }
static char ***WINRUN_MS_ABI builtin___p___argv(void) { return &g_loader_argv; }
static int *WINRUN_MS_ABI builtin___p__fmode(void) { return &g_loader_fmode; }
static int *WINRUN_MS_ABI builtin___p__commode(void) { return &g_loader_commode; }

static int WINRUN_MS_ABI builtin__set_new_mode(int mode) { g_loader_commode = mode; return g_loader_commode; }
static int WINRUN_MS_ABI builtin__configthreadlocale(int mode) { (void)mode; return 0; }
static int WINRUN_MS_ABI builtin___setusermatherr(void *handler) { (void)handler; return 0; }
static void *WINRUN_MS_ABI builtin___C_specific_handler(void) { return NULL; }
static void WINRUN_MS_ABI builtin__cexit(void) {}
static int WINRUN_MS_ABI builtin__configure_narrow_argv(int mode) { (void)mode; return 0; }
static int WINRUN_MS_ABI builtin__crt_atexit(void (*fn)(void)) { return fn ? atexit(fn) : 0; }
static int WINRUN_MS_ABI builtin__initialize_narrow_environment(void) { return 0; }
static void WINRUN_MS_ABI builtin__set_app_type(int type) { (void)type; }
static void *WINRUN_MS_ABI builtin__set_invalid_parameter_handler(void *handler) { return handler; }
static void WINRUN_MS_ABI builtin__initterm(void (**start)(void), void (**end)(void)) {
    if (!start || !end) {
        return;
    }
    for (void (**it)(void) = start; it < end; ++it) {
        if (*it) {
            (*it)();
        }
    }
}
static int WINRUN_MS_ABI builtin__initterm_e(int (**start)(void), int (**end)(void)) {
    if (!start || !end) {
        return 0;
    }
    for (int (**it)(void) = start; it < end; ++it) {
        if (*it) {
            int rc = (*it)();
            if (rc != 0) {
                return rc;
            }
        }
    }
    return 0;
}

static FILE *WINRUN_MS_ABI builtin___acrt_iob_func(unsigned idx) {
    switch (idx) {
        case 0: return stdin;
        case 1: return stdout;
        case 2: return stderr;
        default: return NULL;
    }
}

static int WINRUN_MS_ABI builtin___stdio_common_vfprintf(unsigned long long options, FILE *stream, const char *format, void *locale, va_list args) {
    (void)options;
    (void)locale;
    if (!stream || !format) {
        errno = EINVAL;
        return -1;
    }
    return vfprintf(stream, format, args);
}

static void *g_malloc_impl = NULL;
static void *g_calloc_impl = NULL;
static void *g_free_impl = NULL;
static void *g_memcpy_impl = NULL;
static void *g_strlen_impl = NULL;
static void *g_strncmp_impl = NULL;
static void *g_puts_impl = NULL;
static void *g_signal_impl = NULL;
static void *g_abort_impl = NULL;
static void *g_exit_impl = NULL;
static void *g__exit_impl = NULL;

static void *WINRUN_MS_ABI bridge_malloc(size_t size) {
    void *(*fn)(size_t) = (void *(*)(size_t))g_malloc_impl;
    return fn ? fn(size) : NULL;
}
static void *WINRUN_MS_ABI bridge_calloc(size_t n, size_t size) {
    void *(*fn)(size_t, size_t) = (void *(*)(size_t, size_t))g_calloc_impl;
    return fn ? fn(n, size) : NULL;
}
static void WINRUN_MS_ABI bridge_free(void *ptr) {
    void (*fn)(void *) = (void (*)(void *))g_free_impl;
    if (fn) {
        fn(ptr);
    }
}
static void *WINRUN_MS_ABI bridge_memcpy(void *dst, const void *src, size_t size) {
    void *(*fn)(void *, const void *, size_t) = (void *(*)(void *, const void *, size_t))g_memcpy_impl;
    return fn ? fn(dst, src, size) : NULL;
}
static size_t WINRUN_MS_ABI bridge_strlen(const char *s) {
    size_t (*fn)(const char *) = (size_t (*)(const char *))g_strlen_impl;
    return fn ? fn(s) : 0;
}
static int WINRUN_MS_ABI bridge_strncmp(const char *lhs, const char *rhs, size_t n) {
    int (*fn)(const char *, const char *, size_t) = (int (*)(const char *, const char *, size_t))g_strncmp_impl;
    return fn ? fn(lhs, rhs, n) : -1;
}
static int WINRUN_MS_ABI bridge_puts(const char *s) {
    int (*fn)(const char *) = (int (*)(const char *))g_puts_impl;
    return fn ? fn(s) : -1;
}
static void *WINRUN_MS_ABI bridge_signal(int signum, void *handler) {
    void *(*fn)(int, void *) = (void *(*)(int, void *))g_signal_impl;
    return fn ? fn(signum, handler) : SIG_ERR;
}
static void WINRUN_MS_ABI bridge_abort(void) {
    void (*fn)(void) = (void (*)(void))g_abort_impl;
    if (fn) {
        fn();
    }
    raise(SIGABRT);
}
static void WINRUN_MS_ABI bridge_exit(int code) {
    void (*fn)(int) = (void (*)(int))g_exit_impl;
    if (fn) {
        fn(code);
    }
    _Exit(code);
}
static void WINRUN_MS_ABI bridge__exit(int code) {
    void (*fn)(int) = (void (*)(int))g__exit_impl;
    if (fn) {
        fn(code);
    }
    _Exit(code);
}

static void *bridge_sysv_symbol(const char *name, void *target) {
    if (!target || !name) {
        return NULL;
    }
    if (strcmp(name, "malloc") == 0) {
        g_malloc_impl = target;
        return (void *)(uintptr_t)bridge_malloc;
    }
    if (strcmp(name, "calloc") == 0) {
        g_calloc_impl = target;
        return (void *)(uintptr_t)bridge_calloc;
    }
    if (strcmp(name, "free") == 0) {
        g_free_impl = target;
        return (void *)(uintptr_t)bridge_free;
    }
    if (strcmp(name, "memcpy") == 0) {
        g_memcpy_impl = target;
        return (void *)(uintptr_t)bridge_memcpy;
    }
    if (strcmp(name, "strlen") == 0) {
        g_strlen_impl = target;
        return (void *)(uintptr_t)bridge_strlen;
    }
    if (strcmp(name, "strncmp") == 0) {
        g_strncmp_impl = target;
        return (void *)(uintptr_t)bridge_strncmp;
    }
    if (strcmp(name, "puts") == 0) {
        g_puts_impl = target;
        return (void *)(uintptr_t)bridge_puts;
    }
    if (strcmp(name, "signal") == 0) {
        g_signal_impl = target;
        return (void *)(uintptr_t)bridge_signal;
    }
    if (strcmp(name, "abort") == 0) {
        g_abort_impl = target;
        return (void *)(uintptr_t)bridge_abort;
    }
    if (strcmp(name, "exit") == 0) {
        g_exit_impl = target;
        return (void *)(uintptr_t)bridge_exit;
    }
    if (strcmp(name, "_exit") == 0) {
        g__exit_impl = target;
        return (void *)(uintptr_t)bridge__exit;
    }
    return NULL;
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
    if (strcmp(name, "_initterm") == 0) return (void *)(uintptr_t)builtin__initterm;
    if (strcmp(name, "_initterm_e") == 0) return (void *)(uintptr_t)builtin__initterm_e;
    if (strcmp(name, "__acrt_iob_func") == 0) return (void *)(uintptr_t)builtin___acrt_iob_func;
    if (strcmp(name, "__stdio_common_vfprintf") == 0) return (void *)(uintptr_t)builtin___stdio_common_vfprintf;
    return NULL;
}

static void *resolve_symbol(import_resolver *resolver, const char *dll, const char *name) {
    (void)dll;

    if (resolver->default_winapi_lib) {
        void *fn = dlsym(resolver->default_winapi_lib, name);
        if (fn) {
            void *bridged = bridge_sysv_symbol(name, fn);
            return bridged ? bridged : fn;
        }
    }

    return resolve_builtin_symbol(name);
}

static int report_ordinal_import_unsupported(const char *dll_name, uint64_t ordinal) {
    fprintf(stderr, "unresolved import: %s!#%llu (ordinal imports are not supported)\n",
            dll_name ? dll_name : "<unknown>",
            (unsigned long long)ordinal);
    return -ENOSYS;
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
                    uint64_t ordinal = *lk & 0xFFFFULL;
                    return report_ordinal_import_unsupported(dll_name, ordinal);
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
                    uint32_t ordinal = *lk & 0xFFFFU;
                    return report_ordinal_import_unsupported(dll_name, ordinal);
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

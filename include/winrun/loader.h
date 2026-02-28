#ifndef WINRUN_LOADER_H
#define WINRUN_LOADER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "winrun/pe.h"

typedef struct {
    uint8_t *base;
    uint64_t preferred_base;
    uint64_t actual_base;
    size_t size;
} mapped_image;

typedef struct {
    void *default_winapi_lib;
    bool debug;
} import_resolver;

void *mapped_rva_to_ptr(const mapped_image *mapped, uint32_t rva, size_t size);

int map_pe_image(const pe_image *image, mapped_image *mapped);
int protect_pe_image_sections(const pe_image *image, const mapped_image *mapped);
void unmap_pe_image(mapped_image *mapped);
int apply_relocations(const pe_image *image, mapped_image *mapped);
void set_loader_runtime_args(int argc, char **argv);
int resolve_imports(const pe_image *image, mapped_image *mapped, import_resolver *resolver);
int initialize_tls(const pe_image *image, mapped_image *mapped);
int execute_entry_point(const pe_image *image, mapped_image *mapped, int argc, char **argv);
void install_signal_handlers(bool debug);

#endif

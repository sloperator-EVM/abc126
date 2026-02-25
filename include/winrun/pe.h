#ifndef WINRUN_PE_H
#define WINRUN_PE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PE_DOS_MAGIC 0x5A4D
#define PE_NT_MAGIC 0x00004550
#define PE32_MAGIC 0x10b
#define PE32_PLUS_MAGIC 0x20b

#define IMAGE_DIRECTORY_ENTRY_EXPORT 0
#define IMAGE_DIRECTORY_ENTRY_IMPORT 1
#define IMAGE_DIRECTORY_ENTRY_BASERELOC 5
#define IMAGE_DIRECTORY_ENTRY_TLS 9

#define IMAGE_SCN_CNT_UNINITIALIZED_DATA 0x00000080
#define IMAGE_SCN_MEM_EXECUTE 0x20000000
#define IMAGE_SCN_MEM_READ 0x40000000
#define IMAGE_SCN_MEM_WRITE 0x80000000

typedef struct {
    uint16_t e_magic;
    uint8_t _pad[58];
    uint32_t e_lfanew;
} __attribute__((packed)) pe_dos_header;

typedef struct {
    uint16_t machine;
    uint16_t number_of_sections;
    uint32_t time_date_stamp;
    uint32_t pointer_to_symbol_table;
    uint32_t number_of_symbols;
    uint16_t size_of_optional_header;
    uint16_t characteristics;
} __attribute__((packed)) pe_file_header;

typedef struct {
    uint32_t virtual_address;
    uint32_t size;
} __attribute__((packed)) pe_data_directory;

typedef struct {
    uint16_t magic;
    uint8_t major_linker_version;
    uint8_t minor_linker_version;
    uint32_t size_of_code;
    uint32_t size_of_initialized_data;
    uint32_t size_of_uninitialized_data;
    uint32_t address_of_entry_point;
    uint32_t base_of_code;
    uint32_t base_of_data;
    uint32_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    uint16_t major_os_version;
    uint16_t minor_os_version;
    uint16_t major_image_version;
    uint16_t minor_image_version;
    uint16_t major_subsystem_version;
    uint16_t minor_subsystem_version;
    uint32_t win32_version_value;
    uint32_t size_of_image;
    uint32_t size_of_headers;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dll_characteristics;
    uint32_t size_of_stack_reserve;
    uint32_t size_of_stack_commit;
    uint32_t size_of_heap_reserve;
    uint32_t size_of_heap_commit;
    uint32_t loader_flags;
    uint32_t number_of_rva_and_sizes;
    pe_data_directory data_directories[16];
} __attribute__((packed)) pe_optional_header32;

typedef struct {
    uint16_t magic;
    uint8_t major_linker_version;
    uint8_t minor_linker_version;
    uint32_t size_of_code;
    uint32_t size_of_initialized_data;
    uint32_t size_of_uninitialized_data;
    uint32_t address_of_entry_point;
    uint32_t base_of_code;
    uint64_t image_base;
    uint32_t section_alignment;
    uint32_t file_alignment;
    uint16_t major_os_version;
    uint16_t minor_os_version;
    uint16_t major_image_version;
    uint16_t minor_image_version;
    uint16_t major_subsystem_version;
    uint16_t minor_subsystem_version;
    uint32_t win32_version_value;
    uint32_t size_of_image;
    uint32_t size_of_headers;
    uint32_t checksum;
    uint16_t subsystem;
    uint16_t dll_characteristics;
    uint64_t size_of_stack_reserve;
    uint64_t size_of_stack_commit;
    uint64_t size_of_heap_reserve;
    uint64_t size_of_heap_commit;
    uint32_t loader_flags;
    uint32_t number_of_rva_and_sizes;
    pe_data_directory data_directories[16];
} __attribute__((packed)) pe_optional_header64;

typedef struct {
    uint32_t signature;
    pe_file_header file_header;
} __attribute__((packed)) pe_nt_headers_prefix;

typedef struct {
    char name[8];
    uint32_t virtual_size;
    uint32_t virtual_address;
    uint32_t size_of_raw_data;
    uint32_t pointer_to_raw_data;
    uint32_t pointer_to_relocations;
    uint32_t pointer_to_linenumbers;
    uint16_t number_of_relocations;
    uint16_t number_of_linenumbers;
    uint32_t characteristics;
} __attribute__((packed)) pe_section_header;

typedef struct {
    uint32_t original_first_thunk;
    uint32_t time_date_stamp;
    uint32_t forwarder_chain;
    uint32_t name;
    uint32_t first_thunk;
} __attribute__((packed)) pe_import_descriptor;

typedef struct {
    uint32_t virtual_address;
    uint32_t size_of_block;
} __attribute__((packed)) pe_base_reloc_block;

typedef struct {
    uint32_t start_address_of_raw_data;
    uint32_t end_address_of_raw_data;
    uint32_t address_of_index;
    uint32_t address_of_callbacks;
    uint32_t size_of_zero_fill;
    uint32_t characteristics;
} __attribute__((packed)) pe_tls_directory32;

typedef struct {
    uint64_t start_address_of_raw_data;
    uint64_t end_address_of_raw_data;
    uint64_t address_of_index;
    uint64_t address_of_callbacks;
    uint32_t size_of_zero_fill;
    uint32_t characteristics;
} __attribute__((packed)) pe_tls_directory64;

typedef struct {
    uint8_t *file_data;
    size_t file_size;
    bool is_pe64;
    uint64_t image_base;
    uint32_t size_of_image;
    uint32_t size_of_headers;
    uint32_t address_of_entry_point;
    uint32_t section_alignment;
    uint16_t section_count;
    pe_data_directory data_directories[16];
    pe_section_header *sections;
    uint8_t *nt_headers;
} pe_image;

int pe_parse_image(const char *path, pe_image *out_image);
void pe_destroy_image(pe_image *image);
void *pe_rva_to_ptr(const pe_image *image, uint32_t rva);

#endif

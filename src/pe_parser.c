#include "winrun/pe.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int read_file(const char *path, uint8_t **data, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return -errno;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -EIO;
    }

    long file_len = ftell(f);
    if (file_len < 0) {
        fclose(f);
        return -EIO;
    }
    rewind(f);

    uint8_t *buffer = malloc((size_t)file_len);
    if (!buffer) {
        fclose(f);
        return -ENOMEM;
    }

    if (fread(buffer, 1, (size_t)file_len, f) != (size_t)file_len) {
        free(buffer);
        fclose(f);
        return -EIO;
    }

    fclose(f);
    *data = buffer;
    *size = (size_t)file_len;
    return 0;
}

int pe_parse_image(const char *path, pe_image *out_image) {
    memset(out_image, 0, sizeof(*out_image));

    int rc = read_file(path, &out_image->file_data, &out_image->file_size);
    if (rc != 0) {
        return rc;
    }

    if (out_image->file_size < sizeof(pe_dos_header)) {
        return -EINVAL;
    }

    pe_dos_header *dos = (pe_dos_header *)out_image->file_data;
    if (dos->e_magic != PE_DOS_MAGIC) {
        return -EINVAL;
    }

    if ((size_t)dos->e_lfanew + sizeof(pe_nt_headers_prefix) > out_image->file_size) {
        return -EINVAL;
    }

    pe_nt_headers_prefix *prefix = (pe_nt_headers_prefix *)(out_image->file_data + dos->e_lfanew);
    if (prefix->signature != PE_NT_MAGIC) {
        return -EINVAL;
    }

    out_image->section_count = prefix->file_header.number_of_sections;
    out_image->nt_headers = (uint8_t *)prefix;

    uint8_t *optional_header_ptr = (uint8_t *)(prefix + 1);
    uint16_t magic = *(uint16_t *)optional_header_ptr;
    if (magic == PE32_PLUS_MAGIC) {
        pe_optional_header64 *opt = (pe_optional_header64 *)optional_header_ptr;
        out_image->is_pe64 = true;
        out_image->image_base = opt->image_base;
        out_image->size_of_image = opt->size_of_image;
        out_image->size_of_headers = opt->size_of_headers;
        out_image->address_of_entry_point = opt->address_of_entry_point;
        out_image->section_alignment = opt->section_alignment;
        memcpy(out_image->data_directories, opt->data_directories, sizeof(out_image->data_directories));
    } else if (magic == PE32_MAGIC) {
        pe_optional_header32 *opt = (pe_optional_header32 *)optional_header_ptr;
        out_image->is_pe64 = false;
        out_image->image_base = opt->image_base;
        out_image->size_of_image = opt->size_of_image;
        out_image->size_of_headers = opt->size_of_headers;
        out_image->address_of_entry_point = opt->address_of_entry_point;
        out_image->section_alignment = opt->section_alignment;
        memcpy(out_image->data_directories, opt->data_directories, sizeof(out_image->data_directories));
    } else {
        return -EINVAL;
    }

    uint8_t *section_base = optional_header_ptr + prefix->file_header.size_of_optional_header;
    size_t sections_size = (size_t)out_image->section_count * sizeof(pe_section_header);
    if ((size_t)(section_base - out_image->file_data) + sections_size > out_image->file_size) {
        return -EINVAL;
    }

    out_image->sections = (pe_section_header *)section_base;
    return 0;
}

void pe_destroy_image(pe_image *image) {
    free(image->file_data);
    memset(image, 0, sizeof(*image));
}

void *pe_rva_to_ptr(const pe_image *image, uint32_t rva) {
    if (rva < image->size_of_headers && rva < image->file_size) {
        return image->file_data + rva;
    }

    for (uint16_t i = 0; i < image->section_count; ++i) {
        const pe_section_header *sec = &image->sections[i];
        uint32_t size = sec->virtual_size ? sec->virtual_size : sec->size_of_raw_data;
        if (rva >= sec->virtual_address && rva < sec->virtual_address + size) {
            uint32_t offset = rva - sec->virtual_address;
            if (offset >= sec->size_of_raw_data) {
                return NULL;
            }
            if ((size_t)sec->pointer_to_raw_data + offset >= image->file_size) {
                return NULL;
            }
            return image->file_data + sec->pointer_to_raw_data + offset;
        }
    }

    return NULL;
}

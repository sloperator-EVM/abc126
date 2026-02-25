#include "winrun/loader.h"

#include <stddef.h>
#include <stdint.h>

#define IMAGE_REL_BASED_ABSOLUTE 0
#define IMAGE_REL_BASED_HIGHLOW 3
#define IMAGE_REL_BASED_DIR64 10

int apply_relocations(const pe_image *image, mapped_image *mapped) {
    uint64_t delta = mapped->actual_base - mapped->preferred_base;
    if (delta == 0) {
        return 0;
    }

    pe_data_directory dir = image->data_directories[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (dir.virtual_address == 0 || dir.size == 0) {
        return -1;
    }

    uint8_t *cursor = mapped->base + dir.virtual_address;
    uint8_t *end = cursor + dir.size;

    while (cursor + sizeof(pe_base_reloc_block) <= end) {
        pe_base_reloc_block *block = (pe_base_reloc_block *)cursor;
        if (block->size_of_block < sizeof(pe_base_reloc_block)) {
            break;
        }

        size_t entries = (block->size_of_block - sizeof(pe_base_reloc_block)) / sizeof(uint16_t);
        uint16_t *relocs = (uint16_t *)(cursor + sizeof(pe_base_reloc_block));

        for (size_t i = 0; i < entries; ++i) {
            uint16_t type = relocs[i] >> 12;
            uint16_t offset = relocs[i] & 0x0FFF;
            uint8_t *fixup_addr = mapped->base + block->virtual_address + offset;

            if (type == IMAGE_REL_BASED_DIR64 && image->is_pe64) {
                *(uint64_t *)fixup_addr += delta;
            } else if (type == IMAGE_REL_BASED_HIGHLOW && !image->is_pe64) {
                *(uint32_t *)fixup_addr += (uint32_t)delta;
            } else if (type == IMAGE_REL_BASED_ABSOLUTE) {
                continue;
            }
        }

        cursor += block->size_of_block;
    }

    return 0;
}

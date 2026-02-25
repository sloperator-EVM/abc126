#include "winrun/loader.h"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static int section_to_prot(uint32_t characteristics) {
    int prot = 0;
    if (characteristics & IMAGE_SCN_MEM_READ) {
        prot |= PROT_READ;
    }
    if (characteristics & IMAGE_SCN_MEM_WRITE) {
        prot |= PROT_WRITE;
    }
    if (characteristics & IMAGE_SCN_MEM_EXECUTE) {
        prot |= PROT_EXEC;
    }
    return prot;
}

int map_pe_image(const pe_image *image, mapped_image *mapped) {
    memset(mapped, 0, sizeof(*mapped));

    void *desired = (void *)(uintptr_t)image->image_base;
    void *base = mmap(desired, image->size_of_image, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    if (base == MAP_FAILED) {
        base = mmap(NULL, image->size_of_image, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (base == MAP_FAILED) {
            return -errno;
        }
    }

    mapped->base = base;
    mapped->preferred_base = image->image_base;
    mapped->actual_base = (uint64_t)(uintptr_t)base;
    mapped->size = image->size_of_image;

    memcpy(base, image->file_data, image->size_of_headers);

    for (uint16_t i = 0; i < image->section_count; ++i) {
        const pe_section_header *sec = &image->sections[i];
        uint8_t *dest = mapped->base + sec->virtual_address;

        if (sec->size_of_raw_data > 0 && sec->pointer_to_raw_data < image->file_size) {
            size_t copy_size = sec->size_of_raw_data;
            if ((size_t)sec->pointer_to_raw_data + copy_size > image->file_size) {
                copy_size = image->file_size - sec->pointer_to_raw_data;
            }
            memcpy(dest, image->file_data + sec->pointer_to_raw_data, copy_size);
        }

        if ((sec->characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) && sec->virtual_size > sec->size_of_raw_data) {
            memset(dest + sec->size_of_raw_data, 0, sec->virtual_size - sec->size_of_raw_data);
        }
    }

    long page_size = sysconf(_SC_PAGESIZE);
    for (uint16_t i = 0; i < image->section_count; ++i) {
        const pe_section_header *sec = &image->sections[i];
        uintptr_t start = ((uintptr_t)mapped->base + sec->virtual_address) & ~(uintptr_t)(page_size - 1);
        uintptr_t end = ((uintptr_t)mapped->base + sec->virtual_address + sec->virtual_size + page_size - 1) & ~(uintptr_t)(page_size - 1);
        size_t len = end > start ? end - start : 0;
        if (len == 0) {
            continue;
        }

        int prot = section_to_prot(sec->characteristics);
        if (prot == 0) {
            prot = PROT_READ;
        }

        mprotect((void *)start, len, prot);
    }

    return 0;
}

void unmap_pe_image(mapped_image *mapped) {
    if (mapped->base != NULL && mapped->size > 0) {
        munmap(mapped->base, mapped->size);
    }
    memset(mapped, 0, sizeof(*mapped));
}

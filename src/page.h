#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <string.h>

#define PAGE_SIZE 4096

enum PageType {
    ROOT = 1,
    INTERIOR,
    LEAF,
};

typedef struct PageHeader {
    uint32_t id;
    enum PageType type;   // ROOT, INTERIOR, or LEAF
    uint16_t free_start;  // offset to start of free space
    uint16_t free_end;    // offset to end of free space
    uint16_t total_free;  // free_end - free_start
} PageHeader;

typedef struct PointerList {
    uint16_t* start;
    uint16_t size;
} PointerList;

void* new_page(enum PageType type, uint32_t id);
void save_page(uint32_t fd, void* page);
void* load_page(uint32_t fd, uint32_t page_id);
PageHeader* get_page_header(void* page);
uint16_t add_cell(void* page, void* cell, uint16_t cell_size);
void remove_cell(void* page, uint16_t cell_pointer);
void* get_cell(void* page, uint16_t cell_location);

#endif

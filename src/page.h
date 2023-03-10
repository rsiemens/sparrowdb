#ifndef PAGE_H
#define PAGE_H

#include <stdint.h>
#include <string.h>

#define PAGE_SIZE 4096
#define CAN_COMPACT 0x01

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
    uint8_t flags;
} PageHeader;

typedef struct CellPointer {
    uint16_t cell_location;
    uint16_t cell_size;
} CellPointer;

typedef struct PointerList {
    CellPointer* start;
    uint16_t size;
} PointerList;

#define PAGE_HEADER(page) \
    ((PageHeader*)page)

void* new_page(enum PageType type, uint32_t id);
void save_page(uint32_t fd, void* page);
void* load_page(uint32_t fd, uint32_t page_id);
PointerList pointer_list(void* page);
uint16_t add_cell(void* page, void* cell, uint16_t cell_size);
void remove_cell(void* page, uint16_t idx);
void* get_cell(void* page, uint16_t idx);
void compact(void* page);

#endif

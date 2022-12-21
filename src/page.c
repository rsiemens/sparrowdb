#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "page.h"

#define CAN_COMPACT 0x01

/**
 * A page starts as just a blank slate of congruent bytes of a fixed PAGE_SIZE.
 */
void* alloc_page() {
    return calloc(1, PAGE_SIZE);
}

/**
 * Create a new page and initialize the header.
 */
void* new_page(enum PageType type, uint32_t id) {
    void* page = alloc_page();
    PageHeader* header = PAGE_HEADER(page);
    header->id = id;
    header->type = type;
    header->free_start = sizeof(PageHeader);
    header->free_end = PAGE_SIZE - 1;
    header->total_free = header->free_end - header->free_start;

    return page;
}

/**
 * Seek to the proper page offset and write the page to disk to persist it.
 */
void save_page(uint32_t fd, void* page) {
    PageHeader* header = PAGE_HEADER(page);

    lseek(fd, header->id * PAGE_SIZE, SEEK_SET);
    assert(write(fd, page, PAGE_SIZE) != -1);
    fsync(fd);
}

/**
 * Load a page from disk into memory.
 */
void* load_page(uint32_t fd, uint32_t page_id) {
    void* page = alloc_page();

    lseek(fd, page_id * PAGE_SIZE, SEEK_SET);
    assert(read(fd, page, PAGE_SIZE) != -1);
    return page;
}

PointerList pointer_list(void* page) {
    PageHeader* header = PAGE_HEADER(page);
    PointerList list;
    list.start = page + sizeof(PageHeader);
    list.size = (header->free_start - sizeof(PageHeader)) / sizeof(CellPointer);

    return list;
}

/**
 * All the bytes after the page header is our work space for storing cell pointers and cells.
 * Practically it looks something like this:
 *
 *                                       free_start                       free_end
 *                                           |                                |
 *                                           |--->grows              grows<---|
 *                                           V                                V
 *  +--------+---------------+---------------+--------------------------------+-------+-------+
 *  | header | cell_pointer1 | cell_pointer2 |        unused space...         | cell2 | cell1 |
 *  +--------+---------------+---------------+--------------------------------+-------+-------+
 *
 *  This function simply adds a new cell at free_end and a cell pointer free_start.
 *  free_start and free_end are then updated so the next call knows where to place things
 */
uint16_t add_cell(void* page, void* cell, uint16_t cell_size) {
    CellPointer cell_pointer;
    PageHeader* header = PAGE_HEADER(page);
    // We shouldn't be trying to add a cell to a page that doesn't have room for it!
    // The total space that'll need to be available is the size of the cell + the cell pointer.
    assert(header->total_free >= cell_size + sizeof(CellPointer));

    cell_pointer.cell_location = header->free_end - cell_size;
    cell_pointer.cell_size = cell_size;
    // Add the cell to the page
    memcpy((void*) page + cell_pointer.cell_location, cell, cell_size);

    // Add cell pointer to the page
    uint16_t pointer_offset = header->free_start;
    memcpy((void*) page + pointer_offset, &cell_pointer, sizeof(CellPointer));

    // Update the header with the new page bounds
    header->free_end -= cell_size;
    header->free_start += sizeof(CellPointer);
    header->total_free = header->free_end - header->free_start;

    return pointer_offset;
}

/**
 * Simply unlink the cell and mark as eligible for compaction. Recovery of the space can be deferred
 * till later.
 */
void remove_cell(void* page, uint16_t pointer_offset) {
    PAGE_HEADER(page)->flags |= CAN_COMPACT;
    ((CellPointer*)(page + pointer_offset))->cell_location = 0;
}

/**
 * Returns the pointer to a cell
 */
void* get_cell(void* page, uint16_t pointer_offset) {
    uint16_t cell_location = ((CellPointer*)(page + pointer_offset))->cell_location;

    if (cell_location == 0) {
        return NULL;
    }

    return page + cell_location;
}

/**
 * A simple page compaction routine. It works by allocating a new in memory page and copying all
 * live cell pointers + cells to it. Finally it will copy back into the original page and update
 * headers.
 *
 * If there is nothing to compact (CAN_COMPACT isn't set) then we exit early.
 */
void compact(void* page) {
    PageHeader* header = PAGE_HEADER(page);
    PointerList plist = pointer_list(page);

    if (!(header->flags & CAN_COMPACT))
        return;

    void* copy = new_page(ROOT, 0);  // doesn't matter
    CellPointer* cur_pointer;
    for (int i = 0; i < plist.size; i++) {
        cur_pointer = plist.start + i;

        // Only copy live cell pointers and cells
        if (cur_pointer->cell_location != 0) {
            add_cell(copy, page + cur_pointer->cell_location, cur_pointer->cell_size);
        }
    }

    header->free_start = PAGE_HEADER(copy)->free_start;
    header->free_end = PAGE_HEADER(copy)->free_end;
    header->total_free = header->free_end - header->free_start;
    header->flags &= ~CAN_COMPACT;
    memcpy(page + sizeof(PageHeader), copy + sizeof(PageHeader), PAGE_SIZE - sizeof(PageHeader));

    free(copy);
}

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "page.h"


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
    PageHeader* header = get_page_header(page);
    header->id = id;
    header->type = type;
    header->free_start = sizeof(PageHeader) + 1;
    header->free_end = PAGE_SIZE - 1;
    header->total_free = header->free_end - header->free_start;

    return page;
}

/**
 * Seek to the proper page offset and write the page to disk to persist it.
 */
void save_page(uint32_t fd, void* page) {
    PageHeader* header = get_page_header(page);

    lseek(fd, header->id * PAGE_SIZE, SEEK_SET);
    write(fd, page, PAGE_SIZE);
    fsync(fd);
}

/**
 * Load a page from disk into memory.
 */
void* load_page(uint32_t fd, uint32_t page_id) {
    void* page = alloc_page();

    lseek(fd, page_id * PAGE_SIZE, SEEK_SET);
    read(fd, page, PAGE_SIZE);
    return page;
}

/**
 * Cast the start of the page into our PageHeader struct.
 */
PageHeader* get_page_header(void* page) {
    return (PageHeader*)page;
}

PointerList get_pointer_list(void* page, PageHeader* header) {
    PointerList list;
    list.size = (header->free_start - sizeof(PageHeader)) / sizeof(uint16_t);
    list.start = page + (header->free_start - sizeof(PageHeader));

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
    PageHeader* header = get_page_header(page);
    // We shouldn't be trying to add a cell to a page that doesn't have room for it!
    // The total space that'll need to be available is the size of the cell + the cell pointer.
    assert(header->total_free >= cell_size + sizeof(uint16_t));

    // Add the cell to the page
    uint16_t cell_location = header->free_end - cell_size;
    memcpy((void*) page + cell_location, cell, cell_size);

    // Add cell pointer to the page
    uint16_t cell_pointer = header->free_start;
    *(uint16_t*)(page + cell_pointer) = cell_location;

    // Update the header with the new page bounds
    header->free_end -= cell_size;
    header->free_start += sizeof(uint16_t);
    header->total_free = header->free_end - header->free_start;

    return cell_pointer;
}

/**
 * Simply unlink the cell. Recovery of the space can be deferred till later.
 */
void remove_cell(void* page, uint16_t cell_pointer) {
    *(uint16_t*)(page + cell_pointer) = 0;
}

/**
 * Returns the pointer to a cell
 */
void* get_cell(void* page, uint16_t cell_pointer) {
    uint16_t cell_location = *(uint16_t*)(page + cell_pointer);

    if (cell_location == 0) {
        return NULL;
    }

    return page + cell_location;
}

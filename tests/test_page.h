#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "test.h"
#include "page.h"

typedef struct User {
    uint32_t id;
    char name[100];
    char email[100];
} User;

const User user1 = {.id=1, .name="Bob", .email="bob@hotmail.com"};
const User user2 = {.id=2, .name="Alice", .email="alice@gmail.com"};
const User user3 = {.id=3, .name="Eve", .email="eve@msn.com"};

void test_new_page() {
    TEST_START()
    void* page = new_page(ROOT, 0);
    PageHeader* header = PAGE_HEADER(page);

    assert(header->id == 0);
    assert(header->type == ROOT);
    free(page);

    page = new_page(LEAF, 1);
    header = PAGE_HEADER(page);

    assert(header->id == 1);
    assert(header->type == LEAF);

    free(page);
}

void test_add_and_get_cell() {
    TEST_START()
    void* page = new_page(LEAF, 0);
    
    int cid1 = add_cell(page, (void*)&user1, sizeof(User));
    int cid2 = add_cell(page, (void*)&user2, sizeof(User));
    int cid3 = add_cell(page, (void*)&user3, sizeof(User));

    assert(cid1 == 0);
    assert(cid2 == 1);
    assert(cid3 == 2);

    PointerList plist = pointer_list(page);
    assert(plist.size == 3);

    User* user = (User*)get_cell(page, cid1);
    assert(user->id == 1);
    assert(strcmp(user->name, "Bob") == 0);
    assert(strcmp(user->email, "bob@hotmail.com") == 0);

    user = (User*)get_cell(page, cid2);
    assert(user->id == 2);
    assert(strcmp(user->name, "Alice") == 0);
    assert(strcmp(user->email, "alice@gmail.com") == 0);

    user = (User*)get_cell(page, cid3);
    assert(user->id == 3);
    assert(strcmp(user->name, "Eve") == 0);
    assert(strcmp(user->email, "eve@msn.com") == 0);

    free(page);
}

void test_remove_cell_and_compact() {
    TEST_START()
    void* page = new_page(LEAF, 0);
    
    add_cell(page, (void*)&user1, sizeof(User));
    int cid2 = add_cell(page, (void*)&user2, sizeof(User));
    add_cell(page, (void*)&user3, sizeof(User));

    PointerList plist = pointer_list(page);
    assert(plist.size == 3);

    assert(!(PAGE_HEADER(page)->flags & CAN_COMPACT))
    remove_cell(page, cid2);
    assert((PAGE_HEADER(page)->flags & CAN_COMPACT))
    assert(get_cell(page, cid2) == NULL);

    plist = pointer_list(page);
    assert(plist.size == 3);  // remains until compaction

    uint16_t free_before = PAGE_HEADER(page)->total_free;
    compact(page);
    uint16_t free_after = PAGE_HEADER(page)->total_free;
    assert(free_after > free_before);
    assert(!(PAGE_HEADER(page)->flags & CAN_COMPACT))
    plist = pointer_list(page);
    assert(plist.size == 2);
}

void run_test_page() {
    TEST_GROUP("test_page");
    test_new_page();
    test_add_and_get_cell();
    test_remove_cell_and_compact();
}

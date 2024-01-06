#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "page.h"

/**
 *  Simple hard coded table definition. Equivalent to:
 *
 *  CREATE TABLE movie (
 *      id INT,
 *      title VARCHAR(100),
 *      rating FLOAT,
 *      release INT
 *  );
 */
typedef struct Movie {
    uint32_t id;
    char title[100];
    float rating;
    uint32_t release;
} Movie;


void display_header(void* page) {
    PageHeader* header = PAGE_HEADER(page);
    printf(
        "Page: id=%d, type=%d, free_start=%d, free_end=%d, total_free=%d, flags=%d\n",
        header->id, header->type, header->free_start, header->free_end, header->total_free, header->flags
    );
}
void display_movie(Movie* movie) {
    printf(
        "Movie: id=%d, title=%s, rating=%f, release=%d\n",
        movie->id, movie->title, movie->rating, movie->release
    );
}

uint32_t open_table(char* fpath) {
    return open(fpath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
}

int main() {
    uint32_t table_fd = open_table("movies.db");
    void* page1 = new_page(LEAF, 0);

    display_header(page1);

    Movie movie1 = {.id=1, .title="Toy Story", .rating=0.92, .release=1995};
    uint16_t cell1 = add_cell(page1, (void*)&movie1, sizeof(Movie));

    Movie movie2 = {.id=2, .title="Black Panther", .rating=0.96, .release=2018};
    uint16_t cell2 = add_cell(page1, (void*)&movie2, sizeof(Movie));

    Movie movie3 = {.id=3, .title="Alien", .rating=0.98, .release=1979};
    uint16_t cell3 = add_cell(page1, (void*)&movie3, sizeof(Movie));

    Movie movie4 = {.id=4, .title="Star Wars", .rating=0.96, .release=1977};
    uint16_t cell4 = add_cell(page1, (void*)&movie4, sizeof(Movie));

    Movie movie5 = {.id=5, .title="The Incredibles", .rating=0.75, .release=2004};
    uint16_t cell5 = add_cell(page1, (void*)&movie5, sizeof(Movie));

    display_header(page1);
    save_page(table_fd, page1);

    void* loaded_page = load_page(table_fd, 0);
    display_movie((Movie*)get_cell(loaded_page, cell1));
    display_movie((Movie*)get_cell(loaded_page, cell2));
    display_movie((Movie*)get_cell(loaded_page, cell3));
    display_movie((Movie*)get_cell(loaded_page, cell4));
    display_movie((Movie*)get_cell(loaded_page, cell5));
    printf("\n");

    remove_cell(loaded_page, cell3);
    remove_cell(loaded_page, cell5);
    printf("before compact\n");
    display_header(loaded_page);
    compact(loaded_page);
    printf("after compact\n");
    display_header(loaded_page);
    printf("\n");

    PointerList plist = pointer_list(loaded_page);
    CellPointer* cur;
    printf("plist size=%d\n", plist.size);
    for (int i = 0; i < plist.size; i++) {
        cur = plist.start + i;
        display_movie((Movie*)(loaded_page + cur->cell_location));
    }

    return 0;
}

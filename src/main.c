#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
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

    Movie movie1 = {.id=1, .title="Toy Story", .rating=0.92, .release=1995};
    uint16_t cell1 = add_cell(page1, (void*)&movie1, sizeof(Movie));

    Movie movie2 = {.id=2, .title="Black Panther", .rating=0.96, .release=2018};
    uint16_t cell2 = add_cell(page1, (void*)&movie2, sizeof(Movie));

    save_page(table_fd, page1);

    void* page2 = new_page(LEAF, 1);
    Movie movie3 = {.id=3, .title="Alien", .rating=0.98, .release=1979};
    uint16_t cell3 = add_cell(page2, (void*)&movie3, sizeof(Movie));

    save_page(table_fd, page2);

    void* loaded_page = load_page(table_fd, 0);
    display_movie((Movie*)get_cell(loaded_page, cell1));
    display_movie((Movie*)get_cell(loaded_page, cell2));

    loaded_page = load_page(table_fd, 1);
    display_movie((Movie*)get_cell(loaded_page, cell3));
    return 0;
}

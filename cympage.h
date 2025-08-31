#ifndef CYMPAGE_HEADER
#define CYMPAGE_HEADER

#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define CYM_DEFAULT_PAGE_CAPACITY (1000*1000)

// page allocation linked list element
typedef struct CymPage CymPage;

typedef struct CymPage
{
    uint64_t capacity;
    uint64_t size;
    CymPage* last;
    CymPage* next;
} CymPage;


#ifdef __cplusplus
extern "C" {
#endif

// allocates a new page with a given capacity and preallocated page_data of size bytes
// and puts it between pages last and next
// \returns a pointer to the allocated page on success or NULL on failure
CymPage* cym_alloc_page(uint64_t capacity, uint64_t size, const void* page_data, CymPage* last, CymPage* next);

// frees a page properly connecting its last and next pages so as to not disorganize anything
void cym_free_page(CymPage* page);

// reads size bytes starting at position pos (in bytes) from page to dest
// \returns the dest pointer on success or NULL on failure
void* cym_page_read(const CymPage* page, void* dest, size_t pos, size_t size);

// allocates data of size bytes in page, if page can't fit the whole data in it it will create a new page to fit the rest
// use cym_page_read to read the data properly
// \returns a pointer to the first part of the allocated data (in the first page) on success or NULL on failure
void* cym_page_alloc(CymPage* page, const void* data, size_t size);

// deallocates size bytes from the end to start of the linked list of pages
// \param free indicates if a page's memory should be freed once the page is empty (0 to not free otherwise free)
// \returns the last page in the linked list on success or NULL on failure
CymPage* cym_page_dealloc(CymPage* page, size_t size, int free);

// allocates new pages with at least min_page_capacity and at most max_page_capacity untill reserve_size can be allocated by cym_page_alloc
// \returns the reserved size (0 on failure)
size_t cym_page_reserve(CymPage* page, size_t reserve_size, size_t min_page_capacity, size_t max_page_capacity);



#ifdef CYMPAGE_IMPLEMENTATION


CymPage* cym_alloc_page(uint64_t capacity, uint64_t size, const void* page_data, CymPage* last, CymPage* next){

    if(size > capacity){
        return NULL;
    }

    CymPage new_page;
    new_page.capacity = capacity;
    new_page.size     = size;
    new_page.next     = next;
    new_page.last     = last;


    // allocate enough memory to fit the requested capacity and the page's data
    void* data = malloc(capacity + sizeof(new_page));

    if(!memcpy(data, &new_page, sizeof(new_page))){
        free(data);
        return NULL;
    }

    if(!memcpy((uint8_t*)(data) + sizeof(new_page), page_data, size) && size){
        free(data);
        return NULL;
    }

    if(last){
        last->next = (CymPage*) data;
    }
    if(next){
        next->last = (CymPage*) data;
    }

    return (CymPage*) data;
}

void cym_free_page(CymPage* page){
    if(page->last){
        page->last->next = page->next;
    }
    if(page->next){
        page->next->last = page->last;
    }
    free(page);
}


void* cym_page_read(const CymPage* page, void* dest, size_t pos, size_t size){

    while (page && pos >= page->capacity)
    {
        pos -= page->capacity;
        page = page->next;
    }

    // dummy pointer used to later test if all memcpy succeded
    const void* test = page;

    while(page && size && test){
        size_t read = (pos + size < page->capacity)? size : page->capacity - pos;
        test = memcpy(dest, (uint8_t*)(page + 1) + pos, read);
        dest = (uint8_t*)(dest) + read;
        size -= read;
        page = page->next;
    }

    return (test && (page || !size))? dest : NULL;
}

void* cym_page_alloc(CymPage* page, const void* data, size_t size){

    if(!page) return NULL;

    while (page && page->size >= page->capacity && page->next)
    {
        page = page->next;
    }
    
    const void* const memblock_start = (void*) ((uint8_t*)(page + 1) + page->size);

    // dummy pointer used to later test if all memcpy succeded
    // by first setting it to page it also tests whether page == NULL
    const void* test = page;

    CymPage* last_page = page;

    while(page && size && test){
        const size_t written = (page->capacity - page->size < size)? page->capacity - page->size : size;
        test = memcpy((uint8_t*)(page + 1) + page->size, data, written);
        data = (uint8_t*)(data) + written;
        page->size += written;
        size -= written;
        last_page = page;
        page = page->next;
    }

    if(test && (page || !size)){ // success
        return memblock_start;
    }
    // failure
    while (!((uint8_t*)(last_page + 1) + last_page->capacity > memblock_start && last_page < memblock_start))
    {
        last_page->size = 0;
        last_page = last_page->last;
    }

    last_page->size = (uint64_t) ((uint8_t*)(memblock_start) - (uint8_t*)(last_page + 1));
    
    return NULL;
}

size_t cym_page_reserve(CymPage* page, size_t reserve_size, size_t min_page_capacity, size_t max_page_capacity){

    

}


void* cym_alloc_cymbol(CymPage* page, int cymbol_id, const void* data, size_t size){

    Cymbol cymbol = (Cymbol){.cymbol_id = cymbol_id, .size = size};

    const size_t reserve_size = sizeof(cymbol.cymbol_id) + sizeof(cymbol.size) + size;

    if(cym_page_reserve(page, reserve_size, 1000, 1000*1000) != reserve_size){
        return NULL;
    }

    const void* const output = cym_page_alloc(page, &cymbol.cymbol_id, sizeof(cymbol.cymbol_id));

    if(!output) return NULL;

    if(!cym_page_alloc(page, &cymbol.size, sizeof(cymbol.size))){
        cym_page_dealloc(page, sizeof(cymbol.cymbol_id), 1);
        return NULL;
    }

    if(!cym_page_alloc(page, data, size)){
        cym_page_dealloc(page, sizeof(cymbol.size) + sizeof(cymbol.size), 1);
        return NULL;
    }

    return output;

}


#endif // ======================== END OF FUNCTION IMPLEMENTATIONS ==================================================


#ifdef __cplusplus
}
#endif

#endif // =====================  END OF FILE CYMPAGE_HEADER ===========================
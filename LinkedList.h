#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct LList_item {
	struct LList_item* prev;
	struct LList_item* next;
	void* item;
} LList_item_t;

typedef struct LList {
	LList_item_t* head;
	size_t length;
} LList_t;

LList_t* LList_create_list();
void LList_delete_list(LList_t* list);
void LList_add_item_to_head(LList_t* list, void* i);
void LList_add_item_to_end(LList_t* list, void* i);
void LList_add_item(LList_t* list, void* i);
void LList_remove_item(LList_t* list, size_t index);
void* LList_get_item(LList_t* list, size_t index);
size_t LList_get_length(LList_t* list);
int LList_search_list(LList_t* list, int(*f)(void*, void*), void* data);


#endif

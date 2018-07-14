#include <gcc-lib-funcs/gcc-lib-funcs.h>
#include <Memory/kmalloc.h>
#include <Utility/LinkedList.h>

static LList_item_t* LList_create_item(void* item) {
	LList_item_t* item_container = (LList_item_t*)memset(kmalloc(sizeof(LList_item_t)), (int)NULL, sizeof(LList_item_t));
	item_container->item = item;
	return item_container;
};

static LList_item_t* LList_get_container(LList_t* list, size_t index) {
	LList_item_t* ditem = list->head;
	for (size_t i = 0; i < index; i++, ditem = ditem->next);
	return ditem;
};

static LList_item_t* LList_get_container_end(LList_t* list) {
	size_t length = list->length;
	if (length == 0) {
		return NULL;
	}
	else {
		return LList_get_container(list, length - 1);
	}
}

LList_t* LList_create_list() {
	return (LList_t*)memset(kmalloc(sizeof(LList_t)), (int)NULL, sizeof(LList_t));
}

void LList_delete_list(LList_t* list) {
	for (size_t i = 0; i < LList_get_length(list); i++) {
		LList_remove_item(list, 0);
	}
	kfree(list);
}

void LList_add_item_to_head(LList_t* list, void* i) {
	LList_item_t* nitem = LList_create_item(i);
	nitem->next = list->head;
	list->head = nitem;
	if (nitem->next != NULL) {
		nitem->next->prev = nitem;
	}
	list->length = list->length + 1;
}

void LList_add_item_to_end(LList_t* list, void* i) {
	LList_item_t* nitem = LList_create_item(i);
	nitem->prev = LList_get_container_end(list);
	if (nitem->prev != NULL) {
		nitem->prev->next = nitem;
	}
	else {
		list->head = nitem;
	}
	list->length = list->length + 1;
}

void LList_add_item(LList_t* list, void* i) {
	LList_add_item_to_head(list, i);
}

void LList_remove_item(LList_t* list, size_t index) {
	LList_item_t* ditem = LList_get_container(list, index);
	if (ditem != NULL) {
		if (ditem == list->head) {
			list->head = ditem->next;
		}
		if (ditem->next != NULL) {
			ditem->next->prev = ditem->prev;
		};
		if (ditem->prev != NULL) {
			ditem->prev->next = ditem->next;
		};
		kfree(ditem);
		list->length = list->length - 1;
	};
};

void* LList_get_item(LList_t* list, size_t index) {
	LList_item_t* item_container = LList_get_container(list, index);
	if (item_container != NULL) {
		return item_container->item;
	};
	return NULL;
};

size_t LList_get_length(LList_t* list) {
	return list->length;
};

int LList_search_list(LList_t* list, int(*f)(void*, void*), void* data) {
	LList_item_t* sitem = list->head;
	for (size_t i = 0; i < LList_get_length(list); i++, sitem = sitem->next) {
		if (f(sitem->item, data)) {
			return i;
		};
	};
	return -1;
};


#ifndef USTAR_H
#define USTAR_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "LinkedList.h"


typedef void* (*ustar_alloc_func)(size_t size);
typedef void  (*ustar_free_func)(void* ptr);

typedef struct { 
	void* ptr;
	uint64_t size;
} ustar_data_t;

typedef enum { 
	ustar_type_dir,
	ustar_type_file
} ustar_type_t;


typedef struct { 
	ustar_data_t* name;
	uint64_t mode;
	uint64_t uid;
	uint64_t gid;
	uint64_t size;
	uint64_t mtime;
	uint64_t cksum;
	ustar_type_t type;
	ustar_data_t* linkname;
	ustar_data_t* uname;
	ustar_data_t* gname;
	uint64_t devmajor;
	uint64_t devminor;
	ustar_data_t* prefix;
} ustar_mdata_t;


typedef struct { 
	ustar_mdata_t* mdata;
	ustar_data_t* content;
} ustar_entry_t;

typedef struct { 
	LList_t* entries;
	size_t index;
} ustar_entries_t;

typedef struct { 
	ustar_entries_t* entries;
	const char* error_str;
	int status;
} ustar_t;

/* Init and memory management functions */
void ustar_init(ustar_alloc_func, ustar_free_func);
void* ustar_alloc(size_t);
void ustar_free(void*); 

/* Create */
ustar_t* ustar_new_archive(void); 
ustar_entries_t* ustar_new_entries(void);
ustar_entry_t* ustar_new_entry(void); 
ustar_data_t* ustar_new_data(void);
ustar_mdata_t* ustar_new_mdata(void);
void ustar_add_entry(ustar_t*, ustar_entry_t*); 

/* Read */
ustar_t* ustar_parse(ustar_data_t*);
ustar_entry_t* ustar_iterate(ustar_t*);

/* Update */
ustar_data_t* ustar_serialize(ustar_t*);

/* Destroy */ 
void ustar_remove(ustar_t*, ustar_entry_t*);
void ustar_free_archive(ustar_t*);
void ustar_free_entries(ustar_entries_t*);
void ustar_free_entry(ustar_entry_t*);
void ustar_free_data(ustar_data_t*);
void ustar_free_mdata(ustar_mdata_t*);

#endif

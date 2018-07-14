#ifndef USTAR_H
#define USTAR_H

#include <stdint.h>
#include <stddef.h>
#include "LList.h"

typedef struct { 
	LList_t entries;
	size_t index;
} ustar_entries_t;

typedef void* (*ustar_alloc_func)(size_t size);
typedef void  (*ustar_free_func)(void* ptr);

typedef struct { 
	void* ptr;
	uint64_t size;
} ustar_data_t;

typedef enum { 
	DIR,
	FILE
} ustar_type_t;


typedef struct { 
	ustar_data_t* name;
	uint64_t mode;
	uint64_t uid;
	uint64_t gid;
	uint64_t size;
	uint64_t mtime;
	uint64_t cksum;
	uint8_t type;
	ustar_data_t* linkname;
	ustar_data_t* uname;
	ustar_data_t* gname;
	uint64_t devmajor;
	uint64_t devminor;
	ustar_data_t* prefix;
} ustar_mdata_t;



typedef struct {
	ustar_entries_t* entries;
	ustar_mdata_t* m;
} ustar_dir_t;

typedef struct { 
	ustar_data_t* d;
	ustar_mdata_t* m;
} ustar_file_t;

typedef struct { 
	ustar_type_t type;
	void* ptr;
} ustar_entry_t;



void ustar_init(ustar_alloc_func afunc, ustar_free_func ffunc);
ustar_dir_t* ustar_parse(ustar_data_t* data, size_t size);
ustar_data_t* ustar_serialize(ustar_dir_t* dir);
ustar_entry_t* ustar_iterate(ustar_dir_t* dir);
ustar_type_t ustar_get_type(ustar_entry_t* entry);
ustar_dir_t* ustar_get_dir(ustar_entry_t* entry);
ustar_file_t* ustar_get_file(ustar_entry_t* entry);
void* ustar_alloc(size_t size);
ustar_entry_t* ustar_alloc_entry();
ustar_entries_t* ustar_alloc_entries();
ustar_data_t* ustar_alloc_data();
ustar_mdata_t* ustar_alloc_mdata();
ustar_file_t* ustar_alloc_file();
ustar_dir_t* ustar_alloc_dir();
void ustar_free(void* ptr);
void ustar_free_entry(ustar_entry_t* e);
void ustar_free_entries(ustar_entries_t* e);
void ustar_free_data(ustar_data_t* data);
void ustar_free_mdata(ustar_mdata_t* mdata);
void ustar_free_file(ustar_file_t* file);
void ustar_free_dir(ustar_dir_t* dir);


#endif

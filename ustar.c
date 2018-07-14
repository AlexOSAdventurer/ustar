#include "ustar.h"

#define USTAR_BLOCK_SIZE 512
#define USTAR_HEADER_SIZE USTAR_BLOCK_SIZE

static ustar_alloc_func ustar_afunc;
static ustar_free_func ustar_ffunc;

void ustar_init(ustar_alloc_func afunc, ustar_free_func ffunc) { 
	ustar_afunc = afunc;
	ustar_ffunc = ffunc;
}

ustar_dir_t* ustar_parse(ustar_data_t* data, size_t size) { 
	const char* ptr = (const char*)data->ptr;
	const char* des = ptr + size;
	ustar_dir_t* res = ustar_alloc_dir();
	while (ptr < des) { 
		_ustar_check(ptr);
		

		ptr += (fa_size + USTAR_HEADER_SIZE);
	}
	return res;
}

ustar_data_t* ustar_serialize(ustar_dir_t* dir) { 


}

ustar_entry_t* ustar_iterate(ustar_dir_t* dir) { 
	ustar_entries_t* entries = dir->entries;
	ustar_entry_t* centry = (ustar_entry_t*)LList_get_item(entries->entries, entries->index);
	if (centry == NULL) { 
		entries->index = 0;
	}
	return centry;
};

ustar_type_t ustar_get_type(ustar_entry_t* entry) { 
	return entry->type;
};

ustar_dir_t* ustar_get_dir(ustar_entry_t* entry) { 
	return (ustar_dir_t*)(entry->ptr);
};

ustar_file_t* ustar_get_file(ustar_entry_t* entry) { 
	return (ustar_file_t*)(entry->ptr);
}

void* ustar_alloc(size_t size) { 
	return ustar_afunc(size);
};

ustar_entry_t* ustar_alloc_entry() { 
	ustar_entry_t* res = ustar_alloc(sizeof(ustar_entry_t));
	return res;
};

ustar_entries_t* ustar_alloc_entries() { 
	ustar_entries_t* res = ustar_alloc(sizeof(ustar_entries_t));
	res->entries = LList_create_list();
	res->index = 0;
	return res;
}


ustar_data_t* ustar_alloc_data() { 
	ustar_data_t* res = ustar_alloc(sizeof(ustar_data_t));
	return res;
} 

ustar_mdata_t* ustar_alloc_mdata() { 
	ustar_mdata_t* res = ustar_alloc(sizeof(ustar_mdata_t));
	res->name = ustar_alloc_data();
	res->linkname = ustar_alloc_data();
	res->uname = ustar_alloc_data();
	res->gname = ustar_alloc_data();
	res->prefix = ustar_alloc_data();
	return res;
}

ustar_file_t* ustar_alloc_file() { 
	ustar_file_t* res = ustar_alloc(sizeof(ustar_file_t));
	res->d = ustar_alloc_data();
	res->m = ustar_alloc_mdata();
	return res;
}

ustar_dir_t* ustar_alloc_dir() { 
	ustar_dir_t* res = ustar_alloc(sizeof(ustar_dir_t));
	res->entries = ustar_alloc_entries();
	res->m = ustar_alloc_mdata();
	return res;
}

void ustar_free(void* ptr) { 
	ustar_ffunc(ptr);
}

void ustar_free_entry(ustar_entry_t* e) {
	ustar_free(e);
};

void ustar_free_entries(ustar_entries_t* e) { 
	LList_delete_list(e->entries);
	ustar_free(e);
}

void ustar_free_data(ustar_data_t* data) { 
	ustar_free(data->ptr);
	ustar_free(data);
} 

void ustar_free_mdata(ustar_mdata_t* mdata) { 
	ustar_free_data(mdata->linkname);
	ustar_free_data(mdata->uname);
	ustar_free_data(mdata->gname);
	ustar_free_data(mdata->prefix);
	ustar_free(mdata);
} 

void ustar_free_file(ustar_file_t* file) { 
	ustar_free_data(file->d);
	ustar_free_mdata(file->m);
	ustar_free(file);
}


void ustar_free_dir(ustar_dir_t* dir) { 
	for (ustar_entry_t* e = ustar_iterate(dir); e != NULL; e = ustar_iterate(dir)) { 
		if (ustar_get_type(e) == DIR) { 
			ustar_free_dir(ustar_get_dir(e));
		}
		else { 
			ustar_free_file(ustar_get_file(e));
		}
		ustar_free_entry(e);
	};
	ustar_free_mdata(dir->m);
	ustar_free_entries(dir->entries);
	ustar_free(dir);
}

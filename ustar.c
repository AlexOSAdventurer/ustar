#include "ustar.h"

/* Sets the default block size, which is 512 according to the standard */
#define USTAR_BLOCK_SIZE 512
#define USTAR_HEADER_SIZE USTAR_BLOCK_SIZE

static ustar_alloc_func ustar_afunc;
static ustar_free_func ustar_ffunc;

/* The USTAR magic string - used in checks and upon serialization */
static const char ustar_str[] = "ustar\00000";

/* Various error strings used internally */
static const char checksum_ck_error[] = "Checksum check failed!";
static const char ustar_str_error[] = "Ustar entry does not appear to be ustar!";
static const char etc_error[] = "Some error.... happened?";

/* Various utility functions */
static uint64_t _round_up_to_512(uint64_t num) { 
	uint64_t res = num & 0xFFFFFFFFFFFFFE00;
	if (res < num) { 
		res += 512;
	};
	return res;
};

static uint64_t _parse_octal(const uint8_t* data, size_t size) { 
	uint64_t n = 0;
	const unsigned char* c = data;
	while (size > 0) {
		/* Some USTAR octals are space terminated. Weird. */ 
		if ((*c) == ' ') { 
			break;
		}; 
		n *= 8;
		n += *c - '0';
		c++;
		size--;
	};
	return n;
};

static ustar_data_t* _generate_octal_str(uint64_t n, size_t l) { 
	ustar_data_t* res = ustar_new_data();
	res->size = l;
	res->ptr = ustar_alloc(l);
	char* ptr = res->ptr + res->size - 1;
	while (n > 0) { 
		size_t c_n = n % 8;
		n = (n - c_n) / 8;
		char ch_n = '0' + c_n;
		*ptr = ch_n;
		ptr--;
	};
	while ((void*)ptr >= res->ptr) { 
		*ptr = '0';
		ptr--;
	};
	return res;
};

static ustar_type_t _parse_type(uint8_t* data) { 
	unsigned char c = (unsigned char)(*data);
	if (c == '0') { 
		return ustar_type_file;
	};
	return ustar_type_dir;
}

static char _serialize_type(ustar_type_t type) { 
	if (type == ustar_type_file) { 
		return '0';
	}
	return '5';
}

/* Sets the memory management functions */
void ustar_init(ustar_alloc_func afunc, ustar_free_func ffunc) { 
	ustar_afunc = afunc;
	ustar_ffunc = ffunc;
};

void* ustar_alloc(size_t size) { 
	return ustar_afunc(size);
};


void ustar_free(void* ptr) {
	ustar_ffunc(ptr);
};


/* Utility functions for creating new data structures */

ustar_t* ustar_new_archive(void) { 
	ustar_t* res = ustar_alloc(sizeof(ustar_t));
	res->entries = ustar_new_entries();
	res->error_str = NULL;
	res->status = 0;
	return res;
}

ustar_entries_t* ustar_new_entries(void) { 
	ustar_entries_t* res = ustar_alloc(sizeof(ustar_entries_t));
	res->entries = LList_create_list();
	res->index = 0;

	return res;
}

ustar_entry_t* ustar_new_entry(void) { 
	ustar_entry_t* res = ustar_alloc(sizeof(ustar_entry_t));
	res->mdata = ustar_new_mdata();
	res->content = ustar_new_data();

	return res;
} 

ustar_data_t* ustar_new_data(void) { 
	ustar_data_t* res = ustar_alloc(sizeof(ustar_data_t));
	memset(res, 0, sizeof(ustar_data_t));

	return res;
}

ustar_mdata_t* ustar_new_mdata(void) { 
	ustar_mdata_t* res = ustar_alloc(sizeof(ustar_mdata_t));
	memset(res, 0, sizeof(ustar_mdata_t));

	return res;
}

/* Lets you add a new entry to an archive */
void ustar_add_entry(ustar_t* archive, ustar_entry_t* entry) { 
	ustar_entries_t* entries = archive->entries;
	LList_add_item_to_end(entries->entries, entry);
}

/* Read */

/* More utility functions used internally */
static uint64_t _generate_checksum(ustar_t* archive, const uint8_t* ptr) {  
	uint64_t res = 0;
	uint64_t orig_sum;
	for (size_t i = 0; i < 148; i++) { 
		res += ptr[i];
	}
	orig_sum = _parse_octal(&(ptr[148]), 6);
	res += (8 * (uint64_t)(' ')); // Checksum field is to be treated as an ALL space. 
	for (size_t i = 156; i < 512; i++) { 
		res += ptr[i];
	}
	if (orig_sum != res) { 
		archive->status = 1;
		archive->error_str = checksum_ck_error;
	}
	return res;
};

static int _ustar_check_ustar_value(uint8_t* data) { 
	return memcmp(data, ustar_str, 8) != 0;
}

static void _ustar_parse_mdata(ustar_t* archive, uint8_t* data, ustar_entry_t* entry) { 
	const uint8_t* orig = data;
	ustar_mdata_t* mdata = entry->mdata;
	ustar_data_t* name = ustar_new_data();
	name->ptr = ustar_alloc(100);
	name->size = 100;
	memcpy(name->ptr, data, 100);
	data += 100;
	
	memcpy(&(mdata->mode), data, 8);
	data += 8;

	memcpy(&(mdata->uid), data, 8);
	data += 8;

	memcpy(&(mdata->gid), data, 8);
	data += 8;
	
	mdata->size = _parse_octal(data, 12);
	data += 12;

	mdata->mtime = _parse_octal(data, 12);
	data += 12;

	mdata->cksum = _generate_checksum(archive, (const uint8_t*)orig);
	data += 8;

	mdata->type = _parse_type(data);
	data += 1;

	ustar_data_t* linkname = ustar_new_data();
	linkname->ptr = ustar_alloc(100);
	linkname->size = 100;
	memcpy(linkname->ptr, data, 100);
	data += 100;

	if (_ustar_check_ustar_value(data)) { 
		archive->status = 1;
		archive->error_str = ustar_str_error;
	}
	data += 8;
	
	ustar_data_t* uname = ustar_new_data();
	uname->ptr = ustar_alloc(32);
	uname->size = 32;
	memcpy(uname->ptr, data, 32);
	data += 32;

	ustar_data_t* gname = ustar_new_data();
	gname->ptr = ustar_alloc(32);
	gname->size = 32;
	memcpy(gname->ptr, data, 32);
	data += 32;

	memcpy(&(mdata->devmajor), data, 8);
	data += 8;

	memcpy(&(mdata->devminor), data, 8);
	data += 8;

	ustar_data_t* prefix = ustar_new_data();
	prefix->ptr = ustar_alloc(155);
	prefix->size = 155;
	memcpy(prefix->ptr, data, 155);
	data += 155;

	mdata->name = name;
	mdata->linkname = linkname;
	mdata->uname = uname;
	mdata->gname = gname;
	mdata->prefix = prefix;
};

static size_t _ustar_parse_content(uint8_t* data, ustar_entry_t* entry) { 
	uint64_t size = entry->mdata->size;
	size = _round_up_to_512(size);
	ustar_data_t* ndata = entry->content;
	ndata->ptr = ustar_alloc(size);
	ndata->size = size;
	memcpy(ndata->ptr, data, size);

	return size;
};

static void _ustar_check_null_end_blocks(ustar_t* archive, uint8_t* ptr) { 
	for (size_t i = 0; i < (2 * USTAR_BLOCK_SIZE); i++) { 
		if (ptr[i] != 0) { 
			archive->status = -1;
			archive->error_str = etc_error;
			break;
		}
	}
};

/* Takes in a ustar_data_t block and parses it into an archive if possible. 
   On success, the status field of the archive will be 0. Otherwise, it will 
   be 1, and the error_str field will be set to a message - it's NULL otherwise. */
ustar_t* ustar_parse(ustar_data_t* data) { 
	ustar_t* res = ustar_new_archive();
	uint8_t* data_start = data->ptr;
	uint8_t* data_cur = data_start;
	uint8_t* data_end = data_start + data->size - (2 * USTAR_BLOCK_SIZE);
	_ustar_check_null_end_blocks(res, data_end);
	if (res->status != 0) { 
		return res;
	}
	data_end -= 1024;
	while (data_cur < data_end) { 
		ustar_entry_t* nentry = ustar_new_entry();
		_ustar_parse_mdata(res, data_cur, nentry);
		if (res->status != 0) { 
			break;
		}
		data_cur += USTAR_HEADER_SIZE;
		size_t size = _ustar_parse_content(data_cur, nentry);
		ustar_add_entry(res, nentry);
 		data_cur += size;
	}
	return res;
}

/* Call this to yield an entry in the archive - each new call will yield the next entry. 
   Returns NULL when there are no more entries. You can then iterate over the archive 
   again if you want. */
ustar_entry_t* ustar_iterate(ustar_t* archive) { 
	ustar_entries_t* entries = archive->entries;
	ustar_entry_t* entry = LList_get_item(entries->entries, entries->index);

	entries->index++;
	if (entry == NULL) {
		entries->index = 0;
	}

	return entry;
}

/* Update */
/* The below four static functions are internal utilities. Don't worry about them */

static uint64_t _compute_archive_size(ustar_t* archive) { 
	uint64_t res = 0;
	ustar_entry_t* entry = ustar_iterate(archive);
	while (entry != NULL) {
		ustar_mdata_t* mdata = entry->mdata;
		res += USTAR_HEADER_SIZE;
		res += _round_up_to_512(mdata->size);
		entry = ustar_iterate(archive);
	}
	res += (USTAR_HEADER_SIZE * 2);
	return res;
};

static uint8_t* _serialize_mdata(uint8_t* ptr, ustar_mdata_t* mdata) { 
	memcpy(ptr, mdata->name->ptr, 100);
	ptr += 100;

	memcpy(ptr, &(mdata->mode), 8);
	ptr += 8;

	memcpy(ptr, &(mdata->uid), 8);
	ptr += 8;
	
	memcpy(ptr, &(mdata->gid), 8);
	ptr += 8;
	
	ustar_data_t* fsize_str = _generate_octal_str(mdata->size, 11);
	memcpy(ptr, fsize_str->ptr, 11);
	ustar_free(fsize_str);
	ptr += 11;
	*ptr = ' ';
	ptr++;
	
	ustar_data_t* mtime_str = _generate_octal_str(mdata->mtime, 11);
	memcpy(ptr, mtime_str->ptr, 11);
	ustar_free(mtime_str);
	ptr += 11;
	*ptr = ' ';
	ptr++;
	
	ustar_data_t* cksum = _generate_octal_str(mdata->cksum, 6);
	memcpy(ptr, cksum->ptr, 6);
	ptr += 6;
	*ptr = '\0';
	ptr++;
	*ptr = ' ';
	ptr++;
	
	*ptr = _serialize_type(mdata->type);
	ptr++;
	
	memcpy(ptr, mdata->linkname->ptr, 100);
	ptr += 100;
	
	memcpy(ptr, ustar_str, 8);
	ptr += 8;
	
	memcpy(ptr, mdata->uname->ptr, 32);
	ptr += 32;
	
	memcpy(ptr, mdata->gname->ptr, 32);
	ptr += 32;
	
	memcpy(ptr, &(mdata->devmajor), 8);
	ptr += 8;
	
	memcpy(ptr, &(mdata->devminor), 8);
	ptr += 8;
	
	memcpy(ptr, mdata->prefix->ptr, 155);	
	ptr += 155;
	
	memset(ptr, 0, 12);
	ptr += 12;
	return ptr;
}

static uint8_t* _serialize_content(uint8_t* ptr, ustar_data_t* data) { 
	int diff = _round_up_to_512(data->size) - data->size;
	memcpy(ptr, data->ptr, data->size);
	ptr += data->size;
	memset(ptr, 0, diff);
	ptr += diff;
	return ptr;
}

static void _write_end_null_blocks(uint8_t* ptr) { 
	memset(ptr, 0, (USTAR_HEADER_SIZE * 2));
}

/*Takes an archive and serializes it into a ustar_data_t type, which you can use to write to disk or whatever */
ustar_data_t* ustar_serialize(ustar_t* archive) { 
	ustar_data_t* res = ustar_new_data();
	res->size = _compute_archive_size(archive);
	res->ptr = ustar_alloc(res->size);
	ustar_entry_t* entry = ustar_iterate(archive);
	uint8_t* cptr = res->ptr;
	while (entry != NULL) { 
		ustar_mdata_t* mdata = entry->mdata;
		ustar_data_t* content = entry->content;
		cptr = _serialize_mdata(cptr, mdata);
		cptr = _serialize_content(cptr, content);
		entry = ustar_iterate(archive);
	}
	_write_end_null_blocks(cptr); // USTAR requires that two 512 0 blocks are at the end of every archive
	return res;
}

/* Destroy */ 
/* This stuff is just a remove entry function and various utility functions for freeing data */
static int _ustar_remove_search(void* item, void* data) { 
	return item == data;
};

void ustar_remove(ustar_t* archive, ustar_entry_t* entry) { 
	ustar_entries_t* entries = archive->entries;
	int index = LList_search_list(entries->entries, _ustar_remove_search, entry);
	if (index >= 0) { 
		LList_remove_item(entries->entries, index);
	};
}

void ustar_free_archive(ustar_t* archive) { 
	ustar_free_entries(archive->entries);
	ustar_free(archive);
}

void ustar_free_entries(ustar_entries_t* entries) { 
	LList_delete_list(entries->entries);
	ustar_free(entries);
}

void ustar_free_entry(ustar_entry_t* entry) { 
	ustar_free_mdata(entry->mdata);
	ustar_free_data(entry->content);
	ustar_free(entry);
}

void ustar_free_data(ustar_data_t* data) { 
	ustar_free(data->ptr);
	ustar_free(data);
}

void ustar_free_mdata(ustar_mdata_t* mdata) { 
	ustar_free_data(mdata->name);
	ustar_free_data(mdata->linkname);
	ustar_free_data(mdata->uname);
	ustar_free_data(mdata->gname);
	ustar_free_data(mdata->prefix);
	ustar_free(mdata);
}

#include <stdio.h>
#include <stdlib.h>
#include "ustar.h"

ustar_data_t* file_load(char* fname) { 
	ustar_data_t* data = ustar_new_data();
	FILE* fp = fopen(fname, "r");
	if (fp) {
		fseek(fp, 0, SEEK_END);
		uint64_t size = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		void* fcontent = ustar_alloc(size);
		fread(fcontent, 1, size, fp);
		data->size = size;
		data->ptr = fcontent;
	};
	return data;
};

int main(int argc, char* argv[]) { 
	char* fname = argv[1];
	printf("%s\n", fname);
	ustar_init(malloc, free);
	ustar_data_t* data = file_load(fname);	
	ustar_t* archive = ustar_parse(data);
	ustar_entry_t* entry = ustar_iterate(archive);
	while (entry != NULL) { 
		ustar_mdata_t* mdata = entry->mdata;
		ustar_data_t* name = mdata->name;
		printf("(%.*s)\n", (int)(name->size), (char*)(name->ptr));
		entry = ustar_iterate(archive);
	}

	return 0;
}

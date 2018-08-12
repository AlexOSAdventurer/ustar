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
	fclose(fp);
	return data;
};

void file_write(char* fname, ustar_data_t* data) { 
	FILE* fp = fopen(fname, "w+");
	fwrite(data->ptr, 1, data->size, fp);
	fclose(fp);
}

int main(int argc, char* argv[]) { 
	if (argc < 3) {
		puts("Not enough arguments you POS!\n");
		puts("Regards, ");
		puts(argv[0]);
		putchar('\n');
		return 1;
	}
	char* fname = argv[1];
	char* fname2 = argv[2];
	printf("%s\n", fname);
	ustar_init(malloc, free);
	ustar_data_t* data = file_load(fname);	
	ustar_t* archive = ustar_parse(data);
	if (archive->status == 0) {
		ustar_entry_t* entry = ustar_iterate(archive);
		while (entry != NULL) { 
			ustar_mdata_t* mdata = entry->mdata;
			ustar_data_t* name = mdata->name;
			printf("(%.*s)\n", (int)(name->size), (char*)(name->ptr));
			entry = ustar_iterate(archive);
		}
		ustar_data_t* ndata = ustar_serialize(archive);
		file_write(fname2, ndata);
	}
	else { 
		puts(archive->error_str);
		putchar('\n');
	}
	return 0;
}

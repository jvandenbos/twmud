#ifndef MODIFY_H
#define MODIFY_H

void string_add(struct descriptor_data* d, char* str);
void show_string(struct descriptor_data* d, const char* input);
// min -- moved to proto.h void page_string(struct descriptor_data* d, const char* str,
//		 int keep_internal);
void build_help_index(void);
void delete_help_index(void);

#endif

#ifndef EDITOR_H
#define EDITOR_H

void editor_prompt(struct descriptor_data* point);
int editor_parser(struct descriptor_data* d, char* str);
void editor_get_arguments(char* linePtr, int* lineNo1,
			  int* lineNo2, char**text);

#endif

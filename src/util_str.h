#ifndef UTIL_STR
#define UTIL_STR

#if NEED_STRDUP || !defined(__cplusplus)
char* strdup(const char*);
#endif
int scan_number(const char* text, int* rval);
int str_cmp(const char*, const char*);
int strn_cmp(const char*, const char*, int);
void sprintbit(unsigned long vector, const char* names[], char* result);
void sprinttype(int type, const char* names[], char* result);
bool getall(const char* name, char* newname);
int getabunch(char* name, char* newname);
char* lower(const char* s);
int file_to_string(const char* file, char* buf, int max);
void center(char* buf, char* str, int width);
char* escape(const char* src);
int get_number(char** name);
int isname(const char *str, const char *namelist);
char *fname(char *namelist);
int split_string(char *str, char *sep, char **argv);
void split_last(const char* src, char* first, char* last);
char* one_argument(const char* src, char* buffer);
void only_argument(const char *argument,char *first_arg);
int fill_word(const char *argument);
void half_chop(const char *string, char *arg1, char *arg2);
int is_abbrev(const char *arg1, const char *arg2);
int is_number(const char* str);
int search_block(const char *arg, const char **list, bool exact);
int old_search_block(const char *argument,int begin,int length,
		     const char **list,int mode);

#endif

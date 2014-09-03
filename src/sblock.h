/* string blocks */
void init_string_block(struct string_block* sb);
void append_to_string_block(struct string_block* sb, const char* str);
// min -- void page_string_block(struct string_block* sb, struct char_data* ch);
void destroy_string_block(struct string_block* sb);


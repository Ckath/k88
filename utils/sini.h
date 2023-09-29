#ifndef SINI_H
#define SINI_H

int sini_write(INI *ini, char *section, char *item, char *value);
int sini_remove(INI *ini, char *section, char *item);
char **sini_list_sections(INI *ini);
char **sini_list_items(INI *ini, char *section);

#endif

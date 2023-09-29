#include "../ini_rw/ini_rw.h"
#include <pthread.h>

pthread_mutex_t ini_lock = PTHREAD_MUTEX_INITIALIZER;

int
sini_write(INI *ini, char *section, char *item, char *value)
{
	pthread_mutex_lock(&ini_lock);
	int r = ini_write(ini, section, item, value);
	pthread_mutex_unlock(&ini_lock);
	return r;
}

int
sini_remove(INI *ini, char *section, char *item)
{
	pthread_mutex_lock(&ini_lock);
	int r = ini_remove(ini, section, item);
	pthread_mutex_unlock(&ini_lock);
	return r;
}

char **
sini_list_sections(INI *ini)
{
	pthread_mutex_lock(&ini_lock);
	char **r = ini_list_sections(ini);
	pthread_mutex_unlock(&ini_lock);
	return r;
}

char **
sini_list_items(INI *ini, char *section)
{
	pthread_mutex_lock(&ini_lock);
	char **r = ini_list_items(ini, section);
	pthread_mutex_unlock(&ini_lock);
	return r;
}

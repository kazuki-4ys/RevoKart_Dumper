#ifndef _DIRECTORY_H_
#define _DIRECTORY_H_

#include <malloc.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

typedef struct{
    unsigned int capacity;
    unsigned int oneSize;
    void *buf;
}my_array;

typedef struct {
    char* fullpath;
    bool isDir;
} entry;

#define INITIAL_CAPACITY 512

void my_array_init(my_array *src, unsigned int oneSize);
void my_array_extend(my_array *src);
bool my_array_is_over(my_array *src, unsigned int count);
entry* getChildListRec(const char *path, unsigned int *count);
void free_entry_list(entry *src, unsigned int count);
void delDir(const char *path);
char *getNotIncludeLastSlashCopyMalloc(const char *src);
unsigned int getSlashCount(const char *_src);
unsigned int getSlashIndexByCount(const char *src, unsigned int count);
char *getParentNameMalloc(const char *src);
char *getBaseNameMalloc(const char *src);
bool isAParentOfB(const char *_a, const char *_b);
bool isABrotherOfB(const char *_a, const char *_b);

#endif//_DIRECTORY_H_
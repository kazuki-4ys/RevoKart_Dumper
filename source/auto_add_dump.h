#ifndef _AUTO_ADD_DUMP_H_
#define _AUTO_ADD_DUMP_H_

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>

#include "u8_archive.h"
#include "wdvd.h"

typedef struct{
    unsigned int subfile_count;
    const char *szs_filename;
    const char **subfile_filename;
}szs_subfile_dump_config;

void createDirectoryRecursive(char *path);
bool bytesToFile(unsigned char *buf,unsigned int fileSize,char *path);

void freeConfig(szs_subfile_dump_config* config);
szs_subfile_dump_config* getAutoAddDumpConfig(void);
bool dumpFromOneConfig(szs_subfile_dump_config* oneConfig);
bool dumpByConfig(szs_subfile_dump_config* src);

#endif//_AUTO_ADD_DUMP_H_
#ifndef _U8_ARCHIVE_H_
#define _U8_ARCHIVE_H_

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include "directory.h"

#define U8_MAGIC 0x55AA382D
#define U8_NODE_SIZE 0xC

typedef struct{
    unsigned int magic;
    unsigned int firstNodeOffset;
    unsigned int nodeAndStringTableSize;
    unsigned int fileDataOffset;
}u8_header;

typedef struct{
    unsigned int fileNameOffset;
    unsigned int fileDataOffset;
    unsigned int fileDataLength;
}u8_node;

typedef struct{
    u8_header header;
    unsigned char *nodeAndStringTable;
}u8_archive;

FILE *getFileSizeAndPointer(char *path,unsigned int *fileSize);
unsigned char *fileToBytes(char *path, unsigned int *fileSize);
bool bytesToFile(unsigned char *buf,unsigned int fileSize,char *path);
void u8_archive_ro_init(u8_archive *src, unsigned char *rawData);
void getFullPath_rec(u8_archive *src, unsigned int targetNodeIndex, char *destPath);
char* u8_get_full_path(u8_archive *src, unsigned int targetNodeIndex, char *pathBuffer);
unsigned char* u8_archive_ro_get_file(u8_archive *src, const char *targetPath, unsigned int *size);
bool create_u8_archive(const char *srcDirPath, const char *destArcPath);

#endif//_U8_ARCHIVE_H_
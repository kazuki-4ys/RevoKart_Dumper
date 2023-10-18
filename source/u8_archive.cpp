#include "u8_archive.h"

#define U8_MAGIC 0x55AA382D

FILE *getFileSizeAndPointer(char *path,unsigned int *fileSize){
    int fd = open(path,O_RDONLY);
    struct stat fileStat;
    if(fd < 0)return NULL;
    if(fstat(fd, &fileStat) == -1){
        close(fd);
        return NULL;
    }
    *fileSize = (unsigned int)fileStat.st_size;
    FILE*f = fdopen(fd, "rb");
    if(!f){
        close(fd);
        return NULL;
    }
    return f;
}

unsigned char *fileToBytes(char *path, unsigned int *fileSize){
    FILE *f = getFileSizeAndPointer(path, fileSize);
    if(!f)return NULL;
    unsigned char *buf = (unsigned char*)calloc(*fileSize, sizeof(unsigned char));
    if(!buf)return NULL;
    fread(buf, sizeof(unsigned char), *fileSize, f);
    fclose(f);
    return buf;
}

void createDirectoryRecursive(char *path){
    char *firstSlash = path;
    while(*firstSlash != '/' && *firstSlash != '\0'){
        firstSlash++;
    }
    if(!(*firstSlash))return;
    char *currentSlash = firstSlash + 1;
    while(1){
        while(*currentSlash != '/' && *currentSlash != '\0'){
            currentSlash++;
        }
        char *createDirPath = (char*)calloc((currentSlash - path) + 1, sizeof(char));
        memcpy(createDirPath, path, currentSlash - path);
        mkdir(createDirPath, 0777);
        free(createDirPath);
        if(*currentSlash == '\0')return;
        currentSlash++;
    }
}

bool bytesToFile(unsigned char *buf,unsigned int fileSize,char *path){
    unsigned int pathLen = strlen(path);
    unsigned int lastSlashIndex = pathLen;
    while(*(path + lastSlashIndex) != '/'){
        lastSlashIndex--;
    }
    char *tmpPath = (char*)calloc(lastSlashIndex + 1, sizeof(char));
    memcpy(tmpPath, path, lastSlashIndex);
    createDirectoryRecursive(tmpPath);
    free(tmpPath);
    FILE*f = fopen(path,"wb");
    if(!f)return false;
    fwrite(buf,sizeof(char),fileSize,f);
    fclose(f);
    return true;
}

void u8_archive_ro_init(u8_archive *src, unsigned char *rawData){
    src->nodeAndStringTable = NULL;
    memcpy(&(src->header), rawData, 0x10);
    src->nodeAndStringTable = (unsigned char*)((void*)(rawData + src->header.firstNodeOffset));
}

void getFullPath_rec(u8_archive *src, unsigned int targetNodeIndex, char *destPath){
    unsigned int parentNodeIndex = 0;
    u8_node *firstNode = (u8_node*)((void*)(src->nodeAndStringTable));
    u8_node *targetNode = firstNode + targetNodeIndex;
    u8_node *parentNode = NULL;
    unsigned int allNodeCount = firstNode->fileDataLength;
    if((targetNode->fileNameOffset & 0x01000000) == 0x01000000){
        parentNodeIndex = targetNode->fileDataOffset;
        parentNode = firstNode + parentNodeIndex;
    }else{
        parentNodeIndex = targetNodeIndex - 1;
        while(1){
            parentNode = firstNode + parentNodeIndex;
            if((parentNode->fileNameOffset & 0x01000000) == 0x01000000)break;
            if(!parentNodeIndex)break;
            parentNodeIndex--;
        }
    }
    char *path = (char*)((void*)(src->nodeAndStringTable + (targetNode->fileNameOffset & 0xFFFFFF) + allNodeCount * U8_NODE_SIZE));
    *destPath = '/';
    unsigned int pathLen = strlen(path);
    if((targetNode->fileNameOffset & 0x01000000) == 0x01000000){
        memcpy(destPath - pathLen, path, pathLen);
    }else{
        memcpy(destPath - pathLen + 1, path, pathLen);
    }
    if(!parentNodeIndex)return;
    if((targetNode->fileNameOffset & 0x01000000) == 0x01000000){
        getFullPath_rec(src, parentNodeIndex, destPath - pathLen - 1);
    }else{
        getFullPath_rec(src, parentNodeIndex, destPath - pathLen);
    }
}

char* u8_get_full_path(u8_archive *src, unsigned int targetNodeIndex, char *pathBuffer){
    memset(pathBuffer, 0, 1024);
    if(!targetNodeIndex)return pathBuffer;
    getFullPath_rec(src, targetNodeIndex, pathBuffer + 1022);
    char *fullPath = pathBuffer + 1022;
    while(*(fullPath) != '\0'){
        fullPath--;
    }
    fullPath++;
    return fullPath;
}

unsigned char* u8_archive_ro_get_file(u8_archive *src, const char *targetPath, unsigned int *size){
    char *tmpPathBuffer = (char*)calloc(1024, sizeof(unsigned char));
    u8_node *firstNode = (u8_node*)((void*)(src->nodeAndStringTable));
    unsigned int allNodeCount = firstNode->fileDataLength;
    for(unsigned int i = 1;i < allNodeCount;i++){
        char *path = u8_get_full_path(src, i, tmpPathBuffer);
        if(strcmp(targetPath, path))continue;
        free(tmpPathBuffer);
        unsigned int offset = (firstNode + i)->fileDataOffset;
        *size = (firstNode + i)->fileDataLength;
        return ((unsigned char*)((void*)src->nodeAndStringTable)) - src->header.firstNodeOffset + offset;
    }
    free(tmpPathBuffer);
    return NULL;
}

bool isASaki(entry *a, entry *b);

unsigned int padTo0x20(unsigned int val){
    if(!(val & 0x1F))return val;
    val = (val & (~0x1F));
    return val + 0x20;
}

unsigned int findParentIndex(const char *path, entry *src, unsigned int count){
    char* parentPath = getParentNameMalloc(path);
    for(unsigned int i = 0;i < count;i++){
        if(!strcmp(parentPath, (src + i)->fullpath)){
            free(parentPath);
            return i;
        }
    }
    free(parentPath);
    return count;
}

unsigned int findNotChildFirst(unsigned int myIndex, entry *src, unsigned int count){
    unsigned int start = myIndex + 1;
    char *me = (src + myIndex)->fullpath;
    unsigned int meLen = strlen(me);
    while(count > start){
        char *target = (src + start)->fullpath;
        unsigned int targetLen = strlen(target);
        if((targetLen > meLen) && (target[meLen] == '/') && (!memcmp(me, target, meLen))){
            start++;
            continue;
        }
        return start;
    }
    return count;
}

void writePadding(FILE *f, unsigned int paddingSize){
    if(!paddingSize)return;
    unsigned char *buf = (unsigned char*)calloc(paddingSize, sizeof(unsigned char));
    fwrite(buf, sizeof(unsigned char), paddingSize, f);
    free(buf);
}

bool create_u8_archive(const char *srcDirPath, const char *destArcPath){
    unsigned int childrenCount = 0;
    entry *children = getChildListRec(srcDirPath, &childrenCount);
    u8_node *nodes = (u8_node*)calloc(2 + childrenCount, sizeof(u8_node));
    unsigned int stringPoolSize = 3;
    //caluculate string pool size
    for(unsigned int i = 0;i < childrenCount;i++){
        char *baseName = getBaseNameMalloc((children + i)->fullpath);
        stringPoolSize += strlen(baseName);
        free(baseName);
        stringPoolSize++;
    }
    unsigned int curFileOffset = padTo0x20((2 + childrenCount) * 0xC + stringPoolSize + 0x20);
    unsigned char *stringPool = (unsigned char*)calloc(stringPoolSize, sizeof(unsigned char));
    stringPool[1] = '.';
    unsigned int curStringPoolOffset = 3;
    for(unsigned int i = 0;i < childrenCount;i++){
        char *baseName = getBaseNameMalloc((children + i)->fullpath);
        memcpy(stringPool + curStringPoolOffset, baseName, strlen(baseName));
        (nodes + 2 + i)->fileNameOffset = curStringPoolOffset;
        curStringPoolOffset += strlen(baseName);
        curStringPoolOffset++;
        if((children + i)->isDir){
            (nodes + 2 + i)->fileNameOffset |= 0x01000000;
            unsigned int parentIndex = findParentIndex((children + i)->fullpath, children, childrenCount);
            if(parentIndex == childrenCount){
                parentIndex = 1;
            }else{
                parentIndex += 2;
            }
            (nodes + 2 + i)->fileDataOffset = parentIndex;
            unsigned int notChildIndex = findNotChildFirst(i, children, childrenCount);
            (nodes + 2 + i)->fileDataLength = notChildIndex + 2;
        }else{
            unsigned int fileSize = 0;
            FILE *f = getFileSizeAndPointer((children + i)->fullpath, &fileSize);
            fclose(f);
            (nodes + 2 + i)->fileDataOffset = curFileOffset;
            (nodes + 2 + i)->fileDataLength = fileSize;
            curFileOffset += fileSize;
            curFileOffset = padTo0x20(curFileOffset);
        }
        free(baseName);
    }
    /*for(unsigned int i = 0;i < childrenCount;i++){
        printf("u8list = %s\n", (children + i)->fullpath);
    }*/
    (nodes + 0)->fileNameOffset = 0x01000000;
    (nodes + 0)->fileDataOffset = 0;
    (nodes + 0)->fileDataLength = childrenCount + 2;
    (nodes + 1)->fileNameOffset = 0x01000001;
    (nodes + 1)->fileDataOffset = 0;
    (nodes + 1)->fileDataLength = childrenCount + 2;
    unsigned char *header = (unsigned char*)calloc(0x20, sizeof(unsigned char));
    memset(header + 0x10, 0xCC, 0x10);
    *((unsigned int*)((void*)(header))) = U8_MAGIC;
    *((unsigned int*)((void*)(header + 4))) = 0x20;
    *((unsigned int*)((void*)(header + 8))) = 0xC * (childrenCount + 2) + stringPoolSize;
    curFileOffset = padTo0x20((2 + childrenCount) * 0xC + stringPoolSize + 0x20);
    *((unsigned int*)((void*)(header + 0xC))) = curFileOffset;
    FILE *f = fopen(destArcPath, "wb");
    if(!f){
        free(header);
        free(nodes);
        free(stringPool);
        free_entry_list(children, childrenCount);
        return false;
    }
    fwrite(header, sizeof(unsigned char), 0x20, f);
    fwrite(nodes, sizeof(u8_node), 2 + childrenCount, f);
    fwrite(stringPool, sizeof(unsigned char), stringPoolSize, f);
    free(header);
    free(nodes);
    free(stringPool);
    char padBuffer[0x20];
    memset(padBuffer, 0, 0x20);
    curFileOffset = (2 + childrenCount) * 0xC + stringPoolSize + 0x20;
    for(unsigned int i = 0;i < childrenCount;i++){
        if((children + i)->isDir)continue;
        unsigned int next = padTo0x20(curFileOffset);
        unsigned int padSize = next - curFileOffset;
        fwrite(padBuffer, padSize, sizeof(char), f);
        //writePadding(f, next - curFileOffset);
        curFileOffset = next;
        unsigned int fileDataSize = 0;
        unsigned char *fileData = fileToBytes((children + i)->fullpath, &fileDataSize);
        //printf("fileWrite = %s\n", (children + i)->fullpath);
        if(!fileData){
            free_entry_list(children, childrenCount);
            fclose(f);
            return false;
        }
        if(fwrite(fileData, sizeof(unsigned char), fileDataSize, f) != fileDataSize){
            free_entry_list(children, childrenCount);
            fclose(f);
            free(fileData);
            return false;
        }
        curFileOffset += fileDataSize;
        free(fileData);
    }
    unsigned int next = padTo0x20(curFileOffset);
    unsigned int padSize = next - curFileOffset;
    fwrite(padBuffer, padSize, sizeof(char), f);
    free_entry_list(children, childrenCount);
    fclose(f);
    return true;
}

bool isASaki(entry *a, entry *b){
    char *aStr = getNotIncludeLastSlashCopyMalloc(a->fullpath);
    char *bStr = getNotIncludeLastSlashCopyMalloc(b->fullpath);
    if(isAParentOfB(aStr, bStr)){
        free(aStr);
        free(bStr);
        return true;
    }else if(isAParentOfB(bStr, aStr)){
        free(aStr);
        free(bStr);
        return false;
    }else if(isABrotherOfB(aStr, bStr)){
        if(a->isDir == b->isDir){
            int ret = strcmp(aStr, bStr);
            free(aStr);
            free(bStr);
            if(ret > 0)return false;
            return true;
        }else{
            free(aStr);
            free(bStr);
            if(a->isDir){
                return false;
            }
            return true;
        }
    }
    unsigned int commonIndex = 0;
    while(aStr[commonIndex] == bStr[commonIndex]){
        commonIndex++;
    }
    unsigned int aRealCompIndex = commonIndex;
    while(!(aStr[aRealCompIndex] == '/' || aStr[aRealCompIndex] == '\0')){
        aRealCompIndex++;
    }
    aRealCompIndex--;
    unsigned int bRealCompIndex = commonIndex;
    while(!(bStr[bRealCompIndex] == '/' || bStr[bRealCompIndex] == '\0')){
        bRealCompIndex++;
    }
    bRealCompIndex--;
    char *aCompStr = (char*)calloc(aRealCompIndex + 1, sizeof(char));
    char *bCompStr = (char*)calloc(bRealCompIndex + 1, sizeof(char));
    memcpy(aCompStr, aStr, aRealCompIndex);
    memcpy(bCompStr, bStr, bRealCompIndex);
    int ret = strcmp(aCompStr, bCompStr);
    free(aCompStr);
    free(bCompStr);
    free(aStr);
    free(bStr);
    if(ret > 0)return false;
    return true;
}
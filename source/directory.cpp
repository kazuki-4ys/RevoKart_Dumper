#include "directory.h"

void my_array_init(my_array *src, unsigned int oneSize){
    src->buf = calloc(INITIAL_CAPACITY, oneSize);
    src->oneSize = oneSize;
    src->capacity = INITIAL_CAPACITY;
}
void my_array_extend(my_array *src){
    src->buf = realloc(src->buf, src->capacity * src->oneSize * 2);
    src->capacity *= 2;
}

bool my_array_is_over(my_array *src, unsigned int count){
    return src->capacity < count;
}

char *getLowwerTextMalloc(const char *_s){
    unsigned int ss = strlen(_s);
    char *s = (char*)calloc(ss + 1, sizeof(char));
    memcpy(s, _s, ss);
    for(unsigned int i = 0;i < ss;i++){
        if(s[i] > 0x60 && s[i] < 0x7B)s[i] -= 0x20;
    }
    return s;
}

int my_strcmp(const char *_s1, const char *_s2){
    char *s1 = getLowwerTextMalloc(_s1);
    char *s2 = getLowwerTextMalloc(_s2);
    int ret = strcmp(s1, s2);
    free(s1);
    free(s2);
    return ret;
}

entry* getChildListRec(const char *path, unsigned int *count){
    my_array dest;
    my_array_init(&dest, sizeof(entry));
    unsigned int curIndex = 0;
    DIR *dir;
    struct dirent *direntEntry;
    dir = opendir(path);
    if (dir == NULL) {
        *count = 0;
        return NULL;
    }
    unsigned int dirCount = 0;
    while ((direntEntry = readdir(dir)) != NULL) {
        if(my_array_is_over(&dest, curIndex + 1)){
            my_array_extend(&dest);
        }
        if (strcmp(direntEntry->d_name, ".") == 0 || strcmp(direntEntry->d_name, "..") == 0) {
            continue;
        }
        if (direntEntry->d_type == DT_DIR) {
            dirCount++;
            continue;
        }
        (((entry*)(dest.buf)) + curIndex)->fullpath = (char*)calloc(strlen(path) + strlen(direntEntry->d_name) + 2, sizeof(char));
        if(*(path + strlen(path) - 1) == '/'){
            sprintf((((entry*)(dest.buf)) + curIndex)->fullpath, "%s%s", path, direntEntry->d_name);
        }else{
            sprintf((((entry*)(dest.buf)) + curIndex)->fullpath, "%s/%s", path, direntEntry->d_name);
        }
        (((entry*)(dest.buf)) + curIndex)->isDir = false;
        curIndex++;
    }
    closedir(dir);
    if(curIndex > 1){
        for(int i = 0;i < curIndex;i++){
            for(int j = i + 1; j < curIndex; j++){
                entry tmp;
                if(my_strcmp((((entry*)(dest.buf)) + i)->fullpath, (((entry*)(dest.buf)) + j)->fullpath) > 0){
                    tmp = *(((entry*)(dest.buf)) + i);
                    *(((entry*)(dest.buf)) + i) = *(((entry*)(dest.buf)) + j);
                    *(((entry*)(dest.buf)) + j) = tmp;
                }
            }
        }
    }
    unsigned int curDirIndex = 0;
    char **childrenFullname = (char**)calloc(sizeof(char*), dirCount);
    dir = opendir(path);
    //printf("dirCount = %d\n", dirCount);
    while ((direntEntry = readdir(dir)) != NULL && curDirIndex < dirCount) {
        if (strcmp(direntEntry->d_name, ".") == 0 || strcmp(direntEntry->d_name, "..") == 0 || direntEntry->d_type != DT_DIR) {
            continue;
        }
        childrenFullname[curDirIndex] = (char*)calloc(strlen(path) + strlen(direntEntry->d_name) + 2, sizeof(char));
        if(*(path + strlen(path) - 1) == '/'){
            sprintf(childrenFullname[curDirIndex], "%s%s", path, direntEntry->d_name);
        }else{
            sprintf(childrenFullname[curDirIndex], "%s/%s", path, direntEntry->d_name);
        }
        curDirIndex++;
    }
    closedir(dir);
    if(dirCount > 1){
        for(int i = 0;i < dirCount;i++){
            for(int j = i + 1; j < dirCount; j++){
                char *tmp;
                if(my_strcmp(childrenFullname[i], childrenFullname[j]) > 0){
                    tmp = childrenFullname[i];
                    childrenFullname[i] = childrenFullname[j];
                    childrenFullname[j] = tmp;
                }
            }
        }
    }
    for(unsigned int i = 0;i < dirCount;i++){
        unsigned int childrenChildrenCount;
        entry* childrenChildren = getChildListRec(childrenFullname[i], &childrenChildrenCount);
        if(my_array_is_over(&dest, curIndex + 1 + childrenChildrenCount)){
            my_array_extend(&dest);
        }
        (((entry*)(dest.buf)) + curIndex)->isDir = true;
        (((entry*)(dest.buf)) + curIndex)->fullpath = childrenFullname[i];
        memcpy(((entry*)(dest.buf)) + curIndex + 1, childrenChildren, sizeof(entry) * childrenChildrenCount);
        free(childrenChildren);
        curIndex++;
        curIndex += childrenChildrenCount;
    }
    free(childrenFullname);
    *count = curIndex;
    return (entry*)(dest.buf);
}

void free_entry_list(entry *src, unsigned int count){
    for(unsigned int i = 0;i < count;i++)free((src + i)->fullpath);
    free(src);
}

void delDir(const char *path){
    unsigned int childrenCount;
    entry *children = getChildListRec(path, &childrenCount);
    for(unsigned int i = 0;i < childrenCount;i++){
        if((children + i)->isDir){
            delDir((children + i)->fullpath);
        }else{
            remove((children + i)->fullpath);
        }
    }
    free_entry_list(children, childrenCount);
    rmdir(path);
}

char *getNotIncludeLastSlashCopyMalloc(const char *src){
    unsigned int srcLen = strlen(src);
    char *dest = (char*)calloc(srcLen + 1, sizeof(char));
    if(src[srcLen] == '/'){
        memcpy(dest, src, srcLen - 1);
    }else{
        memcpy(dest, src, srcLen);
    }
    return dest;
}

unsigned int getSlashCount(const char *_src){
    unsigned int dest = 0;
    unsigned int index = 0;
    char *src = getNotIncludeLastSlashCopyMalloc(_src);
    while(src[index]){
        if(src[index] == '/')dest++;
        index++;
    }
    free(src);
    return dest;
}

unsigned int getSlashIndexByCount(const char *src, unsigned int count){
    unsigned int dest = 0;
    unsigned int index = 0;
    while(src[index]){
        if(src[index] == '/')dest++;
        if(dest != 0 && (dest - 1 == count))return index;
        index++;
    }
    return strlen(src) + 1;
}

char *getParentNameMalloc(const char *src){
    unsigned int slashCount = getSlashCount(src);
    unsigned int index = getSlashIndexByCount(src, slashCount - 1);
    char *dest = (char*)calloc(index + 1, sizeof(char));
    memcpy(dest, src, index);
    return dest;
}

char *getBaseNameMalloc(const char *src){
    unsigned int lastSlashIndex = getSlashIndexByCount(src, getSlashCount(src) - 1);
    char *src2 = getNotIncludeLastSlashCopyMalloc(src);
    unsigned int srcLen = strlen(src2);
    free(src2);
    unsigned int destLen = (srcLen - 1) - lastSlashIndex;
    char *dest = (char*)calloc(destLen + 1, sizeof(char));
    memcpy(dest, src + lastSlashIndex + 1, destLen);
    return dest;
}

bool isAParentOfB(const char *_a, const char *_b){
    char *a = getNotIncludeLastSlashCopyMalloc(_a);
    char *b = getNotIncludeLastSlashCopyMalloc(_b);
    if(strlen(b) > strlen(a)){
        if(b[strlen(a)] == '/'){
            if(!memcmp(a, b, strlen(a))){
                free(a);
                free(b);
                return true;
            }
        }
    }
    free(a);
    free(b);
    return false;
}

bool isABrotherOfB(const char *_a, const char *_b){
    char *a = getNotIncludeLastSlashCopyMalloc(_a);
    char *b = getNotIncludeLastSlashCopyMalloc(_b);
    unsigned int aLastSlashIndex = getSlashIndexByCount(a, getSlashCount(a) - 1);
    unsigned int bLastSlashIndex = getSlashIndexByCount(b, getSlashCount(b) - 1);
    if(aLastSlashIndex == bLastSlashIndex){
        if(!memcmp(a, b, aLastSlashIndex)){
            free(a);
            free(b);
            return true;
        }
    }
    free(a);
    free(b);
    return false;
}
/*
 *  Wii DVD interface API
 *  Copyright (C) 2008 Jeff Epler <jepler@unpythonic.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <ogc/ipc.h>
#include <string.h>
#include "wdvd.h"

#define min(a,b) ((a)>(b) ? (b) : (a))
#define max(a,b) ((a)>(b) ? (a) : (b))

struct DiscNode;
DiscNode* fst = NULL;
u32 partition_info[24] ATTRIBUTE_ALIGN(32);

static int di_fd = -1;

// Apparently GCC is too cool for casting.
static union {
	u32 inbuffer[0x10]; //u32 inbuffer[0x10] ATTRIBUTE_ALIGN(32);
	ioctlv iovector[0x08];
} ATTRIBUTE_ALIGN(32);

int WDVD_Init() {
    if (di_fd >= 0)
		return di_fd;

   	di_fd = IOS_Open("/dev/di", 0);

    return di_fd;
}

bool WDVD_Reset() {
	if (di_fd < 0)
		return false;

	inbuffer[0x00] = 0x8A000000;
	inbuffer[0x01] = 1;
	return (IOS_Ioctl(di_fd, 0x8A, inbuffer, 0x20, inbuffer + 0x08, 0x20)==1);
}

int WDVD_LowUnencryptedRead(void *buf, u32 len, u64 offset) {
	int result;

	if (di_fd<0)
		return -1;

	inbuffer[0] = 0x8d000000;
	inbuffer[1] = len;
	inbuffer[2] = offset >> 2;

	result = IOS_Ioctl(di_fd, 0x8d, inbuffer, 0x20, buf, len);

	return (result==1) ? 0 : -result;
}


int WDVD_LowRead(void *buf, u32 len, u64 offset) {
	int result;

	if (di_fd<0 || (u32)buf&0x1F)
		return -1;

	memset(inbuffer, 0, 0x20);
	inbuffer[0] = 0x71000000;
	inbuffer[1] = len;
	inbuffer[2] = offset >> 2;

	result = IOS_Ioctl(di_fd, 0x71, inbuffer, 0x20, buf, len);

	return (result==1) ? 0 : -result;
}

bool WDVD_LowClosePartition() {

	if (di_fd<0)
		return false;

	inbuffer[0x00] = 0x8C000000;

	return !!IOS_Ioctl(di_fd, 0x8C, inbuffer, 0x20, 0, 0);
}

int WDVD_LowReadDiskId() {
	int result;
	void *outbuf = (void*)0x80000000;

	if (di_fd<0)
		return -1;

	memset(outbuf, 0, 0x20);
	inbuffer[0] = 0x70000000;

	result = IOS_Ioctl(di_fd, 0x70, inbuffer, 0x20, outbuf, 0x20);
	return (result==1) ? 0 : -result;
}

// ugh. Don't even need this tmd shit.
static u8 tmd_data[0x49e4] ATTRIBUTE_ALIGN(32);
static u8 errorbuffer[0x40] ATTRIBUTE_ALIGN(32);
int WDVD_LowOpenPartition(u64 offset) {
	int result;

	if (di_fd<0)
		return -1;

	WDVD_LowClosePartition();

	inbuffer[12] = 0x8b000000;
	inbuffer[13] = offset >> 2;
	inbuffer[14] = inbuffer[15] = 0;

	iovector[0].data = inbuffer+12;
	iovector[0].len = 0x20;
	iovector[1].data = 0;
	iovector[1].len = 0x24a;
	iovector[2].data = 0;
	iovector[2].len = 0;
	iovector[3].data = tmd_data;
	iovector[3].len = 0x49e4;
	iovector[4].data = errorbuffer;
	iovector[4].len = 0x20;

	result = IOS_Ioctlv(di_fd, 0x8B, 3, 2, iovector);

	return (result==1) ? 0 : -result;
}
tmd* WDVD_GetTMD()
{
	return (tmd*)tmd_data;
}

void WDVD_Close() {
	if (di_fd < 0)
		return;

	IOS_Close(di_fd);
	di_fd = -1;
}

int WDVD_VerifyCover(bool* cover) {
	*cover = false;
	if (di_fd<0)
		return -1;

	inbuffer[0] = 0xdb000000;
	if (IOS_Ioctl(di_fd, 0xdb, inbuffer, 0x20, inbuffer + 0x08, 0x20) != 1)
		return -1;

	*cover = !inbuffer[0x08];
	return 0;
}

static DiscNode* RVL_FindNode(const char* name, DiscNode* root, bool recursive)
{
	const char* nametable = (const char*)(fst + fst->Size);
	int offset = root - fst;
	DiscNode* node = root;
	while ((void*)node < (void*)nametable) {
		if (!strcasecmp(nametable + node->GetNameOffset(), name))
			return node;

		if (recursive || node->Type == 0)
			node++;
		else
			node = root + node->Size - offset;
	}

	return NULL;
}

DiscNode* RVL_FindNode(const char* fstname)
{
	if (fstname[0] != '/')
		return RVL_FindNode(fstname, fst, true);

	char namebuffer[IPC_MAXPATH_LEN];
	char* name = namebuffer;
	strcpy(name, fstname + 1);

	DiscNode* root = fst;

	while (root) {
		char* slash = strchr(name, '/');
		if (!slash)
			return RVL_FindNode(name, root + 1, false);

		*slash = '\0';
		root = RVL_FindNode(name, root + 1, false);
		name = slash + 1;
	}

	return NULL;
}

string PathCombine(string path, string file)
{
	if (path.empty())
		return file;
	if (file.empty())
		return path;

	bool flag = true;
	while (flag) {
		flag = false;
		if (!strncmp(file.c_str(), "../", 3)) {
			string::size_type pos = path.find_last_of('/', path.size() - 1);
			if (pos != string::npos)
				path = path.substr(0, pos);

			file = file.substr(3);

			flag = true;
		} else if (!strncmp(file.c_str(), "./", 2)) {
			file = file.substr(2);

			flag = true;
		}
	}

	if (path[path.size() - 1] == '/' && file[0] == '/')
		return path + file.substr(1);

	if (path[path.size() - 1] == '/' || file[0] == '/')
		return path + file;

	return path + "/" + file;
}

bool DumpFolder(DiscNode* node, string path)
{
	if (!node)
		return false;

	if (!node->Type)
		return false;

	mkdir(path.c_str(), 0777);
	printf("\nDumping to %s...", path.c_str());

	for (DiscNode* file = node + 1; file < fst + node->Size; ) {
		string name = PathCombine(path, file->GetName());
		if (file->Type) {
			bool ret = DumpFolder(file, name);
			if (!ret)
				return false;
		} else {
			FILE* fd = fopen(name.c_str(), "wb");
			if (fd) {
				static u8 buffer[0x8000] ATTRIBUTE_ALIGN(32);
				memset(buffer, 0, 0x8000);
				u32 written = 0;
				u32 toRead = (file->Size > 0x8000) ? 0x8000 : file->Size;
				while (written < file->Size) {
					if (WDVD_LowRead(buffer, toRead, ((u64)file->DataOffset << 2) + written))
						return false;
					//int towrite = MIN(sizeof(buffer), file->Size - written);
					int towrite = min(sizeof(buffer), file->Size - written);
					fwrite(buffer, 1, towrite, fd);
					written += towrite;
					if ( file->Size - written < 0x8000 ) toRead = file->Size - written;
				}
				fclose(fd);
				printf(".");
			} else {
				return false;
			}
		}
		file = file->Type ? fst + file->Size : file + 1;
	}

	return true;
}

bool DumpFolder(const char* disc, string path)
{
	DiscNode* node = !strcmp(disc, "/") ? fst : RVL_FindNode(disc);
	return DumpFolder(node, path);
}

bool DumpMainDol(void)
{
	FILE *header_nfo = fopen("/reggie/header.bin", "wb");
	if (!header_nfo)
		return false;

	u32 written = fwrite((void*)0x80000000, 1, 0x20, header_nfo);
	if (written != 0x20) {
		printf("Expected %d got %d\n", 0x20, written);
		fclose(header_nfo);
		return false;
	}
	fclose(header_nfo);
	printf("Dumped header.bin\n");

	u8 * dol_nfo = (u8*)memalign(32, 0x100);
	if (!dol_nfo)
		return false;

	WDVD_LowRead(dol_nfo, 0x100, (u64)fstdata[0] << 2);
    u32 max = 0;
    for (u32 i = 0; i < 7; i++) {
        u32 offset = *(u32 *)(dol_nfo + (i * 4));
        u32 size = *(u32 *)(dol_nfo + (i * 4) + 0x90);
        if ((offset + size) > max) max = offset + size;
    }
    for (u32 i = 0; i < 11; i++) {
        u32 offset = *(u32 *)(dol_nfo + (i * 4) + 0x1c);
        u32 size = *(u32 *)(dol_nfo + (i * 4) + 0xac);
        if ((offset + size) > max) max = offset + size;
    }
	free(dol_nfo);

	void * dol = memalign(32, max);
	if (!dol)
		return false;

	if (WDVD_LowRead(dol, max, (u64)fstdata[0] << 2))
	{
		printf("Failed to read main.dol\n");
		free(dol);
		return false;
	}

	FILE *main_dol = fopen("/reggie/main.dol", "wb");
	if (!main_dol)
	{
		printf("Failed to open /reggie/main.dol for write\n");
		free(dol);
		return false;
	}

	bool ret = true;
	written = fwrite(dol, 1, max, main_dol);
	if (written != max)
	{
		printf("Expected %d got %d\n", max, written);
		ret = false;
	}
	else
	{
		printf("Dumped main.dol\n");
	}
	fclose(main_dol);
	free(dol);
	return ret;
}

void* allocate_memory(unsigned int size){
    void* buf = memalign(32,(size+31)&(~31));
	memset(buf,0,(size+31)&(~31));
	return buf;
}

unsigned char* loadFileToMemory(const char *path, unsigned int *fileSize){
    DiscNode* file = RVL_FindNode(path);
    if(!file)return NULL;
    unsigned int size = file->Size;
    unsigned char *dest = (unsigned char*)allocate_memory(size);
	*fileSize = size;
    if(WDVD_LowRead(dest, size, ((u64)file->DataOffset << 2))){
		free(dest);
		return NULL;
	}
    return dest;
}
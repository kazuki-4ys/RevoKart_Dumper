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

#pragma once

#include <ogc/es.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>

using namespace std;

int WDVD_Init();
bool WDVD_Reset();
int WDVD_LowRead(void *buf, u32 len, u64 offset);
int WDVD_LowUnencryptedRead(void *buf, u32 len, u64 offset);
int WDVD_LowReadDiskId();
int WDVD_LowOpenPartition(u64 offset);
int WDVD_VerifyCover(bool* cover);
tmd* WDVD_GetTMD();
void WDVD_Close();
unsigned char* loadFileToMemory(const char *path, unsigned int *fileSize);

struct DiscNode;
extern DiscNode* fst;
extern u32 fstdata[0x40] ATTRIBUTE_ALIGN(32);

struct DiscNode
{
	u8 Type;
	u8 NameOffsetMSB;
	u16 NameOffset; //really a u24
	u32 DataOffset;
	u32 Size;
	u32 GetNameOffset() { return ((u32)NameOffsetMSB << 16) | NameOffset; }
	void SetNameOffset(u32 offset) { NameOffset = (u16)offset; NameOffsetMSB = offset >> 16; }
	const char* GetName() { return (const char*)(fst + fst->Size) + GetNameOffset(); }
} __attribute__((packed));

extern u32 partition_info[24] ATTRIBUTE_ALIGN(32);
/*
 *  Reggie Dumper
 *  Copyright (C) 2012 AerialX, megazig
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

#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ogcsys.h>
#include <sys/stat.h>
#include <sdcard/wiisd_io.h>
#include <fat.h>

#include <string>

#include "wdvd.h"
#include "auto_add_dump.h"
#include "directory.h"

#define AUTO_ADD_ARC_PATH "sd:/rk_dumper/auto-add.arc"

#define PART_OFFSET			0x00040000

#define min(a,b) ((a)>(b) ? (b) : (a))
#define max(a,b) ((a)>(b) ? (a) : (b))

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

int sdInitialize = 0;

int SD_Initialize(){
	int ret = fatMountSimple("sd", &__io_wiisd);
	if(!ret) return ret;
	return 1;
}

void SD_Deinitialize(){
	fatUnmount("sd:/");
    __io_wiisd.shutdown();
}

u32 fstdata[0x40] ATTRIBUTE_ALIGN(32);
bool Launcher_ReadFST()
{
	memset(partition_info, 0, sizeof(partition_info));
	if (WDVD_LowUnencryptedRead(partition_info, 0x20, PART_OFFSET) || partition_info[0] == 0)
		return false;
	if (WDVD_LowUnencryptedRead(partition_info + 8, max(4, min(8, partition_info[0])) * 8, (u64)partition_info[1] << 2))
		return false;
	u32 i;
	for (i = 0; i < partition_info[0]; i++)
		if (partition_info[i * 2 + 8 + 1] == 0)
			break;
	if (i >= partition_info[0])
		return false;
	if (WDVD_LowOpenPartition((u64)partition_info[i * 2 + 8] << 2))
		return false;

	if (WDVD_LowRead(fstdata, 0x40, 0x420))
		return false;

	fstdata[2] <<= 2;
	fst = (DiscNode*)memalign(32, fstdata[2]);
	if (WDVD_LowRead(fst, fstdata[2], (u64)fstdata[1] << 2))
		return false;

	return true;
}

bool Launcher_DiscInserted()
{
	bool cover;
	if (!WDVD_VerifyCover(&cover))
		return cover;
	return false;
}

volatile unsigned int diskInitResult = 0;
volatile bool isMainTaskRunning = false;

void *waitForDiskThread(void *arg){
	if(!SD_Initialize()){
        printf("SD card not inserted.\n");
		printf("\nPress HOME to exit.\n\n");
		diskInitResult = 2;
        return NULL;
    }
	printf("\nPress HOME to exit.\n\n");
    WDVD_Init();
reinsert_disc:
	printf("Insert disc... ");
	WDVD_Reset();
    while (!Launcher_DiscInserted())
		usleep(16667);

	printf("Done.\n");

	printf("Checking disc...\n");

	if (WDVD_LowReadDiskId() || memcmp((void*)0x80000000, "RMC", 3)) {
		printf("This is not MarioKartWii disc.\n");
		goto reinsert_disc;
	}
	char discID[5];
	discID[4] = 0;
	memcpy(discID, (void*)0x80000000, 4);
	printf("MarioKartWii disc (%s) found.\n", discID);
    if (!Launcher_ReadFST()) {
		printf("There was an error reading the disc. Press Home to exit, and try again.\n");
		SD_Deinitialize();
		diskInitResult = 2;
		return NULL;
	}
	diskInitResult = 1;
	return NULL;
}

void *mainTaskThread(void *arg){
	szs_subfile_dump_config* config = getAutoAddDumpConfig();
	printf("Deleting previously dumped files...\n");
	remove(AUTO_ADD_ARC_PATH);
    mkdir("sd:/rk_dumper", 0777);
	delDir("sd:/rk_dumper/auto-add");
    mkdir("sd:/rk_dumper/auto-add", 0777);
	bool ret = dumpByConfig(config);
	freeConfig(config);
	if(!ret){
		SD_Deinitialize();
		isMainTaskRunning = false;
		return NULL;
	}
	printf("Writing to auto-add.arc...\n");
	create_u8_archive("sd:/rk_dumper/auto-add", "sd:/rk_dumper/auto-add.arc");
	SD_Deinitialize();
	isMainTaskRunning = false;
	printf("\nAll done !\n");
	printf("Press HOME to exit.\n");
    return NULL;
}

bool alreadyStartMainTask = false;

void myExit(void){
	printf("Exiting...");
	exit(0);
}

int main(void){
	unsigned int homeButtonHoldCount = 0;
    // Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb,20,20,rmode->fbWidth,rmode->xfbHeight,rmode->fbWidth*VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();


	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");
    printf("RevoKart Dumper v0.1\n");
	printf("by kazuki_4ys\n");
	printf("auto-add library dumper for MarioKartWii.\n");
	printf("Based on Reggie! Dumper\n");
	printf("by AerialX and megazig\n");

	void *waitForDiskThreadArg (void *arg);
    lwp_t waitForDiskThreadHandle = (lwp_t)NULL;
    LWP_CreateThread(&waitForDiskThreadHandle,	waitForDiskThread, NULL, NULL, 64*1024, 50);
    while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME ) exit(0);
		// Wait for the next frame
		VIDEO_WaitVSync();
		if(diskInitResult == 1)break;
	}

    void *mainTaskThreadArg (void *arg);
    lwp_t mainTaskThreadHandle = (lwp_t)NULL;
	printf("\nPress A to continue.\n");
	printf("Press HOME to exit.\n");
    while(1) {

		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);
		u32 hold = WPAD_ButtonsHeld(0);

		// We return to the launcher application via exit
		if ( pressed & WPAD_BUTTON_HOME ){
			if(isMainTaskRunning){
				printf("\n\x1b[31mDUMP PROCESS IN RUNNING.\nTO FORCE KILL THIS PROCESS, HOLD HOME BUTTON FOR 3 SECONDS\x1b[39m\n");
			}else{
				myExit();
			}
		}

		if ( hold & WPAD_BUTTON_HOME ){
			homeButtonHoldCount++;
		}else{
			homeButtonHoldCount = 0;
		}
		if(homeButtonHoldCount > 180)myExit();

		if ( pressed & WPAD_BUTTON_A ){
			if(!alreadyStartMainTask){
				alreadyStartMainTask = true;
				isMainTaskRunning = true;
				LWP_CreateThread(&mainTaskThreadHandle,	mainTaskThread, NULL, NULL, 64*1024, 50);
			}
		}

		// Wait for the next frame
		VIDEO_WaitVSync();
	}

	return 0;
}
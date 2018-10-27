#include <string.h>
#include <stdio.h>
#include <dirent.h>

#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>


#include <switch.h>

#include "util.h"

#include "mp3.h"

#define TITLE_ID 0x420000000000000F
#define HEAP_SIZE 0x000540000

// we aren't an applet
u32 __nx_applet_type = AppletType_None;

// setup a fake heap
char fake_heap[HEAP_SIZE];

bool rfirst = true;
bool attach = false;

// we override libnx internals to do a minimal init
void __libnx_initheap(void)
{
	extern char *fake_heap_start;
	extern char *fake_heap_end;

	// setup newlib fake heap
	fake_heap_start = fake_heap;
	fake_heap_end = fake_heap + HEAP_SIZE;
}

void registerFspLr()
{
	if (kernelAbove400())
		return;

	Result rc = fsprInitialize();
	if (R_FAILED(rc))
		fatalLater(rc);

	u64 pid;
	svcGetProcessId(&pid, CUR_PROCESS_HANDLE);

	rc = fsprRegisterProgram(pid, TITLE_ID, FsStorageId_NandSystem, NULL, 0, NULL, 0);
	if (R_FAILED(rc))
		fatalLater(rc);
	fsprExit();
}

void __appInit(void)
{
	Result rc;
	svcSleepThread(10000000000L);
	rc = smInitialize();
	if (R_FAILED(rc))
		fatalLater(rc);
	rc = fsInitialize();
	if (R_FAILED(rc))
		fatalLater(rc);
	registerFspLr();
	rc = fsdevMountSdmc();
	if (R_FAILED(rc))
		fatalLater(rc);
	rc = timeInitialize();
	if (R_FAILED(rc))
		fatalLater(rc);
	rc = hidInitialize();
	if (R_FAILED(rc))
		fatalLater(rc);
}

void __appExit(void)
{
	fsdevUnmountAll();
	fsExit();
	smExit();
	audoutExit();
	timeExit();
}


void inputPoller()
{
	mp3MutInit();
	pauseInit();
	
	while (appletMainLoop())
	{
		svcSleepThread(1e+8L);
	
		HidSharedMemory *hmem = (HidSharedMemory*)hidGetSharedmemAddr();
		if(rfirst)
		{
			rfirst = false;
			u64 connection = hmem->controllers[0].layouts[6].entries[hmem->controllers[0].layouts[6].header.latestEntry].connectionState;
			attach = !(connection & CONTROLLER_STATE_CONNECTED);
		}
		u64 connection = hmem->controllers[0].layouts[6].entries[hmem->controllers[0].layouts[6].header.latestEntry].connectionState;
		bool nattach = !(connection & CONTROLLER_STATE_CONNECTED);
		if(nattach != attach)
		{
			attach = nattach;
			if(nattach){
				playMp3("SlideNX/attach.mp3");
			}
			else{
				playMp3("SlideNX/detach.mp3");
			}	
		}
	}
}

int main(int argc, char **argv)
{
	(void)argc;
	(void)argv;

	Thread pauseThread;
	Result rc = threadCreate(&pauseThread, inputPoller, NULL, 0x4000, 49, 3);
	if (R_FAILED(rc))
		fatalLater(rc);
	rc = threadStart(&pauseThread);
	if (R_FAILED(rc))
		fatalLater(rc);


	while (true)
	{
		while (isPaused())
		{
			svcSleepThread(1000000000L);
		}
	}

	return 0;
}
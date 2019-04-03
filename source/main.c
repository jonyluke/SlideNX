#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <malloc.h>
#include <stdarg.h>
#include <unistd.h>
#include <switch.h>
#include "mp3.h"
#include <switch.h>

u32 __nx_applet_type = AppletType_None;

#define INNER_HEAP_SIZE 0x80000
size_t nx_inner_heap_size = INNER_HEAP_SIZE;
char   nx_inner_heap[INNER_HEAP_SIZE];

void __libnx_initheap(void)
{
	void*  addr = nx_inner_heap;
	size_t size = nx_inner_heap_size;

	// Newlib
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	fake_heap_start = (char*)addr;
	fake_heap_end   = (char*)addr + size;
}

void __attribute__((weak)) __appInit(void)
{
    Result rc;

    rc = smInitialize();
    if (R_FAILED(rc))
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    rc = fsInitialize();
    if (R_FAILED(rc))
        fatalSimple(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    fsdevMountSdmc();
}

void __attribute__((weak)) userAppExit(void);

void __attribute__((weak)) __appExit(void)
{
    fsdevUnmountAll();
    fsExit();
    smExit();
}

int main(int argc, char* argv[])
{
    mp3MutInit();
    gpioInitialize();
    GpioPadSession joycon_L_attach, joycon_R_attach;
    bool rfirst = true, rfirst2 = true;

    gpioOpenSession(&joycon_L_attach, (GpioPadName)0x0c);
    gpioOpenSession(&joycon_R_attach, (GpioPadName)0x34);

    gpioPadSetDirection(&joycon_L_attach, GpioDirection_Input);
    gpioPadSetDirection(&joycon_R_attach, GpioDirection_Input);

    GpioValue val1, val2, val3, val4;
    while (appletMainLoop())
    {
        svcSleepThread(1e+8L);

        if(rfirst) {
            rfirst = false;
            gpioPadGetValue(&joycon_L_attach, &val1);
        }

        gpioPadGetValue(&joycon_L_attach, &val2);

        if(val2 != val1) {
            val1 = val2;
            if(val2) {
                playMp3("SlideNX/detach.mp3");
            }

            else {
                playMp3("SlideNX/attach.mp3");
            }
        }

        if(rfirst2) {
            rfirst2 = false;
            gpioPadGetValue(&joycon_R_attach, &val3);
        }

        gpioPadGetValue(&joycon_R_attach, &val4);

        if(val4 != val3) {
            val3 = val4;
            if(val4) {
                playMp3("SlideNX/detach.mp3");
            }

            else {
                playMp3("SlideNX/attach.mp3");
            }
        }
    }
    return 0;
}
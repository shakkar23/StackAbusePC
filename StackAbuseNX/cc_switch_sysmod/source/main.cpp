#include "Shak2/Shak2.hpp"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <thread>
#include <unistd.h>

#define ONEINTERFACE
#define TITLE_ID 0x4206942069420690
[[maybe_unused]] std::thread PcWorker;
Service hidService;

extern "C" {
// Sysmodules should not use applet*.
u32 __nx_applet_type = AppletType_None;

// Adjust size as needed.#
#define INNER_HEAP_SIZE 0xa0000
#define HEAP_SIZE 0x000889000
size_t nx_inner_heap_size = HEAP_SIZE;
char fake_heap[HEAP_SIZE];

void __libnx_init_time(void);
void __libnx_initheap(void);
void __appInit(void);
void __appExit(void);
}

// we override libnx internals to do a minimal init
void __libnx_initheap(void) {
	extern char* fake_heap_start;
	extern char* fake_heap_end;

	// setup newlib fake heap
	fake_heap_start = fake_heap;
	fake_heap_end   = fake_heap + HEAP_SIZE;
}
void __appInit(void) {
	Result rc;
	rc = smInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = ldrDmntInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);
	else if(hosversionGet() == 0) {
		rc = setsysInitialize();
		if(R_FAILED(rc))
			fatalThrow(rc);
		else if(R_SUCCEEDED(rc)) {
			SetSysFirmwareVersion fw;
			rc = setsysGetFirmwareVersion(&fw);
			if(R_FAILED(rc))
				fatalThrow(rc);
			else if(R_SUCCEEDED(rc))
				hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
			setsysExit();
		}
	}
	rc = fsInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = fsdevMountSdmc();
	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = timeInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = pmdmntInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);
#ifdef ONEINTERFACE
	rc = usbCommsInitialize();
#else
	rc = usbCommsInitializeEx(2, NULL, ???, ???);
#endif
	usbCommsSetErrorHandling(true);

	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = viInitialize(ViServiceType_System);
	if(R_FAILED(rc))
		fatalThrow(rc);
	rc = hiddbgInitialize();
	if(R_FAILED(rc))
		fatalThrow(rc);
}

void __appExit(void) {
	fsdevUnmountAll();
	fsExit();
	smExit();
	audoutExit();
	timeExit();
	socketExit();
	pmdmntExit();
	ldrDmntExit();
	usbCommsExit();
	hiddbgExit();
}

//#define PCTALK
int main() {
	PcTalk2();
	return 0;
}

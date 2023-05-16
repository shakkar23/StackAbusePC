
#include "commands.hpp"
#include "util.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <atomic>
// Result rc;

Mutex actionLock;

// Controller:
bool bControllerIsInitialised = false;
HiddbgHdlsHandle controllerHandle = {0};
HiddbgHdlsDeviceInfo controllerDevice = {0};
HiddbgHdlsState controllerState = {0};

std::atomic_bool shouldKeepRunning = true;

u64 buttonClickSleepTime = 50;

void initController()
{
	if (bControllerIsInitialised)
		return;
	// taken from switchexamples github
	Result rc = hiddbgInitialize();
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	;
	// Set the controller type to Pro-Controller, and set the npadInterfaceType.
	controllerDevice.deviceType = HidDeviceType_FullKey3;
	// Make controller bluetooth
	controllerDevice.npadInterfaceType = HidNpadInterfaceType_Bluetooth;
	// Set the controller colors. The grip colors are for Pro-Controller on [9.0.0+].
	controllerDevice.singleColorBody = RGBA8_MAXALPHA(255, 255, 255);
	controllerDevice.singleColorButtons = RGBA8_MAXALPHA(0, 0, 0);
	controllerDevice.colorLeftGrip = RGBA8_MAXALPHA(230, 255, 0);
	controllerDevice.colorRightGrip = RGBA8_MAXALPHA(0, 40, 20);

	// Setup example controller state.
	// Setup example controller state.
	controllerState.battery_level = 4; // Set battery charge to full.
	controllerState.analog_stick_l = {0x0, 0x0};
	// controllerState.joysticks[JOYSTICK_LEFT].dy = -0x0;
	controllerState.analog_stick_r = {0x0, 0x0};
	//controllerState.joysticks[JOYSTICK_RIGHT].dy = -0x0;
	//rc = hiddbgAttachHdlsWorkBuffer();
	// if (R_FAILED(rc) && debugResultCodes)
	// 	fatalThrow(rc);
	// ;
	rc = hiddbgAttachHdlsVirtualDevice(&controllerHandle, &controllerDevice);
	if (R_FAILED(rc) && debugResultCodes)
		fatalThrow(rc);
	bControllerIsInitialised = true;
}

void click(HidNpadButton btn)
{
	initController();
	press(btn);
	svcSleepThread(buttonClickSleepTime * 1e+6L);
	release(btn);
}
void press(HidNpadButton btn)
{
	initController();
	controllerState.buttons |= btn;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}


void release(HidNpadButton btn)
{
	initController();
	controllerState.buttons &= ~btn;
	hiddbgSetHdlsState(controllerHandle, &controllerState);
}

void setStickState(int side, int dxVal, int dyVal)
{
	initController();
	switch (side)
	{
	case JOYSTICK_MIN:
		controllerState.analog_stick_l = {dxVal, dyVal};
		hiddbgSetHdlsState(controllerHandle, &controllerState);
	case JOYSTICK_MAX:
		controllerState.analog_stick_r = {dxVal, dyVal};
		//controllerState.joysticks[side].dy = dyVal;
		hiddbgSetHdlsState(controllerHandle, &controllerState);
	}
}
u64 getPID(){
	Result rc{};
	u64 pids[300]{};
	s32 numProc{};
	rc = svcGetProcessList(&numProc, pids, (sizeof(pids) / sizeof(pids[0])));
	if (R_FAILED(rc))
		fatalThrow(0x123);
	u64 pid = pids[numProc - 1];
	LoaderModuleInfo proc_modules[2]{}; // proc_modules
	s32 numModules = 0;			 // numModules

	pmdmntGetApplicationProcessId(&pid);
	return pid;
}
u64 getMainNsoBase()
{
	Result rc{};
	u64 pids[300]{};
	s32 numProc{};
	rc = svcGetProcessList(&numProc, pids, (sizeof(pids) / sizeof(pids[0])));
	if (R_FAILED(rc))
		fatalThrow(0x123);
	u64 pid = pids[numProc - 1];
	LoaderModuleInfo proc_modules[2]{}; // proc_modules
	s32 numModules = 0;			 // numModules

	pmdmntGetApplicationProcessId(&pid);
	rc = ldrDmntGetProcessModuleInfo(pid, proc_modules, 2, &numModules);
	if (R_FAILED(rc))
	{
		fatalThrow(0x01200D);
	}
	// ldrDmntExit();
	LoaderModuleInfo *mainPaste2 = nullptr;
	if (numModules == 2)
	{
		mainPaste2 = &proc_modules[1];
	}
	else
	{
		mainPaste2 = &proc_modules[0];
	}
	return mainPaste2->base_address;
}

void BuildID(u8 joe[0x20])
{
	Result rc;
	u64 pids[300];
	s32 numProc;
	rc = svcGetProcessList(&numProc, pids, 300);
	if (R_FAILED(rc))
		fatalThrow(0x123);
	u64 pid = pids[numProc - 1];
	LoaderModuleInfo mainpls[2]; // proc_modules
	s32 mainPaste = 0;			 // numModules

	pmdmntGetApplicationProcessId(&pid);
	rc = ldrDmntGetProcessModuleInfo(pid, mainpls, 2, &mainPaste);
	if (R_FAILED(rc))
	{
		fatalThrow(0x01200D);
	}
	// ldrDmntExit();
	LoaderModuleInfo *mainPaste2 = 0;
	if (mainPaste == 2)
	{
		mainPaste2 = &mainpls[1];
	}
	else
	{
		mainPaste2 = &mainpls[0];
	}
	for (int i = 0; i < 0x20; i++)
		joe[i] = mainPaste2->build_id[i];
}

bool issame(u8 one[0x20], u8 two[0x20])
{
	for (int i = 0; i < 0x20; i++)
	{
		if (one[i] == two[i])
		{
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool sysmodMainLoop(){
	return shouldKeepRunning;
}

void DestroySysmod(){
	shouldKeepRunning = false;
	return;
}

/**
 *
 * @file psp.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "psp.h"

#ifdef PSP

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman.h>
#include <pspmoduleinfo.h>
#include <pspdebug.h>
#include <psputility.h>
#include <pspdisplay.h>
#include <stdlib.h>

PSP_MODULE_INFO("OpenJazz", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);
PSP_HEAP_SIZE_KB(-2048);

static int exitRequest = 0;

static int exitCallback(int arg1, int arg2, void *common) {
	exitRequest = 1;
	return 0;
}

static int callbackThread(SceSize args, void *argp) {
	int callbackID;

	callbackID = sceKernelCreateCallback("Exit Callback", exitCallback, NULL);
	sceKernelRegisterExitCallback(callbackID);

	sceKernelSleepThreadCB();

	return 0;
}

int PSP_WantsExit() {
	return exitRequest;
}

void PSP_PrepareSystem() {
	pspDebugScreenInit();
	//pspDebugInstallErrorHandler(NULL);

	int threadID = sceKernelCreateThread("Callback Update Thread", callbackThread, 0x11, 0xFA0, THREAD_ATTR_USER, 0);
	if(threadID >= 0) {
		sceKernelStartThread(threadID, 0, 0);
	}

	atexit(sceKernelExitGame);

	sceIoChdir("ms0:/PSP/GAME/OpenJazz");
}

void PSP_ErrorNoDatafiles() {
	pspDebugScreenClear();
	pspDebugScreenSetXY(12, 0);
	pspDebugScreenPuts("Unable to find game data files.");
	pspDebugScreenPuts("Please put them in the same directory as this executable.");
	pspDebugScreenPuts("\te.g. ms0:/PSP/GAME/OpenJazz");
	sceKernelDelayThread(3000000);
}

#endif

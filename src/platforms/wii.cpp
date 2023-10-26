
/**
 *
 * @file wii.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "wii.h"

#ifdef __wii__

#include <gccore.h>
#include <ogc/usbgecko.h>
#include <sys/iosupport.h>
#include <unistd.h>
#include <fat.h>

#include "util.h"
#include "io/file.h"

/* USBGecko Debugging */

#define SLOT 1 // A: 0, B: 1
bool usbgecko = false;
mutex_t usbgecko_mutex = 0;

static ssize_t __usbgecko_write(struct _reent * /* r */, void* /* fd */, const char *ptr, size_t len) {
	uint32_t level;

	if (!ptr || !len || !usbgecko)
		return 0;

	LWP_MutexLock(usbgecko_mutex);
	level = IRQ_Disable();
	usb_sendbuffer(SLOT, ptr, len);
	IRQ_Restore(level);
	LWP_MutexUnlock(usbgecko_mutex);

	return len;
}

extern const devoptab_t dotab_stdnull;
const devoptab_t dotab_geckoout = {
	"geckoout", 0, NULL, NULL, __usbgecko_write, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
	NULL, NULL, NULL
};

void WII_Init() {
	// enable file system
	fatInitDefault();

	// set console
	LWP_MutexInit(&usbgecko_mutex, false);
	usbgecko = usb_isgeckoalive(SLOT);

	if (usbgecko) {
		usb_flush(SLOT);

		devoptab_list[STD_OUT] = &dotab_geckoout;
		devoptab_list[STD_ERR] = &dotab_geckoout;
	} else {
		devoptab_list[STD_OUT] = &dotab_stdnull;
		devoptab_list[STD_ERR] = &dotab_stdnull;
	}
}

void WII_AddGamePaths() {
	gamePaths.add(createString("sd:/apps/OpenJazz/"), PATH_TYPE_GAME|PATH_TYPE_CONFIG);
}

#endif

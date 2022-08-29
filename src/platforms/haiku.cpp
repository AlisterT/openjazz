
/**
 *
 * @file haiku.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "haiku.h"

#ifdef __HAIKU__

#include <Alert.h>
#include <SupportDefs.h>
#include <FindDirectory.h>
#include <fs_info.h>

#include "util.h"
#include "io/file.h"

static char dir_buffer[10 + B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH];

void HAIKU_AddGamePaths() {
	dev_t volume = dev_for_path("/boot");

	find_directory(B_SYSTEM_DATA_DIRECTORY,
		volume, false, dir_buffer, sizeof(dir_buffer));
	strncat(dir_buffer, "/openjazz/", sizeof(dir_buffer) - 1);
	firstPath = new Path(firstPath, createString(dir_buffer));

	find_directory(B_USER_NONPACKAGED_DATA_DIRECTORY,
		volume, false, dir_buffer, sizeof(dir_buffer));
	strncat(dir_buffer, "/openjazz/", sizeof(dir_buffer) - 1);
	firstPath = new Path(firstPath, createString(dir_buffer));
}

void HAIKU_ErrorNoDatafiles() {
	char alertBuffer[100 + B_PATH_NAME_LENGTH + B_FILE_NAME_LENGTH];
	strcpy(alertBuffer, "Unable to find game data files!\n"
		"Put the data into the folder:\n");
	strncat(alertBuffer, dir_buffer, sizeof(alertBuffer));

	BAlert* alert = new BAlert("OpenJazz", alertBuffer, "Exit", NULL, NULL,
		B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();
}

#endif

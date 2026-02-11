
/**
 *
 * @file switch.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */

#include "switch.h"

#ifdef __SWITCH__

#include <switch.h>
#include <malloc.h>
#include "util.h"
#include "io/file.h"
#include "io/log.h"
#include "version.h"

namespace {
	int sock = -1;

	SwkbdTextCheckResult InputString_Filter(char *text, size_t textlen) {
		for (size_t i = 0; i < textlen; i++) {
			// we do not care about the unused buffer end
			if (text[i] == '\0' || i > STRING_LENGTH) {
				return SwkbdTextCheckResult_OK;
			}
			if ((text[i] != ' ') && (text[i] != '.') &&
				((text[i] < '0') || (text[i] > '9')) &&
				((text[i] < 'a') || (text[i] > 'z'))) {

				strncpy(text, "Invalid character(s) found.\nOnly a-z, 0-9, Dot and Space are allowed.", textlen - 1);
				text[textlen] = '\0';
				return SwkbdTextCheckResult_Bad;
			}
		}
		return SwkbdTextCheckResult_OK;
	}

}

SwitchPlatform::SwitchPlatform() {
	// file system
	romfsInit();

#ifndef NDEBUG
	// network console
	NetInit();
	sock = nxlinkStdio();
#endif
}

SwitchPlatform::~SwitchPlatform() {
	romfsExit();

#ifndef NDEBUG
	NetExit();
#endif
}

void SwitchPlatform::NetInit() {
	int ret = socketInitializeDefault();
	if (ret != 0) {
		LOG_WARN("Failed to initialize sockets 0x%08X\n", (unsigned int)ret);
	}
}

void SwitchPlatform::NetExit() {
	if (sock >= 0) {
		close(sock);
		sock = -1;
	}

	socketExit();
}

bool SwitchPlatform::NetHasConsole() {
	return sock >= 0;
}

void SwitchPlatform::AddGamePaths() {
	gamePaths.add(createString("sdmc:/switch/OpenJazz/"), PATH_TYPE_GAME|PATH_TYPE_CONFIG);
	gamePaths.add(createString("romfs:/"), PATH_TYPE_SYSTEM|PATH_TYPE_GAME);
}

void SwitchPlatform::ErrorNoDatafiles() {
	// never generate an error report without CFW
	if (!hosversionIsAtmosphere()) return;

	// only applications should use the error applet
	AppletType at = appletGetAppletType();
	if (at != AppletType_Application && at != AppletType_SystemApplication) return;

	// make error
	const char *errorMsg = "Unable to find game data files.\n\n"
		"Game files can be shipped inside RomFS and are searched under sdmc:/switch/OpenJazz/";
	size_t msgLen = strlen(errorMsg) + 60; // enough room for version string
	char *error = new char[msgLen];
	snprintf(error, msgLen, "OpenJazz %s:\n\n%s", oj_version, errorMsg);
	ErrorApplicationConfig eac;
	if (R_SUCCEEDED(errorApplicationCreate(&eac, error, nullptr))) {
		errorApplicationShow(&eac);
	}
	delete[] error;
}

bool SwitchPlatform::InputIP(char*& current_ip, char*& new_ip) {
	SwkbdConfig kbd;
	Result rc = swkbdCreate(&kbd, 0);
	if (R_SUCCEEDED(rc)) {
		swkbdConfigSetType(&kbd, SwkbdType_NumPad);
		swkbdConfigSetRightOptionalSymbolKey(&kbd, ".");
		swkbdConfigSetHeaderText(&kbd, "OpenJazz");
		swkbdConfigSetSubText(&kbd, "Please enter an IP address");
		swkbdConfigSetInitialText(&kbd, current_ip);
		char ip_buf[16] = {0};
		swkbdConfigSetInitialCursorPos(&kbd, 1); // at end
		swkbdConfigSetStringLenMax(&kbd, sizeof(ip_buf) - 1);
		swkbdConfigSetStringLenMin(&kbd, 7);
		rc = swkbdShow(&kbd, ip_buf, sizeof(ip_buf));
		swkbdClose(&kbd);
		if (R_SUCCEEDED(rc)) {
			new_ip = createString(ip_buf);
			return true;
		}
	}
	return false;
}

bool SwitchPlatform::InputString(const char* hint, char*& current_string, char*& new_string) {
	SwkbdConfig kbd;
	Result rc = swkbdCreate(&kbd, 0);
	if (R_SUCCEEDED(rc)) {
		swkbdConfigSetType(&kbd, SwkbdType_QWERTY);
		swkbdConfigSetHeaderText(&kbd, "OpenJazz");
		swkbdConfigSetSubText(&kbd, hint);
		swkbdConfigSetInitialText(&kbd, current_string);
		char string_buf[80] = {0}; // needs to be big enough for the StringFilter message
		swkbdConfigSetInitialCursorPos(&kbd, 1); // at end
		swkbdConfigSetStringLenMax(&kbd, STRING_LENGTH);
		swkbdConfigSetStringLenMin(&kbd, 1);
		swkbdConfigSetTextCheckCallback(&kbd, InputString_Filter);
		swkbdConfigSetKeySetDisableBitmask(&kbd, SwkbdKeyDisableBitmask_UserName);
		rc = swkbdShow(&kbd, string_buf, sizeof(string_buf));
		swkbdClose(&kbd);
		if (R_SUCCEEDED(rc)) {
			new_string = createString(string_buf);
			return true;
		}
	}
	return false;
}

#endif

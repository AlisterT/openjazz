
/**
 *
 * @file file_dir.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with files in directories.
 *
 */

#include "file_dir.h"
#include "util.h"
#include "io/log.h"

#include <string.h>
#include <unistd.h>

#if !(defined(_WIN32) || defined(WII) || defined(PSP) || defined(__3DS__))
    #define UPPERCASE_FILENAMES
    #define LOWERCASE_FILENAMES
#endif

/**
 * Try opening a file from the given path
 *
 * @param path Directory path
 * @param name File name
 * @param write Whether or not the file can be written to
 */
DirFile::DirFile (const char* path, const char* name, bool write) {
#if defined(UPPERCASE_FILENAMES) || defined(LOWERCASE_FILENAMES)
	int count;
#endif

	// Create the file path for the given directory
	filePath = createString(path, name);

	// Open the file from the path
	file = fopen(filePath, write ? "wb": "rb");

#ifdef UPPERCASE_FILENAMES
    if (!file) {
        // Convert the file name to upper case
        for (count = strlen(path); filePath[count]; count++) {

            if ((filePath[count] >= 97) && (filePath[count] <= 122)) filePath[count] -= 32;

        }

        // Open the file from the path
        file = fopen(filePath, write ? "wb": "rb");
    }
#endif

#ifdef LOWERCASE_FILENAMES
    if (!file) {
        // Convert the file name to lower case
        for (count = strlen(path); filePath[count]; count++) {

            if ((filePath[count] >= 65) && (filePath[count] <= 90)) filePath[count] += 32;

        }

        // Open the file from the path
        file = fopen(filePath, write ? "wb": "rb");
    }
#endif

	if (file) {
        LOG_TRACE("Opened file: %s", filePath);

		return;
	}

	LOG_TRACE("Could not open file: %s", filePath);
	delete[] filePath;

	throw E_FILE;
}

DirFile::~DirFile () {
	fclose(file);

	LOG_TRACE("Closed file: %s", filePath);
	delete[] filePath;
}

size_t DirFile::getSize () {
	size_t pos = tell();
	fseek(file, 0, SEEK_END);
	size_t size = tell();
	seek(pos, true);

	return size;
}

size_t DirFile::tell () {
	return ftell(file);
}

void DirFile::seek (size_t offset, bool reset) {
	fseek(file, offset, reset ? SEEK_SET: SEEK_CUR);
}

unsigned char DirFile::loadChar () {
	return fgetc(file);
}

void DirFile::storeChar (unsigned char val) {
	fputc(val, file);
}

size_t DirFile::readBlock (void* buffer, size_t length) {
	return fread(buffer, 1, length, file);
}

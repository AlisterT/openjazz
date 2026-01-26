
/**
 *
 * @file util.cpp
 *
 * Part of the OpenJazz project
 *
 * @par History:
 * - 23rd August 2005: Created main.c
 * - 22nd July 2008: Created util.c from parts of main.c
 * - 3rd February 2009: Renamed util.c to util.cpp
 * - 3rd February 2009: Created file.cpp from parts of util.cpp
 * - 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * - 13th July 2009: Created graphics.cpp from parts of util.cpp
 *
 * @par Licence:
 * Copyright (c) 2005-2012 AJ Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Contains core utility functions.
 *
 */


#include "util.h"
#include "io/file.h"
#include "io/log.h"

#include <string.h>
#include <cstdio>
#include <cctype>

/**
 * Check if a file exists.
 *
 * @param fileName The file to check
 * @param pathType Kind of directory
 *
 * @return Whether or not the file exists
 */
bool fileExists (const char * fileName, int pathType) {

	File *file;

#ifdef VERBOSE
	printf("Check: ");
#endif

	try {

		file = new File(fileName, pathType);

	} catch (int e) {

		return false;

	}

	delete file;

	return true;

}


/**
 * Create a short based on the little-endian contents of the first two bytes in
 * the given memory location.
 *
 * @param data Pointer to the memory location
 *
 * @return The generated short
 */
unsigned short int createShort (const unsigned char* data) {

	unsigned short int val;

	val = data[0] + (data[1] << 8);

	return val;

}


/**
 * Create an int based on the little-endian contents of the first two bytes in
 * the given memory location.
 *
 * @param data Pointer to the memory location
 *
 * @return The generated int
 */
int createInt (const unsigned char* data) {

	unsigned int val;

	val = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);

	return *((int *)&val);

}


/**
 * Create a new string from the contents of an existing string.
 *
 * @param string The existing string
 *
 * @return The new string
 */
char * createString (const char *string) {

	char *cloned;

	cloned = new char[strlen(string) + 1];
	strcpy(cloned, string);

	return cloned;

}


/**
 * Create a new string from the concatenation of two existing strings.
 *
 * @param first The existing string to form the start of the new string
 * @param second The exisitng string to form the end of the new string
 *
 * @return The new string
 */
char * createString (const char *first, const char *second) {

	char *concatenated;

	concatenated = new char[strlen(first) + strlen(second) + 1];
	strcpy(concatenated, first);
	strcat(concatenated, second);

	return concatenated;

}


/**
 * Create a new file name string with a 3-digit numerical extension.
 *
 * @param type The pre-dot file name
 * @param extension The number to constitute the extension
 *
 * @return The new file name string
 */
char * createFileName (const char *type, int extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[pos + 5];
	strcpy(fileName, type);
	fileName[pos++] = '.';
	fileName[pos++] = '0' + ((extension / 100) % 10);
	fileName[pos++] = '0' + ((extension / 10) % 10);
	fileName[pos++] = '0' + (extension % 10);
	fileName[pos] = 0;

	return fileName;

}


/**
 * Create a new file name string with the given extension.
 *
 * @param type The pre-dot file name
 * @param extension The extension
 *
 * @return The new file name string
 */
char * createFileName (const char *type, const char *extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[pos + strlen(extension) + 2];
	strcpy(fileName, type);
	fileName[pos++] = '.';
	strcpy(fileName + pos, extension);

	return fileName;

}


/**
 * Create a new file name string with a 1-digit numerical suffix and a 3-digit
 * numerical extension.
 *
 * @param type The pre-dot file name
 * @param level The number to constitute the suffix
 * @param extension The number to constitute the extension
 *
 * @return The new file name string
 */
char * createFileName (const char *type, int level, int extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[pos + 6];
	strcpy(fileName, type);
	fileName[pos++] = '0' + (level % 10);
	fileName[pos++] = '.';
	fileName[pos++] = '0' + ((extension / 100) % 10);
	fileName[pos++] = '0' + ((extension / 10) % 10);
	fileName[pos++] = '0' + (extension % 10);
	fileName[pos] = 0;

	return fileName;

}


/**
 * Create a new variable-length string from the contents of an existing string.
 *
 * @param string The existing string
 *
 * @return The new string
 */
char * createEditableString (const char *string) {

	char *cloned;

	cloned = new char[STRING_LENGTH + 1];
	strcpy(cloned, string);

	return cloned;

}

void lowercaseString (char *string) {
	for (int i = 0; string[i]; i++) {
		if ((string[i] >= 65) && (string[i] <= 90))
			string[i] += 32;
	}
}

void uppercaseString (char *string) {
	for (int i = 0; string[i]; i++) {
		if ((string[i] >= 97) && (string[i] <= 122))
			string[i] -= 32;
	}
}

void camelcaseString (char *string) {
	bool isFirst = true;

	for (int i = 0; string[i]; i++) {
		if(isFirst) {
			if ((string[i] >= 97) && (string[i] <= 122))
				string[i] -= 32;
			isFirst = false;
		} else {
			if ((string[i] >= 65) && (string[i] <= 90))
				string[i] += 32;
		}
		if(string[i] == 32)
			isFirst = true;
	}
}


/**
 * Get the sine of the given angle
 *
 * @param angle The given angle (where 1024 represents a full circle)
 *
 * @return The sine of the angle
 */
fixed fSin (fixed angle) {

	return sinLut[angle & 1023];

}


/**
 * Get the cosine of the given angle
 *
 * @param angle The given angle (where 1024 represetns a full circle)
 *
 * @return The cosine of the angle
 */
fixed fCos (fixed angle) {

	return sinLut[(angle + 256) & 1023];

}


/**
 * Unpack a block of RLE compressed data.
 *
 * @param data Buffer containing compressed data
 * @param size The length of the compressed block
 * @param outSize The length of the uncompressed block
 *
 * @return Buffer containing the uncompressed data
 */
unsigned char* unpackRLE (unsigned char* data, unsigned int size, unsigned int outSize) {
	unsigned char* buffer = new unsigned char[outSize];

	unsigned int posIn = 0, posOut = 0;
	while (posIn < size) {
		unsigned char code = data[posIn];
		unsigned char amount = code & 127;

		if (code & 128) { // repeat
			unsigned char value = data[posIn+1];

			if (posOut + amount >= outSize) {
				LOG_WARN("Exceeding write buffer while RLE unpacking.");
				break;
			}

			memset(buffer + posOut, value, amount);

			posIn += 2;
			posOut += amount;
		} else if (amount) { // copy

			if (posOut + amount >= outSize) {
				LOG_WARN("Exceeding write buffer while RLE unpacking.");
				break;
			}

			memcpy(buffer + posOut, data + posIn + 1, amount);

			posIn += amount + 1;
			posOut += amount;
		} else { // end marker
			buffer[posOut++] = data[posIn+1];
			posIn += 2;

			//LOG_MAX("End marker found while RLE unpacking.");
			break;
		}
	}

	if(size != posIn || outSize != posOut) {
		LOG_DEBUG("RLE block has incorrect size: in %d/%d out %d/%d", size, posIn, outSize, posOut);
	}

	delete[] data;
	return buffer;
}


int episodeToWorld (int episode) {
	if(episode < 0 || episode > 9) {
		LOG_WARN("Invalid Episode");

		return 0;
	}

	// 1-6
	if (episode < 6)
		return episode * 3;

	// A-C
	if (episode < 9)
		return (episode + 4) * 3;

	// X
	return 50;
}


int worldToEpisode (int world) {
	if(world < 0 || world > 50) {
		LOG_WARN("Invalid World");

		return 0;
	}

	// X
	if (world == 50)
		return 9;

	int episode = world / 3;

	// A-C
	if (episode > 8)
		return episode - 4;

	// 1-6
	return episode;
}

/**
 * Make a hex dump of data
 *
 * @param desc if non-NULL, printed as a description before hex dump
 * @param addr Address to start dumping from
 * @param size Number of bytes to dump
 * @param perLine Number of bytes on each output line
 */
void hexDump (const char * desc, const void * addr, const int size, int perLine) {
	// Length check
	if (!(size > 0)) {
		LOG_WARN("Cannot hexdump");
		return;
	}

	if (desc != nullptr)
		printf("\x1b[34m%s:\x1b[0m\n", desc);

	unsigned char buffer[perLine + 1];
	const unsigned char * pc = (const unsigned char *)addr;

	// Process every byte in the data.
	int i;
	for (i = 0; i < size; i++) {
		// Multiple of perLine means new or first line (with line offset).
		if ((i % perLine) == 0) {
			// Only print previous-line ASCII buffer for lines beyond first.
			if (i != 0)
				printf("  \x1b[32m%s\n", buffer);

			// Output the offset of current line.
			printf("\x1b[90m  %04x \x1b[35m", i);
		}

		// Now the hex code for the specific character.
		printf(" %02x", pc[i]);

		// And buffer a printable ASCII character for later.
		buffer[i % perLine] = isprint(pc[i]) ? pc[i] : '.';
		buffer[(i % perLine) + 1] = '\0';
	}

	// Pad out last line if not exactly perLine characters.
	while ((i % perLine) != 0) {
		printf("   ");
		i++;
	}

	// And print the final ASCII buffer.
	printf("  \x1b[32m%s\x1b[0m\n", buffer);
}

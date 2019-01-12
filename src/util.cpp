
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
 * Copyright (c) 2005-2012 Alister Thomson
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

#include <string.h>


/**
 * Check if a file exists.
 *
 * @param fileName The file to check
 *
 * @return Whether or not the file exists
 */
bool fileExists (const char * fileName) {

	File *file;

#ifdef VERBOSE
	printf("Check: ");
#endif

	try {

		file = new File(fileName, false);

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
unsigned short int createShort (unsigned char* data) {

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
int createInt (unsigned char* data) {

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


/**
 * Add a message to the log.
 *
 * @param message The log message
 */
void log (const char *message) {

	printf("%s\n", message);

	return;

}


/**
 * Add a message with a detail message to the log.
 *
 * @param message The log message
 * @param detail The detail message
 */
void log (const char *message, const char *detail) {

	printf("%s: %s\n", message, detail);

	return;

}


/**
 * Add a message with a detail number to the log.
 *
 * @param message The log message
 * @param number The detail number
 */
void log (const char *message, int number) {

	printf("%s: %d\n", message, number);

	return;

}


/**
 * Add a message with a detail message to the error log.
 *
 * @param message The log message
 * @param detail The detail message
 */
void logError (const char *message, const char *detail) {

	fprintf(stderr, "%s: %s\n", message, detail);

	return;

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


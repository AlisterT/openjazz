
/*
 *
 * util.cpp
 *
 * 22nd July 2008: Created util.c from parts of main.c
 * 3rd February 2009: Renamed util.c to util.cpp
 * 3rd February 2009: Created file.cpp from parts of util.cpp
 * 4th February 2009: Created palette.cpp from parts of main.cpp and util.cpp
 * 13th July 2009: Created graphics.cpp from parts of util.cpp
 *
 * Part of the OpenJazz project
 *
 *
 * Copyright (c) 2005-2010 Alister Thomson
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * Contains core utility functions.
 *
 */


#include "io/file.h"

#include <string.h>


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


char * createString (const char *string) {

	char *cloned;

	cloned = new char[strlen(string) + 1];
	strcpy(cloned, string);

	return cloned;

}


char * createString (const char *first, const char *second) {

	char *concatenated;

	concatenated = new char[strlen(first) + strlen(second) + 1];
	strcpy(concatenated, first);
	strcat(concatenated, second);

	return concatenated;

}


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


char * createFileName (const char *type, const char *extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[strlen(type) + strlen(extension) + 2];
	strcpy(fileName, type);
	fileName[pos++] = '.';
	strcpy(fileName + pos, extension);

	return fileName;

}


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


char * createEditableString (const char *string) {

	char *cloned;

	cloned = new char[STRING_LENGTH + 1];
	strcpy(cloned, string);

	return cloned;

}


void log (const char *message) {

	printf("%s\n", message);

	return;

}


void log (const char *message, const char *detail) {

	printf("%s: %s\n", message, detail);

	return;

}


void log (const char *message, int number) {

	printf("%s: %d\n", message, number);

	return;

}


void logError (const char *message, const char *detail) {

	fprintf(stderr, "%s: %s\n", message, detail);

	return;

}


fixed fSin (fixed angle) {

	return sinLut[angle & 1023];

}


fixed fCos (fixed angle) {

	return sinLut[(angle + 256) & 1023];

}


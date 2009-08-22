
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
 * Copyright (c) 2005-2009 Alister Thomson
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


bool fileExists (char * fileName) {

	File *file;

	//printf("Check: ");

	try {

		file = new File(fileName, false);

	} catch (int e) {

		return false;

	}

	delete file;

	return true;

}


char * createString (char *string) {

	char *cloned;

	cloned = new char[strlen(string) + 1];
	strcpy(cloned, string);

	return cloned;

}


char * createString (char *first, char *second) {

	char *concatenated;

	concatenated = new char[strlen(first) + strlen(second) + 1];
	strcpy(concatenated, first);
	strcat(concatenated, second);

	return concatenated;

}


char * createFileName (char *type, int extension) {

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


char * createFileName (char *type, char *extension) {

	char *fileName;
	int pos;

	pos = strlen(type);
	fileName = new char[strlen(type) + strlen(extension) + 2];
	strcpy(fileName, type);
	fileName[pos++] = '.';
	strcpy(fileName + pos, extension);

	return fileName;

}


char * createFileName (char *type, int level, int extension) {

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


char * createEditableString (char *string) {

	char *cloned;

	cloned = new char[STRING_LENGTH + 1];
	strcpy(cloned, string);

	return cloned;

}


void log (char *message) {

	printf("%s\n", message);

	return;

}


void log (char *message, char *detail) {

	printf("%s: %s\n", message, detail);

	return;

}


void log (char *message, int number) {

	printf("%s: %d\n", message, number);

	return;

}


void logError (char *message, char *detail) {

	fprintf(stderr, "%s: %s\n", message, detail);

	return;

}



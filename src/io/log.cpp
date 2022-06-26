
/**
 *
 * @file log.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 * Copyright (c) 2015-2022 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Logs to console and file.
 *
 */

#include "log.h"
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#ifdef __vita__
#include <psp2/kernel/clib.h>
#endif

static struct level_info {
	const char *name;
	const char *color;
} levels[7] = {
	{ "MAX",   "\x1b[95m" },
	{ "TRACE", "\x1b[94m" },
	{ "DEBUG", "\x1b[36m" },
	{ "INFO",  "\x1b[32m" },
	{ "WARN",  "\x1b[33m" },
	{ "ERROR", "\x1b[31m" },
	{ "FATAL", "\x1b[35m" }
};

/**
 * Create logfile, set defaults
 */
Log::Log () {

	level = LL_DEBUG;
	quiet = false;
	logfile = fopen("openjazz.log", "w");
	color_stdout = false;
	color_stderr = false;

// these systems may use colored terminal output
#if defined(__linux__) || defined(__APPLE__)
	// allow users to disable all colors (see https://no-color.org/)
	if (!getenv("NO_COLOR")) {
		color_stdout = !!isatty(fileno(stdout));
		color_stderr = !!isatty(fileno(stderr));
	}
#endif

#ifdef NDEBUG
	// be more quiet in release builds
	level = LL_INFO;
	quiet = true;
#endif

}

/**
 * Close logfile
 */
Log::~Log () {

	if (logfile) fclose(logfile);

}

/**
 * Set new loglevel.
 *
 * @param new_level The loglevel
 */

void Log::setLevel(int new_level) {

	if (new_level > LL_FATAL)
		level = LL_FATAL;
	else if (new_level < LL_MAX)
		level = LL_MAX;
	else
		level = new_level;

}

/**
 * Get the loglevel.
 *
 * @return The loglevel
 */

int Log::getLevel() {

	return level;

}

/**
 * Enable/Disable quiet logging.
 *
 * @param enable Whether to enable quiet logging
 */

void Log::setQuiet(bool enable) {

	quiet = enable;

}

/**
 * Add a message to the log.
 *
 * @param lvl Verbosity level
 * @param file Source file
 * @param line Source line
 * @param fmt printf formatted message
 */

void Log::log(int lvl, const char *file, int line, const char *fmt, ...) {

	// skip if nothing to write
	if (lvl < level || (quiet && !logfile))	return;

	// extract file name (like basename)
	const char *src = strrchr(file, '\\');
	if (!src) src = strrchr(file, '/');
	if (!src)
		src = file;
	else
		src++;

	// get current time
	time_t t = time(NULL);
	struct tm *now = localtime(&t);

	// log to console
	if (!quiet) {
		// choose stderr/stdout depending on loglevel, use color if available
		FILE *stream = stdout;
		bool color = color_stdout;
		if (lvl < LL_INFO) {
			stream = stderr;
			color = color_stderr;
		}
#ifdef __vita__
		// on vita we can only read stdout easily (with psp2shell)
		#define LOG(...) sceClibPrintf(__VA_ARGS__)
#else
		#define LOG(...) fprintf(stream, __VA_ARGS__)
#endif
		char timebuf[9];
		strftime(timebuf, 9, "%H:%M:%S", now);

		if (color)
			LOG("%s %s%-5s\x1b[0m ", timebuf, levels[lvl].color, levels[lvl].name);
		else
			LOG("%s %-5s ", timebuf, levels[lvl].name);

		va_list args;
		va_start(args, fmt);
#ifdef __vita__
		// use an immediate buffer for output
		char outbuffer[1024];
		vsprintf(outbuffer, fmt, args);
		LOG(outbuffer);
#else
		vfprintf(stream, fmt, args);
#endif
		va_end(args);

		// only include source information if doing debug logs
		if (level >= LL_INFO)
			LOG("\n");
		else if (color)
			LOG(" \x1b[90m(%s:%d)\x1b[0m\n", src, line);
		else
			LOG(" (%s:%d)\n", src, line);

		fflush(stream);
	}

	// Log to file
	if (logfile) {
		char timebuf[20];
		strftime(timebuf, 20, "%Y-%m-%d %H:%M:%S", now);

		fprintf(logfile, "%s %-5s %s:%d: ", timebuf, levels[lvl].name, src, line);

		va_list args;
		va_start(args, fmt);
		vfprintf(logfile, fmt, args);
		va_end(args);

		fprintf(logfile, "\n");

		fflush(logfile);
	}

}

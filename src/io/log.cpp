
/**
 *
 * @file log.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Logs to console and file.
 *
 */

#include "log.h"
#include "file.h"
#include "util.h"
#include <cstdarg>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <unistd.h>

#ifdef __vita__
#include <psp2/kernel/clib.h>
#endif

#ifdef __ANDROID__
#include <android/log.h>
#endif

namespace {
	struct level_info {
		const char *name;
		const char *termColor;
		const char *htmlColor;
	} levels[7] = {
		{ "MAX",   "\x1b[95m", "#4d0027" },
		{ "TRACE", "\x1b[94m", "#003973" },
		{ "DEBUG", "\x1b[36m", "#222222" },
		{ "INFO",  "\x1b[32m", "#008000" },
		{ "WARN",  "\x1b[33m", "#ff6a00" },
		{ "ERROR", "\x1b[31m", "#ff0000" },
		{ "FATAL", "\x1b[35m", "#800080" }
	};

	// message queueing
	constexpr int MAX_BUFFERED_MESSAGES = 25;
	constexpr int MAX_MESSAGE_LENGTH = 2 * 1024;
	char *message_buffer[MAX_BUFFERED_MESSAGES] = { 0 }; // 50kb
	int message_buffer_index = 0;

	// html logfile
	void writeHtmlHead(FILE *handle) {
		// head
		fprintf(handle, "%s\n", "<!DOCTYPE html><html><head>");

		// css
		fprintf(handle, "%s",
			"<style> "
			"body { background-color: #fff; color: #555; font-family: Arial, sans-serif; } "
			"h1 { font-weight: bolder; } "
			"h1 span:nth-child(1) { color: #f00; } h1 span:nth-child(2) { color: #00f; } "
			"table { width: 100%; border-collapse: collapse; } "
			"th, td { padding: 6px; text-align: left; border-bottom: 1px solid #777; } "
			"td:nth-child(1), td:nth-child(2) { width: 0px; white-space: nowrap; } "
			"td:nth-child(3) { max-width: 0px; word-wrap: break-word; } ");
		for (int i = 0; i < 7; i++) {
			fprintf(handle, ".l%d { color: %s; } ", i, levels[i].htmlColor);
		}
		fprintf(handle, "</style>\n");

		// js
		fprintf(handle, "%s\n",
			"<script> "
			"document.addEventListener('DOMContentLoaded', function(e) {"
			"lvl.addEventListener('change', function() {"
			"const table = document.getElementsByTagName('table')[0];"
			"for (var r = 1; r < table.rows.length; r++) {"
			"if (parseInt(table.rows[r].className.match(/\\d/)) >= parseInt(this.value)) "
			"{ table.rows[r].style.display = 'table-row' }"
			"else { table.rows[r].style.display = 'none' }}});"
			"lvl.dispatchEvent(new Event('change'));"
			"}); "
			"</script>");

		// body
		fprintf(handle, "%s",
			"</head>\n<body><h1><span>Open</span><span>Jazz</span> Log</h1>"
			"Show level: <select id=\"lvl\"><option value=\"-1\">All</option>");
		for (int i = 6; i >= 0; i--) {
			fprintf(handle, "<option value=\"%d\"%s>%s</option>", i, (i == 2 ? " selected" : ""), levels[i].name);
		}
		fprintf(handle, "%s\n",
			"</select><br><br>\n"
			"<table>\n<tr><th>Timestamp</th><th>Level</th><th>Message</th></tr>");
	}

	void writeHtmlFoot(FILE *handle) {
		fprintf(handle, "</table></body></html>\n");
	}
}

/**
 * Create logfile, set defaults
 */
Log::Log () {

	level = LL_DEBUG;
	quiet = false;
	logToFile = true;
	logfile = nullptr;
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
#endif

}

/**
 * Close logfile
 */
Log::~Log () {

	// finish logfile
	if (logfile) {
		writeHtmlFoot(logfile);
		fclose(logfile);
	}

	// clear buffered messages
	for (int i = 0; i < MAX_BUFFERED_MESSAGES; i++) {
		if (message_buffer[i] != nullptr) {
			delete[] message_buffer[i];
			message_buffer[i] = nullptr;
		}
	}
	message_buffer_index = 0;
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
 * Enable/Disable file logging.
 *
 * @param enable Whether to enable file logging
 */

void Log::setFile(bool enable) {

	logToFile = enable;

}

/**
 * Enable logging to file.
 *
 */
void Log::setFileReady() {
	if(!logToFile || !gamePaths.has_temp) return;

	char *logpath = createString(gamePaths.getTemp(), "openjazz-log.html");
	logfile = fopen(logpath, "w");
	if(!logfile) {
		LOG_WARN("Could not open logfile: %s", logpath);
		delete[] logpath;
		return;
	}
	delete[] logpath;

	writeHtmlHead(logfile);

	// Replay log buffer
	for (int i = 0; i < MAX_BUFFERED_MESSAGES; i++) {
		int index = (i + message_buffer_index) % MAX_BUFFERED_MESSAGES;
		if (message_buffer[index] != nullptr) {
			fprintf(logfile, "%s\n", message_buffer[index]);
			delete[] message_buffer[index];
			message_buffer[index] = nullptr;
		}
	}

	fflush(logfile);
	message_buffer_index = 0;
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
#ifdef __ANDROID__
	// on android write only to journal, and do not spew for release builds
	int maxlvl = LL_TRACE; //level;
	#ifdef NDEBUG
	maxlvl = LL_WARN;
	#endif

	// log up to set verbosity
	if(lvl < maxlvl) return;

	// use an immediate buffer for output
	char outbuffer[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(outbuffer, sizeof(outbuffer), fmt, args);
	va_end(args);

	__android_log_write((lvl == LL_ERROR) ? ANDROID_LOG_ERROR :
		(lvl == LL_WARN) ? ANDROID_LOG_WARN :
		(lvl == LL_INFO) ? ANDROID_LOG_INFO : ANDROID_LOG_DEBUG,
		"OpenJazz", outbuffer);
#else
	// get current time
	time_t t = time(nullptr);
	struct tm *now = localtime(&t);

	// log to console, up to set verbosity
	if (!quiet && lvl >= level) {
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
			LOG("%s %s%-5s\x1b[0m ", timebuf, levels[lvl].termColor, levels[lvl].name);
		else
			LOG("%s %-5s ", timebuf, levels[lvl].name);

		va_list args;
		va_start(args, fmt);
#ifdef __vita__
		// use an immediate buffer for output
		char outbuffer[1024];
		vsnprintf(outbuffer, sizeof(outbuffer), fmt, args);
		LOG(outbuffer);
#else
		vfprintf(stream, fmt, args);
#endif
		va_end(args);

		// only include source information if doing debug logs
		if (level > LL_DEBUG)
			LOG("\n");
		else {
			// extract file name (like basename)
			const char *src = strrchr(file, '\\');
			if (!src) src = strrchr(file, '/');
			if (!src)
				src = file;
			else
				src++;

			if (color)
				LOG(" \x1b[90m(%s:%d)\x1b[0m\n", src, line);
			else
				LOG(" (%s:%d)\n", src, line);
		}

		fflush(stream);

		#undef LOG
	}

	// Log to file (buffer in queue)
	char timebuf[20];
	char msgstring[MAX_MESSAGE_LENGTH];
	strftime(timebuf, 20, "%Y-%m-%d %H:%M:%S", now);

	// extract short source path
	const char *src = strstr(file, "\\src\\");
	if (!src) src = strstr(file, "/src/");
	if (!src)
		src = file;
	else
		src++;

	// timestamp column
	int len = snprintf(msgstring, MAX_MESSAGE_LENGTH, "<tr class=\"l%d\"><td>%s</td>", lvl, timebuf);
	// level column
	len += snprintf(msgstring + len, MAX_MESSAGE_LENGTH - len, "<td>%s</td>", levels[lvl].name);
	// message column hover text
	len += snprintf(msgstring + len, MAX_MESSAGE_LENGTH - len, "<td title=\"in %s, line %d\">", src, line);

	// message column
	va_list args;
	va_start(args, fmt);
	len += vsnprintf(msgstring + len, MAX_MESSAGE_LENGTH - len, fmt, args);
	va_end(args);
	snprintf(msgstring + len, MAX_MESSAGE_LENGTH - len, "</td></tr>");

	// terminate
	msgstring[MAX_MESSAGE_LENGTH - 1] = '\0';

	if (logfile) {
		// print to file
		fprintf(logfile, "%s\n", msgstring);
		fflush(logfile);
	} else {
		// add to queue

		// create or allow round-trip
		if(!message_buffer[message_buffer_index])
			message_buffer[message_buffer_index] = new char[MAX_MESSAGE_LENGTH];

		// add and advance
		strncpy(message_buffer[message_buffer_index], msgstring, MAX_MESSAGE_LENGTH);
		message_buffer[message_buffer_index][MAX_MESSAGE_LENGTH - 1] = '\0';
		message_buffer_index = (message_buffer_index + 1) % MAX_BUFFERED_MESSAGES;
	}
#endif
}

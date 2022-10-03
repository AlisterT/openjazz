
/**
 *
 * @file log.h
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
 */

#ifndef _LOG_H
#define _LOG_H

#include "OpenJazz.h"
#include <cstdio>

// Loglevels
enum {
	LL_MAX,
	LL_TRACE,
	LL_DEBUG,
	LL_INFO,
	LL_WARN,
	LL_ERROR,
	LL_FATAL
};

// Let the compiler help with parameter formats
#ifdef __GNUG__
	#define LIKE_PRINTF __attribute__((format(printf, 5, 6)))
#else
	#define LIKE_PRINTF
#endif

// Class

/// Configuration
class Log {

	public:
		Log();
		~Log();

		void setLevel(int level);
		int  getLevel();
		void setQuiet(bool enable);
		void log(int level, const char *file, int line, const char *fmt, ...) LIKE_PRINTF;

	private:
		Log(const Log&); // non construction-copyable
		Log& operator=(const Log&); // non copyable

		FILE *logfile;
		int level;
		bool quiet;
		bool color_stdout, color_stderr;

};

// Variable

EXTERN Log logger;

// Helper macros

#define LOG_MAX(...)   logger.log(LL_MAX,   __FILE__, __LINE__, __VA_ARGS__)
#define LOG_TRACE(...) logger.log(LL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_DEBUG(...) logger.log(LL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  logger.log(LL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  logger.log(LL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) logger.log(LL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) logger.log(LL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif

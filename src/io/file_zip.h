
/**
 *
 * @file file_zip.h
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
 */

#ifndef OJ_FILE_ZIP_H
#define OJ_FILE_ZIP_H

#include <miniz.h>
#include "OpenJazz.h"
#include "file.h"

// Classes

/// ZipFile i/o
class ZipFile final : public File {
	public:
		ZipFile                  (const char* name);
		~ZipFile                 ();

		size_t         getSize   () override;
		void           seek      (size_t offset, bool reset) override;
		size_t         tell      () override;
		unsigned char  loadChar  () override;

	private:
		size_t readBlock (void* buffer, size_t length) override;
};

#endif

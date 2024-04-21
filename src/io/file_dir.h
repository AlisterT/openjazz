
/**
 *
 * @file file_dir.h
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

#ifndef OJ_FILE_DIR_H
#define OJ_FILE_DIR_H

#include <stdio.h>
#include "OpenJazz.h"
#include "file.h"

// Classes

/// Directory File i/o
class DirFile final : public File {
	public:
		DirFile                  (const char* path, const char* name, bool write);
		~DirFile                 ();

		size_t         getSize   () override;
		void           seek      (size_t offset, bool reset) override;
		size_t         tell      () override;
		unsigned char  loadChar  () override;
		void           storeChar (unsigned char val) override;

	private:
		size_t readBlock (void* buffer, size_t length) override;
		FILE* file;
};

#endif

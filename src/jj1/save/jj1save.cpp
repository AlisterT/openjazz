
/**
 *
 * @file jj1save.cpp
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 * @par Description:
 * Deals with savegames.
 *
 */

#include "jj1save.h"
#include "util.h"
#include "io/log.h"

/* This algorithm has been found out by CYBERDEViL by analysing the Assembly in
 * DoxBox-X. For more information see: https://codeberg.org/CYBERDEV/JJSave/
 */

namespace {
	constexpr int multiplicatorField = 256; // 0x100
}

static void keyIterate(unsigned short &key1, unsigned short &key2) {
	// multiplicators
	constexpr int m1 = 33797; // 0x8405
	constexpr int m2 = 2056; // 0x808
	constexpr int m3 = 5;
	constexpr int m4 = 4;

	//LOG_TRACE("FROM key1, key2: 0x%04X, 0x%04X\n", key1, key2);

	// save value
	unsigned short oldKey2 = key2;

	// calculate keys
	unsigned int keyBuffer = key1 * m1;
	key2 = keyBuffer + 1; // use lower bits
	unsigned short part1 = keyBuffer >> 16; // use upper bits
	unsigned short part2 = key1 * m2;
	unsigned short part3 = oldKey2 * m3;
	unsigned short val1 = part1 + part2 + part3;
	part1 = val1 >> 8; // use upper bit
	part2 = (unsigned char)oldKey2 + (oldKey2 << m3);
	unsigned short val2 = part1 + part2 * m4;
	key1 = val2 * multiplicatorField | (unsigned char)val1; // add lower bit

	//LOG_TRACE("TO key1, key2: 0x%04X, 0x%04X\n", key1, key2);
}

static unsigned short dataOffset(unsigned short key, bool skipGarbage = false) {
	// multiplicators
	constexpr int mData = 2000; // 0x7D0

	unsigned short offset;

	if(skipGarbage) {
		// return offset to data
		offset = (key * mData) >> 16;
	} else {
		// return next data field
		offset = (key * multiplicatorField) >> 16;
	}
	//LOG_TRACE("SKIP offset: 0x%04X\n", offset);

	return offset;
}

JJ1Save::JJ1Save (const char* fileName) :
	valid(false) {

	FilePtr file;
	LOG_TRACE("Save: %s", fileName);

	try {
		file = std::make_unique<File>(fileName, PATH_TYPE_GAME);
	} catch (int e) {
		name = createString("empty");
		return;
	}

	// First load player's name

	name = file->loadString(16);
	lowercaseString(name);

	// Use termination marker as magic

	if (file->loadChar() != 0x1A) {
		LOG_WARN("Wrong save magic.");
		delete[] name;
		name = createString("invalid");
		return;
	}

	// Read creation time for initial key generation

	int m = file->loadChar();
	int h = file->loadChar();
	int t = file->loadChar();
	int s = file->loadChar();
	LOG_MAX("Save creation time: %02d:%02d:%02d:%02d.", h, m, s, t);

	unsigned short key1 = m | (h << 8);
	unsigned short key2 = t | (s << 8);

	// Calculate offset from keys
	keyIterate(key1, key2);

	// Seek to data offset
	file->seek(dataOffset(key1, true));

	// Read keys for data entries
	key1 = file->loadShort();
	key2 = file->loadShort();

	// Read XOR'ed values
	auto readValue = [&]() -> unsigned short {
		keyIterate(key1, key2);
		unsigned short bit = dataOffset(key1);
		unsigned short val = file->loadShort();

		// Switch keys
		auto tmp = key2;
		key2 = key1;
		key1 = tmp;

		return val ^ bit;
	};

	level = readValue();
	planet = readValue();
	difficulty = readValue();

	LOG_TRACE("Saved planet = %d, level = %d, difficulty = %d", planet, level, difficulty);

	// TODO: Find out about the next 6 unknown fields
	for(int i = 0; i < 6; i++)
		unknown[i] = readValue();

	LOG_MAX("Unknown save fields: %d %d %d %d %d %d", unknown[0], unknown[1], unknown[2], unknown[3], unknown[4], unknown[5]);

	valid = true;
}


JJ1Save::~JJ1Save () {
	delete[] name;
}

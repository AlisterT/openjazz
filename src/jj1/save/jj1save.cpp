
/**
 *
 * @file jj1save.cpp
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
 * Deals with savegames.
 *
 */

#include "jj1save.h"
#include "util.h"
#include "io/log.h"

/* This algorithm has been found out by CYBERDEViL by analysing the Assembly in
 * DoxBox-X. For more information see: https://codeberg.org/CYBERDEV/JJSave/
 * It has been further verified by disassembling the executable in GHIDRA
 */

namespace {
	constexpr int multiplicatorGarbage = 2000;
	constexpr int multiplicatorData = 0x100; // 256

	constexpr int creatorLength = 5;
	constexpr const char *endMessage = "figured diz partly out! -OpenJazz";

	struct KeyGenerator {
		KeyGenerator() : key(0), state(0) {}
		KeyGenerator(unsigned int initialSeed) {
			seed(initialSeed);
		}
		KeyGenerator(time_t &timepoint) {
			seedFromTime(timepoint);
		}

		// seed
		void seed(unsigned int value) {
			key = value >> 16; // upper bits
			state = value; // lower bits
		}
		void seedFromTime(time_t &timepoint) {
			struct tm* tm_info = localtime(&timepoint);
			int ts_h = tm_info->tm_hour;
			int ts_m = tm_info->tm_min;
			int ts_s = tm_info->tm_sec;

			key = (ts_s << 8) | 0; // we ignore hundredths of a second
			state = (ts_h << 8) | ts_m;
		}

		// get
		unsigned short value() {
			keyIterate();

			unsigned short val = key;
			//LOG_TRACE("value: 0x%04X", val);

			return val;
		}
		unsigned short valueWithSalt(unsigned int salt) {
			keyIterate();

			unsigned short val = (key * salt) >> 16; // use upper bits
			//LOG_TRACE("valueWithSalt(%d): 0x%04X", salt, val);

			return val;
		}

		// state
		unsigned int getKey() {
			return (key << 16) | state;
		}
		void keyIterate() {
			// multiplicator
			constexpr int magic = 0x8405; // 33797

			//unsigned short oldKey = key;
			//unsigned short oldState = state;

			// calculate keys
			unsigned int buffer = state * magic;
			unsigned short val1 = (buffer >> 16) + state * 2056 + key * 5;
			unsigned short val2 = (val1 >> 8) + ((key & 0xFF) + (key << 5)) * 4;

			// set new values
			key = (val2 << 8) | (val1 & 0xFF);
			state = buffer + 1; // use lower bits

			//LOG_TRACE("KeyIterate key: 0x%04X -> 0x%04X | state: 0x%04X -> 0x%04X",
			//	oldKey, key, oldState, state);
		}

		unsigned short key, state;
	};
}

JJ1Save::JJ1Save() :
	valid(false), name(nullptr) {
}

JJ1Save::JJ1Save(const char* saveName, int planet, int level, difficultyType difficulty) :
	valid(true), planet(planet), level(level), difficulty(difficulty) {

	name = new char[17];
	strncpy(name, saveName, 16);
	name[16] = '\0';

	for(int i = 0; i < 5; i++)
		unknown[i] = 0;
}

JJ1Save::~JJ1Save () {
	delete[] name;
}

bool JJ1Save::load (int slot) {
	valid = false;

	FilePtr file;

	if(name)
		delete[] name;

	char *fileName = makeFileName(slot);

	try {
		// FIXME: only allow game and config
		file = std::make_unique<File>(fileName, PATH_TYPE_ANY);
	} catch (int e) {
		name = createString("empty");
		delete[] fileName;

		return false;
	}

	LOG_TRACE("Loading savefile: %s", fileName);
	delete[] fileName;

	// First load player's name
	name = file->loadString(16);
	lowercaseString(name);

	// Use termination marker as magic
	if (file->loadChar() != 0x1A) {
		LOG_WARN("Wrong savefile magic.");
		delete[] name;
		name = createString("invalid");
		return false;
	}

	// Read creation time for initial key
	unsigned int key = file->loadInt();

	int h = (key >> 8) & 0xFF;
	int m = key & 0xFF;
	int s = key >> 24;
	int t = (key >> 16) & 0xFF;
	LOG_MAX("Savefile creation time: %02d:%02d:%02d.%02d", h, m, s, t);

	KeyGenerator gen(key);

	// Seek to data offset
	file->seek(gen.valueWithSalt(multiplicatorGarbage));

	// Read keys for data entries
	key = file->loadInt();
	gen.seed(key);

	// Read XOR'ed values
	auto readValue = [&]() -> unsigned short {
		unsigned short bit = gen.valueWithSalt(multiplicatorData);
		unsigned short val = file->loadShort();

		return val ^ bit;
	};

	level = readValue();
	planet = readValue();
	difficulty = static_cast<difficultyType>(readValue());
	LOG_TRACE("Loaded planet = %d, level = %d, difficulty = %d", planet, level, +difficulty);

	// TODO: Find out about the 5 unknown fields
	unknown[0] = readValue();
	unknown[1] = readValue();
	unknown[2] = readValue();
	worldDisplay = readValue();
	unknown[3] = readValue();
	unknown[4] = readValue();

	// FIXME: calculation is not 100%
	int num1, num2;
	if(worldDisplay < 99) {
		num1 = worldDisplay + 1;
		num2 = 1;
	} else {
		worldDisplay++;
		num1 = worldDisplay / 10;
		num2 = worldDisplay % 10;
	}
	LOG_MAX("Savefile World Display: %d-%d", num1, num2);
	LOG_MAX("Unknown savefile fields: %d %d %d %d %d", unknown[0], unknown[1], unknown[2], unknown[3], unknown[4]);

	// Seek to end of message
	file->seek(file->getSize() - creatorLength - 1, true);
	char *creator = file->loadString(creatorLength);
	bool isJJ1 = strncmp(creator, "arjan", creatorLength) == 0;
	bool isOJ = strncmp(creator, "nJazz", creatorLength) == 0;
	delete[] creator;

	LOG_TRACE("Savefile created by %s", isJJ1 ? "original engine" :
		isOJ ? "OpenJazz" : "unknown/J1E");

	valid = true;
	return true;
}

std::unique_ptr<JJ1Save> JJ1Save::fromSlot(int slot) {
	auto save = std::make_unique<JJ1Save>();
	save->load(slot);
	return save;
}

bool JJ1Save::save(int slot) {
	if(!valid) {
		LOG_ERROR("Invalid save data.");

		return false;
	}

	FilePtr file;
	char *fileName = makeFileName(slot);
	LOG_TRACE("Saving savefile: %s", fileName);

	try {
		file = std::make_unique<File>(fileName, PATH_TYPE_CONFIG, true);
	} catch (int e) {
		LOG_ERROR("Could not open save file '%s' for writing", fileName);
		delete[] fileName;

		return false;
	}
	delete[] fileName;

	// Write uppercase save name
	char *saveName = createString(name);
	uppercaseString(saveName);
	file->storeString(saveName);
	// Pad with spaces
	for (int i = strlen(saveName); i < 16; i++) {
		file->storeChar(' ');
	}
	delete[] saveName;

	// Magic terminator
	file->storeChar(0x1A);

	// Timestamp from current time
	time_t now = time(nullptr);
	KeyGenerator gen(now);
	file->storeInt(gen.getKey());

	// Write garbage bytes up to data offset
	int garbageCount = gen.valueWithSalt(multiplicatorGarbage);
	for (int i = 0; i < garbageCount; i++) {
#if WRITE_GARBAGE
		file->storeChar(gen.valueWithSalt(multiplicatorData));
#else
		gen.keyIterate();
		file->storeChar(0);
#endif
	}

	// Write data section keys
	file->storeInt(gen.getKey());

	// Write XOR'ed values
	auto writeValue = [&](unsigned short value) {
		unsigned short bit = gen.valueWithSalt(multiplicatorData);

		file->storeShort(value ^ bit);
	};

	// write known fields
	writeValue(level);
	writeValue(planet);
	writeValue(+difficulty);
	LOG_TRACE("Saved planet = %d, level = %d, difficulty = %d", planet, level, +difficulty);

	// write unknown fields
	writeValue(unknown[0]);
	writeValue(unknown[1]);
	writeValue(unknown[2]);

	// write display number "23-2" (unused by OJ)
	writeValue(232);

	// write unknown fields
	writeValue(unknown[3]);
	writeValue(unknown[4]);

#ifdef WRITE_GARBAGE
	// Write garbage bytes up to message
	garbageCount = gen.valueWithSalt(1000);
	for (int i = 0; i < garbageCount; i++)
		file->storeChar(gen.valueWithSalt(multiplicatorData));
#else
	// just separate the message
	file->storeChar(0);
#endif

	// write message
	file->storeString(endMessage);

	// write end marker
	file->storeChar(0);

	return true;
}

char *JJ1Save::makeFileName(int slot) {
	char* fileName = createString("SAVE.0");
	fileName[5] += slot;
	return fileName;
}

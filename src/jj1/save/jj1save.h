
/**
 *
 * @file jj1save.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2026 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef OJ_JJ1SAVE_H
#define OJ_JJ1SAVE_H

#include "io/file.h"
#include "types.h"

// Class

class JJ1Save {

	public:
		JJ1Save(); // default (invalid until file loaded)
		JJ1Save(const char* saveName, int planet, int level, difficultyType difficulty);
		~JJ1Save();

		bool load(int slot);
		static std::unique_ptr<JJ1Save> fromSlot(int slot); // factory
		bool save(int slot);

		bool isValid() const;
		const char* getName() const;
		int getPlanet() const;
		int getLevel() const;
		difficultyType getDifficulty() const;

	private:
		JJ1Save(const JJ1Save&); // non construction-copyable
		JJ1Save& operator=(const JJ1Save&); // non copyable

		char *makeFileName(int slot);

		bool valid;
		char* name;
		int planet;
		int level;
		difficultyType difficulty;
		int worldDisplay;
		int unknown[5];

};

// Inline Functions

inline bool JJ1Save::isValid() const { return valid; }
inline const char* JJ1Save::getName() const { return name; }
inline int JJ1Save::getPlanet() const { return planet; }
inline int JJ1Save::getLevel() const { return level; }
inline difficultyType JJ1Save::getDifficulty() const { return difficulty; }

#endif

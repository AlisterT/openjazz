
/**
 *
 * @file platform_interface.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2015-2024 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef OJ_PLATFORM_INTERFACE_H
#define OJ_PLATFORM_INTERFACE_H

class IPlatform {
	public:
		virtual ~IPlatform() =0;

		static IPlatform* make();

		virtual bool WantsExit();

		virtual void AddGamePaths();
		virtual void ErrorNoDatafiles();

		virtual bool InputIP(char*& current_ip, char*& new_ip);
		virtual bool InputString(const char* hint, char*& current_string, char*& new_string);
	protected:
		IPlatform() {};
};

#endif

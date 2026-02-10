
/**
 *
 * @file platform_interface.h
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


#ifndef _PLATFORM_INTERFACE_H
#define _PLATFORM_INTERFACE_H

class IPlatform {
	public:
		virtual ~IPlatform() =0;

		static IPlatform* make();

		virtual void NetInit();
		virtual void NetExit();
		virtual bool NetHasConsole();

		virtual bool WantsExit();

		virtual void AddGamePaths();
		virtual void ErrorNoDatafiles();

		virtual bool InputIP(char*& current_ip, char*& new_ip);
		virtual bool InputString(const char* hint, char*& current_string, char*& new_string);
	protected:
		IPlatform() {};
};

// stubs

inline void IPlatform::NetInit() { }
inline void IPlatform::NetExit() { }
inline bool IPlatform::NetHasConsole() { return false; }

inline bool IPlatform::WantsExit() { return false; }

inline bool IPlatform::InputIP(char*&, char*&) {
	return false;
}
inline bool IPlatform::InputString(const char*, char*&, char*&) {
	return false;
}

#endif

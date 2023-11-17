
/**
 *
 * @file types.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2005-2017 Alister Thomson
 * Copyright (c) 2015-2023 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef OJ_TYPES_H
#define OJ_TYPES_H

// Constants

// Numbers in -10 exponent fixed point
#define FE   128
#define FQ   256
#define FH   512
#define F1   1024
#define F2   2048
#define F4   4096
#define F8   8192
#define F10  10240
#define F12  12288
#define F16  16384
#define F20  20480
#define F24  24576
#define F32  32768
#define F36  36864
#define F40  40960
#define F80  81920
#define F64  65536
#define F100 102400
#define F160 163840
#define F192 196608

// Macros

// For fixed-point operations
#define FTOI(x) ((x) >> 10) ///< Fixed to Int
#define ITOF(x) ((x) << 10) ///< Int to Fixed
#define MUL(x, y) (((x) * (y)) >> 10) ///< multiplication
#define DIV(x, y) (((x) << 10) / (y)) ///< division

// Datatypes

typedef int fixed; ///< Custom fixed-point data type

namespace SE {
	enum Type : int; ///< Sound Index type
}

#endif

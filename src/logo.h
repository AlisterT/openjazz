
/**
 *
 * @file logo.h
 *
 * Part of the OpenJazz project
 *
 * @par Licence:
 * Copyright (c) 2005-2017 AJ Thomson
 * Copyright (c) 2015-2023 Carsten Teibes
 *
 * OpenJazz is distributed under the terms of
 * the GNU General Public License, version 2.0
 *
 */


#ifndef _LOGO_H
#define _LOGO_H

/* This struct contains an image as zlib compressed deflate block with
   additional information about the image */

struct compressed_image {
	unsigned int width;
	unsigned int height;
	unsigned int size;
	unsigned int compressed_size;
	unsigned char *data;
};

// Former openjazz.000, without RLE compression - raw image (without palette)

unsigned char oj_logo_data[] = {
	0x78, 0xda, 0xb5, 0x96, 0xb1, 0x92, 0x83, 0x20, 0x10, 0x86, 0x2b, 0x95,
	0x9e, 0x36, 0xc3, 0x2b, 0xf0, 0x00, 0x57, 0xd3, 0xd3, 0x61, 0x61, 0x4c,
	0x73, 0xcf, 0x90, 0x67, 0xd0, 0x22, 0xd1, 0x67, 0xf3, 0x66, 0x8c, 0x0f,
	0x23, 0xcc, 0x45, 0x0d, 0x61, 0xc1, 0x45, 0x32, 0x37, 0x73, 0x7f, 0xc9,
	0xf0, 0xfd, 0x2e, 0xbb, 0xcb, 0xca, 0xe9, 0x04, 0x95, 0x13, 0x2a, 0x67,
	0xbd, 0x48, 0x48, 0x5e, 0x9c, 0x62, 0x1a, 0xc6, 0xb7, 0xe0, 0x72, 0x41,
	0xb5, 0x51, 0x4e, 0x46, 0xf0, 0x2c, 0xc2, 0x4f, 0x56, 0x80, 0x2f, 0x24,
	0x84, 0x37, 0x69, 0xfa, 0x29, 0x5f, 0xd0, 0x3d, 0xbd, 0x3a, 0xf0, 0x8f,
	0x78, 0xa2, 0x55, 0x44, 0x86, 0x66, 0x69, 0x9e, 0x1b, 0x15, 0x97, 0xc8,
	0x53, 0x3c, 0x39, 0xc2, 0x11, 0x83, 0x80, 0xe7, 0xc7, 0xb8, 0x52, 0xf3,
	0x21, 0x5f, 0xa4, 0x70, 0xa5, 0xe8, 0x11, 0x2f, 0x93, 0xb8, 0x32, 0x24,
	0xce, 0xfb, 0xd1, 0x9b, 0x73, 0xd3, 0x2e, 0xaa, 0x4b, 0xbf, 0x8c, 0x71,
	0xde, 0xab, 0xdc, 0x77, 0xdb, 0xf5, 0x9b, 0xee, 0x95, 0xe7, 0xcb, 0x63,
	0x3c, 0x87, 0xbb, 0x2a, 0x4b, 0x2f, 0x6a, 0x61, 0x08, 0x3a, 0xc7, 0xf9,
	0x4c, 0x40, 0xbc, 0xf7, 0x04, 0x0d, 0x0c, 0xc7, 0x79, 0x58, 0x7a, 0xef,
	0xeb, 0xab, 0x01, 0xf0, 0x96, 0x38, 0x4f, 0xdd, 0x8e, 0x32, 0xc4, 0xfb,
	0xbe, 0xc1, 0x33, 0x08, 0x78, 0x50, 0xbc, 0x66, 0x87, 0xf7, 0x37, 0x77,
	0x02, 0x93, 0xa3, 0xfc, 0xec, 0x36, 0xb4, 0x7b, 0xbe, 0xaf, 0x9d, 0x3d,
	0x41, 0x79, 0x57, 0xbd, 0xf2, 0x86, 0xf0, 0x0d, 0x5a, 0x41, 0x9c, 0xbf,
	0x23, 0x7c, 0x8b, 0xf6, 0xf0, 0xff, 0xf0, 0x7f, 0x8a, 0xdf, 0xe5, 0x4f,
	0x21, 0xf9, 0xeb, 0x2e, 0xa9, 0xfc, 0xc9, 0x68, 0xf7, 0x85, 0xf5, 0xcb,
	0x52, 0xfd, 0x63, 0xf6, 0x09, 0xa8, 0x92, 0xfd, 0x03, 0x87, 0xc7, 0xe5,
	0x7e, 0xd0, 0xbf, 0x5f, 0x23, 0x43, 0xef, 0x1f, 0xbc, 0x3f, 0x75, 0x17,
	0xbf, 0x3f, 0x8f, 0xc9, 0xf1, 0x23, 0xe0, 0x09, 0xbc, 0xbf, 0x35, 0xac,
	0x41, 0x0b, 0x07, 0xc0, 0x75, 0x9a, 0x06, 0x8b, 0xb3, 0x37, 0xbe, 0xac,
	0xcd, 0xd0, 0xa0, 0x6c, 0x5f, 0x67, 0xe8, 0x6e, 0xb5, 0x37, 0x80, 0x1e,
	0xd3, 0xdb, 0x80, 0x8d, 0x1e, 0x1f, 0x4c, 0xdf, 0xb2, 0x5e, 0x06, 0x58,
	0x75, 0xf6, 0x57, 0xaf, 0xdb, 0x6e, 0x16, 0xe0, 0x6b, 0x4e, 0xe8, 0x07,
	0xf3, 0xf3, 0x61, 0x01, 0x00, 0xdb, 0x88, 0x72, 0x9d, 0xe4, 0x7f, 0x26,
	0x5c, 0x2c, 0xf1, 0xf3, 0xb3, 0xb5, 0x8b, 0xe0, 0x36, 0xa3, 0x89, 0xff,
	0x57, 0x0c, 0x77, 0x1d, 0x71, 0x18, 0x81, 0x18, 0x53, 0xf8, 0x33, 0x07,
	0x22, 0x9a, 0x3a, 0xee, 0x65, 0x1c, 0xc7, 0x9f, 0x06, 0x14, 0x0d, 0xc1,
	0x08, 0x12, 0x94, 0x0c, 0xc7, 0x71, 0x87, 0x8d, 0x5e, 0x0c, 0x86, 0xc0,
	0x61, 0x1c, 0xd8, 0xfe, 0x59, 0x92, 0x73, 0x01, 0x1e, 0x50, 0x46, 0x53,
	0x70, 0xe5, 0x3d, 0x07, 0x94, 0x7e, 0x59, 0x50, 0x29, 0x9e, 0x92, 0x94,
	0x93, 0xe0, 0xd5, 0xc0, 0xd8, 0xb0, 0xbe, 0xdb, 0x06, 0x0f, 0xfe, 0x05,
	0xcd, 0x6d, 0x45, 0x28
};

struct compressed_image oj_logo = { 64, 40, 2560, 436, oj_logo_data };

#endif


#include <cstdio>
#include "lodepng-miniz.h"

unsigned lodepng_crc32(const unsigned char* data, size_t length) {
	//mz_ulong mz_crc32(mz_ulong crc, const unsigned char *ptr, size_t buf_len);
	return mz_crc32(0, data, length);
}

unsigned lodepng_miniz_decoder(unsigned char** out, size_t* outsize,
                               const unsigned char* in, size_t insize,
                               const LodePNGDecompressSettings*) {

	*outsize = insize * 260; // arbitrary size
	*out = (unsigned char *)malloc(*outsize);
	if(*out == nullptr) return 110;

	//int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len);
	int res = mz_uncompress(*out, (mz_ulong*)outsize, in, insize);

	//printf("DECODE out: %p (%zu), in: %p (%zu), result: %d\n", *out, *outsize, in, insize, res);

	if (res != 0) {
		//free(*out); // lodepng does this
		return 110;
	}

	return 0;
}

unsigned lodepng_miniz_encoder(unsigned char** out, size_t* outsize,
                               const unsigned char* in, size_t insize,
                               const LodePNGCompressSettings*) {

	*outsize = mz_compressBound(insize);
	*out = (unsigned char *)malloc(*outsize);
	if(*out == nullptr) return 111;

	//int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len, const unsigned char *pSource, mz_ulong source_len, int level);
	int res = mz_compress2(*out, (mz_ulong*)outsize, in, insize, MZ_DEFAULT_LEVEL + 2);

	//printf("ENCODE out: %p (%zu), in: %p (%zu), result: %d\n", *out, *outsize, in, insize, res);

	if (res != 0) {
		free(*out);
		return 111;
	}

	return 0;
}

void lodepng_state_add_miniz(LodePNGState &state) {
	state.decoder.zlibsettings.custom_zlib = lodepng_miniz_decoder;
	state.encoder.zlibsettings.custom_zlib = lodepng_miniz_encoder;
}

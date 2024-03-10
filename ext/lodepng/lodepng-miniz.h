
#include <miniz.h>
#include "lodepng.h"

unsigned lodepng_crc32(const unsigned char* data, size_t length);

/*Should return 0 if success, any non-0 if error (numeric value not exposed).*/
unsigned lodepng_miniz_decoder(unsigned char** out, size_t* outsize,
                               const unsigned char* in, size_t insize,
                               const LodePNGDecompressSettings* settings);

unsigned lodepng_miniz_encoder(unsigned char** out, size_t* outsize,
                               const unsigned char* in, size_t insize,
                               const LodePNGCompressSettings* settings);

void lodepng_state_add_miniz(LodePNGState &state);

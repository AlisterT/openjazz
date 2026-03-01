#ifndef LIBXMP_MIX_ALL_H
#define LIBXMP_MIX_ALL_H

#include "common.h"

/* Mixers array index:
 *
 * bit 0: 0=8 bit sample, 1=16 bit sample
 * bit 1: 0=mono sample, 1=stereo sample
 * bit 2: 0=mono output, 1=stereo output
 * bit 3: 0=unfiltered, 1=filtered
 */

#define MIXER(f) static void libxmp_mix_##f(struct mixer_voice * LIBXMP_RESTRICT vi, \
	int32 * LIBXMP_RESTRICT buffer, int count, int vl, int vr, int step, int ramp, \
	int delta_l, int delta_r)

typedef void (*MIXER_FP)(struct mixer_voice * LIBXMP_RESTRICT vi,
	int32 * LIBXMP_RESTRICT buffer, int count, int vl, int vr, int step, int ramp,
	int delta_l, int delta_r);

#define LIST_MIX_FUNCTIONS(type) \
	libxmp_mix_monoout_mono_8bit_ ## type, \
	libxmp_mix_monoout_mono_16bit_ ## type, \
	libxmp_mix_monoout_stereo_8bit_ ## type, \
	libxmp_mix_monoout_stereo_16bit_ ## type, \
	libxmp_mix_stereoout_mono_8bit_ ## type, \
	libxmp_mix_stereoout_mono_16bit_ ## type, \
	libxmp_mix_stereoout_stereo_8bit_ ## type, \
	libxmp_mix_stereoout_stereo_16bit_ ## type

#define LIST_MIX_FUNCTIONS_PAULA(type) \
	libxmp_mix_monoout_mono_ ## type, NULL, NULL, NULL, \
	libxmp_mix_stereoout_mono_ ## type, NULL, NULL, NULL, \
	NULL, NULL, NULL, NULL, \
	NULL, NULL, NULL, NULL

LIBXMP_BEGIN_DECLS

extern const MIXER_FP libxmp_nearest_mixers[];
extern const MIXER_FP libxmp_linear_mixers[];
extern const MIXER_FP libxmp_spline_mixers[];

/* mix_paula.c */
#ifdef LIBXMP_PAULA_SIMULATOR
extern const MIXER_FP libxmp_a500_mixers[];
extern const MIXER_FP libxmp_a500led_mixers[];
#endif

LIBXMP_END_DECLS

#endif /* LIBXMP_MIX_ALL_H */

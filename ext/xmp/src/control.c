/* Extended Module Player
 * Copyright (C) 1996-2026 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "format.h"
#include "virtual.h"
#include "mixer.h"
#include "rng.h"

/* TODO: Change this to const char *const in a future ABI change */
const char *xmp_version LIBXMP_EXPORT_VAR = XMP_VERSION;
const unsigned int xmp_vercode LIBXMP_EXPORT_VAR = XMP_VERCODE;

xmp_context xmp_create_context(void)
{
	struct context_data *ctx;

	ctx = (struct context_data *) calloc(1, sizeof(struct context_data));
	if (ctx == NULL) {
		return NULL;
	}

	ctx->state = XMP_STATE_UNLOADED;
	ctx->m.defpan = 100;
	ctx->s.numvoc = SMIX_NUMVOC;
	libxmp_init_random(&ctx->rng);

	return (xmp_context)ctx;
}

void xmp_free_context(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct module_data *m = &ctx->m;

	if (ctx->state > XMP_STATE_UNLOADED)
		xmp_release_module(opaque);

	free(m->instrument_path);
	free(opaque);
}

static void set_position(struct context_data *ctx, int pos, int dir)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct flow_control *f = &p->flow;
	int seq;
	int has_marker;

	/* If dir is 0, we can jump to a different sequence */
	if (dir == 0) {
		seq = libxmp_get_sequence(ctx, pos);
	} else {
		seq = p->sequence;
	}

	if (seq < 0 || seq >= m->num_sequences) {
		return;
	}

	has_marker = HAS_QUIRK(QUIRK_MARKER);

	p->sequence = seq;

	if (pos >= 0) {
		int pat;

		while (has_marker && pos > 0 && pos < mod->len - 1 &&
		       mod->xxo[pos] == XMP_MARK_SKIP) {
			if (dir < 0) {
				pos--;
			} else {
				pos++;
			}
		}
		if (pos >= mod->len) {
			return;
		}
		pat = mod->xxo[pos];

		if (pat < mod->pat) {
			if (has_marker && pat == XMP_MARK_END) {
				return;
			}

			if (pos > p->scan[seq].ord) {
				f->end_point = 0;
			} else {
				f->num_rows = mod->xxp[pat]->rows;
				f->end_point = p->scan[seq].num;
				f->jumpline = 0;
			}
		}
	}

	if (pos < mod->len) {
		if (pos == 0) {
			p->pos = -1;
		} else {
			p->pos = pos;
		}
		/* Clear flow vars to prevent old pattern jumps and
		 * other junk from executing in the new position. */
		libxmp_reset_flow(ctx);
	}
}

int xmp_next_position(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (p->pos < 0) {
		/* Restart a stopped or restarting module.
		 * This was previously done implicitly. */
		set_position(ctx, -1, 1);
	} else if (p->pos < m->mod.len) {
		set_position(ctx, p->pos + 1, 1);
	}

	return p->pos;
}

int xmp_prev_position(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (p->pos == m->seq_data[p->sequence].entry_point) {
		set_position(ctx, -1, -1);
	} else if (p->pos > m->seq_data[p->sequence].entry_point) {
		set_position(ctx, p->pos - 1, -1);
	}
	return p->pos < 0 ? 0 : p->pos;
}

int xmp_set_position(xmp_context opaque, int pos)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (pos < 0 || pos >= m->mod.len)
		return -XMP_ERROR_INVALID;

	set_position(ctx, pos, 0);

	return p->pos;
}

int xmp_set_row(xmp_context opaque, int row)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct flow_control *f = &p->flow;
	int pos = p->pos;
	int pattern;

	if (pos < 0 || pos >= mod->len) {
		pos = 0;
	}
	pattern = mod->xxo[pos];

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (pattern >= mod->pat || row < 0 || row >= mod->xxp[pattern]->rows)
		return -XMP_ERROR_INVALID;

	/* See set_position. */
	if (p->pos < 0)
		p->pos = 0;
	p->ord = p->pos;
	p->row = row;
	p->frame = -1;
	f->num_rows = mod->xxp[mod->xxo[p->ord]]->rows;

	return row;
}

void xmp_stop_module(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;

	if (ctx->state < XMP_STATE_PLAYING)
		return;

	p->pos = -2;
	libxmp_reset_flow(ctx);
}

void xmp_restart_module(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;

	if (ctx->state < XMP_STATE_PLAYING)
		return;

	p->loop_count = 0;
	p->pos = -1;
	libxmp_reset_flow(ctx);
}

int xmp_seek_time(xmp_context opaque, int time)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct ord_data *oinfo;
	double t;
	int pos;
	int i;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	for (i = m->mod.len - 1; i >= 0; i--) {
		int pat = m->mod.xxo[i];
		if (pat >= m->mod.pat) {
			continue;
		}
		if (libxmp_get_sequence(ctx, i) != p->sequence) {
			continue;
		}
		/* TODO: using rounding to preserve compatibility with
		 * the old (bad) int conversion here until this API
		 * function can be fixed or replaced. */
		t = m->xxo_info[i].time;
		CLAMP(t, 0.0, (double)INT_MAX);
		if (time >= (int)t) {
			set_position(ctx, i, 1);
			break;
		}
	}
	if (i < 0) {
		xmp_set_position(opaque, 0);
	}

	/* Get the correct start row + force the next xmp_play_frame call to
	 * do a reposition, which updates properties such as BPM and global
	 * volume and normally doesn't happen within the same position: */
	pos = p->pos >= 0 ? p->pos : m->seq_data[p->sequence].entry_point;

	oinfo = &m->xxo_info[pos];
	p->flow.jumpline = oinfo->start_row;
	p->flow.force_reposition = 1;

	/* For the first p->current_time + libxmp_get_frame_time check
	 * in xmp_seek_time_frame: */
	p->current_time = oinfo->time;
	p->bpm = oinfo->bpm;

	return p->pos < 0 ? 0 : p->pos;
}

int xmp_seek_time_frame(xmp_context opaque, int time)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	double max_time, t;
	int ret, i;

	if ((ret = xmp_seek_time(opaque, time)) < 0) {
		return ret;
	}

	/* set_position/xmp_set_position doesn't actually complete the seek;
	 * may need to play frames from the start of the new position until
	 * the player is at the closest frame to the requested time.
	 * TODO: it may be possible to get closer times for xmp_play_buffer
	 * users.
	 */
#if 0
	D_(D_INFO "%d %d init: %.06f, %.06f >=? %.06f", p->pos, p->flow.jumpline,
		p->current_time, p->current_time + libxmp_get_frame_time(ctx),
		(double)time);
#endif

	max_time = m->seq_data[p->sequence].duration - 0.1;
	t = MIN((double)time, max_time);

	/* Try to find the correct frame (this may take a while).
	 * TODO: temporarily put the mixer in a lower rate? */
	for (i = 0; i < (1 << 13); i++) {
		double prev = p->current_time;

		/* TODO: the actual BPM isn't known until mid-frame. */
		if (p->current_time + libxmp_get_frame_time(ctx) > t) {
			break;
		}
		if (xmp_play_frame(opaque) < 0 || p->current_time < prev) {
			break;
		}
#if 0
		D_(D_INFO "%d %d %d: %.06f >=? %.06f", p->pos, p->row, p->frame,
			p->current_time + libxmp_get_frame_time(ctx), t);
#endif
	}

	/* Force an xmp_play_buffer refresh so the new (wrong) frame data
	 * doesn't confuse the caller: */
	xmp_play_buffer(opaque, NULL, 0, 0);

	return p->pos < 0 ? 0 : p->pos;
}

int xmp_channel_mute(xmp_context opaque, int chn, int status)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	int ret;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (chn < 0 || chn >= XMP_MAX_CHANNELS) {
		return -XMP_ERROR_INVALID;
	}

	ret = p->channel_mute[chn];

	if (status >= 2) {
		p->channel_mute[chn] = !p->channel_mute[chn];
	} else if (status >= 0) {
		p->channel_mute[chn] = status;
	}

	return ret;
}

int xmp_channel_vol(xmp_context opaque, int chn, int vol)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	int ret;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (chn < 0 || chn >= XMP_MAX_CHANNELS) {
		return -XMP_ERROR_INVALID;
	}

	ret = p->channel_vol[chn];

	if (vol >= 0 && vol <= 100) {
		p->channel_vol[chn] = vol;
	}

	return ret;
}

#ifdef USE_VERSIONED_SYMBOLS
LIBXMP_BEGIN_DECLS /* no name-mangling */
LIBXMP_EXPORT_VERSIONED extern int xmp_set_player_v40__(xmp_context, int, int) LIBXMP_ATTRIB_SYMVER("xmp_set_player@XMP_4.0");
LIBXMP_EXPORT_VERSIONED extern int xmp_set_player_v41__(xmp_context, int, int)
			__attribute__((alias("xmp_set_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_set_player@XMP_4.1");
LIBXMP_EXPORT_VERSIONED extern int xmp_set_player_v43__(xmp_context, int, int)
			__attribute__((alias("xmp_set_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_set_player@XMP_4.3");
LIBXMP_EXPORT_VERSIONED extern int xmp_set_player_v44__(xmp_context, int, int)
			__attribute__((alias("xmp_set_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_set_player@@XMP_4.4");

#ifndef HAVE_ATTRIBUTE_SYMVER
asm(".symver xmp_set_player_v40__, xmp_set_player@XMP_4.0");
asm(".symver xmp_set_player_v41__, xmp_set_player@XMP_4.1");
asm(".symver xmp_set_player_v43__, xmp_set_player@XMP_4.3");
asm(".symver xmp_set_player_v44__, xmp_set_player@@XMP_4.4");
#endif
LIBXMP_END_DECLS

#define xmp_set_player__ xmp_set_player_v40__
#else
#define xmp_set_player__ xmp_set_player
#endif

int xmp_set_player__(xmp_context opaque, int parm, int val)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = &ctx->s;
	int ret = -XMP_ERROR_INVALID;


	if (parm == XMP_PLAYER_SMPCTL || parm == XMP_PLAYER_DEFPAN) {
		/* these should be set before loading the module */
		if (ctx->state >= XMP_STATE_LOADED) {
			return -XMP_ERROR_STATE;
		}
	} else if (parm == XMP_PLAYER_VOICES) {
		/* these should be set before start playing */
		if (ctx->state >= XMP_STATE_PLAYING) {
			return -XMP_ERROR_STATE;
		}
	} else if (ctx->state < XMP_STATE_PLAYING) {
		return -XMP_ERROR_STATE;
	}

	switch (parm) {
	case XMP_PLAYER_AMP:
		if (val >= 0 && val <= 3) {
			s->amplify = val;
			ret = 0;
		}
		break;
	case XMP_PLAYER_MIX:
		if (val >= -100 && val <= 100) {
			s->mix = val;
			ret = 0;
		}
		break;
	case XMP_PLAYER_INTERP:
		if (val >= XMP_INTERP_NEAREST && val <= XMP_INTERP_SPLINE) {
			s->interp = val;
			ret = 0;
		}
		break;
	case XMP_PLAYER_DSP:
		s->dsp = val;
		ret = 0;
		break;
	case XMP_PLAYER_FLAGS: {
		p->player_flags = val;
		ret = 0;
		break; }

	/* 4.1 */
	case XMP_PLAYER_CFLAGS: {
		int vblank = p->flags & XMP_FLAGS_VBLANK;
		p->flags = val;
		if (vblank != (p->flags & XMP_FLAGS_VBLANK))
			libxmp_scan_sequences(ctx);
		ret = 0;
		break; }
	case XMP_PLAYER_SMPCTL:
		m->smpctl = val;
		ret = 0;
		break;
	case XMP_PLAYER_VOLUME:
		if (val >= 0 && val <= 200) {
			p->master_vol = val;
			ret = 0;
		}
		break;
	case XMP_PLAYER_SMIX_VOLUME:
		if (val >= 0 && val <= 200) {
			p->smix_vol = val;
			ret = 0;
		}
		break;

	/* 4.3 */
	case XMP_PLAYER_DEFPAN:
		if (val >= 0 && val <= 100) {
			m->defpan = val;
			ret = 0;
		}
		break;

	/* 4.4 */
	case XMP_PLAYER_MODE:
		p->mode = val;
		libxmp_set_player_mode(ctx);
		libxmp_scan_sequences(ctx);
		ret = 0;
		break;
	case XMP_PLAYER_VOICES:
		s->numvoc = val;
		break;
	}

	return ret;
}

#ifdef USE_VERSIONED_SYMBOLS
LIBXMP_BEGIN_DECLS /* no name-mangling */
LIBXMP_EXPORT_VERSIONED extern int xmp_get_player_v40__(xmp_context, int) LIBXMP_ATTRIB_SYMVER("xmp_get_player@XMP_4.0");
LIBXMP_EXPORT_VERSIONED extern int xmp_get_player_v41__(xmp_context, int)
		__attribute__((alias("xmp_get_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_get_player@XMP_4.1");
LIBXMP_EXPORT_VERSIONED extern int xmp_get_player_v42__(xmp_context, int)
		__attribute__((alias("xmp_get_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_get_player@XMP_4.2");
LIBXMP_EXPORT_VERSIONED extern int xmp_get_player_v43__(xmp_context, int)
		__attribute__((alias("xmp_get_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_get_player@XMP_4.3");
LIBXMP_EXPORT_VERSIONED extern int xmp_get_player_v44__(xmp_context, int)
		__attribute__((alias("xmp_get_player_v40__"))) LIBXMP_ATTRIB_SYMVER("xmp_get_player@@XMP_4.4");

#ifndef HAVE_ATTRIBUTE_SYMVER
asm(".symver xmp_get_player_v40__, xmp_get_player@XMP_4.0");
asm(".symver xmp_get_player_v41__, xmp_get_player@XMP_4.1");
asm(".symver xmp_get_player_v42__, xmp_get_player@XMP_4.2");
asm(".symver xmp_get_player_v43__, xmp_get_player@XMP_4.3");
asm(".symver xmp_get_player_v44__, xmp_get_player@@XMP_4.4");
#endif
LIBXMP_END_DECLS

#define xmp_get_player__ xmp_get_player_v40__
#else
#define xmp_get_player__ xmp_get_player
#endif

int xmp_get_player__(xmp_context opaque, int parm)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = &ctx->s;
	int ret = -XMP_ERROR_INVALID;

	if (parm == XMP_PLAYER_SMPCTL || parm == XMP_PLAYER_DEFPAN) {
		// can read these at any time
	} else if (parm != XMP_PLAYER_STATE && ctx->state < XMP_STATE_PLAYING) {
		return -XMP_ERROR_STATE;
	}

	switch (parm) {
	case XMP_PLAYER_AMP:
		ret = s->amplify;
		break;
	case XMP_PLAYER_MIX:
		ret = s->mix;
		break;
	case XMP_PLAYER_INTERP:
		ret = s->interp;
		break;
	case XMP_PLAYER_DSP:
		ret = s->dsp;
		break;
	case XMP_PLAYER_FLAGS:
		ret = p->player_flags;
		break;

	/* 4.1 */
	case XMP_PLAYER_CFLAGS:
		ret = p->flags;
		break;
	case XMP_PLAYER_SMPCTL:
		ret = m->smpctl;
		break;
	case XMP_PLAYER_VOLUME:
		ret = p->master_vol;
		break;
	case XMP_PLAYER_SMIX_VOLUME:
		ret = p->smix_vol;
		break;

	/* 4.2 */
	case XMP_PLAYER_STATE:
		ret = ctx->state;
		break;

	/* 4.3 */
	case XMP_PLAYER_DEFPAN:
		ret = m->defpan;
		break;

	/* 4.4 */
	case XMP_PLAYER_MODE:
		ret = p->mode;
		break;
	case XMP_PLAYER_MIXER_TYPE:
		ret = XMP_MIXER_STANDARD;
		if (p->flags & XMP_FLAGS_A500) {
			if (IS_AMIGA_MOD()) {
#ifdef LIBXMP_PAULA_SIMULATOR
				if (p->filter) {
					ret = XMP_MIXER_A500F;
				} else {
					ret = XMP_MIXER_A500;
				}
#endif
			}
		}
		break;
	case XMP_PLAYER_VOICES:
		ret = s->numvoc;
		break;
	}

	return ret;
}

const char *const *xmp_get_format_list(void)
{
	return format_list();
}

void xmp_inject_event(xmp_context opaque, int channel, struct xmp_event *e)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;

	if (ctx->state < XMP_STATE_PLAYING)
		return;

	memcpy(&p->inject_event[channel], e, sizeof(struct xmp_event));
	p->inject_event[channel]._flag = 1;
}

int xmp_set_instrument_path(xmp_context opaque, const char *path)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct module_data *m = &ctx->m;

	if (m->instrument_path != NULL) {
		free(m->instrument_path);
		m->instrument_path = NULL;
	}
	if (path == NULL) {
		return 0;
	}

	m->instrument_path = libxmp_strdup(path);
	if (m->instrument_path == NULL) {
		return -XMP_ERROR_SYSTEM;
	}

	return 0;
}

int xmp_set_tempo_factor(xmp_context opaque, double val)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = &ctx->s;
	int ticksize;

	/* This function relies on values initialized by xmp_start_player
	 * and will behave in an undefined manner if called prior. */
	if (ctx->state < XMP_STATE_PLAYING) {
		return -XMP_ERROR_STATE;
	}

	if (val <= 0.0 || val != val /* NaN */) {
		return -1;
	}

	val *= 10;

	/* s->freq can change between xmp_start_player calls and p->bpm can
	 * change during playback, so repeat these checks in the mixer. */
	ticksize = libxmp_mixer_get_ticksize(s->freq,
		val * p->time_factor_relative, m->rrate, p->bpm);

	/* ticksize is in frames, s->total_size is in frames * 2. */
	if (ticksize < 0 || ticksize > (s->total_size / 2)) {
		return -1;
	}
	m->time_factor = val;

	return 0;
}

int xmp_set_tempo_factor_relative(xmp_context opaque, double val)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = &ctx->s;
	int ticksize;

	/* This function relies on values initialized by xmp_start_player
	 * and will behave in an undefined manner if called prior. */
	if (ctx->state < XMP_STATE_PLAYING) {
		return -XMP_ERROR_STATE;
	}

	if (val <= 0.0 || val != val /* NaN */) {
		return -1;
	}

	ticksize = libxmp_mixer_get_ticksize(s->freq,
		m->time_factor * val, m->rrate, p->bpm);

	/* ticksize is in frames, s->total_size is in frames * 2. */
	if (ticksize < 0 || ticksize > (s->total_size / 2)) {
		return -1;
	}
	p->time_factor_relative = val;

	return 0;
}

double xmp_get_tempo_factor(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;

	if (ctx->state < XMP_STATE_LOADED) {
		return -1.0 * XMP_ERROR_STATE;
	}

	return ctx->m.time_factor * 0.1;
}

double xmp_get_tempo_factor_relative(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;

	if (ctx->state < XMP_STATE_PLAYING) {
		return -1.0 * XMP_ERROR_STATE;
	}

	return ctx->p.time_factor_relative;
}

/* tiv - terminal image viewer - copyleft 2013-2015 - pancake */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <jpeglib.h>

#include "stiv.h"

#define ABS(x) (((x)<0)?-(x):(x))
#define POND(x,y) (ABS((x)) * (y))

typedef void(*stiv_renderer_t)(const uint8_t*, const uint8_t*);

typedef struct stivctx_t {
	uint8_t* buffer;
	stiv_renderer_t renderer;
	size_t buffer_size;
	uint32_t width;
	uint32_t height;
} stivctx_t;

static inline int reduce8(uint8_t r, uint8_t g, uint8_t b) {
	int select = 0, odistance = -1, i = 0,
		distance = 0;

	const int colors[][3] = {
		{ 0x00,0x00,0x00 }, // black
		{ 0xd0,0x10,0x10 }, // red
		{ 0x10,0xe0,0x10 }, // green
		{ 0xf7,0xf5,0x3a }, // yellow
		{ 0x10,0x10,0xf0 }, // blue // XXX
		{ 0xfb,0x3d,0xf8 }, // pink
		{ 0x10,0xf0,0xf0 }, // turqoise
		{ 0xf0,0xf0,0xf0 }, // white
	};
	const int colors_len = sizeof(colors) / sizeof(colors[0]);

	// B&W
	if (r < 30 && g < 30 && b < 30) return 0;
	if (r > 200 && g > 200 && b > 200) return 7;

	for (i = 0; i < colors_len; i++) {
		distance =
			POND(colors[i][0] - r, r) +
			POND(colors[i][1] - g, g) +
			POND(colors[i][2] - b, b);
		if (odistance == -1 || distance < odistance) {
			odistance = distance;
			select = i;
		}
	}
	return select;
}

void render_ansi(const uint8_t* c, const uint8_t* d) {
	int fg = 0, color;

	(void)d;

	if ((color = reduce8(c[0], c[1], c[2])) == -1) return;

	printf("\x1b[%dm", color + (fg ? 30 : 40));
}

static inline int rgb(uint8_t r, uint8_t g, uint8_t b) {
	r = (r / 50.6);
	g = (g / 50.6);
	b = (b / 50.6);
	return 16 + (r * 36) + (g * 6) + b;
}

void render_256(const uint8_t* c, const uint8_t* d) {
	printf("\x1b[%d;5;%dm", 38, rgb(c[0], c[1], c[2]));
	printf("\x1b[%d;5;%dm", 48, rgb(d[0], d[1], d[2]));
}

void render_rgb(const uint8_t* c, const uint8_t* d) {
	printf("\x1b[38;2;%d;%d;%dm", c[0], c[1], c[2]);
	printf("\x1b[48;2;%d;%d;%dm", d[0], d[1], d[2]);
}

void render_greyscale(const uint8_t* c, const uint8_t* d) {
	int color1, color2, k;

	color1 = (c[0] + c[1] + c[2]) / 3;
	color2 = (d[0] + d[1] + d[2]) / 3;
	k = 231 + ((int)((float)color1 / 10.3));

	if (k < 232) k = 232;
	printf("\x1b[%d;5;%dm", 48, k); // bg

	k = 231 + ((int)((float)color2 / 10.3));

	if (k < 232) k = 232;
	printf("\x1b[%d;5;%dm", 38, k); // fg
}

void render_ascii(const uint8_t* c, const uint8_t* d) {
	const char pal[12] = " `.,-:+*%$##";
	int idx;
	float p, q;

	p = (c[0] + c[1] + c[2]) / 3;
	q = (d[0] + d[1] + d[2]) / 3;
	idx = ((p + q) / 2) / (255 / (sizeof(pal) - 1));

	printf("%c", pal[idx]);
}

stivctx_t* stiv_create(const uint8_t* buffer, size_t buffer_size, uint32_t width, uint32_t height, stiv_mode mode)
{
	stivctx_t* ctx;

	if (!(ctx = malloc(sizeof(*ctx)))) return NULL;

	switch (mode) {
	case STIV_MODE_ASCII:
		ctx->renderer = render_ascii;
		break;
	case STIV_MODE_ANSI:
		ctx->renderer = render_ansi;
		break;
	case STIV_MODE_GREY:
		ctx->renderer = render_greyscale;
		break;
	case STIV_MODE_256:
		ctx->renderer = render_256;
		break;
	default:
		ctx->renderer = render_rgb;
		break;
	}

	if (!(ctx->buffer = (uint8_t*)malloc(buffer_size))) {
		free(ctx);
		return NULL;
	}

	memcpy(ctx->buffer, buffer, buffer_size);
	ctx->buffer_size = buffer_size;
	ctx->width = width;
	ctx->height = height;

	return ctx;
}

stivctx_t* stiv_from_jpeg(const char* jpeg_filepath, uint32_t width, uint32_t height, stiv_mode mode)
{
	stivctx_t* ctx;
	uint8_t* p1, * buf;
	uint8_t** p2 = &p1;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int counter, stride;
	FILE* fd;

	(void)height;

	// Open the JPEG file
	if (!(fd = fopen(jpeg_filepath, "rb"))) {
		fprintf(stderr, "Cannot open '%s'\n", jpeg_filepath);
		return NULL;
	}

	// Initialize the JPEG decompression object with default error handling.
	memset(&cinfo, 0, sizeof(cinfo));
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	//jpeg_set_colorspace (&cinfo, JCS_RGB);
	jpeg_stdio_src(&cinfo, fd);
	jpeg_read_header(&cinfo, TRUE);

	// scale image works fine here
	cinfo.scale_num = width;
	cinfo.scale_denom = cinfo.image_width;
	width = (width * cinfo.scale_num) / cinfo.scale_denom;

	jpeg_start_decompress(&cinfo);

	// Check if the image is RGB24
	stride = cinfo.output_width * cinfo.output_components;
	if (cinfo.output_components != 3) {
		printf("Not in rgb24\n");
		return NULL;
	}

	// Allocate memory and decompress the image
	counter = 0;
	stride = cinfo.output_width * 3;
	p1 = malloc(stride);
	buf = malloc(stride * cinfo.output_height);
	p2 = &p1;
	while (cinfo.output_scanline < cinfo.output_height) {
		*p2 = p1;
		jpeg_read_scanlines(&cinfo, p2, 1);
#if STANDALONE
		write(1, p1, cinfo.output_width * 3);
#else
		memcpy(buf + counter, p1, cinfo.output_width * 3);
#endif
		counter += stride;
	}

	if (!(ctx = malloc(sizeof(*ctx)))) return NULL;

	switch (mode) {
	case STIV_MODE_ASCII:
		ctx->renderer = render_ascii;
		break;
	case STIV_MODE_ANSI:
		ctx->renderer = render_ansi;
		break;
	case STIV_MODE_GREY:
		ctx->renderer = render_greyscale;
		break;
	case STIV_MODE_256:
		ctx->renderer = render_256;
		break;
	default:
		ctx->renderer = render_rgb;
		break;
	}

	ctx->buffer = buf;
	ctx->buffer_size = stride * cinfo.output_height;
	ctx->width = cinfo.output_width;
	ctx->height = cinfo.output_height;

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	free(p1);
	fclose(fd);

	return ctx;
}

void stiv_free(stivctx_t* ctx)
{
	if (!ctx) return;

	free(ctx->buffer);
	memset(ctx, 0, sizeof(stivctx_t));
	free(ctx);
}

void stiv_display(stivctx_t* ctx) {
	if (!ctx) return;

	uint8_t* c, * d;
	uint32_t x, y;

	for (y = 0; y < ctx->height; y += 2) {
		for (x = 0; x < ctx->width; x++) {
			c = (ctx->buffer + ((y) * (ctx->width * 3)) + (x * 3));
			d = (ctx->buffer + ((y + 1) * (ctx->width * 3)) + (x * 3));
			if (d > (ctx->buffer + ctx->buffer_size)) break;
			ctx->renderer(c, d);
			if (ctx->renderer != render_ascii)
				render_ascii(c, d);
		}
		printf((ctx->renderer == render_ascii) ? "\n" : "\x1b[0m\n");
	}
}

#ifndef ENGINE_H
#define ENGINE_H

#pragma once
#define _GNU_SOURCE

#include "../lib/glad/include/glad/gl.h"
#include <EGL/egl.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <xkbcommon/xkbcommon.h>
#include "xdg-shell-client-protocol.h"

extern char *vert_src;
extern char *frag_src;

extern unsigned int width;
extern unsigned int height;

// character resolution 
typedef struct {
    unsigned int textureID;
    int width, height;
    int bearingX, bearingY;
    unsigned int advance;
} Glyph;

typedef struct Wl_Engine{
    bool running;
    bool dirty;
    unsigned int width;
    unsigned int height;
    struct wl_display *display;
    struct wl_seat *seat;
    struct wl_compositor *compositor;
    struct wl_surface *surface;
    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *top_window;
    struct xkb_context *xkb_context;
    struct xkb_keymap *keymap;
    struct xkb_state *xkb_state;

    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;
    struct wl_egl_window *egl_window;

    // Preloaded resolution for all the characters, indexed by ascii index: 65 -> 'A'.
    Glyph glyphs[128];
    unsigned int Font_VAO, Font_VBO;
    int ascender;
    int descender;
    unsigned int Font_shader;

    // RECT
    unsigned int Rect_VAO, Rect_VBO, Rect_shader;
} Wl_Engine;

void make_ortho(float *m, float l, float r, float b, float t);

unsigned int create_shader(const char *vert_path, const char *frag_path);

void init_freetype(Wl_Engine *Engine, const char *font_path, int font_size);

void render_text(Wl_Engine *Engine, const char *text, float x, float y, float scale, float r, float g, float b, float max_width) ;

void render_rect(Wl_Engine *Engine, float x, float y, float w, float h, float r, float g, float b);

void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);

void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial) ;

void xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states) ;

void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel);

void output_geometry(void *data, struct wl_output *output, int32_t x, int32_t y, int32_t physical_width, int32_t physical_height, int32_t subpixel, const char *make, const char *model, int32_t transform) ;
void output_done(void *data, struct wl_output *output) ;
void output_scale(void *data, struct wl_output *output, int32_t factor) ;
void output_mode(void *data, struct wl_output *output,
                        uint32_t flags, int32_t w, int32_t h,
                        int32_t refresh);

void global_registry_handler(
    void *data,
    struct wl_registry *registry, 
    uint32_t id,
    const char *interface, 
    uint32_t version
);

void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id);

int Init_Engine(Wl_Engine *Engine);

#endif

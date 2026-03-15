#ifndef MAIN_H
#define MAIN_H

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

typedef struct {
    unsigned int textureID;
    int width, height;
    int bearingX, bearingY;
    unsigned int advance;
} Glyph;

typedef struct Engine_Prototype{
    bool running;
    int ascender;
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
    unsigned int VAO, VBO;
    unsigned int shader;
    EGLDisplay egl_display;
    EGLSurface egl_surface;
    EGLContext egl_context;
    struct wl_egl_window *egl_window;
    Glyph glyphs[128];
} Engine_Prototype;

void make_ortho(float *m, float l, float r, float b, float t);
unsigned int create_shader(const char *vert_path, const char *frag_path);
void init_freetype(const char *font_path, int font_size);
void render_text(const char *text, float x, float y, float scale, float r, float g, float b);
void keyboard_keymap(void *data, struct wl_keyboard *keyboard,uint32_t format, int32_t fd, uint32_t size);
void keyboard_enter(void *data, struct wl_keyboard *keyboard,uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
void keyboard_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);
void keyboard_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
void key_listener(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial);
void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial);
void xdg_toplevel_configure(void *data, struct xdg_toplevel *toplevel, int32_t width, int32_t height, struct wl_array *states);
void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel);
void global_registry_handler(
    void *data, 
    struct wl_registry *registry, 
    uint32_t id,
    const char *interface, 
    uint32_t version
);
void global_registry_remover(void *data, struct wl_registry *registry, uint32_t id);


#endif

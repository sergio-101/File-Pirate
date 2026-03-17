#include "include/main.h"

static struct wl_callback_listener frame_listener = {
    .done = Frame_callback 
};

void Draw_Frame(Global_State *GState){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    for(int i = 0; i < GState->State->n; ++i){
        item _item = GState->State->dir[i];
        render_text(GState->Engine, _item.name, 0.0f, i * 40, 0.4f, 1.0f, 1.0f, 1.0f);
    }
}

void Frame_callback(void* data, struct wl_callback* wl_cb, uint callback_data){
    wl_callback_destroy(wl_cb);
    App_State *State = ((Global_State*) data)->State;
    Wl_Engine *Engine = ((Global_State*) data)->Engine;
    if(Engine->dirty){
        Draw_Frame((Global_State*)data);
        eglSwapBuffers(Engine->egl_display, Engine->egl_surface);
        Engine->dirty = false;
    }
    // next frame;
    struct wl_callback *cb = wl_surface_frame(Engine->surface);
    wl_callback_add_listener(cb, &frame_listener, data);
    wl_surface_commit(Engine->surface);
}

void Render(Global_State *GState){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    eglSwapBuffers(GState->Engine->egl_display, GState->Engine->egl_surface);

    struct wl_callback *cb = wl_surface_frame(GState->Engine->surface);
    wl_callback_add_listener(cb, &frame_listener, GState);
    wl_surface_commit(GState->Engine->surface);
}

void Load_dir(App_State *State, char *path){
    DIR *dir;
    struct dirent *entry;
    State->n = 0;
    dir = opendir(path);
    while ((entry = readdir(dir)) != NULL) {
        char *n = entry->d_name;
        if(!strcmp(".", n) || !strcmp("..", n)) continue;
        struct stat st;
        lstat(n, &st); 
        item_type t = S_ISDIR(st.st_mode)? type_dir: type_file;
        item _item = {
            .type = t
        };
        snprintf(_item.name, 128, "%s", n);
        State->dir[State->n] = _item;
        State->n++;
    }
    closedir(dir);
}

void keyboard_keymap(void *data, struct wl_keyboard *keyboard,uint32_t format, int32_t fd, uint32_t size){
    Wl_Engine *Engine = ((Global_State*)data)->Engine;
	char *keymap_string = mmap (NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	xkb_keymap_unref (Engine->keymap);
	Engine->keymap = xkb_keymap_new_from_string (Engine->xkb_context, keymap_string, XKB_KEYMAP_FORMAT_TEXT_V1, XKB_KEYMAP_COMPILE_NO_FLAGS);
	munmap (keymap_string, size);
	close (fd);
	xkb_state_unref (Engine->xkb_state);
	Engine->xkb_state = xkb_state_new (Engine->keymap);
}
void keyboard_enter(void *data, struct wl_keyboard *keyboard,uint32_t serial, struct wl_surface *surface, struct wl_array *keys){}
void keyboard_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface){}
void keyboard_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group){}
void keyboard_repeat_info(void *data, struct wl_keyboard *keyboard,int32_t rate, int32_t delay){}

void key_listener(void* data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state){
	   Global_State *GState = (Global_State*) data;
	   if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		xkb_keysym_t keysym = xkb_state_key_get_one_sym (GState->Engine->xkb_state, key+8);
		uint32_t utf32 = xkb_keysym_to_utf32 (keysym);
		if (utf32) {
           if(utf32 == 'h'){
               chdir("..");
               Load_dir(GState->State, ".");
               GState->Engine->dirty = true;
           }
		}
		else {
			char name[64];
			xkb_keysym_get_name (keysym, name, 64);
			printf ("the key %s was pressed\n", name);
		}
	}
}
int main(int argv, char *argc[]) {
    Wl_Engine Engine = { .running = true, .dirty = true };
    char *cwd = getcwd(NULL, 0);
    App_State State = { .path = cwd, .n = 0 };
    Init_Engine(&Engine);
    Global_State GState = {&Engine, &State};

    Engine.xkb_context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    struct wl_keyboard *keyboard = wl_seat_get_keyboard(Engine.seat);
    static const struct wl_keyboard_listener keyboard_callbacks = {
        .keymap      = keyboard_keymap,
        .enter       = keyboard_enter,
        .leave       = keyboard_leave,
        .key         = key_listener,
        .modifiers   = keyboard_modifiers,
    };
    wl_keyboard_add_listener(keyboard, &keyboard_callbacks, (void*) &GState);

    if ((State.dir = malloc(1024 * sizeof(item))) == NULL) {
        fprintf(stderr, "malloc");
        exit(1);
    }
    State.allocated = 1024;
    Load_dir(&State, State.path);

    Render(&GState);


    while (Engine.running) {
        wl_display_dispatch(Engine.display);
    }
    wl_display_disconnect(Engine.display);
}

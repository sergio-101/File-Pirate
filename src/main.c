#include "include/main.h"

// (todo): For Debugging Only, Delete This.
void print_stack(App_State *State){
    Path *node = State->current_path;
    while(node->prev != NULL) node = node->prev;
    while(true) {
        if(!node->next){
            break;
        }
        node = node->next;
    }
};

static struct wl_callback_listener frame_listener = {
    .done = Frame_callback 
};

void Draw_Frame(Global_State *GState){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    int X = 0, Y = 0;
    for(int i = 0; i < GState->State->n; ++i){
        Item _item = GState->State->dir[i];
        float x = X + GState->State->padding;
        float y = Y + GState->State->padding;
        if(_item.selected || GState->State->cursor == i){
            render_rect(
                GState->Engine, x, y, 
                GState->State->slot_w, 
                GState->State->slot_h, 
                200, 100, 200
            );
        }
        render_text(GState->Engine, _item.name, x + GState->State->padding, y + GState->State->padding, 0.4f, 255, 255, 255, GState->State->slot_w - GState->State->padding * 2);
        X += GState->State->slot_w;
        if(X + GState->State->slot_w > GState->Engine->width){
            X = 0;
            Y += GState->State->slot_h;
        }
    }
    render_rect(GState->Engine, 0, GState->Engine->height - 50, GState->Engine->width, 50, 100, 100, 200);
    if(GState->State->cmd){
        render_text(GState->Engine, GState->State->cmd, 0, GState->Engine->height - 50, 0.4f, 0, 0, 0, GState->Engine->width);
    }
}

void Frame_callback(void* data, struct wl_callback* wl_cb, uint callback_data){
    wl_callback_destroy(wl_cb);
    App_State *State = ((Global_State*) data)->State;
    Wl_Engine *Engine = ((Global_State*) data)->Engine;
    if(Engine->dirty){
        print_stack(State);
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


// Load content of current_path
void Load_directory(App_State *State){
    DIR *dir;
    struct dirent *entry;
    State->n = 0;
    dir = opendir(State->current_path->path);
    while ((entry = readdir(dir)) != NULL) {
        char *n = entry->d_name;
        if(!strcmp(".", n) || !strcmp("..", n)) continue;
        struct stat st;
        lstat(n, &st); 
        Item _item = {
            .type = S_ISDIR(st.st_mode)? type_dir: type_file
        };
        snprintf(_item.name, 40, "%s", n);
        State->dir[State->n] = _item;
        State->n++;
    }
    closedir(dir);
}
void Open_Directory(App_State *State, char *dir){
    Path *c_path = State->current_path;

    // Clearing if any nodes are ahead of current_path node; Makes the traversal linear;
    Path *temp = c_path->next;
    while(temp != NULL){
        free(temp->path);
        if(!temp->next){
            free(temp);
            break;
        }
        else {
            temp = temp->next;
            free(temp->prev);
        }
    }
    chdir(dir);
    char *cwd = getcwd(NULL, 0);
    Path *path = malloc(sizeof(Path));
    path->path = cwd;
    path->prev = c_path;
    path->next = NULL;
    c_path->next = path;
    State->current_path = path;
    Load_directory(State);
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
void keyboard_enter(void *data, struct wl_keyboard *keyboard,uint32_t serial, struct wl_surface *surface, struct wl_array *keys){
}
void keyboard_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface){}
void keyboard_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group){
	Global_State *GState = (Global_State*) data;
    xkb_state_update_mask(GState->Engine->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
}
void keyboard_repeat_info(void *data, struct wl_keyboard *keyboard,int32_t rate, int32_t delay){}
void key_listener(void* data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state){
	Global_State *GState = (Global_State*) data;
    bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;
    xkb_keysym_t keysym = xkb_state_key_get_one_sym (GState->Engine->xkb_state, key+8);
    char name[64];
    xkb_keysym_get_name(keysym, name, 64);
    // printf ("the key %s was pressed\n", name);
    uint32_t utf32 = xkb_keysym_to_utf32 (keysym);
    // printf ("the key %d was pressed\n", utf32);
    if(pressed){
        if(GState->State->cmd) free(GState->State->cmd);
        char key[2] = {utf32, '\0'};
        GState->State->cmd = strdup(key);
        GState->Engine->dirty = true;
        switch(utf32){
            case 'H':
                if(GState->State->current_path->prev){
                    GState->State->current_path = GState->State->current_path->prev;
                    chdir(GState->State->current_path->path);
                    Load_directory(GState->State);
                    GState->State->cursor = 0;
                    GState->Engine->dirty = true;
                }
                break;

            case 'L':
                if(GState->State->current_path->next){
                    GState->State->current_path = GState->State->current_path->next;
                    chdir(GState->State->current_path->path);
                    Load_directory(GState->State);
                    Load_directory(GState->State);
                    GState->State->cursor = 0;
                    GState->Engine->dirty = true;
                }
                break;
                
            case 'h':
                if( 0 < GState->State->cursor){
                    GState->State->cursor--;
                }
                break;

            case 'j':{
                // starts from 0 to (files_per_row - 1);
                int c_col = GState->State->cursor / GState->State->files_per_row;
                int c_row = GState->State->cursor % GState->State->files_per_row;
                c_col++;
                int next_index = c_col * GState->State->files_per_row + c_row;
                if(next_index < GState->State->n){
                    GState->State->cursor = next_index;
                }
                break;
            }

            case 'k':{
                // starts from 0 to (files_per_row - 1);
                int c_col = GState->State->cursor / GState->State->files_per_row;
                int c_row = GState->State->cursor % GState->State->files_per_row;
                c_col--;
                int next_index = c_col * GState->State->files_per_row + c_row;
                if(0 <= next_index){
                    GState->State->cursor = next_index;
                }
                break;
            }
            case 'l':
                if(GState->State->cursor < GState->State->n - 1){
                    GState->State->cursor++;
                }
                break;
            case ENTER:
                Item item = GState->State->dir[GState->State->cursor];
                if(item.type == type_dir){
                    Open_Directory(GState->State, item.name);
                    GState->State->cursor = 0;
                }

        }
    }
}
void Build_Dir_Stack(App_State *State){
    if(!State->dir_stack){
        char *cwd = getcwd(NULL, 0);
        Path *prev = NULL;
        // Back tracking from cwd
        for(;;){
            Path *path = malloc(sizeof(Path));
            char *c_path = getcwd(NULL, 0);
            path->path = c_path;
            path->next = prev;
            path->prev = NULL;
            if(prev){
                prev->prev = path;
            }
            if(!strcmp(c_path, cwd)){
                State->current_path = path;
            }
            else if(!strcmp(c_path, "/")){
                break;
            }
            prev = path;
            chdir("..");
        }
        chdir(cwd);
        free(cwd);
    }
}
int main(int argv, char *argc[]) {
    Wl_Engine Engine = { .running = true, .dirty = true };
    App_State State = { 
        .dir_stack = NULL,
        .current_path = NULL,
        .cmd = NULL,
        .cursor = 0,
        .n = 0,
        .slot_w = 150,
        .slot_h = 80, 
        .padding = 10
    };
    Init_Engine(&Engine);
    Global_State GState = {&Engine, &State};
    State.files_per_row = Engine.width / State.slot_w;
    printf("%d\n", State.files_per_row);
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

    if ((State.dir = malloc(1024 * sizeof(Item))) == NULL) {
        fprintf(stderr, "malloc");
        exit(1);
    }
    State.allocated = 1024;
    Build_Dir_Stack(&State);
    Load_directory(&State);

    Render(&GState);

    while (Engine.running) {
        wl_display_dispatch(Engine.display);
    }
    wl_display_disconnect(Engine.display);
}

#include "include/main.h"

#define CTRL 0b0001
#define SHIFT 0b0010
#define ALT 0b0100
#define PRESSED 0b1000

// (todo): For Debugging Only, Delete This.
void print_stack(App_State *State){
    Path *node = State->current_path;
    while(node->prev != NULL) node = node->prev;
    while(true) {
        if(!node->next){
            break;
        }
        node = node->next;
        printf("%s", node->path);
        if(node == State->current_path) printf("(<)\n");
        else printf("\n");
    }
};

static struct wl_callback_listener frame_listener = {
    .done = Frame_callback 
};

void Draw_Frame(Global_State *GState){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    int X = 0, Y = -GState->State->fov_y;
    for(int i = 0; i < GState->State->view_n; ++i){
        Item _item = GState->State->view_dir[i];
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
        render_text(
            GState->Engine,
            _item.name,
            x + GState->State->padding,
            y + GState->State->padding,
            0.4f, 255, 255, 255,
            GState->State->slot_w - GState->State->padding * 2
        );

        X += GState->State->slot_w;
        if(X + GState->State->slot_w > GState->Engine->width){
            X = 0;
            Y += GState->State->slot_h;
        }
    }
    render_rect(
        GState->Engine,
        0,
        GState->Engine->height - GState->State->cmd_height,
        GState->Engine->width,
        GState->State->cmd_height,
        100, 100, 200
    );
    render_text(
        GState->Engine,
        GState->State->cmd,
        0, GState->Engine->height - GState->State->cmd_height,
        0.4f, 
        0, 0, 0,
        GState->Engine->width
    );
}

void Frame_callback(void* data, struct wl_callback* wl_cb, uint callback_data){
    wl_callback_destroy(wl_cb);
    App_State *State = ((Global_State*) data)->State;
    Wl_Engine *Engine = ((Global_State*) data)->Engine;
    if(Engine->dirty){
        // print_stack(State);
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
    State->all_n = 0;
    State->view_n = 0;
    dir = opendir(State->current_path->path);
    while ((entry = readdir(dir)) != NULL) {
        if(State->all_n >= State->allocated) {
            State->view_dir = realloc(State->view_dir, State->allocated * 2);
            State->all_dir =  realloc(State->all_dir, State->allocated * 2);
        };
        char *file = entry->d_name;
        if(!strcmp(".", file) || !strcmp("..", file)) continue;
        struct stat st;
        lstat(file, &st); 
        Item _item = {
            .type = S_ISDIR(st.st_mode)? type_dir: type_file
        };
        snprintf(_item.name, 40, "%s", file);

        State->all_dir[State->all_n] = _item;
        State->view_dir[State->view_n] = _item;

        State->all_n++;
        State->view_n++;
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
    State->cursor = 0;
    State->fov_y = 0;
    Load_directory(State);
}
bool push_cursor_down(Global_State *GState){
    // starts from 0 to (files_per_row - 1);
    int c_col = GState->State->cursor / GState->State->files_per_row;
    int c_row = GState->State->cursor % GState->State->files_per_row;
    c_col++;
    int next_index = c_col * GState->State->files_per_row + c_row;
    if(next_index < GState->State->view_n){
        GState->State->cursor = next_index;
        GState->State->cursor_y_pos = c_col * GState->State->slot_h;
        int cursor_y = c_col * GState->State->slot_h;
        int cursor_y_rel_to_screen = cursor_y - GState->State->fov_y;
        if(GState->Engine->height - GState->State->cmd_height < cursor_y_rel_to_screen + GState->State->slot_h){
            GState->State->fov_y += GState->State->slot_h;
        }
        return true;
    }
}
bool push_cursor_up(Global_State *GState){
    // starts from 0 to (files_per_row - 1);
    int c_col = GState->State->cursor / GState->State->files_per_row;
    int c_row = GState->State->cursor % GState->State->files_per_row;
    c_col--;
    int next_index = c_col * GState->State->files_per_row + c_row;
    if(0 <= next_index){
        GState->State->cursor = next_index;
        int cursor_y = c_col * GState->State->slot_h;
        int cursor_y_rel_to_screen = cursor_y - GState->State->fov_y;
        if(cursor_y_rel_to_screen < 0){
            GState->State->fov_y -= GState->State->slot_h;
        }
        return true;
    }
}
void select_from_till(Global_State *GState, int from , int to){
    for(int i = 0; i < GState->State->view_n; i++){
        int included = (from <= i && i <= to); 
        Item *item = &GState->State->view_dir[i];
        if(included){
            item->selected = true;
        }
        else{
            item->selected = false;
        }
    }
}
void Update_Visual_Block(Global_State *GState){
    int from = GState->State->cursor;
    int to = GState->State->visual_start;
    if(GState->State->visual_start < GState->State->cursor){
        from = GState->State->visual_start;
        to = GState->State->cursor;
    };
    select_from_till(GState, from, to);
}

void Clear_Filter(Global_State *GState){
    GState->State->cmd_n = 0;
    memset(GState->State->cmd, '\0', GState->State->cmd_allocated);
    GState->State->view_n = GState->State->all_n;
    memcpy(GState->State->view_dir, GState->State->all_dir, sizeof(GState->State->all_dir));
}
void Update_Filter(Global_State *GState){
    GState->State->cursor = 0;
    int *view_n = &GState->State->view_n;
    int all_n = GState->State->all_n;
    Item *all_dir = GState->State->all_dir;
    Item *view_dir = GState->State->view_dir;
    char *cmd = GState->State->cmd;
    *view_n = 0;
    for(int i = 0; i < all_n; i++){
        Item item = all_dir[i];
        if(GState->State->allocated <= *view_n)
            fprintf(stderr, "somehow view files buffer is trying to reach more than it has been allocated, fix it dumbfuck.\n");

        if(strstr(item.name, cmd+1) != NULL){
            view_dir[*view_n] = item; 
            *view_n = *view_n + 1;
        }
    }    
}

void Cmd_Pop_Char(Global_State *GState){
    if(1 < GState->State->cmd_n){
        GState->State->cmd[GState->State->cmd_n - 1] = '\0';
        GState->State->cmd_n = GState->State->cmd_n - 1;
    }
}
void Cmd_Put_Char(Global_State *GState, uint32_t c){
    if(GState->State->cmd_allocated - 1 <= GState->State->cmd_n){
        GState->State->cmd = realloc(GState->State->cmd, GState->State->cmd_allocated * 2);
        memset(GState->State->cmd + GState->State->cmd_allocated, '\0', GState->State->cmd_allocated);
        GState->State->cmd_allocated *= 2;
    }
    GState->State->cmd[GState->State->cmd_n] = c;
    GState->State->cmd_n++;
}

void Perform_Action(Global_State *GState, uint32_t key, int extras_flag){
    bool pressed = extras_flag & PRESSED;
    bool shift = extras_flag & SHIFT;
    bool ctrl = extras_flag & CTRL;
    bool alt = extras_flag & ALT;
    if(GState->State->record && pressed) {
        if(key == BACKSPACE){
            Cmd_Pop_Char(GState);
            Update_Filter(GState);
        }
        else if(32 <= key && key <= 126){
            Cmd_Put_Char(GState, key);
            Update_Filter(GState);
        }
    };
    if(GState->State->mode == FILTER){
        if(pressed){
            if(key == ESCAPE){
                GState->State->record = false;
                GState->State->mode = NORMAL;
                Clear_Filter(GState);
                GState->Engine->dirty = true;
                return;
            }

            if(GState->State->record){
                if(key == ENTER){
                    GState->State->record = false;
                    Update_Filter(GState);
                }
            }
            else{
                switch(key){
                    case 'h':
                        if( 0 < GState->State->cursor){
                            GState->State->cursor--;
                        }
                        break;

                    case 'j':
                        if(push_cursor_down(GState)) GState->Engine->dirty = true;
                        break;
                    

                    case 'k':
                        if(push_cursor_up(GState)) GState->Engine->dirty = true;
                        break;

                    case 'l':
                        if(GState->State->cursor < GState->State->view_n - 1){
                            GState->State->cursor++;
                        }
                        break;

                    case ENTER:
                        Item item = GState->State->view_dir[GState->State->cursor];
                        if(item.type == type_dir){
                            Open_Directory(GState->State, item.name);
                            GState->State->mode = NORMAL;
                            Clear_Filter(GState);
                        }
                        break;
                }
            }
            GState->Engine->dirty = true;
        }
    } 
    else if(GState->State->mode == NORMAL){
        if(pressed){
            switch(key){
                case 'H':
                    if(GState->State->current_path->prev){
                        GState->State->current_path = GState->State->current_path->prev;
                        chdir(GState->State->current_path->path);
                        Load_directory(GState->State);
                        GState->State->cursor = 0;
                        GState->State->fov_y = 0;
                        GState->Engine->dirty = true;
                    }
                    break;

                case 'L':
                    if(GState->State->current_path->next){
                        GState->State->current_path = GState->State->current_path->next;
                        chdir(GState->State->current_path->path);
                        Load_directory(GState->State);
                        GState->State->cursor = 0;
                        GState->State->fov_y = 0;
                        GState->Engine->dirty = true;
                    }
                    break;

                case 'h':
                    if( 0 < GState->State->cursor){
                        GState->State->cursor--;
                        GState->Engine->dirty = true;
                    }
                    break;

                case 'j':
                    if(push_cursor_down(GState)) GState->Engine->dirty = true;
                    break;
                

                case 'k':
                    if(push_cursor_up(GState)) GState->Engine->dirty = true;
                    break;

                case 'l':
                    if(GState->State->cursor < GState->State->view_n - 1){
                        GState->State->cursor++;
                        GState->Engine->dirty = true;
                    }
                    break;

                case '/':
                    GState->State->mode = FILTER;
                    GState->State->record = true;
                    Cmd_Put_Char(GState, '>');
                    GState->Engine->dirty = true;
                    break;

                case ENTER:
                    Item item = GState->State->view_dir[GState->State->cursor];
                    if(item.type == type_dir){
                        Open_Directory(GState->State, item.name);
                        GState->Engine->dirty = true;
                    }
                    break;
            }
        }

        else{
            switch(key){
                case 'v':
                case 'V':
                    GState->State->visual_start = GState->State->cursor;
                    GState->State->view_dir[GState->State->cursor].selected = true;
                    GState->State->mode = VISUAL;
                    GState->Engine->dirty = true;
                    break;
            }
        }
    }
    else if(GState->State->mode == VISUAL){
        if(pressed){
            switch(key){
                case ESCAPE:
                    // resets the visual block; 
                    select_from_till(GState, -1, -1);
                    GState->State->mode = NORMAL;
                    GState->Engine->dirty = true;
                    break;
                case 'j':{
                    if(push_cursor_down(GState)) {
                        Update_Visual_Block(GState);                        
                        GState->Engine->dirty = true;
                    }
                    break;
                }

                case 'k':{
                    if(push_cursor_up(GState)) {
                        Update_Visual_Block(GState);                        
                        GState->Engine->dirty = true;
                    }
                    break;
                }
                case 'l':
                    if(GState->State->cursor < GState->State->view_n - 1){
                        GState->State->cursor++;
                        Update_Visual_Block(GState);                        
                        GState->Engine->dirty = true;
                    }
                    break;
                case 'h':
                    if(0 < GState->State->cursor){
                        GState->State->cursor--;
                        Update_Visual_Block(GState);                        
                        GState->Engine->dirty = true;
                    }
                    break;
            }
        }        
    }
}
void keyboard_enter(void *data, struct wl_keyboard *keyboard,uint32_t serial, struct wl_surface *surface, struct wl_array *keys){};
void keyboard_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface){};
void keyboard_repeat_info(void *data, struct wl_keyboard *keyboard,int32_t rate, int32_t delay){};
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
void keyboard_modifiers(
    void *data,
    struct wl_keyboard *keyboard,
    uint32_t serial,
    uint32_t mods_depressed,
    uint32_t mods_latched,
    uint32_t mods_locked,
    uint32_t group
)
{
	Global_State *GState = (Global_State*) data;
    xkb_state_update_mask(GState->Engine->xkb_state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
};
void key_listener(void* data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state){
	Global_State *GState = (Global_State*) data;
    bool pressed = state == WL_KEYBOARD_KEY_STATE_PRESSED;
    int shift = xkb_state_mod_name_is_active(
        GState->Engine->xkb_state,
        XKB_MOD_NAME_SHIFT,
        XKB_STATE_MODS_DEPRESSED);

    int ctrl = xkb_state_mod_name_is_active(
        GState->Engine->xkb_state,
        XKB_MOD_NAME_CTRL,
        XKB_STATE_MODS_DEPRESSED);

    int alt = xkb_state_mod_name_is_active(
        GState->Engine->xkb_state,
        XKB_MOD_NAME_ALT,
        XKB_STATE_MODS_DEPRESSED);

    uint8_t flag = 0;
    if(shift) flag |= SHIFT;
    if(ctrl) flag |= CTRL;
    if(alt) flag |= ALT;
    if(pressed) flag |= PRESSED;

    xkb_keysym_t keysym = xkb_state_key_get_one_sym (GState->Engine->xkb_state, key+8);
    uint32_t utf32 = xkb_keysym_to_utf32 (keysym);
    Perform_Action(GState, utf32, flag);
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
        .mode = NORMAL,
        .record = false,
        .dir_stack = NULL,
        .current_path = NULL,

        .cmd = NULL,
        .cmd_allocated = 1024,
        .cmd_n = 0,
        .cmd_height = 50,

        .cursor = 0,
        .cursor_y_pos = 0,
        .view_n = 0,
        .all_n = 0,
        .allocated = 1024,
        .slot_w = 150,
        .slot_h = 80, 
        .padding = 10
    };
    if ((State.cmd = malloc(State.cmd_allocated)) == NULL) {
        fprintf(stderr, "malloc cmd");
        exit(1);
    }
    memset(State.cmd, '\0', State.cmd_allocated);

    if ((State.all_dir = malloc(State.allocated * sizeof(Item))) == NULL) {
        fprintf(stderr, "malloc all dir");
        exit(1);
    }
    if ((State.view_dir = malloc(State.allocated * sizeof(Item))) == NULL) {
        fprintf(stderr, "malloc view dir");
        exit(1);
    }

    Init_Engine(&Engine);
    Global_State GState = {&Engine, &State};
    State.files_per_row = Engine.width / State.slot_w;
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

    Build_Dir_Stack(&State);
    Load_directory(&State);
    Render(&GState);

    while (Engine.running) {
        wl_display_dispatch(Engine.display);
    }
    wl_display_disconnect(Engine.display);
}

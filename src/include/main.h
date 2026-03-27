#ifndef MAIN_H
#define MAIN_H
#pragma once
#include "engine.h"
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ENTER 13
#define ESCAPE 27
#define BACKSPACE 8

typedef enum item_type {
    type_file = 0, 
    type_dir
} item_type;

typedef enum Mode {
    NORMAL = 0,
    VISUAL,
    FILTER,
} Mode;

typedef struct Item {
   item_type type; 
   bool selected;
   char name[128];
   char nickname[2];
} Item;

typedef struct Path {
    struct Path *next;
    struct Path *prev;
    char *path;
} Path;
typedef struct Vector2 {
    uint32_t x;
    uint32_t y;
} Vector2;

typedef struct App_State {
    Mode mode;
    bool quick_navigate;
    bool record;
    Path *dir_stack;
    Path *current_path;
    int files_per_row;
    int cursor;
    int visual_start;

    char *cmd;
    int cmd_height; 
    int cmd_allocated;
    int cmd_n;
    

    uint32_t allocated;
    // ^ for both buffers, shrinks and grows together for now;
    int view_n;  
    Item *view_dir;
    int all_n;  
    Item *all_dir;

    int row_behind;
    int file_capacity_on_screen;
    int rows_capacity_on_screen;
    int cols_capacity_on_screen;
    uint32_t fov_y;
    uint32_t slot_w; 
    uint32_t slot_h; 
    uint32_t padding_x;
    uint32_t padding_y;
} App_State;

typedef struct Global_State {
    Wl_Engine *Engine;
    App_State *State;
} Global_State;

void Load_directory(App_State *State);
void Frame_callback(void* data, struct wl_callback* wl_cb, uint callback_data);
void Render(Global_State *GState);

#endif

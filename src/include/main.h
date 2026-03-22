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

typedef enum item_type {
    type_file = 0, 
    type_dir
} item_type;

typedef enum Mode {
    NORMAL = 0,
    VISUAL
} Mode;

typedef struct Item {
   item_type type; 
   bool selected;
   char name[128];
} Item;

typedef struct Path {
    struct Path *next;
    struct Path *prev;
    char *path;
} Path;
typedef struct Vector2 {
    unsigned int x;
    unsigned int y;
} Vector2;

typedef struct App_State {
    Mode mode;
    Path *dir_stack;
    Path *current_path;
    int files_per_row;
    int cursor;
    int cursor_y_pos;
    int visual_start;
    char *cmd;
    unsigned int cmd_height; 
    unsigned int fov_y;
    unsigned int n;  
    unsigned int total_rows;  
    unsigned int allocated;
    unsigned int slot_w; 
    unsigned int slot_h; 
    unsigned int padding;
    Item *dir;
} App_State;

typedef struct Global_State {
    Wl_Engine *Engine;
    App_State *State;
} Global_State;

void Load_directory(App_State *State);
void Frame_callback(void* data, struct wl_callback* wl_cb, uint callback_data);
void Render(Global_State *GState);

#endif

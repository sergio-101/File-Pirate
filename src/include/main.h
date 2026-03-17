#ifndef MAIN_H
#define MAIN_H
#pragma once
#include "engine.h"
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef enum item_type {
    type_file = 0, 
    type_dir
} item_type;

typedef struct item {
   item_type type; 
   char name[128];
} item;

typedef struct App_State {
    char *path;
    unsigned int n;  
    unsigned int allocated;
    item *dir;
} App_State;

typedef struct Global_State {
    Wl_Engine *Engine;
    App_State *State;
} Global_State;

void Frame_callback(void* data, struct wl_callback* wl_cb, uint callback_data);
void Render(Global_State *GState);

#endif

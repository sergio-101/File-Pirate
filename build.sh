#!/bin/bash
mkdir build
gcc -g -o build/main src/lib/xdg-shell-protocol.c src/main.c src/ui.c src/lib/glad/src/gl.c \
    $(pkg-config --cflags --libs wayland-client wayland-egl egl freetype2 xkbcommon) \
    -lGL



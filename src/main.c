#include "include/main.h"

Engine_Prototype Engine = {
    .running = true
};

void Render(Engine_Prototype *Engine){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    render_text(Engine, "Hello, World!", 0.0f, 0.0f, 0.4f, 1.0f, 1.0f, 1.0f);
    eglSwapBuffers(Engine->egl_display, Engine->egl_surface);
}

int main(){
    Init_Engine(&Engine);
    while (Engine.running) {
        wl_display_dispatch_pending(Engine.display);
        wl_display_flush(Engine.display);
        Render(&Engine);
    }
    wl_display_disconnect(Engine.display);
}

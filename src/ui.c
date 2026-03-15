#include "include/ui.h"

void Render(Engine_Prototype *Engine){
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    render_text("Hello, World!", 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    eglSwapBuffers(Engine->egl_display, Engine->egl_surface);
}

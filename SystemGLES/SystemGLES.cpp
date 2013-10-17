#include "SystemGLES.h"
#include <iostream>
#include <unistd.h>

#include "../ConfigManager.h"
#include "../Epics.h"

static MyGLWindow* win;

void InitGL() {
    std::cout << "Starting InitGL" << std::endl;
    bcm_host_init();
    // here I create a config with RGB bit size 5,6,5 and no alpha
    EGLconfig *config = new EGLconfig();
    config->setRGBA(0,0,0,0);
    // set the depth buffer
    config->setDepth(24);

    // enable stencil buffer
    config->setAttribute(EGL_STENCIL_SIZE,1);
    // now create a new window using the default config
    win = new MyGLWindow(config);
}

void RunGL(GLApp& app) {
    app.Init();
    win->app = &app;
    while(1) {
        ConfigManager::I().MutexLock();
        Epics::I().MutexLock();
        win->paintGL();
        ConfigManager::I().MutexUnlock();
        Epics::I().MutexUnlock();
        usleep(1000); // give other threads (especially EPICS callbacks) some time
    }
    bcm_host_deinit();
}

int GetWindowWidth() {
    return win->getWidth();
}

int GetWindowHeight() {
    return win->getHeight();
}

void ReportGLError() {
    GLenum err = glGetError();
    if(err != GL_NO_ERROR) {
        // theres is no string conversion on the RPI
        std::cerr << "OpenGL Error (fix that!): " << err << std::endl;
    }
}

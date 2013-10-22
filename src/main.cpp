#include "arch.h"
#include "PiGLETApp.h"
#include "Sound.h"

int main()
{
    Sound::I().Play();
    
    // InitGL from arch.h
    InitGL();
    
    // create and start the app
    PiGLETApp app;
    RunGL(app);
}


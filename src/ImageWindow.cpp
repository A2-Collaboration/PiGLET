#include <iostream>
#include <errno.h>
#include "ImageWindow.h"
#include "TextRenderer.h"
#include "ConfigManager.h"

using namespace std;

ImageWindow::ImageWindow( WindowManager* owner, const string& title, const float xscale, const float yscale ):
    Window(owner, title, xscale, yscale),
    _url("http://auge.physik.uni-mainz.de/record/current.jpg"), // view on the Uni Mainz Campus!
    _delay(0), // see Init() for the correct default delay
    _label(this, -.95, .82, .95, .98),
    _image_ok(false),
    _running(true)
{
    cout << "ImageWindow ctor" << endl;
    _label.SetText(title);    
    _label.SetColor(dInvalidAlarm);
        
    // stuff for the image loading
    MagickWandGenesis(); // just once somewhere...also in TextRenderer
    _mw = NewMagickWand();   
    pthread_mutex_init(&_mutex_running, NULL);
    pthread_mutex_init(&_mutex_working, NULL);    
    pthread_cond_init (&_signal, NULL);
    pthread_cond_init (&_signal_delay, NULL);    
}

int ImageWindow::Init() {
    // start loading already now, and get the first result 
    // immediately (default delay is zero timespec!)
    pthread_create(&_thread, 0, &ImageWindow::start_thread, this);
    if(ApplyTexture(pthread_mutex_lock(&_mutex_running))) {
        // by default, we update the image every second
        _delay = 1000;
        pthread_mutex_unlock(&_mutex_running);
    }
    
    ConfigManager::I().addCmd(Name()+"_Delay", BIND_MEM_CB(&ImageWindow::callbackSetDelay, this));
    ConfigManager::I().addCmd(Name()+"_URL", BIND_MEM_CB(&ImageWindow::callbackSetURL, this));
    return Window::Init();
}

ImageWindow::~ImageWindow()
{
    ConfigManager::I().removeCmd(Name()+"_Delay");
    ConfigManager::I().removeCmd(Name()+"_URL");
   
    // at this point, we don't know in which state the 
    // thread is (loading an image or sleeping, or waiting in between)
    
    // we negate a possible long delay by signaling it
    pthread_mutex_lock(&_mutex_working);
    pthread_cond_signal(&_signal_delay);
    pthread_mutex_unlock(&_mutex_working);
    
    // then we stop the while loop
    pthread_mutex_lock(&_mutex_running);
    _running = false;
    pthread_cond_signal(&_signal);
    pthread_mutex_unlock(&_mutex_running);    
    
    // wait until thread has really finished    
    pthread_join(_thread, NULL);
    pthread_cond_destroy(&_signal);
    pthread_cond_destroy(&_signal_delay);    
    pthread_mutex_destroy(&_mutex_working);    
    pthread_mutex_destroy(&_mutex_running);
    
    DestroyMagickWand(_mw);
    cout << "ImageWindow dtor" << endl;
}

void ImageWindow::SetURL(const string &url)
{
    // wait until thread has finished working
    pthread_mutex_lock(&_mutex_working);
    _url = url;
    // negate the delay
    pthread_cond_signal(&_signal_delay);            
    pthread_mutex_unlock(&_mutex_working);
}



string ImageWindow::callbackSetDelay(const string &arg)
{
    int d = atoi(arg.c_str());
    // 20ms should be minimum
    if(d>20) {
        pthread_mutex_lock(&_mutex_working);
        _delay = d;        
        // negate the delay
        pthread_cond_signal(&_signal_delay);        
        pthread_mutex_unlock(&_mutex_working);
        return "";
    }
    else {  
        return "Delay shorter than 20 ms";
    }
}

string ImageWindow::callbackSetURL(const string &arg)
{
    SetURL(arg);
    return ""; // always works, but nobody knows if an image gets displayed
}

bool ImageWindow::ApplyTexture(int state)
{
    if(state==0) {
        // mutex was free and now locked, so data is available
        if(_image_ok) {
            TextRenderer::Image2Texture(_mw, _tex);
            //cout << "Image loaded..." << endl;
            _image_ok = false;
            _label.SetColor(dTextColor);
        }
        else {
            _label.SetColor(dMajorAlarm); // show that something is wrong
        }
        // re-create MagickWand to prevent memory leak...is there a better way?
        DestroyMagickWand(_mw);
        _mw = NewMagickWand();        
        pthread_cond_signal(&_signal);
        return true;
    }
    else if(state != EBUSY) {
        cout << "Something wrong with image thread..." << endl;
    }      
    return false;
}

void ImageWindow::Draw()
{    
    if(ApplyTexture(pthread_mutex_trylock(&_mutex_running))) {
        pthread_mutex_unlock(&_mutex_running);
    }
             
    glPushMatrix();
    glScalef(.95,.83,.1);
    glTranslatef(.0,-.07,.0);    
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    
    _tex.Activate();
    
    Rectangle::unit.Draw( GL_TRIANGLE_FAN );
    
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();
    
    _label.Draw();
}

void ImageWindow::do_work()
{
    pthread_mutex_lock(&_mutex_running);
    while(_running) {
        pthread_mutex_lock(&_mutex_working);
        _image_ok = MagickReadImage(_mw, _url.c_str());
               
        // first we wait with a condition timed wait, 
        // serves as a usleep 
        // but can be cancelled via _signal_delay 
        // to get the data quicker
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts); // the current time
        long sec =_delay / 1000;
        long ms = _delay - sec*1000;
        
        // perform the addition
        ts.tv_nsec+=ms*1000000;
        
        // adjust the time
        ts.tv_sec+=ts.tv_nsec/1000000000 + sec;
        ts.tv_nsec=ts.tv_nsec%1000000000;    
        
        pthread_cond_timedwait(&_signal_delay, &_mutex_working, &ts);   
        pthread_mutex_unlock(&_mutex_working); // ETIMEDOUT EBUSY       
        
        // then we wait that somebody has used the data
        pthread_cond_wait(&_signal, &_mutex_running);
    }
    pthread_mutex_unlock(&_mutex_running);      
    pthread_exit(0);
}




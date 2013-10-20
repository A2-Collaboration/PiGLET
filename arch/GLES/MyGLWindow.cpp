/*
  Copyright (C) 2012 Jon Macey

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received m_a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MyGLWindow.h"

#include <iostream>
#include <time.h>

using namespace std;

MyGLWindow::MyGLWindow(EGLconfig *_config) : EGLWindow(_config), frames(0)
{
	std::cout<<"My GL Window Ctor\n";
	makeSurface();
    clock_gettime(CLOCK_MONOTONIC, &start_fps);
}

MyGLWindow::~MyGLWindow()
{

}

void MyGLWindow::initializeGL()
{
    cout << "init GL" << endl;
}

/**
 * @brief Calculate the differnce in seconds of two timespec values
 * @param start The first (earlier) time point
 * @param stop The later time point
 * @return difference in seconds
 */
float time_difference( const timespec* start, const timespec* stop ) {
    float time = stop->tv_sec - start->tv_sec + (stop->tv_nsec - start->tv_nsec) * 1E-9;
    return time;
}



void MyGLWindow::paintGL()
{


    app->Draw();

	glFlush();
	glFinish();
	swapBuffers();

    ++frames;
    if( frames == 200 ) {
        timespec stop;
        clock_gettime(CLOCK_MONOTONIC, &stop);
        float diff = time_difference( &start_fps, &stop );

        cout << "fps: " << frames / diff << endl;
        frames = 0;
        clock_gettime(CLOCK_MONOTONIC, &start_fps);
    }


}

const vec2_t MyGLWindow::square[4] = { {1,1},{-1,1},{-1,-1},{1,-1} };
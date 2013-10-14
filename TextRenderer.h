#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <string>
#include <wand/magick_wand.h>
#include "system.h"
#include "StopWatch.h"
#include "Structs.h"
#include "GLTools.h"


class TextRenderer {
protected:
    MagickWand *mw;
    StopWatch _watch;

private:
    TextRenderer();
    virtual ~TextRenderer();
    TextRenderer(TextRenderer const& copy);            // Not Implemented
    TextRenderer& operator=(TextRenderer const& copy); // Not Implemented

public:

    static TextRenderer& I() {
        static TextRenderer instance;
        return instance;
    }

    void Text2Texture(const GLuint texhandle, const std::string& text );
    void Text2Texture(const GLuint texhandle, const std::string& text, const int w, const int h);
    void Text2Texture2(const GLuint texhandle, const std::string &text, const int w, const int h, float& texw, float& texh);
    void Text2Texture3(const GLuint texhandle, const std::string &text, float& texw, float& texh);


};

class Rectangle {
private:
    float _width;
    float _height;
    vec2_t _center;
    vec2_t _vertices[4];

    void _update_vertices();

public:
    Rectangle( const float x1, const float y1, const float x2, const float y2 );
    Rectangle( const vec2_t& center, const float width, const float height);

    virtual ~Rectangle() {}

    float Width() const { return _width; }
    float Height() const { return _height; }

    const vec2_t& Center() const { return _center; }

    void SetCenter( const vec2_t& center );
    void SetWidth( const float width );
    void SetHeight( const float height );

    void Draw( GLenum mode );

};

class TextLabel: public Rectangle {
private:

    std::string _text;      // the text to draw
    GLuint      _texture;   // texture handle
    Color       _color;     // color for the text
    Rectangle   _box;       // the actual drawing box, always smaller than the user defined text rectangle
    vec2_t _texcoords[4];      // texture coordinates

public:

    TextLabel(const float x1, const float y1,const float x2, const float y2);

    virtual ~TextLabel();

    void Draw( GLenum mode=GL_TRIANGLE_FAN );

    void SetText( const std::string& text );
    const std::string& GetText() const { return _text; }
    void SetColor( const Color& c ) { _color = c; }
    Color GetColor() const { return _color; }
};

class NumberLabel {
private:
    static GLuint _textures[10];
    static vec2_t _texcoords[4];
    static unsigned int _num_objetcs;
    static Rectangle r;
    void _maketextures();

    Color _color;

public:
    NumberLabel(): _color(1,1,1) {}
    virtual ~NumberLabel();

    void Draw( int i );
    void Init() { _maketextures(); }

    void SetColor( const Color& c ) { _color = c; }
    Color GetColor() const { return _color; }
};


#endif

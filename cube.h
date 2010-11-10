// File: cube.h
// Written by Joshua Green

#ifndef CUBE_H
#define CUBE_H

#include "vectXf.h"

class cube {
  private:
    vect3f _bottom_left;
    float _width;
    vect3f _color, _highlight_color;
    mutable bool _solid;

    enum FACE {
      FRONT, RIGHT, BACK, LEFT, TOP, BOTTOM
    };

    struct side {
        vect3f bottom_left, bottom_right, top_right, top_left;
        mutable bool solid;
        vect3f highlight_color;
        FACE face;

        side() : solid(false) { }

        void initialize(const vect3f& b_left, float width, FACE s);
        bool contains_point(const vect3f& point) const;
        void draw() const;
    } _front, _right, _back, _left, _top, _bottom;

    void _set_solid(bool t=true) const;

  public:
    void set_solid(bool t=true);
    void set_width(float w);
    void set_pos(const vect3f& p);
    void initialize(const vect3f p, float w);
    void set_color(const vect3f& c);
    void set_highlight(const vect3f& c);
    bool contains_point(const vect3f& point) const;

    void draw(const vect3f* const pointer_pos=0) const;
};

#endif

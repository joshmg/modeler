// File: cube.cpp
// Written by Joshua Green

#include "cube.h"
#include "vectXf.h"

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

void cube::_set_solid(bool t) const {
  _solid = t;

  _front.solid = _solid;
  _right.solid = _solid;
  _back.solid = _solid;
  _left.solid = _solid;
  _top.solid = _solid;
  _bottom.solid = _solid;
}

void cube::set_solid(bool t) { _set_solid(t); }

void cube::set_width(float w) {
  _width = w;
  initialize(_bottom_left, _width);
}

void cube::set_pos(const vect3f& p) {
  _bottom_left = p;
  initialize(_bottom_left, _width);
}

void cube::initialize(const vect3f p, float w) {
  _width = w;
  _bottom_left = p;

  _front.initialize(p+vect3f(0.0, 0.0, _width), _width, FRONT);
  _right.initialize(p+vect3f(_width, 0.0, 0.0), _width, RIGHT);
  _back.initialize(p, _width, BACK);
  _left.initialize(p+vect3f(0.0, 0.0, 0.0), _width, LEFT);
  _top.initialize(p+vect3f(0.0, _width, 0.0), _width, TOP);
  _bottom.initialize(p+vect3f(0.0, 0.0, 0.0), _width, BOTTOM);
}

void cube::set_color(const vect3f& c) { _color = c; }

void cube::set_highlight(const vect3f& c, float t) {
  _highlight_color = c;
  _translucency = t;
  _front.highlight_color = c;
  _front.translucency = t;
  _right.highlight_color = c;
  _right.translucency = t;
  _back.highlight_color = c;
  _back.translucency = t;
  _left.highlight_color = c;
  _left.translucency = t;
  _top.highlight_color = c;
  _top.translucency = t;
  _bottom.highlight_color = c;
  _bottom.translucency = t;
}

bool cube::contains_point(const vect3f& point) const {
  return (_front.contains_point(point) && _top.contains_point(point));
}

void cube::draw(const vect3f* const pointer_pos) const {
  bool restore_solid = _solid;

  bool highlight = false;
  if (pointer_pos != 0) highlight = true;

  glColor3f(_color.x, _color.y, _color.z);

  if (highlight) {
    if (contains_point(*pointer_pos)) {
      glColor3f(_highlight_color.x, _highlight_color.y, _highlight_color.z);
      _set_solid();
    }
  }

  glPushMatrix();
  _front.draw();
  _right.draw();
  _back.draw();
  _left.draw();
  _top.draw();
  _bottom.draw();
  glPopMatrix();

  _set_solid(restore_solid);
}





// **** begin class 'side' definitions ****



void cube::side::initialize(const vect3f& b_left, float width, FACE s) {
  face = s;
  bottom_left = b_left;

  switch(face) {
    case FRONT: case BACK: {
      bottom_right = bottom_left + vect3f(width, 0.0, 0.0);
      top_right = bottom_left + vect3f(width, width, 0.0);
      top_left = bottom_left + vect3f(0.0, width, 0.0);
    } break;
    case RIGHT: case LEFT: {
      bottom_right = bottom_left + vect3f(0.0, 0.0, width);
      top_right = bottom_left + vect3f(0.0, width, width);
      top_left = bottom_left + vect3f(0.0, width, 0.0);
    } break;
    case TOP: case BOTTOM: {
      bottom_right = bottom_left + vect3f(width, 0.0, 0.0);
      top_right = bottom_left + vect3f(width, 0.0, width);
      top_left = bottom_left + vect3f(0.0, 0.0, width);
    } break;
    default: {} break;
  }
}

bool cube::side::contains_point(const vect3f& point) const {
  switch(face) {
    case FRONT: case BACK: {
      return (bottom_left.x <= point.x && bottom_right.x >= point.x) &&
             (bottom_left.y <= point.y && top_right.y >= point.y);
    } break;
    case RIGHT: case LEFT: {
      return (bottom_left.z <= point.z && bottom_right.z >= point.z) &&
             (bottom_left.y <= point.y && top_right.y >= point.y);
    } break;
    case TOP: case BOTTOM: {
      return (bottom_left.x <= point.x && bottom_right.x >= point.x) &&
             (bottom_left.z <= point.z && top_right.z >= point.z);
    } break;
    default: {return false;} break;
  }
}

void cube::side::draw() const {
  if (solid) {
    glBegin(GL_QUADS);
    glColor4f(highlight_color.x, highlight_color.y, highlight_color.z, translucency);

    #ifndef USE_GL_COLOR_MATERIAL
      glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, glvect4f(highlight_color.x, highlight_color.y, highlight_color.z, translucency));
    #endif

  }
  else glBegin(GL_LINE_LOOP);

  // specifiy correct normal:
  switch(face) {
    case FRONT: { glNormal3f(0.0, 0.0, 1.0); } break;
    case RIGHT: { glNormal3f(1.0, 0.0, 0.0); } break;
    case BACK: { glNormal3f(0.0, 0.0, -1.0); } break;
    case LEFT: { glNormal3f(-1.0, 0.0, 0.0); } break;
    case TOP: { glNormal3f(0.0, 1.0, 0.0); } break;
    case BOTTOM: { glNormal3f(0.0, -1.0, 0.0); } break;
  }

  // draw with the correct face pointing outwards
  switch(face) {
    case FRONT: case LEFT: case BOTTOM: {
      glVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
      glVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
      glVertex3f(top_right.x, top_right.y, top_right.z);
      glVertex3f(top_left.x, top_left.y, top_left.z);
    } break;
    case BACK: case RIGHT: case TOP: {
      glVertex3f(bottom_right.x, bottom_right.y, bottom_right.z);
      glVertex3f(bottom_left.x, bottom_left.y, bottom_left.z);
      glVertex3f(top_left.x, top_left.y, top_left.z);
      glVertex3f(top_right.x, top_right.y, top_right.z);
    } break;
  }
  glEnd();
}

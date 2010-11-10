// File: model3d.cpp
// Written by Joshua Green

#include "model3d.h"
#include "vectXf.h"
#include "fileio/fileio.h"
#include "str/str.h"
#include <vector>
#include <string>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>
using namespace std;

void model3d::_initialize() {
  _facets.push_back(vector<int>());

  _draw_mode = GL_POLYGON;
  _pos = vect3f(0.0f, 0.0f, 0.0f);
  
  _orientation = 0.0f;
  _new_orientation = _orientation;
  _speed = 1.5f;
  _axis = vect3f(0.0, 1.0, 0.0);
  _smart_rotate = true;
  _anchored = false;
  _child_animate_flag = true;
}

int model3d::_get_facet_id(const vect3f& point) const {
  for (int i=0;i<_coordinates.size();i++) if (_coordinates[i] == point) return i;
  return -1;
}

template <typename T> bool model3d::_in_bounds(const int* const indices, const vector<vector<T>>& vect) const {
  if (indices[0] < 0 || indices[1] < 0) return false;
  return (indices[0] < vect.size() && indices[1] < vect[indices[0]].size());
}

model3d::model3d() { _initialize(); }

model3d::model3d(const vector<vect3f>& coordinates, const vector<vector<int>>& facets, const vector<vector<vect3f>>* const colors) {
  _initialize();

  _coordinates = coordinates;
  _facets = facets;

  if (colors == 0) {
    for (int i=0;i<_facets.size();i++) {
      vector<vect3f> colors_list;
      for (int j=0;j<_facets[i].size();j++) {
        colors_list.push_back(DEFAULT_COLOR());
      }
      _facet_colors.push_back(colors_list);
    }
  }
  else _facet_colors = *colors;
}

vector<vect3f> model3d::get_coordinates() const { return _coordinates; }

vector<vector<int>> model3d::get_facets() const { return _facets; }

vector<vector<vect3f>> model3d::get_facet_colors() const { return _facet_colors; }

GLenum model3d::get_draw_mode() const { return _draw_mode; }

void model3d::set_draw_mode(GLenum draw_mode) { _draw_mode = draw_mode; }

void model3d::vertex_color(int* vertex_id, const vect3f& color) {
  if (_in_bounds(vertex_id, _facet_colors)) _facet_colors[vertex_id[0]][vertex_id[1]] = color;
}

void model3d::add_vertex(const vect3f& point, const vect3f& color) {
  int facet_id(_get_facet_id(point));
  if (facet_id < 0) { // vertex doesn't exist yet
    facet_id = _coordinates.size();
    _coordinates.push_back(point);
  }
  _facets.back().push_back(facet_id);
  _facet_colors.back().push_back(color);
}

void model3d::remove_vertex(int* vertex_id) {
  if (_in_bounds(vertex_id, _facets)) {
    _facets[vertex_id[0]].erase(_facets[vertex_id[0]].begin()+vertex_id[1]);
    _facet_colors[vertex_id[0]].erase(_facet_colors[vertex_id[0]].begin()+vertex_id[1]);
  }
}

void model3d::push_face() {
  _facets.push_back(vector<int>());
  _facet_colors.push_back(vector<vect3f>());
}

void model3d::pop_face() {
  _facets.pop_back();
  _facet_colors.pop_back();
}

void model3d::save(string& filename) const {
  fileio save_file;
  if (filename.length() == 0) {
    filename = "model_0";
    int suffix = 0;
    save_file.open(filename, "r");
    while (save_file.is_open()) {
      save_file.close();

      string temp = "model_";
      temp += itos(suffix);
      suffix++;
      filename = temp;
      save_file.open(filename, "r");
    }
  }
  save_file.open(filename, "w");

  // file header
  save_file.write(SAVE_FILE_HEADER());

  // coordinate data
  for (int i=0;i<_coordinates.size();i++) {
    string data(_coordinates[i].to_string());
    save_file.write(data);
  }

  save_file.write("::");

  // facet data
  for (int i=0;i<_facets.size();i++) {
    string data("{");
    for (int j=0;j<_facets[i].size();j++) {
      data += itos(_facets[i][j]);
      if (j != _facets[i].size()-1) data += ", ";
    }
    data += "}";
    save_file.write(data);
  }

  save_file.write("::");

  // color data
  for (int i=0;i<_facet_colors.size();i++) {
    string data("{");
    for (int j=0;j<_facet_colors[i].size();j++) {
      data += _facet_colors[i][j].to_string();
      if (j != _facet_colors[i].size()-1) data += "; ";
    }
    data += "}";
    save_file.write(data);
  }

  save_file.close();
}

bool model3d::load(const string& filename) {
  _coordinates.clear();
  _facets.clear();
  _facet_colors.clear();

  fileio save_file;
  save_file.open(filename, "r");
  if (!save_file.is_open()) return false;

  // check for valid header
  if (save_file.read(SAVE_FILE_HEADER().length()) != SAVE_FILE_HEADER()) return false;

  // coordinate data
  string data = save_file.read(-1, "::");
  vector<string> point_strings(explode(data, ")", -1));
  for (int i=0;i<point_strings.size();i++) {
    point_strings[i].erase(point_strings[i].begin()); // remove the '('
    vector<string> coordinate_strings(explode(point_strings[i], ",", -1));
    _coordinates.push_back(vect3f(atof(coordinate_strings[0].c_str()), atof(coordinate_strings[1].c_str()), atof(coordinate_strings[2].c_str()))); // create and insert point
  }

  // facet data
  data = save_file.read(-1, "::");
  vector<string> facet_list_list(explode(data, "}", -1)); // removes the last brace
  for (int i=0;i<facet_list_list.size();i++) {
    facet_list_list[i].erase(facet_list_list[i].begin()); // remove the first brace
    vector<string> facet_list(explode(facet_list_list[i], ", ", -1));
    vector<int> facet_int_list;
    for (int j=0;j<facet_list.size();j++) {
      facet_int_list.push_back(atoi(facet_list[j].c_str()));
    }
    _facets.push_back(facet_int_list);
  }

  // color data
  data = save_file.read(-1);
  vector<string> color_list_list(explode(data, "}", -1)); // removes the last brace
  for (int i=0;i<color_list_list.size();i++) {
    color_list_list[i].erase(color_list_list[i].begin()); // remove the first brace
    vector<string> color_list(explode(color_list_list[i], "; ", -1));
    vector<vect3f> color_vect3f_list;
    for (int j=0;j<color_list.size();j++) {
      color_vect3f_list.push_back(vect3f().from_string(color_list[j]));
    }
    _facet_colors.push_back(color_vect3f_list);
  }

  return true;
}

void model3d::set_pos(const vect3f& pos) { _pos = pos; }

void model3d::add_submodel(const model3d& child) { _sub_models.push_back(child); }

void model3d::anchor(bool t) { _anchored = t; }

void model3d::set_axis(const vect3f& axis) { _axis = axis; }

void model3d::set_orientation(float theta) {
  if (_smart_rotate) {
    while (theta > 360.0f) theta -= 360;
    while (theta < 0.0f) theta += 360;
  }
  _new_orientation = theta;
}

void model3d::enable_smart_rotate(bool t) { _smart_rotate = t; }

void model3d::set_speed(float s) { _speed = s; }

void model3d::toggle_child_animations() { _child_animate_flag = !_child_animate_flag; }

void model3d::operator++(int) {
  if (_smart_rotate) {
    // clamp orientation values:
    while (_orientation > 360.0f) _orientation -= 360;
    while (_orientation < 0.0f) _orientation += 360;

    if (_orientation > 180.0f && _new_orientation < 180.0f) _orientation += _speed;     // pass through 4th quad counter-clockwise
    else if (_orientation < 90.0f && _new_orientation > 180.0f) _orientation -= _speed; // pass through 4th quad clockwise
    else {
      if (_new_orientation > _orientation) _orientation += _speed;
      if (_new_orientation < _orientation) _orientation -= _speed;
    }
  }
  else if (_orientation != _new_orientation) _orientation += _speed;

  if (_child_animate_flag) for (int i=0;i<_sub_models.size();i++) _sub_models[i]++; // maintain sub models
}

void model3d::draw() const {
  glPushMatrix();

  if (!_anchored) {
    glTranslatef(_pos.x, _pos.y, _pos.z);
    glRotatef(_orientation, _axis.x, _axis.y, _axis.z);
    glTranslatef(-_pos.x, -_pos.y, -_pos.z);
  }
  glTranslatef(_pos.x, _pos.y, _pos.z);

  for (int i=0;i<_facets.size();i++) { // ...for each face
    glBegin(_draw_mode);
    glColor3f(((float)i+1)/_facets.size(), ((float)i+1)/_facets.size(), ((float)i+1)/_facets.size());
    for (int j=0;j<_facets[i].size();j++) { // ...for each vertex
      // _facets[i][j] is the index which corresponds with _coordinates.
      // _coordinates[index] contains a vertex3f struct containing x,y,z coordinates

      // enable color
      int facet_indices[2]; facet_indices[0]=i; facet_indices[1]=j;
      if (_in_bounds(facet_indices, _facet_colors)) {
        // aliasing: c = the vect3f within _facet_colors
        const vect3f* const c = &(_facet_colors[facet_indices[0]][facet_indices[1]]);
        glColor3f((*c).x, (*c).y, (*c).z);
      }

      glVertex3f(_coordinates[_facets[i][j]].x, _coordinates[_facets[i][j]].y, _coordinates[_facets[i][j]].z);
    }
    glEnd();
  }

  glPopMatrix();

  glPushMatrix();

  glTranslatef(_pos.x, _pos.y, _pos.z);
  glRotatef(_orientation, _axis.x, _axis.y, _axis.z);
  glTranslatef(-_pos.x, -_pos.y, -_pos.z);

  glTranslatef(_pos.x, _pos.y, _pos.z);
  for (int i=0;i<_sub_models.size();i++) _sub_models[i].draw(); // draw sub_models

  glPopMatrix();
}

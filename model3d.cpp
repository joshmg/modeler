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

#include <iostream>
using namespace std;

void model3d::_initialize() {
  _facet_data.push_back(vector<facet>());

  _vertex_count = 0;
  _need_normals = false;

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

// returns the index of the specified point if it exists within _coordinates.
// if it does not exist, -1 is returned.
int model3d::_get_facet_id(const vect3f& point) const {
  for (int i=0;i<_coordinates.size();i++) if (_coordinates[i] == point) return i;
  return -1;
}

// checks if the two dimensional array contains a valid set of indices within the specified two dimensional vector
template <typename T> bool model3d::_in_bounds(const int* const indices, const vector<vector<T>>& vect) const {
  if (indices[0] < 0 || indices[1] < 0) return false;
  return (indices[0] < vect.size() && indices[1] < vect[indices[0]].size());
}

void model3d::_calculate_normals() const {
  if (_facet_data.back().size() < 3) return; // need a plane to calculate normals

  int size = _facet_data.back().size();
  for (int i=0;i<size;i++) {
    const vect3f* const a = &(_coordinates[(((_facet_data.back())[mod(i+0, size)]).id)]);
    const vect3f* const b = &(_coordinates[(((_facet_data.back())[mod(i+1, size)]).id)]);
    const vect3f* const c = &(_coordinates[(((_facet_data.back())[mod(i+2, size)]).id)]);
    ((_facet_data.back())[i]).normal = ((*b)-(*a)).cross((*c)-(*b));
    ((_facet_data.back())[i]).normal.normalize();
  }
}

model3d::model3d() { _initialize(); }

model3d::model3d(const vector<vect3f>& coordinates, const vector<vector<facet>>& facets) {
  _initialize();

  _coordinates = coordinates;
  _facet_data = facets;

  for (int i=0;i<_facet_data.size();i++) {
    _vertex_count += _facet_data[i].size();
  }
}

void model3d::clear() { 
  _coordinates.clear();
  _facet_data.clear();

  _sub_models.clear();

  _initialize();
}

vector<vect3f> model3d::get_coordinates() const { return _coordinates; }

const vector<vect3f>* const model3d::get_coordinates_ptr() const { return &_coordinates; }

vector<vector<facet>> model3d::get_facet_data() const { return _facet_data; }

const vector<vector<facet>>* const model3d::get_facet_data_ptr() const { return &_facet_data; }

GLenum model3d::get_draw_mode() const { return _draw_mode; }

void model3d::set_draw_mode(GLenum draw_mode) { _draw_mode = draw_mode; }

// sets a specific facet color (facet referenced by two dimensional indices)
void model3d::set_vertex_color(int* vertex_id, const vect3f& color) {
  if (_in_bounds(vertex_id, _facet_data)) _facet_data[vertex_id[0]][vertex_id[1]].color = color;
}

vect3f model3d::get_vertex_color(int* vertex_id) const {
  if (_in_bounds(vertex_id, _facet_data)) return _facet_data[vertex_id[0]][vertex_id[1]].color;
  return DEFAULT_COLOR;
}

// appends a vertex to the object's current face vector
index2d model3d::add_vertex(const vect3f& point, const vect3f& color, const vect3f* const normal) {
  int facet_id = _get_facet_id(point);
  if (facet_id < 0) { // vertex doesn't exist yet
    facet_id = _coordinates.size();
    _coordinates.push_back(point);
  }

  // set flag to calculate normals on face push or save:
  if (normal == 0) {
    _need_normals = true;
    _facet_data.back().push_back(facet(facet_id, color));
  }
  else _facet_data.back().push_back(facet(facet_id, color, *normal));

  _vertex_count++;

  return index2d(_facet_data.size()-1, _facet_data.back().size()-1);
}

void model3d::remove_vertex(const index2d& vertex_id) {
  if (_in_bounds(vertex_id, _facet_data)) {
    _facet_data[vertex_id[0]].erase(_facet_data[vertex_id[0]].begin()+vertex_id[1]);
  }
}

void model3d::push_face() {
  if (_facet_data.back().size() > 0) {
    if (_need_normals) _calculate_normals(); // calculate normals if they're undefined
    _facet_data.push_back(vector<facet>()); // only add a face if the current face has a facet
  }
  _need_normals = false;
}

void model3d::pop_face() {
  if (_facet_data.size() > 1) _facet_data.pop_back();
  else if (_facet_data.size() == 1) _facet_data.back().clear();
  _need_normals = false;
}

int model3d::vertex_count() const { return _vertex_count; }

void model3d::save(string& filename) const {
  if (_need_normals) _calculate_normals();

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

  string color_data, normal_data;

  // facet data
  for (int i=0;i<_facet_data.size();i++) {
    string data("{");
    color_data += "{";
    normal_data += "{";
    for (int j=0;j<_facet_data[i].size();j++) {
      data += itos(_facet_data[i][j].id);
      color_data += (_facet_data[i][j].color).to_string();
      normal_data += (_facet_data[i][j].normal).to_string();
      if (j != _facet_data[i].size()-1) {
        data += ", ";
        color_data += "; ";
        normal_data += "; ";
      }
    }
    data += "}";
    color_data += "}";
    normal_data += "}";
    save_file.write(data);
  }

  save_file.write("::");

  // color data
  save_file.write(color_data);
  save_file.write("::");
  
  // normal data
  save_file.write(normal_data);

  save_file.close();
}

bool model3d::load(const string& filename) {
  _coordinates.clear();
  _facet_data.clear();

  _initialize();

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
  _facet_data.clear(); // remove the first vector element because one is added automatically during the load
  vector<string> facet_list_list(explode(data, "}", -1)); // removes the last brace
  for (int i=0;i<facet_list_list.size();i++) {
    facet_list_list[i].erase(facet_list_list[i].begin()); // remove the first brace
    vector<string> facet_str_list(explode(facet_list_list[i], ", ", -1));
    vector<facet> facet_list;
    for (int j=0;j<facet_str_list.size();j++) {
      facet_list.push_back(facet(atoi(facet_str_list[j].c_str()), DEFAULT_COLOR));
      _vertex_count++;
    }
    _facet_data.push_back(facet_list);
  }

  // color data
  data = save_file.read(-1, "::");
  vector<string> color_list_list(explode(data, "}", -1)); // removes the last brace
  for (int i=0;i<color_list_list.size();i++) {
    color_list_list[i].erase(color_list_list[i].begin()); // remove the first brace
    vector<string> face_color_list(explode(color_list_list[i], "; ", -1));
    for (int j=0;j<face_color_list.size();j++) {
      _facet_data[i][j].color = vect3f().from_string(face_color_list[j]);
    }
  }

  // normal data
  data = save_file.read(-1, "::");
  vector<string> normal_list_list(explode(data, "}", -1)); // removes the last brace
  for (int i=0;i<normal_list_list.size();i++) {
    normal_list_list[i].erase(normal_list_list[i].begin()); // remove the first brace
    vector<string> face_normal_list(explode(normal_list_list[i], "; ", -1));
    for (int j=0;j<face_normal_list.size();j++) {
      _facet_data[i][j].normal = vect3f().from_string(face_normal_list[j]);
    }
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

  for (int i=0;i<_facet_data.size();i++) { // ...for each face
    glBegin(_draw_mode);
    for (int j=0;j<_facet_data[i].size();j++) { // ...for each vertex
      // _facets[i][j] is the index which corresponds with _coordinates.
      // _coordinates[index] contains a vertex3f struct containing x,y,z coordinates

      // enable color
      // aliasing: c = the vect3f within _facet_colors
      const vect3f* const c = &(_facet_data[i][j].color);

      glColor3f((*c).x, (*c).y, (*c).z);

      #ifndef USE_GL_COLOR_MATERIAL
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, glvect4f((*c).x, (*c).y, (*c).z, 1.0));
      #endif
      
      glNormal3f(_facet_data[i][j].normal.x, _facet_data[i][j].normal.y, _facet_data[i][j].normal.z);
      glVertex3f(_coordinates[_facet_data[i][j].id].x, _coordinates[_facet_data[i][j].id].y, _coordinates[_facet_data[i][j].id].z);
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

void model3d::face_resolution(int polygon_count) {
  if (_facet_data.back().size() < 3 || polygon_count < 2) return;

  vector<facet> face_facets = _facet_data.back();
  vector<vect3f> face_points;
  
  for (int i=0;i<face_facets.size();i++) face_points.push_back(_coordinates[face_facets[i].id]);

  _facet_data.back().clear();

  vect3f* anchor_point = &face_points[0];
  facet* anchor_facet = &face_facets[0];

  for (int i=2;i<face_points.size();i++) {
    vect3f* wall_point = &face_points[i-1];
    facet* wall_facet = &face_facets[i-1];
    vect3f step = (face_points[i]-(*wall_point))/(float)polygon_count;
    vect3f color_step = (face_facets[i].color-(*wall_facet).color)/(float)polygon_count;

    if (*wall_point == *anchor_point || (*wall_point)+(step*polygon_count) == *anchor_point) {
      (*wall_point) += step*polygon_count;
      continue;
    }

    for (int j=0;j<polygon_count;j++) {
      push_face();
      add_vertex(*anchor_point, (*anchor_facet).color);
      add_vertex(*wall_point, (*wall_facet).color);
      (*wall_point) += step;
      (*wall_facet).color += color_step;
      add_vertex(*wall_point, (*wall_facet).color);
    }
  }
}


// *** BEGIN FACET CLASS DEFINITIONS ***

facet::facet() {
  id = -1;
  color = DEFAULT_COLOR;
  normal = vect3f(0.0, 0.0, 1.0);
}

facet::facet(int _id, const vect3f& _color, const vect3f& _normal)
            : id(_id), color(_color), normal(_normal) { }


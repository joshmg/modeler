// File: model3d.h
// Written by Joshua Green

#ifndef MODEL3D_H
#define MODEL3D_H

#include "vectXf.h"
#include <vector>
#include <string>

#include <GL/gl.h>

const vect3f DEFAULT_COLOR(1.0f, 0.0f, 1.0f);

struct facet {
  int id;
  vect3f color;
  mutable vect3f normal;

  facet();
  facet(int _id, const vect3f& _color, const vect3f& _normal=vect3f(0.0, 0.0, 1.0));
};

struct index2d {
  int data[2];

  index2d() { clear(); }
  index2d(int a, int b) { data[0] = a; data[1] =  b; }
  index2d(const int* const ab) { data[0] = ab[0]; data[1] =  ab[1]; }

  void clear() { data[0] = -1; data[1] = -1; }

  int& operator[](int i) { return data[i]; }
  const int& operator[](int i) const { return data[i]; }
  operator int* () { return data; }
  operator const int* () const { return data; }

};

class model3d {
  private:
    inline static std::string SAVE_FILE_HEADER() { return std::string("model3d="); }

    GLenum _draw_mode;
    std::vector<vect3f> _coordinates;
    std::vector<std::vector<facet>> _facet_data;
    int _vertex_count;
    bool _need_normals;

    std::vector<model3d> _sub_models;

    vect3f _pos, _axis;
    float _orientation, _new_orientation, _old_orientation;
    bool _smart_rotate, _anchored, _child_animate_flag;
    int _speed;

    void _initialize();
    int _get_facet_id(const vect3f& point) const;
    template <typename T> bool _in_bounds(const int* const indices, const std::vector<std::vector<T>>& vect) const;
    void _calculate_normals() const;

  public:
    model3d();
    model3d(const std::vector<vect3f>& coordinates, const std::vector<std::vector<facet>>& facets);

    void clear();

    std::vector<vect3f> get_coordinates() const;
    const std::vector<vect3f>* const get_coordinates_ptr() const;
    std::vector<std::vector<facet>> get_facet_data() const;
    const std::vector<std::vector<facet>>* const get_facet_data_ptr() const;
    GLenum get_draw_mode() const;

    void set_draw_mode(GLenum);
    void set_vertex_color(int* vertex_id, const vect3f& color);
    vect3f get_vertex_color(int* vertex_id) const;
    index2d add_vertex(const vect3f& point, const vect3f& color=DEFAULT_COLOR, const vect3f* const normal=0);
    void remove_vertex(const index2d& vertex_id); // int vertex_id[2] (indexing into a two-dimensional array)
    void push_face();
    void pop_face();

    int vertex_count() const;

    void save(std::string& filename=std::string()) const; // produces filename if filename has zero length to the saved file name
    bool load(const std::string& filename);

    void set_pos(const vect3f& pos);
    void add_submodel(const model3d& child);

    void anchor(bool t=true);
    void set_axis(const vect3f& axis);
    void set_orientation(float theta);
    void enable_smart_rotate(bool t=true);
    void set_speed(float s);
    void toggle_child_animations();

    void operator++(int);

    void draw() const;
};

#endif

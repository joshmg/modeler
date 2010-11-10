// File: model3d.h
// Written by Joshua Green

#ifndef MODEL3D_H
#define MODEL3D_H

#include "vectXf.h"
#include <vector>
#include <string>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

class model3d {
  private:
    inline static std::string SAVE_FILE_HEADER() { return std::string("model3d="); }
    inline static vect3f DEFAULT_COLOR() { return vect3f(1.0f, 0.0f, 1.0f); }

    GLenum _draw_mode;
    std::vector<vect3f> _coordinates;
    std::vector<std::vector<int>> _facets;
    std::vector<std::vector<vect3f>> _facet_colors;

    std::vector<model3d> _sub_models;

    vect3f _pos, _axis;
    float _orientation, _new_orientation, _old_orientation;
    bool _smart_rotate, _anchored, _child_animate_flag;
    int _speed;

    void _initialize();
    int _get_facet_id(const vect3f& point) const;
    template <typename T> bool _in_bounds(const int* const indices, const std::vector<std::vector<T>>& vect) const;

  public:
    model3d();
    model3d(const std::vector<vect3f>& coordinates, const std::vector<std::vector<int>>& facets, const std::vector<std::vector<vect3f>>* const colors = 0);

    std::vector<vect3f> get_coordinates() const;
    std::vector<std::vector<int>> get_facets() const;
    std::vector<std::vector<vect3f>> get_facet_colors() const;
    GLenum get_draw_mode() const;

    void set_draw_mode(GLenum);
    void vertex_color(int* vertex_id, const vect3f& color);
    void add_vertex(const vect3f& point, const vect3f& color=DEFAULT_COLOR());
    void remove_vertex(int* vertex_id); // int vertex_id[2] (indexing into a two-dimensional array)
    void push_face();
    void pop_face();

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

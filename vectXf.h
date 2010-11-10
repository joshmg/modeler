// File: vectXf.h
// Written by Joshua Green

#ifndef vectXf_H
#define vectXf_H

#include "str/str.h"
#include <string>
#include <vector>

struct vect3f {
  float x, y, z;
  vect3f(float _x=0.0f, float _y=0.0f, float _z=0.0f);

  virtual bool operator==(const vect3f& p) const;
  virtual bool operator!=(const vect3f& p) const;

  virtual vect3f operator*(float c) const;
  virtual vect3f operator/(float c) const;
  virtual vect3f operator+(const vect3f& p) const;
  virtual vect3f operator-(const vect3f& p) const;

  virtual void operator+=(const vect3f& p);
  virtual void operator-=(const vect3f& p);
  virtual void normalize();

  virtual std::string to_string() const;
  virtual vect3f& from_string(std::string data);
};

struct vect2f : public vect3f {
  vect2f(float _x=0.0f, float _y=0.0f);

  virtual std::string to_string() const;
  virtual vect3f& from_string(std::string data);
};

#endif

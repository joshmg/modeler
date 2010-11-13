// File: vectXf.h
// Written by Joshua Green

#ifndef vectXf_H
#define vectXf_H

#include "str/str.h"
#include <string>
#include <vector>

int mod(int a, int b);

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

  virtual vect3f cross(const vect3f& p) const;

  virtual std::string to_string() const;
  virtual vect3f& from_string(std::string data);

  virtual void clear();
};

struct vect2f : public vect3f {
  vect2f(float _x=0.0f, float _y=0.0f);

  virtual std::string to_string() const;
  virtual vect3f& from_string(std::string data);
};

struct glvect4f : public vect3f {
  float a;
  mutable float temp[4];

  glvect4f(float _x=0.0f, float _y=0.0f, float _z=0.0f, float _a=0.0f);
  virtual operator const float* () const;

  virtual void clear();
};

#endif

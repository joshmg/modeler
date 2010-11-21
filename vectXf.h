// File: vectXf.h
// Written by Joshua Green

#ifndef vectXf_H
#define vectXf_H

#include "str/str.h"
#include <string>
#include <vector>

int mod(int a, int b);

struct vect2f {
  float x, y;

  vect2f();
  vect2f(float _x, float _y);

  virtual bool operator==(const vect2f& p) const;
  virtual bool operator!=(const vect2f& p) const;
  vect2f operator*(float c) const;
  vect2f operator/(float c) const;
  vect2f operator+(const vect2f& p) const;
  vect2f operator-(const vect2f& p) const;
  virtual void operator+=(const vect2f& p);
  virtual void operator-=(const vect2f& p);
  virtual void normalize();

  vect2f cross() const;

  std::string to_string() const;
  vect2f& from_string(std::string data);

  virtual void clear();
};

struct vect3f : public vect2f {
  float z;

  vect3f();
  vect3f(float _x, float _y, float _z);

  virtual bool operator==(const vect3f& p) const;
  virtual bool operator!=(const vect3f& p) const;
  vect3f operator*(float c) const;
  vect3f operator/(float c) const;
  vect3f operator+(const vect3f& p) const;
  vect3f operator-(const vect3f& p) const;
  virtual void operator+=(const vect3f& p);
  virtual void operator-=(const vect3f& p);
  virtual void normalize();

  vect3f cross(const vect3f& p) const;

  virtual std::string to_string() const;
  vect3f& from_string(std::string data);

  virtual void clear();
};

struct vect4f : public vect3f {
  float a;
  mutable float temp[4];

  vect4f();
  vect4f(float _x, float _y, float _z, float _a);

  virtual operator const float* () const;

  virtual bool operator==(const vect4f& p) const;
  virtual bool operator!=(const vect4f& p) const;
  vect4f operator*(float c) const;
  vect4f operator/(float c) const;
  vect4f operator+(const vect4f& p) const;
  vect4f operator-(const vect4f& p) const;
  virtual void operator+=(const vect4f& p);
  virtual void operator-=(const vect4f& p);
  virtual void normalize();

  virtual std::string to_string() const;
  vect4f& from_string(std::string data);

  virtual void clear();
};

#endif

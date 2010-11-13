// File: vectXf.cpp
// Written by Joshua Green

#include "vectXf.h"
#include "str/str.h"
#include <string>
#include <vector>
using namespace std;

int mod(int a, int b) { return a%b < 0 ? a%b+b : a%b; }

vect3f::vect3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) { }

bool vect3f::operator==(const vect3f& p) const { return (x==p.x && y==p.y && z==p.z); }

bool vect3f::operator!=(const vect3f& p) const { return !(x==p.x && y==p.y && z==p.z); }

vect3f vect3f::operator*(float c) const { return vect3f(x*c, y*c, z*c); }

vect3f vect3f::operator/(float c) const { return vect3f(x/c, y/c, z/c); }

vect3f vect3f::operator+(const vect3f& p) const { return vect3f(x+p.x, y+p.y, z+p.z); }

vect3f vect3f::operator-(const vect3f& p) const { return vect3f(x-p.x, y-p.y, z-p.z); }

void vect3f::operator+=(const vect3f& p) { x+=p.x; y+=p.y; z+=p.z; }

void vect3f::operator-=(const vect3f& p) { x-=p.x; y-=p.y; z-=p.z; }

void vect3f::normalize() {
  float mag = sqrt(x*x + y*y + z*z);
  if (mag != 0.0f) {
    x /= mag;
    y /= mag;
    z /= mag;
  }
}

vect3f vect3f::cross(const vect3f& p) const {
  return vect3f(  (y*p.z - z*p.y),
                  (z*p.x - x*p.z),
                  (x*p.y - y*p.x)
               );
}

string vect3f::to_string() const {
  std::string value = "("; 
  value += ftos(x);
  value += ", ";
  value += ftos(y);
  value += ", ";
  value += ftos(z);
  value += ")";
  return value;
}

vect3f& vect3f::from_string(std::string data) {
  data.erase(data.begin()); // remove '('
  data.erase(data.end()--); // remove ')'
  std::vector<std::string> coordinate_strings(explode(data, ", ", -1));

  if (coordinate_strings.size() < 3) return (*this);

  x = atof(coordinate_strings[0].c_str());
  y = atof(coordinate_strings[1].c_str());
  z = atof(coordinate_strings[2].c_str());
  return (*this);
}

void vect3f::clear() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
}




// **** begin class vect2f definitions ****


vect2f::vect2f(float _x, float _y) {
  x = _x;
  y = _y;
  z = 0.0f;
}

std::string vect2f::to_string() const {
  std::string value = "("; 
  value += itos(x);
  value += ", ";
  value += itos(y);
  value += ")";
  return value;
}

vect3f& vect2f::from_string(std::string data) {
  data.erase(data.begin()); // remove '('
  data.erase(data.end()--); // remove ')'
  std::vector<std::string> coordinate_strings(explode(data, ", ", -1));

  if (coordinate_strings.size() < 2) return (*this);

  x = atof(coordinate_strings[0].c_str());
  y = atof(coordinate_strings[1].c_str());
  z = 0.0f;
  return (*this);
}




// **** begin class glvect4f definitions ****


glvect4f::glvect4f(float _x, float _y, float _z, float _a) {
  x = _x;
  y = _y;
  z = _z;
  a = _a;
}

glvect4f::operator const float* () const {
  temp[0] = x;
  temp[1] = y;
  temp[2] = z;
  temp[3] = a;

  return temp;
}

void glvect4f::clear() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
  a = 0.0f;
}

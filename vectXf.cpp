// File: vectXf.cpp
// Written by Joshua Green

#include "vectXf.h"
#include "str/str.h"
#include <string>
#include <vector>
using namespace std;

int mod(int a, int b) { return a%b < 0 ? a%b+b : a%b; }



// **** begin class vect2f definitions **** //
vect2f::vect2f() : x(0.0f), y(0.0f) { }
vect2f::vect2f(float _x, float _y) : x(_x), y(_y) { }

bool vect2f::operator==(const vect2f& p) const { return ((x == p.x) && (y == p.x)); }
bool vect2f::operator!=(const vect2f& p) const { return ((x != p.x) || (y != p.x)); }
vect2f vect2f::operator+(const vect2f& p) const { return vect2f(x+p.x, y+p.y); }
vect2f vect2f::operator-(const vect2f& p) const { return vect2f(x-p.x, y-p.y); }
void vect2f::operator+=(const vect2f& p) { x += p.x; y += p.y; }
void vect2f::operator-=(const vect2f& p) { x -= p.x; y -= p.y; }

void vect2f::normalize() {
  float mag(sqrt(x*x + y*y));
  if (mag != 0.0000f) {
    x /= mag;
    y /= mag;
  }
}

vect2f vect2f::cross() const { return vect2f(y, -x); }

std::string vect2f::to_string() const {
  std::string value = "("; 
  value += itos(x);
  value += ", ";
  value += itos(y);
  value += ")";
  return value;
}

vect2f& vect2f::from_string(string data) {
  if (data.length() < 3) {
    clear();
    return (*this);
  }

  data.erase(data.begin()); // remove '('
  data.erase(data.end()--); // remove ')'
  std::vector<std::string> coordinate_strings(explode(data, ", ", -1));

  if (coordinate_strings.size() < 2) clear();
  else {
    x = atof(coordinate_strings[0].c_str());
    y = atof(coordinate_strings[1].c_str());
  }

  return (*this);
}

void vect2f::clear() {
  x = 0.0f;
  y = 0.0f;
}




// *** begin class vect3f defintions *** //
vect3f::vect3f() : z(0.0f) { }
vect3f::vect3f(float _x, float _y, float _z) : vect2f(_x, _y), z(_z) { }

bool vect3f::operator==(const vect3f& p) const { return (x==p.x && y==p.y && z==p.z); }
bool vect3f::operator!=(const vect3f& p) const { return (x!=p.x || y!=p.y || z==p.z); }
vect3f vect3f::operator*(float c) const { return vect3f(x*c, y*c, z*c); }
vect3f vect3f::operator/(float c) const { return vect3f(x/c, y/c, z/c); }
vect3f vect3f::operator+(const vect3f& p) const { return vect3f(x+p.x, y+p.y, z+p.z); }
vect3f vect3f::operator-(const vect3f& p) const { return vect3f(x-p.x, y-p.y, z-p.z); }
void vect3f::operator+=(const vect3f& p) { x+=p.x; y+=p.y; z+=p.z; }
void vect3f::operator-=(const vect3f& p) { x-=p.x; y-=p.y; z-=p.z; }

void vect3f::normalize() {
  float mag = sqrt(x*x + y*y + z*z);
  if (mag != 0.0000f) {
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
  if (data.length() < 4) {
    clear();
    return (*this);
  }

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




// **** begin class glvect4f definitions ****
vect4f::vect4f() : a(0.0) { }
vect4f::vect4f(float _x, float _y, float _z, float _a) : vect3f(_x, _y, _z), a(_a) { }

vect4f::operator const float* () const {
  temp[0] = x;
  temp[1] = y;
  temp[2] = z;
  temp[3] = a;

  return temp;
}

bool vect4f::operator==(const vect4f& p) const { return (x==p.x && y==p.y && z==p.z && a==p.a); }
bool vect4f::operator!=(const vect4f& p) const { return (x!=p.x || y!=p.y || z!=p.z || a!=p.a); }
vect4f vect4f::operator*(float c) const { return vect4f(x*c, y*c, z*c, a*c); }
vect4f vect4f::operator/(float c) const { return vect4f(x/c, y/c, z/c, a/c); }
vect4f vect4f::operator+(const vect4f& p) const { return vect4f(x+p.x, y+p.y, z+p.z, a+p.a); }
vect4f vect4f::operator-(const vect4f& p) const { return vect4f(x-p.x, y-p.y, z-p.z, a-p.a); }
void vect4f::operator+=(const vect4f& p) { x+=p.x; y+=p.y; z+=p.z; a+=p.a;}
void vect4f::operator-=(const vect4f& p) { x-=p.x; y-=p.y; z-=p.z; a-=p.a;}


void vect4f::normalize() {
  float mag = sqrt(x*x + y*y + z*z + a*a);
  if (mag != 0.0000f) {
    x /= mag;
    y /= mag;
    z /= mag;
    a /= mag;
  }
}

string vect4f::to_string() const {
  std::string value = "("; 
  value += ftos(x);
  value += ", ";
  value += ftos(y);
  value += ", ";
  value += ftos(z);
  value += ", ";
  value += ftos(a);
  value += ")";
  return value;
}

vect4f& vect4f::from_string(string data) {
  if (data.length() < 5) {
    clear();
    return (*this);
  }
  data.erase(data.begin()); // remove '('
  data.erase(data.end()--); // remove ')'
  vector<string> coordinate_strings(explode(data, ", ", -1));

  if (coordinate_strings.size() < 4) return (*this);

  x = atof(coordinate_strings[0].c_str());
  y = atof(coordinate_strings[1].c_str());
  z = atof(coordinate_strings[2].c_str());
  a = atof(coordinate_strings[3].c_str());
  return (*this);
}


void vect4f::clear() {
  x = 0.0f;
  y = 0.0f;
  z = 0.0f;
  a = 0.0f;
}

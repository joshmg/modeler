// File: modeler.cpp
// Written by Joshua Green

#include <iostream>
#include <vector>
#include <map>

// needed for multithreading...
#include <process.h>

#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glu.h>

#include "vectXf.h"
#include "model3d.h"
#include "cube.h"
using namespace std;


//#define USE_GL_COLOR_MATERIAL
#define USE_SPECULAR

// openGL function declarations
void init_opengl();    // sets openGL settings
void display();        // primary display func
void mouse_callback(int btn, int state, int x, int y);
void keyboard_callback(unsigned char key, int x, int y);
void refresh(); // refreshes the display (a bit convoluted.. essientially calls display() but use this instead)
void window_resize(int w, int h);
void set_camera();
void draw_pointer(); // draws the cursor
void draw_axis();
void draw_color_palette();
void init_lighting();
void set_light_pos();
void set_ambient();
void draw_light();

void quit_branch(void*); // for multithreading
void save_branch(void*); // for multithreading
void load_branch(void*); // for multithreading
void define_grid_branch(void*); // for multithreading
void merge_model_branch(void*); // for multithreading
void face_resolution_branch(void*); // for multithreading
void translate_model_branch(void*); // for multithreading
void transform_model_branch(void*); // for multithreading

// misc utility functions
template <typename T> bool in_bounds(const int* const, const vector<vector<T>>&); // true if int vertices[2] is a valid index within the 2d vector
void edit_model(int); // switches a loaded model buffer with active editing buffer
void define_cube(); // defines the grid lines using UNIT_SIZE and CUBE_COUNT
bool prompt_save();

// globals
int   SCREEN_W = 800,    SCREEN_H = 600;
float  WORLD_W = 100.0f,  WORLD_H = 100.0f;
const float VIEW_ANGLE = 45.0f; // static frustrum angle
vect3f POINTER; // position of the cursor
vector<cube> RUBIX; // the grid lines
float UNIT_SIZE; // grid line width
int CUBE_COUNT; // number of unit cubes centered at origin
index2d SELECTED(-1, 0); // the currently selected vertex (used via Tab button) ((-1, -1) is the convention for no selection)
vect3f HIGHLIGHTED_COLOR(1.0f, 1.0f, 1.0f);
bool DRAW_POLYGON_MODE = false; // if false glBegin(GL_LINES) is used, true = glBegin(GL_POLYGON)
bool HIGHLIGHT = true; // toggles the highlight of the working unit cube

model3d WORKING_MODEL; // the model currently being edited

bool DISPLAY_WORKING_MODEL = true; // if false, the edited model buffer is not displayed.

vector<model3d> LOADED_MODELS; // the loaded models (accessed via load/save) (display toggled via 1-9) (edited via F1-F9)
bool DRAW_MODELS[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // used to toggled the display of loaded/saved models

bool DRAW_PALETTE = true; // never toggled off but still here
const float PALETTE_HEIGHT = 3.5f;
float PALETTE_alpha = 1.0f; // palette color alpha values
float PALETTE_gamma = 0.0f; // palette color gamma values
float PALETTE_MOUSE_CTRL[2]; // the left x value and width of the area which control the alpha/gamma levels
                             //   (since that location is calculated dynamically depending on window size)

vect3f SELECTED_COLOR(1.0f, 0.0, 0.0); // the last selected color from the palette
vect3f* COLOR_MAP = new vect3f[WORLD_W]; // the color map used to map the palette coords to the particular color

bool ALREADY_BRANCHED = false; // only one branching operation at a time (don't want to be loading and saving at the same time...)

bool DRAW_AXIS = true;
bool DRAW_GRID = true; // toggles drawing the grid lines (the cubes)

bool UNSAVED_BUFFER = false;

bool LIGHTS_ON = false;
bool SET_LIGHT_POS = false;
vect4f LIGHT0_POS;
vect4f LIGHT_AMBIENT(0.2, 0.2, 0.2, 1.0);


void init_opengl() {
  glClearColor(0.0, 0.0, 0.0, 1.0);

  glEnable(GL_DEPTH_TEST);
  #ifdef USE_GL_COLOR_MATERIAL
   glEnable(GL_COLOR_MATERIAL);
  #endif

  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

  set_camera();

  if (LIGHTS_ON) init_lighting();

  glLineWidth(1.0);
}

void init_lighting() {
  glEnable(GL_LIGHTING);

  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  glLightfv(GL_LIGHT0, GL_AMBIENT, vect4f(0.0, 0.0, 0.0, 1.0));
  glLightfv(GL_LIGHT0, GL_DIFFUSE, vect4f(1.0, 1.0, 1.0, 1.0));
  #ifdef USE_SPECULAR
    glLightfv(GL_LIGHT0, GL_SPECULAR, vect4f(1.0, 1.0, 1.0, 1.0));
  #endif
}

void set_ambient() {
  // *** NOTE: USE GL_LIGHT_MODEL_AMBIENT, NOT GL_AMBIENT ***
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LIGHT_AMBIENT);
}

void set_light_pos() {
  glLightfv(GL_LIGHT0, GL_POSITION, LIGHT0_POS);
}

void draw_light() {
  cube light;
  light.initialize(vect3f(LIGHT0_POS.x-0.1, LIGHT0_POS.y-0.1, LIGHT0_POS.z-0.1), 0.2);
  light.set_solid(true);
  light.set_color(vect3f(1, 1, 1));
  light.set_highlight(vect3f(1, 1, 1), 1);

  glMaterialfv(GL_FRONT, GL_EMISSION, vect4f(0.4, 0.4, 0.4, 1.0)); // turn on emmissive material
  light.draw();
  glMaterialfv(GL_FRONT, GL_EMISSION, vect4f(0.0, 0.0, 0.0, 1.0)); // turn off emmissive material
}

void refresh() {
  glutPostRedisplay();
}

void set_camera() {

  // set projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glViewport(0, 0, SCREEN_W, SCREEN_H);

  gluPerspective( VIEW_ANGLE,                  // view angle
                  ((float)SCREEN_W)/SCREEN_H,  // aspect ratio
                  1.0, 100.0);                 // near/far clipping planes

  glMatrixMode(GL_MODELVIEW);
}

void window_resize(int w, int h) {
  SCREEN_W = w;
  SCREEN_H = h;

  set_camera();
}

void mouse_callback(int btn, int state, int x, int y) {
  // translate glut window coords to world coords:
  x = x * (WORLD_W / SCREEN_W);
  y = (SCREEN_H - y) * (WORLD_H / SCREEN_H);

  if (state == GLUT_DOWN) {
    if (y <= PALETTE_HEIGHT) { // clicked within the palette area
      if (x >= PALETTE_MOUSE_CTRL[0]) { // clicked within the control area
        if (x < PALETTE_MOUSE_CTRL[0]+PALETTE_MOUSE_CTRL[1]) { // alpha control
          if (y <= PALETTE_HEIGHT/2.0) PALETTE_alpha -= 0.20; // alpha decrease
          else PALETTE_alpha += 0.20; // alpha increase

          if (PALETTE_alpha < 0.0) PALETTE_alpha = 0.0;
          if (PALETTE_alpha > 1.0) PALETTE_alpha = 1.0;
        }
        else { // gamma control
          if (y <= PALETTE_HEIGHT/2.0) PALETTE_gamma -= 0.20; // gamma decrease
          else PALETTE_gamma += 0.20; // gamma increase

          if (PALETTE_gamma < 0.0) PALETTE_gamma = 0.0;
          if (PALETTE_gamma > 1.0) PALETTE_gamma = 1.0;
        }
      }
      else { // clicked a color
        SELECTED_COLOR = COLOR_MAP[x];
        //if (in_bounds(SELECTED, MODEL_POINTS)) MODEL_COLORS[SELECTED[0]][SELECTED[1]] = SELECTED_COLOR;
        if (in_bounds(SELECTED, (*(WORKING_MODEL.get_facet_data_ptr())))) {
          WORKING_MODEL.set_vertex_color(SELECTED, SELECTED_COLOR);
          UNSAVED_BUFFER = true;
        }
      }
    }
  }

  refresh();
}

void special_keys_callback(int key, int x, int y) {
  // translate glut window coords to world coords:
  x = x * (WORLD_W / SCREEN_W);
  y = (SCREEN_H - y) * (WORLD_H / SCREEN_H);

  float movement_unit = 1.0f;

  switch(key) {
    case GLUT_KEY_RIGHT: {
      if (glutGetModifiers() == GLUT_ACTIVE_CTRL) glRotatef(movement_unit, 0.0, 1.0, 0.0);
      else if (glutGetModifiers() == GLUT_ACTIVE_ALT) glRotatef(-movement_unit, 0.0, 0.0, 1.0);
      else glTranslatef(1.0, 0.0, 0.0);
    } break;
    case GLUT_KEY_LEFT: {
      if (glutGetModifiers() == GLUT_ACTIVE_CTRL) glRotatef(-movement_unit, 0.0, 1.0, 0.0);
      else if (glutGetModifiers() == GLUT_ACTIVE_ALT) glRotatef(movement_unit, 0.0, 0.0, 1.0);
      else glTranslatef(-1.0, 0.0, 0.0);
    } break;
    case GLUT_KEY_UP: {
      if (glutGetModifiers() == GLUT_ACTIVE_CTRL) glRotatef(-movement_unit, 1.0, 0.0, 0.0);
      else if (glutGetModifiers() == GLUT_ACTIVE_ALT) {}
      else if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) glTranslatef(0.0, 0.0, 1.0);
      else glTranslatef(0.0, -1.0, 0.0);
    } break;
    case GLUT_KEY_DOWN: {
      if (glutGetModifiers() == GLUT_ACTIVE_CTRL) glRotatef(movement_unit, 1.0, 0.0, 0.0);
      else if (glutGetModifiers() == GLUT_ACTIVE_ALT) {}
      else if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) glTranslatef(0.0, 0.0, -1.0);
      else glTranslatef(0.0, 1.0, 0.0);
    } break;

    case GLUT_KEY_F1: { edit_model(0); } break;
    case GLUT_KEY_F2: { edit_model(1); } break;
    case GLUT_KEY_F3: { edit_model(2); } break;
    case GLUT_KEY_F4: { edit_model(3); } break;
    case GLUT_KEY_F5: { edit_model(4); } break;
    case GLUT_KEY_F6: { edit_model(5); } break;
    case GLUT_KEY_F7: { edit_model(6); } break;
    case GLUT_KEY_F8: { edit_model(7); } break;
    case GLUT_KEY_F9: { edit_model(8); } break;

    default: {} break;
  }

  refresh();
}
void keyboard_callback(unsigned char key, int x, int y) {
  if (key == 'q') {
    if (!ALREADY_BRANCHED) _beginthread(&quit_branch, 0, (void*)0);
  }

  switch(key) {
    case 'w': {
      if (SET_LIGHT_POS) LIGHT0_POS.y += UNIT_SIZE/10.0f;
      else POINTER.y += UNIT_SIZE/10.0f;
    } break;
    case 'a': {
      if (SET_LIGHT_POS) LIGHT0_POS.x -= UNIT_SIZE/10.0f;
      else POINTER.x -= UNIT_SIZE/10.0f;
    } break;
    case 's': {
      if (SET_LIGHT_POS) LIGHT0_POS.y -= UNIT_SIZE/10.0f;
      else POINTER.y -= UNIT_SIZE/10.0f;
    } break;
    case 'd': {
      if (SET_LIGHT_POS) LIGHT0_POS.x += UNIT_SIZE/10.0f;
      else POINTER.x += UNIT_SIZE/10.0f;
    } break;
    case 'W': {
      if (SET_LIGHT_POS) LIGHT0_POS.z -= UNIT_SIZE/10.0f;
      else POINTER.z -= UNIT_SIZE/10.0f;
    } break;
    case 'S': {
      if (SET_LIGHT_POS) LIGHT0_POS.z += UNIT_SIZE/10.0f;
      else POINTER.z += UNIT_SIZE/10.0f;
    } break;
    case 8: { // backspace
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glTranslatef(0.0, 0.0, -UNIT_SIZE*(CUBE_COUNT+2));
      POINTER.clear();
      LIGHT0_POS = vect4f(UNIT_SIZE*(1), UNIT_SIZE*(1), UNIT_SIZE*(1), 1.0);
    } break;

    case 'c': {
      if (in_bounds(SELECTED, (*(WORKING_MODEL.get_facet_data_ptr())))) {
        WORKING_MODEL.remove_vertex(SELECTED);
        UNSAVED_BUFFER = true;
      }
      SELECTED.clear();
    } break;
    case 'C': {
      WORKING_MODEL.clear();
      SELECTED.clear();
      UNSAVED_BUFFER = false;
    } break;
    case 'f': {
      const vector<vect3f>* const model_coordinates = WORKING_MODEL.get_coordinates_ptr();
      const vector<vector<facet>>* const model_facets = WORKING_MODEL.get_facet_data_ptr();
      if (in_bounds(SELECTED, *model_facets)) {
        WORKING_MODEL.add_vertex((*model_coordinates)[((*model_facets)[SELECTED[0]][SELECTED[1]]).id], SELECTED_COLOR);
        UNSAVED_BUFFER = true;
      }
    } break;

    case 'p': {
      WORKING_MODEL.push_face();
      SELECTED.clear();
      UNSAVED_BUFFER = true;
    } break;
    case 'P': {
      WORKING_MODEL.pop_face();
      UNSAVED_BUFFER = true;
    } break;
    case 'o': {
      DISPLAY_WORKING_MODEL = !DISPLAY_WORKING_MODEL; // hides the model
    } break;
    case 'g': {
      DRAW_GRID = !DRAW_GRID;
    } break;
    case 'G': {
      if (!ALREADY_BRANCHED) _beginthread(&define_grid_branch, 0, (void*)0);
    } break;
    case 'm': {
      DRAW_POLYGON_MODE = !DRAW_POLYGON_MODE;
    } break;
    case 'h': {
      HIGHLIGHT = !HIGHLIGHT;
    } break;
    case 'r' : {
      if (!ALREADY_BRANCHED) _beginthread(&face_resolution_branch, 0, (void*)0);
    }

    case 32: { // space key
      WORKING_MODEL.add_vertex(POINTER, SELECTED_COLOR);
      UNSAVED_BUFFER = true;
    } break;
    case 9: { // tab key
      const vector<vector<facet>>* const facet_data = WORKING_MODEL.get_facet_data_ptr();

      if (!in_bounds(SELECTED, *facet_data)) {
        if (WORKING_MODEL.vertex_count() > 0) {
          SELECTED[0] = 0;
          SELECTED[1] = 0;
        }
        else SELECTED.clear();
      }
      else {
        if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
          if (SELECTED[1] > 0) SELECTED[1]--; // set the selected vertex to the previous vertex
          else if (SELECTED[0] > 0) {
            // set the selected vertex to the last vertex in the previous face
            SELECTED[0]--;
            SELECTED[1] = facet_data[SELECTED[0]].size()-1;
          }
          else {
            // set the selected vertex to the last vertex of the last face
            SELECTED[0] = facet_data->size()-1;
            SELECTED[1] = (*facet_data)[SELECTED[0]].size()-1;
          }
        }
        else {
          if (SELECTED[1] < (*facet_data)[SELECTED[0]].size()-1) SELECTED[1]++;  // set the selected vertex to the next vertex
          else if (SELECTED[0] < (*facet_data).size()-1) {
            // set the vertex to the first vertex in the next face
            SELECTED[0]++;
            SELECTED[1] = 0;
          }
          else {
            // set the vertex to the first vertex
            SELECTED[0] = 0;
            SELECTED[1] = 0;
          }
        }
      }
    } break;

    case 13: { // enter
      if (!ALREADY_BRANCHED) _beginthread(save_branch, 0, (void*)0);
    } break;

    case '1': {
      if (LOADED_MODELS.size() > 0) DRAW_MODELS[0] = !DRAW_MODELS[0];
      else DRAW_MODELS[0] = false;
    } break;
    case '2': {
      if (LOADED_MODELS.size() > 1) DRAW_MODELS[1] = !DRAW_MODELS[1];
      else DRAW_MODELS[1] = false;
    } break;
    case '3': {
      if (LOADED_MODELS.size() > 2) DRAW_MODELS[2] = !DRAW_MODELS[2];
      else DRAW_MODELS[2] = false;
    } break;
    case '4': {
      if (LOADED_MODELS.size() > 3) DRAW_MODELS[3] = !DRAW_MODELS[3];
      else DRAW_MODELS[3] = false;
    } break;
    case '5': {
      if (LOADED_MODELS.size() > 4) DRAW_MODELS[4] = !DRAW_MODELS[4];
      else DRAW_MODELS[4] = false;
    } break;
    case '6': {
      if (LOADED_MODELS.size() > 5) DRAW_MODELS[5] = !DRAW_MODELS[5];
      else DRAW_MODELS[5] = false;
    } break;
    case '7': {
      if (LOADED_MODELS.size() > 6) DRAW_MODELS[6] = !DRAW_MODELS[6];
      else DRAW_MODELS[6] = false;
    } break;
    case '8': {
      if (LOADED_MODELS.size() > 7) DRAW_MODELS[7] = !DRAW_MODELS[7];
      else DRAW_MODELS[7] = false;
    } break;
    case '9': {
      if (LOADED_MODELS.size() > 8) DRAW_MODELS[8] = !DRAW_MODELS[8];
      else DRAW_MODELS[8] = false;
    } break;

    case 'l': {
      if (!ALREADY_BRANCHED) _beginthread(&load_branch, 0, (void*)0);
    } break;

    case 'x': {
      DRAW_AXIS = !DRAW_AXIS;
    } break;

    case 'M': {
      if (!ALREADY_BRANCHED) _beginthread(&merge_model_branch, 0, (void*)0);
    } break;

    case 't': {
      LIGHTS_ON = !LIGHTS_ON;
      if (LIGHTS_ON) {
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
      }
      else {
        glDisable(GL_LIGHTING);
        glDisable(GL_LIGHT0);
      }
    } break;
    case 'T': {
      SET_LIGHT_POS = !SET_LIGHT_POS;
    } break;
    case '>': {
      if (!ALREADY_BRANCHED) _beginthread(&translate_model_branch, 0, (void*)0);
    } break;
    case '<': {
      if (!ALREADY_BRANCHED) _beginthread(&transform_model_branch, 0, (void*)0);
    } break;

    case 27: { // escape key
      SELECTED.clear();
    } break;
    default: {} break;
  }

  refresh();
}

void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (LIGHTS_ON) {
    set_ambient();
    set_light_pos();
  }
  set_camera();
  if (LIGHTS_ON) draw_light();

  glDisable(GL_LIGHTING);
  if (DRAW_AXIS) draw_axis();
  draw_pointer();
  if (LIGHTS_ON) glEnable(GL_LIGHTING);

  glLineWidth(2.0);

  #ifdef USE_SPECULAR
    glMaterialfv(GL_FRONT, GL_SPECULAR, vect4f(1.0, 1.0, 1.0, 1.0));
    glMaterialfv(GL_FRONT, GL_SHININESS, vect4f(50.0, 1.0, 1.0, 1.0));
  #endif

  // draw edit model buffer
  if (DISPLAY_WORKING_MODEL) {
    GLenum restore_gl_draw_mode = WORKING_MODEL.get_draw_mode();
    if (DRAW_POLYGON_MODE) WORKING_MODEL.set_draw_mode(GL_LINE_LOOP); // if drawing wireframe mode, change draw mode appropriately

    if (in_bounds(SELECTED, *(WORKING_MODEL.get_facet_data_ptr()))) {
      // if SELECTED is valid, change the color of the selected vertex to the highlighted color and then draw, restoring original color afterwards
      vect3f restore_color = WORKING_MODEL.get_vertex_color(SELECTED);
      WORKING_MODEL.set_vertex_color(SELECTED, HIGHLIGHTED_COLOR);
      WORKING_MODEL.draw();
      WORKING_MODEL.set_vertex_color(SELECTED, restore_color);
    }
    else WORKING_MODEL.draw();
    if (DRAW_POLYGON_MODE) WORKING_MODEL.set_draw_mode(restore_gl_draw_mode); // restore old draw mode if it was modified...
  }

  // draw loaded models
  for (int i=0;i<LOADED_MODELS.size();i++) {
    if (DRAW_MODELS[i]) {
      GLenum old_draw_mode = LOADED_MODELS[i].get_draw_mode();
      if (DRAW_POLYGON_MODE) LOADED_MODELS[i].set_draw_mode(GL_LINE_LOOP);
      LOADED_MODELS[i].draw();
      LOADED_MODELS[i].set_draw_mode(old_draw_mode);
    }
  }
  glLineWidth(1.0);

  glDisable(GL_LIGHTING);
  // draw grid lines
  if (DRAW_GRID) {
    glColor3f(0.0f, 0.3f, 0.0f);
    for (int i=0;i<RUBIX.size();i++) {
      if (HIGHLIGHT) RUBIX[i].draw(&POINTER);
      else RUBIX[i].draw();
    }
  }
  
  // draw palette
  draw_color_palette();
  if (LIGHTS_ON) glEnable(GL_LIGHTING);

  glutSwapBuffers();
}

void draw_axis() {
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(1.0, 0.0, 0.0);

  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 1.0, 0.0);

  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(0.0, 0.0, 1.0);
  glEnd();
}

void draw_pointer() {
  glColor3f(1.0, 1.0, 1.0);
  glBegin(GL_POINTS);
  glVertex3f(POINTER.x, POINTER.y, POINTER.z);
  glEnd();

  glColor3f(0.3, 0.3, 0.3);

  float line_length = 0.2;
  float offset = 0.05f;

  glBegin(GL_LINES);
  glVertex3f(POINTER.x, POINTER.y+offset, POINTER.z);
  glVertex3f(POINTER.x, POINTER.y+offset+line_length, POINTER.z);

  glVertex3f(POINTER.x, POINTER.y-offset, POINTER.z);
  glVertex3f(POINTER.x, POINTER.y-offset-line_length, POINTER.z);

  glVertex3f(POINTER.x, POINTER.y, POINTER.z+offset);
  glVertex3f(POINTER.x, POINTER.y, POINTER.z+offset+line_length);

  glVertex3f(POINTER.x, POINTER.y, POINTER.z-offset);
  glVertex3f(POINTER.x, POINTER.y, POINTER.z-offset-line_length);

  glVertex3f(POINTER.x+offset, POINTER.y, POINTER.z);
  glVertex3f(POINTER.x+offset+line_length, POINTER.y, POINTER.z);

  glVertex3f(POINTER.x-offset, POINTER.y, POINTER.z);
  glVertex3f(POINTER.x-offset-line_length, POINTER.y, POINTER.z);
  glEnd();

  line_length /= 1.5;

  glBegin(GL_LINES);
  glVertex3f(POINTER.x-offset, POINTER.y-offset, POINTER.z);
  glVertex3f(POINTER.x-offset-line_length, POINTER.y-offset-line_length, POINTER.z);
  
  glVertex3f(POINTER.x+offset, POINTER.y+offset, POINTER.z);
  glVertex3f(POINTER.x+offset+line_length, POINTER.y+offset+line_length, POINTER.z);

  glVertex3f(POINTER.x-offset, POINTER.y+offset, POINTER.z);
  glVertex3f(POINTER.x-offset-line_length, POINTER.y+offset+line_length, POINTER.z);

  glVertex3f(POINTER.x+offset, POINTER.y-offset, POINTER.z);
  glVertex3f(POINTER.x+offset+line_length, POINTER.y-offset-line_length, POINTER.z);
  glEnd();

  glPushMatrix();

  glTranslatef(POINTER.x, POINTER.y, POINTER.z);
  glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
  glTranslatef(-POINTER.x, -POINTER.y, -POINTER.z);

  glBegin(GL_LINES);
  glVertex3f(POINTER.x-offset, POINTER.y-offset, POINTER.z);
  glVertex3f(POINTER.x-offset-line_length, POINTER.y-offset-line_length, POINTER.z);
  
  glVertex3f(POINTER.x+offset, POINTER.y+offset, POINTER.z);
  glVertex3f(POINTER.x+offset+line_length, POINTER.y+offset+line_length, POINTER.z);

  glVertex3f(POINTER.x-offset, POINTER.y+offset, POINTER.z);
  glVertex3f(POINTER.x-offset-line_length, POINTER.y+offset+line_length, POINTER.z);

  glVertex3f(POINTER.x+offset, POINTER.y-offset, POINTER.z);
  glVertex3f(POINTER.x+offset+line_length, POINTER.y-offset-line_length, POINTER.z);
  glEnd();
  glPopMatrix();
}

int main(int argc, char** argv) {

  cout << "Directions: " << endl
       << "  Use 'w', 'a', 's', 'd' to move the cursor." << endl
       << "    'W' and 'S' move the cursor forward and backwards in the Z-Direction." << endl
       << "  CTRL-[arrow-keys] rotate the world. (Up/Down->X-Axis, Left/Right->Y-Axis)." << endl
       << "  ALT-[arrow-keys] rotate the world along the Z-axis (only Left/Right)." << endl
       << "  [arrow-keys] move the scene along the respective axis." << endl
       << "    (Left/Right->X-axis), (Up/Down->Y-axis), (Shift-Up/Shift-Down->Z-axis)" << endl
       << "  'h' toggles unit square highlighting." << endl
       << "  The space-bar creates a point at the current cursor location." << endl
       << "  Tab cycles through the drawn vertices (selected vertex highlighted in white)." << endl
       << "  'f' draws a line from the most recent point to the selected vertex." << endl
       << "  'm' toggles the wireframe display of the drawn vertices." << endl
       << "  'c' deletes the selected vertex." << endl
       << "  'C' clears all drawn vertices in the current buffer." << endl
       << "  Backspace resets the scene to the initial view (does not delete vertices)." << endl
       << "  'o' toggles the display of the edited model." << endl
       << "  'g' toggles the display of the grid." << endl
       << "  'G' opens a dialog to redefine the grid size." << endl
       << "  'p' pushes the current face onto the model." << endl
       << "  'P' pops the current face from the model." << endl
       << "  Enter saves the model to a file." << endl
       << "      - opens a dialog to enter a filename in the command window." << endl
       << "  'l' loads a saved model." << endl
       << "      - opens a dialog to enter the filename in the command window." << endl
       << "  1-9 toggles the display of the saved model's respective number." << endl 
       << "  F1-F9 edits the saved or loaded model associated with that number." << endl
       << "      - the current model buffer is swapped to the respective slot." << endl
       << "      - the swapped model buffer is not saved to a file." << endl
       << "  Select a color from the palette to change the Tab-selected facet's color." << endl
       << "      - additional vertices are drawn in the most recently selected color." << endl
       << "  The + or - buttons on the palette change its alpha and gamma levels." << endl
       << "  'q' exits the program." << endl
       << "  't' toggles lighting." << endl
       << "  'T' toggles control of the cursor position or the light source position." << endl
       << "  'r' sets the current working model's face resolution to a number of polygons." << endl
       << endl;

  UNIT_SIZE = 1.0f;
  CUBE_COUNT = 5;

  define_cube();

  LIGHT0_POS = vect4f(UNIT_SIZE*(1), UNIT_SIZE*(1), UNIT_SIZE*(1), 1.0);

  for (int i=0;i<9;i++) LOADED_MODELS.push_back(model3d());

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); // double buffer, rgb color, depth buffer
  glutInitWindowSize(SCREEN_W, SCREEN_H);
  glutCreateWindow("3D Modeler");

  glutDisplayFunc(display);
  glutMouseFunc(mouse_callback);
  glutKeyboardFunc(keyboard_callback);
  glutSpecialFunc(special_keys_callback);
  glutReshapeFunc(window_resize);
  init_opengl();

  glTranslatef(0.0, 0.0, -UNIT_SIZE*(CUBE_COUNT+2));

  refresh();
  glutMainLoop();

  delete[] COLOR_MAP;

  return 0;
}

template <typename T> bool in_bounds(const int* const indices, const vector<vector<T>>& vect) {
  if (indices[0] < 0 || indices[1] < 0) return false;
  return (indices[0] < vect.size() && indices[1] < vect[indices[0]].size());
}

void draw_color_palette() {
  if (DRAW_PALETTE) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluOrtho2D(0, WORLD_W, 0, WORLD_H);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_QUADS);
    glVertex2f(0.0, 0.0);
    glVertex2f(WORLD_W, 0.0);
    glVertex2f(WORLD_W, PALETTE_HEIGHT);
    glVertex2f(0.0, PALETTE_HEIGHT);
    glEnd();

    float x = 0.0;
    glBegin(GL_LINES);
    for (float r=0.0;r<=1.0;r+=0.25) {
      for (float g=0.0;g<=1.0;g+=0.25) {
        for (float b=0.0;b<=1.0;b+=0.25) {
          for (int i=0;i<3;i++) {
            if ((int)x > (int)WORLD_W) break; // shouldn't happen but avoids out of bounds possibilities by varying values of WORLD_W

            COLOR_MAP[(int)x] = vect3f(r*PALETTE_alpha+PALETTE_gamma, g*PALETTE_alpha+PALETTE_gamma, b*PALETTE_alpha+PALETTE_gamma);

            glColor3f(COLOR_MAP[(int)x].x, COLOR_MAP[(int)x].y, COLOR_MAP[(int)x].z);
            glVertex2f(x, 0.0);
            glVertex2f(x, PALETTE_HEIGHT);
            x += 0.25f;
          }
        }
      }
    }
    glEnd();

    float width = (WORLD_W-x)/2.0f;

    PALETTE_MOUSE_CTRL[0] = x;
    PALETTE_MOUSE_CTRL[1] = width;

    for (int i=0;i<2;i++) {
      glBegin(GL_QUADS);
        glColor3f(SELECTED_COLOR.x, SELECTED_COLOR.y, SELECTED_COLOR.z);
        glVertex2f(x, PALETTE_HEIGHT/2.0f+0.12);
        glVertex2f(x+width, PALETTE_HEIGHT/2.0f+0.12);
        glVertex2f(x+width, PALETTE_HEIGHT);
        glVertex2f(x, PALETTE_HEIGHT);
      glEnd();
      glBegin(GL_LINES);
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(x+width/4.0, PALETTE_HEIGHT-PALETTE_HEIGHT/4.0f);
        glVertex2f(x+width/4.0+width/2.0, PALETTE_HEIGHT-PALETTE_HEIGHT/4.0f);
        glVertex2f(x+width/2.0, PALETTE_HEIGHT-0.25);
        glVertex2f(x+width/2.0, PALETTE_HEIGHT/2.0+0.12);
      glEnd();

      glBegin(GL_QUADS);
        glColor3f(SELECTED_COLOR.x, SELECTED_COLOR.y, SELECTED_COLOR.z);
        glVertex2f(x, PALETTE_HEIGHT/2.0f-0.12);
        glVertex2f(x+width, PALETTE_HEIGHT/2.0f-0.12);
        glVertex2f(x+width, 0.0);
        glVertex2f(x, 0.0);
      glEnd();
      glBegin(GL_LINES);
        glColor3f(0.0f, 0.0f, 0.0f);
        glVertex2f(x+width/4.0, PALETTE_HEIGHT/4.0f);
        glVertex2f(x+width/4.0+width/2.0, PALETTE_HEIGHT/4.0f);
      glEnd();

      x += width + 0.12;
    }

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
  }
}

void edit_model(int id) {
  if (id < LOADED_MODELS.size()) {
    bool confirmed = true;
    if (UNSAVED_BUFFER) confirmed = prompt_save();
    if (confirmed) {
      model3d temp_model(LOADED_MODELS[id]);
      LOADED_MODELS[id] = WORKING_MODEL;
      WORKING_MODEL = temp_model;

      DRAW_MODELS[id] = false;
    }
  }
}

bool prompt_save() {
  cout << "There are unsaved changes to the current model. " << endl << " Continue without saving? (yes/no) ";
  string input;
  getline(cin, input);
  if (input[0] == 'y' || input[0] == 'Y') {
    UNSAVED_BUFFER = false;
    return true;
  }
  return false;
}

void define_cube() {
  RUBIX.clear();

  vect3f cube_pos(-UNIT_SIZE*CUBE_COUNT/2.0, -UNIT_SIZE*CUBE_COUNT/2.0, -UNIT_SIZE*CUBE_COUNT/2.0);

  for (int i=0;i<CUBE_COUNT;i++) {
    for (int j=0;j<CUBE_COUNT;j++) {
      for (int k=0;k<CUBE_COUNT;k++) {
        RUBIX.push_back(cube());
        RUBIX.back().initialize(cube_pos+vect3f(UNIT_SIZE*i, UNIT_SIZE*j, UNIT_SIZE*k), UNIT_SIZE);
        RUBIX.back().set_solid(false);
        RUBIX.back().set_color(vect3f(0.0, 0.2, 0.0));
        RUBIX.back().set_highlight(vect3f(0.6, 0.6, 0.6));
      }
    }
  }
}

void save_branch(void*) {
  ALREADY_BRANCHED = true;

  glutIconifyWindow();

  string filename;
  cout << "[SAVE] Enter filename: ";
  getline(cin, filename);

  cout << "Saving...";

  WORKING_MODEL.save(filename);
  cout << " done. (file: " << filename << ")" << endl;

  refresh();

  glutShowWindow();

  UNSAVED_BUFFER = false;

  ALREADY_BRANCHED = false;
}

void load_branch(void*) {
  ALREADY_BRANCHED = true;

  glutIconifyWindow();

  bool confirmed = true;
  if (UNSAVED_BUFFER) confirmed = prompt_save();

  if (confirmed) {
    string filename;

    cout << "[LOAD] Enter filename: ";
    getline(cin, filename);

    model3d loaded_model;

    // add a check to see if current model is saved...

    if (WORKING_MODEL.load(filename)) {
      cout << "Loaded model. (file: " << filename << ")" << endl;
    }
    else cout << "Error loading model. (file: " << filename << ")" << endl;

    refresh();
  }

  glutShowWindow();

  ALREADY_BRANCHED = false;
}

void define_grid_branch(void*) {
  ALREADY_BRANCHED = true;

  glutIconifyWindow();

  string input;
  cout << "Define grid unit size: ";
  getline(cin, input);

  UNIT_SIZE = atof(input.c_str());

  cout << "Define grid width (in unit squares): ";
  getline(cin, input);

  CUBE_COUNT = atof(input.c_str());

  define_cube();
  refresh();

  glutShowWindow();

  ALREADY_BRANCHED = false;
}

void quit_branch(void*) {
  ALREADY_BRANCHED = true;

  glutIconifyWindow();

  cout << "Are you sure you want to quit? (y/n) ";
  string input;
  getline(cin, input);
  if (input[0] == 'y' || input[0] == 'Y') {
    bool confirmed = true;
    if (UNSAVED_BUFFER) confirmed = prompt_save();

    if (confirmed) exit(0); // quit the program
  }

  glutShowWindow();

  ALREADY_BRANCHED = false;
}

void merge_model_branch(void*) {
  ALREADY_BRANCHED = true;
  glutIconifyWindow();

  bool confirmed = true;
  if (UNSAVED_BUFFER) confirmed = prompt_save();

  if (confirmed) {
    cout << "Merge current edited model with which model number? ";
    string input;
    getline(cin, input);

    int model_id = atoi(input.c_str())-1;

    if (model_id < LOADED_MODELS.size() && model_id > -1) {
      cout << "Merging...";
      const vector<vect3f>* const coordinate_data = LOADED_MODELS[model_id].get_coordinates_ptr();
      const vector<vector<facet>>* const facet_data = LOADED_MODELS[model_id].get_facet_data_ptr();
      for (int i=0;i<facet_data->size();i++) {
        WORKING_MODEL.push_face();
        for (int j=0;j<(*facet_data)[i].size();j++) {
          WORKING_MODEL.add_vertex((*coordinate_data)[(*facet_data)[i][j].id], (*facet_data)[i][j].color);
        }
      }
      cout << " done." << endl;
    }
    else cout << "Invalid model number." << endl;
  }

  glutShowWindow();
  ALREADY_BRANCHED = false;
}

void face_resolution_branch(void*) {
  ALREADY_BRANCHED = true;
  glutIconifyWindow();

  cout << "Set current face (size: " << (*(WORKING_MODEL.get_facet_data_ptr())).back().size() << ") to how many polygons? ";
  string input;
  getline(cin, input);

  cout << "Building face...";
  WORKING_MODEL.face_resolution(atoi(input.c_str()));
  cout << " done." << endl;

  glutShowWindow();
  ALREADY_BRANCHED = false;
}

void translate_model_branch(void*) {
  ALREADY_BRANCHED = true;
  glutIconifyWindow();

  cout << "Translate Direction: (x/y/z) ";
  string input;
  getline(cin, input);

  vect3f direction;
  if (input[0] == 'x' || input[0] == 'X')      direction.x = 1;
  else if (input[0] == 'y' || input[0] == 'Y') direction.y = 1;
  else if (input[0] == 'z' || input[0] == 'Z') direction.z = 1;

  int magnitude;
  cout << "Magnitude: ";
  getline(cin, input);
  magnitude = atoi(input.c_str());

  cout << "Translating model...";
  const vector<vect3f>* const coords = WORKING_MODEL.get_coordinates_ptr();

  for (int i=0;i<(*coords).size();i++) {
    WORKING_MODEL.edit_coord(i, (*coords)[i]+direction*magnitude);
  }
  cout << " done." << endl;

  UNSAVED_BUFFER = true;

  glutShowWindow();
  ALREADY_BRANCHED = false;
}

// polygon faces don't transfer correctly >.<
void transform_model_branch(void*) {
  ALREADY_BRANCHED = true;
  glutIconifyWindow();

  cout << "Transform invert axis: (x/y/z) ";
  string input;
  getline(cin, input);

  vect3f direction(1.0, 1.0, 1.0), normal;
  if (input[0] == 'x' || input[0] == 'X') {
    direction.x = -1.0;
    normal.x = 1.0;
  }
  else if (input[0] == 'y' || input[0] == 'Y') {
    direction.y = -1.0;
    normal.y = 1.0;
  }
  else if (input[0] == 'z' || input[0] == 'Z') {
    direction.z = -1.0;
    normal.z = 1.0;
  }

  cout << "Translating model...";
  const vector<vect3f>* const coords = WORKING_MODEL.get_coordinates_ptr();
  const vector<vector<facet>>* const facets = WORKING_MODEL.get_facet_data_ptr();

  for (int i=0;i<(*coords).size();i++) {
    vect3f new_point = (*coords)[i];
    new_point.x *= direction.x;
    new_point.y *= direction.y;
    new_point.z *= direction.z;
    WORKING_MODEL.edit_coord(i, new_point);
  }

  for (int i=0;i<(*facets).size();i++) {
    int face_size = (*facets)[i].size();
    if (face_size > 0) {
      vect3f cross(((*facets)[i][0].normal).cross(normal));
      if (cross.x + cross.y + cross.z == 0) { // the face is facing the switched axis
        for (int j=0;j<face_size;j++) {
          facet front_facet = (*facets)[i][j]; // the j'th facet from the front
          facet back_facet = (*facets)[i][(*facets)[i].size()-j-1]; // the j'th facet from the back

          // swap the order of the facets:
          WORKING_MODEL.edit_vertex(index2d(i, j), back_facet);
          WORKING_MODEL.edit_vertex(index2d(i, (*facets)[i].size()-j-1), front_facet);

          face_size--; // only swap the first half
        }
      }
    }
  }
  WORKING_MODEL.recalculate_normals();
  cout << " done." << endl;

  UNSAVED_BUFFER = true;

  glutShowWindow();
  ALREADY_BRANCHED = false;
}

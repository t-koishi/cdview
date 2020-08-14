#ifndef KS_GL_H
#define KS_GL_H

#ifdef MSVC
#include <windows.h>
#endif

#include "ks_std.h"
#include <GL/gl.h>

enum {
  KS_GL_ATOM_SIZE_SMALL,
  KS_GL_ATOM_SIZE_NORMAL,
  KS_GL_ATOM_SIZE_LARGE,
  KS_GL_ATOM_SIZE_NUM
};
enum {
  KS_GL_ATOM_BRIGHT_NORMAL,
  KS_GL_ATOM_BRIGHT_LOW,
  KS_GL_ATOM_BRIGHT_HIGH,
  KS_GL_ATOM_BRIGHT_NUM
};
enum {
  KS_GL_ATOM_TRANS_OFF,
  KS_GL_ATOM_TRANS_NORMAL,
  KS_GL_ATOM_TRANS_NUM
};
enum {
  KS_GL_ATOM_COLOR_ATOM        = 0,
  KS_GL_ATOM_COLOR_ATOM_NUM    = 1,
  KS_GL_ATOM_COLOR_AMINO       = KS_GL_ATOM_COLOR_ATOM+KS_GL_ATOM_COLOR_ATOM_NUM,
  KS_GL_ATOM_COLOR_AMINO_NUM   = 20,
  KS_GL_ATOM_COLOR_CHARGE      = KS_GL_ATOM_COLOR_AMINO+KS_GL_ATOM_COLOR_AMINO_NUM,
  KS_GL_ATOM_COLOR_CHARGE_NUM  = 11,
  KS_GL_ATOM_COLOR_UNKNOWN     = KS_GL_ATOM_COLOR_CHARGE+KS_GL_ATOM_COLOR_CHARGE_NUM,
  KS_GL_ATOM_COLOR_UNKNOWN_NUM = 6,
  KS_GL_ATOM_COLOR_NUM         = KS_GL_ATOM_COLOR_UNKNOWN+KS_GL_ATOM_COLOR_UNKNOWN_NUM
};

#define KS_GL_STEREO_NOT_USE 0
#define KS_GL_STEREO_LEFT    1
#define KS_GL_STEREO_RIGHT   2

#define KS_GL_BASE_STEREO_NOT_USE         0
#define KS_GL_BASE_STEREO_QUAD_BUFFER     1
#define KS_GL_BASE_STEREO_HORIZONTAL_SLIT 2
#define KS_GL_BASE_STEREO_VERTICAL_SLIT   3

#define KS_GL_BASE_PERSPECTIVE 0
#define KS_GL_BASE_ORTHO       1

#define KS_GL_SELECT_BUFFER_SIZE 512

#define KS_GL_SELECTED_EMISSION 0.6

typedef struct KS_GL_BASE KS_GL_BASE;

typedef struct KS_GL_KEY_FUNC KS_GL_KEY_FUNC;
struct KS_GL_KEY_FUNC{
  int key;
  void (*func)(KS_GL_BASE*,int,int,void*);
  int active_flags;
  int *state;
  int state_add, state_range[2];
  char *comment;
  void *vp;
  KS_GL_BASE *gb;
  KS_GL_KEY_FUNC *next;
};
typedef struct {
  float value;
  KS_GL_KEY_FUNC *p;
} KS_GL_KEY_FUNC_SORT_BUF;

enum {
  KS_GL_FONT_TIMES_ROMAN_24,
  KS_GL_FONT_TIMES_ROMAN_10,
  KS_GL_FONT_HELVETICA_18,
  KS_GL_FONT_HELVETICA_12,
  KS_GL_FONT_HELVETICA_10,
  KS_GL_FONT_8x13,
  KS_GL_FONT_9x15,
  KS_GL_FONT_NUM
};

enum {
  KS_GL_VIEWPORT_X,
  KS_GL_VIEWPORT_Y,
  KS_GL_VIEWPORT_WIDTH,
  KS_GL_VIEWPORT_HEIGHT,
  KS_GL_VIEWPORT_NUM
};

enum {
  KS_GL_WINDOW_SIZE_WIDTH,
  KS_GL_WINDOW_SIZE_HEIGHT,
  KS_GL_WINDOW_SIZE_NUM
};

typedef struct KS_GL_FONT KS_GL_FONT;
struct KS_GL_FONT{
  int id;
  GLuint base;
  void *type;
  int height;
};

enum {
  KS_GL_INPUT_KEY_MAX = 128
};

typedef struct KS_GL_INPUT_KEY KS_GL_INPUT_KEY;
struct KS_GL_INPUT_KEY{
  BOOL mode;
  int font_id;
  //  GLfloat transparency;
  int cursor_pos;
  char message[KS_GL_INPUT_KEY_MAX];
  char text[KS_GL_INPUT_KEY_MAX];
  KS_GL_BASE *gb;
  BOOL bool_flg;
  void *vp;
  void (*return_func)(KS_GL_BASE*,char*,void*);
};

#define KS_GL_BUFFER_3D_DEFAULT (unsigned int)0x01
#define KS_GL_BUFFER_3D_NORMAL  (unsigned int)0x02
#define KS_GL_BUFFER_3D_SPLINE  (unsigned int)0x04

typedef struct KS_GL_BUFFER_3D KS_GL_BUFFER_3D;
struct KS_GL_BUFFER_3D{
  double *x, *y, *z;
  double *nx, *ny, *nz;
  double *h, *d;
  double *p, *a, *b, *c;
  int *label;
  unsigned int *flags;
  int grow;
  size_t num;
  size_t size;
  GLfloat **color;
};

enum {
  KS_GL_WINDOW_FRAME_SIZE = 6,
  KS_GL_WINDOW_TITLE_BAR_SIZE = 18,
  KS_GL_WINDOW_CLOSE_WIDTH = 8,
  KS_GL_WINDOW_CLOSE_HEIGHT= 8,
  KS_GL_WINDOW_CLOSE_X = -KS_GL_WINDOW_CLOSE_WIDTH,
  KS_GL_WINDOW_CLOSE_Y = 3
};

#define KS_GL_WINDOW_SHOW              (unsigned int)0x001
#define KS_GL_WINDOW_PICK_MAIN         (unsigned int)0x002
#define KS_GL_WINDOW_PICK_TITLE_BAR    (unsigned int)0x004
#define KS_GL_WINDOW_PICK_FRAME_TOP    (unsigned int)0x008
#define KS_GL_WINDOW_PICK_FRAME_BOTTOM (unsigned int)0x010
#define KS_GL_WINDOW_PICK_FRAME_LEFT   (unsigned int)0x020
#define KS_GL_WINDOW_PICK_FRAME_RIGHT  (unsigned int)0x040
#define KS_GL_WINDOW_PICK_ALL          (unsigned int)0x07e
#define KS_GL_WINDOW_FRAME_FIX         (unsigned int)0x080
#define KS_GL_WINDOW_CLOSE             (unsigned int)0x100
#define KS_GL_WINDOW_NO_FRAME          (unsigned int)0x200

typedef struct KS_GL_WINDOW KS_GL_WINDOW;
struct KS_GL_WINDOW{
  int id;
  unsigned int flags;
  char name[64];
  int viewport[KS_GL_VIEWPORT_NUM];
  void *vp;
  void (*display)(KS_GL_BASE*,int,int,int,void*);
  void (*mouse)(KS_GL_BASE*,int,int,int,int,int,int,int,void*);
  void (*motion)(KS_GL_BASE*,int,int,int,int,int,void*);
  void (*passive)(KS_GL_BASE*,int,int,int,int,int,void*);
  KS_GL_WINDOW *next;
};

enum {
  KS_GL_BASE_VERTEX_BUF_NUM = 2
};
enum {
  KS_GL_RIBBON_A_1,
  KS_GL_RIBBON_A_2,
  KS_GL_RIBBON_A_3,
  KS_GL_RIBBON_A_4,
  KS_GL_RIBBON_A_5,
  KS_GL_RIBBON_A_NUM
};
enum {
  KS_GL_RIBBON_B_1,
  KS_GL_RIBBON_B_2,
  KS_GL_RIBBON_B_3,
  KS_GL_RIBBON_B_4,
  KS_GL_RIBBON_B_5,
  KS_GL_RIBBON_B_NUM
};
enum {
  KS_GL_RIBBON_VERTEX_NUM = 12+1
};

enum {
  KS_GL_MOUSE_TIME_NOW,
  KS_GL_MOUSE_TIME_LAP,
  KS_GL_MOUSE_TIME_NUM
};

#ifdef GLUT
enum {
  KS_GL_MOUSE_BUTTON_LEFT   = GLUT_LEFT_BUTTON,
  KS_GL_MOUSE_BUTTON_RIGHT  = GLUT_RIGHT_BUTTON,
  KS_GL_MOUSE_BUTTON_MIDDLE = GLUT_MIDDLE_BUTTON,
  KS_GL_MOUSE_BUTTON_NUM = 3
};
enum {
  KS_GL_MOUSE_STATE_UP   = GLUT_UP,
  KS_GL_MOUSE_STATE_DOWN = GLUT_DOWN,
  KS_GL_MOUSE_STATE_MOTION = 2,
  KS_GL_MOUSE_STATE_NUM = 3
};
enum {
  KS_GL_KEY_F1        = GLUT_KEY_F1,
  KS_GL_KEY_F2        = GLUT_KEY_F2,
  KS_GL_KEY_F3        = GLUT_KEY_F3,
  KS_GL_KEY_F4        = GLUT_KEY_F4,
  KS_GL_KEY_F5        = GLUT_KEY_F5,
  KS_GL_KEY_F6        = GLUT_KEY_F6,
  KS_GL_KEY_F7        = GLUT_KEY_F7,
  KS_GL_KEY_F8        = GLUT_KEY_F8,
  KS_GL_KEY_F9        = GLUT_KEY_F9,
  KS_GL_KEY_F10       = GLUT_KEY_F10,
  KS_GL_KEY_F11       = GLUT_KEY_F11,
  KS_GL_KEY_F12       = GLUT_KEY_F12,
  KS_GL_KEY_LEFT      = GLUT_KEY_LEFT,
  KS_GL_KEY_UP        = GLUT_KEY_UP,
  KS_GL_KEY_RIGHT     = GLUT_KEY_RIGHT,
  KS_GL_KEY_DOWN      = GLUT_KEY_DOWN,
  KS_GL_KEY_PAGE_UP   = GLUT_KEY_PAGE_UP,
  KS_GL_KEY_PAGE_DOWN = GLUT_KEY_PAGE_DOWN,
  KS_GL_KEY_HOME      = GLUT_KEY_HOME,
  KS_GL_KEY_END       = GLUT_KEY_END,
  KS_GL_KEY_INSERT    = GLUT_KEY_INSERT
};
#define KS_GL_ACTIVE_SHIFT GLUT_ACTIVE_SHIFT
#define KS_GL_ACTIVE_CTRL  GLUT_ACTIVE_CTRL
#define KS_GL_ACTIVE_ALT   GLUT_ACTIVE_ALT
#else
enum {
  KS_GL_MOUSE_BUTTON_LEFT,
  KS_GL_MOUSE_BUTTON_RIGHT,
  KS_GL_MOUSE_BUTTON_MIDDLE,
  KS_GL_MOUSE_BUTTON_NUM = 3
};
enum {
  KS_GL_MOUSE_STATE_UP,
  KS_GL_MOUSE_STATE_DOWN,
  KS_GL_MOUSE_STATE_MOTION = 2,
  KS_GL_MOUSE_STATE_NUM = 3
};
enum {
  KS_GL_KEY_F1,
  KS_GL_KEY_F2,
  KS_GL_KEY_F3,
  KS_GL_KEY_F4,
  KS_GL_KEY_F5,
  KS_GL_KEY_F6,
  KS_GL_KEY_F7,
  KS_GL_KEY_F8,
  KS_GL_KEY_F9,
  KS_GL_KEY_F10,
  KS_GL_KEY_F11,
  KS_GL_KEY_F12,
  KS_GL_KEY_LEFT,
  KS_GL_KEY_UP,
  KS_GL_KEY_RIGHT,
  KS_GL_KEY_DOWN,
  KS_GL_KEY_PAGE_UP,
  KS_GL_KEY_PAGE_DOWN,
  KS_GL_KEY_HOME,
  KS_GL_KEY_END,
  KS_GL_KEY_INSERT
};
#define KS_GL_ACTIVE_SHIFT 0x01
#define KS_GL_ACTIVE_CTRL  0x02
#define KS_GL_ACTIVE_ALT   0x04
#endif
#define KS_GL_ACTIVE_NO_GET 0x100

enum {
  KS_GL_PLANE_X,
  KS_GL_PLANE_Y,
  KS_GL_PLANE_Z
};

enum {
  KS_GL_LOOKAT_PREV,
  KS_GL_LOOKAT_NOW,
  KS_GL_LOOKAT_NEXT,
  KS_GL_LOOKAT_NUM
};

typedef struct KS_GL_MOUSE_MENU_STATE KS_GL_MOUSE_MENU_STATE;
struct KS_GL_MOUSE_MENU_STATE{
  int state;
  int button;
  int point;
  int select;
};


#define KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION 1.3
#define KS_GL_KEY_HELP_KEY_LEN 45

struct KS_GL_BASE{
  unsigned int stereo;
  unsigned int projection;
  unsigned int verbose_level,verbose_level_max;

  int active_flags;

  int eye_flg;
  double eye_width;
  double eye_len;
  double eye_ang;

  double trans[3];
  double angle[3];

  int mouse_state[KS_GL_MOUSE_BUTTON_NUM];
  int mouse_pos[KS_GL_MOUSE_BUTTON_NUM][KS_GL_MOUSE_STATE_NUM][2];
  BOOL mouse_angle_change;

  GLfloat foreground_color[4];
  GLfloat background_color[4];

  double rot_mat[16];
  double rot_imat[16];
  double init_mat[16];

  double time;

  GLuint select_buffer[KS_GL_SELECT_BUFFER_SIZE];
  GLint hits;
  int hit_name_num;
  GLuint *hit_name;

  KS_GL_KEY_FUNC *key_func;
  KS_GL_KEY_FUNC *special_key_func;

  KS_GL_INPUT_KEY input;

  KS_GL_FONT font[KS_GL_FONT_NUM];

  int rod_vnum;
  GLfloat ***rod_v;
  GLfloat **arrow_n;
  KS_CIRCLE_BUFFER *tube;
  double **verbuf[KS_GL_BASE_VERTEX_BUF_NUM];
  KS_GL_BUFFER_3D *ribbon_a,*ribbon_b;
  double **ribbon_v[KS_GL_BASE_VERTEX_BUF_NUM];
  double **ribbon_n[KS_GL_BASE_VERTEX_BUF_NUM];
  KS_GL_BUFFER_3D *hokan;

  double double_click_time;
  double mouse_time[KS_GL_MOUSE_TIME_NUM][KS_GL_MOUSE_BUTTON_NUM][KS_GL_MOUSE_STATE_NUM];

  double *move_target;
  double *move_target_pos;
  double move_target_initial[3];
  double move_target_motion_scale;
  int move_target_plane;
  BOOL move_target_motion;

  double rotation_center[3];
  double rotation_center_offset[3];

  double lookat[KS_GL_LOOKAT_NUM][3];
  double trans_prev[3],trans_next[3];
  BOOL lookat_moving;
  double lookat_moving_time,lookat_moving_start_time;
  double lookat_stack[3],trans_stack[3];

  void (*display_func)(void);
  KS_GL_WINDOW *window,*pick_window;
  int window_id_max;

  int key_help_key,key_help_font;
  int key_help_window_id;
  int key_help_point_pos;

  int window_size[KS_GL_WINDOW_SIZE_NUM];
  int viewport_3d[KS_GL_VIEWPORT_NUM];
  int viewport_2d[KS_GL_VIEWPORT_NUM];
  /*
  int viewport[KS_GL_VIEWPORT_NUM];
  */

  GLdouble depth_3d_near,depth_3d_far;
  GLdouble depth_2d_near,depth_2d_far;

#ifdef ENABLE_DOME
  int dome_id;
#endif
};
/*
typedef struct KS_GL_TEXTURE KS_GL_TEXTURE;
struct KS_GL_TEXTURE{
  char *file_name;
  char *name;
  int w,h;
  GLuint label;
  KS_GL_TEXTURE *next;
};
*/

#define KS_GL_GRAPH_AUTO       (unsigned int)0x01
#define KS_GL_GRAPH_FIXED      (unsigned int)0x02
#define KS_GL_GRAPH_DRAW_LINE  (unsigned int)0x04
#define KS_GL_GRAPH_DRAW_SCALE (unsigned int)0x08

typedef struct KS_GL_GRAPH_2D KS_GL_GRAPH_2D;
struct KS_GL_GRAPH_2D{
  unsigned int flags;
  char name[24];
  int font_id;
  GLfloat color[3];
  GLfloat dot_size;
  GLfloat dot_color[3];
  int size;
  int num;
  int current;
  double scale_unit;
  double *data;
  double range[2];
};

/*typedef GLuint**** KS_GL_ATOM;*/
typedef struct KS_GL_ATOM KS_GL_ATOM;
struct KS_GL_ATOM{
  GLuint**** atom;
  float *radius;
  GLfloat ***color;
  GLfloat **emission;
  int *detail;
  float *scale;
};

typedef struct KS_GL_SPHERE_OBJ KS_GL_SPHERE_OBJ;
struct KS_GL_SPHERE_OBJ {
  int s_num1,s_num2;
  double ***cd;
};

enum {
  KS_DRAW_GL_ROD_COVER_NON,
  KS_DRAW_GL_ROD_COVER_START,
  KS_DRAW_GL_ROD_COVER_END,
  KS_DRAW_GL_ROD_COVER_START_AND_END
};

enum {
  KS_DRAW_GL_BOX_FRONT,
  KS_DRAW_GL_BOX_BACK,
  KS_DRAW_GL_BOX_FRONT_AND_BACK
};

enum {
  KS_DRAW_GL_RIBBON_USER_COLOR,
  KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR,
  KS_DRAW_GL_RIBBON_RAINBOW,
  KS_DRAW_GL_RIBBON_RAINBOW_LOW
};

enum {
  KS_GL_RINGO_TYPE_AKA,
  KS_GL_RINGO_TYPE_AO,
  KS_GL_RINGO_TYPE_BRIGHT,
  KS_GL_RINGO_TYPE_NUM
};

// these definition is the same as that in ks_md.h
#define KS_GL_RESIDUE_NORMAL            (unsigned int)0x001
#define KS_GL_RESIDUE_START             (unsigned int)0x002
#define KS_GL_RESIDUE_END               (unsigned int)0x004
//#define KS_GL_RESIDUE_MPI_RANGE_OUTSIDE (unsigned int)0x008
#define KS_GL_RESIDUE_ALPHA             (unsigned int)0x010
#define KS_GL_RESIDUE_BETA              (unsigned int)0x020
#define KS_GL_RESIDUE_LOOP              (unsigned int)0x040
#define KS_GL_RESIDUE_BETA_N_BOND       (unsigned int)0x080
#define KS_GL_RESIDUE_BETA_O_BOND       (unsigned int)0x100
#define KS_GL_RESIDUE_SELECTED          (unsigned int)0x200

#ifdef USE_GLSL
// type of standard shader
enum {
  KS_GL_STANDARD_SHADER_SPHERE,          // no rot, no color (for sphere)
  KS_GL_STANDARD_SHADER_TEXTURED_SPHERE, // no color (for textured sphere)
  KS_GL_STANDARD_SHADER_REGULAR,         // regular shader
  KS_GL_STANDARD_SHADER_POINT            // for GL_POINTS
};

typedef struct {
  GLfloat elem[3];
} KS_GL_FLOAT_ELEM3;

typedef struct {
  GLfloat trans[3];
  GLfloat angle_euler_zyz[3];
  GLfloat scale[3];
} KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM;

typedef struct {
  int elem_cnt;
  int elem_capacity;   // elem malloc count
  KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM *elem;
  size_t model_view_size;
  GLfloat *model_view;
} KS_GL_SHADER_REGULAR_DRAW_INFO;

typedef struct {                // parameters for instanced rendering for regular shader
  GLfloat x,y,z;                // vertex position
  GLfloat nx,ny,nz;             // vertex normal vector
  GLfloat r,g,b;                // vertex color
} KS_GL_SHADER_REGULAR_PARM_ELEM;

typedef struct {
  KS_GL_SHADER_REGULAR_PARM_ELEM *elem;  // element of vertices
  size_t elem_size;
  int index_cnt;
  GLushort *index;                     // indices of vertices
  size_t index_size;
} KS_GL_SHADER_REGULAR_PARM;

typedef struct {                // parameters for instanced rendering for sphere
  GLfloat x,y,z,size;           // position and size
  GLfloat r,g,b,em;             // color and emission
} KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM;

typedef struct {
  int elem_cnt;        // elem count
  int elem_capacity;   // elem malloc count
  KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM* elem;
} KS_GL_SHADER_SPHERE_DRAW_INFO;

typedef struct {
  int vertex_cnt,index_cnt;
  KS_GL_FLOAT_ELEM3 *vertex;
  GLushort *index;
} KS_GL_SHADER_SPHERE_PARM;

typedef struct {
  GLint unit;
  GLuint texture_id;
} KS_GL_SHADER_TEXTURE_INFO;

typedef struct {                // parameters for instanced rendering for TEXTURED_SPHERE
  GLfloat x,y,z;                // vertex position
  GLfloat u,v;                  // vertex u,v
} KS_GL_SHADER_TEXTURED_SPHERE_PARM_ELEM;

typedef struct {
  KS_GL_SHADER_TEXTURED_SPHERE_PARM_ELEM *elem;  // element of vertices
  size_t elem_size;
  int index_cnt;
  GLushort *index;                     // indices of vertices
  size_t index_size;
} KS_GL_SHADER_TEXTURED_SPHERE_PARM;

typedef struct {
  GLfloat light_position[3];
  GLfloat light_ambient;
  GLfloat material_shininess;
  GLfloat material_specular;
} KS_GL_STANDARD_SHADER_PARM;

// color mode of shader
enum {
  KS_GL_SHADER_COLOR_MODE_LIGHTING,  // lighting color
  KS_GL_SHADER_COLOR_MODE_DIRECT,    // non-lighting color (setting color is used directly)
  KS_GL_SHADER_COLOR_MODE_PICK,      // picking color (color is changed for each instance)
  KS_GL_SHADER_COLOR_MODE_CNT
};

enum {
  KS_GL_SHADER_PICK_ADJUST = 1  // adjust number of avoid zero value when background is selected
};

// type of vbo
enum {
  KS_GL_SHADER_SPHERE_VBO_VERTEX,                // vbo for shape
  KS_GL_SHADER_SPHERE_VBO_PROPERTY,              // vbo for draw
  KS_GL_SHADER_SPHERE_VBO_CNT
};

enum {
  KS_GL_SHADER_REGULAR_VBO_VERTEX,              // vbo for shape
  KS_GL_SHADER_REGULAR_VBO_MODEL_VIEW_MATRIX,   // vbo for draw
  KS_GL_SHADER_REGULAR_VBO_CNT
};

enum {
  KS_GL_SHADER_TEXTURED_SPHERE_VBO_VERTEX,             // vbo for shape
  KS_GL_SHADER_TEXTURED_SPHERE_VBO_MODEL_VIEW_MATRIX,  // vbo for draw
  KS_GL_SHADER_TEXTURED_SPHERE_VBO_CNT
};

enum {
  KS_GL_SHADER_POINT_VBO_VERTEX,                // vbo for shape
  KS_GL_SHADER_POINT_VBO_PROPERTY,              // vbo for draw
  KS_GL_SHADER_POINT_VBO_CNT 
};

typedef struct {
  int color_mode;
  unsigned int pick_id_offset;
  GLenum draw_mode;
  GLuint vao[KS_GL_SHADER_COLOR_MODE_CNT];
  GLuint *vbo;
  GLuint vbi;
  int vbo_cnt;
  int draw_elem_cnt;                                  // number of elements for instanced rendering
  int index_cnt;                                    // indices count for regular shader
} KS_GL_SHADER_OBJECT;

// type of attribute variables
enum {
  KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION,
  KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR,
  KS_GL_SHADER_IN_LOCATION_VERTEX_NORMAL,
  KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX,
  KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION,
  KS_GL_SHADER_IN_LOCATION_VERTEX_UV,
  KS_GL_SHADER_IN_LOCATION_CNT
};

// type of uniform variables
enum {
  KS_GL_SHADER_UNIFORM_LOCATION_VIEW_MATRIX,
  KS_GL_SHADER_UNIFORM_LOCATION_PROJECTION_MATRIX,
  KS_GL_SHADER_UNIFORM_LOCATION_SAMPLER2D,
  KS_GL_SHADER_UNIFORM_LOCATION_CNT
};

typedef struct {
  int shader_type;
  GLuint **program;        // shader program
  // location for in variable of shader
  GLuint in_loc[KS_GL_SHADER_IN_LOCATION_CNT][KS_GL_SHADER_COLOR_MODE_CNT];
  // location for uniform variable of shader
  GLuint ***uniform_loc;
  KS_GL_SHADER_OBJECT *obj;
  int obj_cnt;
} KS_GL_SHADER_INFO;

// type of sphere parameters in generate_sphere_parm()
enum {
  GENERATE_SPHERE_PARM_SPHERE,
  GENERATE_SPHERE_PARM_TEXTURED_SPHERE
};

#endif /* USE_GLSL */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  KS_GL_BASE *ks_allocate_gl_base(void);
  void ks_free_gl_base(KS_GL_BASE *gb);
  void ks_glutInit(int* acp, char** av);
  void ks_init_glut_font(KS_GL_BASE *gb);
  BOOL ks_use_glut_font(KS_GL_BASE *gb, int font);
  /*
  BOOL ks_make_glut_font(KS_GL_BASE *gb, void *font_type, char *font_name);
  KS_GL_FONT *ks_lookup_gl_font(KS_GL_BASE *gb, char *name);
  */
  BOOL ks_get_glut_font_width(KS_GL_BASE *gb, int font, char *str, int *len);
  BOOL ks_get_glut_font_height(KS_GL_BASE *gb, int font, int *len);
  void ks_hsv2rgb(double h, double s, double v, double *r, double *g ,double *b);
  int ks_save_gl_bmp(char *file_name, BOOL swap_endian, int *viewport);
  void ks_draw_gl_line(double x0, double y0, double z0, double x1, double y1, double z1,
		       double cr, double cg, double cb, double width);
  void ks_draw_gl_box(double x0, double y0, double z0, double x1, double y1, double z1, int type);
  void ks_draw_gl_box_line(double x0, double y0, double z0, double x1, double y1, double z1);
  void ks_draw_glut_sphere(double x0, double y0, double z0,
			   double cr, double cg, double cb, double ct, double r, int n0, int n1);
  /*void ks_multi_gl_mat(KS_GL_BASE gb, double *v, double *m);*/
  void ks_multi_gl_matrix(double mat[16], double *v, double *m);
  void ks_multi_gl_rotational_matrix(KS_GL_BASE gb, double *v, double *m);
  void ks_multi_gl_inverse_rotational_matrix(KS_GL_BASE gb, double *v, double *m);
  void ks_apply_gl_initial_matrix(KS_GL_BASE *gb);
  void ks_apply_gl_rotational_matrix(KS_GL_BASE *gb);
  void ks_set_gl_rod_vertex_num(KS_GL_BASE *gb, int n);
  void ks_draw_gl_rod(KS_GL_BASE *gb, GLfloat r, GLfloat len, int type, int line_flg);
  void ks_draw_gl_rod_pos(KS_GL_BASE *gb,
			  GLfloat sx, GLfloat sy, GLfloat sz, GLfloat ex, GLfloat ey, GLfloat ez,
			  GLfloat r, int type, int line_flg);
  void ks_draw_gl_arrow(KS_GL_BASE *gb, GLfloat r, GLfloat len, int head_div, int type, 
			int line_flg);
  void ks_draw_gl_arrow_pos(KS_GL_BASE *gb,
			    GLfloat sx, GLfloat sy, GLfloat sz, GLfloat ex, GLfloat ey, GLfloat ez,
			    GLfloat r, int head_div, int type, int line_flg);
  GLuint ks_set_gl_texture(char *file_name, KS_CHAR_LIST *search_path);
  void ks_draw_gl_sphere(float r, int n);
  void ks_draw_gl_sphere_shadow_volume(float r, int n, 
				       double light_x, double light_y, double light_z, double len,
				       BOOL light_face);
  void ks_draw_gl_icosa(float r, int div_num);
  BOOL ks_draw_gl_ringo(double r, int flg);
  void ks_calc_gl_rot_inv_mat(double rmat[16], double imat[16]);
  /*  void ks_gl_base_lookat(KS_GL_BASE *gb, int stereo);*/
  /*  void ks_trans_gl(KS_GL_BASE *gb);*/
  /*  void ks_rotate_gl(KS_GL_BASE *gb);*/
  int ks_gl_base_verbose_level(KS_GL_BASE *gb);
  void ks_gl_base_clearcolor(KS_GL_BASE *gb);
  void ks_gl_base_foreground_color(KS_GL_BASE *gb);
  GLfloat *ks_get_gl_base_foreground_color(KS_GL_BASE *gb);
  void ks_gl_base_scene(KS_GL_BASE *gb, int viewport[4]);
  int ks_gl_base_get_stereo_mode(KS_GL_BASE *gb);
  void ks_gl_base_set_stereo_mode(KS_GL_BASE *gb, unsigned int mode);
  BOOL ks_gl_base_mouse(KS_GL_BASE *gb, void *vp, int button, int state, int x, int y, 
			void(*display_3d)(KS_GL_BASE*,void*));
  void ks_gl_base_motion(KS_GL_BASE *gb, int x, int y);
  void ks_gl_base_passive(KS_GL_BASE *gb, int x, int y);
  void ks_gl_call_key_func(KS_GL_BASE *gb, unsigned int key,int x, int y,unsigned int active_flags);
  void ks_gl_call_special_key_func(KS_GL_BASE *gb, unsigned int key, int x, int y,
				   unsigned int active_flags);
  void ks_gl_base_keyboard(KS_GL_BASE *gb, unsigned char key, int x, int y);
  void ks_gl_base_special_key(KS_GL_BASE *gb, int key, int x, int y);
  void ks_gl_base_set_rotational_matrix(KS_GL_BASE *gb, double ax,double ay, double az);
  void ks_gl_base_set_projection_mode(KS_GL_BASE *gb, unsigned int mode);
  unsigned int ks_gl_base_get_projection_mode(KS_GL_BASE *gb);
  void ks_gl_base_set_translational_array(KS_GL_BASE *gb, double tx,double ty, double tz,
					  double cell_size);
  void ks_set_gl_eye_position(KS_GL_BASE *gb, double pos[3]);
  void ks_gl_base_display(KS_GL_BASE *gb, void *vp,
			  void (*display_3d)(KS_GL_BASE*, void*), 
			  void (*display_2d)(KS_GL_BASE*, void*,int,int));
  void ks_gl_base_reshape(KS_GL_BASE *gb, int w, int h);

  GLuint ks_set_gl_sphere_texture(int size, GLfloat light_pos[3]);

  KS_GL_ATOM *ks_allocate_gl_atom(float radius, float *color, int detail, float *scale, 
				  BOOL use_displa_list);
  /*  KS_GL_ATOM ks_set_gl_atom(float radius, float *color, int ditail, float *scale);*/
  /*
    KS_GL_TEXTURE *ks_new_gl_texture(char *file_name);
    KS_GL_TEXTURE *ks_add_gl_texture(KS_GL_TEXTURE *listp, KS_GL_TEXTURE *newp);
  */
  /*
    BOOL ks_init_gl_atom(KS_MD_ATOM *atom, int ditail);
    void ks_draw_gl_atom(int atomic_number, int color, int size, int bright, int trans);
  */
  /*  void ks_draw_gl_atom(KS_GL_ATOM gl_atom, int color, int size, int bright, int trans);*/
  void ks_draw_gl_atom(KS_GL_ATOM *gla, int color, int size, int bright, int trans);
  GLfloat *ks_get_gl_atom_color(KS_GL_ATOM *gla, int color, int bright);
  GLfloat *ks_get_gl_atom_emission(KS_GL_ATOM *gla, int bright);
  float ks_get_gl_atom_size(KS_GL_ATOM *gla, int size);
  float ks_get_gl_atom_detail(KS_GL_ATOM *gla, int size);

  BOOL ks_get_glut_shift(KS_GL_BASE *gb);
  BOOL ks_get_glut_ctrl(KS_GL_BASE *gb);
  BOOL ks_get_glut_alt(KS_GL_BASE *gb);
  KS_GL_GRAPH_2D *ks_allocate_gl_graph_2d(char *name, int size, double min, double max, 
					  double scale_unit, 
					  GLfloat red, GLfloat green, GLfloat blue,
					  GLfloat dot_size,
					  GLfloat dot_red, GLfloat dot_green, GLfloat dot_blue,
					  int font, unsigned int flags);
  void ks_free_gl_graph_2d(KS_GL_GRAPH_2D *gra);
  void ks_set_gl_graph_2d(KS_GL_GRAPH_2D *gra, double data);
  void ks_draw_gl_graph_2d(KS_GL_BASE *gb, 
			   KS_GL_GRAPH_2D *gra, int x, int y, int width, int height, 
			   double line_value, int dot_pos);
  KS_GL_BUFFER_3D *ks_allocate_spline_buffer_3d(void);
  BOOL ks_realloc_spline_buffer_3d(KS_GL_BUFFER_3D *spline, int new_size);
  void ks_free_gl_buffer_3d(KS_GL_BUFFER_3D *buf);
  BOOL ks_set_spline_buffer_3d(KS_GL_BUFFER_3D *spline,double x, double y, double z,
			       GLfloat *color, int label);
  void ks_clear_gl_buffer_3d(KS_GL_BUFFER_3D *buf);
  BOOL ks_draw_gl_tube_spline(KS_GL_BASE *gb, KS_GL_BUFFER_3D *spline, double r, int div_num, 
			      double skip_angle, int circle_num);
  void ks_draw_gl_tube(KS_GL_BASE *gb, KS_GL_BUFFER_3D *buf,
		       double r, double skip_angle, int circle_num);
  BOOL ks_set_gl_key_func(KS_GL_BASE *gb, unsigned char key, int active_flags, 
			  void *state, int state_min, int state_max, int state_add,
			  void (*func)(KS_GL_BASE*,int,int,void*), void *vp, char *comment);
  BOOL ks_set_gl_special_key_func(KS_GL_BASE *gb, unsigned char key, int active_flags, 
				  void *state, int state_min, int state_max, int state_add,
				  void (*func)(KS_GL_BASE*,int,int,void*),void *vp,
				  char *comment);
  BOOL ks_set_gl_input_key(KS_GL_BASE *gb, char *message, char *before,int font,
			   void (*return_func)(KS_GL_BASE*,char*,void*), BOOL bool_flg, void *vp);
  void ks_draw_glut_input_key_text(KS_GL_BASE *gb, GLfloat transparency);
  BOOL ks_is_gl_input_key_mode(KS_GL_BASE *gb);
  void ks_free_gl_atom(KS_GL_ATOM *gla);
  /*  void ks_free_gl_atom(KS_GL_ATOM atom);*/
  BOOL ks_get_gl_amino_color(int i, GLfloat *color);
  BOOL ks_get_gl_charge_color(int i, GLfloat *color);
  BOOL ks_get_gl_unknown_color(int i, GLfloat *color);
  void ks_draw_gl_texture_2d(GLuint texture, GLfloat x, GLfloat y, GLfloat width, GLfloat height);
  KS_GL_BUFFER_3D *ks_allocate_gl_buffer_3d(int num, int grow, int size, unsigned int flags, 
					    char *msg);
  BOOL ks_set_gl_buffer_3d(KS_GL_BUFFER_3D *buf, 
			   double x, double y, double z, double nx, double ny, double nz,
			   GLfloat *color, int label, unsigned int flags);
  BOOL ks_init_gl_ribbon(KS_GL_BASE *gb, double scale);
  BOOL ks_draw_gl_ribbon(KS_GL_BASE *gb, KS_GL_BUFFER_3D *ribbon, int color_type, 
			 GLfloat *alpha_color, GLfloat *beta_color, GLfloat *loop_color);
  void ks_draw_gl_color_sample_right(KS_GL_BASE *gb, GLfloat x, GLfloat y, 
				     KS_SPHERE *sphere, int font,
				     int y_max, GLfloat y_sep, 
				     GLfloat sphere_size, GLuint sphere_texture);
  double ks_get_double_click_time(KS_GL_BASE *gb);
  void ks_set_double_click_time(KS_GL_BASE *gb, double t);
  BOOL ks_check_double_click_time(KS_GL_BASE *gb, double t);
  BOOL ks_is_gl_double_click(KS_GL_BASE *gb, int button, int state);
  int ks_get_gl_mouse_state(KS_GL_BASE *gb, int button);
  void ks_draw_gl_move_target_plane(KS_GL_BASE *gb, double plane_size, int line_num, 
				    GLfloat color0, GLfloat color1, GLfloat color2);
  void ks_set_gl_move_target(KS_GL_BASE *gb, double *target, double *target_pos, 
			     double motion_scale);
  void ks_clear_gl_move_target(KS_GL_BASE *gb);
  void ks_set_gl_move_target_motion(KS_GL_BASE *gb, BOOL mode);
  BOOL ks_is_gl_move_target_motion(KS_GL_BASE *gb);
  void ks_gl_base_idle(KS_GL_BASE *gb);
  void ks_set_gl_rotation_center(KS_GL_BASE *gb, double *pos);
  void ks_set_gl_lookat_position(KS_GL_BASE *gb, double *pos);
  void ks_change_gl_lookat(KS_GL_BASE *gb, double *pos, double time);
  BOOL ks_is_gl_lookat_moving(KS_GL_BASE *gb);
  double ks_get_gl_time(KS_GL_BASE *gb);
  void ks_redraw_gl(KS_GL_BASE *gb);
  BOOL ks_draw_glut_text(KS_GL_BASE *gb, char *text, int font, GLfloat transparency, int margin);
  void ks_add_gl_rotate(KS_GL_BASE *gb, double mat[16]);
  void ks_gl_base_set_display_function(KS_GL_BASE *gb,void (*display)(void));
  void ks_draw_gl(KS_GL_BASE *gb);
  void ks_set_gl_base_foreground_color(KS_GL_BASE *gb, 
				       GLfloat c0, GLfloat c1, GLfloat c2, GLfloat c3);
  void ks_set_gl_base_background_color(KS_GL_BASE *gb, 
				       GLfloat c0, GLfloat c1, GLfloat c2, GLfloat c3);
  GLboolean ks_gl_enable(GLenum capability);
  GLboolean ks_gl_disable(GLenum capability);
  void ks_increase_gl_base_verbose_level(KS_GL_BASE *gb, int x, int y, void *vp);
  void ks_decrease_gl_base_verbose_level(KS_GL_BASE *gb, int x, int y, void *vp);
  void ks_set_gl_base_verbose_level(KS_GL_BASE *gb, unsigned int level);
  void ks_set_gl_base_verbose_level_max(KS_GL_BASE *gb, unsigned int max);

  BOOL ks_set_gl_base_key_help(KS_GL_BASE *gb, int key, int font);
  unsigned int ks_add_gl_base_window(KS_GL_BASE *gb, unsigned int flags, char *name,
				     int pos_x, int pos_y, int width, int height, 
				     void (*display)(KS_GL_BASE*,int,int,int,void*),
				     void (*mouse)(KS_GL_BASE*,int,int,int,int,int,int,int,void*),
				     void (*motion)(KS_GL_BASE*,int,int,int,int,int,void*),
				     void (*passive)(KS_GL_BASE*,int,int,int,int,int,void*),
				     void *vp);
  void ks_del_gl_base_window(KS_GL_BASE *gb, int id);
  BOOL ks_show_gl_base_window(KS_GL_BASE *gb, int id);
  BOOL ks_hide_gl_base_window(KS_GL_BASE *gb, int id);
  BOOL ks_toggle_gl_base_window(KS_GL_BASE *gb, int id);
  BOOL ks_toggle_gl_base_window_frame(KS_GL_BASE *gb, int id);
  char *ks_get_gl_key_name(int key);
  BOOL ks_print_gl_key_func(KS_GL_BASE *gb);
  void ks_save_gl_pov_header(KS_GL_BASE *gb, FILE *fp);
  void ks_save_gl_pov_footer(KS_GL_BASE *gb, FILE *fp, BOOL no_shadow);
  void ks_gl_base_stop_x_rotate(KS_GL_BASE *gb);
  void ks_gl_base_stop_y_rotate(KS_GL_BASE *gb);
  void ks_gl_base_stop_z_rotate(KS_GL_BASE *gb);
  /*
  int ks_get_gl_viewport_width(KS_GL_BASE *gb);
  int ks_get_gl_viewport_height(KS_GL_BASE *gb);
  */
  int *ks_get_gl_viewport_3d(KS_GL_BASE *gb);
  int ks_get_gl_viewport_3d_width(KS_GL_BASE *gb);
  int ks_get_gl_viewport_3d_height(KS_GL_BASE *gb);
  int *ks_get_gl_viewport_2d(KS_GL_BASE *gb);
  int ks_get_gl_viewport_2d_width(KS_GL_BASE *gb);
  int ks_get_gl_viewport_2d_height(KS_GL_BASE *gb);

  void ks_io_gl_base(KS_IO io, KS_GL_BASE *gb);
  void ks_set_gl_base_depth_3d_near(KS_GL_BASE *gb, GLdouble d);
  void ks_set_gl_base_depth_3d_far(KS_GL_BASE *gb, GLdouble d);
  void ks_set_gl_base_depth_2d_near(KS_GL_BASE *gb, GLdouble d);
  void ks_set_gl_base_depth_2d_far(KS_GL_BASE *gb, GLdouble d);
  void ks_get_gl_eye_pos(KS_GL_BASE *gb, double rpos[3]);
  void ks_init_gl_rot_mat(double mat[16]);
  double ks_get_gl_perspective_angle(KS_GL_BASE *gb);
  void ks_set_gl_perspective_angle(KS_GL_BASE *gb, double angle);
  void ks_push_gl_lookat(KS_GL_BASE *gb);
  void ks_pop_gl_lookat(KS_GL_BASE *gb);
  void ks_free_gl_key_func(KS_GL_BASE *gb);
  void ks_set_gl_base_eye_len(KS_GL_BASE *gb, double len);
  double ks_get_gl_base_eye_len(KS_GL_BASE *gb);
  void ks_set_gl_base_eye_width(KS_GL_BASE *gb, double width);
  double ks_get_gl_base_eye_width(KS_GL_BASE *gb);
  double *ks_get_gl_modelview_matrix(KS_GL_BASE *gb, double mat[16]);
  KS_GL_SPHERE_OBJ *ks_allocate_gl_sphere_obj(int detail);
  void ks_free_gl_sphere_obj(KS_GL_SPHERE_OBJ* obj);
  void ks_draw_gl_sphere_obj(KS_GL_SPHERE_OBJ* obj, double r);

#ifdef USE_GLSL
  GLuint ks_make_gl_shader(const char *shader_file, GLenum type);
  GLuint ks_make_gl_shader_from_file(char* file_name, GLenum type);
  GLuint ks_make_gl_shader_program(GLuint *shader, int shader_cnt);
  KS_GL_SHADER_INFO *ks_allocate_gl_standard_shader_info(int standard_shader_type,
							 int object_cnt);
  void ks_free_gl_standard_shader_info(KS_GL_SHADER_INFO *info);
  void ks_set_gl_shader_color_mode(KS_GL_SHADER_INFO *info, int color_mode);
  void ks_set_gl_shader_color_mode_all(KS_GL_SHADER_INFO **info_array, int cnt, int color_mode);
  unsigned int ks_get_gl_shader_pick_id(int window_height, int x, int y);
  unsigned int ks_get_gl_shader_pick_id_offset(KS_GL_SHADER_INFO *info, int obj_id);
  BOOL ks_check_gl_shader_pick_id(KS_GL_SHADER_INFO *info, int obj_id, unsigned int *pick_id_p);
  KS_GL_STANDARD_SHADER_PARM *ks_allocate_gl_standard_shader_parm();
  KS_GL_SHADER_SPHERE_PARM *ks_allocate_gl_shader_sphere_parm(int detail);
  void ks_free_gl_shader_sphere_parm(KS_GL_SHADER_SPHERE_PARM *parm);
  void ks_init_gl_standard_shader_sphere(KS_GL_SHADER_INFO *info, int obj_id,
					 KS_GL_SHADER_SPHERE_PARM *sphere_parm, int share_obj_id);
  void ks_draw_gl_standard_shader(KS_GL_SHADER_INFO *info, int obj_id,
				  GLfloat* projection_matrix, GLfloat* view_matrix);
  KS_GL_SHADER_SPHERE_DRAW_INFO *ks_allocate_gl_shader_sphere_draw_info(int elem_cnt);
  BOOL ks_resize_gl_shader_sphere_draw_info(KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info, int elem_cnt);
  void ks_free_gl_shader_sphere_draw_info(KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info);
  void ks_set_gl_shader_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					 KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
					 KS_GL_STANDARD_SHADER_PARM* shader_parm,
					 int share_obj_id);
  void ks_set_gl_shader_sphere_draw_info_share(KS_GL_SHADER_INFO *info,
					       int obj_id_start, int obj_id_end,
					       KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
					       KS_GL_STANDARD_SHADER_PARM* shader_parm);
  void ks_update_gl_shader_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					    KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
					    int share_obj_id);
  void ks_update_gl_shader_sphere_draw_info_share(KS_GL_SHADER_INFO *info,
						  int obj_id_start, int obj_id_end,
						  KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info);
  void ks_init_gl_standard_shader_regular(KS_GL_SHADER_INFO *info, int obj_id,
					  KS_GL_SHADER_REGULAR_PARM *parm,
					  int share_obj_id);
  void ks_set_gl_shader_regular_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					  KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
					  KS_GL_STANDARD_SHADER_PARM* shader_parm,
					  int share_obj_id);
  void ks_update_gl_shader_regular_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					     KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
					     int share_obj_id);
  void ks_update_gl_shader_regular_draw_info_share(KS_GL_SHADER_INFO *info,
						   int obj_id_start, int obj_id_end,
						   KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info);
  KS_GL_SHADER_TEXTURED_SPHERE_PARM *ks_allocate_gl_shader_textured_sphere_parm(int detail);
  void ks_free_gl_shader_textured_sphere_parm(KS_GL_SHADER_TEXTURED_SPHERE_PARM *parm);
  void ks_init_gl_standard_shader_textured_sphere(KS_GL_SHADER_INFO *info, int obj_id,
						  KS_GL_SHADER_TEXTURED_SPHERE_PARM* sphere_parm,
						  int share_obj_id);
  void ks_set_gl_shader_textured_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
						  KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
						  KS_GL_STANDARD_SHADER_PARM* shader_parm,
						  int share_obj_id);
  void ks_set_gl_shader_draw_mode(KS_GL_SHADER_INFO *info,int obj_id, GLenum draw_mode);
  KS_GL_SHADER_REGULAR_PARM *ks_allocate_gl_shader_regular_parm(int vertex_cnt,
								int index_cnt);
  void ks_free_gl_shader_regular_parm(KS_GL_SHADER_REGULAR_PARM *info);
  KS_GL_SHADER_REGULAR_DRAW_INFO* ks_allocate_gl_shader_regular_draw_info(int elem_cnt);
  BOOL ks_resize_gl_shader_regular_draw_info(KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
					     int elem_cnt);
  void ks_free_gl_shader_regular_draw_info(KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info);
  void ks_draw_gl_standard_shader_with_texture(KS_GL_SHADER_INFO *info, int obj_id,
					       GLfloat* projection_matrix, GLfloat *view_matrix,
					       KS_GL_SHADER_TEXTURE_INFO *tex_info);
  KS_GL_SHADER_TEXTURE_INFO* ks_allocate_gl_shader_file_texture(GLint unit, char *file_name,
								KS_CHAR_LIST *search_path);
  KS_GL_SHADER_TEXTURE_INFO* ks_allocate_gl_shader_sphere_texture(GLint unit, int size,
								  GLfloat light_pos[3]);
  void ks_free_gl_shader_texture_info(KS_GL_SHADER_TEXTURE_INFO *tex_info);
  void ks_use_gl_shader_texture(GLuint uniform_loc, KS_GL_SHADER_TEXTURE_INFO *tex_info);
  void ks_init_gl_standard_shader_point(KS_GL_SHADER_INFO *info, int obj_id);
  void ks_set_gl_shader_point_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
					KS_GL_STANDARD_SHADER_PARM* shader_parm,
					int share_obj_id);

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif


#ifndef CV_LIB_H
#define CV_LIB_H

#ifdef MSVC
#include <windows.h>
#endif

#define USE_OPENGL32
#include <ks_gl.h>
#include <ks_pdb.h>

#define CV_DEFAULT_RADIUS 1.0

enum {
  CV_VER_MAJOR = 3,
  CV_VER_MINOR = 2,
  CV_VER_TEENY = 0
};

enum {
  CV_ATOM_TYPE_MAX = 100,
  CV_ATOM_TYPE_INI_CNT = 11,
  CV_BOND_TYPE_MAX = 10,
  CV_FACE_TYPE_MAX = 10,
  CV_USER_TEXT_MAX = 10,
  CV_AGGREGATE_REQUIREMENT = 2,
  CV_ATYPE_MODE_MAX = 5,
  CV_ATYPE_WORK = CV_ATYPE_MODE_MAX,
  CV_FRAME_FILE_NAME_MAX = 256,
  CV_NAME_MAX = 256,
  CV_FDVIEW_CONTOUR_LEVEL_NUM = 9,
  CV_COMMAND_LEN_MAX = 64,
  CV_SELECT_PARTICLE_MAX = 4,
  CV_DARK_ATOM_TYPE_ADD = 100
};

static const char* __KS_USED__ CV_FILE_KEY_ENABLE_RMSD = {"CDVIEW_ENABLE_RMSD_RMSF"};
static const char* __KS_USED__ CV_FILE_KEY_ENABLE_AXIS = {"CDVIEW_ENABLE_AXIS"};
static const char* __KS_USED__ CV_FILE_KEY_BOND = {"CDVIEW_BOND"};
static const char* __KS_USED__ CV_FILE_KEY_FACE = {"CDVIEW_FACE"};

static const char* __KS_USED__ CV_ENV_KEY_PARTICLE_SIZE_SCALE = {"CDVIEW_PARTICLE_SIZE_SCALE"};
static const char* __KS_USED__ CV_ENV_KEY_LIGHT_POSITION_X = {"CDVIEW_LIGHT_POSITION_X"};
static const char* __KS_USED__ CV_ENV_KEY_LIGHT_POSITION_Y = {"CDVIEW_LIGHT_POSITION_Y"};
static const char* __KS_USED__ CV_ENV_KEY_LIGHT_POSITION_Z = {"CDVIEW_LIGHT_POSITION_Z"};
static const char* __KS_USED__ CV_ENV_KEY_DISABLE_ERROR_HIT_ENTER =
  {"CDVIEW_DISABLE_ERROR_HIT_ENTER"};
static const char* __KS_USED__ CV_ENV_KEY_MOVIE_COMMAND = {"CDVIEW_MAKE_MOVIE_COMMAND"};
static const char* __KS_USED__ CV_ENV_KEY_REMOVE_BMP_MOVIE={"CDVIEW_REMOVE_BMP_AFTER_MAKING_MOVIE"};

enum {
  CV_SPHERE_DETAIL_LOW,    // detail = 4
  CV_SPHERE_DETAIL_MODEST, // detail = 8
  CV_SPHERE_DETAIL_NORMAL, // detail = 12
  CV_SPHERE_DETAIL_HIGH,   // detail = 20, valued are defined in cv_sphere_detail_values
  CV_SPHERE_DETAIL_CNT,
  CV_SPHERE_DETAIL_MIN = 0,
  CV_SPHERE_DETAIL_MAX = CV_SPHERE_DETAIL_CNT-1
};

enum {
  CV_FILE_KEY_LEN_BOND = 11,
  CV_FILE_KEY_LEN_FACE = 11
};

enum {
  CV_NAME_DEFAULT,
  CV_NAME_PARTICLE,
  CV_NAME_BOND,
  CV_NAME_RIBBON
};

enum {
  CV_DRAW_MODE_LINE,
  CV_DRAW_MODE_PARTICLE,
  CV_DRAW_MODE_MAIN_CHAIN,
  CV_DRAW_MODE_TUBE,
  CV_DRAW_MODE_RIBBON,
  CV_DRAW_MODE_SOLVENT_EXCLUDE_SURFACE,
  CV_DRAW_MODE_OFF,
  CV_DRAW_MODE_NUM
};

enum {
  CV_DRAW_WATER_MODE_LINE,
  CV_DRAW_WATER_MODE_PARTICLE,
  CV_DRAW_WATER_MODE_OFF,
  CV_DRAW_WATER_MODE_NUM
};

enum {
  CV_COLOR_MODE_PARTICLE,
  CV_COLOR_MODE_AMINO,
  CV_COLOR_MODE_AMINO_TYPE,
  CV_COLOR_MODE_AMINO_HYDROPATHY,
  CV_COLOR_MODE_CHAIN,
  CV_COLOR_MODE_AGGREGATE,
  CV_COLOR_MODE_RAINBOW,
  CV_COLOR_MODE_NUM
};

enum {
  CV_LABEL_MODE_OFF,
  CV_LABEL_MODE_PARTICLE_LABEL,
  CV_LABEL_MODE_PARTICLE_NAME,
  CV_LABEL_MODE_RESIDUE_LABEL,
  CV_LABEL_MODE_RESIDUE_NAME,
  CV_LABEL_MODE_NUM
};

enum {
  CV_PARTICLE_SIZE_SMALL,
  CV_PARTICLE_SIZE_NORMAL,
  CV_PARTICLE_SIZE_LARGE,
  CV_PARTICLE_SIZE_NUM
};

enum {
  CV_KABE_MODE_OFF,
  CV_KABE_MODE_HAKO,
  CV_KABE_MODE_SIDE,
  CV_KABE_MODE_NUM
};

enum {
  CV_PREP_CHANGE_OFF,
  CV_PREP_CHANGE_PARTICLE_ORDER,
  CV_PREP_CHANGE_PARTICLE_MAIN,
  CV_PREP_CHANGE_IMPROPER,
  CV_PREP_CHANGE_NUM
};

enum {
  CV_SHOW_SELECTED_PARTICLE_MODE_OFF,
  CV_SHOW_SELECTED_PARTICLE_MODE_ON,
  CV_SHOW_SELECTED_PARTICLE_MODE_RANGE,
  CV_SHOW_SELECTED_PARTICLE_MODE_NUM
};

enum {
  CV_COMMAND_RMSD,
  CV_COMMAND_RMSF,
  CV_COMMAND_DISTANCE,
  CV_COMMAND_DEL_DISTANCE,
  CV_COMMAND_STORE_MEMORY,
  CV_COMMAND_SET_SKIP,
  CV_COMMAND_SET_TIME_UNIT,
  CV_COMMAND_SHOW_LINE,
  CV_COMMAND_HIDE_LINE,
  CV_COMMAND_SHOW_PARTICLE,
  CV_COMMAND_HIDE_PARTICLE,
  CV_COMMAND_CALC_SOLVENT_EXCLUDE_SURFACE,
  CV_COMMAND_CALC_SOLVENT_EXCLUDE_SURFACE_ALL,
  CV_COMMAND_ADD_TEXT,
  CV_COMMAND_FIND_PARTICLE_LABEL,
  CV_COMMAND_OUTPUT_PICTURE_BMP,
  CV_COMMAND_OUTPUT_PICTURE_POV,
  CV_COMMAND_OUTPUT_PICTURE_POV_SHADOW,
  CV_COMMAND_TOGGLE_POV_WATER,
  CV_COMMAND_TOGGLE_POV_BUBBLE,
  CV_COMMAND_CALC_DENSITY,
  CV_COMMAND_FIT_AXIS,
  CV_COMMAND_ADD_RESIDUE_NEXT,
  CV_COMMAND_ADD_RESIDUE_PREV,
  CV_COMMAND_CHANGE_RESIDUE,
  CV_COMMAND_CHANGE_ATOM,
  CV_COMMAND_MAKE_BOND,
  CV_COMMAND_REMOVE_BOND,
  CV_COMMAND_CHANGE_LEVEL_MAX,
  CV_COMMAND_CHANGE_LEVEL_MIN,
  CV_COMMAND_CALC_GR_PARTICLE,
  CV_COMMAND_CALC_GR_MONOMER,
  CV_COMMAND_ADJUST_DEPTH,
  CV_COMMAND_OUTPUT_PREP,
  CV_COMMAND_MOVE_PREP_DUMMY,
  CV_COMMAND_CHANGE_SHOW_SELECTED_PARTICLE_RANGE,
  CV_COMMAND_CLIP_PLANE,
  CV_COMMAND_ADD_CLIP_PLANE,
  CV_COMMAND_ROTATION,
  CV_COMMAND_TRANSLATION,
  CV_COMMAND_SET_CONTINUOUS_COLOR_NUM,
  CV_COMMAND_SET_TIME_OFFSET,
  CV_COMMAND_SET_EYE_ANGLE,
  CV_COMMAND_SET_ROT_MOL,
  CV_COMMAND_NUM
};

enum {
  CV_COMMAND_KEY_1,
  CV_COMMAND_KEY_2,
  CV_COMMAND_KEY_NUM
};

enum {
  CV_QUATERNION_UEDA,
  CV_QUATERNION_ALLEN,
  CV_QUATERNION_TYPE_NUM
};

enum {
  CV_AXIS_MODE_OFF,
  CV_AXIS_MODE_ON,
  CV_AXIS_MODE_NUM
};

enum {
  CV_BOX_FRAME_COLOR_1,
  CV_BOX_FRAME_COLOR_2,
  CV_BOX_FRAME_COLOR_CURRENT,
  CV_BOX_FRAME_COLOR_NUM
};

typedef struct CV_BASE CV_BASE;

typedef struct CV_COMMAND CV_COMMAND;
struct CV_COMMAND{
  char key[CV_COMMAND_KEY_NUM][64];
  char comment[256];
  void (*process_command)(KS_GL_BASE*,CV_BASE*,char*);
};
/*
#define CV_RESIDUE_NORMAL       KS_MD_RESIDUE_NORMAL
#define CV_RESIDUE_START        KS_MD_RESIDUE_START
#define CV_RESIDUE_END          KS_MD_RESIDUE_END
#define CV_RESIDUE_ALPHA        KS_MD_RESIDUE_ALPHA
#define CV_RESIDUE_BETA         KS_MD_RESIDUE_BETA              
#define CV_RESIDUE_LOOP         KS_MD_RESIDUE_LOOP              
#define CV_RESIDUE_BETA_N_BOND  KS_MD_RESIDUE_BETA_N_BOND
#define CV_RESIDUE_BETA_O_BOND  KS_MD_RESIDUE_BETA_O_BOND      
#define CV_RESIDUE_SELECTED     KS_MD_RESIDUE_SELECTED
*/
#define CV_RESIDUE_NORMAL            (unsigned int)0x001
#define CV_RESIDUE_START             (unsigned int)0x002
#define CV_RESIDUE_END               (unsigned int)0x004
#define CV_RESIDUE_ALPHA             (unsigned int)0x010
#define CV_RESIDUE_BETA              (unsigned int)0x020
#define CV_RESIDUE_LOOP              (unsigned int)0x040
#define CV_RESIDUE_BETA_N_BOND       (unsigned int)0x080
#define CV_RESIDUE_BETA_O_BOND       (unsigned int)0x100
#define CV_RESIDUE_SELECTED          (unsigned int)0x200
#define CV_RESIDUE_AMINO        (unsigned int)0x10000

typedef struct CV_PARTICLE CV_PARTICLE;

typedef struct CV_RESIDUE CV_RESIDUE;
struct CV_RESIDUE{
  unsigned int label;
  unsigned int flags;
  unsigned int file_label;
  int number;
  char name[8];
  char type;
  CV_PARTICLE **particle;
  int particle_num;
  int particle_num0;
  CV_PARTICLE *main_N,*main_C,*main_CA,*main_O;
  double vec_C_O[3];
  int pdb_residue_head;
  CV_RESIDUE *next;
  CV_RESIDUE *hash_next;
};

typedef struct CV_SOLVENT_EXCLUDE_SURFACE CV_SOLVENT_EXCLUDE_SURFACE;
struct CV_SOLVENT_EXCLUDE_SURFACE{
  int vertex_num;
  double *vertex;
  double *normal;
  int face_num;
  int *face;
  int pl_num;
  int *pl_no;
  CV_PARTICLE **pl;
};

#define CV_CHAIN_AMINO 	  (unsigned int)0x001
#define CV_CHAIN_SELECTED (unsigned int)0x002

typedef struct CV_CHAIN CV_CHAIN;
struct CV_CHAIN{
  unsigned int flags;
  unsigned int label;
  char name[CV_NAME_MAX];
  int len;
  CV_RESIDUE *residue;
  CV_SOLVENT_EXCLUDE_SURFACE *ses;
  CV_CHAIN *next;
};

#define CV_AGGREGATE_SELECTED (unsigned int)0x001

typedef struct CV_AGGREGATE CV_AGGREGATE;
struct CV_AGGREGATE{
  unsigned int label;
  unsigned int flags;
  CV_CHAIN *chain;
  CV_AGGREGATE *next;
};

#define CV_ATOM_SHOW (unsigned int)0x001

typedef struct CV_ATOM CV_ATOM;
struct CV_ATOM {
  unsigned int flags;
  int atype;
  char *name;
  /*
  GLfloat color[3];
  float radius;
  */
  double mass;
  KS_GL_ATOM *gl_atom;
  CV_ATOM *next;
};

typedef struct CV_PARTICLE_PROPERTY CV_PARTICLE_PROPERTY;
struct CV_PARTICLE_PROPERTY{
  char name[8];
  CV_PARTICLE *alternative;
  CV_RESIDUE *residue;
  CV_CHAIN *chain;
  CV_AGGREGATE *aggregate;
  CV_PARTICLE_PROPERTY *next;
};

typedef struct CV_BOND_TYPE CV_BOND_TYPE;
struct CV_BOND_TYPE{
  unsigned int type;
  GLfloat color[3];
  float radius;
  CV_BOND_TYPE *next;
};

#define CV_BOND_SELECTED  (unsigned int)0x01
#define CV_BOND_OUTPUTED  (unsigned int)0x02

typedef struct CV_BOND CV_BOND;
struct CV_BOND{
  unsigned int flags;
  CV_PARTICLE *p;
  double len;
  CV_BOND_TYPE *type;
  CV_BOND *next;
};

#define CV_CDV_BOND_LIST_PDB_USER  (unsigned int)0x01

typedef struct CV_CDV_BOND_LIST CV_CDV_BOND_LIST;
struct CV_CDV_BOND_LIST{
  unsigned int flags;
  unsigned int type;
  unsigned int label[2];
  CV_CDV_BOND_LIST *next;
};

typedef struct CV_FACE_TYPE CV_FACE_TYPE;
struct CV_FACE_TYPE{
  unsigned int type;
  GLfloat color[3];
  CV_FACE_TYPE *next;
};
typedef struct CV_CDV_FACE_LIST CV_CDV_FACE_LIST;
struct CV_CDV_FACE_LIST{
  unsigned int type;
  unsigned int label[3];
  CV_CDV_FACE_LIST *next;
};

#define CV_PARTICLE_SELECTED   (unsigned int)0x01
#define CV_PARTICLE_CHAIN_END  (unsigned int)0x02
#define CV_PARTICLE_WATER      (unsigned int)0x04
#define CV_PARTICLE_MAIN       (unsigned int)0x08
#define CV_PARTICLE_PRIME      (unsigned int)0x10
#define CV_PARTICLE_HETATM     (unsigned int)0x20
#define CV_PARTICLE_MARKED     (unsigned int)0x40
#define CV_PARTICLE_SOLVENT    (unsigned int)0x80

struct CV_PARTICLE {
  unsigned int flags;
  unsigned int label;
  unsigned int file_label;
  int atype[CV_ATYPE_MODE_MAX+1];
  CV_ATOM *atom;
  CV_PARTICLE_PROPERTY *property;
  double quaternion[4];
  double cd[3];
  double move[3];
  /*  double add[3];*/
  int color_mode[CV_COLOR_MODE_NUM];
  CV_BOND *bond;
  CV_PARTICLE *prev;
  CV_PARTICLE *next;
  CV_PARTICLE *hash_next;
#ifdef USE_GLSL
  KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM *draw_info_elem_p;  // to refer from CV_PARTICLE to draw_info
#endif
};

typedef struct CV_ADDITIONAL_SURFACE CV_ADDITIONAL_SURFACE;
struct CV_ADDITIONAL_SURFACE{
  BOOL cull_face;
  GLfloat transparency;
  int vertex_num, face_num;
  int vertex_malloc_num,face_malloc_num;
  GLfloat *vertex,*normal,*color;
  int *face;
};

typedef struct CV_COLOR_MAP CV_COLOR_MAP;
struct CV_COLOR_MAP {
  GLfloat **color;
  double *value;
  int num,malloc_num;
};

typedef struct CV_REUSE CV_REUSE;
struct CV_REUSE {
  CV_PARTICLE *particle;
  CV_PARTICLE_PROPERTY *property;
  CV_BOND *bond;
  CV_AGGREGATE *aggregate;
  CV_CHAIN *chain;
  CV_RESIDUE *residue;
  KS_LABEL_LIST *label;
  CV_CDV_BOND_LIST *cdv_bond;
  CV_CDV_FACE_LIST *cdv_face;
  CV_ADDITIONAL_SURFACE *sur;
  CV_COLOR_MAP *color_map;
};

typedef struct CV_USER_TEXT CV_USER_TEXT;
struct CV_USER_TEXT {
  float x,y;
  char *text;
};

typedef struct CV_PARTICLE_POS_HASH CV_PARTICLE_POS_HASH;
struct CV_PARTICLE_POS_HASH {
  int n[3];
  double len[3],min[3];
  int ***particle_num;
  CV_PARTICLE *****particle;
};

typedef struct CV_POV_INFO CV_POV_INFO;
struct CV_POV_INFO {
  double diffuse;
  double ambient;
};

typedef struct {
  GLfloat *start_color;
  GLfloat *end_color;
  BOOL set_min_max;
  double min[4],max[4];   /* array num is 4 because it depend on quaternion num */
  CV_COLOR_MAP **map;
} CV_CONTINUOUS_COLOR_INFO;

typedef struct {
  char key;
  BOOL alt;
  BOOL special;
} CV_AUTO_COMMAND_KEY;

typedef struct {
  int current;
  int num;
  CV_AUTO_COMMAND_KEY *command;
} CV_AUTO_COMMAND;

#include "fv033.h"

#define CV_LABEL_DUPLICATE (unsigned int)0x01
#define CV_LABEL_USE       (unsigned int)0x02

#define CV_FRAME_REFERENCE (unsigned int)0x01
#define CV_FRAME_READ      (unsigned int)0x02
#define CV_FRAME_CDV       (unsigned int)0x04
#define CV_FRAME_FDV       (unsigned int)0x08
#define CV_FRAME_PDB       (unsigned int)0x10
#define CV_FRAME_PREP      (unsigned int)0x20
#define CV_FRAME_ENABLE    (unsigned int)0x40

typedef struct CV_FRAME CV_FRAME;
struct CV_FRAME {
  unsigned int label;
  unsigned int flags;
  char file_name[CV_FRAME_FILE_NAME_MAX];
  char file_name_fdv[CV_FRAME_FILE_NAME_MAX];
  char file_path[CV_FRAME_FILE_NAME_MAX];
  unsigned int particle_label_max;
  unsigned int particle_file_label_max;
  unsigned int residue_label_max;
  unsigned int chain_label_max;
  unsigned int aggregate_label_max;
  CV_PARTICLE *particle;
  CV_PARTICLE *particle_end;
  CV_PARTICLE ***periodic_p;
  size_t particle_cnt;
  int *periodic_p_num;
  CV_AGGREGATE *aggregate;
  double center[3];
  double system_size;
  double fit_trans[3];
  double fit_rot[16];
  double time;
  BOOL have_time;
  double side[2][2][2][3];
  float side_radius;
  BOOL have_side;
  double rmsd;
  double distance;
  CV_REUSE *reuse;
  char *bond_file_name;
  char *face_file_name;
  BOOL pdb_ext;
  unsigned int unknown_residue_label_max;
  KS_CHAR_LIST *unknown_residue_label;
  BOOL have_solvent_exclude_surface;
  CV_USER_TEXT user_text[CV_USER_TEXT_MAX];
  CV_CDV_BOND_LIST *cdv_bond;
  CV_CDV_FACE_LIST *cdv_face;
  CV_ADDITIONAL_SURFACE *sur;
  CV_COLOR_MAP *color_map;

  KS_AMBER_PREP_BLOCK *prep_block;

  FV_BASE *fv;

  int atype_mode;
  CV_CONTINUOUS_COLOR_INFO c_color_info;

  CV_BOND *pdb_user_bond;
  KS_FILE_LINES **fip;
};

enum {
  CV_PLANE_X,
  CV_PLANE_Y,
  CV_PLANE_Z
};
enum {
  CV_DRAW_TIME_UNIT_FS,
  CV_DRAW_TIME_UNIT_PS,
  CV_DRAW_TIME_UNIT_NS
};

enum {
  CV_OUTPUT_PICTURE_BMP,
  CV_OUTPUT_PICTURE_POV,
  CV_OUTPUT_PICTURE_POV_SHADOW,
  CV_OUTPUT_PICTURE_NUM
};

enum {
  CV_PROJECTION_MODE_PERSPECTIVE,
  CV_PROJECTION_MODE_ORTHO
};

typedef struct {
  int mode;
  int num;
} CV_CONTINUOUS_COLOR;

typedef struct {
  CV_COLOR_MAP *color_map;
  CV_ADDITIONAL_SURFACE *sur;
} CV_FIXED_SURFACE_INFO;

#define CV_SHOW_LINE     (unsigned int)0x01
#define CV_SHOW_PARTICLE (unsigned int)0x02

#ifdef USE_GLSL
enum {
  SHADER_SPHERE,   // shader for spheres
  SHADER_REGULAR,  // shader for faces and rods
  SHADER_POINT,    // shader for points
  SHADER_TYPE_CNT
};
enum {
  SHADER_PARM_NORMAL,
  SHADER_PARM_HIGH_AMBIENT,
  SHADER_PARM_CNT
};
enum {
  SPHERE_TYPE_POINT = CV_SPHERE_DETAIL_CNT,
  SPHERE_TYPE_CNT = CV_SPHERE_DETAIL_CNT+1
};
enum {
  OBJ_TYPE_BOX,        // vertices for system box
  OBJ_TYPE_ROD_NORMAL, // vertices for rods
  OBJ_TYPE_ROD_LOW,    // vertices for low detail rods
  OBJ_TYPE_CNT
};
typedef struct {
  KS_GL_SHADER_INFO *shader[SHADER_TYPE_CNT];
  KS_GL_STANDARD_SHADER_PARM *shader_parm[SHADER_PARM_CNT];
  KS_GL_SHADER_SPHERE_PARM *sphere_parm[SPHERE_TYPE_CNT];
  KS_GL_SHADER_SPHERE_DRAW_INFO *sphere_draw_info;
  CV_PARTICLE **sphere_draw_info_particle;           // to refer from CV_PARTICLE to draw_info
  KS_GL_SHADER_REGULAR_PARM *obj_parm[OBJ_TYPE_CNT]; // parameters of vertices, normals, and colors
  KS_GL_SHADER_REGULAR_DRAW_INFO *obj_draw_info[OBJ_TYPE_CNT];
} CV_SHADER_BASE;
#else
typedef void CV_SHADER_BASE;
#endif

struct CV_BASE {
  CV_FRAME *frame,*reference_frame,*avrage_frame;
  int frame_num, current_frame;

  int color_mode;
  char color_mode_name[CV_COLOR_MODE_NUM][64];
  GLuint sphere_texture;
  KS_GL_BUFFER_3D *ribbon;

  BOOL no_mem;
  BOOL no_water;
  BOOL move_mode;
  BOOL look_selected_particle;
  BOOL fit_reference;
  BOOL overlap_reference;

  int draw_mode;
  int draw_water_mode;
  int particle_size;
  int label_mode;
  int kabe_mode;

  double particle_size_scale;

  unsigned int show_flags;

  KS_TEXT_LIST *telop;
  KS_INT_LIST *selected_label;
  KS_INT_LIST *selected_residue_label;
  double move_add[3];

  CV_REUSE reuse_base;
  KS_PDB_BUF *pdb_buf;

  CV_PARTICLE **particle_hash;
  int particle_hash_size;
  CV_RESIDUE **residue_hash;
  int residue_hash_size;
  KS_LABEL_LIST **particle_label_hash, **residue_label_hash;
  int particle_label_hash_size, residue_label_hash_size;

  CV_ATOM *atom;
  CV_BOND_TYPE *bond_type;
  CV_FACE_TYPE *face_type;
  KS_SPHERE *color_sample[CV_COLOR_MODE_NUM];

  void (*idle)(KS_GL_BASE*, CV_BASE*);

  char rmsd_file_name[CV_NAME_MAX];
  char rmsf_file_name[CV_NAME_MAX];
  char distance_file_name[CV_NAME_MAX];
  KS_GL_GRAPH_2D *rmsd_graph;
  KS_GL_GRAPH_2D *rmsf_graph;
  KS_GL_GRAPH_2D *distance_graph;
  int rmsd_window_id;
  int rmsf_window_id;
  int distance_window_id;

  int atype_mode;
  int atype_mode_num;
  BOOL atype_use[CV_ATOM_TYPE_MAX];

  int detail;
  int show_selected_particle_mode;
  double show_selected_particle_range;
  int input_key_font;
  double background_transparency;

  CV_COMMAND command[CV_COMMAND_NUM];
  int command_help_window_id;
  int command_help_font;

  int skip_frame;
  int draw_time_unit_type;

  KS_CHAR_LIST *text_window;
  int text_window_font;

  char *header_file_name;

  int output_file_mode;

  FV_BASE fv_base;
  FV_TARGET fv_target;
  double ***fv_level;
  int fv_level_num;

  BOOL draw_additional_surface;
  int projection_mode;
  int calc_quaternion_type;

  int prep_change_mode;

  BOOL *clip_plane_flg;
  double **clip_plane;

  BOOL enable_cdv_rmsd_rmsf;
  CV_POV_INFO pov_info;
  int axis_mode;
  GLfloat light_position[4];
  GLfloat box_frame_color[CV_BOX_FRAME_COLOR_NUM][4];

  CV_CONTINUOUS_COLOR continuous_color;

  int periodic_mode;
  GLfloat periodic_inside_color[4];

  char file_name_base[CV_FRAME_FILE_NAME_MAX];
  int order_bmp_counter;
  int output_bmp_file_cnt;

  double time_offset;
  BOOL fv_color_map_select;

  KS_AMBER_PREP *prep;
  KS_GL_SPHERE_OBJ **sphere_obj;

  CV_AUTO_COMMAND *auto_command;
  CV_FIXED_SURFACE_INFO sur_fixed;

  CV_SHADER_BASE *sb;

  KS_FILE_LINES *fi;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

  CV_BASE* cv_allocate_base(char **file_name, int file_num, BOOL no_mem, BOOL no_water, 
			    int projection_mode,
			    int initial_detail, char *header_file_name, 
			    double ***level_min_max, int level_num, BOOL fv_bw,
			    BOOL enable_cdv_rmsd_rmsf);
  void cv_free_base(CV_BASE *cb);
  void cv_draw(KS_GL_BASE *gb, CV_BASE *cb);
  double cv_get_system_size(CV_BASE *cb);
  void cv_set_glut_key(KS_GL_BASE *gb, CV_BASE *cb);
  void cv_draw_color_sample(KS_GL_BASE *gb, CV_BASE *cb, int width, int height);
  void cv_draw_text(KS_GL_BASE *gb, CV_BASE *cb, int width, int height);
  void cv_process_mouse(KS_GL_BASE *gb, CV_BASE *cb, int button, int state, int x, int y);
  void cv_process_motion(KS_GL_BASE *gb, CV_BASE *cb, int x, int y);
  BOOL cv_process_idle(KS_GL_BASE *gb, CV_BASE *cb);
  void cv_process_keyboard(KS_GL_BASE *gb, CV_BASE *cb, unsigned char key, int x, int y);
  void cv_process_special_keyboard(KS_GL_BASE *gb, CV_BASE *cb, unsigned char key, int x, int y);
  void cv_init_lookat(KS_GL_BASE *gb, CV_BASE *cb);
  void cv_set_lookat(KS_GL_BASE *gb, CV_BASE *cb, double *pos);
  void cv_draw_2d(KS_GL_BASE *gb, CV_BASE *cb, int width, int height);
  void cv_draw_info(KS_GL_BASE *gb, CV_BASE *cb, int width, int height);
  void cv_process_display(KS_GL_BASE *gb, CV_BASE *cb);
  CV_COLOR_MAP *cv_allocate_color_map(void);
  void cv_free_color_map(CV_COLOR_MAP *color_map);
  void cv_init_gl(KS_GL_BASE *gb, CV_BASE *cb);
  void cv_adjust_depth_range(KS_GL_BASE *gb, CV_BASE *cb);
  BOOL cv_have_side(CV_BASE *cb);
  BOOL cv_set_auto_command(CV_BASE *cb, char *auto_command);
  BOOL cv_is_set_disable_error_hit_enter();

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

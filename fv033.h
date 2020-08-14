#ifndef FV_LIB_H
#define FV_LIB_H

enum {
  FV_MAX_COLUMN_NUM = 10,
};

enum {
  FV_DRAW_OFF,
  FV_DRAW_LINE,
  FV_DRAW_FILL,
  FV_DRAW_TRANS,
  FV_DRAW_NUM
};

typedef struct FV_FIELD_SIZE FV_FIELD_SIZE;
struct FV_FIELD_SIZE{
  //  int num;          /* total number */
  int n[3];         /* nubmer of x,y,z */
  int val_num;      /* number of values */
  double ccd[3];    /* center position */
  double max,min;
};

typedef struct FV_FIELD FV_FIELD;
typedef struct FV_CONTOUR FV_CONTOUR;
struct FV_FIELD{
  double cd[3];     /* position */
  double *s;        /* values */
  FV_FIELD* l[3][2]; /* pointer of neighbor lattice */
  FV_CONTOUR* c[3][2]; /* pointer of neighbor lattice */
};
#define FV_CONTOUR_SELECTED (unsigned int)0x01
struct FV_CONTOUR{
  unsigned int flags;
  int ln,vn;
  double cd[3];
  double nv[3];
  FV_FIELD* l[2]; /* pointer of neighbor lattice */
  FV_CONTOUR* c[3][2]; /* pointer of neighbor lattice */
  FV_CONTOUR* next;
  FV_CONTOUR* loop[2];
};
typedef struct FV_CONTOUR_SIZE FV_CONTOUR_SIZE;
struct FV_CONTOUR_SIZE{
  int num;
  double *level;
  float **color;
};

typedef struct FV_CONTOUR_FAN FV_CONTOUR_FAN;
struct FV_CONTOUR_FAN{
  int num;
  FV_CONTOUR **pcs;
  FV_CONTOUR_FAN *next;
};

typedef struct FV_TARGET FV_TARGET;
struct FV_TARGET {
  int pl;  /* 0:yz 1:zx 2:xy */
  int pl_sign;
  int num[3];
  int val;
  int *level;
};

typedef struct FV_RANGE_HIS_DATA FV_RANGE_HIS_DATA;
struct FV_RANGE_HIS_DATA {
  double min;
  double max;
  double *level;
};

typedef struct FV_BASE FV_BASE;
struct FV_BASE{
  FV_FIELD ****field;
  FV_FIELD_SIZE field_size;
  FV_CONTOUR_SIZE contour_size;
  FV_CONTOUR_FAN ***contour_fan;
  BOOL black_and_white;
};
/*
typedef struct FV_RANGE_HIS FV_RANGE_HIS;
struct FV_RANGE_HIS {
  int pos;
  int num;
  FV_RANGE_HIS_DATA *hd;
};


#define CV_MAX_ATYPE_NUM 20
#define CV_MAX_ATYPE_LIST_NUM 10
typedef struct CV_PROPERTY CV_PROPERTY;
struct CV_PROPERTY {
  int atype_list_num;
  int dnum;
  float box_s[3],box_e[3],box_wt;
  float color[CV_MAX_ATYPE_NUM][4];
  float radii[CV_MAX_ATYPE_NUM];
  int flg[CV_MAX_ATYPE_NUM];
  int ditail;
  float radius;
};
*/

BOOL fv_read_field_file(char *file_name, FV_FIELD *****fi, FV_FIELD_SIZE *fs, char* file_name_cdv);
BOOL fv_calc_field_contour(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_CONTOUR_SIZE cs);

void fv_gldraw_field(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_TARGET ft, FV_CONTOUR_SIZE cs,
		     int dm);
void fv_gldraw_contour_surface(FV_CONTOUR_SIZE cs, FV_TARGET ft, FV_CONTOUR_FAN ***fcf, 
			       int draw_mode);
void fv_output_contour_surface(FV_CONTOUR_SIZE cs, FV_TARGET tg, FV_CONTOUR_FAN ***fcf,
			       int mode, FILE *fp);
void fv_set_level(double min, double max, FV_CONTOUR_SIZE *cs, BOOL black_and_white);
int fv_get_level(double val, FV_CONTOUR_SIZE cs);
void fv_get_level_color(double val, double max, double min, FV_CONTOUR_SIZE cs, GLfloat *color);
void fv_set_max_min(FV_FIELD ****fi, FV_FIELD_SIZE *fs, int val_num);
void fv_free_field(FV_FIELD ****fi, FV_FIELD_SIZE fs);

BOOL fv_init_field(FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE *cn, FV_CONTOUR_FAN ****csf, 
		   FV_TARGET *tg, int contour_level_num, BOOL black_and_white);
void fv_draw_field(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms,FV_CONTOUR_SIZE cn,
		   FV_CONTOUR_FAN ***csf, FV_TARGET *tg, FILE *pov);
void fv_free_contour_fan(FV_CONTOUR_FAN ***cf,FV_CONTOUR_SIZE cs, FV_FIELD_SIZE ms);
void fv_free_contour_size(FV_CONTOUR_SIZE cs);
void fv_free_target(FV_TARGET tg);
void fv_set_center_and_size(FV_FIELD ****mn, FV_FIELD_SIZE ms, double *center, double *size);
BOOL fv_calc_contour_surface(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_CONTOUR_SIZE cs,
			     FV_CONTOUR_FAN ***csf);
void fv_move_forward_contour_plane(KS_GL_BASE *gb, int x, int y, void *vp);
void fv_move_backward_contour_plane(KS_GL_BASE *gb, int x, int y, void *vp);
void fv_increase_target_surface(KS_GL_BASE *gb, int x, int y, void *vp);
void fv_decrease_target_surface(KS_GL_BASE *gb, int x, int y, void *vp);
void fv_set_field_connection(FV_FIELD *****fi, FV_FIELD_SIZE *fs);
void fv_reconstruct_contour(FV_BASE *fv);
void fv_display_2d(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn,
		   FV_TARGET tg, int width, int height, BOOL black_and_white);

void fv_increase_draw_surface_mode(KS_GL_BASE *gb, int x, int y, void *vp);
void fv_increase_draw_contour_mode(KS_GL_BASE *gb, int x, int y, void *vp);
void fv_free_base(FV_BASE *fv);
void fv_init_base(FV_BASE *fv);
void fv_init_target(FV_TARGET *ft);
BOOL fv_malloc_field(FV_FIELD *****fi, FV_FIELD_SIZE *fs);
void fv_debug_mode(int mode);
BOOL fv_get_contour_level_color(FV_CONTOUR_SIZE *cs, int level, float color[3]);
void fv_set_contour_level_color(FV_CONTOUR_SIZE *cs, int level, float color[3]);
void fv_change_target_column(KS_GL_BASE *gb, int x, int y, void *vp);
BOOL fv_hit_color_map_xy(int x, int y);
BOOL fv_hit_color_map_y(int y);
int fv_get_color_map_num(int x, int y,FV_CONTOUR_SIZE cn);
void fv_save_povray(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn,
		    FV_CONTOUR_FAN ***csf, FV_TARGET tg, char *name);
void fv_save_povray_mesh(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn,
			 FV_CONTOUR_FAN ***csf, FV_TARGET tg, FILE *fp);
BOOL fv_get_pov_water_flag();
BOOL fv_get_pov_bubble_flag();
void fv_set_pov_water_flag(BOOL value);
void fv_set_pov_bubble_flag(BOOL value);

#endif


#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#ifdef MSVC
#define snprintf _snprintf
#endif
#ifdef ICL
#include <mathimf.h>
#else
#include <math.h>
#endif
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef USE_GLSL
#include <glew.h>
#include "ksp_gl.h"
#endif

#include <GL/glut.h>

#include "cv304.h"
#include "fv033.h"

#include <ks_std.h>
//#include <ks_md.h>
#include <ks_atom.h>
#include <ks_gl.h>
#include <ks_math.h>

/*int particle_mode = 1;*/

static struct {
  float radius;
  GLfloat color[3];
  BOOL scaled;
  float scale[3];
  BOOL show;
} init_cv_atom_parm[CV_ATOM_TYPE_MAX] = {
  {1.0, {1.0, 0.0, 0.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  0 */
  {1.0, {0.0, 1.0, 0.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  1 */
  {1.0, {0.0, 0.0, 1.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  2 */
  {1.0, {0.0, 1.0, 1.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  3 */
  {1.0, {1.0, 0.0, 1.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  4 */
  {1.0, {1.0, 1.0, 0.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  5 */
  {1.0, {0.0, 0.5, 0.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  6 */
  {1.0, {0.5, 0.5, 0.5}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  7 */
  {1.0, {1.0, 1.0, 1.0}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /*  8 */
  {1.0, {0.5, 0.5, 0.5}, KS_FALSE, {1.0,1.0,1.0}, KS_FALSE}, /*  9 */
  {1.0, {0.5, 0.5, 0.5}, KS_FALSE, {1.0,1.0,1.0}, KS_TRUE},  /* 10 */
};
static struct {
  float radius;
  GLfloat color[3];
} init_cv_bond_type = {0.1,{0.5,0.5,0.5}};

static CV_FACE_TYPE init_cv_face_type = {0,{0.5,0.5,0.5},NULL}; // default setting of face

static int change_particle_order_target = 4;
static int display_improper_target = 1;

static const int cv_sphere_detail_values[CV_SPHERE_DETAIL_CNT] = {4, 8, 12, 20};

static void set_select_residue(CV_RESIDUE *p, BOOL select);
static void set_calc_rmsd(KS_GL_BASE *gb, CV_BASE *cb, char *text);
static void set_calc_rmsf(KS_GL_BASE *gb, CV_BASE *cb, char *text);
static void set_calc_distance(KS_GL_BASE *gb, CV_BASE *cb, char *text);
static void del_distance(KS_GL_BASE *gb, CV_BASE *cb, char *text);
static void set_store_memory(KS_GL_BASE *gb, CV_BASE *cb, char *text);
static void free_cv_solvent_exclude_surface_particle(CV_SOLVENT_EXCLUDE_SURFACE *ses);
static void free_cv_solvent_exclude_surface_vertex(CV_SOLVENT_EXCLUDE_SURFACE *ses);
static void free_cv_solvent_exclude_surface_face(CV_SOLVENT_EXCLUDE_SURFACE *ses);
static void get_color(CV_PARTICLE *p, int color_mode, int bright, GLfloat *color, BOOL prep_mode);
static void first_frame(KS_GL_BASE *gb, int x, int y, void *vp);
static BOOL move_frame(KS_GL_BASE *gb, CV_BASE *cb, int move_frame);
static void del_particle(KS_GL_BASE *gb, char *text, void *vp);
static void free_cv_particle(CV_PARTICLE *listp);
static void display_3d(KS_GL_BASE *gb, void *vp);
static void calc_internal_cd(double *cd0, double *cd1, double *cd2, double *icd, double *cd);
static void set_bond_all(CV_PARTICLE *pl, CV_BOND **reuse);
static void set_atype(CV_BASE *cb, CV_FRAME *fr);
static void set_periodic_frame(CV_BASE *cb, CV_FRAME *fr);
static void make_common_file_name(CV_BASE *cb, char *file_name, size_t size, char *add);
static void make_common_file_name_fdv(CV_BASE *cb, char *file_name, size_t size, char *add);
static BOOL is_shown_fv_map(KS_GL_BASE *gb, CV_FRAME *fr);
static void output_pov_file(KS_GL_BASE *gb, char *text, void *vp);
static void set_cdv_bond(char *cp, CV_CDV_BOND_LIST **bond, CV_CDV_BOND_LIST **reuse,
			 unsigned int flags);
static void set_cdv_face(char *cp, CV_CDV_FACE_LIST **face, CV_CDV_FACE_LIST **reuse);
static BOOL read_cdv_bond_info(char *read_line, CV_BOND_TYPE **bond_type, char **file_name);
static BOOL read_cdv_face_info(char *read_line, CV_FACE_TYPE **face_type, char **file_name);
static BOOL read_bond_and_face_file(CV_FRAME *fr, unsigned int cdv_bond_flags);

#ifdef USE_GLSL
static CV_SHADER_BASE* allocate_shader_base();
static void free_shader_base(CV_SHADER_BASE *sb);
static BOOL init_shader_sphere_draw_info(CV_BASE *cb, CV_FRAME *fr);
static BOOL update_shader_sphere_draw_info(CV_BASE *cb, CV_FRAME *fr);
static void set_shader_sphere_draw_info_size(CV_BASE *cb, CV_FRAME *fr);
static void set_shader_sphere_draw_info_color(CV_BASE *cb, CV_FRAME *fr);
static BOOL update_shader_rod_draw_info(CV_BASE *cb, CV_FRAME *fr);
static void process_shader_pick(KS_GL_BASE *gb, CV_SHADER_BASE *sb, unsigned int pick_id);
static BOOL init_system_box_draw_info(CV_BASE *cb);
static BOOL init_rod_draw_info(CV_BASE *cb, CV_FRAME *fr);
#endif

static void __KS_USED__ trans_sp_mol_ueda(double *q, double *cd, double *tcd)
{
  tcd[0] = (cd[0]     *(-q[0]*q[0]+q[1]*q[1]-q[2]*q[2]+q[3]*q[3])+
	    cd[1]*( 2)*( q[2]*q[3]-q[0]*q[1])+
	    cd[2]*( 2)*( q[1]*q[2]+q[0]*q[3]));
  tcd[1] = (cd[0]*(-2)*( q[0]*q[1]+q[2]*q[3])+
	    cd[1]     *( q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3])+
	    cd[2]*( 2)*( q[1]*q[3]-q[0]*q[2]));
  tcd[2] = (cd[0]*( 2)*( q[1]*q[2]-q[0]*q[3])+
	    cd[1]*(-2)*( q[0]*q[2]+q[1]*q[3])+
	    cd[2]     *(-q[0]*q[0]-q[1]*q[1]+q[2]*q[2]+q[3]*q[3]));
}
static void trans_sp_mol_ueda0(double *q, double *tcd)
{
  tcd[0] = -q[0]*q[0]+q[1]*q[1]-q[2]*q[2]+q[3]*q[3];
  tcd[4] = (-2)*( q[0]*q[1]+q[2]*q[3]);
  tcd[8] = ( 2)*( q[1]*q[2]-q[0]*q[3]);
}
static void trans_sp_mol_ueda1(double *q, double *tcd)
{
  tcd[1] = ( 2)*( q[2]*q[3]-q[0]*q[1]);
  tcd[5] = q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3];
  tcd[9] = (-2)*( q[0]*q[2]+q[1]*q[3]);
}
static void trans_sp_mol_ueda2(double *q, double *tcd)
{
  tcd[2]  = ( 2)*( q[1]*q[2]+q[0]*q[3]);
  tcd[6]  = ( 2)*( q[1]*q[3]-q[0]*q[2]);
  tcd[10] = -q[0]*q[0]-q[1]*q[1]+q[2]*q[2]+q[3]*q[3];
}
static void __KS_USED__ trans_sp_mol_allen(double *q, double *cd, double *tcd)
{
  tcd[0] = (cd[0]     *( q[0]*q[0]+q[1]*q[1]-q[2]*q[2]-q[3]*q[3])+
	    cd[1]*( 2)*( q[1]*q[2]+q[0]*q[3])+
	    cd[2]*( 2)*( q[1]*q[3]-q[0]*q[2]));
  tcd[1] = (cd[0]*( 2)*( q[1]*q[2]-q[0]*q[3])+
	    cd[1]     *( q[0]*q[0]-q[1]*q[1]+q[2]*q[2]-q[3]*q[3])+
	    cd[2]*( 2)*( q[2]*q[3]+q[0]*q[1]));
  tcd[2] = (cd[0]*( 2)*( q[1]*q[3]+q[0]*q[2])+
	    cd[1]*( 2)*( q[2]*q[3]-q[0]*q[1])+
	    cd[2]     *( q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3]));
}
static void trans_sp_mol_allen0(double *q, double *tcd)
{
  tcd[0] = q[0]*q[0]+q[1]*q[1]-q[2]*q[2]-q[3]*q[3];
  tcd[4] = ( 2)*( q[1]*q[2]-q[0]*q[3]);
  tcd[8] = ( 2)*( q[1]*q[3]+q[0]*q[2]);
}
static void trans_sp_mol_allen1(double *q, double *tcd)
{
  tcd[1] = ( 2)*( q[1]*q[2]+q[0]*q[3]);
  tcd[5] = q[0]*q[0]-q[1]*q[1]+q[2]*q[2]-q[3]*q[3];
  tcd[9] = ( 2)*( q[2]*q[3]-q[0]*q[1]);
}
static void trans_sp_mol_allen2(double *q, double *tcd)
{
  tcd[2]  = ( 2)*( q[1]*q[3]-q[0]*q[2]);
  tcd[6]  = ( 2)*( q[2]*q[3]+q[0]*q[1]);
  tcd[10] = q[0]*q[0]-q[1]*q[1]-q[2]*q[2]+q[3]*q[3];
}
/*
static void (*trans_sp_mol[CV_QUATERNION_TYPE_NUM])(double *q, double *cd, double *tcd) = {
  trans_sp_mol_ueda,
  trans_sp_mol_allen
};
*/
static void (*trans_sp_mol[CV_QUATERNION_TYPE_NUM][3])(double *q, double *tcd) = {
  {trans_sp_mol_ueda0, trans_sp_mol_ueda1, trans_sp_mol_ueda2},
  {trans_sp_mol_allen0,trans_sp_mol_allen1,trans_sp_mol_allen2}
};


static void draw_error(CV_BASE *cb, char *fmt, ...)
{
  char msg[256];
  double print_time = 5;
  va_list argp;
  va_start(argp, fmt);
#ifdef MSVC
  _vsnprintf(msg,sizeof(msg),fmt,argp);
#else
  vsnprintf(msg,sizeof(msg),fmt,argp);
#endif
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,print_time));
  va_end(argp);
}
static size_t calc_hash(unsigned int label, int size)
{
  return label % size;
}
static CV_ATOM *new_cv_atom(unsigned int flags, int atype, char *name, float radius, float *color, 
			    int detail, float *scale, double mass)
{
  CV_ATOM *newp;

  if((newp = (CV_ATOM*)ks_malloc(sizeof(CV_ATOM),"CV_ATOM")) == NULL){
    ks_error_memory();
    return NULL;
  }

  newp->flags = flags;
  newp->atype = atype;
  if(name == NULL){
    newp->name = NULL;
  } else {
    if((newp->name = ks_malloc_char_copy(name,"new_atom")) == NULL)
      return NULL;
  }
  newp->gl_atom = ks_allocate_gl_atom(radius,color,detail,scale,KS_FALSE);
  /*  newp->gl_atom = ks_allocate_gl_atom(radius,color,detail,scale);*/
  /*
  newp->radius = radius;
  newp->color[0] = color[0];
  newp->color[1] = color[1];
  newp->color[2] = color[2];
  */
  newp->mass = mass;
  newp->next = NULL;
  return newp;
}
static CV_ATOM *add_cv_atom(CV_ATOM *listp, CV_ATOM *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_ATOM *addend_cv_atom(CV_ATOM *listp, CV_ATOM *newp)
{
  CV_ATOM *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static CV_ATOM *lookup_cv_atom(CV_ATOM *listp, char *name)
{
  for(; listp != NULL; listp = listp->next){
    if(strcmp(listp->name,name) == 0)
      return listp;
  }
  return NULL;
}
static CV_ATOM *lookup_cv_atom_atype(CV_ATOM *listp, int atype)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->atype == atype)
      return listp;
  }
  return NULL;
}
static int count_cv_atom(CV_ATOM *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_ATOM* __KS_USED__ cat_cv_atom(CV_ATOM *listp, CV_ATOM *newp)
{
  CV_ATOM *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_atom(CV_ATOM *listp)
{
  CV_ATOM *next;
  for(; listp != NULL; listp = next){
    ks_free_gl_atom(listp->gl_atom);
    ks_free(listp->name);
    next = listp->next;
    ks_free(listp);
  }
}
static CV_BOND_TYPE *new_cv_bond_type(unsigned int type, GLfloat *color, float r)
{
  int i;
  CV_BOND_TYPE *newp;

  if((newp = (CV_BOND_TYPE*)ks_malloc(sizeof(CV_BOND_TYPE),"CV_BOND_TYPE")) == NULL){
    ks_error_memory();
    return NULL;
  }
  newp->type = type;
  for(i = 0; i < 3; i++)
    newp->color[i] = color[i];
  newp->radius = r;
  newp->next = NULL;
  return newp;
}
static CV_BOND_TYPE *add_cv_bond_type(CV_BOND_TYPE *listp, CV_BOND_TYPE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_BOND_TYPE* __KS_USED__ addend_cv_bond_type(CV_BOND_TYPE *listp, CV_BOND_TYPE *newp)
{
  CV_BOND_TYPE *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static int __KS_USED__ count_cv_bond_type(CV_BOND_TYPE *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_BOND_TYPE *lookup_cv_bond_type(CV_BOND_TYPE *listp, int type)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->type == type)
      return listp;
  }
  return NULL;
}
static CV_BOND_TYPE* __KS_USED__ cat_cv_bond_type(CV_BOND_TYPE *listp, CV_BOND_TYPE *newp)
{
  CV_BOND_TYPE *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_bond_type(CV_BOND_TYPE *listp)
{
  CV_BOND_TYPE *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static CV_FACE_TYPE *new_cv_face_type(unsigned int type, GLfloat *color)
{
  int i;
  CV_FACE_TYPE *newp;

  if((newp = (CV_FACE_TYPE*)ks_malloc(sizeof(CV_FACE_TYPE),"CV_FACE_TYPE")) == NULL){
    ks_error_memory();
    return NULL;
  }
  newp->type = type;
  for(i = 0; i < 3; i++)
    newp->color[i] = color[i];
  newp->next = NULL;
  return newp;
}
static CV_FACE_TYPE *add_cv_face_type(CV_FACE_TYPE *listp, CV_FACE_TYPE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_FACE_TYPE* __KS_USED__ addend_cv_face_type(CV_FACE_TYPE *listp, CV_FACE_TYPE *newp)
{
  CV_FACE_TYPE *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static int __KS_USED__ count_cv_face_type(CV_FACE_TYPE *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_FACE_TYPE *lookup_cv_face_type(CV_FACE_TYPE *listp, int type)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->type == type)
      return listp;
  }
  return NULL;
}
static CV_FACE_TYPE* __KS_USED__ cat_cv_face_type(CV_FACE_TYPE *listp, CV_FACE_TYPE *newp)
{
  CV_FACE_TYPE *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_face_type(CV_FACE_TYPE *listp)
{
  CV_FACE_TYPE *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static CV_BOND *new_cv_bond(unsigned int flags, CV_PARTICLE *p, double len, CV_BOND **reuse)
{
  CV_BOND *newp;

  if(*reuse == NULL){
    if((newp = (CV_BOND*)ks_malloc(sizeof(CV_BOND),"CV_BOND")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }
  newp->flags = flags;
  newp->p = p;
  newp->len = len;
  newp->type = NULL;
  newp->next = NULL;
  return newp;
}
static CV_BOND *add_cv_bond(CV_BOND *listp, CV_BOND *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_BOND* __KS_USED__ addend_cv_bond(CV_BOND *listp, CV_BOND *newp)
{
  CV_BOND *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static int count_cv_bond(CV_BOND *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_BOND *cat_cv_bond(CV_BOND *listp, CV_BOND *newp)
{
  CV_BOND *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static CV_BOND *del_cv_bond(CV_BOND *listp, CV_PARTICLE *particle)
{
  CV_BOND *p, *prev;

  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(p->p == particle){
      if(prev == NULL)
	listp = p->next;
      else
	prev->next = p->next;
      ks_free(p);
      return listp;
    }
    prev = p;
  }
  ks_error("%p is not found",particle);
  return NULL;
}
static void free_cv_bond(CV_BOND *listp)
{
  CV_BOND *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static CV_CDV_BOND_LIST *new_cv_cdv_bond_list(unsigned int flags,
					      unsigned int type, unsigned int label[2], 
					      CV_CDV_BOND_LIST **reuse)
{
  CV_CDV_BOND_LIST *newp;

  if(*reuse == NULL){
    if((newp = (CV_CDV_BOND_LIST*)ks_malloc(sizeof(CV_CDV_BOND_LIST),"CV_CDV_BOND_LIST")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }
  newp->flags = flags;
  newp->type = type;
  newp->label[0] = label[0];
  newp->label[1] = label[1];
  newp->next = NULL;
  return newp;
}
static CV_CDV_BOND_LIST *add_cv_cdv_bond_list(CV_CDV_BOND_LIST *listp, CV_CDV_BOND_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static int __KS_USED__ count_cv_cdv_bond_list(CV_CDV_BOND_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_CDV_BOND_LIST *cat_cv_cdv_bond_list(CV_CDV_BOND_LIST *listp, CV_CDV_BOND_LIST *newp)
{
  CV_CDV_BOND_LIST *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_cdv_bond_list(CV_CDV_BOND_LIST *listp)
{
  CV_CDV_BOND_LIST *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static CV_CDV_FACE_LIST *new_cv_cdv_face_list(unsigned int type, unsigned int label[2], 
					      CV_CDV_FACE_LIST **reuse)
{
  CV_CDV_FACE_LIST *newp;

  if(*reuse == NULL){
    if((newp = (CV_CDV_FACE_LIST*)ks_malloc(sizeof(CV_CDV_FACE_LIST),"CV_CDV_FACE_LIST")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }
  newp->type = type;
  newp->label[0] = label[0];
  newp->label[1] = label[1];
  newp->label[2] = label[2];
  newp->next = NULL;
  return newp;
}
static CV_CDV_FACE_LIST *add_cv_cdv_face_list(CV_CDV_FACE_LIST *listp, CV_CDV_FACE_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static int count_cv_cdv_face_list(CV_CDV_FACE_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_CDV_FACE_LIST *cat_cv_cdv_face_list(CV_CDV_FACE_LIST *listp, CV_CDV_FACE_LIST *newp)
{
  CV_CDV_FACE_LIST *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_cdv_face_list(CV_CDV_FACE_LIST *listp)
{
  CV_CDV_FACE_LIST *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static CV_RESIDUE *new_cv_residue(unsigned int label, unsigned int flags, int number, char *name,
				  unsigned int file_label, CV_RESIDUE **reuse)
{
  CV_RESIDUE *newp;

  if(*reuse == NULL){
    if((newp = (CV_RESIDUE*)ks_malloc(sizeof(CV_RESIDUE),"CV_RESIDUE")) == NULL){
      ks_error_memory();
      return NULL;
    }
    newp->particle = NULL;
    newp->particle_num = 0;
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
    /*    printf("reuse residue %s %d -> %s\n",newp->name,newp->particle_num,name);*/
    newp->particle_num0 = newp->particle_num;
    newp->particle_num = 0;
  }

  newp->label = label;
  newp->flags = flags;
  newp->file_label = file_label;
  newp->number = number;
  strcpy(newp->name,name);
  /*
  newp->type = '\0';
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(ks_amino[i].name,name) == 0){
      newp->type = ks_amino[i].c;
    }
  }
  */
  ks_get_amino_name_to_char(name,&newp->type);
  newp->main_N = NULL;
  newp->main_C = NULL;
  newp->main_CA= NULL;
  newp->main_O = NULL;
  newp->next = NULL;
  newp->hash_next = NULL;
  return newp;
}
static CV_RESIDUE* __KS_USED__ add_cv_residue(CV_RESIDUE *listp, CV_RESIDUE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_RESIDUE *addend_cv_residue(CV_RESIDUE *listp, CV_RESIDUE *newp)
{
  CV_RESIDUE *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static CV_RESIDUE *add_cv_residue_hash(CV_RESIDUE *listp, CV_RESIDUE *newp)
{
  if(newp == NULL) return listp;
  newp->hash_next = listp;
  return newp;
}
static CV_RESIDUE* __KS_USED__ lookup_cv_residue(CV_RESIDUE *listp, unsigned int value)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->label == value)
      return listp;
  }
  return NULL;
}
static CV_RESIDUE *lookup_cv_residue_hash_label(CV_RESIDUE **hash, unsigned int hash_size,
						  unsigned int value)
{
  CV_RESIDUE *listp;
  listp = hash[calc_hash(value,hash_size)];
  for(; listp != NULL; listp = listp->hash_next){
    if(listp->label == value)
      return listp;
  }
  return NULL;
}
static int count_cv_residue(CV_RESIDUE *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_RESIDUE *cat_cv_residue(CV_RESIDUE *listp, CV_RESIDUE *newp)
{
  CV_RESIDUE *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_residue(CV_RESIDUE *listp)
{
  CV_RESIDUE *next;
  for(; listp != NULL; listp = next){
    if(listp->particle != NULL){
      ks_free(listp->particle);
    }
    next = listp->next;
    ks_free(listp);
  }
}
static CV_CHAIN *new_cv_chain(unsigned int label, unsigned int flags, char *name, int len,
			      CV_RESIDUE *residue, CV_CHAIN **reuse)
{
  CV_CHAIN *newp;

  if(*reuse == NULL){
    if((newp = (CV_CHAIN*)ks_malloc(sizeof(CV_CHAIN),"CV_CHAIN")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }

  newp->label = label;
  newp->flags = flags;
  if(strlen(name) > CV_NAME_MAX){
    ks_error("name %s is too long",name);
    return NULL;
  }
  strcpy(newp->name,name);
  newp->len = len;
  newp->residue = residue;
  newp->ses = NULL;
  newp->next = NULL;
  /*  printf("new chain %d %s\n",label,name);*/
  return newp;
}
static CV_CHAIN* __KS_USED__ add_cv_chain(CV_CHAIN *listp, CV_CHAIN *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_CHAIN *addend_cv_chain(CV_CHAIN *listp, CV_CHAIN *newp)
{
  CV_CHAIN *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static CV_CHAIN* __KS_USED__ lookup_cv_chain(CV_CHAIN *listp, unsigned int value)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->label == value)
      return listp;
  }
  return NULL;
}
static int __KS_USED__ count_cv_chain(CV_CHAIN *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_CHAIN *cat_cv_chain(CV_CHAIN *listp, CV_CHAIN *newp)
{
  CV_CHAIN *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_chain(CV_CHAIN *listp)
{
  CV_CHAIN *next;
  for(; listp != NULL; listp = next){
    /*    printf("free chain %d %s\n",listp->label,listp->name);*/
    free_cv_residue(listp->residue);
    if(listp->ses != NULL){
      if(listp->ses->pl != NULL)
	free_cv_solvent_exclude_surface_particle(listp->ses);
      if(listp->ses->vertex != NULL)
	free_cv_solvent_exclude_surface_vertex(listp->ses);
      if(listp->ses->face != NULL)
	free_cv_solvent_exclude_surface_face(listp->ses);
      ks_free(listp->ses);
    }
    next = listp->next;
    ks_free(listp);
  }
}
static CV_AGGREGATE *new_cv_aggregate(unsigned int label, unsigned int flags, CV_CHAIN *chain,
				      CV_AGGREGATE **reuse)
{
  CV_AGGREGATE *newp;

  if(*reuse == NULL){
    if((newp = (CV_AGGREGATE*)ks_malloc(sizeof(CV_AGGREGATE),"CV_AGGREGATE")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }

  /*  printf("new agg %p %d\n",newp,label);*/
  newp->label = label;
  newp->flags = flags;
  newp->chain = chain;
  newp->next = NULL;
  return newp;
}
static CV_AGGREGATE *add_cv_aggregate(CV_AGGREGATE *listp, CV_AGGREGATE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_AGGREGATE *addend_cv_aggregate(CV_AGGREGATE *listp, CV_AGGREGATE *newp)
{
  CV_AGGREGATE *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static CV_AGGREGATE* __KS_USED__ lookup_cv_aggregate(CV_AGGREGATE *listp, unsigned int value)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->label == value)
      return listp;
  }
  return NULL;
}
static int __KS_USED__ count_cv_aggregate(CV_AGGREGATE *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_AGGREGATE *cat_cv_aggregate(CV_AGGREGATE *listp, CV_AGGREGATE *newp)
{
  CV_AGGREGATE *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_aggregate(CV_AGGREGATE *listp)
{
  CV_AGGREGATE *next;
  for(; listp != NULL; listp = next){
    /*    printf("free agg %p %d\n",listp,listp->label);*/
    free_cv_chain(listp->chain);
    next = listp->next;
    /*    printf("%p %d\n",listp,listp->label);*/
    ks_free(listp);
  }
}
static CV_PARTICLE *new_cv_particle(unsigned int label, unsigned int file_label, 
				    unsigned int flags, CV_ATOM *atom, 
				    CV_PARTICLE_PROPERTY *property, 
				    double x, double y, double z, CV_PARTICLE **reuse)
{
  int i;
  CV_PARTICLE *newp;

  if((*reuse) == NULL){
    if((newp = (CV_PARTICLE*)ks_malloc(sizeof(CV_PARTICLE),"CV_PARTICLE")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }
  newp->label = label;
  newp->file_label = file_label;
  newp->flags = flags;
  newp->atype[0] = atom->atype;
  for(i = 1; i < CV_ATYPE_MODE_MAX; i++)
    newp->atype[i] = -1;
  newp->atom = atom;
  newp->property = property;
  newp->cd[0] = x;
  newp->cd[1] = y;
  newp->cd[2] = z;
  for(i = 0; i < 3; i++)
    newp->move[i] = 0;
  for(i = 0; i < CV_COLOR_MODE_NUM; i++)
    newp->color_mode[i] = 0;
  newp->bond = NULL;
  newp->prev = NULL;
  newp->next = NULL;
  newp->hash_next = NULL;
#ifdef USE_GLSL
  newp->draw_info_elem_p = NULL;
#endif
  return newp;
}
static CV_PARTICLE *add_cv_particle(CV_PARTICLE *listp, CV_PARTICLE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  if(listp != NULL)
    listp->prev = newp;
  return newp;
}
static CV_PARTICLE *add_cv_particle_hash(CV_PARTICLE *listp, CV_PARTICLE *newp)
{
  if(newp == NULL) return listp;
  newp->hash_next = listp;
  return newp;
}
static CV_PARTICLE* __KS_USED__ addend_cv_particle(CV_PARTICLE *listp, CV_PARTICLE *newp)
{
  CV_PARTICLE *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static CV_PARTICLE* __KS_USED__ lookup_cv_particle_label(CV_PARTICLE *listp, unsigned int value)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->label == value)
      return listp;
  }
  return NULL;
}
static CV_PARTICLE *lookup_cv_particle_hash_label(CV_PARTICLE **hash, unsigned int hash_size,
						  unsigned int value)
{
  CV_PARTICLE *listp;
  listp = hash[calc_hash(value,hash_size)];
  /*
  printf("%p %d %d %d\n",listp,calc_hash(value,hash_size),value,hash_size);
  */
  for(; listp != NULL; listp = listp->hash_next){
    /*
    printf("%p %d %s %s\n",
	   listp,listp->label,listp->property->name,listp->property->residue->name);
    */
    if(listp->label == value)
      return listp;
  }
  return NULL;
}
static void del_cv_particle_hash_label(CV_PARTICLE **hash, unsigned int hash_size,
				       unsigned int value)
{
  CV_PARTICLE *p,**pl,*prev = NULL;
  pl = &hash[calc_hash(value,hash_size)];
  /*
  printf("%p %d %d %d\n",*pl,calc_hash(value,hash_size),value,hash_size);
  */
  for(p = *pl; p != NULL; p = p->hash_next){
    if(p->label == value){
      /*
      printf("%p %d %s %s %p\n",
	     p,p->label,p->property->name,p->property->residue->name,prev);
      */
      if(prev == NULL){
	(*pl) = p->hash_next;
      } else {
	prev->hash_next = p->hash_next;
      }
    }
    prev = p;
  }
  /*  printf("%p\n",*pl);*/
}
static int count_cv_particle(CV_PARTICLE *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_PARTICLE *cat_cv_particle(CV_PARTICLE *listp, CV_PARTICLE *newp)
{
  CV_PARTICLE *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_particle(CV_PARTICLE *listp)
{
  CV_PARTICLE *next;
  for(; listp != NULL; listp = next){
    if(listp->property != NULL){
      free_cv_particle(listp->property->alternative);
      ks_free(listp->property);
    }
    free_cv_bond(listp->bond);
    next = listp->next;
    ks_free(listp);
  }
}
static CV_PARTICLE_PROPERTY *new_cv_particle_property(char *name,
						      CV_AGGREGATE *agg, CV_CHAIN *chain, 
						      CV_RESIDUE *residue,CV_PARTICLE *alternative,
						      CV_PARTICLE_PROPERTY **reuse)
{
  CV_PARTICLE_PROPERTY *newp;

  if(*reuse == NULL){
    if((newp = (CV_PARTICLE_PROPERTY*)ks_malloc(sizeof(CV_PARTICLE_PROPERTY),
						"CV_PARTICLE_PROPERTY")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
  }

  strcpy(newp->name,name);
  newp->aggregate = agg;
  newp->chain = chain;
  newp->residue = residue;
  newp->alternative = alternative;
  newp->next = NULL;
  return newp;
}
static CV_PARTICLE_PROPERTY* __KS_USED__ add_cv_particle_property(CV_PARTICLE_PROPERTY *listp, 
								  CV_PARTICLE_PROPERTY *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static CV_PARTICLE_PROPERTY* __KS_USED__ addend_cv_particle_property(CV_PARTICLE_PROPERTY *listp, 
								     CV_PARTICLE_PROPERTY *newp)
{
  CV_PARTICLE_PROPERTY *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static int __KS_USED__ count_cv_particle_property(CV_PARTICLE_PROPERTY *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static CV_PARTICLE_PROPERTY *cat_cv_particle_property(CV_PARTICLE_PROPERTY *listp, 
						      CV_PARTICLE_PROPERTY *newp)
{
  CV_PARTICLE_PROPERTY *p;
  if(newp == NULL){
    return listp;
  } else {
    for(p = newp; p->next != NULL; p = p->next);
    p->next = listp;
    return newp;
  }
}
static void free_cv_particle_property(CV_PARTICLE_PROPERTY *listp)
{
  CV_PARTICLE_PROPERTY *next;
  for(; listp != NULL; listp = next){
    free_cv_particle(listp->alternative);
    next = listp->next;
    ks_free(listp);
  }
}
static BOOL guess_atom_symbol(char *type, char element[3])
{
  char symbol[3];
  int atomic_number;
  for(;type && !isalpha(*(unsigned char*)type);type++);

  if(type == NULL){
    strcpy(element,"??");
    return KS_FALSE;
  }

  symbol[0] = type[0];
  if(type[1] == ' ' || type[1] == '+' || type[1] == '-'){
    symbol[1] = '\0';
    if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
      strcpy(element,"??");
      return KS_FALSE;
    }
  } else if(strcmp(type,"HE  ") == 0 || strcmp(type,"HG  ") == 0){
    strcpy(symbol,"H");
  } else if(strcmp(type,"CA  ") == 0 || strcmp(type,"CD  ") == 0 || strcmp(type,"CE  ") == 0){
    strcpy(symbol,"C");
  } else if(strcmp(type,"NE  ") == 0){
    strcpy(symbol,"N");
  } else if(strcmp(type,"SG  ") == 0){
    strcpy(symbol,"S");
  } else if(strcmp(type,"PA  ") == 0 || strcmp(type,"PB  ") == 0){
    strcpy(symbol,"P");
  } else if(strcmp(type,"MG  ") == 0){
    strcpy(symbol,"Mg");
  } else if(strcmp(type,"FE  ") == 0){
    strcpy(symbol,"Fe");
  } else if(strcmp(type,"NA+ ") == 0){
    strcpy(symbol,"Na");
  } else if(strcmp(type,"CL- ") == 0){
    strcpy(symbol,"Cl");
  } else if(type[2] == ' ' || type[2] == '+' || type[2] == '-'){
    if(isalpha(((unsigned char*)type)[1])){
      if(isupper(((unsigned char*)type)[1])){
	symbol[1] = '\0';
	if(ks_get_atom_number(symbol,&atomic_number) == KS_TRUE){
	  goto GUESS_ATOM_SYMBOL_OUT;
	}
      }
      symbol[1] = tolower(((unsigned char*)type)[1]);
      symbol[2] = '\0';
      if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
	symbol[1] = '\0';
      } else {
	/*	printf("%s %s %d\n",type,symbol,atomic_number);*/
	if(atomic_number < 40){
	  symbol[1] = '\0';
	  if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
	    symbol[0] = type[0];
	    symbol[1] = tolower(((unsigned char*)type)[1]);
	    symbol[2] = '\0';
	  }
	}
      }
    } else {
      symbol[1] = '\0';
    }
    if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
      strcpy(element,"??");
      return KS_FALSE;
    }
  } else if(strcmp(type,"DU") == 0){
    strcpy(symbol,"DU");
  } else if(type[0] == 'L'){
    strcpy(symbol,"LJ");
  } else {             /* 3,4 char */
    symbol[1] = '\0';
    if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
      strcpy(element,"??");
      return KS_FALSE;
    }
    /*    printf("%s %s %s %d\n",type,symbol,element,atomic_number);*/
  }
  /*
  printf("symbol %s\n",symbol);
  ks_exit(EXIT_FAILURE);
  */
  /*
  if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
    symbol[1] = '\0';
    if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
      symbol[0] = type[0];
      if(isalpha(type[1])){
	symbol[1] = tolower(type[1]);
	symbol[2] = '\0';
      }
      if(ks_get_atom_number(symbol,&atomic_number) == KS_FALSE){
	strcpy(element,"??");
	return KS_FALSE;
      }
    }
  }
  */
 GUESS_ATOM_SYMBOL_OUT:
  strcpy(element,symbol);
  return KS_TRUE;
}
static CV_AGGREGATE *set_cv_aggregate_sequence(unsigned int *aggregate_label, 
					       unsigned int *chain_label, 
					       unsigned int *residue_label, KS_PDB_BUF *pdb,
					       CV_REUSE *reuse)
{
  int i;
  CV_AGGREGATE *agg = NULL;
  CV_CHAIN *chain = NULL;
  CV_RESIDUE *res = NULL, *new_res;
  int residue_num = 0, residue_head = 0;

  for(i = 0; i < pdb->num; i++){
    if(!(pdb->atom[i]->flags&KS_PDB_ATOM_WATER)){
      /*    printf("%d\n",i);*/
      /*      ks_fprint_pdb_atom(stdout,*pdb->atom[i]);*/
      if(pdb->atom[i]->flags&KS_PDB_ATOM_RESIDUE_END){
	new_res = new_cv_residue((*residue_label)++,0,residue_num,pdb->atom[i]->residue,
				 pdb->atom[i]->residue_label,&reuse->residue);
	residue_num++;
	new_res->pdb_residue_head = residue_head;
	new_res->particle_num = i-residue_head+1;
	res = addend_cv_residue(res,new_res);
	/*	printf("    %d RESIDUE_END\n",residue_num);*/
	residue_head = i+1;
      }
      if(pdb->atom[i]->flags&KS_PDB_ATOM_CHAIN_END){
	chain = addend_cv_chain(chain,new_cv_chain((*chain_label)++,0,
						   residue_num == 1?pdb->atom[i]->residue:"CHAIN",
						   residue_num,res,&reuse->chain));
	res = NULL;
	/*	printf("  %d CHAIN_END %d\n",*chain_label,residue_num);*/
	residue_num = 0;
      }
      if(pdb->atom[i]->flags&KS_PDB_ATOM_AGGREGATE_END){
	/*
	  printf("%d ",i);
	  ks_fprint_pdb_atom(stdout,*pdb->atom[i]);
	*/
	agg = addend_cv_aggregate(agg,new_cv_aggregate((*aggregate_label)++,0,chain,
						       &reuse->aggregate));
	chain = NULL;
	/*	printf("AGGREGATE_END %d\n",*chain_label);*/
      }
    }
  }
  /*  ks_assert(chain != NULL);*/
  /*  ks_exit(EXIT_FAILURE);*/

  return agg;
}
static int get_unuse_atype(BOOL *atype_use)
{
  int atype;
  for(atype = 0; atype < CV_ATOM_TYPE_MAX; atype++){
    if(atype_use[atype] == KS_FALSE){
      atype_use[atype] = KS_TRUE;
      return atype;
    }
  }
  ks_error("too meny atom types");
  return -1;
}
static CV_ATOM *make_cv_atom(CV_ATOM **atom, char *name, BOOL *atype_use)
{
  CV_ATOM *hit_atom;
  float color[3];
  float radius;
  double weight;
  char element[3];
  int atype;

  guess_atom_symbol(name,element);
  /*  printf("%s %s\n",name,element);*/
  hit_atom = lookup_cv_atom((*atom),element);
  if(hit_atom == NULL){
    if(ks_get_atom_color(element,color) == KS_FALSE){
      ks_get_gl_unknown_color(0,color);
      radius = 1.0;
    } else {
      ks_get_atom_radius(element,&radius);
    }
    /*    printf("%s %s %f %f %f\n",name,element,color[0],color[1],color[2]);*/
    if(ks_get_atom_weight(element,&weight) == KS_FALSE){
      weight = 1.0;
    }
    if((atype = get_unuse_atype(atype_use)) < 0){
      return NULL;
    }
    hit_atom = new_cv_atom(CV_ATOM_SHOW,atype, element,radius,color,
			   CV_SPHERE_DETAIL_NORMAL,NULL,weight);
    (*atom) = addend_cv_atom((*atom),hit_atom);
  }
  return hit_atom;
}
static void set_water(CV_PARTICLE **pl, CV_PARTICLE *pw[], CV_REUSE *reuse)
{
  int i,j;
  enum {O,H1,H2,NUM};
  double d0;
  static double len = -1;

  if(len < 0 && pw[H1] != NULL){
    len = 0;
    for(i = 0;i < 3; i++){
      d0 = pw[O]->cd[i]-pw[H1]->cd[i];
      len += d0*d0;
    }
    len = sqrt(len);
  }
  for(i = 0; i < NUM; i++){
    if(pw[i] != NULL){
      if(i == O){
	for(j = 1; j < NUM; j++){
	  if(pw[j] != NULL){
	    pw[O]->bond = add_cv_bond(pw[O]->bond,new_cv_bond(0,pw[j],len,&reuse->bond));
	  }
	}
      } else {
	pw[i]->bond = add_cv_bond(pw[i]->bond,new_cv_bond(0,pw[O],len,&reuse->bond));
      }
      /*
      {
	CV_PARTICLE *p;
	CV_BOND *bond;
	printf("%d %s %s %f %f %f\n",pw[i]->label,pw[i]->atom->name,pw[i]->property->name,
	       pw[i]->cd[0],pw[i]->cd[1],pw[i]->cd[2]);
	for(bond = pw[i]->bond; bond != NULL; bond = bond->next){
	  p = bond->p;
	  printf("  %d %s %s %f %f %f %f\n",p->label,p->atom->name,p->property->name,
		 p->cd[0],p->cd[1],p->cd[2],bond->len);
	}
      }
      */
      (*pl) = add_cv_particle((*pl),pw[i]);
    }
  }
  for(i = 0; i < NUM; i++)
    pw[i] = NULL;
}
static BOOL set_water_coordinate(unsigned int *label, KS_PDB_BUF *pdb, CV_ATOM **atom,
				 CV_PARTICLE **particle, BOOL *atype_use,
				 CV_REUSE *reuse)
{
  int i;
  int type;
  enum {O,H1,H2,NUM};
  CV_PARTICLE *pw[NUM] = {NULL, NULL, NULL};
  CV_ATOM *hit_atom;

  for(i = 0; i < pdb->num; i++){
    if(pdb->atom[i]->flags&KS_PDB_ATOM_WATER){
      /*      ks_fprint_pdb_atom(stdout,*pdb->atom[i]);*/
      if(strcmp(pdb->atom[i]->name,"O   ") == 0 || pdb->atom[i]->name[0] == 'O'){
	type = O;
	set_water(particle,pw,reuse);
      } else if(strcmp(pdb->atom[i]->name,"H1  ") == 0){
	type = H1;
      } else if(strcmp(pdb->atom[i]->name,"H2  ") == 0){
	type = H2;
      } else {
	ks_error("Unknown water type %s",pdb->atom[i]->name);
	ks_fprint_pdb_atom(stdout,*pdb->atom[i]);
	return KS_FALSE;
      }
      hit_atom = make_cv_atom(atom,pdb->atom[i]->name,atype_use);
      pw[type]=new_cv_particle((*label)++,pdb->atom[i]->label,CV_PARTICLE_WATER,hit_atom,NULL,
			       pdb->atom[i]->cd[0],
			       pdb->atom[i]->cd[1],
			       pdb->atom[i]->cd[2],
			       &reuse->particle);
    }
  }
  set_water(particle,pw,reuse);
  return KS_TRUE;
}
static void __KS_USED__ print_ser(CV_FRAME *fr)
{
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  CV_PARTICLE *p;
  int i;

  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	if(pr->file_label == 870){
	  for(i = 0; i < pr->particle_num; i++){
	    p = pr->particle[i];
	    printf("      %d %d %s '%s' %p\n",i,p->file_label,p->atom->name,
		   p->property->name,p->property->name);
	  }
	}
      }
    }
  }
}
CV_COLOR_MAP *cv_allocate_color_map(void)
{
  CV_COLOR_MAP *color_map;
  if((color_map = (CV_COLOR_MAP*)ks_malloc(sizeof(CV_COLOR_MAP),"color_map")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }

  color_map->num = 0;
  color_map->malloc_num = 0;
  color_map->color = NULL;
  color_map->value = NULL;
  return color_map;
}
BOOL cv_allocate_color_map_color(CV_COLOR_MAP *color_map)
{
  int i;
  color_map->malloc_num = color_map->num;
  if((color_map->color = (GLfloat**)ks_malloc(color_map->num*sizeof(GLfloat*),
					      "color_map->color")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(i = 0; i < color_map->num; i++){
    if((color_map->color[i] = (GLfloat*)ks_malloc(3*sizeof(GLfloat),"color_map->color[i]"))
       ==NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if((color_map->value = ks_malloc_double_p(color_map->num,"color_map->value")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  return KS_TRUE;
}
void cv_free_color_map(CV_COLOR_MAP *color_map)
{
  int i;
  if(color_map->color != NULL){
    for(i = 0; i < color_map->num; i++){
      ks_free(color_map->color[i]);
    }
    ks_free(color_map->color);
  }
  if(color_map->value != NULL){
    ks_free(color_map->value);
  }
  ks_free(color_map);
}
static CV_ADDITIONAL_SURFACE* allocate_additional_surface(void)
{
  CV_ADDITIONAL_SURFACE* sur;
  if((sur = (CV_ADDITIONAL_SURFACE*)ks_malloc(sizeof(CV_ADDITIONAL_SURFACE),"sur")) == NULL){
    ks_error_memory();
    return NULL;
  }
  sur->transparency = 1.0;
  sur->cull_face = KS_TRUE;
  sur->vertex = NULL;
  sur->normal = NULL;
  sur->color = NULL;
  sur->face = NULL;
  return sur;
}
static BOOL allocate_additional_surface_vertex(CV_ADDITIONAL_SURFACE* sur, int vertex_num)
{
  sur->vertex_num = vertex_num;
  sur->vertex_malloc_num = vertex_num;
  if((sur->vertex = (GLfloat*)ks_malloc(sur->vertex_num*3*sizeof(GLfloat),"sur->vertex")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  if((sur->normal = (GLfloat*)ks_malloc(sur->vertex_num*3*sizeof(GLfloat),"sur->normal")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  if((sur->color = (GLfloat*)ks_malloc(sur->vertex_num*4*sizeof(GLfloat),"sur->color")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  return KS_TRUE;
}
static BOOL allocate_additional_surface_face(CV_ADDITIONAL_SURFACE* sur, int face_num)
{
  sur->face_num = face_num;
  sur->face_malloc_num = face_num;
  if((sur->face = (int*)ks_malloc(sur->face_num*3*sizeof(int),"sur->face")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  return KS_TRUE;
}
static void free_additional_surface(CV_ADDITIONAL_SURFACE *sur)
{
  ks_free(sur->vertex);
  ks_free(sur->normal);
  ks_free(sur->color);
  ks_free(sur->face);
  ks_free(sur);
}
static BOOL read_additional_surface(CV_FRAME *fr, char *file_name)
{
  int i;
  int vertex_num = 0, face_num = 0;
  FILE *fp;
  char read_buf[256];
  char path[256], *cp;

 READ_ADDITIONAL_SURFACE_START :;
  if((fp = fopen(file_name,"rt")) == NULL){             // surface file can not be found
    ks_get_file_path(fr->file_name,path,sizeof(path));  // get file path from cdv file
    if(path[0] != '\0'){                                // make surface file with the path
      if(strlen(path)+strlen(file_name) < sizeof(path)){
	strcat(path,file_name);
	if((fp = fopen(path,"rt")) == NULL){   // open surface with the path from cdv file
	  if((cp = strstr(file_name,"\\")) == NULL){ // try to remove path from surface file
	    cp = strstr(file_name,"/");
	  }
	  if(cp == NULL){            // surface file has no path
	    ks_error_file(file_name);
	    return KS_FALSE;
	  } else {                   // surface file has a path
	    cp++;
	    file_name = cp;          // remove path from the surface file
	    goto READ_ADDITIONAL_SURFACE_START;  // try again
	  }
	}
      } else {
	ks_error("file name is too long path %s name %s",path,file_name);
	return KS_FALSE;
      }
    } else {  // cdv file has no path
      if(strstr(file_name,"\\") == NULL && strstr(file_name,"/") == NULL){//surface file has no path
	ks_error_file(file_name);
	return KS_FALSE;
      } else {  // surface file has a path
	if((cp = strstr(file_name,"\\")) == NULL){
	  cp = strstr(file_name,"/");
	}
	cp++;
	if((fp = fopen(cp,"rt")) == NULL){
	  file_name = cp;                      // remove path from the surface file
	  goto READ_ADDITIONAL_SURFACE_START;  // try again
	}
      }
    }
  }

  if(fr->sur == NULL){
    if(fr->reuse->sur != NULL){
      fr->sur = fr->reuse->sur;   // reuse sur
      fr->reuse->sur = NULL;
    } else {
      if((fr->sur = allocate_additional_surface()) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    }
  }

  if(fr->color_map == NULL){
    if(fr->reuse->color_map != NULL){
      fr->color_map = fr->reuse->color_map;  // reuse color_map
      fr->reuse->color_map = NULL;
    } else {
      if((fr->color_map = cv_allocate_color_map()) == NULL){
	return KS_FALSE;
      }
    }
  }

  if(fgets(read_buf,sizeof(read_buf),fp) == NULL){
    ks_error("Illegal file format %s\n",file_name);
    return KS_FALSE;
  }
  if(strncmp(read_buf,"color_num",9) == 0 || strncmp(read_buf,"color_count",11) == 0){
    if(strncmp(read_buf,"color_num",9) == 0){
      sscanf(read_buf,"color_num %d",&fr->color_map->num);
    } if(strncmp(read_buf,"color_count",11) == 0){
      sscanf(read_buf,"color_count %d",&fr->color_map->num);
    }
    if(fr->color_map->color != NULL && fr->color_map->malloc_num < fr->color_map->num){
      for(i = 0; i < fr->color_map->num; i++){
	ks_free(fr->color_map->color[i]);
      }
      ks_free(fr->color_map->color);
      fr->color_map->color = NULL;
    }
    if(fr->color_map->color == NULL){
      if(cv_allocate_color_map_color(fr->color_map) == KS_FALSE){
	return KS_FALSE;
      }
    }

    for(i = 0; fgets(read_buf,sizeof(read_buf),fp) != NULL; i++){
      if(strncmp(read_buf,"vertex_num",10) == 0 || strncmp(read_buf,"vertex_count",12) == 0){
	if(i < fr->color_map->num){
	  ks_error("Illegal file format %s (color_map data is too small)",file_name);
	  return KS_FALSE;
	}
	break;
      }
      if(i < fr->color_map->num){
	sscanf(read_buf,"%f %f %f %lf\n",
	       &fr->color_map->color[i][0],&fr->color_map->color[i][1],&fr->color_map->color[i][2],
	       &fr->color_map->value[i]);
      } else {
	ks_error("Illegal file format %s (color_map data is too large)\n%s",file_name,read_buf);
	return KS_FALSE;
      }
    }
  }

  if(strncmp(read_buf,"vertex_num",10) != 0 && strncmp(read_buf,"vertex_count",12) != 0){
    ks_error("Illegal file format %s (vertex_num is not found)",file_name);
    return KS_FALSE;
  }

  if(strncmp(read_buf,"vertex_num",10) == 0){
    sscanf(read_buf,"vertex_num %d\n",&vertex_num);
  } else if(strncmp(read_buf,"vertex_count",12) == 0){
    sscanf(read_buf,"vertex_count %d\n",&vertex_num);
  }
  /*  printf("%d\n",vertex_num);*/

  if(fr->sur->vertex != NULL){  // vertex is reused
    fr->sur->vertex_num = vertex_num;
    if(fr->sur->vertex_malloc_num < vertex_num){
      ks_free(fr->sur->vertex);
      ks_free(fr->sur->normal);
      ks_free(fr->sur->color);
      fr->sur->vertex = NULL;
    }
  }
  if(fr->sur->vertex == NULL){
    if(allocate_additional_surface_vertex(fr->sur,vertex_num) == KS_FALSE){
      return KS_FALSE;
    }
  }

  for(i = 0; fgets(read_buf,sizeof(read_buf),fp) != NULL; i++){
    if(strncmp(read_buf,"face_num",8) == 0 || strncmp(read_buf,"face_count",10) == 0){
      if(i < fr->sur->vertex_num){
	ks_error("Illegal file format %s (vertex data is too small)",file_name);
	return KS_FALSE;
      }
      break;
    }
    if(i < fr->sur->vertex_num){
      sscanf(read_buf,"%f %f %f %f %f %f %f %f %f\n",
	     &fr->sur->vertex[i*3],&fr->sur->vertex[i*3+1],&fr->sur->vertex[i*3+2],
	     &fr->sur->normal[i*3],&fr->sur->normal[i*3+1],&fr->sur->normal[i*3+2],
	     &fr->sur->color[i*4],&fr->sur->color[i*4+1],&fr->sur->color[i*4+2]);
      /*
	printf("%f %f %f  % f % f % f %f %f %f\n",
	fr->sur->vertex[i*3],fr->sur->vertex[i*3+1],fr->sur->vertex[i*3+2],
	fr->sur->normal[i*3],fr->sur->normal[i*3+1],fr->sur->normal[i*3+2],
	fr->sur->color[i*4],fr->sur->color[i*4+1],fr->sur->color[i*4+2]);
      */
    } else {
      ks_error("Illegal file format %s (vertex data is too large)\n%s",file_name,read_buf);
      return KS_FALSE;
    }
  }
  /*
  for(i = 0; i < fr->sur->vertex_num; i++){
    fscanf(fp,"%f %f %f %f %f %f %f %f %f\n",
	   &fr->sur->vertex[i*3],&fr->sur->vertex[i*3+1],&fr->sur->vertex[i*3+2],
	   &fr->sur->normal[i*3],&fr->sur->normal[i*3+1],&fr->sur->normal[i*3+2],
	   &fr->sur->color[i*4],&fr->sur->color[i*4+1],&fr->sur->color[i*4+2]);
  }
  */
  /*
  printf("read\n");
  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    printf("'%s'",read_buf);
  }
  */
  if(strncmp(read_buf,"face_num",8) == 0){
    sscanf(read_buf,"face_num %d",&face_num);
  } else if(strncmp(read_buf,"face_count",10) == 0){
    sscanf(read_buf,"face_count %d",&face_num);
  } else {
    ks_error("Illegal file format %s (face_num is not found)",file_name);
    return KS_FALSE;
  }
  /*  printf("%d\n",fr->sur->face_num);*/

  if(fr->sur->face != NULL){ // face is reused
    fr->sur->face_num = face_num;
    if(fr->sur->face_malloc_num < face_num){
      ks_free(fr->sur->face);
      fr->sur->face = NULL;
    }
  }
  if(fr->sur->face == NULL){
    if(allocate_additional_surface_face(fr->sur,face_num) == KS_FALSE){
      return KS_FALSE;
    }
  }

  for(i = 0; fgets(read_buf,sizeof(read_buf),fp) != NULL; i++){
    if(i < fr->sur->face_num){
      sscanf(read_buf,"%d %d %d\n",
	     &fr->sur->face[i*3],&fr->sur->face[i*3+1],&fr->sur->face[i*3+2]);
    } else {
      ks_error("Illegal file format %s (face data is too large)\n%s",file_name,read_buf);
      return KS_FALSE;
    }
    /*    printf("%d %d %d\n",fr->sur->face[i*3],fr->sur->face[i*3+1],fr->sur->face[i*3+2]);*/
    if(fr->sur->face[i*3] >= fr->sur->vertex_num ||
       fr->sur->face[i*3+1] >= fr->sur->vertex_num ||
       fr->sur->face[i*3+2] >= fr->sur->vertex_num){
      ks_error("Illegal file format %s (number of vertex is larger than vertex count)\n%s",
	       file_name,read_buf);
      return KS_FALSE;
    }
  }
  if(i < fr->sur->face_num){
    ks_error("Illegal file format %s (face data is too small)",file_name);
  }
  
  fclose(fp);
  return KS_TRUE;
}
BOOL find_additional_surface_key(CV_FRAME *fr, char *read_buf, CV_FIXED_SURFACE_INFO *p_sur_fixed)
{
  int i, hit = -1;
  char *c = NULL;
  char *keys[4] = {"ADDITIONAL_SURFACE_FIXED",  // 0 fixed
		   "additional_surface_fixed",  // 1 fixed
		   "ADDITIONAL_SURFACE",        // 2 not fixed
		   "additional_surface"};       // 3 not fixed
  char file_name[64];

  for(i = 0; i < 4; i++){
    if((c = strstr(read_buf,keys[i])) != NULL){
      hit = i;
      break;
    }
  }
  if(c != NULL){
    c += strlen(keys[hit])+1;
    for(i = 0; c[i] != '\n' && c[i] != ' '; i++){
      file_name[i] = c[i];
    }
    file_name[i] = '\0';
    if(p_sur_fixed->sur != NULL){    // fixed surface is set
      fr->sur = p_sur_fixed->sur;
      fr->color_map = p_sur_fixed->color_map;
    } else {
      printf("read additional surface %s\n",file_name);
      if(read_additional_surface(fr,file_name) == KS_FALSE){
	return KS_FALSE;
      }
      if(hit == 0 || hit == 1){  // keyword is for fixed surface
	p_sur_fixed->sur = fr->sur;
	p_sur_fixed->color_map = fr->color_map;
      }
    }
  } else {
    if(p_sur_fixed->sur != NULL){    // fixed surface is set
      fr->sur = p_sur_fixed->sur;
      fr->color_map = p_sur_fixed->color_map;
    }
  }

  return KS_TRUE;
}
static BOOL read_pdb_header(CV_FRAME *fr, CV_BOND_TYPE **bond_type, CV_FACE_TYPE **face_type,
			    CV_FIXED_SURFACE_INFO *p_sur_fixed)
{
  int ix,iy,iz;
  FILE *fp;
  char read_buf[256];
  double d0;
  int side_flg = 0;
  char comment_key[] = {"REMARK"};
  int comment_key_len;
  comment_key_len = strlen(comment_key);

  if((fp = fopen(fr->file_name,"rt")) == NULL){
    ks_error_file(fr->file_name);
    return KS_FALSE;
  }

  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    if(strncmp(read_buf,comment_key,6) == 0){
      /*      printf("%s",read_buf);*/
      if(ks_strval(read_buf,"side[0]=",&d0) == KS_TRUE){
	/*	fr->side[KS_RANGE_MAX][0] = d0;*/
	fr->side[1][1][1][0] = d0;
	side_flg |= 1;
      }
      if(ks_strval(read_buf,"side[1]=",&d0) == KS_TRUE){
	/*	fr->side[KS_RANGE_MAX][1] = d0;*/
	fr->side[1][1][1][1] = d0;
	side_flg |= 2;
      } 
      if(ks_strval(read_buf,"side[2]=",&d0) == KS_TRUE){
	/*	fr->side[KS_RANGE_MAX][2] = d0;*/
	fr->side[1][1][1][2] = d0;
	side_flg |= 4;
      } 
      if(ks_strval(read_buf,"time=",&d0) == KS_TRUE){
	fr->time= d0;
	fr->have_time = KS_TRUE;
      }
      if(strstr(read_buf,KS_PDB_EXTENSION_IDENTIFIER) != NULL){
	fr->pdb_ext = KS_TRUE;
      }
      if(find_additional_surface_key(fr,read_buf,p_sur_fixed) == KS_FALSE){
	return KS_FALSE;
      }
      if(strncmp(&read_buf[comment_key_len+1],CV_FILE_KEY_BOND,CV_FILE_KEY_LEN_BOND) == 0){
	set_cdv_bond(&read_buf[comment_key_len+1+CV_FILE_KEY_LEN_BOND],&fr->cdv_bond,
		     &fr->reuse->cdv_bond,CV_CDV_BOND_LIST_PDB_USER);
      } else if(strncmp(&read_buf[comment_key_len+1],CV_FILE_KEY_FACE,CV_FILE_KEY_LEN_FACE) == 0){
	set_cdv_face(&read_buf[comment_key_len+1+CV_FILE_KEY_LEN_FACE],&fr->cdv_face,
		     &fr->reuse->cdv_face);
      }
      if(read_cdv_bond_info(read_buf,bond_type,&fr->bond_file_name) == KS_FALSE){
	return KS_FALSE;
      }
      if(read_cdv_face_info(read_buf,face_type,&fr->face_file_name) == KS_FALSE){
	return KS_FALSE;
      }
    } else if(strncmp(read_buf,"CRYST1",6) == 0){
      int i;
      char c0[256];
      double d0,d1;
      double uv[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
      double len[3],ang[3],vec[3][3];
      sscanf(read_buf,"%s %lf %lf %lf %lf %lf %lf",c0
	     ,&len[0],&len[1],&len[2]
	     ,&ang[0],&ang[1],&ang[2]);
      for(i = 0; i < 3; i++){
	uv[i][0] *= len[0];
	uv[i][1] *= len[1];
	uv[i][2] *= len[2];
	ang[i] = ang[i]/180.*M_PI;
      }
      d0 = (cos(ang[0])-cos(ang[1])*cos(ang[2]))/sin(ang[2]);
      d1 = sqrt(pow(sin(ang[1]),2)-d0*d0);
      for(i = 0; i < 3; i++){
	vec[i][0] = uv[i][0]+uv[i][1]*cos(ang[2])+uv[i][2]*cos(ang[1]);
	vec[i][1] = uv[i][1]*sin(ang[2]) + uv[i][2]*d0;
	vec[i][2] = uv[i][2]*d1;
      }
      for(ix = 0; ix < 2; ix++){
	for(iy = 0; iy < 2; iy++){
	  for(iz = 0; iz < 2; iz++){
	    if(ix == 1){
	      for(i = 0; i < 3; i++){
		fr->side[ix][iy][iz][i] += vec[0][i];
	      }
	    }
	    if(iy == 1){
	      for(i = 0; i < 3; i++){
		fr->side[ix][iy][iz][i] += vec[1][i];
	      }
	    }
	    if(iz == 1){
	      for(i = 0; i < 3; i++){
		fr->side[ix][iy][iz][i] += vec[2][i];
	      }
	    }
	  }
	}
      }
      fr->have_side = KS_TRUE;
    }
  }
  if(side_flg == 7){
    fr->have_side = KS_TRUE;
    for(ix = 0; ix < 2; ix++){
      for(iy = 0; iy < 2; iy++){
	for(iz = 0; iz < 2; iz++){
	  if(!((ix == 0 && iy == 0 && iz == 0) || (ix == 1 && iy == 1 && iz == 1))){
	    fr->side[ix][iy][iz][0] = fr->side[ix][ix][ix][0];
	    fr->side[ix][iy][iz][1] = fr->side[iy][iy][iy][1];
	    fr->side[ix][iy][iz][2] = fr->side[iz][iz][iz][2];
	  }
	}
      }
    }
  }
  /*
  if(fr->have_time == KS_TRUE){
    printf("time = %e\n",fr->time);
  }
  if(fr->have_side == KS_TRUE){
    printf("side (%f %f %f)\n",fr->side[0],fr->side[1],fr->side[2]);
  }
  */

  fclose(fp);
  return KS_TRUE;
}
static unsigned int pdb_name_to_particle_flags(char *name)
{
  unsigned int flags = 0;
  if(strcmp(name,"N   ") == 0){
    flags |= CV_PARTICLE_MAIN;
  } else if(strcmp(name,"CA  ") == 0){
    flags |= CV_PARTICLE_MAIN|CV_PARTICLE_PRIME;
  } else if(strcmp(name,"C   ") == 0){
    flags |= CV_PARTICLE_MAIN;
  }
  return flags;
}
static BOOL read_pdb_frame(CV_FRAME *fr, CV_BASE *cb)
{
  int i,j,k;
  double offset[3] = {0,0,0};
  CV_ATOM *hit_atom;

  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  CV_PARTICLE_PROPERTY *pro;
  CV_PARTICLE *new_pl, *alternative = NULL;
  double alt_cd[3];
  int alt_num;
  int particle_num;
  KS_PDB_BUF **pdb = &cb->pdb_buf;
  /*  CV_PARTICLE_PROPERTY *ser_pro = NULL;*/

  printf("read pdb %s\n",fr->file_name);
  fflush(stdout);

  fr->flags |= CV_FRAME_READ;

  if(read_pdb_header(fr,&cb->bond_type,&cb->face_type,&cb->sur_fixed) == KS_FALSE){
    return KS_FALSE;
  }
  if(read_bond_and_face_file(fr,CV_CDV_BOND_LIST_PDB_USER) == KS_FALSE){
    return KS_FALSE;
  }

  if(ks_set_pdb_buf(fr->file_name,offset,NULL,NULL,pdb,KS_TRUE,KS_TRUE,KS_FALSE) == KS_FALSE){
    return KS_FALSE;
  }
  /*
  if(ks_set_pdb_buf(fr->file_name,offset,NULL,pdb,"HETATM") == KS_FALSE){
    return KS_FALSE;
  }
  */
  /*
  printf("%d %d\n",(*pdb)->num,(*pdb)->size);
  ks_exit(EXIT_FAILURE);
  */
  /*
  for(i = 0; i < (*pdb)->num; i++){
    ks_fprint_pdb_atom(stdout,*(*pdb)->atom[i]);
  }
  ks_exit(EXIT_FAILURE);
  */

  ks_analysis_pdb_buf((*pdb),CV_AGGREGATE_REQUIREMENT,KS_FALSE);

  /*
  printf("set_cv_aggregate_sequence %p %d %d %d\n",fr,
	 fr->aggregate_label_max,
	 fr->chain_label_max,
	 fr->residue_label_max);
  */

  if((fr->aggregate = set_cv_aggregate_sequence(&fr->aggregate_label_max,&fr->chain_label_max,
						&fr->residue_label_max,(*pdb),
						fr->reuse))==NULL){
    /*    return KS_FALSE;*/
  }

  /*
  printf("set_cv_aggregate_sequence end\n");
  */
  /*
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    printf("agg %d\n",pa->label);
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      printf("  chain %d %s %d %p\n",pc->label,pc->name,pc->len,pc);
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	printf("    residue %d %d %s %d %d\n"
	       ,pr->label,pr->number,pr->name,pr->pdb_residue_head,pr->particle_num);
      }
    }
  }
  ks_exit(EXIT_FAILURE);
  */
  /*
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    printf("agg %d\n",pa->label);
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      printf("  chain %d %s %d %p\n",pc->label,pc->name,pc->len,pc);
    }
  }
  */
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    /*    printf("agg %d\n",pa->label);*/
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      /*      printf("  chain %d %s %d %p\n",pc->label,pc->name,pc->len,pc);*/
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	/*
	printf("    residue %d %s %d %d\n"
	       ,pr->label,pr->name,pr->pdb_residue_head,pr->particle_num);
	*/
	particle_num = 0;
	for(i = pr->pdb_residue_head; i < pr->pdb_residue_head+pr->particle_num; i++){
	  if((*pdb)->atom[i]->altloc != ' '){
	    for(j = i+1; j < pr->pdb_residue_head+pr->particle_num && 
		  (*pdb)->atom[i]->altloc <  (*pdb)->atom[j]->altloc; j++){
	    }
	    i = j-1;
	  }
	  particle_num++;
	}
	/*	  printf("%d\n",particle_num);*/
	if(pr->particle == NULL){
	  if((pr->particle=(CV_PARTICLE**)ks_malloc(particle_num*sizeof(CV_PARTICLE*),
						    "pr->particle")) == NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }
	} else if(particle_num != pr->particle_num0){
	  /*	  printf("reuse %d  != %d\n",pr->particle_num,particle_num);*/
	  if((pr->particle=(CV_PARTICLE**)ks_realloc(pr->particle,
						     particle_num*sizeof(CV_PARTICLE*),
						    "pr->particle")) == NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }
	} else {
	  /*	  printf("reuse %d  == %d\n",pr->particle_num,particle_num);*/
	}
	/*	printf("%p\n",pr->particle);*/
	particle_num = 0;
	for(i = pr->pdb_residue_head; i < pr->pdb_residue_head+pr->particle_num; i++){
	  if(fr->particle_file_label_max < (*pdb)->atom[i]->label) 
	    fr->particle_file_label_max = (*pdb)->atom[i]->label;
	  /*	    ks_fprint_pdb_atom(stdout,*(*pdb)->atom[i]);*/
	  /*	  printf("%d %s %d\n",i,(*pdb)->atom[i]->name,(*pdb)->atom[i]->flags);*/
	  hit_atom = make_cv_atom(&cb->atom,(*pdb)->atom[i]->name,cb->atype_use);

	  if((*pdb)->atom[i]->altloc != ' '){
	    alt_num = 0;
	    for(k = 0; k < 3; k++)
	      alt_cd[k] = (*pdb)->atom[i]->cd[k];
	    alternative = add_cv_particle(alternative,
					  new_cv_particle(alt_num++,(*pdb)->atom[i]->label,0,
							  hit_atom,NULL,
							  (*pdb)->atom[i]->cd[0],
							  (*pdb)->atom[i]->cd[1],
							  (*pdb)->atom[i]->cd[2],
							  &fr->reuse->particle));
	    for(j = i+1; j < pr->pdb_residue_head+pr->particle_num && 
		  (*pdb)->atom[i]->altloc <  (*pdb)->atom[j]->altloc; j++){
	      for(k = 0; k < 3; k++)
		alt_cd[k] += (*pdb)->atom[j]->cd[k];
	      alternative = add_cv_particle(alternative,
					    new_cv_particle(alt_num++,(*pdb)->atom[j]->label,0,
							    hit_atom,NULL,
							    (*pdb)->atom[j]->cd[0],
							    (*pdb)->atom[j]->cd[1],
							    (*pdb)->atom[j]->cd[2],
							    &fr->reuse->particle));
	      /*
		printf("alt %d %d %s %s %d %f %f %f\n",j
		,(*pdb)->atom[j]->label
		,(*pdb)->atom[j]->name
		,(*pdb)->atom[j]->residue
		,(*pdb)->atom[j]->residue_label
		,(*pdb)->atom[j]->cd[0],(*pdb)->atom[j]->cd[1],(*pdb)->atom[j]->cd[2]);
	      */
	    }
	    for(k = 0; k < 3; k++)
	      alt_cd[k] /= alt_num;
	    /*
	      printf("alt_cd %f %f %f %d\n",alt_cd[0],alt_cd[1],alt_cd[2],alt_num);
	    */
	    if((pro=new_cv_particle_property((*pdb)->atom[i]->name,pa,pc,pr,alternative,
					     &fr->reuse->property)) == NULL){
	      return KS_FALSE;
	    }
	    alternative = NULL;
	    new_pl=new_cv_particle(fr->particle_label_max++,(*pdb)->atom[i]->label,
				   pdb_name_to_particle_flags((*pdb)->atom[i]->name)|
				   ((*pdb)->atom[i]->flags&KS_PDB_ATOM_HETATM?
				    CV_PARTICLE_HETATM:0),
				   hit_atom,pro,
				   alt_cd[0],alt_cd[1],alt_cd[2],&fr->reuse->particle);
	    i = j-1;
	  } else {
	    if((pro=new_cv_particle_property((*pdb)->atom[i]->name,pa,pc,pr,NULL,
					     &fr->reuse->property)) == NULL){
	      return KS_FALSE;
	    }
	    new_pl=new_cv_particle(fr->particle_label_max++,(*pdb)->atom[i]->label,
				   pdb_name_to_particle_flags((*pdb)->atom[i]->name)|
				   ((*pdb)->atom[i]->flags&KS_PDB_ATOM_HETATM?
				    CV_PARTICLE_HETATM:0),
				   hit_atom,pro,
				   (*pdb)->atom[i]->cd[0],
				   (*pdb)->atom[i]->cd[1],
				   (*pdb)->atom[i]->cd[2],
				   &fr->reuse->particle);

	    /*
	    if((*pdb)->atom[i]->flags&KS_PDB_ATOM_HETATM){
	      printf("%d %x %s\n",fr->particle_label_max,new_pl->flags,pro->name);
	      printf("%x %x %d\n"
		     ,pdb_name_to_particle_flags((*pdb)->atom[i]->name)|
		     ((*pdb)->atom[i]->flags&KS_PDB_ATOM_HETATM?CV_PARTICLE_HETATM:0)
		     ,pdb_name_to_particle_flags((*pdb)->atom[i]->name),
		     (*pdb)->atom[i]->flags&KS_PDB_ATOM_HETATM?CV_PARTICLE_HETATM:0);
	    }
	    */
	  }
	  /*
	    printf("%d %s %p\n",new_pl->file_label,new_pl->atom->name,new_pl->property);
	  */
	  /*
	    printf("%d %d %p %p\n",fi,fr->particle_label_max,fr->particle,new_pl);
	  */

	  if(fr->pdb_ext == KS_TRUE && new_pl->property->residue->particle_num > 1){
	    if((*pdb)->atom[i]->flags&KS_PDB_ATOM_EXT_MAIN){
	      new_pl->flags |= CV_PARTICLE_MAIN;
	      /*	      new_pl->property->residue->flags |= CV_RESIDUE_AMINO;*/
	    }
	    if((*pdb)->atom[i]->flags&KS_PDB_ATOM_EXT_PRIME){
	      new_pl->flags |= CV_PARTICLE_PRIME;
	      /*	      new_pl->property->chain->flags |= CV_CHAIN_AMINO;*/
	    }
	  }
	  if(fr->pdb_ext == KS_TRUE){
	    if((*pdb)->atom[i]->flags&KS_PDB_ATOM_EXT_SOLVENT){
	      new_pl->flags |= CV_PARTICLE_SOLVENT; // set solvent flag
	    }
	  }

	  fr->particle = add_cv_particle(fr->particle,new_pl);
	  /*
	  if(pr->file_label == 870 && particle_num == 0){
	    ser_pro = new_pl->property;
	  }
	  */
	  /*
	  printf("%d %s %d %s %p %s\n"
		 ,pr->file_label,pr->name,particle_num,new_pl->atom->name
		 ,new_pl->property->name,new_pl->property->name);

	  if(ser_pro != NULL)
	    printf("%s\n",ser_pro->name);
	  */
	  ks_assert(particle_num < pr->particle_num);
	  pr->particle[particle_num++] = new_pl;
	}

	pr->particle_num = particle_num;
	if(pr->next == NULL)
	  pr->particle[pr->particle_num-1]->flags |= CV_PARTICLE_CHAIN_END;
      }
    }
  }
  if(cb->no_water == KS_FALSE){
    if(set_water_coordinate(&fr->particle_label_max,*pdb,&cb->atom,&fr->particle,cb->atype_use,
			    fr->reuse)
       == KS_FALSE){
      return KS_FALSE;
    }
  }
  /*
  {
    CV_PARTICLE *p;
    for(p = fr->particle; p != NULL; p = p->next)
      printf("%d '%s' '%s' %s main %c prime %c HETATM %c %x %f %f %f\n"
	     ,p->label,p->atom->name,p->property->name
	     ,p->property != NULL ? p->property->residue->name:"X"
	     ,p->flags&KS_PDB_ATOM_EXT_MAIN ? 'o':'x'
	     ,p->flags&KS_PDB_ATOM_EXT_PRIME ? 'o':'x'
	     ,p->flags&CV_PARTICLE_HETATM ? 'o':'x'
	     ,p->flags
	     ,p->cd[0],p->cd[1],p->cd[2]);
  }
  ks_exit(EXIT_FAILURE);
  */
  /*
    {
    CV_ATOM *p;
    for(p = fr->atom; p != NULL; p = p->next){
    printf("%d %s\n",p->label,p->name);
    }
    }
    ks_exit(EXIT_FAILURE);
  */
  /*    printf("%d\n",fr->particle_file_label_max);*/
  fr->particle_file_label_max++;
  ks_clear_pdb_buf(*pdb);

  return KS_TRUE;
}
static BOOL read_prep_frame(CV_FRAME *fr, CV_ATOM **atom, BOOL *atype_use)
{
  int i,j;
  double icd[3],**cd = NULL;
  unsigned int residue_label = 0;
  KS_AMBER_PREP_BLOCK *pb;
  CV_ATOM *hit_atom;
  CV_PARTICLE_PROPERTY *pro;
  CV_PARTICLE *new_pl;

  CV_CHAIN *chain = NULL;
  CV_RESIDUE *res = NULL;

  fr->flags |= CV_FRAME_READ;

  {
    pb = fr->prep_block;
    /*
    printf("  name: '%s'\n",pb->name);
    printf("  type: '%s'\n",pb->type);
    */
    res = new_cv_residue(residue_label++,0,0,pb->type,0,&fr->reuse->residue);
    res->particle_num = pb->atom_num;
    if((res->particle = (CV_PARTICLE**)ks_malloc(pb->atom_num*sizeof(CV_PARTICLE*),"atom"))==NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
    chain = new_cv_chain(0,0,"CHAIN",1,res,&fr->reuse->chain);
    fr->aggregate = add_cv_aggregate(fr->aggregate,
				     new_cv_aggregate(0,0,chain,&fr->reuse->aggregate));
    if(cd == NULL){
      if((cd = ks_malloc_double_pp(pb->atom_num,3,"cd")) == NULL){
	return KS_FALSE;
      }
    }
    for(i = 0; i < pb->atom_num; i++){
      if(i != 0 && 
	 pb->atom[i].label == 3 &&
	 pb->atom[i].link_label[0] == 2 && 
	 pb->atom[i].link_label[1] == 1 && 
	 pb->atom[i].link_label[2] == 0 &&
	 pb->atom[i].bond == pb->atom[i-1].bond &&
	 pb->atom[i].angle == pb->atom[i-1].angle &&
	 pb->atom[i].torsion == pb->atom[i-1].torsion &&
	 pb->atom[i].angle == 0 &&
	 pb->atom[i].torsion == 0){
	printf("----------------\n");
	printf("name: '%s'\n",pb->name);
	printf("type: '%s'\n",pb->type);
	printf("Warning: angle of internal coordination changed from %f to 90.0\n",
	       pb->atom[i].angle);
	printf("----------------\n");
	pb->atom[i].angle = 90;
      }
      /*
      printf("    %3d %-4s %-2s %c % 4d % 4d % 4d  %5.3f  %6.2f  % 7.2f  % 7.4f\n"
	     ,pb->atom[i].label
	     ,pb->atom[i].name
	     ,pb->atom[i].type
	     ,pb->atom[i].topological_type
	     ,pb->atom[i].link_label[0],pb->atom[i].link_label[1],pb->atom[i].link_label[2]
	     ,pb->atom[i].bond
	     ,pb->atom[i].angle
	     ,pb->atom[i].torsion
	     ,pb->atom[i].charge);
      */
      if(pb->flags&KS_AMBER_PREP_BLOCK_XYZ){
	for(j = 0; j < 3; j++){
	  cd[i][j] = pb->atom[i].cd[j];
	}
      } else {
	icd[0] = pb->atom[i].bond;
	icd[1] = pb->atom[i].angle;
	icd[2] = pb->atom[i].torsion;
	if(i < 3){
	  if(i == 0){
	    cd[0][0] = 0; cd[0][1] = 0; cd[0][2] = 0;
	  } else if(i == 1){
	    calc_internal_cd(cd[0],NULL,NULL,icd,cd[1]);
	  } else if(i == 2){
	    calc_internal_cd(cd[1],cd[0],NULL,icd,cd[2]);
	  }
	} else {
	  calc_internal_cd(cd[pb->atom[i].link_label[0]-1],
			   cd[pb->atom[i].link_label[1]-1],
			   cd[pb->atom[i].link_label[2]-1],icd,cd[i]);
	}
      }
      if(1){
	hit_atom = make_cv_atom(atom,pb->atom[i].type,atype_use);
	/*	printf("%d %s %p %p %p\n",i,pb->atom[i].name,fr->aggregate,chain,res);*/
	if((pro=new_cv_particle_property(pb->atom[i].name,fr->aggregate,chain,res,NULL,
					 &fr->reuse->property)) == NULL){
	  return KS_FALSE;
	}
	new_pl=new_cv_particle(fr->particle_label_max++,i,
			       pdb_name_to_particle_flags(pb->atom[i].name)|
			       0,hit_atom,pro,cd[i][0],cd[i][1],cd[i][2],
			       &fr->reuse->particle);
	new_pl->quaternion[0] = pb->atom[i].charge;  // use quaternion array for charge
	if(pb->atom[i].topological_type == 'M'){
	  new_pl->flags |= CV_PARTICLE_MAIN;
	}
	res->particle[i] = new_pl;
	fr->particle = add_cv_particle(fr->particle,new_pl);
      }
    }
    if(pb->flags&KS_AMBER_PREP_BLOCK_INT){
      CV_PARTICLE *p0,*p1;
      double r,dd[3];
      for(i = 0; i < pb->atom_num; i++){
	/*	printf("%d\n",i);*/
	p0 = res->particle[i];
	if(pb->atom[i].link_label[0] > 0){
	  p1 = res->particle[pb->atom[i].link_label[0]-1];
	  for(j = 0; j < 3; j++){
	    dd[j] = p0->cd[j]-p1->cd[j];
	  }
	  r = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);
	  p0->bond = add_cv_bond(p0->bond, new_cv_bond(0,p1,r,&fr->reuse->bond));
	  p1->bond = add_cv_bond(p1->bond, new_cv_bond(0,p0,r,&fr->reuse->bond));
	}
      }
    } else if(pb->flags&KS_AMBER_PREP_BLOCK_XYZ){
      set_bond_all(fr->particle,&fr->reuse->bond);
    }
    ks_free_double_pp(pb->atom_num,cd);
  }
  /*
  {
    CV_PARTICLE *p;
    CV_BOND *bond;
    printf("bond\n");
    for(p = fr->particle; p != NULL; p = p->next){
      printf("%d %s\n",p->label,p->property->name);
      for(bond = p->bond; bond != NULL; bond = bond->next){
	printf("  %d %s %f\n",bond->p->label,bond->p->property->name,bond->len);
      }
    }
  }
  */
  if(pb->flags&KS_AMBER_PREP_BLOCK_INT){
    KS_INT_LIST_LIST *pii;
    CV_PARTICLE *p0,*p1;
    double r,dd[3];
    /*
    for(pii = pb->loop; pii != NULL; pii = pii->next){
      printf("  loop: ");
      for(pi = pii->p; pi != NULL; pi = pi->next){
	printf("%-4s ",pb->atom[pi->value].name);
       }
      for(pi = pii->p; pi != NULL; pi = pi->next){
	printf("% 3d ",pi->value);
      }
      printf("\n");
    }
    */
    for(pii = pb->loop; pii != NULL; pii = pii->next){
      p0 = res->particle[pii->p->value];
      p1 = res->particle[pii->p->next->value];
      for(j = 0; j < 3; j++){
	dd[j] = p0->cd[j]-p1->cd[j];
      }
      r = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);
      p0->bond = add_cv_bond(p0->bond, new_cv_bond(0,p1,r,&fr->reuse->bond));
      p1->bond = add_cv_bond(p1->bond, new_cv_bond(0,p0,r,&fr->reuse->bond));
    }
  }
  res->particle[res->particle_num-1]->flags |= CV_PARTICLE_CHAIN_END;

  /*
  {
    CV_ATOM* p;
    for(p = *atom; p != NULL; p = p->next){
      printf("%d %s\n",p->atype,p->name);
    }
  }
  {
    CV_PARTICLE *p;
    for(p = fr->particle; p != NULL; p = p->next)
      printf("%d '%s' '%s' %s main %c prime %c HETATM %c %x %f %f %f\n"
	     ,p->label,p->atom->name,p->property->name
	     ,p->property != NULL ? p->property->residue->name:"X"
	     ,p->flags&KS_PDB_ATOM_EXT_MAIN ? 'o':'x'
	     ,p->flags&KS_PDB_ATOM_EXT_PRIME ? 'o':'x'
	     ,p->flags&CV_PARTICLE_HETATM ? 'o':'x'
	     ,p->flags
	     ,p->cd[0],p->cd[1],p->cd[2]);
  }
  ks_exit(EXIT_FAILURE);
  */

  return KS_TRUE;
}
static BOOL is_comment(char *str)
{
  char *cp;
  for(cp = str; *cp == ' '; cp++);
  if(*cp == '\'' || *cp == '#'){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
BOOL get_key_parm(char *read_line, char *key, void *vp, int num)
{
  int i;
  char *cp,*cpa;
  char val[64];

GET_KEY_PARM_START:;
  /*  printf("get_key_parm %s %s",key,read_line);*/
  if((cp = strstr(read_line,key)) == NULL){
    return KS_FALSE;
  }
  if(cp != read_line){
    if(*(cp-1) != ' ' && *(cp-1) != '\'' && *(cp-1) != '#'){
      return KS_FALSE;
    }
  }
  cp += strlen(key);
  /*  printf("cp %s\n",cp);*/
  if((cp = ks_skip_chars(" \n",cp)) == NULL) return KS_FALSE;
  if(*cp == '='){
    cp++;
  } else {
    if((cpa = strstr(cp,key)) != NULL){ // check key again (because c1 also hit c12)
      read_line = cpa;
      goto GET_KEY_PARM_START;
    }
    return KS_FALSE;
  }
  if((cp = ks_skip_chars(" \n",cp)) == NULL) return KS_FALSE;
  if(num == 0){
    *(char**)vp = cp;
  } else {
    if(*cp == '('){
      cp++;
      /*    printf("%s",cp);*/
      for(i = 0; i < num; i++){
	cp = ks_get_str("),",cp,val,sizeof(val));
	*(float*)vp = atof(val);
	/*      printf("%d %s %f\n",i,val,*(float*)vp);*/
	vp = (float*)vp+1;
      }
    } else if(num > 1){
      return KS_FALSE;
    } else {
      for(i = 0; *cp != ' ' && *cp != '\n'; i++,cp++){
	/*      printf("%d %x %c\n",i,*cp,*cp);*/
	val[i] = *cp;
      }
      if(val[0] == '\0') return KS_FALSE;
      val[i] = '\0';
      /*    printf("'%s' %f '%s' %d\n",key,atof(val),val,i);*/
      *(float*)vp = atof(val);
    }
  }
  return KS_TRUE;
}
static BOOL set_atom_radius(int atype, float radius, BOOL scaled, float scale[3])
{
  if(atype < CV_ATOM_TYPE_MAX){
    init_cv_atom_parm[atype].radius = radius;
    if(scaled == KS_TRUE){
      int i;
      init_cv_atom_parm[atype].scaled = KS_TRUE;
      for(i = 0; i < 3; i++){
	init_cv_atom_parm[atype].scale[i] = scale[i];
      }
    }
    return KS_TRUE;
  } else {
    ks_error("atype %d is too large",atype);
    return KS_FALSE;
  }
}
static BOOL set_atom_color(int atype, float *color)
{
  /*
  CV_ATOM *hit_atom = NULL;
  char name;

  name = '\0';
  hit_atom = lookup_cv_atom_atype(*atom,atype);
  if(hit_atom == NULL){
    *atom = add_cv_atom(*atom,new_cv_atom(atype,&name,CV_DEFAULT_RADIUS,color,
					  CV_SPHERE_DETAIL_NORMAL,
					  NULL,1.0));
  } else {
    int i;
    for(i = 0; i < 3; i++)
      hit_atom->color[i] = color[i];
  }
  */
  if(atype < CV_ATOM_TYPE_MAX){
    init_cv_atom_parm[atype].color[0] = color[0];
    init_cv_atom_parm[atype].color[1] = color[1];
    init_cv_atom_parm[atype].color[2] = color[2];
    return KS_TRUE;
  } else {
    ks_error("atype %d is too large",atype);
    return KS_FALSE;
  }
}
static void read_cdv_side(char *read_line, double side[2][2][2][3], float *r, BOOL *have_side)
{
  int i,j;
  char se[2] = {'s','e'};
  char xyz[3] = {'x','y','z'};
  static unsigned int flg = 0;
  char key[64];
  double d0;

  sprintf(key,"box_sx=");
  for(i = 0; i < 2; i++){
    key[4] = se[i];
    for(j = 0; j < 3; j++){
      key[5] = xyz[j];
      /*      printf("%d %d %s %s\n",i,j,key,read_line);*/
      if(ks_strval(read_line,key,&d0) == KS_TRUE){
	/*	side[i][j] = d0;*/
	side[i][i][i][j] = d0;
	flg |= (unsigned int)(pow(8,i)*pow(2,j)+.5);
	/*	printf("%s %f %d\n",key,d0,flg);*/
      }
    }
  }
  if(flg == 63){
    int ix,iy,iz;
    *have_side = KS_TRUE;
    for(ix = 0; ix < 2; ix++){
      for(iy = 0; iy < 2; iy++){
	for(iz = 0; iz < 2; iz++){
	  if(!((ix == 0 && iy == 0 && iz == 0) || (ix == 1 && iy == 1 && iz == 1))){
	    side[ix][iy][iz][0] = side[ix][ix][ix][0];
	    side[ix][iy][iz][1] = side[iy][iy][iy][1];
	    side[ix][iy][iz][2] = side[iz][iz][iz][2];
	  }
	}
      }
    }
  }
  if(ks_strval(read_line,"box_wt=",&d0) == KS_TRUE){
    *r = d0;
  }
}
static BOOL read_cdv_atom(char *read_line)
{
  int i;
  char key[64];
  float val;
  float vals[3];
  BOOL hit = KS_FALSE;
  for(i = 0; i < CV_ATOM_TYPE_MAX; i++){
    hit = KS_FALSE;
    sprintf(key,"r%d",i);
    if(get_key_parm(read_line,key,vals,3) == KS_TRUE){
      /*      printf("%s %f %f %f\n",key,vals[0],vals[1],vals[2]);*/
      if(set_atom_radius(i,1.0,KS_TRUE,vals) == KS_FALSE){
	return KS_FALSE;
      }
      hit = KS_TRUE;
    } else if(get_key_parm(read_line,key,&val,1) == KS_TRUE){
      /*      printf("%s %f\n",key,val);*/
      if(set_atom_radius(i,val,KS_FALSE,NULL) == KS_FALSE){
	return KS_FALSE;
      }
      hit = KS_TRUE;
    }
    sprintf(key,"c%d",i);
    if(get_key_parm(read_line,key,vals,3) == KS_TRUE){
      /*      printf("%s %f %f %f\n",key,vals[0],vals[1],vals[2]);*/
      if(set_atom_color(i,vals) == KS_FALSE){
	return KS_FALSE;
      }
      hit = KS_TRUE;
    }
    if(hit == KS_TRUE){
      init_cv_atom_parm[i].show = KS_TRUE;
    }
  }
  return KS_TRUE;
}
static void read_cdv_light_pos(char *read_line, GLfloat *light_pos)
{
  float vals[3];
  if(get_key_parm(read_line,"light_position",vals,3) == KS_TRUE){
    light_pos[2] = vals[0]; /* x */
    light_pos[0] = vals[1]; /* y */
    light_pos[1] = vals[2]; /* z */
  }
}
static void read_cdv_box_frame_color(char *read_line, 
				     GLfloat box_frame_color[CV_BOX_FRAME_COLOR_NUM][4])
{
  int i;
  float vals[3];
  if(get_key_parm(read_line,"box_c",vals,3) == KS_TRUE){
    for(i = 0; i < 3; i++){
      box_frame_color[CV_BOX_FRAME_COLOR_1][i] = vals[i];
      box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][i] = vals[i];
    }
  }
  if(get_key_parm(read_line,"box_c1",vals,3) == KS_TRUE){
    for(i = 0; i < 3; i++){
      box_frame_color[CV_BOX_FRAME_COLOR_1][i] = vals[i];
      box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][i] = vals[i];
    }
  }
  if(get_key_parm(read_line,"box_c2",vals,3) == KS_TRUE){
    for(i = 0; i < 3; i++){
      box_frame_color[CV_BOX_FRAME_COLOR_2][i] = vals[i];
    }
  }
}
static void read_cdv_c_color(char *read_line, CV_CONTINUOUS_COLOR* c_color, 
			     CV_CONTINUOUS_COLOR_INFO *c_info)
{
  int i;
  char key[] = {"c_color="};
  char *cp;
  float vals[3];
  if((cp = strstr(read_line,key)) != NULL){
    cp += sizeof(key)/sizeof(char)-1;
    c_color->num = atoi(cp);
  }
  if(get_key_parm(read_line,"c_color_start",vals,3) == KS_TRUE){
    if(c_info->start_color == NULL){
      if((c_info->start_color = (GLfloat*)ks_malloc(3*sizeof(GLfloat),"start_color")) == NULL){
	ks_error_memory();
      }
    }
    for(i = 0; i < 3; i++){
      c_info->start_color[i] = vals[i];
    }
  }
  if(get_key_parm(read_line,"c_color_end",vals,3) == KS_TRUE){
    if(c_info->end_color == NULL){
      if((c_info->end_color = (GLfloat*)ks_malloc(3*sizeof(GLfloat),"end_color")) == NULL){
	ks_error_memory();
      }
    }
    for(i = 0; i < 3; i++){
      c_info->end_color[i] = vals[i];
    }
  }
}
static void read_cdv_periodic_inside_color(char *read_line, GLfloat periodic_inside_color[4])
{
  float vals[3];
  if(get_key_parm(read_line,"inside_c",vals,3) == KS_TRUE){
    periodic_inside_color[0] = vals[0];
    periodic_inside_color[1] = vals[1];
    periodic_inside_color[2] = vals[2];
  }
}
static BOOL read_cdv_bond_info(char *read_line, CV_BOND_TYPE **bond_type, char **file_name)
{
  int i,j;
  char key[64];
  char *cp;
  float val;
  float vals[3];
  CV_BOND_TYPE *hit;

  if(*file_name == NULL && get_key_parm(read_line,"bond_file",&cp,0) == KS_TRUE){
    int len;
    for(len = 0; *(cp+len) != ' ' && *(cp+len) != '\n'; len++);
    /*    printf("%d %s\n",len,cp);*/
    len++;
    if((*file_name = ks_malloc_char_p(len,"file_name")) == KS_FALSE){
      return KS_FALSE;
    }
    for(i = 0; i < len-1; i++){
      (*file_name)[i] = *(cp+i);
      /*      printf("%d %x\n",i,*(cp+i));*/
    }
    (*file_name)[len-1] = '\0';
    /*    printf("%s\n",*file_name);*/
  }
  if(get_key_parm(read_line,"bond_wt",&val,1) == KS_TRUE){
    /*    printf("bond_wt %f\n",val);*/
    hit = lookup_cv_bond_type(*bond_type,0);
    if(hit == NULL){
      hit = new_cv_bond_type(0,init_cv_bond_type.color,val);
      *bond_type = add_cv_bond_type(*bond_type,hit);
    } else {
      hit->radius = val;
    }
  }
  if(get_key_parm(read_line,"bond_c",vals,3) == KS_TRUE){
    /*    printf("bond_c %f %f %f\n",vals[0],vals[1],vals[2]);*/
    hit = lookup_cv_bond_type(*bond_type,0);
    if(hit == NULL){
      hit = new_cv_bond_type(0,vals,init_cv_bond_type.radius);
      *bond_type = add_cv_bond_type(*bond_type,hit);
    } else {
      for(j = 0; j < 3; j++){
	hit->color[j] = vals[j];
      }
    }
  }

  for(i = 0; i < CV_BOND_TYPE_MAX; i++){
    sprintf(key,"bond%d_wt",i);
    /*    printf("%s\n",key);*/
    if(get_key_parm(read_line,key,&val,1) == KS_TRUE){
      /*      printf("%d %s %f\n",i,key,val);*/
      hit = lookup_cv_bond_type(*bond_type,i);
      if(hit == NULL){
	hit = new_cv_bond_type(i,init_cv_bond_type.color,val);
	*bond_type = add_cv_bond_type(*bond_type,hit);
      } else {
	hit->radius = val;
      }
    }
    sprintf(key,"bond%d_c",i);
    if(get_key_parm(read_line,key,vals,3) == KS_TRUE){
      /*      printf("%d %s %f %f %f\n",i,key,vals[0],vals[1],vals[2]);*/
      hit = lookup_cv_bond_type(*bond_type,i);
      if(hit == NULL){
	hit = new_cv_bond_type(i,vals,init_cv_bond_type.radius);
	*bond_type = add_cv_bond_type(*bond_type,hit);
      } else {
	for(j = 0; j < 3; j++){
	  hit->color[j] = vals[j];
	}
      }
    }
  }

  return KS_TRUE;
}
static BOOL read_cdv_face_info(char *read_line, CV_FACE_TYPE **face_type, char **file_name)
{
  int i,j;
  char key[64];
  char *cp;
  float vals[3];
  CV_FACE_TYPE *hit;

  if(*file_name == NULL && get_key_parm(read_line,"face_file",&cp,0) == KS_TRUE){
    int len;
    for(len = 0; *(cp+len) != ' '; len++);
    /*    printf("%d %s\n",len,cp);*/
    len++;
    if((*file_name = ks_malloc_char_p(len,"file_name")) == KS_FALSE){
      return KS_FALSE;
    }
    for(i = 0; i < len-1; i++){
      (*file_name)[i] = *(cp+i);
      /*      printf("%d %x\n",i,*(cp+i));*/
    }
    (*file_name)[len-1] = '\0';
    /*    printf("%s\n",*file_name);*/
  }
  if(get_key_parm(read_line,"face_c",vals,3) == KS_TRUE){
    /*    printf("face_c %f %f %f\n",vals[0],vals[1],vals[2]);*/
    hit = lookup_cv_face_type(*face_type,0);
    if(hit == NULL){
      hit = new_cv_face_type(0,vals);
      *face_type = add_cv_face_type(*face_type,hit);
    } else {
      for(j = 0; j < 3; j++){
	hit->color[j] = vals[j];
      }
    }
  }

  for(i = 0; i < CV_FACE_TYPE_MAX; i++){
    sprintf(key,"face%d_c",i);
    if(get_key_parm(read_line,key,vals,3) == KS_TRUE){
      /*      printf("%d %s %f %f %f\n",i,key,vals[0],vals[1],vals[2]);*/
      hit = lookup_cv_face_type(*face_type,i);
      if(hit == NULL){
	hit = new_cv_face_type(i,vals);
	*face_type = add_cv_face_type(*face_type,hit);
      } else {
	for(j = 0; j < 3; j++){
	  hit->color[j] = vals[j];
	}
      }
    }
  }

  return KS_TRUE;
}
static BOOL read_cdv_user_text(char *read_line, CV_USER_TEXT *string)
{
  int i;
  int len;
  char key[64];
  char *p;
  float pos[2];
  for(i = 0; i < CV_ATOM_TYPE_MAX; i++){
    sprintf(key,"st%d",i);
    if(get_key_parm(read_line,key,&p,0) == KS_TRUE){
      if(p[0] == '"'){
	for(len = 1; p[len] != '"' && p[len] != '\0'; len++);
	len--;
      } else {
	for(len = 0; p[len] != ' ' && p[len] != '\0'; len++);
      }
      /*
      printf("%d %s '%s' %d %p %p\n",i,key,p,len,string,string[i].text);
      if(string[i].text != NULL){
	printf("free %s\n",string[i].text);
	ks_free(string[i].text);
      }
      */
      if(string[i].text != NULL){
	if(strlen(string[i].text) < len){
	  ks_free(string[i].text);
	  string[i].text = NULL;
	}
      }
      if(string[i].text == NULL){
	if((string[i].text = ks_malloc_char_p(len+1,"string[i].text")) == NULL){
	  ks_error_memory();
	  return KS_FALSE;
	}
      }
      if(p[0] == '"'){
	for(len = 1; p[len] != '"' && p[len] != '\0'; len++){
	  string[i].text[len-1] = p[len];
	}
	len--;
      } else {
	for(len = 0; p[len] != ' ' && p[len] != '\0'; len++){
	  string[i].text[len] = p[len];
	}
      }
      string[i].text[len] = '\0';
    }
    sprintf(key,"st%d_pos",i);
    if(get_key_parm(read_line,key,pos,2) == KS_TRUE){
      /*      printf("%s %f %f\n",key,pos[0],pos[1]);*/
      string[i].x = pos[0];
      string[i].y = pos[1];
    }
  }
  return KS_TRUE;
}
static BOOL read_cdv_header(KS_FILE_LINES *fi, CV_FRAME *fr, CV_BASE *cb, BOOL header_file)
{
  char *read_line;
  long long int fl;
  for(fl = 0; fl < fi->line_cnt; fl++){
    read_line = fi->file_lines[fl];
    if(is_comment(read_line) == KS_TRUE){
      if(read_cdv_atom(read_line) == KS_FALSE){
	return KS_FALSE;
      }
      if(header_file == KS_FALSE){
	read_cdv_side(read_line,fr->side,&fr->side_radius,&fr->have_side);
	if(read_cdv_bond_info(read_line,&cb->bond_type,&fr->bond_file_name) == KS_FALSE){
	  return KS_FALSE;
	}
	if(read_cdv_face_info(read_line,&cb->face_type,&fr->face_file_name) == KS_FALSE){
	  return KS_FALSE;
	}
	if(read_cdv_user_text(read_line,fr->user_text) == KS_FALSE){
	  return KS_FALSE;
	}
      }
      if(strstr(read_line,CV_FILE_KEY_ENABLE_RMSD) != NULL){
	cb->enable_cdv_rmsd_rmsf = KS_TRUE;
      }
      if(strstr(read_line,CV_FILE_KEY_ENABLE_AXIS) != NULL){
	cb->axis_mode = CV_AXIS_MODE_ON;
      }
      read_cdv_light_pos(read_line,cb->light_position);
      read_cdv_box_frame_color(read_line,cb->box_frame_color);
      read_cdv_c_color(read_line,&cb->continuous_color,&fr->c_color_info);
      read_cdv_periodic_inside_color(read_line,cb->periodic_inside_color);
      if(find_additional_surface_key(fr,read_line,&cb->sur_fixed) == KS_FALSE){
	return KS_FALSE;
      }
    } else {
      break;
    }
  }
  /*
  {
    CV_ATOM *p;
    for(p = *atom; p != NULL; p = p->next){
      printf("%p %d '%s' %f (%f %f %f)\n",p,p->atype,p->name,p->radius
	     ,p->color[0],p->color[1],p->color[2]);
    }
  }
  */
  /*
  printf("(%f %f %f)(%f %f %f) %f %d\n",
	 fr->side[KS_RANGE_MIN][0],fr->side[KS_RANGE_MIN][1],fr->side[KS_RANGE_MIN][2],
	 fr->side[KS_RANGE_MAX][0],fr->side[KS_RANGE_MAX][1],fr->side[KS_RANGE_MAX][2],
	 fr->side_radius,fr->have_side);
  */
  /*
  {
    CV_BOND_TYPE *p;
    if(fr->bond_file_name != NULL){
      printf("%s\n",fr->bond_file_name);
    }
    for(p = *bond_type; p != NULL; p = p->next){
      printf("%d %f (%f %f %f)\n",p->type,p->radius,p->color[0],p->color[1],p->color[2]);
    }
  }
  */
  return KS_TRUE;
}
static void set_cdv_bond(char *cp, CV_CDV_BOND_LIST **bond, CV_CDV_BOND_LIST **reuse,
			 unsigned int flags)
{
  int i;
  int bond_type;
  char c0[64];
  unsigned int label[2],sw;
  bond_type = 0;
  for(i = 0; (cp = ks_get_str(" ",cp,c0,sizeof(c0))) != NULL; i++){
    /* printf("cp %d %s\n",i,c0);*/
    if(i < 2){
      label[i] = atoi(c0);
    } else {
      bond_type = atoi(c0);
    }
  }
  if(label[0] > label[1]){
    sw = label[0];
    label[0] = label[1];
    label[1] = sw;
  }
  *bond = add_cv_cdv_bond_list(*bond,new_cv_cdv_bond_list(flags,bond_type,label,reuse));
}
static void set_cdv_face(char *cp, CV_CDV_FACE_LIST **face, CV_CDV_FACE_LIST **reuse)
{
  int i;
  int face_type;
  char c0[64];
  unsigned int label[3];
  face_type = 0;
  for(i = 0; (cp = ks_get_str(" ",cp,c0,sizeof(c0))) != NULL; i++){
    /*    printf("cp %d %s\n",i,c0);*/
    if(i < 3){
      label[i] = atoi(c0);
    } else {
      face_type = atoi(c0);
    }
  }
  *face = add_cv_cdv_face_list(*face,new_cv_cdv_face_list(face_type,label,reuse));
}
CV_ATOM* make_new_atom(int atype)
{
  char *cp;
  char name;
  float *scale;
  float size_scale;
  GLfloat color[3];
  int atype_org = atype;

  if(init_cv_atom_parm[atype].scaled == KS_TRUE){
    scale = init_cv_atom_parm[atype].scale;
  } else {
    scale = NULL;
  }
  name = '\0';

  cp = getenv(CV_ENV_KEY_PARTICLE_SIZE_SCALE);
  if(cp != NULL){
    /*    printf("'%s'\n",cp);*/
    if(cp[0] != 0){
      size_scale = atof(cp)*2;  /* because radius is divided by 2 in KS_GL_ATOM_SIZE_NORMAL */
    } else {
      size_scale = 1.4;
    }
  } else {
    size_scale = 1.4;
  }
  if(atype >= CV_DARK_ATOM_TYPE_ADD){
    atype -= CV_DARK_ATOM_TYPE_ADD;
    color[0] = init_cv_atom_parm[atype].color[0]*.5;
    color[1] = init_cv_atom_parm[atype].color[1]*.5;
    color[2] = init_cv_atom_parm[atype].color[2]*.5;
  } else {
    color[0] = init_cv_atom_parm[atype].color[0];
    color[1] = init_cv_atom_parm[atype].color[1];
    color[2] = init_cv_atom_parm[atype].color[2];
  }

  return new_cv_atom(init_cv_atom_parm[atype].show == KS_TRUE ? CV_ATOM_SHOW:0,
		     atype_org,&name,
		     init_cv_atom_parm[atype].radius*size_scale,
		     color,
		     CV_SPHERE_DETAIL_NORMAL,scale,1.0);
}
static BOOL read_cdv_particle(KS_FILE_LINES *fi, CV_PARTICLE **pl, CV_ATOM **atom, 
			      unsigned int *label_max, unsigned int *file_label_max,
			      CV_PARTICLE **reuse_particle, int *atype_mode_num,
			      CV_CDV_BOND_LIST **bond, CV_CDV_BOND_LIST **reuse_bond,
			      CV_CDV_FACE_LIST **face, CV_CDV_FACE_LIST **reuse_face,
			      int atype_mode)
{
  int i;
  long long int fl;
  char *read_line;
  char *cp;
  char c0[256];
  unsigned int file_label;
  unsigned int set_flg;
  int atype;
  double pos[3];
  CV_ATOM *hit_atom;
  CV_PARTICLE *new_particle;
  int atype_num,atype_num_max=0;
  unsigned int particle_flags;

  for(fl = 0; fl < fi->line_cnt; fl++){
    read_line = fi->file_lines[fl];
    if(is_comment(read_line) == KS_TRUE){
    } else if(strncmp(read_line,CV_FILE_KEY_BOND,CV_FILE_KEY_LEN_BOND) == 0){
      set_cdv_bond(&read_line[CV_FILE_KEY_LEN_BOND],bond,reuse_bond,0);
    } else if(strncmp(read_line,CV_FILE_KEY_FACE,CV_FILE_KEY_LEN_FACE) == 0){
      set_cdv_face(&read_line[CV_FILE_KEY_LEN_FACE],face,reuse_face);
    } else {
      set_flg = 0;
      new_particle = NULL;
      atype_num = 0;
      particle_flags = 0;
      for(cp = read_line, i = 0; (cp = ks_get_str(" ",cp,c0,sizeof(c0))) != NULL; i++){
	/*	printf("%d %d '%s' %x\n",i,set_flg,c0,c0[0]);*/
	if(i == 0){
	  file_label = (unsigned int)(atoi(c0));
	  set_flg |= 1;
	} else if(i == 1){
	  atype = atoi(c0);
	  if(atype >= CV_ATOM_TYPE_MAX){
	    if(!(atype-CV_DARK_ATOM_TYPE_ADD >= 0 && 
		 atype-CV_DARK_ATOM_TYPE_ADD < CV_ATOM_TYPE_MAX)){ // not dark color condition
	      ks_error("atype %d is too large",atype);
	      return KS_FALSE;
	    }
	  }
	  set_flg |= 2;
	} else if(i == 2 || i == 3 || i == 4){
	  pos[i-2] = atof(c0);
	  set_flg |= (int)(pow(2,i)+.5);
	} else if(set_flg == 31 && new_particle != NULL){
	  /*
	  printf("add atype %d %d %d atype_mode %d\n",i-4,atoi(c0),ks_isdigit_all(c0),atype_mode);
	  */
	  if(ks_isdigit_all(c0) == KS_TRUE){
	    atype = atoi(c0);
	    if(atype >= CV_ATOM_TYPE_MAX){
	      if(!(atype-CV_DARK_ATOM_TYPE_ADD >= 0 && 
		   atype-CV_DARK_ATOM_TYPE_ADD < CV_ATOM_TYPE_MAX)){ // not dark color condition
		ks_error("atype %d is too large",atype);
		return KS_FALSE;
	      }
	    }
	    new_particle->atype[i-4] = atype;
	    hit_atom = lookup_cv_atom_atype(*atom,atype);
	    if(atype_mode == i-4){
	      new_particle->atom = hit_atom;
	    }
	    atype_num++;
	    if(hit_atom == NULL){
	      /*
	      char name;
	      name = '\0';
	      hit_atom = new_cv_atom(init_cv_atom_parm[atype].show == KS_TRUE ? CV_ATOM_SHOW:0,
				     atype,&name,
				     init_cv_atom_parm[atype].radius,
				     init_cv_atom_parm[atype].color,
				     CV_SPHERE_DETAIL_NORMAL,NULL,1.0);
	      *atom = add_cv_atom(*atom,hit_atom);
	      */
	      *atom = add_cv_atom(*atom,hit_atom = make_new_atom(atype));
	    }
	    /*	    printf("%p %d %d\n",new_particle,i-4,new_particle->atype[i-4]);*/
	  } else {
	    //	    printf("quaternion %d %s\n",i,c0);
	    if(i-5 < 4){
	      new_particle->quaternion[i-5] = atof(c0);
	    }
	  }
	}
	/*	printf("%d\n",set_flg);*/
	if(set_flg == 31 && new_particle == NULL){
	  hit_atom = lookup_cv_atom_atype(*atom,atype);
	  if(hit_atom == NULL){
	    /*
	    char name;
	    name = '\0';
	    hit_atom = new_cv_atom(init_cv_atom_parm[atype].show == KS_TRUE ? CV_ATOM_SHOW:0,
				   atype,&name,
				   init_cv_atom_parm[atype].radius,
				   init_cv_atom_parm[atype].color,
				   CV_SPHERE_DETAIL_NORMAL,NULL,1.0);
	    *atom = add_cv_atom(*atom,hit_atom);
	    */
	    *atom = add_cv_atom(*atom,hit_atom = make_new_atom(atype));
	  }
	  new_particle = new_cv_particle((*label_max)++,file_label,particle_flags,hit_atom,NULL,
					 pos[0],pos[1],pos[2],reuse_particle);
	}
      }
      if(set_flg != 31){
	ks_error("Illigal file format in following line\n%s",read_line);
	return KS_FALSE;
      }
      if(atype_num_max < atype_num)
	atype_num_max = atype_num;
      /*      printf("\n");*/
      /*
      printf("new %p %d %d %d %f %f %f %d",new_particle,new_particle->label,
	     new_particle->file_label,
	     new_particle->atype[0],
	     new_particle->cd[0],
	     new_particle->cd[1],
	     new_particle->cd[2],atype_num_max);
      for(i = 1; i < CV_ATYPE_MODE_MAX; i++){
	printf(" %d",new_particle->atype[i]);
      }
      printf("\n");
      */
      (*pl) = add_cv_particle((*pl),new_particle);
    }
  }
  /*  printf("%d\n",atype_num_max);*/
  *atype_mode_num = atype_num_max+1;
  if(*atype_mode_num > CV_ATYPE_MODE_MAX){
    ks_error("number of atom type is too large. (MAX is %d)",CV_ATYPE_MODE_MAX);
    return KS_FALSE;
  }
  return KS_TRUE;
}
static void read_cdv_bond(FILE *fp, CV_CDV_BOND_LIST **bond, CV_CDV_BOND_LIST **reuse,
			  unsigned int flags)
{
  char read_line[256];
  while(fgets(read_line,sizeof(read_line),fp) != NULL){
    if(is_comment(read_line) == KS_FALSE){
      /*      printf("%s",read_line);*/
      set_cdv_bond(read_line,bond,reuse,flags);
    }
  }
}
static void read_cdv_face(FILE *fp, CV_CDV_FACE_LIST **face, CV_CDV_FACE_LIST **reuse)
{
  char read_line[256];
  while(fgets(read_line,sizeof(read_line),fp) != NULL){
    if(is_comment(read_line) == KS_FALSE){
      /*      printf("%s",read_line);*/
      set_cdv_face(read_line,face,reuse);
    }
  }
}
static void set_c_color_min_max(CV_FRAME *fr, CV_CONTINUOUS_COLOR *c_color, BOOL set_min_max)
{
  CV_PARTICLE *p;
  int i,j,c;
  double v;
  //  double diff[4];
  BOOL malloc_flg = KS_FALSE;
  if(fr->particle != NULL){
    p = fr->particle;
    if(set_min_max && fr->c_color_info.set_min_max == KS_FALSE){
      for(i = 0; i < c_color->num; i++){
	fr->c_color_info.min[i] = p->quaternion[i];
	fr->c_color_info.max[i] = p->quaternion[i];
      }
      for(p = fr->particle; p != NULL; p = p->next){
	for(i = 0; i < c_color->num; i++){
	  if(fr->c_color_info.min[i] > p->quaternion[i]) fr->c_color_info.min[i] = p->quaternion[i];
	  if(fr->c_color_info.max[i] < p->quaternion[i]) fr->c_color_info.max[i] = p->quaternion[i];
	}
      }
    }
    /*
    for(i = 0; i < c_color->num; i++){
      diff[i] = fr->c_color_info.max[i] - fr->c_color_info.min[i];
    }
    for(p = fr->particle; p != NULL; p = p->next){
      for(i = 0; i < c_color->num; i++){
	p->quaternion[i] = (p->quaternion[i]-fr->c_color_info.min[i])/diff[i];
      }
    }
    */
    if(fr->c_color_info.map == NULL){
      malloc_flg = KS_TRUE;
    }
    if(malloc_flg){
      if((fr->c_color_info.map = (CV_COLOR_MAP**)ks_malloc(4*sizeof(CV_COLOR_MAP*),
							   "c_color_info.map")) == NULL){
	ks_error_memory();
	ks_exit(EXIT_FAILURE);
      }
      for(c = 0; c < 4; c++){
	fr->c_color_info.map[c] = NULL;
      }
    }
    for(c = 0; c < c_color->num; c++){
      if(malloc_flg){
	if((fr->c_color_info.map[c] = cv_allocate_color_map()) == NULL){
	  ks_error_memory();
	  ks_exit(EXIT_FAILURE);
	}
      }
      fr->c_color_info.map[c]->num = 5;
      if(malloc_flg){
	if(cv_allocate_color_map_color(fr->c_color_info.map[c]) == KS_FALSE){
	  ks_error_memory();
	  ks_exit(EXIT_FAILURE);
	}
      }
      for(i = 0; i < fr->c_color_info.map[c]->num; i++){
	v = (double)i/(fr->c_color_info.map[c]->num-1);
	if(fr->c_color_info.start_color != NULL && fr->c_color_info.end_color != NULL){
	  for(j = 0; j < 3; j++){
	    fr->c_color_info.map[c]->color[i][j] = ((1.0-v)*fr->c_color_info.start_color[j]+
						    v*fr->c_color_info.end_color[j]);
	  }
	} else {
	  double r,g,b;
	  ks_hsv2rgb((1.0-v)*240,1.0,1.0,&r,&g,&b);
	  fr->c_color_info.map[c]->color[i][0] = r;
	  fr->c_color_info.map[c]->color[i][1] = g;
	  fr->c_color_info.map[c]->color[i][2] = b;
	}
	fr->c_color_info.map[c]->value[i] = ((1.0-v)*fr->c_color_info.min[c]+
					     v*fr->c_color_info.max[c]);
      }
    }
  }
}
static BOOL read_bond_and_face_file(CV_FRAME *fr, unsigned int cdv_bond_flags)
{
  FILE *fp;
  if(fr->bond_file_name != NULL){
    ks_remove_return(fr->bond_file_name);
    if((fp = fopen(fr->bond_file_name,"rt")) == NULL){
      if(fr->file_path[0] == '\0'){
	ks_error_file(fr->bond_file_name);
	return KS_FALSE;
      } else {
	char file_name[256];
	if(strlen(fr->file_path) + strlen(fr->bond_file_name) > sizeof(file_name)){
	  ks_error("file name %s%s is too long",fr->file_path,fr->bond_file_name);
	  return KS_FALSE;
	}
	strcpy(file_name,fr->file_path);
	strcat(file_name,fr->bond_file_name);
	if((fp = fopen(file_name,"rt")) == NULL){
	  ks_error_file(fr->bond_file_name);
	  return KS_FALSE;
	}
      }
    }
    read_cdv_bond(fp,&fr->cdv_bond,&fr->reuse->cdv_bond,cdv_bond_flags);
    fclose(fp);
    /*    ks_exit(EXIT_FAILURE);*/
  }
  if(fr->face_file_name != NULL){
    ks_remove_return(fr->face_file_name);
    if((fp = fopen(fr->face_file_name,"rt")) == NULL && fr->file_path[0] == '\0'){
      ks_error_file(fr->face_file_name);
      return KS_FALSE;
    } else {
      char file_name[256];
      if(strlen(fr->file_path) + strlen(fr->face_file_name) > sizeof(file_name)){
	ks_error("file name %s%s is too long",fr->file_path,fr->face_file_name);
	return KS_FALSE;
      }
      strcpy(file_name,fr->file_path);
      strcat(file_name,fr->face_file_name);
      if((fp = fopen(file_name,"rt")) == NULL){
	ks_error_file(fr->face_file_name);
	return KS_FALSE;
      }
    }
    read_cdv_face(fp,&fr->cdv_face,&fr->reuse->cdv_face);
    fclose(fp);
    /*    ks_exit(EXIT_FAILURE);*/
  }
  return KS_TRUE;
}
static BOOL read_cdv_frame(CV_FRAME *fr, CV_BASE *cb)
{
  printf("read cdv %s\n",fr->file_name);
  fflush(stdout);

  fr->flags |= CV_FRAME_READ;

  ks_read_file_lines(fr->file_name,*fr->fip);

  if(read_cdv_header(*fr->fip,fr,cb,KS_FALSE) == KS_FALSE){
    return KS_FALSE;
  }
  if(cb->header_file_name != NULL){
    KS_FILE_LINES *fi_header;
    if((fi_header = ks_allocate_file_lines()) == NULL){
      return KS_FALSE;
    }
    if(read_cdv_header(fi_header,fr,cb,KS_TRUE) == KS_FALSE){
      return KS_FALSE;
    }
    ks_free_file_lines(fi_header);
  }

  if(read_cdv_particle(*fr->fip,&fr->particle,&cb->atom,&fr->particle_label_max,
		       &fr->particle_file_label_max,
		       &fr->reuse->particle,&cb->atype_mode_num,
		       &fr->cdv_bond,&fr->reuse->cdv_bond,
		       &fr->cdv_face,&fr->reuse->cdv_face,
		       fr->atype_mode)
     == KS_FALSE){
    return KS_FALSE;
  }
  /*
  {
    CV_ATOM *p;
    for(p = *atom; p != NULL; p = p->next){
      printf("%d %s\n",p->atype,p->name);
    }
  }
  */
  /*
  {
    CV_PARTICLE *p;
    for(p = fr->particle; p != NULL; p = p->next){
      printf("%d %d %f %f %f %f %f %f %f\n",p->label,p->atype[0],
	     p->cd[0],p->cd[1],p->cd[2],
	     p->quaternion[0],
	     p->quaternion[1],
	     p->quaternion[2],
	     p->quaternion[3]);
    }
    ks_exit(EXIT_FAILURE);
  }
  */

  if(cb->continuous_color.num != 0){
    set_c_color_min_max(fr,&cb->continuous_color,KS_TRUE);
  }

  if(read_bond_and_face_file(fr,0) == KS_FALSE){
    return KS_FALSE;
  }

  return KS_TRUE;
}
static void set_particle_color(CV_PARTICLE *p, KS_CHAR_LIST **unknown_residue_label, 
			       unsigned int *unknown_residue_label_max)
{
  int type;
  double hydropathy;
  if(p->property != NULL){
    p->color_mode[CV_COLOR_MODE_PARTICLE] = KS_GL_ATOM_COLOR_ATOM;
    if(ks_get_amino_id(p->property->residue->name,&type) == KS_TRUE){
      p->color_mode[CV_COLOR_MODE_AMINO] = type+KS_GL_ATOM_COLOR_AMINO;
    } else {
      KS_CHAR_LIST *hit;
      /*      printf("%s\n",p->property->residue->name);*/
      hit = ks_lookup_char_list(*unknown_residue_label,p->property->residue->name);
      if(hit == NULL){
	hit = ks_new_char_list(p->property->residue->name,(*unknown_residue_label_max)++);
	*unknown_residue_label = ks_add_char_list(*unknown_residue_label,hit);
      }
      p->color_mode[CV_COLOR_MODE_AMINO] = (KS_GL_ATOM_COLOR_UNKNOWN+
					    (hit->value%KS_GL_ATOM_COLOR_UNKNOWN_NUM));
      /*
      p->color_mode[CV_COLOR_MODE_AMINO] = KS_GL_ATOM_COLOR_UNKNOWN;
      */
      /*      printf("%s %d %d\n",hit->name,hit->value,p->color_mode[CV_COLOR_MODE_AMINO]);*/
    }
    if(ks_get_amino_type(p->property->residue->name,&type) == KS_TRUE){
      p->color_mode[CV_COLOR_MODE_AMINO_TYPE] = type+KS_GL_ATOM_COLOR_AMINO;
      if(type == KS_AMINO_ACIDIC || 
	 type == KS_AMINO_BASIC || 
	 type == KS_AMINO_POLAR || 
	 type == KS_AMINO_NO_CHARGE){
	p->color_mode[CV_COLOR_MODE_AMINO_TYPE] = type+KS_GL_ATOM_COLOR_AMINO;
      } else {
	p->color_mode[CV_COLOR_MODE_AMINO_TYPE] = KS_GL_ATOM_COLOR_UNKNOWN;
      }
    } else {
      p->color_mode[CV_COLOR_MODE_AMINO_TYPE] = KS_GL_ATOM_COLOR_UNKNOWN;
    }
    if(ks_get_amino_hydropathy_index(p->property->residue->name,&hydropathy) == KS_TRUE){
      p->color_mode[CV_COLOR_MODE_AMINO_HYDROPATHY] = 
	(int)((hydropathy+4.5)*5/9)+KS_GL_ATOM_COLOR_CHARGE;
      /*
	printf("%f %d\n"
	,hydropathy
	,p->color_mode[CV_COLOR_MODE_AMINO_HYDROPATHY]-KS_GL_ATOM_COLOR_CHARGE);
      */
    } else {
      p->color_mode[CV_COLOR_MODE_AMINO_HYDROPATHY] = KS_GL_ATOM_COLOR_UNKNOWN;
    }
    p->color_mode[CV_COLOR_MODE_CHAIN] = 
      (p->property->chain->label%(KS_AMINO_LIST_NUM-1))+KS_GL_ATOM_COLOR_AMINO;
    p->color_mode[CV_COLOR_MODE_AGGREGATE] = 
      (p->property->aggregate->label%(KS_AMINO_LIST_NUM-1))+KS_GL_ATOM_COLOR_AMINO;

    p->color_mode[CV_COLOR_MODE_RAINBOW] = KS_GL_ATOM_COLOR_UNKNOWN;
    /*
      printf("%d %s %d %d\n",p->label,p->property->name
      ,p->color_mode[CV_COLOR_MODE_PARTICLE]
      ,p->color_mode[CV_COLOR_MODE_AMINO]);
    */
  }
}
static void set_particle_color_frame(CV_FRAME *fr)
{
  CV_PARTICLE *p;
  for(p = fr->particle; p != NULL; p = p->next){
    set_particle_color(p,&fr->unknown_residue_label,&fr->unknown_residue_label_max);
  }
}
static void get_charge_color(double charge, GLfloat* color)
{
  double dcolor[3];
  double max = 0.9;
  double s,h;
  double abs_charge = fabs(charge);
  if(charge > 0){
    h = 240.0; /* blue */
  } else {
    h = 0.0;   /* red */
  }
  if(abs_charge > max){
    s = 1.0;
  } else {
    s = abs_charge/max;
  }
  ks_hsv2rgb(h,s,1.0,&dcolor[0],&dcolor[1],&dcolor[2]);
  color[0] = dcolor[0];
  color[1] = dcolor[1];
  color[2] = dcolor[2];
  color[3] = 1.0;
}
static void set_color_sample(CV_FRAME *fr, GLuint *sphere_texture, 
			     CV_ATOM **atom, KS_SPHERE **color_sample, BOOL prep_mode)
{
  int i;
  CV_ATOM *at;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  float r;
  int label = 0;
  float color[3];
  GLfloat *atom_color;
  GLfloat light_position[] = {1.0, 1.1, 1.2, 0.0};
  *sphere_texture = ks_set_gl_sphere_texture(7,light_position);

  for(at = (*atom); at != NULL; at = at->next){
    if(ks_get_atom_radius(at->name,&r) == KS_FALSE){
      r = 1.0;
    }
    if(ks_lookup_sphere(color_sample[CV_COLOR_MODE_PARTICLE],at->name)==NULL && r<10.0){
      atom_color = ks_get_gl_atom_color(at->gl_atom,KS_GL_ATOM_COLOR_ATOM,
					KS_GL_ATOM_BRIGHT_NORMAL);
      for(i = 0; i < 3; i++){
	color[i] = atom_color[i];
      }
      if(ks_get_atom_color(at->name,color) == KS_FALSE){
	ks_get_gl_unknown_color(0,color);
      }
      color_sample[CV_COLOR_MODE_PARTICLE]=
	ks_addend_sphere(color_sample[CV_COLOR_MODE_PARTICLE],
			 ks_new_sphere(label++,at->name,r,color));
    }
  }
  r = 1.2;
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	if(ks_lookup_sphere(color_sample[CV_COLOR_MODE_AMINO],pr->name)==NULL){
	  if(ks_get_amino_color(pr->name,color) == KS_FALSE){
	    ks_get_gl_unknown_color(0,color);
	  }
	  ks_assert(pr->particle[0] != NULL);
	  atom_color = ks_get_gl_atom_color(pr->particle[0]->atom->gl_atom,
					    pr->particle[0]->color_mode[CV_COLOR_MODE_AMINO],
					    KS_GL_ATOM_BRIGHT_NORMAL);
	  for(i = 0; i < 3; i++){
	    color[i] = atom_color[i];
	  }
	  /*
	    printf("%s %d (%f %f %f)\n",pr->name,ks_get_amino_color(pr->name,color)
	    ,color[0],color[1],color[2]);
	  */
	  color_sample[CV_COLOR_MODE_AMINO]=
	    ks_addend_sphere(color_sample[CV_COLOR_MODE_AMINO],
			     ks_new_sphere(label++,pr->name,r,color));
	}
      }
    }
  }
  {
    int amino_type[KS_AMINO_TYPE_NUM] = {KS_AMINO_ACIDIC,
					 KS_AMINO_BASIC,
					 KS_AMINO_POLAR,
					 KS_AMINO_NO_CHARGE};
    char *amino_type_name[KS_AMINO_TYPE_NUM] = {"Acidic","Basic","Polar","Uncharged"};

    for(i = 0; i < KS_AMINO_TYPE_NUM; i++){
      if(ks_lookup_sphere(color_sample[CV_COLOR_MODE_AMINO_TYPE],amino_type_name[i]) == NULL){
	color_sample[CV_COLOR_MODE_AMINO_TYPE]=
	  ks_addend_sphere(color_sample[CV_COLOR_MODE_AMINO_TYPE],
			   ks_new_sphere(label++,
					 amino_type_name[i],r,ks_amino[amino_type[i]].color));
      }
    }
  }
  {
    char name[8];
    for(i = 0; i < 6; i++){
      sprintf(name,"%+.1f",-4.5+9./5.*i);
      ks_get_gl_charge_color(i,color);
      if(ks_lookup_sphere(color_sample[CV_COLOR_MODE_AMINO_HYDROPATHY],name) == NULL){
	color_sample[CV_COLOR_MODE_AMINO_HYDROPATHY]=
	  ks_addend_sphere(color_sample[CV_COLOR_MODE_AMINO_HYDROPATHY],
			   ks_new_sphere(label++,name,r,color));
      }
    }
  }
  if(prep_mode == KS_TRUE){
    float charge;
    char name[8];
    r = 1.2;
    for(charge = 1.0; charge > -1.2; charge -= 0.25){
      get_charge_color(charge,color);
      if(fabs(charge) < 0.01){
	sprintf(name,"  %.2f",charge);
      } else {
	sprintf(name,"%+.2f",charge);
      }
      if(ks_lookup_sphere(color_sample[CV_COLOR_MODE_RAINBOW],name) == NULL){
	color_sample[CV_COLOR_MODE_RAINBOW] = 
	  ks_addend_sphere(color_sample[CV_COLOR_MODE_RAINBOW],
			   ks_new_sphere(label++,name,r,color));
      }
    }
  }
  /*
    {
    KS_SPHERE *p;
    int j;
    for(j = 0; j < CV_COLOR_MODE_NUM; j++){
    for(p = color_sample[j]; p != NULL; p = p->next){
    printf("%d %d %s %f (%f %f %f)\n",j,p->label,p->name,p->radius
    ,p->color[0]
    ,p->color[1]
    ,p->color[2]);
    }
    }
    }
  */
}
static void set_center_and_size_frame(CV_FRAME *fr, int *kabe_mode)
{
  int i;
  CV_PARTICLE *p;
  double dd[4];

  if(fr->particle != NULL){
    for(i = 0; i < 3; i++)
      fr->center[i] = 0;
    dd[0] = 0;
    for(p = fr->particle; p != NULL; p = p->next){
      for(i = 0; i < 3; i++)
	fr->center[i] += p->cd[i]*p->atom->mass;
      dd[0] += p->atom->mass;
    }
    for(i = 0; i < 3; i++)
      fr->center[i] /= dd[0];

    if(fr->have_side == KS_TRUE){
      /*
      printf("%f %f %f\n",
	     (fr->side[1][0][0][0] - fr->side[0][0][0][0]),
	     (fr->side[0][1][0][1] - fr->side[0][0][0][1]),
	     (fr->side[0][0][1][2] - fr->side[0][0][0][2]));
      */
      fr->system_size = ks_max((fr->side[1][0][0][0] - fr->side[0][0][0][0]),
			       ks_max((fr->side[0][1][0][1] - fr->side[0][0][0][1]),
				      (fr->side[0][0][1][2] - fr->side[0][0][0][2])));
    } else {
      /*  printf("%f %f %f\n",fr->center[0],fr->center[1],fr->center[2]);*/
      fr->system_size = 0;
      for(p = fr->particle; p != NULL; p = p->next){
	for(i = 0; i < 3; i++)
	  dd[i] = p->cd[i] - fr->center[i];
	dd[3] = dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2];
	if(fr->system_size < dd[3]){
	  fr->system_size = dd[3];
	  /*
	  printf("%d %s %f %f %f %f\n"
		 ,p->label,p->property->name,sqrt(dd[3]),p->cd[0],p->cd[1],p->cd[2]);
	  */
	}
      }
      fr->system_size = sqrt(fr->system_size)*2*1.2;
    }
  } else {
    fv_set_center_and_size(fr->fv->field,fr->fv->field_size,fr->center,&fr->system_size);
  }
}
void set_second_structure_frame(CV_FRAME *fr)
{
  int i;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr,*prev = NULL, *probe;
  CV_PARTICLE *p;
  double ang[2];
  double dd[7][3];
  int i0;
  double d0,d1;
  int required_sequence_num;
  double n_o_bond_max_square = 3.2*3.2;

  if(fr->aggregate != NULL){
    for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      /*      printf("agg %d\n",pa->label);*/
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	/*	printf("  chain %d %s\n",pc->label,pc->name);*/
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  if(pr == pc->residue){
	    pr->flags |= CV_RESIDUE_START|CV_RESIDUE_LOOP;
	  } else if(pr->next == NULL){
	    pr->flags |= CV_RESIDUE_END|CV_RESIDUE_LOOP;
	  } else {
	    pr->flags |= CV_RESIDUE_NORMAL;
	  }
	  for(i = 0; i < pr->particle_num; i++){
	    p = pr->particle[i];
	    /*
	    printf("      %d %d %s '%s' %p\n",i,p->file_label,p->atom->name,
		   p->property->name,p->property->name);
	    */
	    if(strcmp(p->property->name,"N   ") == 0){
	      pr->main_N = p;
	    } else if(strcmp(p->property->name,"CA  ") == 0){
	      pr->main_CA = p;
	    } else if(strcmp(p->property->name,"C   ") == 0){
	      pr->main_C = p;
	    } else if(strcmp(p->property->name,"O   ") == 0){
	      pr->main_O = p;
	    }
	  }
	  if(pr->main_N!=NULL && pr->main_CA!=NULL && pr->main_C!=NULL && pr->main_O!=NULL){
	    pr->flags |= CV_RESIDUE_AMINO;
	    for(i = 0; i < 3; i++){
	      pr->vec_C_O[i] = pr->main_O->cd[i] - pr->main_C->cd[i];
	    }
	  }
	  /*
	  printf("    %d %s %s %s\n",pr->label,pr->name,
		 pr->flags&CV_RESIDUE_START ? "START":
		 pr->flags&CV_RESIDUE_END ? "END":
		 pr->flags&CV_RESIDUE_NORMAL ? "NORMAL":"NON",
		 pr->flags&CV_RESIDUE_AMINO ? "AMINO":"NON"
		 );
	  */
	}
	i0 = 0;
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  if(pr->flags&CV_RESIDUE_AMINO){
	    i0++;
	  }
	}
	if(i0 != 0){
	  pc->flags |= CV_CHAIN_AMINO;
	}
      }
    }

    for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  if(pr->flags&CV_RESIDUE_NORMAL && pr->flags&CV_RESIDUE_AMINO && 
	     prev->flags&CV_RESIDUE_AMINO && pr->next->flags&CV_RESIDUE_AMINO){
	    /*
	    printf("    %d %s %s %s    %d %s %s %s\n",
		   pr->label,pr->name,
		   pr->flags&CV_RESIDUE_START ? "START":
		   pr->flags&CV_RESIDUE_END ? "END":
		   pr->flags&CV_RESIDUE_NORMAL ? "NORMAL":"NON",
		   pr->flags&CV_RESIDUE_AMINO ? "AMINO":"NON",
		   pr->next->label,pr->next->name,
		   pr->next->flags&CV_RESIDUE_START ? "START":
		   pr->next->flags&CV_RESIDUE_END ? "END":
		   pr->next->flags&CV_RESIDUE_NORMAL ? "NORMAL":"NON",
		   pr->next->flags&CV_RESIDUE_AMINO ? "AMINO":"NON"
		   );
	    */
	    for(i = 0; i < 3; i++){
	      dd[0][i] = prev->main_C->cd[i]  - pr->main_N->cd[i];
	      dd[1][i] = pr->main_CA->cd[i]   - pr->main_N->cd[i];
	    }
	    ks_exterior_product(dd[0],dd[1],dd[2]);
	    for(i = 0; i < 3; i++){
	      dd[3][i] = pr->main_N->cd[i] - pr->main_CA->cd[i];
	      dd[4][i] = pr->main_C->cd[i] - pr->main_CA->cd[i];
	    }
	    ks_exterior_product(dd[3],dd[4],dd[5]);
	    ks_exterior_product(dd[2],dd[5],dd[6]);
	    ang[0] = (ks_calc_vectors_angle(dd[2],dd[5])/M_PI*180*
		      ((pr->main_CA->cd[0]-pr->main_N->cd[0])*dd[6][0]<0 ? -1:1));
	    for(i = 0; i < 3; i++){
	      dd[0][i] = dd[3][i];
	      dd[1][i] = dd[4][i];
	      dd[2][i] = dd[5][i];
	    }
	    for(i = 0; i < 3; i++){
	      dd[3][i] = pr->main_CA->cd[i] - pr->main_C->cd[i];
	      dd[4][i] = pr->next->main_N->cd[i] - pr->main_C->cd[i];
	    }
	    ks_exterior_product(dd[3],dd[4],dd[5]);
	    ks_exterior_product(dd[2],dd[5],dd[6]);
	    ang[1] = (ks_calc_vectors_angle(dd[2],dd[5])/M_PI*180*
		      ((pr->main_C->cd[0]-pr->main_CA->cd[0])*dd[6][0]<0 ?-1:1));

	    if(ang[0] > -170 && ang[0] < -40 && ang[1] > -80 && ang[1] < 5){
	      pr->flags |= CV_RESIDUE_ALPHA;
	    } else if(ang[0] > -170 && ang[0] < -50 && 
		      ((ang[1] > 70 && ang[1] < 180) || (ang[1] > -180 && ang[1] < -170))){
	      pr->flags |= CV_RESIDUE_BETA;
	    } else {
	      pr->flags |= CV_RESIDUE_LOOP;
	    }
	  }
	  /*
	    printf("    %d %s %s\n",pr->label,pr->name,
	    pr->flags&CV_RESIDUE_ALPHA ? "ALPHA":
	    pr->flags&CV_RESIDUE_BETA ? "BETA":
	    pr->flags&CV_RESIDUE_LOOP ? "LOOP":"NON");
	  */
	  prev = pr;
	}
      }
    }

    for(pa = fr->aggregate; pa != NULL; pa = pa->next){ /* check sequence num */
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	for(pr = pc->residue; pr != NULL;){
	  i0 = 0;
	  if(pr->next != NULL && (pr->flags&CV_RESIDUE_ALPHA || pr->flags&CV_RESIDUE_BETA)){
	    for(probe = pr; 
		((probe->flags&(CV_RESIDUE_ALPHA|CV_RESIDUE_BETA)) ==
		 (pr->flags&   (CV_RESIDUE_ALPHA|CV_RESIDUE_BETA))) && probe != NULL;
		probe = probe->next){
	      i0++;
	    }
	  }
	  if(pr->flags&CV_RESIDUE_ALPHA)
	    required_sequence_num = 4;
	  else
	    required_sequence_num = 3;
	  if(i0 >= required_sequence_num){
	    pr = probe;
	  } else {
	    if(pr->flags&CV_RESIDUE_ALPHA || pr->flags&CV_RESIDUE_BETA){
	      pr->flags &= ~(CV_RESIDUE_ALPHA|CV_RESIDUE_BETA);
	      pr->flags |= CV_RESIDUE_LOOP;
	    }
	    pr = pr->next;
	  }
	}
      }
    }

    for(pa = fr->aggregate; pa != NULL; pa = pa->next){/* calc n_bond o_bond of beta */
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  if(pr->flags&CV_RESIDUE_BETA){
	    for(probe = pr->next; probe->flags&CV_RESIDUE_BETA; probe = probe->next);
	    for(; probe != NULL; probe = probe->next){
	      if(probe->flags&CV_RESIDUE_BETA){
		for(i = 0; i < 3; i++) dd[0][i] = pr->main_N->cd[i] - probe->main_O->cd[i];
		for(i = 0; i < 3; i++) dd[1][i] = pr->main_O->cd[i] - probe->main_N->cd[i];
		d0 = dd[0][0]*dd[0][0]+dd[0][1]*dd[0][1]+dd[0][2]*dd[0][2];
		d1 = dd[1][0]*dd[1][0]+dd[1][1]*dd[1][1]+dd[1][2]*dd[1][2];
		if(d0 < n_o_bond_max_square){
		  pr->flags |= CV_RESIDUE_BETA_N_BOND;
		  probe->flags |= CV_RESIDUE_BETA_O_BOND;
		}
		if(d1 < n_o_bond_max_square){
		  pr->flags |= CV_RESIDUE_BETA_O_BOND;
		  probe->flags |= CV_RESIDUE_BETA_N_BOND;
		}
		/*
		  printf("      probe %d %s %s %f %f\n",probe->label,probe->type,
		  probe->flags&CV_RESIDUE_ALPHA ? "ALPHA":
		  probe->flags&CV_RESIDUE_BETA  ? "BETA" :
		  probe->flags&CV_RESIDUE_LOOP  ? "LOOP" : "NON",
		  sqrt(d0),sqrt(d1));
		*/
	      }
	    }
	  }
	}
      }
    }

    for(pa = fr->aggregate; pa != NULL; pa = pa->next){/* check n_bond o_bond of beta*/
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  /*
	    printf("    residue %d %s %s N %c O %c\n",pr->label,pr->type,
	    pr->flags&CV_RESIDUE_ALPHA ? "ALPHA":
	    pr->flags&CV_RESIDUE_BETA  ? "BETA" :
	    pr->flags&CV_RESIDUE_LOOP  ? "LOOP" : "NON",
	    pr->flags&CV_RESIDUE_BETA_N_BOND ? 'o':'x',
	    pr->flags&CV_RESIDUE_BETA_O_BOND ? 'o':'x');
	  */
	  if(pr->flags&CV_RESIDUE_BETA){
	    if(pr->flags&CV_RESIDUE_BETA &&
	       !(pr->flags&CV_RESIDUE_BETA_N_BOND) &&
	       !(pr->flags&CV_RESIDUE_BETA_O_BOND) &&
	       !(prev->flags&CV_RESIDUE_BETA_N_BOND) &&
	       !(prev->flags&CV_RESIDUE_BETA_O_BOND) &&
	       !(pr->next->flags&CV_RESIDUE_BETA_N_BOND) &&
	       !(pr->next->flags&CV_RESIDUE_BETA_O_BOND)){
	      /*	    printf("no_beta neighbor\n");*/
	      pr->flags &= ~CV_RESIDUE_BETA;
	      pr->flags |= CV_RESIDUE_LOOP;
	    }
	  }
	  prev = pr;
	}
      }
    }

    for(pa = fr->aggregate; pa != NULL; pa = pa->next){/*check n_bond o_bond of beta */
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  if(pr->flags&CV_RESIDUE_BETA){
	    i0 = 0;
	    for(probe = pr; probe->flags&CV_RESIDUE_BETA; probe = probe->next){
	      if(probe->flags&CV_RESIDUE_BETA_N_BOND) i0++;
	      if(probe->flags&CV_RESIDUE_BETA_O_BOND) i0++;
	    }
	    /*
	      printf("    residue %d %s %s N %c O %c %d\n",pr->label,pr->type,
	      pr->flags&CV_RESIDUE_ALPHA ? "ALPHA":
	      pr->flags&CV_RESIDUE_BETA  ? "BETA" :
	      pr->flags&CV_RESIDUE_LOOP  ? "LOOP" : "NON",
	      pr->flags&CV_RESIDUE_BETA_N_BOND ? 'o':'x',
	      pr->flags&CV_RESIDUE_BETA_O_BOND ? 'o':'x',i0);
	    */
	    if(i0 < 3){
	      for(probe = pr; probe->flags&CV_RESIDUE_BETA; probe = probe->next){
		/*
		  printf("no beta %d %s\n",probe->label,probe->type);
		*/
		probe->flags &= ~CV_RESIDUE_BETA;
		probe->flags |= CV_RESIDUE_LOOP;
	      }
	    }
	    pr = probe;
	    if(pr->flags&CV_RESIDUE_BETA &&
	       !(pr->flags&CV_RESIDUE_BETA_N_BOND) &&
	       !(pr->flags&CV_RESIDUE_BETA_O_BOND) &&
	       !(prev->flags&CV_RESIDUE_BETA_N_BOND) &&
	       !(prev->flags&CV_RESIDUE_BETA_O_BOND) &&
	       !(pr->next->flags&CV_RESIDUE_BETA_N_BOND) &&
	       !(pr->next->flags&CV_RESIDUE_BETA_O_BOND)){
	      pr->flags &= ~CV_RESIDUE_BETA;
	      pr->flags |= CV_RESIDUE_LOOP;
	    }
	  }
	  prev = pr;
	}
      }
    }
    /*
      for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      printf("agg %d\n",pa->label);
      for(pc = pa->chain; pc != NULL; pc = pc->next){
      printf("  chain %d %s %s\n",pc->label,pc->name,pc->flags&CV_CHAIN_AMINO ? "AMINO":"NON");
      for(pr = pc->residue; pr != NULL; pr = pr->next){
      if(pr->flags&CV_RESIDUE_AMINO){
      printf("    %d %s %s\n",pr->label,pr->name,
      pr->flags&CV_RESIDUE_ALPHA ? "ALPHA":
      pr->flags&CV_RESIDUE_BETA ? "BETA":
      pr->flags&CV_RESIDUE_LOOP ? "LOOP":"NON");
      }
      }
      }
      }
    */
  }
}
static BOOL is_pdb_bond(CV_PARTICLE *p0, CV_PARTICLE *p1, double *rd)
{
  int i;
  double dd[3];
  //  unsigned int check_label = 1665;

  *rd = 0;
  for(i = 0; i < 3; i++)
    dd[i] = p0->cd[i] - p1->cd[i];
  *rd = dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2];
  /*
  if(p0->file_label == check_label || p1->file_label == check_label){
    printf("%d %-4s %s %d %-4s %s %f %f %f\n"
	   ,p0->file_label,p0->property->name,p0->atom->name
	   ,p1->file_label,p1->property->name,p1->atom->name
	   ,sqrt(*rd),*rd,1.12*1.12);
  }
  */
  if((*rd < 1.20*1.20 && ((p0->atom->name[0] == 'H' && p1->atom->name[0] != 'H') ||
			  (p0->atom->name[0] != 'H' && p1->atom->name[0] == 'H'))) || 
     (*rd < 1.70*1.70 && (p0->atom->name[0] != 'H' && p1->atom->name[0] != 'H')) || 
     (*rd < 1.80*1.80 && (p0->atom->name[0] == 'P' || p1->atom->name[0] == 'P')) ||
     (*rd < 1.92*1.92 && (p0->atom->name[0] == 'S' || p1->atom->name[0] == 'S')) 
      ){
    /*
    if(p0->file_label == check_label || p1->file_label == check_label)
    printf("(%d %s)(%d %s) %f\n"
	   ,p0->label,p0->property->name
	   ,p1->label,p1->property->name,sqrt(*rd));
    */
    if((p0->atom->name[0] == 'D' || p1->atom->name[0] == 'D') &&
       (p0->atom->name[0] != 'D' || p1->atom->name[0] != 'D') &&
       (p0->label != 3 && p1->label != 3)){
      /*
      printf("(%d %s)(%d %s) %f\n"
	     ,p0->label,p0->property->name
	     ,p1->label,p1->property->name,sqrt(*rd));
      */
      return KS_FALSE;
    }
    return KS_TRUE;
  }
  return KS_FALSE;
}
static void set_inter_residue_bond(CV_RESIDUE *pr0, CV_RESIDUE *pr1, CV_BOND **reuse)
{
  int i,j;
  CV_PARTICLE *p0,*p1;
  double rd;

  for(i = 0; i < pr0->particle_num; i++){
    p0 = pr0->particle[i];
    for(j = 0; j < pr1->particle_num; j++){
      p1 = pr1->particle[j];
      if(is_pdb_bond(p0,p1,&rd) == KS_TRUE){
	p0->bond = add_cv_bond(p0->bond, new_cv_bond(0,p1,sqrt(rd),reuse));
      }
    }
  }
}
static void set_residue_bond(CV_RESIDUE *pr, CV_BOND **reuse)
{
  int i,j;
  CV_PARTICLE *p0,*p1;
  double rd,r;
  for(i = 0; i < pr->particle_num; i++){
    p0 = pr->particle[i];
    for(j = i+1; j < pr->particle_num; j++){
      p1 = pr->particle[j];
      if(is_pdb_bond(p0,p1,&rd) == KS_TRUE){
	r = sqrt(rd);
	p0->bond = add_cv_bond(p0->bond, new_cv_bond(0,p1,r,reuse));
	p1->bond = add_cv_bond(p1->bond, new_cv_bond(0,p0,r,reuse));
      }
    }
  }
}
static void set_bond_all(CV_PARTICLE *pl, CV_BOND **reuse)
{
  CV_PARTICLE *p0,*p1;
  double rd,r;
  for(p0 = pl; p0 != NULL; p0 = p0->next){
    for(p1 = pl; p1 != NULL; p1 = p1->next){
      if(p0->label < p1->label){
	if(is_pdb_bond(p0,p1,&rd) == KS_TRUE){
	  r = sqrt(rd);
	  p0->bond = add_cv_bond(p0->bond, new_cv_bond(0,p1,r,reuse));
	  p1->bond = add_cv_bond(p1->bond, new_cv_bond(0,p0,r,reuse));
	}
      }
    }
  }
}
static void set_pdb_bond_frame(CV_FRAME *fr)
{
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr,*prev;
  //  CV_PARTICLE *p;
  //  CV_BOND *bond;

  if(fr->aggregate != NULL){
    for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      /*	printf("agg %d\n",pa->label);*/
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	/*	  printf("  chain %d %s\n",pc->label,pc->name);*/
	prev = NULL;
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  /*
	    printf("    %d %s %s %s\n",pr->label,pr->name,
	    pr->flags&CV_RESIDUE_START ? "START":
	    pr->flags&CV_RESIDUE_END ? "END":
	    pr->flags&CV_RESIDUE_NORMAL ? "NORMAL":"NON",
	    pr->flags&CV_RESIDUE_AMINO ? "AMINO":"NON"
	    );
	  */
	  if(prev != NULL){
	    set_inter_residue_bond(pr,prev,&fr->reuse->bond);
	  }
	  set_residue_bond(pr,&fr->reuse->bond);
	  if(pr->next != NULL){
	    set_inter_residue_bond(pr,pr->next,&fr->reuse->bond);
	  }
	  /*
	  for(i = 0; i < pr->particle_num; i++){
	    p = pr->particle[i];
	    if(pr->label == 104){
	      printf("     %d %d %d %s:",pr->label,i,p->file_label,p->atom->name);
	      for(bond = p->bond; bond != NULL; bond = bond->next){
		printf("(%d %s %f)",bond->p->label,bond->p->atom->name,bond->len);
	      }
	      printf("\n");
	    }
	  }
	  */
	  prev = pr;
	}
      }
    }
  }
  /*
  {
    CV_PARTICLE *p;
    int num;
    for(p = fr->particle; p != NULL; p = p->next){
      num = count_cv_bond(p->bond);
      if(num == 0){
	printf("%d %-4s %d\n",p->file_label,p->property->name,num);
      }
    }
  }
  */
}
KS_LABEL_LIST *new_label_list(unsigned int label, unsigned int value, unsigned int flags, 
			      KS_LABEL_LIST **reuse)
{
  KS_LABEL_LIST *newp;
  if((*reuse) == NULL){
    return ks_new_label_list(label,value,flags);
  } else {
    newp = *reuse;
    (*reuse) = (*reuse)->next;
    newp->label = label;
    newp->value = value;
    newp->flags = flags;
    newp->next = NULL;
    return newp;
  }
}
static KS_LABEL_LIST *lookup_label_list(KS_LABEL_LIST **hash, int hash_size, unsigned int label)
{
  size_t h;
  h = calc_hash(label,hash_size);
  return ks_lookup_label_list(hash[h],label);
}
static void set_particle_hash(CV_FRAME *fr, CV_PARTICLE *p, CV_PARTICLE **hash, int hash_size,
			      KS_LABEL_LIST **label_hash, int label_hash_size, 
			      KS_LABEL_LIST **reuse)
{
  size_t h;
  KS_LABEL_LIST *ll;

  if(lookup_cv_particle_hash_label(hash,hash_size,p->label) == NULL){
    h = calc_hash(p->label,hash_size);
    hash[h] = add_cv_particle_hash(hash[h],p);
  }

  h = calc_hash(p->file_label,label_hash_size);
  if((ll = ks_lookup_label_list(label_hash[h],p->file_label)) == NULL){
    label_hash[h]=ks_add_label_list(label_hash[h],new_label_list(p->file_label,p->label,
								 CV_LABEL_USE,reuse));
  } else if(ll->flags&CV_LABEL_USE){
    ll->flags |= CV_LABEL_DUPLICATE;
    label_hash[h]=ks_add_label_list(label_hash[h],new_label_list(p->file_label,p->label,
								 CV_LABEL_USE|
								 CV_LABEL_DUPLICATE,reuse));
  } else {
    ll->flags |= CV_LABEL_USE;
    ll->value = p->label;
  }
}
static void collect_label_list(KS_LABEL_LIST **label_hash, int label_hash_size, 
			       KS_LABEL_LIST **reuse)
{
  int i;
  KS_LABEL_LIST *p,*next,*prev;

  for(i = 0; i < label_hash_size; i++){
    prev = NULL;
    for(p = label_hash[i]; p != NULL; p = next){
      next = p->next;
      if(!(p->flags&CV_LABEL_USE)){
	/*	printf("NOT UST %d %d\n",p->label,p->value);*/
	if(prev == NULL)
	  label_hash[i] = p->next;
	else
	  prev->next = p->next;
	p->next = NULL;
	*reuse = ks_add_label_list(*reuse,p);
      } else {
	prev = p;
      }
    }
  }
}
static BOOL set_particle_hash_frame(CV_FRAME *fr, KS_LABEL_LIST **reuse,
				    CV_PARTICLE ***hash, int *hash_size,
				    KS_LABEL_LIST ***label_hash, int *label_hash_size)
{
  int i;
  CV_PARTICLE *p;

  if((*hash_size)*2 < count_cv_particle(fr->particle)){
    ks_free(*hash);
    (*hash) = NULL;
  }
  if((*hash) == NULL){
    *hash_size = count_cv_particle(fr->particle);
    if((*hash = (CV_PARTICLE**)ks_malloc((*hash_size)*sizeof(CV_PARTICLE*),
					 "particle_hash")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  for(i = 0; i < *hash_size; i++)
    (*hash)[i] = NULL;

  if((*label_hash_size)*2 < count_cv_particle(fr->particle)){
    for(i = 0; i < *label_hash_size; i++){
      if((*label_hash)[i] != NULL){
	ks_free_label_list((*label_hash)[i]);
      }
    }
    ks_free(*label_hash);
    (*label_hash) = NULL;
  }
  if((*label_hash) == NULL){
    *label_hash_size = count_cv_particle(fr->particle);
    if((*label_hash = (KS_LABEL_LIST**)ks_malloc((*label_hash_size)*sizeof(KS_LABEL_LIST*),
					       "particle_hash")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < *label_hash_size; i++)
      (*label_hash)[i] = NULL;
  }

  for(p = fr->particle; p != NULL; p = p->next){
    set_particle_hash(fr,p,*hash,*hash_size,*label_hash,*label_hash_size,reuse);
  }

  collect_label_list(*label_hash,*label_hash_size,reuse);

  /*
  for(i = 0; i < *hash_size; i++){
    printf("%d:",i);
    for(p = (*hash)[i]; p != NULL; p = p->hash_next){
      printf("(%d %s)",p->label,p->atom->name);
    }
    printf("\n");
  }
  {
    KS_LABEL_LIST *ll;
    for(i = 0; i < *label_hash_size; i++){
      printf("%d %p:",i,(*label_hash)[i]);
      for(ll = (*label_hash)[i]; ll != NULL; ll = ll->next){
	printf("(%d %d %c %c)",ll->label,ll->value,
	       ll->flags&CV_LABEL_USE?'o':'x',
	       ll->flags&CV_LABEL_DUPLICATE?'o':'x'
	       );
      }
      printf("\n");
    }
  }
  */

  return KS_TRUE;
}
static void clear_particle_hash_frame(CV_FRAME *fr, CV_PARTICLE **hash, int hash_size,
				      KS_LABEL_LIST **label_hash, int label_hash_size)
{
  int i;
  CV_PARTICLE *p;
  KS_LABEL_LIST *ll;
  for(p = fr->particle; p != NULL; p = p->next){
    p->hash_next = NULL;
  }
  for(i = 0; i < label_hash_size; i++){
    for(ll = label_hash[i]; ll != NULL; ll = ll->next){
      ll->flags = 0;
    }
  }
}
static BOOL set_residue_hash_frame(CV_FRAME *fr, KS_LABEL_LIST **reuse,
				   CV_RESIDUE ***hash, int *hash_size, 
				   KS_LABEL_LIST ***label_hash, int *label_hash_size)
{
  int i;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  size_t h;
  KS_LABEL_LIST *ll;
  int size = 0;

  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      size += count_cv_residue(pc->residue);
    }
  }
  if((*hash_size)*2 < size){
    ks_free(*hash);
    *hash = NULL;
  }
  if(*hash == NULL){
    *hash_size = size;
    if((*hash = (CV_RESIDUE**)ks_malloc((*hash_size)*sizeof(CV_RESIDUE*),"residue_hash")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  for(i = 0; i < *hash_size; i++)
    (*hash)[i] = NULL;

  if((*label_hash_size)*2 < size && *label_hash != NULL){
    for(i = 0; i < *label_hash_size; i++){
      if((*label_hash)[i] != NULL){
	ks_free((*label_hash)[i]);
      }
    }
    ks_free(*label_hash);
    *label_hash = NULL;
  }
  if(*label_hash == NULL){
    *label_hash_size = size;
    if((*label_hash = (KS_LABEL_LIST**)ks_malloc((*label_hash_size)*sizeof(KS_LABEL_LIST*),
						 "label_residue_hash")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < *hash_size; i++)
      (*label_hash)[i] = NULL;
  }

  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	h = calc_hash(pr->label,*hash_size);
	if(lookup_cv_residue_hash_label(*hash,*hash_size,pr->label)==NULL){
	  (*hash)[h] = add_cv_residue_hash((*hash)[h],pr);
	}
	h = calc_hash(pr->file_label,*label_hash_size);
	if((ll = ks_lookup_label_list((*label_hash)[h],pr->file_label)) == NULL){
	  (*label_hash)[h]=ks_add_label_list((*label_hash)[h],
					     new_label_list(pr->file_label,
							    pr->label,CV_LABEL_USE,reuse));
	} else if(ll->flags&CV_LABEL_USE){
	  ll->flags |= CV_LABEL_DUPLICATE;
	  (*label_hash)[h]=ks_add_label_list((*label_hash)[h],
					     new_label_list(pr->file_label,pr->label,
							    CV_LABEL_USE|
							    CV_LABEL_DUPLICATE,reuse));
	} else {
	  ll->flags |= CV_LABEL_USE;
	  ll->value = pr->label;
	}
      }
    }
  }

  collect_label_list(*label_hash,*label_hash_size,reuse);

  /*
    for(i = 0; i < fr->residue_hash_size; i++){
    printf("%d:",i);
    for(pr = fr->residue_hash[i]; pr != NULL; pr = pr->hash_next){
    printf("(%d %d %s)",pr->label,pr->file_label,pr->name);
    }
    printf("\n");
    }
  */
  /*
    {
    for(i = 0; i < fr->residue_label_hash_size; i++){
    printf("%d %p:",i,fr->residue_label_hash[i]);
    for(ll = fr->residue_label_hash[i]; ll != NULL; ll = ll->next){
    printf("(%d %d %c)",ll->label,ll->value,ll->flags&CV_LABEL_DUPLICATE?'o':'x');
    }
    printf("\n");
    }
    }
    ks_exit(EXIT_FAILURE);
  */
  return KS_TRUE;
}
static void clear_residue_hash_frame(CV_FRAME *fr, CV_RESIDUE **hash, int hash_size,
				     KS_LABEL_LIST **label_hash, int label_hash_size)
{
  int i;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  KS_LABEL_LIST *ll;
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	pr->hash_next = NULL;
      }
    }
  }
  for(i = 0; i < label_hash_size; i++){
    for(ll = label_hash[i]; ll != NULL; ll = ll->next){
      ll->flags = 0;
    }
  }
}
static CV_FRAME *allocate_frame(int frame_num, int atype_mode, KS_FILE_LINES **fip)
{
  CV_FRAME *fr;
  int i,j,k;
  int ix,iy,iz;

  if((fr = (CV_FRAME*)ks_malloc(frame_num*sizeof(CV_FRAME),"fr")) == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < frame_num; i++){
    fr[i].flags = 0;
    fr[i].file_name[0] = '\0';
    fr[i].file_name_fdv[0] = '\0';
    fr[i].file_path[0] = '\0';
    fr[i].particle_label_max = 0;
    fr[i].particle_file_label_max = 0;
    fr[i].residue_label_max = 0;
    fr[i].chain_label_max = 0;
    fr[i].aggregate_label_max = 0;
    fr[i].particle = NULL;
    fr[i].particle_end = NULL;
    fr[i].periodic_p = NULL;
    fr[i].periodic_p_num = NULL;
    fr[i].aggregate = NULL;
    for(j = 0; j < 3; j++)
      fr[i].fit_trans[j] = 0;
    for(j = 0; j < 4; j++){
      for(k = 0; k < 4; k++){
	if(j == k){
	  fr[i].fit_rot[j*4+k] = 1.0;
	} else {
	  fr[i].fit_rot[j*4+k] = 0.;
	}
      }
    }

    fr[i].time = 0;
    fr[i].have_time = KS_FALSE;
    /*
    for(j = 0; j < 3; j++){
      fr[i].side[KS_RANGE_MIN][j] = 0;
      fr[i].side[KS_RANGE_MAX][j] = 0;
    }
    */
    for(ix = 0; ix < 2; ix++){
      for(iy = 0; iy < 2; iy++){
	for(iz = 0; iz < 2; iz++){
	  fr[i].side[ix][iy][iz][0] = 0;
	  fr[i].side[ix][iy][iz][1] = 0;
	  fr[i].side[ix][iy][iz][2] = 0;
	}
      }
    }
    fr[i].side_radius = -1;
    fr[i].have_side = KS_FALSE;
    fr[i].rmsd = 0;
    fr[i].bond_file_name = NULL;
    fr[i].face_file_name = NULL;
    fr[i].pdb_ext = KS_FALSE;
    fr[i].unknown_residue_label_max = 0;
    fr[i].unknown_residue_label = NULL;
    fr[i].have_solvent_exclude_surface = KS_FALSE;
    for(j = 0; j < CV_USER_TEXT_MAX; j++){
      fr[i].user_text[j].text = NULL;
    }
    fr[i].cdv_bond = NULL;
    fr[i].cdv_face = NULL;
    fr[i].fv = NULL;
    fr[i].sur = NULL;
    fr[i].color_map = NULL;
    fr[i].prep_block = NULL;
    fr[i].atype_mode = atype_mode;
    fr[i].c_color_info.start_color = NULL;
    fr[i].c_color_info.end_color = NULL;
    fr[i].c_color_info.set_min_max = KS_FALSE;
    fr[i].c_color_info.map = NULL;
    fr[i].pdb_user_bond = NULL;
    fr[i].fip = fip;
  }
  return fr;
}
static BOOL is_same_series_name(char *file_name, KS_DIR_DATA dir_data, char *name, int name_size,
				BOOL *same)
{
  int i,j;

  if(strstr(file_name,"/") != NULL || strstr(file_name,"\\") != NULL){
    for(i = (int)strlen(file_name); i != 0 && file_name[i] != '/' && file_name[i] != '\\'; i--);
    i++;
    /*    printf("%s\n",&file_name[i]);*/
    if(strlen(ks_get_dir_file_name(dir_data))+i+1 > name_size){
      ks_error("file name is too long");
      return KS_FALSE;
    }
    for(j = 0; j < i; j++)
      name[j] = file_name[j];
    name[j] = '\0';
    /*    printf("name = '%s'\n",name);*/
    strcat(name,ks_get_dir_file_name(dir_data));
  } else {
    if(strlen(ks_get_dir_file_name(dir_data)) > name_size){
      ks_error("file name is too long");
      return KS_FALSE;
    }
    strcpy(name,ks_get_dir_file_name(dir_data));
  }

  /*  printf("'%s' '%s'\n",file_name,name);*/

  if(strcmp(file_name,name) == 0){
    *same = KS_TRUE;
    /*    printf("same\n");*/
    return KS_TRUE;
  } else {
    *same = KS_FALSE;
  }
  for(i = 0; file_name[i]; i++){
    if(name[i] == '\0') return KS_FALSE;
    if(!isdigit(((unsigned char*)file_name)[i]) && ((unsigned char*)file_name)[i] != '-'){
      if(isdigit(((unsigned char*)name)[i])) return KS_FALSE;
      if(file_name[i] != name[i]) return KS_FALSE;
    }
  }
  if(name[i] != '\0') return KS_FALSE;
  /*  printf("KS_TRUE\n");*/
  return KS_TRUE;
}
int comp_frame(const CV_FRAME *fr0, const CV_FRAME *fr1)
{
  int i;
  for(i = 0; fr0->file_name[i] == fr1->file_name[i]; i++);
  return fr0->file_name[i] - fr1->file_name[i];
}
int comp_frame_fdv(const CV_FRAME *fr0, const CV_FRAME *fr1)
{
  int i;
  for(i = 0; fr0->file_name_fdv[i] == fr1->file_name_fdv[i]; i++);
  return fr0->file_name_fdv[i] - fr1->file_name_fdv[i];
}
static void sort_frame(CV_BASE *cb)
{
  qsort(cb->frame,cb->frame_num,sizeof(CV_FRAME),(int (*)(const void*, const void*))comp_frame);
}
static void sort_frame_fdv(CV_BASE *cb)
{
  qsort(cb->frame,cb->frame_num,sizeof(CV_FRAME),(int (*)(const void*, const void*))comp_frame_fdv);
}
static BOOL get_dir_file(CV_BASE *cb, char *file_name)
{ 
  int i;
  int path_end;
  KS_DIR_HANDLE dir_handle;
  KS_DIR_DATA dir_data;
  char path[256];
  char name[256];
  BOOL same;
  BOOL fdv_flg = KS_FALSE;

  /*  printf("%s\n",file_name);*/
  if(strstr(file_name,"/") == NULL && strstr(file_name,"\\") == NULL){
    strcpy(path,".");
  } else {
    for(i = 0; file_name[i]; i++){
      if(file_name[i] == '/' || file_name[i] == '\\'){
	path_end = i;
      }
    }
    for(i = 0; i < path_end && i < sizeof(path); i++){
      path[i] = file_name[i];
    }
    path[i] = '\0';
  }
  /*  printf("path '%s'\n",path);*/
  cb->frame_num = 0;
  if(ks_read_dir_first(path,&dir_handle, &dir_data) == KS_FALSE){
    ks_error("open dir error %s",path);
    return KS_FALSE;
  }
  do{
    /*    printf("%s\n",ks_get_dir_file_name(dir_data));*/
    if(is_same_series_name(file_name,dir_data,name,sizeof(name),&same) == KS_TRUE){
      /*      printf("%s\n",ks_get_dir_file_name(dir_data));*/
      cb->frame_num++;
    }
  }while(ks_read_dir_next(dir_handle,&dir_data) != KS_FALSE);

  ks_close_dir(dir_handle);

  if((cb->frame = allocate_frame(cb->frame_num,cb->atype_mode,&cb->fi)) == NULL){
    return KS_FALSE;
  }

  cb->frame_num = 0;
  if(ks_read_dir_first(path,&dir_handle, &dir_data) == KS_FALSE){
    ks_error("open dir error %s",path);
    return KS_FALSE;
  }
  do{
    if(is_same_series_name(file_name,dir_data,name,sizeof(name),&same) == KS_TRUE){
      /*      printf("%s %s %d\n",file_name,ks_get_dir_file_name(dir_data),same);*/
      if(same == KS_TRUE)
	cb->frame[cb->frame_num].flags |= CV_FRAME_REFERENCE;
      if(strlen(name) > CV_FRAME_FILE_NAME_MAX){
	ks_error("file name %s is too long",name);
	return KS_FALSE;
      }
      if(strstr(name,".fdv") != NULL || strstr(name,".FDV") != NULL){
	fdv_flg = KS_TRUE;
	strcpy(cb->frame[cb->frame_num].file_name_fdv,name);
      } else {
	strcpy(cb->frame[cb->frame_num].file_name,name);
      }
      cb->frame_num++;
    }
  }while(ks_read_dir_next(dir_handle,&dir_data) != KS_FALSE);

  ks_close_dir(dir_handle);

  if(fdv_flg == KS_TRUE){
    sort_frame_fdv(cb);
  } else {
    sort_frame(cb);
  }
  /*
  for(i = 0; i < cb->frame_num; i++){
    printf("%d %s %d\n",i,cb->frame[i].file_name,cb->frame[i].flags);
  }
  ks_exit(EXIT_FAILURE);
  */
  return KS_TRUE;
}
static BOOL fit_frame(CV_FRAME *ref, CV_FRAME *fr)
{
  int i,j,k;
  const int n = 6;
  double **a, **u, **b, **c;
  double *d, *work;
  //  double mu[3][3];
  double mh[3][3],mk[3][3],mr[3][3];
  CV_PARTICLE *p,*q;
  double d0;

  /*
  printf("fit (%f %f %f) (%f %f %f)\n"
	 ,ref->center[0],ref->center[1],ref->center[2]
	 ,fr->center[0],fr->center[1],fr->center[2]);
  */
  for(i = 0; i < 3; i++)
    fr->fit_trans[i] = ref->center[i] - fr->center[i];

  if(count_cv_particle(ref->particle) == count_cv_particle(fr->particle)){

    if((a = ks_malloc_double_pp(n,n,"a")) == NULL){
      return KS_FALSE;
    }
    if((b = ks_malloc_double_pp(n,n,"b")) == NULL){
      return KS_FALSE;
    }
    if((c = ks_malloc_double_pp(n,n,"c")) == NULL){
      return KS_FALSE;
    }
    if((u = ks_malloc_double_pp(n,n,"u")) == NULL){
      return KS_FALSE;
    }
    if((d = ks_malloc_double_p(n,"d")) == NULL){
      return KS_FALSE;
    }
    if((work = ks_malloc_double_p(n,"work")) == NULL){
      return KS_FALSE;
    }

    for(i = 0; i < n; i++)
      for(j = 0; j < n; j++)
	a[i][j] = 0;

    for(p = ref->particle, q = fr->particle; p != NULL && q != NULL; p = p->next, q = q->next){
      /*      printf("%d %s %f\n",p->label,p->atom->name,p->atom->mass);*/
      for(i = 0; i < 3; i++){
	for(j = 0; j < 3; j++){
	  a[i][j+3] += p->atom->mass*(p->cd[i]-ref->center[i])*(q->cd[j]-fr->center[j]);
	}
      }
    }
    /*
    for(i = 0; i < 3; i++){
      for(j = 0; j < 3; j++){
	mu[i][j] = a[i][j+3];
      }
    }
    */
    for(i = 0; i < n; i++){
      for(j = 0; j < i; j++){
	a[i][j] = a[j][i];
      }
    }
    for(i = 0; i < n; i++){
      for(j = 0; j < n; j++){
	u[i][j] = a[i][j];
      }
    }

    if(ks_diagonalize(n, u, d, work) == KS_FALSE){
      return KS_FALSE;
    }
    for(i = 0; i < n; i++){
      for(j = 0; j < n; j++){
	b[i][j] = u[i][j];
      }
    }
    for(i = 0; i < n; i++){
      for(j = 0; j < n; j++){
	u[i][j] = b[j][i];
      }
    }

    for(i = 0; i < 3; i++){
      for(j = 0; j < 3; j++){
	mk[j][i] = u[i][j];
      }
    }
    for(i = 0; i < 3; i++){
      for(j = 0; j < 3; j++){
	mh[j][i] = u[i+3][j];
      }
    }

    for(i = 0; i < 3; i++){
      d0 = sqrt(pow(mk[i][0],2)+pow(mk[i][1],2)+pow(mk[i][2],2));
      mk[i][0] /= d0; mk[i][1] /= d0; mk[i][2] /= d0;
    }
    for(i = 0; i < 3; i++){
      d0 = sqrt(pow(mh[i][0],2)+pow(mh[i][1],2)+pow(mh[i][2],2));
      mh[i][0] /= d0; mh[i][1] /= d0; mh[i][2] /= d0;
    }
    for(i = 0; i < 3; i++){
      for(j = 0; j < 3; j++){
	mr[i][j] = 0;
	for(k = 0; k < 3; k++){
	  mr[i][j] += mk[k][i]*mh[k][j]*(k == 2 ? 1:1);
	}
      }
    }
    /*
    for(i = 0; i < 3; i++){
      for(j = 0; j < 3; j++){
	fr->fit_rot[i][j] = mr[i][j];
      }
    }
    */
    /*
    for(i = 0; i < 3; i++){
      for(j = 0; j < 3; j++){
	printf(" %f",mr[i][j]);
      }
      printf("\n");
    }
    */

    fr->fit_rot[0] = mr[0][0]; fr->fit_rot[4] = mr[0][1]; fr->fit_rot[8]  = mr[0][2];
    fr->fit_rot[1] = mr[1][0]; fr->fit_rot[5] = mr[1][1]; fr->fit_rot[9]  = mr[1][2];
    fr->fit_rot[2] = mr[2][0]; fr->fit_rot[6] = mr[2][1]; fr->fit_rot[10] = mr[2][2];

    ks_free_double_pp(n,a);
    ks_free_double_pp(n,b);
    ks_free_double_pp(n,c);
    ks_free_double_pp(n,u);
    ks_free(d);
    ks_free(work);

  } else {
    for(i = 0; i < 4; i++){
      for(j = 0; j < 4; j++){
	if(i == j)
	  fr->fit_rot[i*4+j] = 1.0;
	else
	  fr->fit_rot[i*4+j] = 0.0;
      }
    }
  }

  return KS_TRUE;
}
static void set_particle_end(CV_FRAME *fr)
{
  CV_PARTICLE *p;
  if(fr->particle == NULL){
    fr->particle_end = NULL;
  } else {
    for(p = fr->particle; p->next != NULL; p = p->next);
    fr->particle_end = p;
  }
}
static BOOL read_fdv_frame(CV_FRAME *fr, FV_TARGET *fv_target, 
			   double ***fv_level, int fv_level_num)
{
  printf("read fdv %s\n",fr->file_name_fdv);
  fflush(stdout);

  if(fv_read_field_file(fr->file_name_fdv,&fr->fv->field,&fr->fv->field_size,
			fr->file_name) == KS_FALSE){
    return KS_FALSE;
  }

  if(fr->file_name[0] != '\0'){
    fr->flags |= CV_FRAME_CDV;
  }

  fv_set_max_min(fr->fv->field,&fr->fv->field_size,fv_target->val);
  /*  fr->fv->field_size.max = 0;*/
  if(fv_level[KS_RANGE_MAX][fv_target->val] != NULL){
    fr->fv->field_size.max = *fv_level[KS_RANGE_MAX][fv_target->val];
  }
  if(fv_level[KS_RANGE_MIN][fv_target->val] != NULL){
    fr->fv->field_size.min = *fv_level[KS_RANGE_MIN][fv_target->val];
  }
  if(fv_init_field(fr->fv->field,fr->fv->field_size,&fr->fv->contour_size,&fr->fv->contour_fan,
		   fv_target,fv_level_num,fr->fv->black_and_white) == KS_FALSE){
    /*    fv_debug_mode(0);*/
    //    return KS_FALSE;
  }
  /*  fr->fv_target.level[3] = 1;*/

  return KS_TRUE;
}
static BOOL init_frame(CV_FRAME *fr, FV_BASE *fv, CV_BASE *cb)
{
  static BOOL first = KS_TRUE;
  fr->fv = fv;
  /*
  if(fr->fv != NULL){
    fv_reconstruct_contour(fv);
  }
  */
  fr->reuse = &cb->reuse_base;
  /*
  printf("init_frame read %c pdb %c prep %c fdv %c cdv %c\n",
	 fr->flags&CV_FRAME_READ ? 'o':'x',
	 fr->flags&CV_FRAME_PDB  ? 'o':'x',
	 fr->flags&CV_FRAME_PREP  ? 'o':'x',
	 fr->flags&CV_FRAME_FDV ? 'o':'x',
	 fr->flags&CV_FRAME_CDV  ? 'o':'x');
  */

  if(fr->flags&CV_FRAME_READ)
    return KS_TRUE;

  if(fr->flags&CV_FRAME_PDB){
    if(read_pdb_frame(fr,cb)
       == KS_FALSE){
      return KS_FALSE;
    }
  } 
  if(fr->flags&CV_FRAME_PREP){
    if(read_prep_frame(fr,&cb->atom,cb->atype_use) == KS_FALSE){
      return KS_FALSE;
    }
  }
  if(fr->flags&CV_FRAME_FDV){
    if(read_fdv_frame(fr,&cb->fv_target,cb->fv_level,cb->fv_level_num) == KS_FALSE){
      return KS_FALSE;
    }
  } 
  if(fr->flags&CV_FRAME_CDV){
    if(read_cdv_frame(fr,cb) == KS_FALSE){
      return KS_FALSE;
    }
    /*    printf("particle_label_max %d\n",fr->particle_label_max);*/
    if(first == KS_TRUE){
      if(fr->particle_label_max > 1000){
	cb->detail = CV_SPHERE_DETAIL_MODEST;
      } else if(fr->particle_label_max > 10000){
	cb->detail = CV_SPHERE_DETAIL_LOW;
      }
    }
    if(fr->have_side == KS_FALSE){
      cb->kabe_mode = CV_KABE_MODE_OFF;
    }
  }

  fr->particle_cnt = count_cv_particle(fr->particle);
  set_particle_end(fr);
  /*  printf("%d\n",count_cv_particle(fr->particle));*/
  set_center_and_size_frame(fr,&cb->kabe_mode);
  set_particle_color_frame(fr);
  set_second_structure_frame(fr);
  if(!(fr->flags&CV_FRAME_PREP)){
    set_pdb_bond_frame(fr);
  }
  set_color_sample(fr,&cb->sphere_texture,&cb->atom,cb->color_sample,fr->prep_block != NULL);
  if(!(fr->flags&CV_FRAME_REFERENCE)){
    fit_frame(cb->reference_frame,fr);
  }

#ifdef USE_GLSL
  if(first == KS_TRUE){
    if(init_shader_sphere_draw_info(cb,fr) == KS_FALSE){
      return KS_FALSE;
    }
    if(init_system_box_draw_info(cb) == KS_FALSE){
      return KS_FALSE;
    }
  }
#endif
  /*
  printf("%d\n",ks_output_memory_infomation(stdout,0));
  */

  first = KS_FALSE;

  return KS_TRUE;
}
BOOL set_hash(CV_FRAME *fr, KS_LABEL_LIST **reuse,
	      CV_PARTICLE ***particle_hash, int *particle_hash_size,
	      KS_LABEL_LIST ***particle_label_hash, int *particle_label_hash_size,
	      CV_RESIDUE ***residue_hash, int *residue_hash_size,
	      KS_LABEL_LIST ***residue_label_hash, int *residue_label_hash_size)
{
  if(set_particle_hash_frame(fr,reuse,
			     particle_hash,particle_hash_size,
			     particle_label_hash,particle_label_hash_size) == KS_FALSE){
    return KS_FALSE;
  }
  if(set_residue_hash_frame(fr,reuse,
			    residue_hash,residue_hash_size,
			    residue_label_hash,residue_label_hash_size) == KS_FALSE){
    return KS_FALSE;
  }
  return KS_TRUE;
}
void clear_hash(CV_FRAME *fr,
	      CV_PARTICLE **particle_hash, int particle_hash_size,
	      KS_LABEL_LIST **particle_label_hash, int particle_label_hash_size,
	      CV_RESIDUE **residue_hash, int residue_hash_size,
	      KS_LABEL_LIST **residue_label_hash, int residue_label_hash_size)
{
  clear_particle_hash_frame(fr,
			    particle_hash,particle_hash_size,
			    particle_label_hash,particle_label_hash_size);
  clear_residue_hash_frame(fr,
			   residue_hash,residue_hash_size,
			   residue_label_hash,residue_label_hash_size);
}
static void set_command(CV_BASE *cb, int n, char *key1, char *key2, 
			void (*func)(KS_GL_BASE*,CV_BASE*,char*), char *comment)
{
  ks_strncpy(cb->command[n].key[CV_COMMAND_KEY_1],key1,
	     sizeof(cb->command[n].key[CV_COMMAND_KEY_1]));
  ks_strncpy(cb->command[n].key[CV_COMMAND_KEY_2],key2,
	     sizeof(cb->command[n].key[CV_COMMAND_KEY_2]));
  ks_strncpy(cb->command[n].comment,comment,sizeof(cb->command[n].comment));
  cb->command[n].process_command = func;
}
char *get_command_arg(CV_BASE *cb, int command,  char *text)
{
  char *c;
  if(strncmp(text,cb->command[command].key[CV_COMMAND_KEY_1],
	     strlen(cb->command[command].key[CV_COMMAND_KEY_1])) == 0){
    c = strstr(text,cb->command[command].key[CV_COMMAND_KEY_1]);
    c += strlen(cb->command[command].key[CV_COMMAND_KEY_1])+1;
  } else if(strncmp(text,cb->command[command].key[CV_COMMAND_KEY_2],
		    strlen(cb->command[command].key[CV_COMMAND_KEY_2])) == 0){
    c = strstr(text,cb->command[command].key[CV_COMMAND_KEY_2]);
    c += strlen(cb->command[command].key[CV_COMMAND_KEY_2])+1;
  } else {
    c = text;
  }
  return c;
}
static void set_skip_frame(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  int i;
  char *c;
  if(strncmp(text,cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_1],
	     strlen(cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_1])) == 0){
    c = strstr(text,cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_1]);
    c += strlen(cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_1])+1;
  } else if(strncmp(text,cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_2],
		    strlen(cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_2])) == 0){
    c = strstr(text,cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_2]);
    c += strlen(cb->command[CV_COMMAND_SET_SKIP].key[CV_COMMAND_KEY_2])+1;
  } else {
    c = text;
  }
  /*  printf("'%s' '%s'\n",text,c);*/
  cb->skip_frame = atoi(c);
  /*
  printf("%d ref %d %d\n",cb->skip_frame
	 ,cb->reference_frame->label,cb->reference_frame->label%cb->skip_frame);
  */
  if(cb->skip_frame != 1){
    for(i = cb->reference_frame->label; i < cb->frame_num; i++){
      if((i-cb->reference_frame->label)%cb->skip_frame == 0){
	/*      printf("%d o\n",i);*/
	cb->frame[i].flags |= CV_FRAME_ENABLE;
      } else {
	/*      printf("%d x\n",i);*/
	cb->frame[i].flags &= ~CV_FRAME_ENABLE;
      }
    }
  }
}
static void set_time_unit2(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  char *c = text;
  cb = (CV_BASE*)vp;
  if(strcmp(c,"fs") == 0){
    cb->draw_time_unit_type = CV_DRAW_TIME_UNIT_FS;
  } else if(strcmp(c,"ps") == 0){
    cb->draw_time_unit_type = CV_DRAW_TIME_UNIT_PS;
  } else if(strcmp(c,"ns") == 0){
    cb->draw_time_unit_type = CV_DRAW_TIME_UNIT_NS;
  }
}
static void set_time_unit(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  ks_set_gl_input_key(gb,"input unit (fs/ps/ns) > ",NULL,cb->input_key_font,set_time_unit2,
		      KS_FALSE,cb);
}
static void set_time_offset2(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  cb->time_offset = atof(text);
}
static void set_time_offset(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  ks_set_gl_input_key(gb,"input offset (s) > ",NULL,cb->input_key_font,set_time_offset2,
		      KS_FALSE,cb);
}
static void set_show_flags(unsigned int *show_flags, unsigned int flag, BOOL condition)
{
  if(condition == KS_TRUE){
    *show_flags |= flag;
  } else {
    *show_flags &= ~flag;
  }
}
static void show_line(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  set_show_flags(&cb->show_flags,CV_SHOW_LINE,KS_TRUE);
}
static void hide_line(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  set_show_flags(&cb->show_flags,CV_SHOW_LINE,KS_FALSE);
}
static void show_particle(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  set_show_flags(&cb->show_flags,CV_SHOW_PARTICLE,KS_TRUE);
}
static void hide_particle(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  set_show_flags(&cb->show_flags,CV_SHOW_PARTICLE,KS_FALSE);
}
static BOOL malloc_cv_solvent_exclude_surface_particle(CV_SOLVENT_EXCLUDE_SURFACE *ses)
{
  if((ses->pl = (CV_PARTICLE**)ks_malloc(sizeof(CV_PARTICLE*)*ses->pl_num,"ses->pl")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  return KS_TRUE;
}
static void free_cv_solvent_exclude_surface_particle(CV_SOLVENT_EXCLUDE_SURFACE *ses)
{
  ks_free(ses->pl); ses->pl = NULL;
}
static BOOL malloc_cv_solvent_exclude_surface_vertex(CV_SOLVENT_EXCLUDE_SURFACE *ses)
{
  if((ses->pl_no = (int*)ks_malloc(sizeof(int)*ses->vertex_num,"ses->pl_no")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  if((ses->vertex = (double*)ks_malloc(sizeof(double)*ses->vertex_num*3,"ses->vertex")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  if((ses->normal = (double*)ks_malloc(sizeof(double)*ses->vertex_num*3,"ses->vertex")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  return KS_TRUE;
}
static void free_cv_solvent_exclude_surface_vertex(CV_SOLVENT_EXCLUDE_SURFACE *ses)
{
  ks_free(ses->pl_no); ses->pl_no = NULL;
  ks_free(ses->vertex); ses->vertex = NULL;
  ks_free(ses->normal); ses->normal = NULL;
}
static BOOL malloc_cv_solvent_exclude_surface_face(CV_SOLVENT_EXCLUDE_SURFACE *ses)
{
  if((ses->face = (int*)ks_malloc(sizeof(int)*ses->face_num*4,"ses->face")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  return KS_TRUE;
}
static void free_cv_solvent_exclude_surface_face(CV_SOLVENT_EXCLUDE_SURFACE *ses)
{
  ks_free(ses->face); ses->face = NULL;
}
static void calc_solvent_exclude_surface(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  int i,ix3,ix4;
  int i1,i2;
  char c0[256];
  double d0,d1;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  CV_PARTICLE *p;
  int n;
  FILE *fp;

  cb->draw_mode = CV_DRAW_MODE_SOLVENT_EXCLUDE_SURFACE;
  cb->frame[cb->current_frame].have_solvent_exclude_surface = KS_TRUE;
  for(pa = cb->frame[cb->current_frame].aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      if(pc->len > 1){
	n = 0;
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  for(i = 0; i < pr->particle_num; i++){
	    p = pr->particle[i];
	    if(p->atom->name[0] != 'H'){
	      /*
	      printf("%d %s %f %f %f %f\n",p->label,p->atom->name,
		     ks_get_gl_atom_size(p->atom->gl_atom,KS_GL_ATOM_SIZE_NORMAL),
		     p->cd[0],p->cd[1],p->cd[2]);
	      */
	      n++;
	    }
	  }
	}
	if(pc->ses == NULL){
	  if((pc->ses = (CV_SOLVENT_EXCLUDE_SURFACE*)ks_malloc(sizeof(CV_SOLVENT_EXCLUDE_SURFACE),
							       "pc->ses")) == NULL){
	    ks_error_memory();
	  }
	  pc->ses->vertex_num = 0;
	  pc->ses->vertex = NULL;
	  pc->ses->normal = NULL;
	  pc->ses->face_num = 0;
	  pc->ses->face = NULL;
	  pc->ses->pl_num = 0;
	  pc->ses->pl = NULL;
	  pc->ses->pl_no = NULL;
	}
	if(pc->ses->pl_num != n){
	  if(pc->ses->pl != NULL){
	    free_cv_solvent_exclude_surface_particle(pc->ses);
	  }
	  pc->ses->pl_num = n;
	  malloc_cv_solvent_exclude_surface_particle(pc->ses);
	}
      }
    }
  }
  for(pa = cb->frame[cb->current_frame].aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      if(pc->len > 1){
	fp = fopen("tmp.xyzr","wt");
	n = 0;
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  for(i = 0; i < pr->particle_num; i++){
	    p = pr->particle[i];
	    if(p->atom->name[0] != 'H'){
	      fprintf(fp,"%f %f %f %f\n",
		     p->cd[0],p->cd[1],p->cd[2],
		     ks_get_gl_atom_size(p->atom->gl_atom,KS_GL_ATOM_SIZE_LARGE));
	      pc->ses->pl[n++] = p;
	    }
	  }
	}
	fclose(fp);
	system("C:/cygwin/home/koishi/programs/cdview3/msms_win32_6.2.1/msms.exe"
	/*
	system("/home/koishi/programs/cdview3/msms_win32_6.2.1/msms.exe"
	*/
	       " -density 2 -if tmp.xyzr -of tmp > tmp.out");
	fp = fopen("tmp.vert","rt");
	while(fgets(c0,sizeof(c0),fp) != NULL)
	  if(c0[0] != '#') break;
	/*    printf("%s",c0);*/
	sscanf(c0,"%d %d %lf %lf",&n,&i1,&d0,&d1);
	if(pc->ses->vertex_num != n){
	  if(pc->ses->vertex != NULL){
	    free_cv_solvent_exclude_surface_vertex(pc->ses);
	  }
	  pc->ses->vertex_num = n;
	  malloc_cv_solvent_exclude_surface_vertex(pc->ses);
	}
	for(i = 0; i < pc->ses->vertex_num; i++){
	  ix3 = i*3;
	  fscanf(fp,"%lf %lf %lf %lf %lf %lf %d %d %d"
		 ,&pc->ses->vertex[ix3],&pc->ses->vertex[ix3+1],&pc->ses->vertex[ix3+2]
		 ,&pc->ses->normal[ix3],&pc->ses->normal[ix3+1],&pc->ses->normal[ix3+2]
		 ,&i1,&pc->ses->pl_no[i],&i2);
	  /*
	  printf("%d (%f %f %f) (%f %f %f) %d\n",i,
		 pc->ses->vertex[ix3],pc->ses->vertex[ix3+1],pc->ses->vertex[ix3+2],
		 pc->ses->normal[ix3],pc->ses->normal[ix3+1],pc->ses->normal[ix3+2],
		 pc->ses->pl_no[i]);
	  */
	  pc->ses->pl_no[i]--;
	}
	fclose(fp);

	fp = fopen("tmp.face","rt");
	while(fgets(c0,sizeof(c0),fp) != NULL)
	  if(c0[0] != '#') break;
	sscanf(c0,"%d %d %lf %lf",&n,&i1,&d0,&d1);
	if(pc->ses->face_num != n){
	  if(pc->ses->face != NULL){
	    free_cv_solvent_exclude_surface_face(pc->ses);
	  }
	  pc->ses->face_num = n;
	  malloc_cv_solvent_exclude_surface_face(pc->ses);
	}
	for(i = 0; i < pc->ses->face_num; i++){
	  ix4 = i*4;
	  fscanf(fp,"%d %d %d %d %d"
		 ,&pc->ses->face[ix4]  ,&pc->ses->face[ix4+1]
		 ,&pc->ses->face[ix4+2],&pc->ses->face[ix4+3],&i1);
	  /*
	  printf("%d (%d %d %d %d)\n",i,
		 pc->ses->face[ix4],pc->ses->face[ix4+1],
		 pc->ses->face[ix4+2],pc->ses->face[ix4+3]);
	  */
	  pc->ses->face[ix4]--;
	  pc->ses->face[ix4+1]--;
	  pc->ses->face[ix4+2]--;
	}
	fclose(fp);
      }
    }
  }
  remove("tmp.xyzr");
  remove("tmp.face");
  remove("tmp.vert");
  remove("tmp.out");
}
static void calc_solvent_exclude_surface_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      calc_solvent_exclude_surface(gb,cb,NULL);
      return;
    }
  }
  cb->idle = NULL;
}
static void set_calc_solvent_exclude_surface_idle(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  first_frame(gb,0,0,(void*)cb);
  calc_solvent_exclude_surface(gb,cb,text);
  cb->idle = calc_solvent_exclude_surface_idle;
}
static void draw_solvent_exclude_surface(CV_FRAME *fr, int color_mode, BOOL bright_low)
{
  int i,j;
  int ix3,ix4;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  //  CV_RESIDUE *pr;
  //  CV_PARTICLE *p;
  GLfloat color[4] = {1,1,1,1};

  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      if(pc->ses != NULL){
	for(i = 0; i < pc->ses->face_num; i++){
	  ix4 = i*4;
	  glBegin(GL_TRIANGLES);
	  for(j = 0; j < 3; j++){

	    get_color(pc->ses->pl[pc->ses->pl_no[pc->ses->face[ix4+j]]],color_mode,
		      (bright_low == KS_TRUE) ? KS_GL_ATOM_BRIGHT_LOW:KS_GL_ATOM_BRIGHT_NORMAL,
		      color,KS_FALSE);

	    ix3 = pc->ses->face[ix4+j]*3;
	    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
	    glNormal3d(pc->ses->normal[ix3],pc->ses->normal[ix3+1],pc->ses->normal[ix3+2]);
	    glVertex3d(pc->ses->vertex[ix3],pc->ses->vertex[ix3+1],pc->ses->vertex[ix3+2]);
	  }
	  glEnd();
	}
      }
    }
  }
}
static void process_text_window_mouse(KS_GL_BASE *gb, int id, int width, int height, int button, 
				      int state, int x, int y, void *vp)
{
  CV_BASE *cb;
  KS_CHAR_LIST *p;
  cb = (CV_BASE*)vp;
  if(button == KS_GL_MOUSE_BUTTON_LEFT && state == KS_GL_MOUSE_STATE_UP){
    /*
    printf("%d\n",ks_is_gl_double_click(gb,KS_GL_MOUSE_BUTTON_LEFT,KS_GL_MOUSE_STATE_UP));
    */
    if(ks_is_gl_double_click(gb,KS_GL_MOUSE_BUTTON_LEFT,KS_GL_MOUSE_STATE_UP) == KS_TRUE){
      for(p = cb->text_window; p != NULL; p = p->next){
	/*    printf("%d %s %d %d\n",p->value,p->name,id,y);*/
	if(id == p->value){
	  ks_toggle_gl_base_window_frame(gb,id);
	}
      }
    }
  }
}
static void process_text_window_display(KS_GL_BASE *gb, int id, int width, int height, void *vp)
{
  CV_BASE *cb;
  KS_CHAR_LIST *p;
  int font_height;
  int y;

  cb = (CV_BASE*)vp;
  ks_get_glut_font_height(gb,cb->text_window_font,&font_height);
  y = (height - font_height)*.5+3;
  /*  printf("%d %d %d\n",height,font_height,y);*/
  glRasterPos2i(2,y);
  for(p = cb->text_window; p != NULL; p = p->next){
    /*    printf("%d %s %d %d\n",p->value,p->name,id,y);*/
    if(id == p->value){
      ks_draw_glut_text(gb,p->name,cb->text_window_font,1.0,1);
    }
  }
}
static void add_text(KS_GL_BASE *gb, char *text, void *vp)
{
  KS_CHAR_LIST *p;
  int width,height;
  CV_BASE *cb;

  cb = (CV_BASE*)vp;
  p = ks_new_char_list(text,0);

  ks_get_glut_font_width(gb,cb->text_window_font,text,&width);
  width += 4;
  ks_get_glut_font_height(gb,cb->text_window_font,&height);
  height += 4;

  p->value = ks_add_gl_base_window(gb,
				   KS_GL_WINDOW_SHOW|KS_GL_WINDOW_FRAME_FIX,
				   "TEXT",
				   ks_get_gl_viewport_2d_width(gb)*.5,
				   ks_get_gl_viewport_2d_height(gb)*.5,
				   width,height,
				   process_text_window_display,
				   process_text_window_mouse,NULL,NULL,cb);
  cb->text_window = ks_add_char_list(cb->text_window,p);
}
static void set_add_text(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *c;
  if(strncmp(text,cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_1],
	     strlen(cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_1])) == 0){
    c = strstr(text,cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_1]);
    c += strlen(cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_1])+1;
  } else if(strncmp(text,cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_2],
		    strlen(cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_2])) == 0){
    c = strstr(text,cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_2]);
    c += strlen(cb->command[CV_COMMAND_ADD_TEXT].key[CV_COMMAND_KEY_2])+1;
  } else {
    c = text;
  }
  if(strcmp(c,"r24") == 0){
    cb->text_window_font = KS_GL_FONT_TIMES_ROMAN_24;
  } else if(strcmp(c,"r10") == 0){
    cb->text_window_font = KS_GL_FONT_TIMES_ROMAN_10;
  } else if(strcmp(c,"h18") == 0){
    cb->text_window_font = KS_GL_FONT_HELVETICA_18;
  } else if(strcmp(c,"h12") == 0){
    cb->text_window_font = KS_GL_FONT_HELVETICA_12;
  } else if(strcmp(c,"h10") == 0){
    cb->text_window_font = KS_GL_FONT_HELVETICA_10;
  } else {
    cb->text_window_font = KS_GL_FONT_TIMES_ROMAN_24;
  }
  ks_set_gl_input_key(gb,"add_text > ",NULL,cb->input_key_font,add_text,KS_FALSE,cb);
}
static void change_output_bmp(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  cb->output_file_mode = CV_OUTPUT_PICTURE_BMP;
}
static void set_pov_info(CV_BASE *cb, char *text)
{
  int i,j;
  char *cp;
  char str[64];
  printf("text '%s'\n",text);
  cp = text;
  for(i = 0; i < 2; i++){
    for(; *cp == ' '; cp++);
    for(j = 0; j < sizeof(str) && *cp != ' ' && *cp != '\n'; j++, cp++){
      str[j] = *cp;
    }
    str[j] = '\0';
    /*    printf("%d '%s'\n",i,str);*/
    if(i == 0){
      if(ks_isfloat_all(str)){
	cb->pov_info.diffuse = atof(str);
      }
    } else if(i == 1){
      if(ks_isfloat_all(str)){
	cb->pov_info.ambient = atof(str);
      }
    }
  }
  /*  printf("%f %f\n",cb->pov_info.diffuse,cb->pov_info.ambient);*/
}
static void change_output_pov(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  printf("change_output_pov\n");
  arg = get_command_arg(cb,CV_COMMAND_OUTPUT_PICTURE_POV,text);
  if(*arg != '\0'){
    set_pov_info(cb,arg);
  }
  cb->output_file_mode = CV_OUTPUT_PICTURE_POV;
}
static void change_output_pov_shadow(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  printf("change_output_pov_shadow\n");
  arg = get_command_arg(cb,CV_COMMAND_OUTPUT_PICTURE_POV_SHADOW,text);
  if(*arg != '\0'){
    set_pov_info(cb,arg);
  }
  cb->output_file_mode = CV_OUTPUT_PICTURE_POV_SHADOW;
}
static void toggle_pov_water(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char msg[256];
  if(fv_get_pov_water_flag()){
    strcpy(msg,"water_mode off");
    fv_set_pov_water_flag(KS_FALSE);
  } else {
    strcpy(msg,"water_mode on");
    fv_set_pov_water_flag(KS_TRUE);
    fv_set_pov_bubble_flag(KS_FALSE);
  }
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,2));
}
static void toggle_pov_bubble(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char msg[256];
  if(fv_get_pov_bubble_flag()){
    strcpy(msg,"bubble_mode off");
    fv_set_pov_bubble_flag(KS_FALSE);
  } else {
    strcpy(msg,"bubble_mode on");
    fv_set_pov_bubble_flag(KS_TRUE);
    fv_set_pov_water_flag(KS_FALSE);
  }
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,2));
}
static void set_selected_for_particle(CV_PARTICLE *p)
{
  p->flags |= CV_PARTICLE_SELECTED;
#ifdef USE_GLSL
  if(p->draw_info_elem_p != NULL){
    p->draw_info_elem_p->em = KS_GL_SELECTED_EMISSION;
  }
#endif
}
static void unset_selected_for_particle(CV_PARTICLE *p)
{
  p->flags &= ~CV_PARTICLE_SELECTED;
#ifdef USE_GLSL
  if(p->draw_info_elem_p != NULL){
    p->draw_info_elem_p->em = 0.0f;
  }
#endif
}
static void set_find_particle_label(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *c,c0[64];
  unsigned int label;
  CV_PARTICLE *p;
  if(strncmp(text,cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_1],
	     strlen(cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_1])) == 0){
    c = strstr(text,cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_1]);
    c += strlen(cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_1])+1;
  } else if(strncmp(text,cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_2],
		    strlen(cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_2]))==0){
    c = strstr(text,cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_2]);
    c += strlen(cb->command[CV_COMMAND_FIND_PARTICLE_LABEL].key[CV_COMMAND_KEY_2])+1;
  } else {
    c = text;
  }
  /*  printf("find_particle_label %s\n",c);*/

  while((c = ks_get_str(", ",c,c0,sizeof(c0))) != NULL){
    label = atoi(c0);
    p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,label);
    /*    printf("%d\n",p->label);*/
    if(p != NULL){
      set_selected_for_particle(p);
      cb->selected_label=ks_add_int_list(cb->selected_label,ks_new_int_list(label));
    }
  }
}
static BOOL make_cdv_bond(CV_CDV_BOND_LIST *cdv_bond, CV_BOND_TYPE *bond_type,
			  CV_PARTICLE **particle_hash, int particle_hash_size,
			  KS_LABEL_LIST **particle_label_hash, int particle_label_hash_size,
			  CV_BOND **pdb_user_bond, CV_BOND **reuse)
{
  int i;
  CV_CDV_BOND_LIST *pb;
  CV_PARTICLE *p,*q;
  KS_LABEL_LIST *ll[2];
  CV_BOND *new_bond[2];
  CV_BOND_TYPE *hit_bond_type;
  double dd[3],r;

  for(pb = cdv_bond; pb != NULL; pb = pb->next){
    /* printf("%d %d %d\n",pb->label[0],pb->label[1],pb->type); */
    for(i = 0; i < 2; i++){
      if((ll[i] = lookup_label_list(particle_label_hash,
				    particle_label_hash_size,pb->label[i])) == NULL){
	ks_error("particle of label %d is not found",pb->label[i]);
	return KS_FALSE;
      }
    }
    for(;ll[0] != NULL; ll[0] = ll[0]->next){
      if(pb->label[0] == ll[0]->label){
	/*	printf("p %d\n",ll[0]->label);*/
	p = lookup_cv_particle_hash_label(particle_hash,particle_hash_size,ll[0]->value);
	if(p != NULL){
	  for(;ll[1] != NULL; ll[1] = ll[1]->next){
	    if(pb->label[1] == ll[1]->label){
	      /*	      printf("q %d\n",ll[1]->value);*/
	      q = lookup_cv_particle_hash_label(particle_hash,particle_hash_size,ll[1]->value);
	      if(q != NULL){
		for(i = 0; i < 3; i++){
		  dd[i] = p->cd[i] - q->cd[i];
		}
		r = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);
		new_bond[0] = new_cv_bond(0,q,r,reuse);
		new_bond[1] = new_cv_bond(0,p,r,reuse);
		hit_bond_type = lookup_cv_bond_type(bond_type,pb->type);
		if(hit_bond_type != NULL){
		  new_bond[0]->type = hit_bond_type;
		  new_bond[1]->type = hit_bond_type;
		}
		if(pb->flags&CV_CDV_BOND_LIST_PDB_USER){
		  *pdb_user_bond = add_cv_bond(*pdb_user_bond,new_bond[0]);
		  *pdb_user_bond = add_cv_bond(*pdb_user_bond,new_bond[1]);
		} else {
		  p->bond = add_cv_bond(p->bond, new_bond[0]);
		  q->bond = add_cv_bond(q->bond, new_bond[1]);
		}
	      }
	    }
	  }
	}
      }
    }
  }
  return KS_TRUE;
}
static void set_normal(double cd0[3], double cd1[3], double cd2[3], double n[3])
{
  int i;
  double vec[2][3];
  for(i = 0; i < 3; i++){
    vec[0][i] = cd1[i] - cd0[i];
    vec[1][i] = cd2[i] - cd0[i];
  }
  ks_exterior_product(vec[0],vec[1],n);
  ks_normalize_vector(n,3);
}
static BOOL make_cdv_face(CV_CDV_FACE_LIST *cdv_face, CV_FACE_TYPE *face_type,
			  CV_ADDITIONAL_SURFACE **sp,
			  CV_PARTICLE **particle_hash, int particle_hash_size,
			  KS_LABEL_LIST **particle_label_hash, int particle_label_hash_size,
			  CV_ADDITIONAL_SURFACE **reuse)
{
  int i,j;
  CV_CDV_FACE_LIST *pf;
  CV_PARTICLE *p[3];
  KS_LABEL_LIST *ll[3];
  CV_FACE_TYPE *hit_face_type;
  int face_num,fn,vn;
  double normal[3];

  face_num = count_cv_cdv_face_list(cdv_face);
  if(*sp == NULL){       // malloc additional surace
    if(*reuse != NULL){  // use reuse buffer
      *sp = *reuse;
      if(face_num*3 > (*sp)->vertex_malloc_num){
	// free and realloc vertex
	ks_free((*reuse)->vertex);
	ks_free((*reuse)->normal);
	ks_free((*reuse)->color);
	if(allocate_additional_surface_vertex(*sp,face_num*3) == KS_FALSE){
	  return KS_FALSE;
	}
      } else {
	// not need to realloc vertex
	(*sp)->vertex = (*reuse)->vertex;
	(*sp)->normal = (*reuse)->normal;
	(*sp)->color = (*reuse)->color;
	(*sp)->vertex_num = face_num*3;
      }
      if(face_num > (*sp)->face_malloc_num){
	// free and realloc face
	ks_free((*reuse)->face);
	if(allocate_additional_surface_face(*sp,face_num) == KS_FALSE){
	  return KS_FALSE;
	}
      } else {
	// not need to realloc face
	(*sp)->face = (*reuse)->face;
	(*sp)->face_num = face_num;
      }
      *reuse = NULL;
    } else {
      // not use reuse buffer
      if(((*sp) = allocate_additional_surface()) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
      if(allocate_additional_surface_vertex(*sp,face_num*3) == KS_FALSE){
	return KS_FALSE;
      }
      if(allocate_additional_surface_face(*sp,face_num) == KS_FALSE){
	return KS_FALSE;
      }
      (*sp)->cull_face = KS_FALSE;
    }
  }

  fn = 0; vn = 0;
  for(pf = cdv_face; pf != NULL; pf = pf->next){
    hit_face_type = lookup_cv_face_type(face_type,pf->type);
    if(hit_face_type == NULL){
      hit_face_type = &init_cv_face_type;
    }
    { // replaced from if(hit_face_type != NULL)
      /*
      printf("%d %d %d %d (%f %f %f)\n",pf->label[0],pf->label[1],pf->label[2],pf->type,
	     hit_face_type->color[0],hit_face_type->color[1],hit_face_type->color[2]);
      */
      for(i = 0; i < 3; i++){
	if((ll[i] = lookup_label_list(particle_label_hash,
				      particle_label_hash_size,pf->label[i])) == NULL){
	  ks_error("particle of label %d is not found",pf->label[i]);
	  return KS_FALSE;
	}
	/*
	  for(;ll[i] != NULL; ll[i] = ll[i]->next){
	  printf("%d %d (%d %d)\n",i,pf->label[i],ll[i]->label,ll[i]->value);
	  }
	*/
      }
      for(;ll[0] != NULL; ll[0] = ll[0]->next){
	if(pf->label[0] == ll[0]->label){
	  /*	printf("p %d\n",ll[0]->label);*/
	  p[0] = lookup_cv_particle_hash_label(particle_hash,particle_hash_size,ll[0]->value);
	  if(p[0] != NULL){
	    for(;ll[1] != NULL; ll[1] = ll[1]->next){
	      if(pf->label[1] == ll[1]->label){
		/*	      printf("q %d\n",ll[1]->value);*/
		p[1]=lookup_cv_particle_hash_label(particle_hash,particle_hash_size,ll[1]->value);
		if(p[1] != NULL){
		  for(;ll[2] != NULL; ll[2] = ll[2]->next){
		    if(pf->label[2] == ll[2]->label){
		      /*	      printf("q %d\n",ll[2]->value);*/
		      p[2] = lookup_cv_particle_hash_label(particle_hash,
							   particle_hash_size,ll[2]->value);
		      if(p[2] != NULL){
			set_normal(p[0]->cd,p[1]->cd,p[2]->cd,normal);
			for(i = 0; i < 3; i++){
			  /*
			  printf("%d (%f %f %f)\n",i,p[i]->cd[0],p[i]->cd[1],p[i]->cd[2]);
			  */
			  for(j = 0; j < 3; j++){
			    (*sp)->vertex[vn*3+j] = p[i]->cd[j];
			    (*sp)->normal[vn*3+j] = normal[j];
			    (*sp)->color[vn*4+j] = hit_face_type->color[j];
			  }
			  (*sp)->color[vn*4+3] = 1.0;
			  (*sp)->face[fn*3+i] = vn;
			  vn++;
			}
			fn++;
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
  /*
  for(i = 0; i < (*sp)->face_num; i++){
    printf("%d (%d %d %d)\n",i,(*sp)->face[i*3],(*sp)->face[i*3+1],(*sp)->face[i*3+2]);
    for(j = 0; j < 3; j++){
      printf("  (%f %f %f)\n",
	     (*sp)->vertex[((*sp)->face[i*3]+j)*3],
	     (*sp)->vertex[((*sp)->face[i*3]+j)*3+1],
	     (*sp)->vertex[((*sp)->face[i*3]+j)*3+2]);
    }
  }
  ks_exit(EXIT_FAILURE);
  */
  return KS_TRUE;
}
static void calc_pos_hash(CV_PARTICLE_POS_HASH *ph, double pos[3], int hash[3])
{
  int i;
  for(i = 0; i < 3; i++){
    hash[i] = (pos[i]-ph->min[i])/ph->len[i];
  }
}
static CV_PARTICLE_POS_HASH *allocate_cv_particle_pos_hash(CV_FRAME *fr, double set_len)
{
  int i,j,k;
  int ii[3];
  double min[3],max[3];
  CV_PARTICLE_POS_HASH *ph;
  CV_PARTICLE *p;

  if((ph = (CV_PARTICLE_POS_HASH*)ks_malloc(sizeof(CV_PARTICLE_POS_HASH),"CV_PARTICLE_POS_HASH"))
     == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < 3; i++){
    min[i] = fr->center[i];
    max[i] = fr->center[i];
  }
  for(p = fr->particle; p != NULL; p = p->next){
    for(i = 0; i < 3; i++){
      if(min[i] > p->cd[i]) min[i] = p->cd[i];
      if(max[i] < p->cd[i]) max[i] = p->cd[i];
    }
  }
  for(i = 0; i < 3; i++){
    min[i] -= .1;
    max[i] += .1;
    ph->n[i] = (max[i] - min[i])/set_len+.5;
    ph->len[i] = (max[i] - min[i])/ph->n[i];
    ph->min[i] = min[i];
  }
  if((ph->particle_num = ks_malloc_int_ppp(ph->n[0],ph->n[1],ph->n[2],"ph->particle_num"))
     == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < ph->n[0]; i++){
    for(j = 0; j < ph->n[1]; j++){
      for(k = 0; k < ph->n[2]; k++){
	ph->particle_num[i][j][k] = 0;
      }
    }
  }
  for(p = fr->particle; p != NULL; p = p->next){
    calc_pos_hash(ph,p->cd,ii);
    ph->particle_num[ii[0]][ii[1]][ii[2]]++;
  }

  if((ph->particle=(CV_PARTICLE*****)ks_malloc(ph->n[0]*sizeof(CV_PARTICLE****),"ph->article"))
     == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < ph->n[0]; i++){
    if((ph->particle[i] = (CV_PARTICLE****)ks_malloc(ph->n[1]*sizeof(CV_PARTICLE***),
						      "ph->particle[i]")) == NULL){
      ks_error_memory();
      return NULL;
    }
  }
  for(i = 0; i < ph->n[0]; i++){
    for(j = 0; j < ph->n[1]; j++){
      if((ph->particle[i][j] = (CV_PARTICLE***)ks_malloc(ph->n[2]*sizeof(CV_PARTICLE**),
							  "ph->particle[i][j]")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
  }
  for(i = 0; i < ph->n[0]; i++){
    for(j = 0; j < ph->n[1]; j++){
      for(k = 0; k < ph->n[2]; k++){
	if((ph->particle[i][j][k] = 
	    (CV_PARTICLE**)ks_malloc(ph->particle_num[i][j][k]*sizeof(CV_PARTICLE*),
				    "ph->particle[i][j][k]")) == NULL){
	  ks_error_memory();
	  return NULL;
	}
      }
    }
  }
  for(i = 0; i < ph->n[0]; i++){
    for(j = 0; j < ph->n[1]; j++){
      for(k = 0; k < ph->n[2]; k++){
	ph->particle_num[i][j][k] = 0;
      }
    }
  }
  for(p = fr->particle; p != NULL; p = p->next){
    calc_pos_hash(ph,p->cd,ii);
    ph->particle[ii[0]][ii[1]][ii[2]][ph->particle_num[ii[0]][ii[1]][ii[2]]++] = p;
  }

  return ph;
}
static void free_cv_particle_pos_hash(CV_PARTICLE_POS_HASH *ph)
{
  int i,j,k;
  for(i = 0; i < ph->n[0]; i++){
    for(j = 0; j < ph->n[1]; j++){
      for(k = 0; k < ph->n[2]; k++){
	ks_free(ph->particle[i][j][k]);
      }
    }
  }
  for(i = 0; i < ph->n[0]; i++){
    for(j = 0; j < ph->n[1]; j++){
      ks_free(ph->particle[i][j]);
    }
  }
  for(i = 0; i < ph->n[0]; i++){
    ks_free(ph->particle[i]);
  }
  ks_free(ph->particle);
  ks_free_int_ppp(ph->n[0],ph->n[1],ph->particle_num);
  ks_free(ph);
}
static void calc_density(KS_GL_BASE *gb, char *text, void *vp)
{
  int i,j,k;
  int l,m;
  int ii[3];
  double set_len, len[3], min[3], max[3];
  double width;
  int check_type;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;
  FV_FIELD_SIZE *fs;
  FV_FIELD *****fi;
  char *cp, c0[256];

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  fs = &fr->fv->field_size;
  fi = &fr->fv->field;

  check_type = 0;
  set_len = 5.0;
  if(text[0] != '\0'){
    cp = text;
    for(i = 0; (cp = ks_get_str(", ",cp,c0,sizeof(c0))) != NULL; i++){
      if(i == 0){
	set_len = atof(c0);
      } else if(i == 1){
	check_type = atoi(c0);
      }
    }
  }
  //  printf("set_len =%f check_type=%d\n",set_len,check_type);
  if(fr->have_side == KS_TRUE){
    fs->n[0] = (fr->side[1][0][0][0] - fr->side[0][0][0][0])/set_len+.5;
    fs->n[1] = (fr->side[0][1][0][1] - fr->side[0][0][0][1])/set_len+.5;
    fs->n[2] = (fr->side[0][0][1][2] - fr->side[0][0][0][2])/set_len+.5;
    for(i = 0; i < 3; i++){
      min[i] = fr->side[0][0][0][i];
    }
    len[0] = (fr->side[1][0][0][0] - fr->side[0][0][0][0])/fs->n[0];
    len[1] = (fr->side[0][1][0][1] - fr->side[0][0][0][1])/fs->n[1];
    len[2] = (fr->side[0][0][1][2] - fr->side[0][0][0][2])/fs->n[2];
  } else {
    for(i = 0; i < 3; i++){
      min[i] = fr->center[i];
      max[i] = fr->center[i];
    }
    for(p = fr->particle; p != NULL; p = p->next){
      for(i = 0; i < 3; i++){
	if(min[i] > p->cd[i]) min[i] = p->cd[i];
	if(max[i] < p->cd[i]) max[i] = p->cd[i];
      }
    }
    for(i = 0; i < 3; i++){
      fs->n[i] = (max[i] - min[i])*1.01/set_len+.5;
      len[i] = (max[i] - min[i])*1.01/fs->n[i];
    }
  }
  /*
  printf("%d %d %d %d %f %f %f\n",fr->have_side,fs->n[0],fs->n[1],fs->n[2],len[0],len[1],len[2]);
  */

  fs->val_num = count_cv_atom(cb->atom);
  fs->val_num = 1;

  if(fs->val_num >= FV_MAX_COLUMN_NUM){
    draw_error(cb,"ERROR: atom type is too large. It must smaller than %d",FV_MAX_COLUMN_NUM);
    return;
  }

  if((*fi) == NULL){
    if(((*fi) = (FV_FIELD****)ks_malloc(fs->n[0]*sizeof(FV_FIELD***),"fi")) == NULL){
      draw_error(cb,"MEMORY ERROR: set_calc_density");
      return;
    }
    for(i = 0; i < fs->n[0]; i++)
      if(((*fi)[i] = (FV_FIELD***)ks_malloc(fs->n[1]*sizeof(FV_FIELD**),"fi[i]")) == NULL){
	draw_error(cb,"MEMORY ERROR: set_calc_density");
	return;
      }
    for(i = 0; i < fs->n[0]; i++){
      for(j = 0; j < fs->n[1]; j++){
	if(((*fi)[i][j] = (FV_FIELD**)ks_malloc(fs->n[2]*sizeof(FV_FIELD*),"fi[i][j]")) == NULL){
	  draw_error(cb,"MEMORY ERROR: set_calc_density");
	  return;
	}
      }
    }
    for(i = 0; i < fs->n[0]; i++){
      for(j = 0; j < fs->n[1]; j++){
	for(k = 0; k < fs->n[2]; k++){
	  if(((*fi)[i][j][k] = (FV_FIELD*)ks_malloc(sizeof(FV_FIELD),"fi")) == NULL){
	    draw_error(cb,"MEMORY ERROR: set_calc_density");
	    return;
	  }
	  (*fi)[i][j][k]->cd[0] = min[0]+(i+.5)*len[0];
	  (*fi)[i][j][k]->cd[1] = min[1]+(j+.5)*len[1];
	  (*fi)[i][j][k]->cd[2] = min[2]+(k+.5)*len[2];
	  if(((*fi)[i][j][k]->s=(double*)ks_malloc(fs->val_num*sizeof(double),
						   "fi->s")) ==NULL){
	    draw_error(cb,"MEMORY ERROR: set_calc_density");
	    return;
	  }
	  /*
	  if(i == 11 && j == 11 & k == 21){
	    printf("%d %d %d %p\n",i,j,k,(*fi)[i][j][k]->s);
	  }
	  */
	  for(l = 0; l < fs->val_num; l++){
	    (*fi)[i][j][k]->s[l] = 0;
	  }
	  for(l = 0; l < 3; l++){
	    for(m = 0; m < 2; m++){
	      (*fi)[i][j][k]->c[l][m] = NULL;
	    }
	  }
	}
      }
    }
  }
  /*
  printf("end malloc\n");
  */
  for(p = fr->particle; p != NULL; p = p->next){
    if(p->atom->atype == check_type){
      for(i = 0; i < 3; i++){
	ii[i] = (p->cd[i]-min[i])/len[i];
      }
      //    (*fi)[ii[0]][ii[1]][ii[2]]->s[p->atom->atype]++;
      (*fi)[ii[0]][ii[1]][ii[2]]->s[0]++;
    }
  }
  for(i = 0; i < fs->n[0]; i++){
    for(j = 0; j < fs->n[1]; j++){
      for(k = 0; k < fs->n[2]; k++){
	for(l = 0; l < fs->val_num; l++){
	  (*fi)[i][j][k]->s[l] /= len[0]*len[1]*len[2];
	}
      }
    }
  }
  /*
  printf("set_field_connection\n");
  */
  fv_set_field_connection(fi,fs);
  /*
  printf("set_max_min\n");
  */
  fv_set_max_min(fr->fv->field,&fr->fv->field_size,cb->fv_target.val);
  width = (fr->fv->field_size.max - fr->fv->field_size.min)*.01;
  fr->fv->field_size.min -= width*.5;
  fr->fv->field_size.max += width*.5;
  //  printf("%f %f\n",fr->fv->field_size.min,fr->fv->field_size.max);
  /*
  printf("init\n");
  */
  if(fv_init_field(fr->fv->field,fr->fv->field_size,&fr->fv->contour_size,&fr->fv->contour_fan,
		   &cb->fv_target,CV_FDVIEW_CONTOUR_LEVEL_NUM,fr->fv->black_and_white)
     == KS_FALSE){
    draw_error(cb,"ERROR: fv_init_field");
    return;
  }
  /*
  printf("init end\n");
  */

}
static void set_calc_density(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_CALC_DENSITY,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input length and type (5.0 0) > ",NULL,cb->input_key_font,
			calc_density,KS_FALSE,cb);
  } else {
    calc_density(gb,arg,(void*)cb);
  }
}
static void fit_axis(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  int i;
  char *arg;
  CV_FRAME *fr;
  CV_PARTICLE *sp[2],*p;
  double axis[3],rot_axis[3],vector[3];
  double angle;
  double rot_mat[3][3];
  double pos[3];

  fr = &cb->frame[cb->current_frame];
  arg = get_command_arg(cb,CV_COMMAND_FIT_AXIS,text);
  if(ks_count_int_list(cb->selected_label) != 2){
    draw_error(cb,"Please select two particles");
    return;
  }
  if(*arg == '\0'){
    draw_error(cb,"Please set axis;  x, y or z");
    return;
  }
  if(*arg == 'x'){
    axis[0] = 1.0; axis[1] = 0.0; axis[2] = 0.0;
  } else if(*arg == 'y'){
    axis[0] = 0.0; axis[1] = 1.0; axis[2] = 0.0;
  } else if(*arg == 'z'){
    axis[0] = 0.0; axis[1] = 0.0; axis[2] = 1.0;
  }

  sp[0] = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       cb->selected_label->value);
  sp[1] = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       cb->selected_label->next->value);
  /*
  printf("%d %s %d %s\n",sp[0]->label,sp[0]->property->name,sp[1]->label,sp[1]->property->name);
  */
  for(i = 0; i < 3; i++){
    vector[i] = sp[0]->cd[i] - sp[1]->cd[i];
  }
  angle = ks_calc_vectors_angle(axis,vector);
  ks_exterior_product(axis,vector,rot_axis);
  /*
  printf("%f (%f %f %f)\n",angle/M_PI*180,rot_axis[0],rot_axis[1],rot_axis[2]);
  */
  ks_get_rotate_matrix(-angle,rot_axis[0],rot_axis[1],rot_axis[2],rot_mat);
  for(p = fr->particle; p != NULL; p = p->next){
    for(i = 0; i < 3; i++){
      pos[i] = p->cd[i] - sp[1]->cd[i];
    }
    ks_multi_matrix_33x3(rot_mat,pos);
    for(i = 0; i < 3; i++){
      p->cd[i] = pos[i] + sp[1]->cd[i];
    }
  }
}
static void calc_internal_cd(double *cd0, double *cd1, double *cd2, double *icd, double *cd)
{
  int i;
  double v10[3],v12[3],v21[3];
  double ax[3];
  double m[3][3];
  double angle[2];

  /*
  printf("calc_internal_cd\n");
  if(cd2 != NULL)
    printf("cd2 (%f %f %f)\n",cd2[0],cd2[1],cd2[2]);
  if(cd1 != NULL)
    printf("cd1 (%f %f %f)\n",cd1[0],cd1[1],cd1[2]);
  if(cd0 != NULL)
    printf("cd0 (%f %f %f)\n",cd0[0],cd0[1],cd0[2]);
  */

  if(cd2 == NULL && cd1 == NULL){
    cd[0] = cd0[0];
    cd[1] = cd0[1] + icd[0];
    cd[2] = cd0[2];
    /*    printf("internal_cd %f %f %f\n",cd[0],cd[1],cd[2]);*/
    return;
  } else if(cd2 == NULL){
    angle[0] = 0;
  } else {
    for(i = 0; i < 3; i++) v10[i] = cd2[i] - cd1[i];
    angle[0] = icd[2]/180.*M_PI;
  }
  angle[1] = icd[1]/180.*M_PI;
  for(i = 0; i < 3; i++) v12[i] = cd0[i] - cd1[i];
  for(i = 0; i < 3; i++) v21[i] = cd1[i] - cd0[i];
  ks_normalize_vector(v21,3);

  for(i = 0; i < 3; i++) cd[i] = icd[0]*v21[i];
  if(cd2 == NULL){
    ax[0] =-1.0; ax[1] = 0.0; ax[2] = 0.0;
  } else {
    ks_exterior_product(v10,v12,ax);
    /*
    printf("v10 %f %f %f\n",v10[0],v10[1],v10[2]);
    printf("v12 %f %f %f\n",v12[0],v12[1],v12[2]);
    printf("ax %f %f %f\n",ax[0],ax[1],ax[2]);
    */
  }

  if(ks_calc_vector_length(ax,3) == 0){
    ks_error("vector length is zero");
    ks_assert(0);
  }

  ks_get_rotate_matrix(-angle[0],v21[0],v21[1],v21[2],m);
  ks_multi_matrix_33x3(m,ax);
  /*
  printf("ax %f %f %f\n",ax[0],ax[1],ax[2]);
  printf("angle1 %f %f\n",angle[1],icd[1]);
  */
  ks_get_rotate_matrix(angle[1],ax[0],ax[1],ax[2],m);
  /*  ks_print_mat_33(m);*/
  /*  printf("cd %f %f %f\n",cd[0],cd[1],cd[2]);*/
  ks_multi_matrix_33x3(m,cd);
  /*  printf("cd %f %f %f\n",cd[0],cd[1],cd[2]);*/
  for(i = 0; i < 3; i++) cd[i] += cd0[i];
  /*  exit(0);*/
}
static void add_residue_next(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_RESIDUE *new_pr = NULL, *prev_pr;
  CV_PARTICLE *p, *new_p, *next_p;
  int residue_file_label,residue_number;
  char *cp, c0[256], name[8];
  KS_CHAR_LIST *add_residue = NULL,*cl;
  enum {N,CA,C,O,NUM};
  double icd[NUM][3] = {{1.335, 116.600, 180.000},
			{1.449, 121.900, 180.000},
			{1.522, 111.100, 180.000},
			{1.229, 120.500,   0.000}};
  double cd[NUM][3];
  char *element[NUM] = {"N","C","C","O"};
  char *atom_name[NUM] = {"N   ","CA  ","C   ","O   "};
  CV_ATOM *atom;
  CV_PARTICLE_PROPERTY *pro;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  cp = text;
  while((cp = ks_get_str(", ",cp,c0,sizeof(c0))) != NULL){
    /*    printf("%s\n",c0);*/
    if(strlen(c0) != 3){
      draw_error(cb,"ERROR: length of residue name must be 3");
      return;
    }
    add_residue = ks_addend_char_list(add_residue,ks_new_char_list(c0,0));
  }
  /*
  if(cb->selected_label->value < cb->selected_label->next->value){
    label[0] = cb->selected_label->value;
    label[1] = cb->selected_label->next->value;
  } else {
    label[1] = cb->selected_label->value;
    label[0] = cb->selected_label->next->value;
  }
  */
  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				    cb->selected_label->value);
  /*
  p[1] = lookup_cv_particle_hash_label(fr->particle_hash,fr->particle_hash_size,label[1]);
  */
  if(p != NULL){
    if(p->property != NULL){
      /*
      printf("(%d %s %d %s)(%d %s %d %s)\n"
	     ,p->file_label,p->atom->name
	     ,p->property->residue->file_label,p->property->residue->name
	     ,p[1]->file_label,p[1]->atom->name
	     ,p[1]->property->residue->file_label,p[1]->property->residue->name);
      */
      residue_file_label = p->property->residue->file_label+1;
      residue_number = p->property->residue->number+1;
      prev_pr = p->property->residue;
      ks_assert(prev_pr->particle_num != 0);
      /*      next_p = prev_pr->main_C;*/
      next_p = prev_pr->particle[prev_pr->particle_num-1];
      for(cl = add_residue; cl != NULL; cl = cl->next){
	for(i = 0; i < 4; i++)
	  name[i] = toupper(((unsigned char*)cl->name)[i]);
	new_pr = new_cv_residue(fr->residue_label_max++,CV_RESIDUE_AMINO,residue_number++,name
				,residue_file_label++,
				&cb->reuse_base.residue);
	if(new_pr->particle == NULL){
	  if((new_pr->particle=(CV_PARTICLE**)ks_malloc(NUM*sizeof(CV_PARTICLE*),
							"pr->particle")) == NULL){
	    draw_error(cb,"ERROR: memory error in add_residue_next");
	    return;
	  }
	} else if(NUM != new_pr->particle_num){
	  new_pr->particle_num = NUM;
	  if((new_pr->particle=(CV_PARTICLE**)ks_realloc(new_pr->particle,NUM*sizeof(CV_PARTICLE*),
							"pr->particle")) == NULL){
	    draw_error(cb,"ERROR: memory error in add_residue_next");
	    return;
	  }
	}
	calc_internal_cd(prev_pr->main_C->cd,prev_pr->main_CA->cd,prev_pr->main_N->cd,
			 icd[N],cd[N]);
	calc_internal_cd(cd[N],prev_pr->main_C->cd,prev_pr->main_CA->cd,icd[CA],cd[CA]);
	calc_internal_cd(cd[CA],cd[N],prev_pr->main_C->cd,icd[C],cd[C]);
	calc_internal_cd(cd[C],cd[CA],cd[N],icd[O],cd[O]);
	/*
	printf("%f %f %f\n",prev_pr->main_C->cd[0],prev_pr->main_C->cd[1],prev_pr->main_C->cd[2]);
	*/
	for(i = 0; i < NUM; i++){
	  /*
	  printf("%d '%s' '%s' %f %f %f\n",i,atom_name[i],element[i],cd[i][0],cd[i][1],cd[i][2]);
	  */
	  if((pro=new_cv_particle_property(atom_name[i],
					   p->property->aggregate,p->property->chain,
					   new_pr,NULL,&fr->reuse->property)) == NULL){

	  }
	  atom = lookup_cv_atom(cb->atom,element[i]);
	  new_p=new_cv_particle(fr->particle_label_max++,fr->particle_file_label_max++,
				pdb_name_to_particle_flags(atom_name[i]),
				atom,pro,cd[i][0],cd[i][1],cd[i][2],&fr->reuse->particle);
	  new_pr->particle[new_pr->particle_num++] = new_p;
	  if(i == N){
	    new_pr->main_N = new_p;
	    prev_pr->main_C->bond = 
	      add_cv_bond(prev_pr->main_C->bond,new_cv_bond(0,new_pr->main_N, icd[N][0],
							    &fr->reuse->bond));
	    new_pr->main_N->bond =  
	      add_cv_bond(new_pr->main_N->bond, new_cv_bond(0,prev_pr->main_C,icd[N][0],
							    &fr->reuse->bond));
	  } else if(i == CA){
	    new_pr->main_CA = new_p;
	    new_pr->main_N->bond  = 
	      add_cv_bond(new_pr->main_N->bond, new_cv_bond(0,new_pr->main_CA,icd[CA][0],
							    &fr->reuse->bond));
	    new_pr->main_CA->bond = 
	      add_cv_bond(new_pr->main_CA->bond,new_cv_bond(0,new_pr->main_N, icd[CA][0],
							    &fr->reuse->bond));
	  } else if(i == C){
	    new_pr->main_C = new_p;
	    new_pr->main_CA->bond = 
	      add_cv_bond(new_pr->main_CA->bond,new_cv_bond(0,new_pr->main_C, icd[C][0],
							    &fr->reuse->bond));
	    new_pr->main_C->bond  = 
	      add_cv_bond(new_pr->main_C->bond, new_cv_bond(0,new_pr->main_CA,icd[C][0],
							    &fr->reuse->bond));
	  } else if(i == O){
	    new_pr->main_O = new_p;
	    new_pr->main_C->bond = 
	      add_cv_bond(new_pr->main_C->bond,new_cv_bond(0,new_pr->main_O, icd[O][0],
							   &fr->reuse->bond));
	    new_pr->main_O->bond  = 
	      add_cv_bond(new_pr->main_O->bond,new_cv_bond(0,new_pr->main_C, icd[O][0],
							   &fr->reuse->bond));
	    if(prev_pr->particle[prev_pr->particle_num-1]->flags&CV_PARTICLE_CHAIN_END){
	      prev_pr->particle[prev_pr->particle_num-1]->flags &= ~(CV_PARTICLE_CHAIN_END);
	      new_p->flags |= CV_PARTICLE_CHAIN_END;
	    }
	  }
	  set_particle_color(new_p,&fr->unknown_residue_label,&fr->unknown_residue_label_max);
	  set_particle_hash(fr,new_p,
			    cb->particle_hash,cb->particle_hash_size,
			    cb->particle_label_hash,cb->particle_label_hash_size,
			    &cb->reuse_base.label);
	  /*
	  printf("%d %d %s end %c\n",i,new_p->label,new_p->atom->name,
		 new_p->flags&CV_PARTICLE_CHAIN_END?'o':'x');
	  */
	  if(next_p->next == NULL)
	    fr->particle_end = new_p;
	  if(next_p->prev == NULL)
	    fr->particle = new_p;

	  new_p->next = next_p;
	  new_p->prev = next_p->prev;
	  if(next_p->prev != NULL)
	    next_p->prev->next = new_p;
	  next_p->prev = new_p;
	  next_p = new_p;
	  /*
	  new_p->next = prev_p->next;
	  new_p->prev = prev_p;
	  prev_p->next = new_p;
	  if(prev_p->next != NULL)
	    prev_p->next->prev = new_p;
	  prev_p = new_p;
	  */
	}
	if(prev_pr->flags&CV_RESIDUE_END){
	  prev_pr->flags &= ~(CV_RESIDUE_END);
	  prev_pr->flags |= CV_RESIDUE_NORMAL;
	  new_pr->flags |= CV_RESIDUE_END;
	}
	new_pr->next = prev_pr->next;
	prev_pr->next = new_pr;
	prev_pr = new_pr;
      }
    }
  }
  ks_free_char_list(add_residue);
}
/*
static void set_add_residue_next(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(ks_count_int_list(cb->selected_label) == 1){
    ks_set_gl_input_key(gb,"input residue > ",NULL,((CV_BASE*)vp)->input_key_font,
			add_residue_next,KS_FALSE,cb);
  }
}
*/
static void add_residue_prev(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_RESIDUE *new_pr = NULL, *next_pr;
  CV_PARTICLE *p, *new_p, *prev_p;
  int residue_file_label,residue_number;
  char *cp, c0[256], name[8];
  KS_CHAR_LIST *add_residue = NULL,*cl;
  int add_residue_num;
  enum {O,C,CA,N,NUM};
  double icd[NUM][3] = {{1.229, 120.500,   0.000},
			{1.335, 121.900, 180.000},
			{1.449, 111.100, 180.000},
			{1.522, 116.600, 180.000}};
  double cd[NUM][3];
  char *element[NUM] = {"O","C","C","N"};
  char *atom_name[NUM] = {"O   ","C   ","CA  ","N   "};
  CV_ATOM *atom;
  CV_PARTICLE_PROPERTY *pro;
  int particle_label, particle_file_label;
  int residue_label;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  add_residue_num = 0;
  cp = text;
  while((cp = ks_get_str(", ",cp,c0,sizeof(c0))) != NULL){
    if(strlen(c0) != 3){
      draw_error(cb,"ERROR: length of residue name must be 3");
      return;
    }
    add_residue = ks_addend_char_list(add_residue,ks_new_char_list(c0,0));
    add_residue_num++;
  }
  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				    cb->selected_label->value);
  if(p != NULL){
    if(p->property != NULL){
      particle_label = fr->particle_label_max+add_residue_num*NUM-1;
      particle_file_label = fr->particle_file_label_max+add_residue_num*NUM-1;
      residue_label = fr->residue_label_max+add_residue_num-1;
      residue_file_label = p->property->residue->file_label-1;
      residue_number = p->property->residue->number-1;
      next_pr = p->property->residue;
      ks_assert(next_pr->particle_num != 0);
      prev_p = next_pr->main_N;
      for(cl = add_residue; cl != NULL; cl = cl->next){
	for(i = 0; i < 4; i++)
	  name[i] = toupper(((unsigned char*)cl->name)[i]);
	new_pr = new_cv_residue(residue_label--,0,residue_number--,name,residue_file_label--,
				&cb->reuse_base.residue);
	if(new_pr->particle == NULL){
	  if((new_pr->particle=(CV_PARTICLE**)ks_malloc(NUM*sizeof(CV_PARTICLE*),
							"pr->particle")) == NULL){
	    draw_error(cb,"ERROR: memory error in add_residue_next");
	    return;
	  }
	} else if(NUM != new_pr->particle_num){
	  new_pr->particle_num = NUM;
	  if((new_pr->particle=(CV_PARTICLE**)ks_realloc(new_pr->particle,NUM*sizeof(CV_PARTICLE*),
							"pr->particle")) == NULL){
	    draw_error(cb,"ERROR: memory error in add_residue_next");
	    return;
	  }
	}
	/*	printf("%s\n",name);*/
	new_pr->particle_num = NUM;
	calc_internal_cd(next_pr->main_N->cd,next_pr->main_CA->cd,next_pr->main_C->cd,
			 icd[C],cd[C]);
	calc_internal_cd(cd[C],next_pr->main_N->cd,next_pr->main_CA->cd,icd[CA],cd[CA]);
	calc_internal_cd(cd[CA],cd[C],next_pr->main_N->cd,icd[N],cd[N]);
	calc_internal_cd(cd[C],cd[CA],cd[N],icd[O],cd[O]);
	for(i = 0; i < NUM; i++){
	  /*
	  printf("%d '%s' '%s' %f %f %f\n",i,atom_name[i],element[i],cd[i][0],cd[i][1],cd[i][2]);
	  */
	  if((pro=new_cv_particle_property(atom_name[i],
					   p->property->aggregate,p->property->chain,
					   new_pr,NULL,&fr->reuse->property)) == NULL){

	  }
	  atom = lookup_cv_atom(cb->atom,element[i]);
	  new_p=new_cv_particle(particle_label--,particle_file_label--,
				pdb_name_to_particle_flags(atom_name[i]),
				atom,pro,cd[i][0],cd[i][1],cd[i][2],&fr->reuse->particle);
	  new_pr->particle[NUM-i-1] = new_p;
	  if(i == O){
	    new_pr->main_O = new_p;
	  } else if(i == C){
	    new_pr->main_C = new_p;
	    next_pr->main_N->bond = 
	      add_cv_bond(next_pr->main_N->bond,new_cv_bond(0,new_pr->main_C, icd[C][0],
							    &fr->reuse->bond));
	    new_pr->main_C->bond =  
	      add_cv_bond(new_pr->main_C->bond, new_cv_bond(0,next_pr->main_N,icd[C][0],
							    &fr->reuse->bond));
	    new_pr->main_C->bond = 
	      add_cv_bond(new_pr->main_C->bond,new_cv_bond(0,new_pr->main_O, icd[O][0],
							   &fr->reuse->bond));
	    new_pr->main_O->bond  = 
	      add_cv_bond(new_pr->main_O->bond,new_cv_bond(0,new_pr->main_C, icd[O][0],
							   &fr->reuse->bond));
	  } else if(i == CA){
	    new_pr->main_CA = new_p;
	    new_pr->main_C->bond  = 
	      add_cv_bond(new_pr->main_C->bond, new_cv_bond(0,new_pr->main_CA,icd[CA][0],
							    &fr->reuse->bond));
	    new_pr->main_CA->bond = 
	      add_cv_bond(new_pr->main_CA->bond,new_cv_bond(0,new_pr->main_C, icd[CA][0],
							    &fr->reuse->bond));
	  } else if(i == N){
	    new_pr->main_N = new_p;
	    new_pr->main_CA->bond = 
	      add_cv_bond(new_pr->main_CA->bond,new_cv_bond(0,new_pr->main_N, icd[C][0],
							    &fr->reuse->bond));
	    new_pr->main_N->bond  = 
	      add_cv_bond(new_pr->main_N->bond, new_cv_bond(0,new_pr->main_CA,icd[C][0],
							    &fr->reuse->bond));
	  }
	  set_particle_color(new_p,&fr->unknown_residue_label,&fr->unknown_residue_label_max);
	  set_particle_hash(fr,new_p,
			    cb->particle_hash,cb->particle_hash_size,
			    cb->particle_label_hash,cb->particle_label_hash_size,
			    &cb->reuse_base.label);

	  if(prev_p->next == NULL)
	    fr->particle_end = new_p;
	  if(prev_p->prev == NULL)
	    fr->particle = new_p;
	  new_p->next = prev_p->next;
	  new_p->prev = prev_p;
	  if(prev_p->next != NULL)
	    prev_p->next->prev = new_p;
	  prev_p->next = new_p;
	  prev_p = new_p;
	}

	if(next_pr->flags&CV_RESIDUE_START){
	  next_pr->flags &= ~(CV_RESIDUE_START);
	  next_pr->flags |= CV_RESIDUE_NORMAL;
	  new_pr->flags |= CV_RESIDUE_START;
	}

	{
	  CV_RESIDUE *pr,*prev;
	  prev = NULL;
	  for(pr = p->property->chain->residue; pr != next_pr; pr = pr->next){
	    prev = pr;
	  }
	  new_pr->next = next_pr;
	  if(prev == NULL){
	    p->property->chain->residue = new_pr;
	  } else {
	    prev->next = new_pr;
	  }
	  next_pr = new_pr;
	}
	
	/*
	new_pr->next = prev_pr->next;
	prev_pr->next = new_pr;
	prev_pr = new_pr;
	*/
      }
    }
  }
  ks_free_char_list(add_residue);
}
/*
static void set_add_residue_prev(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(ks_count_int_list(cb->selected_label) == 1){
    ks_set_gl_input_key(gb,"input residue > ",NULL,((CV_BASE*)vp)->input_key_font,
			add_residue_prev,KS_FALSE,cb);
  }
}
*/
static void set_add_residue_next(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  if(ks_count_int_list(cb->selected_label) == 1){
    ks_set_gl_input_key(gb,"input residue > ",NULL,cb->input_key_font,
			add_residue_next,KS_FALSE,cb);
  }
}
static void set_add_residue_prev(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  if(ks_count_int_list(cb->selected_label) == 1){
    ks_set_gl_input_key(gb,"input residue > ",NULL,cb->input_key_font,
			add_residue_prev,KS_FALSE,cb);
  }
}
static void change_residue(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_BASE *cb;
  KS_INT_LIST *il;
  CV_RESIDUE *pr;

  cb = (CV_BASE*)vp;
  if(ks_count_int_list(cb->selected_residue_label) == 0){
    draw_error(cb,"ERROR: Please select residue");
    return;
  }

  for(il = cb->selected_residue_label; il != NULL; il = il->next){
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size, il->value);
    if(pr != NULL){
      CV_PARTICLE *p,**residue_particle;
      for(i = 0; i < (int)strlen(text); i++){
	text[i] = toupper(((unsigned char*)text)[i]);
      }
      /*      printf("%d '%s' -> '%s'\n",pr->label,pr->name,text);*/
      strcpy(pr->name,text);
      /*      ks_set_amino_char(text,&pr->type);*/
      ks_get_amino_name_to_char(text,&pr->type);
      pr->flags &= ~(CV_RESIDUE_SELECTED);
      pr->particle_num0 = pr->particle_num;
      pr->particle_num = 0;
      for(i = 0; i < pr->particle_num0; i++){
	p = pr->particle[i];
	/*
	printf("%d %s %c\n",p->label,p->property->name,p->flags&CV_PARTICLE_MAIN?'M':'S');
	*/
	if(p->flags&CV_PARTICLE_MAIN || 
	   (p->property != NULL && strcmp(p->property->name,"O   ") == 0)){
	  unset_selected_for_particle(p);
	  pr->particle_num++;
	}
      }
      if((residue_particle = (CV_PARTICLE**)ks_malloc(pr->particle_num*sizeof(CV_PARTICLE*),
						      "residue_particle")) == NULL){
	draw_error(cb,"ERROR: memory error in change_residue");
	return;
      }
      pr->particle_num = 0;
      for(i = 0; i < pr->particle_num0; i++){
	p = pr->particle[i];
	if(p->flags&CV_PARTICLE_MAIN || 
	   (p->property != NULL && strcmp(p->property->name,"O   ") == 0)){
	  residue_particle[pr->particle_num++] = p;
	}
      }
      ks_free(pr->particle);
      pr->particle = residue_particle;
    }
  }

  del_particle(gb,"y",(void*)cb);

}
static void set_change_residue(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_CHANGE_RESIDUE,text);
  /*  printf("'%s' '%s'\n",text,arg);*/
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input residue > ",NULL,cb->input_key_font,change_residue,KS_FALSE,cb);
  } else {
    change_residue(gb,arg,(void*)cb);
  }
}
static void change_atom(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  KS_INT_LIST *il;
  CV_PARTICLE *p;
  CV_ATOM *hit_atom;

  cb = (CV_BASE*)vp;
  if(ks_count_int_list(cb->selected_label) == 0){
    draw_error(cb,"ERROR: Please select atom");
    return;
  }

  hit_atom = make_cv_atom(&cb->atom,text,cb->atype_use);
  for(il = cb->selected_label; il != NULL; il = il->next){
    p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size, il->value);
    if(p != NULL){
      p->atom = hit_atom;
      if(p->property != NULL){
	strcpy(p->property->name,text);
      }
    }
  }
}
static void set_change_atom(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_CHANGE_ATOM,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input residue > ",NULL,cb->input_key_font,change_atom,KS_FALSE,cb);
  } else {
    change_atom(gb,arg,(void*)cb);
  }
}
static void make_bond(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p0,*p1;
  double r;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  p0 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				     (unsigned int)cb->selected_label->value);
  p1 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				     (unsigned int)cb->selected_label->next->value);
  r = atof(text);
  p0->bond = add_cv_bond(p0->bond, new_cv_bond(0,p1,r,&fr->reuse->bond));
  p1->bond = add_cv_bond(p1->bond, new_cv_bond(0,p0,r,&fr->reuse->bond));
}
static void set_make_bond(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  int i;
  CV_PARTICLE *p0,*p1;
  char bond_len[64];
  double dd[3],r;

  if(ks_count_int_list(cb->selected_label) == 2){
    p0 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       (unsigned int)cb->selected_label->value);
    p1 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       (unsigned int)cb->selected_label->next->value);
    ks_assert(p0 != NULL);
    ks_assert(p1 != NULL);
    
    for(i = 0; i < 3; i++)
      dd[i] = p0->cd[i]-p1->cd[i];
    r = dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2];
    sprintf(bond_len,"%.3f",sqrt(r));
    if(p0->property != NULL && p1->property != NULL){
      if((strcmp(p0->property->name,"N   ")==0&&strcmp(p1->property->name,"CA  ")==0)||
	 (strcmp(p1->property->name,"N   ")==0&&strcmp(p0->property->name,"CA  ")==0)){
	sprintf(bond_len,"1.449");
      }
      if((strcmp(p0->property->name,"CA  ")==0&&strcmp(p1->property->name,"C   ")==0)||
	 (strcmp(p1->property->name,"CA  ")==0&&strcmp(p0->property->name,"C   ")==0)){
	sprintf(bond_len,"1.522");
      }
      if((strcmp(p0->property->name,"C   ")==0&&strcmp(p1->property->name,"N   ")==0)||
	 (strcmp(p1->property->name,"C   ")==0&&strcmp(p0->property->name,"N   ")==0)){
	sprintf(bond_len,"1.335");
      }
    }
    ks_set_gl_input_key(gb,"bond length > ",bond_len,cb->input_key_font,
			make_bond,KS_FALSE,cb);
  }
}
static void set_remove_bond(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  CV_PARTICLE *p0,*p1;

  if(ks_count_int_list(cb->selected_label) == 2){
    p0 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       (unsigned int)cb->selected_label->value);
    p1 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       (unsigned int)cb->selected_label->next->value);
    ks_assert(p0 != NULL);
    ks_assert(p1 != NULL);
    p0->bond = del_cv_bond(p0->bond,p1);
    p1->bond = del_cv_bond(p1->bond,p0);
    /*
    {
      CV_BOND *bond;
      printf("remove %d %d\n",p0->label,p1->label);
      for(bond = p0->bond; bond != NULL; bond = bond->next){
	printf("0 %d: %d\n",p0->label,bond->p->label);
      }
      for(bond = p1->bond; bond != NULL; bond = bond->next){
	printf("1 %d: %d\n",p1->label,bond->p->label);
      }
    }
    */
  }
}
static void change_level(CV_BASE *cb, double value, BOOL max_flg)
{
  CV_FRAME *fr;
  //  double level_value;

  fr = &cb->frame[cb->current_frame];
  /*
  printf("%f %f %f\n",fr->fv->field_size.min,fr->fv->field_size.max,value);
  */
  if(max_flg == KS_TRUE){
    if(fr->fv->field_size.min < value){
      /*
      level_value = (value - fr->fv->field_size.min)/(CV_FDVIEW_CONTOUR_LEVEL_NUM);
      fr->fv->field_size.max = value+level_value;
      */
      fr->fv->field_size.max = value;
    } else {
      return;
    }
  } else {
    if(fr->fv->field_size.max > value){
      /*
      level_value = (fr->fv->field_size.max - value)/(CV_FDVIEW_CONTOUR_LEVEL_NUM);
      fr->fv->field_size.min = value-level_value;
      */
      fr->fv->field_size.min = value;
    } else {
      return;
    }
  }
  /*  fr->fv->field_size.max = 0;*/
}
static void change_level_max(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  double value;
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(strcmp(text,"auto") == 0 || strcmp(text,"AUTO") == 0){
    fv_set_max_min(fr->fv->field,&fr->fv->field_size,cb->fv_target.val);
  } else {
    value = atof(text);
    /*  printf("max %f\n",value);*/
    change_level(cb,value,KS_TRUE);
  }
  if(fv_init_field(fr->fv->field,fr->fv->field_size,&fr->fv->contour_size,&fr->fv->contour_fan,
		   &cb->fv_target,CV_FDVIEW_CONTOUR_LEVEL_NUM,fr->fv->black_and_white)
     == KS_FALSE){
    return;
  }
}
static void set_change_level_max(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_CHANGE_LEVEL_MAX,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input max > ",NULL,cb->input_key_font,change_level_max,KS_FALSE,cb);
  } else {
    change_level_max(gb,arg,(void*)cb);
  }
}
static void change_level_min(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  double value;
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(strcmp(text,"auto") == 0 || strcmp(text,"AUTO") == 0){
    fv_set_max_min(fr->fv->field,&fr->fv->field_size,cb->fv_target.val);
  } else {
    value = atof(text);
    /*  printf("max %f\n",value);*/
    change_level(cb,value,KS_FALSE);
  }
  if(fv_init_field(fr->fv->field,fr->fv->field_size,&fr->fv->contour_size,&fr->fv->contour_fan,
		   &cb->fv_target,CV_FDVIEW_CONTOUR_LEVEL_NUM,fr->fv->black_and_white)
     == KS_FALSE){
    return;
  }
}
static void set_change_level_min(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_CHANGE_LEVEL_MIN,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input min > ",NULL,cb->input_key_font,change_level_min,KS_FALSE,cb);
  } else {
    change_level_min(gb,arg,(void*)cb);
  }
}
static void calc_gr_particle(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
}
static void calc_gr_monomer(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i,j,k;
  int num;
  double dd[3];
  double mass;
  double side[3],sideh[3];
  CV_FRAME *fr;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  KS_CHAR_LIST *cp,*name = NULL;
  int name_type_num = 0;
  int *name_type_data_num;
  typedef struct{
    int type;
    double cd[3];
  } MONOMER;
  MONOMER *mono = NULL;
  double r, dr = .2, r_max = 10;
  int idr;
  int **gr, gr_num, gr_type_num;
  int ***gr_mat;
  
  fr = &cb->frame[cb->current_frame];
  num = 0;
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    /*    printf("agg %d\n",pa->label);*/
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      /*      printf("  chain %d %s\n",pc->label,pc->name);*/
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	/*	printf("    residue %d %s\n",pr->label,pr->name);*/
	num++;
	if(ks_lookup_char_list(name,pr->name) == NULL){
	  name = ks_add_char_list(name,ks_new_char_list(pr->name,name_type_num++));
	}
      }
    }
  }
  /*
  for(cp = name; cp != NULL; cp = cp->next){
    printf("%d %s\n",cp->value,cp->name);
  }
  printf("%d\n",num);
  */
  if((name_type_data_num = ks_malloc_int_p(name_type_num,"name_type_data_num")) == NULL){
    draw_error(cb,"ERROR: memory error");
  }
  for(i = 0; i < name_type_num; i++){
    name_type_data_num[i] = 0;
  }
  if((mono = (MONOMER*)ks_malloc(num*sizeof(MONOMER),"mono")) == NULL){
    draw_error(cb,"ERROR: memory error");
  }
  r_max = fr->side[1][1][1][0]*.5;
  gr_num = r_max/dr+1;
  gr_type_num = (int)(name_type_num*(name_type_num+1)*.5);
  /*  printf("%d %d\n",gr_type_num,name_type_num);*/
  if((gr = ks_malloc_int_pp(gr_type_num,gr_num,"gr")) == NULL){
    draw_error(cb,"ERROR: memory error");
  }
  for(i = 0; i < gr_type_num; i++){
    for(j = 0; j < gr_num; j++){
      gr[i][j] = 0;
    }
  }
  if((gr_mat = (int***)ks_malloc(name_type_num*sizeof(int**),"gr_mat")) == NULL){
    draw_error(cb,"ERROR: memory error");
  }
  for(i = 0; i < name_type_num; i++){
    if((gr_mat[i] = (int**)ks_malloc(name_type_num*sizeof(int*),"gr_mat[i]")) == NULL){
      draw_error(cb,"ERROR: memory error");
    }
  }
  k = 0;
  for(i = 0; i < name_type_num; i++){
    for(j = 0; j < name_type_num; j++){
      if(i <= j){
	gr_mat[i][j] = gr[k];
	/*	printf("%d %d is %d %p\n",i,j,k,gr[k]);*/
	k++;
      } else {
	gr_mat[i][j] = gr_mat[j][i];
	/*	printf("%d %d is %d %d %p\n",i,j,j,i,gr_mat[j][i]);*/
      }
    }
  }
  /*  printf("%d %d\n",name_type_num,gr_type_num);*/
  /*
  for(i = 0; i < gr_type_num; i++){
    printf("%d %p\n",i,gr[i]);
  }
  for(i = 0; i < gr_num; i++){
    printf("%d %d\n",i,gr_mat[1][1][i]);
  }
  */

  num = 0;
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	cp = ks_lookup_char_list(name,pr->name);
	ks_assert(cp);
	mono[num].type = cp->value;
	name_type_data_num[cp->value]++;
	for(i = 0; i < 3; i++){
	  mono[num].cd[i] = 0;
	}
	mass = 0;
	for(i = 0; i < pr->particle_num; i++){
	  for(j = 0; j < 3; j++){
	    mono[num].cd[j] += pr->particle[i]->cd[j]*pr->particle[i]->atom->mass;
	  }
	  mass += pr->particle[i]->atom->mass;
	}
	for(i = 0; i < 3; i++){
	  mono[num].cd[i] /= mass;
	}
	num++;
      }
    }
  }
  /*
  for(i = 0; i < num; i++){
    printf("%d %d %f %f %f\n",i,mono[i].type,mono[i].cd[0],mono[i].cd[1],mono[i].cd[2]);
  }
  */
  /*
  printf("side %f %f %f\n",fr->side[1][1][1][0],fr->side[1][1][1][1],fr->side[1][1][1][2]);
  for(i = 0; i < name_type_num; i++){
    printf("%d %d\n",i,name_type_data_num[i]);
  }
  */
  for(i = 0; i < 3; i++){
    side[i] = fr->side[1][1][1][i];
    sideh[i] = side[i]*.5;
  }
  for(i = 0; i < num; i++){
    for(j = i+1; j < num; j++){
      for(k = 0; k < 3; k++){
	dd[k] = mono[i].cd[k] - mono[j].cd[k];
	if(dd[k] < -sideh[k]) dd[k] += side[k];
	if(dd[k] >  sideh[k]) dd[k] -= side[k];
      }
      r = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);
      idr = r/dr;
      /*
      if(mono[i].type == 0 && mono[j].type == 0){
	printf("%3d %3d %d %f\n",i,j,idr,r);
      }
      */
      if(idr < gr_num){
	gr_mat[mono[i].type][mono[j].type][idr] += 2;
      }
    }
  }
  /*
  for(i = 0; i < gr_num; i++){
    printf("%d %d\n",i,gr_mat[1][1][i]);
  }
  */
  {
    FILE *fp;
    char file_name[256];
    ks_remove_file_ext(fr->file_name,file_name,sizeof(file_name));
    strcat(file_name,".mgr");
    printf("%s -> %s\n",fr->file_name,file_name);
    if((fp = fopen(file_name,"wt")) == NULL){
      draw_error(cb,"ERROR: file open error %s",file_name);
    }
    fprintf(fp,"' ");
    for(i = 0; i < name_type_num; i++){
      for(j = i; j < name_type_num; j++){
	cp = ks_lookup_char_list_value(name,i);
	fprintf(fp,"%s",cp->name);
	cp = ks_lookup_char_list_value(name,j);
	fprintf(fp,"-%s ",cp->name);
      }
    }
    fprintf(fp,"\n");
    for(k = 0; k < gr_num; k++){
      r = (k+.5)*dr;
      fprintf(fp,"%f ",r);
      for(i = 0; i < name_type_num; i++){
	for(j = i; j < name_type_num; j++){
	  fprintf(fp," %f",(double)gr_mat[i][j][k]
		  *side[0]*side[1]*side[2]/
		  (name_type_data_num[i]*name_type_data_num[j]*4.0*M_PI*r*r)
		  * (i == j ? 1 : .5)
		  );
	}
      }
      fprintf(fp,"\n");
    }
    fclose(fp);
  }
  ks_free_char_list(name);
  ks_free(mono);
  ks_free_int_pp(gr_type_num,gr);
  for(i = 0; i < name_type_num; i++){
    ks_free(gr_mat[i]);
  }
  ks_free(gr_mat);
  ks_free(name_type_data_num);

  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  cb->idle = NULL;

}
static void set_calc_gr_monomer(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  cb->idle = calc_gr_monomer;
  first_frame(gb,0,0,(void*)cb);
}
static void set_adjust_depth_range(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  cv_adjust_depth_range(gb,cb);
}
static double calc_bond(double cd0[3], double cd1[3])
{
  int i;
  double dd[3];
  for(i = 0; i < 3; i++){
    dd[i] = cd0[i] - cd1[i];
  }
  return sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);
}
static double calc_angle(double cd0[3], double cd1[3], double cd2[3])
{
  int i;
  double vec[2][3];
  for(i = 0; i < 3; i++){
    vec[0][i] = cd0[i] - cd1[i];
    vec[1][i] = cd2[i] - cd1[i];
  }
  return ks_calc_vectors_angle(vec[0],vec[1]);
}
static double calc_torsion(double cd0[3], double cd1[3], double cd2[3], double cd3[3])
{
  int i;
  double vec[2][3];
  double n[2][3];
  double nv[3];
  double ip;
  for(i = 0; i < 3; i++){
    vec[0][i] = cd0[i] - cd1[i];
    vec[1][i] = cd2[i] - cd1[i];
  }
  ks_exterior_product(vec[0],vec[1],n[0]);
  for(i = 0; i < 3; i++){
    vec[0][i] = cd1[i] - cd2[i];
    vec[1][i] = cd3[i] - cd2[i];
  }
  ks_exterior_product(vec[0],vec[1],n[1]);
  ks_exterior_product(n[1],n[0],nv);
  ks_normalize_vector(vec[0],3);
  ks_normalize_vector(nv,3);
  ks_normalize_vector(n[0],3);
  ks_normalize_vector(n[1],3);
  ip = n[0][0]*n[1][0]+n[0][1]*n[1][1]+n[0][2]*n[1][2];
  if(ip >= 1.0){
    return 0.0;
  } else if(ip <= -1.0){
    return M_PI;
  } else {
    return acos(ip)*(vec[0][0]*nv[0]+vec[0][1]*nv[1]+vec[0][2]*nv[2]);
  }
  /*
  printf("%f %f\n",vec[0][0]*nv[0]+vec[0][1]*nv[1]+vec[0][2]*nv[2],ip);
  return ks_calc_vectors_angle(n[0],n[1]);
  */
}
static void set_internal_cd(double cd0[3], double cd1[3], double cd2[3], double cd3[3], 
			    double icd[3])
{
  icd[0] = calc_bond(cd2,cd3);
  icd[1] = calc_angle(cd1,cd2,cd3);
  icd[2] = calc_torsion(cd0,cd1,cd2,cd3);
  /*
  printf("%f %f %f\n",cd0[0],cd0[1],cd0[2]);
  printf("%f %f %f\n",cd1[0],cd1[1],cd1[2]);
  printf("%f %f %f\n",cd2[0],cd2[1],cd2[2]);
  printf("%f %f %f\n",cd3[0],cd3[1],cd3[2]);
  */
}
static void output_internal_cd_particle(FILE *fp, 
					int id0, double cd0[3],
					int id1, double cd1[3],
					int id2, double cd2[3],
					int id3, double cd3[3], char *name, 
					char *type, char topo, double charge)
{
  double icd[3];
  if(cd2 == NULL){
    icd[0] = 0;
  } else {
    icd[0] = calc_bond(cd2,cd3);
  }
  if(cd1 == NULL || cd2 == NULL){
    icd[1] = 0;
  } else {
    icd[1] = calc_angle(cd1,cd2,cd3)/M_PI*180;
  }
  if(cd0 == NULL || cd1 == NULL || cd2 == NULL){
    icd[2] = 0;
  } else {
    icd[2] = calc_torsion(cd0,cd1,cd2,cd3)/M_PI*180;
  }
  if(type == NULL){
    fprintf(fp,"%4d  %-4s  %c%c    %c  % 3d % 3d % 3d   %7.3f % 9.3f % 9.3f  0.00000\n",
	    id3,name,name[0],name[1],
	    name[0] == 'H' ? 'E':'M',
	    id2,id1,id0,icd[0],icd[1],icd[2]);
  } else {
    fprintf(fp,"%4d  %-4s  %-4s  %c  % 3d % 3d % 3d   %7.3f % 9.3f % 9.3f % 8.5f\n",
	    id3,name,type,topo,
	    id2,id1,id0,icd[0],icd[1],icd[2],charge);
  }
}
static void set_dummy_pos(CV_FRAME *fr, double dummy[3][3])
{
  int i,j,k,n[3];
  int ii,jj,kk;
  double default_icd[3][3] = {{1.0,90.0,180.0},{1.0,90.0,180.0},{1.0,90.0,180.0}};
  CV_PARTICLE *p[3];
  CV_BOND *bond[3];

  for(p[0] = fr->particle_end; p[0] != NULL; p[0] = p[0]->prev){
    /*
    printf("%d %s %f %f %f\n"
	   ,p[0]->atype[CV_ATYPE_WORK],p[0]->property->name,p[0]->cd[0],p[0]->cd[1],p[0]->cd[2]);
    for(bond[0] = p[0]->bond; bond[0] != NULL; bond[0] = bond[0]->next){
      printf("  %d %s\n",bond[0]->p->atype[CV_ATYPE_WORK],bond[0]->p->property->name);
    }
    */
    if(strncmp(p[0]->property->name,"DUMM",4) != 0){
      n[0] = count_cv_bond(p[0]->bond);
      for(i = 0; i < n[0]; i++){
	for(ii = 0, bond[0] = p[0]->bond; ii < n[0]-i-1; ii++){
	  bond[0] = bond[0]->next;
	}
	if(bond[0] != NULL && bond[0]->p->atype[CV_ATYPE_WORK] > p[0]->atype[CV_ATYPE_WORK]){
	  /*
	  printf("  i-> %d %d %s %d\n",i,
		 bond[0]->p->atype[CV_ATYPE_WORK],bond[0]->p->property->name,
		 count_cv_bond(bond[0]->p->bond));
	  */
	  p[1] = bond[0]->p;
	  n[1] = count_cv_bond(p[1]->bond);
	  for(j = 0; j < n[1]; j++){
	    for(jj = 0, bond[1] = p[1]->bond; jj < n[1]-j-1; jj++){
	      bond[1] = bond[1]->next;
	    }
	    if(bond[1] != NULL && bond[1]->p->atype[CV_ATYPE_WORK] > p[1]->atype[CV_ATYPE_WORK]){
	      /*
	      printf("    j-> %d %d %s\n",j,
		     bond[1]->p->atype[CV_ATYPE_WORK],bond[1]->p->property->name);
	      */
	      p[2] = bond[1]->p;
	      n[2] = count_cv_bond(p[2]->bond);
	      for(k = 0; k < n[2]; k++){
		for(kk = 0, bond[2] = p[2]->bond; kk < n[2]-k-1; kk++){
		  bond[2] = bond[2]->next;
		}
		if(bond[2]!=NULL && bond[2]->p->atype[CV_ATYPE_WORK] > p[2]->atype[CV_ATYPE_WORK]){
		  /*
		  printf("      k-> %d %d %s\n",k,bond[2]->p->atype[CV_ATYPE_WORK],
			 bond[2]->p->property->name);
		  for(l = 0;l < 3; l++){
		    printf("%d %s %f %f %f\n",p[l]->atype[CV_ATYPE_WORK],p[l]->property->name,
			   p[l]->cd[0],p[l]->cd[1],p[l]->cd[2]);
		  }
		  */
		  calc_internal_cd(p[0]->cd,p[1]->cd,p[2]->cd,default_icd[2],dummy[2]);
		  calc_internal_cd(dummy[2],p[0]->cd,p[1]->cd,default_icd[1],dummy[1]);
		  calc_internal_cd(dummy[1],dummy[2],p[0]->cd,default_icd[0],dummy[0]);

		  set_internal_cd(dummy[0],dummy[1],dummy[2],p[0]->cd,p[0]->move); 
		  set_internal_cd(dummy[1],dummy[2],p[0]->cd,p[1]->cd,p[1]->move); 
		  set_internal_cd(dummy[2],p[0]->cd,p[1]->cd,p[2]->cd,p[2]->move); 
		  /*
		  for(l = 0;l < 3; l++){
		    printf("d%d %f %f %f\n",l,dummy[l][0],dummy[l][1],dummy[l][2]);
		  }
		  */
		  return;
		}
	      }
	    }
	  }
	}
      }
    }
  }
}
static void output_internal_cd(FILE *fp, double dummy[3][3], CV_PARTICLE *particle,
			       char *type, char topo, double charge)
{
  CV_BOND *bond[3];
  CV_PARTICLE *p[3];

  /*  fp = stdout;*/
  /*
  printf("%d %d %s %f %f %f\n",particle->label,particle->atype[CV_ATYPE_WORK],
	 particle->property->name,particle->cd[0],particle->cd[1],particle->cd[2]);
  for(bond[0] = particle->bond; bond[0] != NULL; bond[0] = bond[0]->next){
    p[0] = bond[0]->p;
    printf("all %d %d %s %f %f %f\n",p[0]->label,p[0]->atype[CV_ATYPE_WORK],
	   p[0]->property->name,p[0]->cd[0],p[0]->cd[1],p[0]->cd[2]);
  }
  */
  if(particle->atype[CV_ATYPE_WORK] == 4){
    output_internal_cd_particle(fp,1,dummy[0],2,dummy[1],3,dummy[2],4,particle->cd,
				particle->property->name,type,topo,charge);
    return;
  }
  for(bond[0] = particle->bond; bond[0] != NULL; bond[0] = bond[0]->next){
    if(bond[0]->p->atype[CV_ATYPE_WORK] < particle->atype[CV_ATYPE_WORK]){
      p[0] = bond[0]->p;
      if(p[0]->atype[CV_ATYPE_WORK] == 4){
	output_internal_cd_particle(fp,2,dummy[1],3,dummy[2],4,p[0]->cd,
				    particle->atype[CV_ATYPE_WORK],particle->cd,
				    particle->property->name,type,topo,charge);
	bond[0]->flags |= CV_BOND_OUTPUTED;
	return;
      }
      /*
      printf("    %d %d %s %f %f %f\n",p[0]->label,p[0]->atype[CV_ATYPE_WORK],
	     p[0]->property->name,p[0]->cd[0],p[0]->cd[1],p[0]->cd[2]);
      */
      for(bond[1] = p[0]->bond; bond[1] != NULL; bond[1] = bond[1]->next){
	if(bond[1]->p->atype[CV_ATYPE_WORK] < p[0]->atype[CV_ATYPE_WORK]){
	  p[1] = bond[1]->p;
	  if(p[1]->atype[CV_ATYPE_WORK] == 4){
	    output_internal_cd_particle(fp,3,dummy[2],
					p[1]->atype[CV_ATYPE_WORK],p[1]->cd,
					p[0]->atype[CV_ATYPE_WORK],p[0]->cd,
					particle->atype[CV_ATYPE_WORK],particle->cd,
					particle->property->name,type,topo,charge);
	    bond[0]->flags |= CV_BOND_OUTPUTED;
	    return;
	  }
	  /*
	  printf("      %d %d %s %f %f %f\n",p[1]->label,p[1]->atype[CV_ATYPE_WORK],
		 p[1]->property->name,p[1]->cd[0],p[1]->cd[1],p[1]->cd[2]);
	  */
	  for(bond[2] = p[1]->bond; bond[2] != NULL; bond[2] = bond[2]->next){
	    if(bond[2]->p->atype[CV_ATYPE_WORK] < p[1]->atype[CV_ATYPE_WORK]){
	      p[2] = bond[2]->p;
	      /*
	      printf("         %d %d %s %f %f %f hit\n",p[2]->label,p[2]->atype[CV_ATYPE_WORK],
		     p[2]->property->name,p[2]->cd[0],p[2]->cd[1],p[2]->cd[2]);
	      */
	      ks_assert(p[0] != p[1]);
	      ks_assert(p[0] != p[2]);
	      ks_assert(p[1] != p[2]);
	      output_internal_cd_particle(fp,
					  p[2]->atype[CV_ATYPE_WORK],p[2]->cd,
					  p[1]->atype[CV_ATYPE_WORK],p[1]->cd,
					  p[0]->atype[CV_ATYPE_WORK],p[0]->cd,
					  particle->atype[CV_ATYPE_WORK],particle->cd,
					  particle->property->name,type,topo,charge);
	      bond[0]->flags |= CV_BOND_OUTPUTED;
	      return;
	    }
	  }
	}
      }
    }
  }

  if(particle->atype[CV_ATYPE_WORK] == 1){
    output_internal_cd_particle(fp,-2,NULL,-1,NULL,0,NULL,
				particle->atype[CV_ATYPE_WORK],particle->cd,
				particle->property->name,type,topo,charge);
    return;
  } else if(particle->atype[CV_ATYPE_WORK] == 2){
    for(bond[0] = particle->bond; bond[0] != NULL; bond[0] = bond[0]->next){
      if(bond[0]->p->atype[CV_ATYPE_WORK] < particle->atype[CV_ATYPE_WORK]){
	p[0] = bond[0]->p;
	if(p[0]->atype[CV_ATYPE_WORK] == 1){
	  output_internal_cd_particle(fp,-1,NULL,0,NULL,
				      p[0]->atype[CV_ATYPE_WORK],p[0]->cd,
				      particle->atype[CV_ATYPE_WORK],particle->cd,
				      particle->property->name,type,topo,charge);
	  return;
	}
      }
    }
  } else if(particle->atype[CV_ATYPE_WORK] == 3){
    for(bond[0] = particle->bond; bond[0] != NULL; bond[0] = bond[0]->next){
      if(bond[0]->p->atype[CV_ATYPE_WORK] < particle->atype[CV_ATYPE_WORK]){
	p[0] = bond[0]->p;
	if(p[0]->atype[CV_ATYPE_WORK] == 2){
	  for(bond[1] = p[0]->bond; bond[1] != NULL; bond[1] = bond[1]->next){
	    if(bond[1]->p->atype[CV_ATYPE_WORK] < p[0]->atype[CV_ATYPE_WORK]){
	      p[1] = bond[1]->p;
	      if(p[1]->atype[CV_ATYPE_WORK] == 1){
		output_internal_cd_particle(fp,0,NULL,
					    p[1]->atype[CV_ATYPE_WORK],p[1]->cd,
					    p[0]->atype[CV_ATYPE_WORK],p[0]->cd,
					    particle->atype[CV_ATYPE_WORK],particle->cd,
					    particle->property->name,type,topo,charge);
		return;
	      }
	    }
	  }
	}
      }
    }
  }
  ks_error("%d %d %s %f %f %f is not output",particle->label,particle->atype[CV_ATYPE_WORK],
	   particle->property->name,particle->cd[0],particle->cd[1],particle->cd[2]);
  //  ks_assert(0);
}
void reset_bond_outputed(CV_FRAME *fr)
{
  CV_PARTICLE *p;
  CV_BOND *bond;
  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(bond = p->bond; bond != NULL; bond = bond->next){
      bond->flags &= ~CV_BOND_OUTPUTED;
    }
  }
}
static void output_prep_improper(FILE *fp, CV_FRAME *fr)
{
  KS_AMBER_PREP_BLOCK *pb;
  KS_INT_LIST_LIST *pii;
  KS_INT_LIST *pi;
  char ***name;
  int i,j;
  int n;

  pb = fr->prep_block;
  n = ks_count_int_list_list(pb->improper);
  if(n == 0){
    return;
  }
  if((name = (char***)ks_malloc(n*sizeof(char**),"name")) == NULL){
    ks_error_memory();
    ks_exit(EXIT_FAILURE);
  }
  for(i = 0; i < n; i++){
    if((name[i] = (char**)ks_malloc(4*sizeof(char*),"name[i]")) == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
  }

  for(pii = pb->improper, i = 0; pii != NULL; pii = pii->next,i++){
    for(pi = pii->p,j = 0; pi != NULL; pi = pi->next,j++){
      if(pi->value == -2){
	if((name[i][j] = ks_malloc_char_copy("-M   ","name[i][j]")) == NULL){
	  ks_error_memory();
	  ks_exit(EXIT_FAILURE);
	}
      } else if(pi->value == -1){
	if((name[i][j] = ks_malloc_char_copy("+M   ","name[i][j]")) == NULL){
	  ks_error_memory();
	  ks_exit(EXIT_FAILURE);
	}
      } else {
	if((name[i][j] = ks_malloc_char_copy(pb->atom[pi->value].name,"name[i][j]")) == NULL){
	  ks_error_memory();
	  ks_exit(EXIT_FAILURE);
	}
      }
    }
  }
  fprintf(fp,"\nIMPROPER\n");
  for(i = n-1; i >= 0; i--){
    for(j = 0; j < 4; j++){
      fprintf(fp,"%-4s",name[i][j]);
    }
    if(j < 3){
      fprintf(fp," ");
    } else {
      fprintf(fp,"\n");
    }
  }
  for(i = 0; i < n; i++){
    for(j = 0; j < 4; j++){
      ks_free(name[i][j]);
    }
  }
  for(i = 0; i < n; i++){
    ks_free(name[i]);
  }
  ks_free(name);
}
static void output_prep_loop(FILE *fp, CV_FRAME *fr)
{
  CV_PARTICLE *p;
  CV_BOND *bond,*bond2;
  BOOL first = KS_TRUE;
  BOOL outputed;

  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(bond = p->bond; bond != NULL; bond = bond->next){
      /*      printf("%d %d\n",p->label,bond->p->label);*/
      if(p->atype[CV_ATYPE_WORK] > 4 && !(bond->flags&CV_BOND_OUTPUTED)){
	/*
	printf("%2d %2d %-4s %2d %2d %-4s %d\n",p->label,p->atype[CV_ATYPE_WORK],p->property->name,
	       bond->p->label,bond->p->atype[CV_ATYPE_WORK],bond->p->property->name,
	       bond->flags&CV_BOND_OUTPUTED);
	*/
	outputed = KS_FALSE;
	for(bond2 = bond->p->bond; bond2 != NULL; bond2 = bond2->next){
	  if(bond2->p == p){
	    /*
	    printf("  %2d %2d %-4s %2d %2d %-4s %d\n",
		   bond->p->label,bond->p->atype[CV_ATYPE_WORK],bond->p->property->name,
		   bond2->p->label,p->atype[CV_ATYPE_WORK],bond2->p->property->name,
		   bond2->flags&CV_BOND_OUTPUTED);
	    */
	    if(bond2->flags&CV_BOND_OUTPUTED){
	      outputed = KS_TRUE;
	    }
	  }
	}
	if(outputed == KS_FALSE && p->label < bond->p->label){
	  if(first == KS_TRUE){
	    first = KS_FALSE;
	    fprintf(fp,"\nLOOP\n");
	  }
	  fprintf(fp,"%-4s %-4s\n",p->property->name,bond->p->property->name);
	}
      }
    }
  }
}
static void output_prep_file(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;
  FILE *fp;
  char file_name[256];
  char residue_type[4] = {"   "};
  double dummy[3][3];

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  for(p = fr->particle_end, i = 4; p != NULL; p = p->prev, i++){
    p->atype[CV_ATYPE_WORK] = i;
  }

  set_dummy_pos(fr,dummy);
  /*
  for(i = 0; i < 3; i++){
    printf("%d %f %f %f\n",i,dummy[i][0],dummy[i][1],dummy[i][2]);
  }
  */
  /*
  for(p = fr->particle_end; p != NULL; p = p->prev){
    printf("%d %d %s %f %f %f\n"
	   ,p->label,p->atype[CV_ATYPE_WORK],p->property->name,p->move[0],p->move[1],p->move[2]);
  }
  */
  if(text[0] == '\0'){
    strcpy(residue_type,"UKN");
  } else {
    for(i = 0; text[i]; i++){
      if(text[i]){
	residue_type[i] = text[i];
      }
    }
  }

  ks_remove_file_ext(fr->file_name,file_name,sizeof(file_name));
  strcat(file_name,"_cv.prep");
  printf("%s -> %s\n",fr->file_name,file_name);
  if((fp = fopen(file_name,"wt")) == NULL){
    draw_error(cb,"ERROR: file open error %s",file_name);
  }
  fprintf(fp,"    1    1    2\n");
  fprintf(fp,"%s\n",file_name);
  fprintf(fp,"PREP file created by cdview\n");
  fprintf(fp,"\n");
  fprintf(fp," %c%c%c  INT     1\n",
	  toupper(((unsigned char*)residue_type)[0]),
	  toupper(((unsigned char*)residue_type)[1]),
	  toupper(((unsigned char*)residue_type)[2]));
  fprintf(fp," CORR OMIT DU   BEG\n");
  fprintf(fp,"   0.00000\n");
  fprintf(fp,"   1  DUMM  DU    M    0  -1  -2     0.000     0.000     0.000  0.00000\n");
  fprintf(fp,"   2  DUMM  DU    M    1   0  -1     1.000     0.000     0.000  0.00000\n");
  fprintf(fp,"   3  DUMM  DU    M    2   1   0     1.000    90.000     0.000  0.00000\n");

  reset_bond_outputed(fr);
  for(p = fr->particle_end; p != NULL; p = p->prev){
    output_internal_cd(fp,dummy,p,NULL,0,0);
  }
  fprintf(fp,"\n");
  fprintf(fp,"DONE\n");
  fprintf(fp,"STOP\n");
  fclose(fp);

  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(i = 0; i < 3; i++){
      p->move[i] = 0;
    }
  }
}
static void output_prep_file_from_prep(KS_GL_BASE *gb, char *text, void *vp)
{
  int i,j;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;
  FILE *fp;
  double dummy[3][3];
  
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  for(p = fr->particle_end, i = 1; p != NULL; p = p->prev, i++){
    p->atype[CV_ATYPE_WORK] = i;
  }
  for(i = 0; i < 3; i++){
    /*
    printf("%d %s %f %f %f\n",i,fr->aggregate->chain->residue->particle[i]->property->name,
	   fr->aggregate->chain->residue->particle[i]->cd[0],
	   fr->aggregate->chain->residue->particle[i]->cd[1],
	   fr->aggregate->chain->residue->particle[i]->cd[2]);
    */
    for(j = 0; j < 3; j++){
      dummy[i][j] = fr->aggregate->chain->residue->particle[i]->cd[j];
    }
  }

  printf("%s -> %s\n",fr->file_name,text);
  if((fp = fopen(text,"wt")) == NULL){
    draw_error(cb,"ERROR: file open error %s",text);
  }
  fprintf(fp,"    1    1    2\n");
  fprintf(fp,"%s\n",text);
  fprintf(fp,"PREP file created by cdview\n");
  fprintf(fp,"\n");
  fprintf(fp," %s  INT     1\n",fr->prep_block->type);
  fprintf(fp," CORR OMIT DU   BEG\n");
  fprintf(fp,"   0.00000\n");
  reset_bond_outputed(fr);
  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(i = 0; i < fr->prep_block->atom_num; i++){
      if(strcmp(p->property->name,fr->prep_block->atom[i].name) == 0){
	break;
      }
    }
    output_internal_cd(fp,dummy,p,
		       fr->prep_block->atom[i].type,
		       p->flags&CV_PARTICLE_MAIN ? 'M':
		       fr->prep_block->atom[i].topological_type == 'M' ? 
		       'B':fr->prep_block->atom[i].topological_type,
		       fr->prep_block->atom[i].charge);
    /*
    printf("%d %s %s %c %f\n",
	   i,
	   fr->prep_block->atom[i].name,
	   fr->prep_block->atom[i].type,
	   fr->prep_block->atom[i].topological_type,
	   fr->prep_block->atom[i].charge);
    */
  }

  output_prep_improper(fp,fr);
  output_prep_loop(fp,fr);

  fprintf(fp,"\n");
  fprintf(fp,"DONE\n");
  fprintf(fp,"STOP\n");
  fclose(fp);
}
void add_file_name(char *file_name, char *add, 
		   char *output_file_name, size_t output_file_name_size)
{
  int i;
  char *cp;
  for(cp = file_name; *cp; cp++);
  for(;cp != file_name && *cp != '.'; cp--);
  for(i = 0; &file_name[i] != cp && i < output_file_name_size+strlen(add)+strlen(cp); i++)
    output_file_name[i] = file_name[i];
  output_file_name[i] = '\0';
  strcat(output_file_name,add);
  strcat(output_file_name,cp);
}
static void set_output_prep_file(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  if(cb->frame[cb->current_frame].flags&CV_FRAME_PREP){
    char file_name[256];
    add_file_name(cb->frame[cb->current_frame].file_name,"_cv",file_name,sizeof(file_name));
    ks_set_gl_input_key(gb,"file name > ",file_name,cb->input_key_font,
			output_prep_file_from_prep,KS_FALSE,cb);
  } else if(cb->frame[cb->current_frame].flags&CV_FRAME_PDB){
    ks_set_gl_input_key(gb,"residue name > ",NULL,cb->input_key_font,
			output_prep_file,KS_FALSE,cb);
  } else {
    draw_error(cb,"ERROR: prep file can be outputed from pdb or prep files");
  }
}
static void move_prep_dummy(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  int i,j;
  CV_FRAME *fr;
  CV_PARTICLE *p,*p1;
  double dummy[3][3];
  double dd[3],r;

  fr = &cb->frame[cb->current_frame];

  if(fr->prep_block == NULL){
    return;
  }
  
  for(p = fr->particle_end,i = 1; p != NULL; p = p->prev, i++){
    p->atype[CV_ATYPE_WORK] = i;
  }
  set_dummy_pos(fr,dummy);
  /*
  for(i = 0; i < 3; i++){
    printf("dummy %d %f %f %f\n",i,dummy[i][0],dummy[i][1],dummy[i][2]);
  }
  */

  for(p = fr->particle_end,i = 0; p != NULL; p = p->prev){
    if(strncmp(p->property->name,"DUMM",4) == 0){
      for(j = 0; j < 3; j++){
	p->cd[j] = dummy[i][j];
      }
      /*
      printf("%d %s %f %f %f\n",
	     p->atype[CV_ATYPE_WORK],p->property->name,p->cd[0],p->cd[1],p->cd[2]);
      */
      if(i == 2){
	p->bond->p->bond = del_cv_bond(p->bond->p->bond,p);
	p->bond = del_cv_bond(p->bond,p->bond->p);
	p1 = p->prev;
	for(j = 0; j < 3; j++){
	  dd[j] = p->cd[j] - p1->cd[j];
	}
	r = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);
	p->bond = add_cv_bond(p->bond, new_cv_bond(0,p1,r,&fr->reuse->bond));
	p1->bond = add_cv_bond(p1->bond, new_cv_bond(0,p,r,&fr->reuse->bond));
	break;
      }
      i++;
    }
  }

  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(i = 0; i < 3; i++){
      p->move[i] = 0;
    }
  }
}
static void change_show_selected_particle_range(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  char msg[256];
  cb = (CV_BASE*)vp;
  cb->show_selected_particle_range = atof(text);
  sprintf(msg,"show_selected_particle_range is %f",cb->show_selected_particle_range);
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,2));
}
static void set_change_show_selected_particle_range(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_CHANGE_SHOW_SELECTED_PARTICLE_RANGE,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input range > ",NULL,cb->input_key_font,
			change_show_selected_particle_range,KS_FALSE,cb);
  } else {
    change_show_selected_particle_range(gb,arg,(void*)cb);
  }
}
static void clip_plane(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  char msg[256];
  double pos[3];
  int i,j;

  cb = (CV_BASE*)vp;

  for(i = 0; i < 3; i++){
    pos[i] = cb->reference_frame->center[i];
  }
  if(text[0] == 'x'){
    pos[0] += atof(&text[1]);
    j = 0;
  } else if(text[0] == 'y'){
    pos[1] += atof(&text[1]);
    j = 1;
  } else if(text[0] == 'z'){
    pos[2] += atof(&text[1]);
    j = 2;
  } else if(text[0] == '-' && text[1] == 'x'){
    pos[0] -= atof(&text[2]);
    j = 3;
  } else if(text[0] == '-' && text[1] == 'y'){
    pos[1] -= atof(&text[2]);
    j = 4;
  } else if(text[0] == '-' && text[1] == 'z'){
    pos[2] -= atof(&text[2]);
    j = 5;
  } else {
    pos[0] += atof(text);
    j = 0;
  }
  cb->clip_plane_flg[j] = KS_TRUE;
  cb->clip_plane[j][3] = 0;
  for(i = 0; i < 3; i++){
    cb->clip_plane[j][3] -= cb->clip_plane[j][i]*pos[i];
  }  
  sprintf(msg,"value of clip plane %d %f %f %f %f",j,
	  cb->clip_plane[j][0],cb->clip_plane[j][1],cb->clip_plane[j][2], cb->clip_plane[j][3]);
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,2));
}
static void malloc_clip_plane(CV_BASE *cb)
{
  int i,j;
  if(cb->clip_plane == NULL){
    if((cb->clip_plane_flg = ks_malloc_uint_p(6,"cb->clip_plane_flg")) == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
    if((cb->clip_plane = ks_malloc_double_pp(6,4,"cb->clip_plane")) == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
  }
  for(i = 0; i < 6; i++){
    cb->clip_plane_flg[i] = KS_FALSE;
    for(j = 0; j < 4; j++){
      cb->clip_plane[i][j] = 0;
    }
  }
  cb->clip_plane[0][0] = -1.0;
  cb->clip_plane[1][1] = -1.0;
  cb->clip_plane[2][2] = -1.0;
  cb->clip_plane[3][0] =  1.0;
  cb->clip_plane[4][1] =  1.0;
  cb->clip_plane[5][2] =  1.0;
}
static void set_clip_plane(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;

  malloc_clip_plane(cb);
  arg = get_command_arg(cb,CV_COMMAND_CLIP_PLANE,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"clip value > ",NULL,cb->input_key_font,
			clip_plane,KS_FALSE,cb);
  } else {
    clip_plane(gb,arg,(void*)cb);
  }
}
static void add_clip_plane(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;

  if(cb->clip_plane == NULL){
    malloc_clip_plane(cb);
  }
  arg = get_command_arg(cb,CV_COMMAND_ADD_CLIP_PLANE,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"clip value > ",NULL,cb->input_key_font,
			clip_plane,KS_FALSE,cb);
  } else {
    clip_plane(gb,arg,(void*)cb);
  }
}
static void rot_cd(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;
  char msg[256];
  double angle;
  double cd[3];
  int i;
  char *c,*d;
  int chain_label = -1;
  char str[64];

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(text[0] == 'x' || text[0] == 'y' || text[0] == 'z'){
    for(c = text; !isdigit(*(unsigned char*)c) && *c != '-' && *c != '\0'; c++);
    if(*c != '\0'){
      d = strstr(c,"c");
      if(d != NULL){
	for(i = 0; c != d && i < sizeof(str); i++,c++){
	  str[i] = *c;
	}
	for(; !isdigit(*(unsigned char*)d) && *d != '-' && *d != '\0'; d++);
	angle = atof(str);
	chain_label = atoi(d)-1;
      } else {
	angle = atof(c);
      }
      angle *= M_PI/180.;
      for(p = fr->particle; p != NULL; p = p->next){
	if(chain_label == -1 || (p->property != NULL && p->property->chain->label == chain_label)){
	  for(i = 0; i < 3; i++){
	    cd[i] = p->cd[i] - fr->center[i];
	  }
	  if(text[0] == 'x'){
	    p->cd[0] = cd[0];
	    p->cd[1] = cd[1]*cos(angle) - cd[2]*sin(angle);
	    p->cd[2] = cd[1]*sin(angle) + cd[2]*cos(angle);
	  } else if(text[0] == 'y'){
	    p->cd[0] = cd[0]*cos(angle) - cd[2]*sin(angle);
	    p->cd[1] = cd[1];
	    p->cd[2] = cd[0]*sin(angle) + cd[2]*cos(angle);
	  } else if(text[0] == 'z'){
	    p->cd[0] = cd[0]*cos(angle) - cd[1]*sin(angle);
	    p->cd[1] = cd[0]*sin(angle) + cd[1]*cos(angle);
	    p->cd[2] = cd[2];
	  } else {
	    ks_assert(0);
	  }
	  for(i = 0; i < 3; i++){
	    p->cd[i] += fr->center[i];
	  }
	}
      }
      return;
    }
  }
  sprintf(msg,"Please input 'direction (x, y or z)' and 'angle'");
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,2));
}
static void set_rot_cd(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_ROTATION,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input angle > ",NULL,cb->input_key_font,rot_cd,KS_FALSE,cb);
  } else {
    rot_cd(gb,arg,(void*)cb);
  }
}
static void trans_cd(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;
  char msg[256];
  double len;
  char *c,*d;
  int i;
  int chain_label = -1;
  char str[64];

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(text[0] == 'x' || text[0] == 'y' || text[0] == 'z'){
    for(c = text; !isdigit(*(unsigned char*)c) && *c != '-' && *c != '\0'; c++);
    if(*c != '\0'){
      d = strstr(c,"c");
      if(d != NULL){
	for(i = 0; c != d && i < sizeof(str); i++,c++){
	  str[i] = *c;
	}
	for(; !isdigit(*(unsigned char*)d) && *d != '-' && *d != '\0'; d++);
	len = atof(str);
	chain_label = atoi(d)-1;
      } else {
	len = atof(c);
      }
      for(p = fr->particle; p != NULL; p = p->next){
	if(chain_label == -1 || (p->property != NULL && p->property->chain->label == chain_label)){
	  if(text[0] == 'x'){
	    p->cd[0] += len;
	  } else if(text[0] == 'y'){
	    p->cd[1] += len;
	  } else if(text[0] == 'z'){
	    p->cd[2] += len;
	  } else {
	    ks_assert(0);
	  }
	}
      }
      return;
    }
  }
  sprintf(msg,"Please input 'direction (x, y or z)' and 'length'");
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,2));
}
static void set_trans_cd(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_TRANSLATION,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input trans > ",NULL,cb->input_key_font,trans_cd,KS_FALSE,cb);
  } else {
    trans_cd(gb,arg,(void*)cb);
  }
}
static void set_c_color_num_min_max(KS_GL_BASE *gb, char *text, void *vp)
{
  int i,j,k;
  CV_BASE *cb;
  char str[3][64];
  int n;

  cb = (CV_BASE*)vp;
  for(k = 0, i = 0; k < 3; k++){
    str[k][0] = '\0';
  }
  for(k = 0, i = 0; k < 3; k++){
    for(;text[i] == ' ' && text[i] != '\n'; i++);
    for(j = 0; text[i] != ' ' && text[i] != '\n' && i < sizeof(str[k]); i++,j++){
      str[k][j] = text[i];
    }
    str[k][j] = '\0';
  }

  if(str[0][0] != '\0' && str[1][0] != '\0' && str[2][0] != '\0'){
    n = atoi(str[0]);
    if((cb->continuous_color.num != 0 && n <= cb->continuous_color.num) || 
       (cb->continuous_color.num == 0 && n <= 4 )){
      cb->continuous_color.num = n;
    }
    if(cb->continuous_color.num  > 4) cb->continuous_color.num = 0;
    for(i = 0; i < cb->frame_num; i++){
      for(j = 0; j < cb->continuous_color.num; j++){
	cb->frame[i].c_color_info.min[j] = atof(str[1]);
	cb->frame[i].c_color_info.max[j] = atof(str[2]);
	cb->frame[i].c_color_info.set_min_max = KS_TRUE;
      }
      set_c_color_min_max(&cb->frame[i],&cb->continuous_color,KS_FALSE);
    }
  } else if(str[0][0] != '\0' && str[1][0] != '\0' && str[2][0] == '\0'){
    for(i = 0; i < cb->frame_num; i++){
      for(j = 0; j < cb->continuous_color.num; j++){
	cb->frame[i].c_color_info.min[j] = atof(str[0]);
	cb->frame[i].c_color_info.max[j] = atof(str[1]);
	cb->frame[i].c_color_info.set_min_max = KS_TRUE;
      }
      set_c_color_min_max(&cb->frame[i],&cb->continuous_color,KS_FALSE);
    }
  } else if(str[0][0] != '\0' && str[1][0] == '\0' && str[2][0] == '\0'){
    n = atoi(str[0]);
    if((cb->continuous_color.num != 0 && n <= cb->continuous_color.num) || 
       (cb->continuous_color.num == 0 && n <= 4 )){
      cb->continuous_color.num = n;
    }
    if(cb->continuous_color.num  > 4) cb->continuous_color.num = 0;
    for(i = 0; i < cb->frame_num; i++){
      set_c_color_min_max(&cb->frame[i],&cb->continuous_color,KS_TRUE);
    }
  }
}
static void set_c_color(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_SET_CONTINUOUS_COLOR_NUM,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input num [min] [max] > ",NULL,
			cb->input_key_font,set_c_color_num_min_max,KS_FALSE,cb);
  } else {
    set_c_color_num_min_max(gb,arg,(void*)cb);
  }
}
static void eye_angle(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(ks_isfloat_all(text)){
    ks_set_gl_perspective_angle(gb,atof(text));
  } else {
    draw_error(cb,"ERROR: '%s' is not number",text);
  }
}
static void set_eye_angle(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_SET_EYE_ANGLE,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input angle > ",NULL,
			cb->input_key_font,eye_angle,KS_FALSE,cb);
  } else {
    eye_angle(gb,arg,(void*)cb);
  }
}
static void rot_mol(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  char *c;
  double angle[3];
  char msg[256];
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;
  CV_PARTICLE *p;
  int i,num;
  double center_pos[3],cd[3],cd_temp[3];
  BOOL hit = KS_FALSE;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  if(text[0] == 'x' || text[0] == 'y' || text[0] == 'z' || text[0] == 'r'){
    for(c = text; !isdigit(*(unsigned char*)c) && *c != '-' && *c != '\0'; c++);
    for(c = text; !isdigit(*(unsigned char*)c) && *c != '-' && *c != '\0'; c++);
    if(*c != '\0' || text[0] == 'r'){
      if(text[0] != 'r'){
	angle[0] = atof(c);
      } else {
	angle[0] = (double)rand()/(double)RAND_MAX*2.0*M_PI;
	angle[1] = (double)rand()/(double)RAND_MAX*2.0*M_PI;
	angle[2] = (double)rand()/(double)RAND_MAX*2.0*M_PI;
      }
      for(pa = fr->aggregate; pa != NULL; pa = pa->next){
	//	printf("agg %d %c\n",pa->label,pa->flags&CV_AGGREGATE_SELECTED? 'o':'x');
	for(pc = pa->chain; pc != NULL; pc = pc->next){
	  //	  printf("  chain %d %s %c\n",pc->label,pc->name,pc->flags&CV_CHAIN_SELECTED? 'o':'x');
	  if(pc->flags&CV_CHAIN_SELECTED){
	    hit = KS_TRUE;
	    num = 0;
	    center_pos[0] = 0; center_pos[1] = 0; center_pos[2] = 0;
	    for(pr = pc->residue; pr != NULL; pr = pr->next){
	      for(i = 0; i < pr->particle_num; i++){
		p = pr->particle[i];
		/*
		printf("      %d %d %s '%s' %p\n",i,p->file_label,p->atom->name,
		       p->property->name,p->property->name);
		*/
		++num;
		center_pos[0] += p->cd[0];
		center_pos[1] += p->cd[1];
		center_pos[2] += p->cd[2];
	      }
	    }
	    center_pos[0] /= num;
	    center_pos[1] /= num;
	    center_pos[2] /= num;
	    //	    printf("center_pos %f %f %f\n",center_pos[0],center_pos[1],center_pos[2]);
	    for(pr = pc->residue; pr != NULL; pr = pr->next){
	      for(i = 0; i < pr->particle_num; i++){
		p = pr->particle[i];
		cd[0] = p->cd[0] - center_pos[0];
		cd[1] = p->cd[1] - center_pos[1];
		cd[2] = p->cd[2] - center_pos[2];
		if(text[0] == 'x'){
		  p->cd[0] = cd[0];
		  p->cd[1] = cd[1]*cos(angle[0]) - cd[2]*sin(angle[0]);
		  p->cd[2] = cd[1]*sin(angle[0]) + cd[2]*cos(angle[0]);
		} else if(text[0] == 'y'){
		  p->cd[0] = cd[0]*cos(angle[0]) - cd[2]*sin(angle[0]);
		  p->cd[1] = cd[1];
		  p->cd[2] = cd[0]*sin(angle[0]) + cd[2]*cos(angle[0]);
		} else if(text[0] == 'z'){
		  p->cd[0] = cd[0]*cos(angle[0]) - cd[1]*sin(angle[0]);
		  p->cd[1] = cd[0]*sin(angle[0]) + cd[1]*cos(angle[0]);
		  p->cd[2] = cd[2];
		} else if(text[0] == 'r'){
		  cd_temp[0] = cd[0];
		  cd_temp[1] = cd[1]*cos(angle[0]) - cd[2]*sin(angle[0]);
		  cd_temp[2] = cd[1]*sin(angle[0]) + cd[2]*cos(angle[0]);
		  cd[0] = cd_temp[0]; cd[1] = cd_temp[1]; cd[2] = cd_temp[2];
		  cd_temp[0] = cd[0]*cos(angle[1]) - cd[2]*sin(angle[1]);
		  cd_temp[1] = cd[1];
		  cd_temp[2] = cd[0]*sin(angle[1]) + cd[2]*cos(angle[1]);
		  cd[0] = cd_temp[0]; cd[1] = cd_temp[1]; cd[2] = cd_temp[2];
		  p->cd[0] = cd[0]*cos(angle[2]) - cd[1]*sin(angle[2]);
		  p->cd[1] = cd[0]*sin(angle[2]) + cd[1]*cos(angle[2]);
		  p->cd[2] = cd[2];
		} else {
		  ks_assert(0);
		}
		p->cd[0] += center_pos[0];
		p->cd[1] += center_pos[1];
		p->cd[2] += center_pos[2];
	      }
	    }
	  }
	}
      }
      if(hit){
	return;
      } else {
	sprintf(msg,"Please select molecule(s) (use triple click!)");
	cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,10));
	return;
      }
    }
  }
  sprintf(msg,"Please input 'direction (x, y or z)' and 'angle'");
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(msg,10));
}
static void set_rot_mol(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *arg;
  arg = get_command_arg(cb,CV_COMMAND_SET_ROT_MOL,text);
  if(*arg == '\0'){
    ks_set_gl_input_key(gb,"input direction [x|y|z|r(random)] (and angle) > ",NULL,
			cb->input_key_font,rot_mol,KS_FALSE,cb);
  } else {
    eye_angle(gb,arg,(void*)cb);
  }
}
static void init_light_pos(GLfloat *light_pos)
{
  char *cp;
  light_pos[0] = 1.0;  /* y */
  light_pos[1] = 1.1;  /* z */
  light_pos[2] = 1.2;  /* x */
  light_pos[3] = 0.0;

  if((cp = getenv(CV_ENV_KEY_LIGHT_POSITION_X)) != NULL){
    sscanf(cp,"%f",&light_pos[2]);
  }
  if((cp = getenv(CV_ENV_KEY_LIGHT_POSITION_Y)) != NULL){
    sscanf(cp,"%f",&light_pos[0]);
  }
  if((cp = getenv(CV_ENV_KEY_LIGHT_POSITION_Z)) != NULL){
    sscanf(cp,"%f",&light_pos[1]);
  }

}
static BOOL set_prep_file(CV_BASE *cb, char *file_name)
{
  int i,c;
  KS_AMBER_PREP_BLOCK *pb;
  int prep_block_num = 0;
  printf("read prep %s\n",file_name);
  fflush(stdout);
  if((cb->prep = ks_new_amber_prep(file_name,NULL)) == NULL){
    return KS_FALSE;
  }
  for(pb = cb->prep->block; pb != NULL; pb = pb->next){
    /*
    printf("  name: '%s'\n",pb->name);
    printf("  type: '%s'\n",pb->type);
    */
    ++prep_block_num;
  }

  if((cb->frame = allocate_frame(prep_block_num,cb->atype_mode,&cb->fi)) == NULL){
    return KS_FALSE;
  }
  for(i = 0; i < prep_block_num; i++){
    // skip data to get pb from the end of cb->prep->block
    for(pb = cb->prep->block, c = 0; pb != NULL && c < prep_block_num-i-1; pb = pb->next,c++);
    strcpy(cb->frame[cb->frame_num].file_name,file_name);
    cb->frame[cb->frame_num].prep_block = pb;
    cb->frame_num++;
  }
  assert(cb->frame_num == prep_block_num);

  cb->frame[0].flags |= CV_FRAME_REFERENCE;

  return KS_TRUE;
}
static void init_atom_parm()
{
  int i;
  for(i = CV_ATOM_TYPE_INI_CNT; i < CV_ATOM_TYPE_MAX; i++){
    init_cv_atom_parm[i] = init_cv_atom_parm[CV_ATOM_TYPE_INI_CNT-1];
  }
}
CV_BASE* cv_allocate_base(char **file_name, int file_num, BOOL no_mem, BOOL no_water, 
			  int projection_mode,
			  int initial_detail, char *header_file_name, 
			  double ***level_min_max, int level_num, BOOL fv_bw,
			  BOOL enable_cdv_rmsd_rmsf)
{
  int i;
  BOOL after_ref = KS_FALSE;
  CV_BASE *cb = NULL;

  /* initialize */

  init_atom_parm();

  if((cb = (CV_BASE*)ks_malloc(sizeof(CV_BASE),"cb")) == NULL){
    ks_error_memory();
    return NULL;
  }

  cb->reference_frame = NULL;
  cb->avrage_frame = NULL;

  cb->frame_num = 0;
  cb->current_frame = 0;

  cb->color_mode = CV_COLOR_MODE_PARTICLE;
  cb->no_mem = no_mem;
  cb->no_water = no_water;
  cb->move_mode = KS_FALSE;
  cb->kabe_mode = CV_KABE_MODE_HAKO;

  cb->look_selected_particle = KS_FALSE;
  cb->fit_reference = KS_FALSE;
  cb->overlap_reference = KS_FALSE;

  cb->draw_mode = CV_DRAW_MODE_LINE;
  cb->draw_water_mode = CV_DRAW_MODE_LINE;
  cb->particle_size = CV_PARTICLE_SIZE_NORMAL;
  cb->label_mode = CV_LABEL_MODE_OFF;

  /*
  cp = getenv("CDVIEW3_PARTICLE_SIZE_SCALE");
  if(cp != NULL){
    cb->particle_size_scale = atof(cp);
  } else {
    cb->particle_size_scale = 1.0;
  }
  */
  cb->particle_size_scale = 1.0;

  cb->telop = NULL;
  cb->selected_label = NULL;
  cb->selected_residue_label = NULL;
  for(i = 0; i < 3; i++)
    cb->move_add[i] = 0;

  cb->reuse_base.particle = NULL;
  cb->reuse_base.property = NULL;
  cb->reuse_base.bond = NULL;
  cb->reuse_base.aggregate = NULL;
  cb->reuse_base.chain = NULL;
  cb->reuse_base.residue = NULL;
  cb->reuse_base.label = NULL;
  cb->reuse_base.cdv_bond = NULL;
  cb->reuse_base.cdv_face = NULL;
  cb->reuse_base.sur = NULL;
  cb->reuse_base.color_map = NULL;

  cb->pdb_buf = NULL;

  cb->particle_hash = NULL;
  cb->particle_hash_size = 0;
  cb->particle_label_hash = NULL;
  cb->particle_label_hash_size = 0;
  cb->residue_hash = NULL;
  cb->residue_hash_size = 0;
  cb->residue_label_hash = NULL;
  cb->residue_label_hash_size = 0;

  cb->atype_mode = 0;
  cb->atype_mode_num = 1;
  for(i = 0; i < CV_ATOM_TYPE_MAX; i++)
    cb->atype_use[i] = KS_FALSE;

  cb->idle = NULL;
  cb->rmsd_graph = NULL;
  cb->rmsf_graph = NULL;
  cb->distance_graph = NULL;
  cb->rmsd_window_id = -1;
  cb->rmsf_window_id = -1;
  cb->distance_window_id = -1;

  cb->text_window = NULL;

  if(header_file_name != NULL){
    if((cb->header_file_name = ks_malloc_char_copy(header_file_name,"header_file_name")) == NULL){
      return NULL;
    }
  } else {
    cb->header_file_name = NULL;
  }

  cb->output_file_mode = CV_OUTPUT_PICTURE_BMP;

  fv_init_base(&cb->fv_base);
  fv_init_target(&cb->fv_target);
  cb->fv_level = level_min_max;
  cb->fv_level_num = level_num;
  cb->fv_base.black_and_white = fv_bw;
  cb->fv_target.val = 0;

  cb->draw_additional_surface = KS_TRUE;

  cb->projection_mode = projection_mode;
  cb->calc_quaternion_type = CV_QUATERNION_UEDA;

  cb->prep_change_mode = CV_PREP_CHANGE_OFF;

  cb->clip_plane_flg = NULL;
  cb->clip_plane = NULL;

  cb->enable_cdv_rmsd_rmsf = enable_cdv_rmsd_rmsf;
  cb->pov_info.diffuse = 0.9;
  cb->pov_info.ambient = 0.1;

  cb->atom = NULL;
  for(i = 0; i < CV_COLOR_MODE_NUM; i++)
    cb->color_sample[i] = NULL;
  cb->bond_type = NULL;
  cb->face_type = NULL;

  for(i = 0; i < 3; i++)
    cb->move_add[i] = 0;

  for(i = 0; i < CV_COLOR_MODE_NUM; i++)
    cb->color_mode_name[i][0] = '\0';
  strcpy(cb->color_mode_name[CV_COLOR_MODE_PARTICLE],"Atom");
  strcpy(cb->color_mode_name[CV_COLOR_MODE_AMINO],"Residue");
  strcpy(cb->color_mode_name[CV_COLOR_MODE_AMINO_TYPE],"Residue type");
  strcpy(cb->color_mode_name[CV_COLOR_MODE_AMINO_HYDROPATHY],"Hydropathy");
  strcpy(cb->color_mode_name[CV_COLOR_MODE_CHAIN],"Chain");
  strcpy(cb->color_mode_name[CV_COLOR_MODE_AGGREGATE],"Aggregate");
  strcpy(cb->color_mode_name[CV_COLOR_MODE_RAINBOW],"Rainbow");

  if((cb->ribbon = ks_allocate_gl_buffer_3d(0,2,1,
					    KS_GL_BUFFER_3D_DEFAULT|
					    KS_GL_BUFFER_3D_SPLINE|
					    KS_GL_BUFFER_3D_NORMAL,"ribbon")) == NULL){
    ks_error_memory();
    return NULL;
  }

  if(initial_detail >= CV_SPHERE_DETAIL_MIN){
    cb->detail = initial_detail;
  } else {
    cb->detail = CV_SPHERE_DETAIL_MIN;
  }
  cb->show_selected_particle_mode = CV_SHOW_SELECTED_PARTICLE_MODE_OFF;
  cb->show_selected_particle_range = 5;
  /*  cb->input_key_font = KS_GLUT_FONT_HELVETICA_12;*/
  cb->input_key_font = KS_GL_FONT_8x13;
  cb->background_transparency = 0.5;

  {
    char *cp;
    cp = getenv(CV_FILE_KEY_ENABLE_AXIS);
    if(cp == NULL){
      cb->axis_mode = CV_AXIS_MODE_OFF;
    } else if(strcmp(cp,"1") == 0){
      cb->axis_mode = CV_AXIS_MODE_ON;
    } else {
      cb->axis_mode = CV_AXIS_MODE_OFF;
    }
  }

  cb->continuous_color.mode = -1;
  cb->continuous_color.num = 0;

  init_light_pos(cb->light_position);

  cb->box_frame_color[CV_BOX_FRAME_COLOR_1][0] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_1][1] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_1][2] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_1][3] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_2][0] = 0.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_2][1] = 0.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_2][2] = 0.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_2][3] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][0] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][1] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][2] = 1.0;
  cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][3] = 1.0;

  cb->periodic_mode = KS_FALSE;
  cb->periodic_inside_color[0] = 0.5;
  cb->periodic_inside_color[1] = 0.5;
  cb->periodic_inside_color[2] = 0.5;
  cb->periodic_inside_color[3] = 1.0;

  cb->file_name_base[0] = '\0';
  cb->order_bmp_counter = -1;
  cb->output_bmp_file_cnt = 0;

  cb->time_offset = 0.0;
  cb->fv_color_map_select = KS_FALSE;

  cb->prep = NULL;

  if((cb->sphere_obj=(KS_GL_SPHERE_OBJ**)ks_malloc(sizeof(KS_GL_SPHERE_OBJ*)*
						   (CV_SPHERE_DETAIL_CNT),"sphere_obj"))==NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < CV_SPHERE_DETAIL_CNT; i++){
    cb->sphere_obj[i] = NULL;
  }
  for(i = 0; i < CV_SPHERE_DETAIL_CNT; i++){
    if((cb->sphere_obj[i] = ks_allocate_gl_sphere_obj(cv_sphere_detail_values[i])) == NULL){
      return NULL;
    }
  }
  cb->auto_command = NULL;
  cb->sur_fixed.color_map = NULL;
  cb->sur_fixed.sur = NULL;

  if((cb->fi = ks_allocate_file_lines()) == NULL){
    return NULL;
  }

  set_command(cb,CV_COMMAND_RMSD,"calc rmsd","rmsd",set_calc_rmsd,"calculate RMSD");
  set_command(cb,CV_COMMAND_RMSF,"calc rmsf","rmsf",set_calc_rmsf,"calculate RMSF");
  set_command(cb,CV_COMMAND_DISTANCE,"calc distance","dis",set_calc_distance,"calculate distance");
  set_command(cb,CV_COMMAND_DEL_DISTANCE,"del distance","ddis",del_distance,
	      "delete distance window");
  set_command(cb,CV_COMMAND_STORE_MEMORY,"store memory","mem",set_store_memory,
	      "store files in memory");
  set_command(cb,CV_COMMAND_SET_SKIP,"set skip","skip",set_skip_frame,
	      "set skip frame number [n]");
  set_command(cb,CV_COMMAND_SET_TIME_UNIT,"set time_unit","tu",set_time_unit,
	      "set time unit [fs/ps/ns]");
  set_command(cb,CV_COMMAND_SET_TIME_OFFSET,"set time_offset","to",set_time_offset,
	      "set time offset (s)");
  set_command(cb,CV_COMMAND_SHOW_LINE,"show line","line",show_line,"show line");
  set_command(cb,CV_COMMAND_HIDE_LINE,"hide line","line_off",hide_line,"hide line");
  set_command(cb,CV_COMMAND_SHOW_PARTICLE,"show particle","particle",show_particle,
	      "show particle");
  set_command(cb,CV_COMMAND_HIDE_PARTICLE,"hide particle","particle_off",hide_particle,
	      "hide particle");
  set_command(cb,CV_COMMAND_CALC_SOLVENT_EXCLUDE_SURFACE,"calc solvent_exclude_surface",
	      "ses",calc_solvent_exclude_surface,"calculate solvent exclude surface (SES)");
  set_command(cb,CV_COMMAND_CALC_SOLVENT_EXCLUDE_SURFACE_ALL,
	      "calc solvent_exclude_surface_all",
	      "ses_all",set_calc_solvent_exclude_surface_idle,
	      "calculate solvent exclude surface (SES) all frame");
  set_command(cb,CV_COMMAND_ADD_TEXT,"add text","text",set_add_text,"add text window");
  set_command(cb,CV_COMMAND_FIND_PARTICLE_LABEL,"find particle_label","label",
	      set_find_particle_label,"find particle by MD label");
  set_command(cb,CV_COMMAND_OUTPUT_PICTURE_BMP,"bmp","output bmp",change_output_bmp,
	      "change output mode BMP");
  set_command(cb,CV_COMMAND_OUTPUT_PICTURE_POV,"pov","output pov",change_output_pov,
	      "change output mode POV (shadow off)");
   set_command(cb,CV_COMMAND_OUTPUT_PICTURE_POV_SHADOW,"pov_shadow","output pov_shadow",
	       change_output_pov_shadow,"change output mode POV (shadow on)");
  set_command(cb,CV_COMMAND_TOGGLE_POV_WATER,"pov_water","pov_w",toggle_pov_water,
	      "toggle pov water mode");
  set_command(cb,CV_COMMAND_TOGGLE_POV_BUBBLE,"pov_bubble","pov_b",toggle_pov_bubble,
	      "toggle pov bubble mode");
  set_command(cb,CV_COMMAND_CALC_DENSITY,"den","calc density", set_calc_density,
	      "calculate density");
  set_command(cb,CV_COMMAND_FIT_AXIS,"fita","fit axis", fit_axis,
	      "fit two particle vector into axis");
  set_command(cb,CV_COMMAND_ADD_RESIDUE_NEXT,"add_rn","add residue next", set_add_residue_next,
	      "add residue next");
  set_command(cb,CV_COMMAND_ADD_RESIDUE_PREV,"add_rp","add residue prev", set_add_residue_prev,
	      "add residue prev");
  set_command(cb,CV_COMMAND_CHANGE_RESIDUE,"cr","change residue", set_change_residue,
	      "change residue name (remove side chain)");
  set_command(cb,CV_COMMAND_CHANGE_ATOM,"change_a","change atom", set_change_atom,
	      "change atom name");
  set_command(cb,CV_COMMAND_MAKE_BOND,"make_b","make bond", set_make_bond,"make bond");
  set_command(cb,CV_COMMAND_REMOVE_BOND,"rm_b","remove bond", set_remove_bond,"remove bond");
  set_command(cb,CV_COMMAND_CHANGE_LEVEL_MAX,"lmax","change level_max",set_change_level_max,
	      "change level max");
  set_command(cb,CV_COMMAND_CHANGE_LEVEL_MIN,"lmin","change level_min",set_change_level_min,
	      "change level min");
  set_command(cb,CV_COMMAND_CALC_GR_PARTICLE,"gr","gr particle",calc_gr_particle,
	      "calc radial distribution function of particle");
  set_command(cb,CV_COMMAND_CALC_GR_MONOMER,"gr mono","gr monomer",set_calc_gr_monomer,
	      "calc radial distribution function of mononer");
  set_command(cb,CV_COMMAND_ADJUST_DEPTH,"depth","adjust depth",set_adjust_depth_range,
	      "adjust depth range");
  set_command(cb,CV_COMMAND_OUTPUT_PREP,"prep","output prep",set_output_prep_file,
	      "output prep file");
  set_command(cb,CV_COMMAND_MOVE_PREP_DUMMY,"mv_d","move dummy",move_prep_dummy,
	      "move dummy particle of prep file");
  set_command(cb,CV_COMMAND_CHANGE_SHOW_SELECTED_PARTICLE_RANGE,"change s_range",
	      "change show_selected_particle_range",set_change_show_selected_particle_range,
	      "change show_selected_particle_range");
  set_command(cb,CV_COMMAND_CLIP_PLANE,"clip","set clip_plane",set_clip_plane,
	      "set clip plate ( x??, y?? or z?? ) ");
  set_command(cb,CV_COMMAND_ADD_CLIP_PLANE,"add_clip","add clip_plane",add_clip_plane,
	      "add clip plate ( x??, y?? or z?? ) ");
  set_command(cb,CV_COMMAND_ROTATION,"rot","rotate",set_rot_cd,
	      "rotate coordination: rot [x|y|z]angle (cNum): Num is chain number");
  set_command(cb,CV_COMMAND_TRANSLATION,"trans","translate",set_trans_cd,
	      "translate coordination: trans [x|y|z]length (cNum): Num is chain number");
  set_command(cb,CV_COMMAND_SET_CONTINUOUS_COLOR_NUM,"set c_color","c_color",
	      set_c_color,"set continuous color num");
  set_command(cb,CV_COMMAND_SET_EYE_ANGLE,"set eye_angle","eye_angle",
	      set_eye_angle,"set eye angle");
  set_command(cb,CV_COMMAND_SET_ROT_MOL,"rotate molecule","rot_mol",set_rot_mol,"rotate molecule");

  cb->command_help_window_id = -1;
  cb->command_help_font = KS_GL_FONT_HELVETICA_10;

  cb->skip_frame = 1;
  cb->draw_time_unit_type = CV_DRAW_TIME_UNIT_PS;

  cb->show_flags = 0;

#if 0
  for(i = 0; i < file_num; i++){
    if(strstr(file_name[i],".fdv") == NULL && strstr(file_name[i],".FDV") == NULL){
      /*      printf("%d %s\n",i,file_name[i]);*/
      cb->frame_num++;
    }
  }
#endif

  if(file_num == 1){
    if(strstr(file_name[0],".prep") != NULL || 
       strstr(file_name[0],".PREP") != NULL ||
       strstr(file_name[0],".in") != NULL ||
       strstr(file_name[0],".IN") != NULL){ /* prep file */
      set_prep_file(cb,file_name[0]);
    } else {
      cb->no_mem = KS_TRUE;
      if(get_dir_file(cb,file_name[0]) == KS_FALSE){  /* read series files */
	return NULL;
      }
    }
  } else {

    if((cb->frame = allocate_frame(file_num,cb->atype_mode,&cb->fi)) == NULL){
      return NULL;
    }

    cb->frame_num = 0;;
    for(i = 0; i < file_num; i++){
      printf("%d %s\n",i,file_name[i]);
      if(strlen(file_name[i]) > CV_FRAME_FILE_NAME_MAX){
	ks_error("file name %s is too long",file_name[i]);
	return KS_FALSE;
      }
      if(i == 0) cb->frame[cb->frame_num].flags |= CV_FRAME_REFERENCE;
      if(strstr(file_name[i],".fdv") != NULL || strstr(file_name[i],".FDV") != NULL){
	strcpy(cb->frame[cb->frame_num++].file_name_fdv,file_name[i]);
      } else {
	strcpy(cb->frame[cb->frame_num++].file_name,file_name[i]);
      }
    }
  }

  if(cb->frame_num == 0){
    ks_error("no input file");
    return NULL;
  }

#ifdef USE_GLSL
  {
    GLenum err = glewInit();
    if (err != GLEW_OK) {
      ks_error("Error: %s\n", glewGetErrorString(err));
      ks_exit(EXIT_FAILURE);
    }
    if((cb->sb = allocate_shader_base()) == NULL){
      ks_exit(EXIT_FAILURE);
    }
  }
#else
  cb->sb = NULL;
#endif

  for(i = 0; i < cb->frame_num; i++){
    /*
    printf("%d %s %d\n",i,cb->frame[i].file_name,cb->frame[i].flags);
    */
    cb->frame[i].label = i;

    ks_get_file_path(cb->frame[i].file_name,cb->frame[i].file_path,sizeof(cb->frame[i].file_path));
    /*
    printf("%d '%s' '%s'\n",i,cb->frame[i].file_name,cb->frame[i].file_path);
    ks_exit(EXIT_FAILURE);
    */
    if(cb->frame[i].flags&CV_FRAME_REFERENCE){
      cb->current_frame = i;
      cb->reference_frame = &cb->frame[i];
      after_ref = KS_TRUE;
    }
    if(after_ref == KS_TRUE){
      cb->frame[i].flags |= CV_FRAME_ENABLE;
    }
    if(strstr(cb->frame[i].file_name,".pdb") != NULL ||
       strstr(cb->frame[i].file_name,".PDB") != NULL){
      cb->frame[i].flags |= CV_FRAME_PDB;
    } else if(strstr(cb->frame[i].file_name,".prep") != NULL || 
	      strstr(cb->frame[i].file_name,".PREP") != NULL ||
	      strstr(cb->frame[i].file_name,".in") != NULL ||
	      strstr(cb->frame[i].file_name,".IN") != NULL){
      cb->frame[i].flags |= CV_FRAME_PREP;
    } else if(strstr(cb->frame[i].file_name_fdv,".fdv") != NULL || 
	      strstr(cb->frame[i].file_name_fdv,".FDV") != NULL){
      cb->frame[i].flags |= CV_FRAME_FDV;
      cb->draw_mode = CV_DRAW_MODE_PARTICLE;
    } else {
      cb->frame[i].flags |= CV_FRAME_CDV;
      cb->draw_mode = CV_DRAW_MODE_PARTICLE;
    }
    if(cb->no_mem == KS_FALSE || (cb->no_mem == KS_TRUE && cb->frame[i].flags&CV_FRAME_REFERENCE)){
      if(init_frame(&cb->frame[i],
		    (cb->frame[i].flags&CV_FRAME_REFERENCE ||
		     cb->frame[i].flags&CV_FRAME_FDV) ? &cb->fv_base:NULL,cb)
	 == KS_FALSE){
	return NULL;
      }
    }
  }

  if(cb->frame[cb->current_frame].have_side == KS_TRUE){
    cb->kabe_mode = CV_KABE_MODE_SIDE;
  }
  if(set_hash(&cb->frame[cb->current_frame],&cb->reuse_base.label,
	      &cb->particle_hash,&cb->particle_hash_size,
	      &cb->particle_label_hash,&cb->particle_label_hash_size,
	      &cb->residue_hash,&cb->residue_hash_size,
	      &cb->residue_label_hash,&cb->residue_label_hash_size) == KS_FALSE){
    return KS_FALSE;
  }

  if(cb->frame[cb->current_frame].cdv_bond != NULL){
    make_cdv_bond(cb->frame[cb->current_frame].cdv_bond,cb->bond_type,
		  cb->particle_hash,cb->particle_hash_size,
		  cb->particle_label_hash,cb->particle_label_hash_size,
		  &cb->frame[cb->current_frame].pdb_user_bond,
		  &cb->reuse_base.bond);
    free_cv_cdv_bond_list(cb->frame[cb->current_frame].cdv_bond);
    cb->frame[cb->current_frame].cdv_bond = NULL;
  }
  if(cb->frame[cb->current_frame].cdv_face != NULL){
    make_cdv_face(cb->frame[cb->current_frame].cdv_face,cb->face_type,
		  &cb->frame[cb->current_frame].sur,
		  cb->particle_hash,cb->particle_hash_size,
		  cb->particle_label_hash,cb->particle_label_hash_size,
		  &cb->reuse_base.sur);
  }

#ifdef USE_GLSL
  if(init_rod_draw_info(cb,&cb->frame[cb->current_frame]) == KS_FALSE){
    return NULL;
  }
#endif
  return cb;
}
static void free_cv_frame(CV_FRAME *fr)
{
  int i;
  free_cv_particle(fr->particle);
  free_cv_aggregate(fr->aggregate);
  if(fr->bond_file_name != NULL)
    ks_free(fr->bond_file_name);
  if(fr->face_file_name != NULL)
    ks_free(fr->face_file_name);
  if(fr->unknown_residue_label != NULL)
    ks_free_char_list(fr->unknown_residue_label);
  for(i = 0; i < CV_USER_TEXT_MAX; i++){
    if(fr->user_text[i].text != NULL){
      ks_free(fr->user_text[i].text);
    }
  }
  if(fr->cdv_bond != NULL){
    free_cv_cdv_bond_list(fr->cdv_bond);
  }
  if(fr->cdv_face != NULL){
    free_cv_cdv_face_list(fr->cdv_face);
  }
  /*
  if(fr->fv != NULL){
    fv_free(fr->fv);
  }
  */
  if(fr->sur != NULL){
    free_additional_surface(fr->sur);
  }
  if(fr->color_map != NULL){
    cv_free_color_map(fr->color_map);
  }
  if(fr->c_color_info.start_color != NULL){
    ks_free(fr->c_color_info.start_color);
  }
  if(fr->c_color_info.end_color != NULL){
    ks_free(fr->c_color_info.end_color);
  }
  if(fr->c_color_info.map != NULL){
    for(i = 0; i < 4; i++){
      if(fr->c_color_info.map[i] != NULL){
	cv_free_color_map(fr->c_color_info.map[i]);
      }
    }
    ks_free(fr->c_color_info.map);
  }
  if(fr->periodic_p != NULL){
    for(i = 0; i < 27; i++){
      if(fr->periodic_p[i] != NULL){
	ks_free(fr->periodic_p[i]);
      }
    }
    ks_free(fr->periodic_p);
  }
  if(fr->periodic_p_num != NULL){
    ks_free(fr->periodic_p_num);
  }
  if(fr->pdb_user_bond != NULL){
    free_cv_bond(fr->pdb_user_bond);
  }
}
void free_cv_reuse(CV_REUSE reuse)
{
  /*
  printf("reuse.particle %d\n",count_cv_particle(reuse.particle));
  printf("reuse.property %d\n",count_cv_particle_property(reuse.property));
  printf("reuse.bond %d\n",count_cv_bond(reuse.bond));
  printf("reuse.aggregate %d\n",count_cv_aggregate(reuse.aggregate));
  printf("reuse.chain %d\n",count_cv_chain(reuse.chain));
  printf("reuse.residue %d\n",count_cv_residue(reuse.residue));
  printf("reuse.label %d\n",ks_count_label_list(reuse.label));
  */
  free_cv_particle(reuse.particle);
  free_cv_particle_property(reuse.property);
  free_cv_bond(reuse.bond);
  free_cv_aggregate(reuse.aggregate);
  free_cv_chain(reuse.chain);
  free_cv_residue(reuse.residue);
  ks_free_label_list(reuse.label);
  free_cv_cdv_bond_list(reuse.cdv_bond);
  free_cv_cdv_face_list(reuse.cdv_face);
  if(reuse.sur != NULL){
    free_additional_surface(reuse.sur);
  }
  if(reuse.color_map != NULL){
    cv_free_color_map(reuse.color_map);
  }
}
void cv_free_base(CV_BASE *cb)
{
  int i,j;
  
  for(i = 0; i < cb->frame_num; i++){
    if(cb->sur_fixed.sur != NULL &&                // fixed surface is set
       cb->frame[i].sur == cb->sur_fixed.sur){     // cb->frame[i].sur is copy of sur_fixed.sur
      cb->frame[i].sur = NULL;                     // not need to free
    }
    if(cb->sur_fixed.color_map != NULL &&          // fixed color_map is set
       cb->frame[i].color_map ==
       cb->sur_fixed.color_map){       // cb->frame[i].color_map is copy of sur_fixed.color_map
      cb->frame[i].color_map = NULL;               // not need to free
    }
    free_cv_frame(&cb->frame[i]);
  }
  ks_free(cb->frame);

  if(cb->sur_fixed.sur != NULL){
    free_additional_surface(cb->sur_fixed.sur);
  }
  if(cb->sur_fixed.color_map != NULL){
    cv_free_color_map(cb->sur_fixed.color_map);
  }

  if(cb->particle_hash != NULL)
    ks_free(cb->particle_hash);
  if(cb->particle_label_hash != NULL){
    for(i = 0; i < cb->particle_label_hash_size; i++)
      ks_free_label_list(cb->particle_label_hash[i]);
    ks_free(cb->particle_label_hash);
  }
  if(cb->residue_hash != NULL)
    ks_free(cb->residue_hash);
  if(cb->residue_label_hash != NULL){
    for(i = 0; i < cb->residue_label_hash_size; i++)
      ks_free_label_list(cb->residue_label_hash[i]);
    ks_free(cb->residue_label_hash);
  }

  free_cv_reuse(cb->reuse_base);

  ks_free_gl_buffer_3d(cb->ribbon);
  ks_free_text_list(cb->telop);
  ks_free_int_list(cb->selected_label);
  ks_free_int_list(cb->selected_residue_label);
  if(cb->pdb_buf != NULL)
    ks_free_pdb_buf(cb->pdb_buf);
  free_cv_atom(cb->atom);
  for(i = 0; i < CV_COLOR_MODE_NUM; i++)
    ks_free_sphere(cb->color_sample[i]);
  if(cb->rmsd_graph != NULL)
    ks_free_gl_graph_2d(cb->rmsd_graph);
  if(cb->rmsf_graph != NULL)
    ks_free_gl_graph_2d(cb->rmsf_graph);
  if(cb->distance_graph != NULL)
    ks_free_gl_graph_2d(cb->distance_graph);
  if(cb->avrage_frame != NULL){
    free_cv_frame(&cb->avrage_frame[0]);
    ks_free(cb->avrage_frame);
  }
  free_cv_bond_type(cb->bond_type);
  free_cv_face_type(cb->face_type);
  if(cb->text_window != NULL){
    ks_free_char_list(cb->text_window);
  }
  if(cb->header_file_name != NULL){
    ks_free(cb->header_file_name);
  }

  if(cb->fv_base.field != NULL){
    fv_free_base(&cb->fv_base);
  }

  fv_free_target(cb->fv_target);

  if(cb->clip_plane != NULL){
    ks_free(cb->clip_plane_flg);
    ks_free_double_pp(6,cb->clip_plane);
  }
  for(i = 0; i < KS_RANGE; i++){
    for(j = 0; j < FV_MAX_COLUMN_NUM; j++){
      if(cb->fv_level[i][j] != NULL){
	ks_free(cb->fv_level[i][j]);
      }
    }
  }
  for(i = 0; i < KS_RANGE; i++){
    ks_free(cb->fv_level[i]);
  }
  ks_free(cb->fv_level);
  if(cb->prep != NULL){
    ks_free_amber_prep(cb->prep);
  }

  for(i = 0; i < CV_SPHERE_DETAIL_MAX+1; i++){
    if(cb->sphere_obj[i] != NULL){
      ks_free_gl_sphere_obj(cb->sphere_obj[i]);
    }
  }
  ks_free(cb->sphere_obj);

  if(cb->auto_command != NULL){
    ks_free(cb->auto_command->command);
    ks_free(cb->auto_command);
  }

#ifdef USE_GLSL
  free_shader_base(cb->sb);
#endif

  ks_free_file_lines(cb->fi);

  ks_free(cb);
}
#ifndef USE_GLSL
static void draw_hako(double len, double *center)
{
  GLfloat color[4];
  GLfloat em[4] = {0,0,0,0};

  len *= .5;

  glMaterialfv(GL_FRONT,GL_EMISSION,em);
  color[0] = 0.0; color[1] = 0.0; color[2] = 0.4; color[3] = 1.0;
  glMaterialfv(GL_FRONT,GL_DIFFUSE,color);
  color[0] = 0.0; color[1] = 0.0; color[2] = 1.0; color[3] = 1.0;
  glMaterialfv(GL_FRONT,GL_AMBIENT,color);
  glPushMatrix();
  glTranslated(center[0],center[1],center[2]);
  ks_draw_gl_box(-len,-len,-len,len,len,len,KS_DRAW_GL_BOX_BACK);
  glPopMatrix();
}
#endif
static void get_color(CV_PARTICLE *p, int color_mode, int bright, GLfloat *color, BOOL prep_mode)
{
  int i;
  GLfloat *atom_color;
  double dcolor[3];

  if(color_mode == CV_COLOR_MODE_RAINBOW && p->property != NULL && p->property->chain->len > 1){
    /*
    printf("%d %d %f\n",
	   p->property->residue->number,
	   p->property->chain->len,
	   (double)(p->property->residue->number)/(double)(p->property->chain->len-1));
    */
    ks_hsv2rgb((1.-(double)(p->property->residue->number)/(double)(p->property->chain->len-1))*240,
	       1.0,1.0,&dcolor[0],&dcolor[1],&dcolor[2]);
    if(bright == KS_GL_ATOM_BRIGHT_LOW){
      for(i = 0; i < 3; i++) color[i] = dcolor[i]*.5;
    } else {
      for(i = 0; i < 3; i++) color[i] = dcolor[i];
    }
  } else if(prep_mode == KS_TRUE && color_mode == CV_COLOR_MODE_RAINBOW){
    get_charge_color(p->quaternion[0],color);
  } else {
    atom_color = ks_get_gl_atom_color(p->atom->gl_atom,p->color_mode[color_mode],bright);
    for(i = 0; i < 3; i++) color[i] = atom_color[i];
  }
  /*
  if(color_mode == CV_COLOR_MODE_PARTICLE || p->property == NULL){
    for(i = 0; i < 3; i++)
      color[i] = ks_get_gl_atom_color(p->atom->gl_atom,
				      KS_GL_ATOM_COLOR_ATOM,KS_GL_ATOM_BRIGHT_NORMAL)[i];
  } else if(color_mode == CV_COLOR_MODE_AMINO){
    if(ks_get_amino_color(p->property->residue->name,color) == KS_FALSE){
      ks_get_gl_unknown_color(0,color);
    }
  } else if(color_mode == CV_COLOR_MODE_AMINO_TYPE){
    if(ks_get_amino_id(p->property->residue->name,&amino_id) == KS_FALSE){
      ks_get_gl_unknown_color(0,color);
    } else {
      for(i = 0; i < 3; i++){
	color[i] = ks_amino[ks_amino[amino_id].type].color[i];
      }
    }
  } else if(color_mode == CV_COLOR_MODE_AMINO_HYDROPATHY){
    if(ks_get_amino_hydropathy_index(p->property->residue->name,&hydropathy) == KS_FALSE){
      ks_get_gl_unknown_color(0,color);
    } else {
      ks_get_gl_charge_color((int)((hydropathy+4.5)*5/9),color);
    }
  } else if(color_mode == CV_COLOR_MODE_CHAIN){
    for(i = 0; i < 3; i++){
      color[i] = ks_amino[p->property->chain->label%KS_AMINO_LIST_NUM].color[i];
    }
  } else if(color_mode == CV_COLOR_MODE_AGGREGATE){
    for(i = 0; i < 3; i++){
      color[i] = ks_amino[p->property->aggregate->label%KS_AMINO_LIST_NUM].color[i];
    }
  } else {
    color[0] = 0.5; color[1] = 0.5; color[2] = 0.5; color[3] = 1.0;
  }
  */
}
static void get_continuous_color(CV_PARTICLE *p, CV_CONTINUOUS_COLOR *c_color, 
				 CV_CONTINUOUS_COLOR_INFO *c_info, GLfloat *color)
{
  int i;
  double r,g,b;
  double v;
  v = ((p->quaternion[c_color->mode]-c_info->min[c_color->mode])/
       (c_info->max[c_color->mode]-c_info->min[c_color->mode]));
  if(v > 1.0) v = 1.0;
  if(v < 0.0) v = 0.0;
  color[3] = 1.0;
  if(c_info->start_color == NULL || c_info->end_color == NULL){
    ks_hsv2rgb((1.0-v)*240,1.0,1.0,&r,&g,&b);
    color[0] = r;
    color[1] = g;
    color[2] = b;
  } else {
    for(i = 0; i < 3; i++){
      color[i] = (v*c_info->end_color[i]+(1.0-v)*c_info->start_color[i]);
    }
  }
}
static void draw_pov_rod_pos(FILE *pov, double *pos0, double *pos1, float *color, float radius,
			     CV_POV_INFO pov_info)
{
  fprintf(pov,"cylinder{<%f,%f,%f,>,<%f,%f,%f>,%f open\n"
	  ,pos0[0],pos0[1],pos0[2]
	  ,pos1[0],pos1[1],pos1[2],fabs(radius));
  fprintf(pov," texture{\n");
  fprintf(pov,"  pigment{\n");
  fprintf(pov,"   color rgb<%f,%f,%f>\n",color[0],color[1],color[2]);
  fprintf(pov,"  }\n");
  fprintf(pov,"  finish{\n");
  fprintf(pov,"   diffuse %f\n",pov_info.diffuse);
  fprintf(pov,"   ambient %f\n",pov_info.ambient);
  fprintf(pov,"  }\n");
  fprintf(pov," }\n");
  fprintf(pov,"}\n");
}
static void set_rod_r_and_color(CV_BOND *bond, BOOL bright_low, GLfloat* rod_color, float* rod_r)
{
  int i;
  if(bond->type != NULL){
    *rod_r = bond->type->radius;
    for(i = 0; i < 3; i++){
      rod_color[i] = bond->type->color[i];
    }
  } else {
    *rod_r = init_cv_bond_type.radius;
    rod_color[0] = init_cv_bond_type.color[0];
    rod_color[1] = init_cv_bond_type.color[1];
    rod_color[2] = init_cv_bond_type.color[2];
  }
  if(bright_low == KS_TRUE){
    for(i = 0; i < 3; i++)
      rod_color[i] *= 0.5;
  }
}
static void draw_bond_particles(KS_GL_BASE *gb, CV_PARTICLE *p, CV_PARTICLE *pb, CV_BOND *bond,
				int color_mode, int draw_mode, BOOL line, BOOL rod, 
				BOOL bright_low, FILE *pov, CV_POV_INFO pov_info)
{
  int i;
  double cd[2][3];
  GLfloat color[4] = {0.0,0.0,0.0,1.0};
  GLfloat rod_color[4] = {0.0,0.0,0.0,1.0};
  float rod_r = 0.1;
  BOOL line_flg;
  BOOL rod_flg;

  for(i = 0; i < 3; i++)
    cd[0][i] = p->cd[i]+p->move[i];

  if(bond->type != NULL && bond->type->radius < 0){
    line_flg = KS_TRUE;
    rod_flg = KS_FALSE;
  } else {
    line_flg = line;
    rod_flg = rod;
  }
  for(i = 0; i < 3; i++)
    cd[1][i] = pb->cd[i]+pb->move[i];
  /*
    if(pb->property != NULL){
    for(i = 0; i < 3; i++)
    cd[1][i] += (pb->property->aggregate->move[i]+
    pb->property->chain->move[i]+
    pb->property->residue->move[i]);
    }
  */
  if(pov == NULL){
    if(line_flg == KS_TRUE){
      if(bond->type == NULL){
	for(i = 0; i < 3; i++)
	  cd[1][i] = (cd[0][i] + cd[1][i])*.5;
	get_color(p,color_mode,
		  (bright_low == KS_TRUE) ? KS_GL_ATOM_BRIGHT_LOW:KS_GL_ATOM_BRIGHT_NORMAL,color,
		  KS_FALSE);
      } else {
	set_rod_r_and_color(bond,bright_low,color,&rod_r);
      }
      if(bond->type == NULL || (p->label < pb->label && 
				(draw_mode != CV_DRAW_MODE_MAIN_CHAIN || 
				 (draw_mode == CV_DRAW_MODE_MAIN_CHAIN && 
				  p->flags&CV_PARTICLE_MAIN && 
				  pb->flags&CV_PARTICLE_MAIN)))){
	glLoadName(CV_NAME_PARTICLE);
	glPushName(p->label);
	glColor3fv(color);
	if(p->flags&CV_PARTICLE_SELECTED)
	  glLineWidth(4.0);
	else
	  glLineWidth(1.0);
	glDisable(GL_LIGHTING);
	glBegin(GL_LINES);
	glVertex3d(cd[0][0],cd[0][1],cd[0][2]);
	glVertex3d(cd[1][0],cd[1][1],cd[1][2]);
	glEnd();
	glEnable(GL_LIGHTING);
	glPopName();
      }
    }
    if(rod_flg == KS_TRUE && p->label < pb->label && 
       (draw_mode != CV_DRAW_MODE_MAIN_CHAIN || 
	(draw_mode == CV_DRAW_MODE_MAIN_CHAIN && 
	 p->flags&CV_PARTICLE_MAIN && 
	 pb->flags&CV_PARTICLE_MAIN))){
      GLfloat em[4] = {0.0,0.0,0.0,1.0};

      set_rod_r_and_color(bond,bright_low,rod_color,&rod_r);

      if(p->flags&CV_PARTICLE_SELECTED && pb->flags&CV_PARTICLE_SELECTED){
	for(i = 0; i < 3; i++){
	  em[i] = KS_GL_SELECTED_EMISSION;
	}
      }
      glLoadName(CV_NAME_BOND);
      glPushName(p->label);
      glMaterialfv(GL_FRONT,GL_EMISSION,em);
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,rod_color);
      /*
	printf("%d %f %f %f %f\n",bright_low,rod_color[0],rod_color[1],rod_color[2],rod_color[3]);
      */
      /*
	printf("(%f %f %f)(%f %f %f)\n",cd[0][0],cd[0][1],cd[0][2],cd[1][0],cd[1][1],cd[1][2]);
      */
      ks_draw_gl_rod_pos(gb,cd[0][0],cd[0][1],cd[0][2],cd[1][0],cd[1][1],cd[1][2],
			 rod_r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
      glPopName();
    }
  } else if(rod_flg == KS_TRUE && p->label < pb->label && 
	    (draw_mode != CV_DRAW_MODE_MAIN_CHAIN || 
	     (draw_mode == CV_DRAW_MODE_MAIN_CHAIN && 
	      p->flags&CV_PARTICLE_MAIN && 
	      pb->flags&CV_PARTICLE_MAIN))){
    set_rod_r_and_color(bond,bright_low,rod_color,&rod_r);
    draw_pov_rod_pos(pov,cd[0],cd[1],rod_color,rod_r,pov_info);
  }
}
static void draw_bond(KS_GL_BASE *gb, CV_PARTICLE *p, int color_mode, int draw_mode, 
		      BOOL line, BOOL rod, BOOL bright_low, FILE *pov, CV_POV_INFO pov_info)
{
  CV_BOND *bond;
  CV_PARTICLE *pb;

  for(bond = p->bond; bond != NULL; bond = bond->next){
    pb = bond->p;
    draw_bond_particles(gb,p,pb,bond,color_mode,draw_mode,line,rod,bright_low,pov,pov_info);
  }
  glLoadName(CV_NAME_DEFAULT);
}
static void draw_label(KS_GL_BASE *gb, CV_PARTICLE *p,int particle_size,double particle_size_scale,
		       int label_mode, BOOL prep_mode)
{
  char text[256];
  double up[3] = {0,-0.1,0}, rot_up[3];

  glDisable(GL_LIGHTING);
  ks_gl_base_foreground_color(gb);
  /*
  if(particle_size == KS_GL_ATOM_SIZE_SMALL)       up[2] = p->atom->radius*0.25;
  else if(particle_size == KS_GL_ATOM_SIZE_NORMAL) up[2] = p->atom->radius*0.5;
  else if(particle_size == KS_GL_ATOM_SIZE_LARGE)  up[2] = p->atom->radius*1.0;
  */
  up[2] = ks_get_gl_atom_size(p->atom->gl_atom,particle_size)*particle_size_scale;
  ks_multi_gl_inverse_rotational_matrix(*gb,up,rot_up);

  glRasterPos3d(rot_up[0],rot_up[1],rot_up[2]);

  if(label_mode == CV_LABEL_MODE_PARTICLE_LABEL){
    sprintf(text,"%d",p->file_label);
  } else if(label_mode == CV_LABEL_MODE_PARTICLE_NAME){
    if(p->property == NULL){
      sprintf(text,"%s",p->atom->name);
    } else {
      sprintf(text,"%s",p->property->name);
    }
  } else if(label_mode == CV_LABEL_MODE_RESIDUE_LABEL){
    if(prep_mode == KS_FALSE){
      if(p->property != NULL){
	sprintf(text,"%d",p->property->residue->file_label);
      }
    } else {
      sprintf(text,"%.2f",p->quaternion[0]);
    }
  } else if(label_mode == CV_LABEL_MODE_RESIDUE_NAME){
    if(p->property != NULL){
      sprintf(text,"%s",p->property->residue->name);
    }
  }
  ks_use_glut_font(gb,KS_GL_FONT_HELVETICA_10);
  glCallLists(strlen(text), GL_BYTE, text);
  glEnable(GL_LIGHTING);
}
static void draw_particle_below(KS_GL_BASE *gb, CV_PARTICLE *p,int particle_size,
				double particle_size_scale,
				char *text)
{
  double up[3] = {0,-0.1,0}, rot_up[3];

  glDisable(GL_LIGHTING);

  up[2] =-ks_get_gl_atom_size(p->atom->gl_atom,particle_size)*particle_size_scale-.2;
  ks_multi_gl_inverse_rotational_matrix(*gb,up,rot_up);

  glRasterPos3d(rot_up[0],rot_up[1],rot_up[2]);

  ks_use_glut_font(gb,KS_GL_FONT_HELVETICA_10);
  glCallLists(strlen(text), GL_BYTE, text);
  glEnable(GL_LIGHTING);
}
static void draw_particle_order(KS_GL_BASE *gb, CV_PARTICLE *p,int particle_size,
				double particle_size_scale,
				int particle_order)
{
#if 1
  char text[256];
  if(particle_order == change_particle_order_target){
    glColor4f(1.0,0.0,0.0,1.0);
  } else {
    glColor4f(1.0,1.0,0.0,1.0);
  }
  sprintf(text,"%d",particle_order);
  draw_particle_below(gb,p,particle_size,particle_size_scale,text);
#else
  char text[256];
  double up[3] = {0,-0.1,0}, rot_up[3];

  glDisable(GL_LIGHTING);

  if(particle_order == change_particle_order_target){
    glColor4f(1.0,0.0,0.0,1.0);
  } else {
    glColor4f(1.0,1.0,0.0,1.0);
  }

  up[2] =-ks_get_gl_atom_size(p->atom->gl_atom,particle_size)*particle_size_scale-.2;
  ks_multi_gl_inverse_rotational_matrix(*gb,up,rot_up);

  glRasterPos3d(rot_up[0],rot_up[1],rot_up[2]);

  sprintf(text,"%d",particle_order);
  ks_use_glut_font(gb,KS_GL_FONT_HELVETICA_10);
  glCallLists(strlen(text), GL_BYTE, text);
  glEnable(GL_LIGHTING);
#endif
}
static void draw_particle_main(KS_GL_BASE *gb, CV_PARTICLE *p,int particle_size,
			       double particle_size_scale)
{
  if(p->flags&CV_PARTICLE_MAIN){
    glColor4f(1.0,1.0,0.0,1.0);
    draw_particle_below(gb,p,particle_size,particle_size_scale,"MAIN");
  }
}
static void draw_particle_dot(KS_GL_BASE *gb, CV_PARTICLE *p, int color_mode,int label_mode, 
			      BOOL bright_low,
			      CV_CONTINUOUS_COLOR *c_color, CV_CONTINUOUS_COLOR_INFO *c_info)
{
  int bright;
  GLfloat color[4];
  glLoadName(CV_NAME_PARTICLE);

  if(bright_low == KS_TRUE){
    bright = KS_GL_ATOM_BRIGHT_LOW;
  } else if(p->flags&CV_PARTICLE_SELECTED){
    bright = KS_GL_ATOM_BRIGHT_HIGH;
  } else {
    bright = KS_GL_ATOM_BRIGHT_NORMAL;
  }

  glDisable(GL_LIGHTING);
  glPushName(p->label);

  /*
  for(i = 0; i < 3; i++)
    color[i] = ks_get_gl_atom_color(p->atom->gl_atom,p->color_mode[color_mode],bright)[i];
  color[i] = 1.0;
  */

  if(c_color->num != 0 && c_color->mode != -1){
    get_continuous_color(p,c_color,c_info,color);
  } else {
    get_color(p,color_mode,bright,color,KS_FALSE);
  }
  glColor4fv(color);
  if(bright == KS_GL_ATOM_BRIGHT_HIGH){
    glPointSize(4.0);
  } else {
    glPointSize(2.0);
  }
  glBegin(GL_POINTS);
  glVertex3d(p->cd[0]+p->move[0],p->cd[1]+p->move[1],p->cd[2]+p->move[2]);
  glEnd();
  glEnable(GL_LIGHTING);
  glPopName();

  glLoadName(CV_NAME_DEFAULT);
}
static void rot_angle(int calc_quaternion_type,double *q)
{
  double rr[16];
  /*
  int i,j;
  double im[3][3] = {{1,0,0},{0,1,0},{0,0,1}};
  double rm[3][3];
  */
  /*
  for(j = 0; j < 3; j++){
    (*trans_sp_mol[calc_quaternion_type])(q,im[j],rm[j]);
  }
  */
  /*
  printf("%f %f %f %f\n",q[0],q[1],q[2],q[3]);
  for(i = 0; i < 3; i++){
    for(j = 0; j < 3; j++){
      printf(" % f",rm[i][j]);
    }
    printf("\n");
  }
  */
  /*
  for(j = 0; j < 3; j++){
    (*trans_sp_mol[calc_quaternion_type][j])(q,rm[j]);
  }
  rr[0] = rm[0][0];
  rr[1] = rm[1][0];
  rr[2] = rm[2][0];
  rr[3] = 0;
  rr[4] = rm[0][1];
  rr[5] = rm[1][1];
  rr[6] = rm[2][1];
  rr[7] = 0;
  rr[8] = rm[0][2];
  rr[9] = rm[1][2];
  rr[10]= rm[2][2];
  rr[11] = 0;
  rr[12] = 0;
  rr[13] = 0;
  rr[14] = 0;
  rr[15] = 1.0;
  */

  (*trans_sp_mol[calc_quaternion_type][0])(q,rr);
  rr[3] = 0;
  (*trans_sp_mol[calc_quaternion_type][1])(q,rr);
  rr[7] = 0;
  (*trans_sp_mol[calc_quaternion_type][2])(q,rr);
  rr[11] = 0;
  rr[12] = 0;
  rr[13] = 0;
  rr[14] = 0;
  rr[15] = 1.0;
  glMultMatrixd(rr);
}
static void draw_pov_sphere(FILE *pov, double r, double x, double y, double z, GLfloat *color,
			    CV_POV_INFO pov_info)
{
  fprintf(pov,"sphere{0.0,%f\n",r);
  fprintf(pov," translate<% f,% f,% f>\n",x,y,z);
  fprintf(pov," texture{\n");
  fprintf(pov,"  pigment{ color rgb <%f,%f %f>}\n",color[0],color[1],color[2]);
  fprintf(pov,"  finish{\n");
  fprintf(pov,"   diffuse %f\n",pov_info.diffuse);
  fprintf(pov,"   ambient %f\n",pov_info.ambient);
  fprintf(pov,"  }\n");
  fprintf(pov," }\n");
  fprintf(pov,"}\n");
}
static void draw_pov_bond(FILE *pov, 
			  double f0, double f1, double f2, double f3, double f4, double f5, 
			  double r, GLfloat *col, CV_POV_INFO pov_info)
{
  fprintf(pov,"cylinder{<%f,%f,%f,>,<%f,%f,%f>,%f open\n"
	  ,f0,f1,f2,f3,f4,f5,r);
  fprintf(pov," texture{\n");
  fprintf(pov,"  pigment{\n");
  fprintf(pov,"   color rgb<%f,%f,%f>\n",col[0],col[1],col[2]);
  fprintf(pov,"  }\n");
  fprintf(pov,"  finish{\n");
  fprintf(pov,"   diffuse 0.6\n");
  fprintf(pov,"   ambient 0.1\n");
  fprintf(pov,"   diffuse %f\n",pov_info.diffuse);
  fprintf(pov,"   ambient %f\n",pov_info.ambient);
  fprintf(pov,"  }\n");
  fprintf(pov," }\n");
  fprintf(pov," no_shadow\n");
  fprintf(pov,"}\n");
}
static void draw_particle(KS_GL_BASE *gb, CV_PARTICLE *p, CV_BASE *cb, CV_FRAME *fr,
			  BOOL bright_low, int trans_mode, BOOL draw_bond_flg, 
			  FILE *pov, int particle_order)
{
  int i;
  int bright;
  int size;
  GLfloat color[4];
  GLboolean normalize;
  int detail = cb->detail;

  if(!(p->atom->flags&CV_ATOM_SHOW)){
    return;
  }

  if(pov == NULL){
    glLoadName(CV_NAME_PARTICLE);

    glPushName(p->label);
    glPushMatrix();
    glTranslated(p->cd[0]+p->move[0],p->cd[1]+p->move[1],p->cd[2]+p->move[2]);
    /*  printf("%f %f %f\n",p->cd[0]+p->move[0],p->cd[1]+p->move[1],p->cd[2]+p->move[2]);*/

    if(bright_low == KS_TRUE){
      bright = KS_GL_ATOM_BRIGHT_LOW;
    } else if(p->flags&CV_PARTICLE_SELECTED){
      bright = KS_GL_ATOM_BRIGHT_HIGH;
    } else {
      bright = KS_GL_ATOM_BRIGHT_NORMAL;
    }
    if(cb->particle_size == CV_PARTICLE_SIZE_NORMAL){
      size = KS_GL_ATOM_SIZE_NORMAL;
    } else if(cb->particle_size == CV_PARTICLE_SIZE_SMALL){
      size = KS_GL_ATOM_SIZE_SMALL;
    } else if(cb->particle_size == CV_PARTICLE_SIZE_LARGE){
      size = KS_GL_ATOM_SIZE_LARGE;
    }

    /*  printf("%d %p %f %f %f\n",p->atype[0],p->atom,p->cd[0],p->cd[1],p->cd[2]);*/

    /*
      ks_draw_gl_atom(p->atom->gl_atom,
      p->color_mode[color_mode],
      size,
      bright,
      trans_mode);
    */
    /*
      for(i = 0; i < 3; i++)
      color[i] = ks_get_gl_atom_color(p->atom->gl_atom,p->color_mode[color_mode],bright)[i];
      color[3] = 1.0;
    */
    if(cb->continuous_color.num != 0 && cb->continuous_color.mode != -1){
      get_continuous_color(p,&cb->continuous_color,&fr->c_color_info,color);
    } else {
      get_color(p,cb->color_mode,bright,color,fr->prep_block != NULL);
    }
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
    glMaterialfv(GL_FRONT,GL_EMISSION,ks_get_gl_atom_emission(p->atom->gl_atom,bright));
    if(p->atom->gl_atom->scale != NULL){
      rot_angle(cb->calc_quaternion_type,p->quaternion);
      normalize = ks_gl_enable(GL_NORMALIZE);
      glScalef(p->atom->gl_atom->scale[0],p->atom->gl_atom->scale[1],p->atom->gl_atom->scale[2]);
      /*
      printf("%f %f %f %f\n",p->quaternion[0],p->quaternion[1],p->quaternion[2],p->quaternion[3]);
      */
    }
    /*
    glutSolidSphere(ks_get_gl_atom_size(p->atom->gl_atom,size)*cb->particle_size_scale,
		    detail*2,detail);
    */
    ks_draw_gl_sphere_obj(cb->sphere_obj[detail],
			  ks_get_gl_atom_size(p->atom->gl_atom,size)*cb->particle_size_scale);

    if(p->atom->gl_atom->scale != NULL && normalize == GL_TRUE){
      glDisable(GL_NORMALIZE);
    }

    if(cb->label_mode != CV_LABEL_MODE_OFF)
      draw_label(gb,p,cb->particle_size,cb->particle_size_scale,cb->label_mode,
		 fr->prep_block != NULL);

    if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_ORDER){
      draw_particle_order(gb,p,cb->particle_size,cb->particle_size_scale,particle_order);
    } else if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_MAIN){
      draw_particle_main(gb,p,cb->particle_size,cb->particle_size_scale);
    }

    glPopMatrix();
    /*
      if(p->property->alternative != NULL){
      CV_PARTICLE *alt;
      for(alt = p->property->alternative; alt != NULL; alt = alt->next){
      glPushName(alt->label);
      glPushMatrix();
      glTranslated(alt->cd[0],alt->cd[1],alt->cd[2]);
      ks_draw_gl_atom(p->atom->gl_atom,
      p->color_mode[color_mode],
      KS_GL_ATOM_SIZE_NORMAL,
      KS_GL_ATOM_BRIGHT_LOW,
      KS_GL_ATOM_TRANS_OFF);
      glPopMatrix();
      glPopName();
      }
      }
    */
    glPopName();

    glLoadName(CV_NAME_DEFAULT);
  } else {
    BOOL flg[6];
    if(cb->clip_plane_flg == NULL){
      for(i = 0; i < 6; i++){
	flg[i] = KS_TRUE;
      }
    } else {
      for(i = 0; i < 6; i++){
	if(cb->clip_plane_flg[i] == KS_FALSE || 
	   (cb->clip_plane_flg[i] == KS_TRUE && 
	    cb->clip_plane[i][0]*p->cd[0]+
	    cb->clip_plane[i][1]*p->cd[1]+
	    cb->clip_plane[i][2]*p->cd[2] + cb->clip_plane[i][3] > 0)){
	  flg[i] = KS_TRUE;
	} else {
	  flg[i] = KS_FALSE;
	}
      }
    }
    if(flg[0] && flg[1] && flg[2] && flg[3] && flg[4] && flg[5]){
      get_color(p,cb->color_mode,KS_GL_ATOM_BRIGHT_NORMAL,color,fr->prep_block != NULL);
      draw_pov_sphere(pov,ks_get_gl_atom_size(p->atom->gl_atom,
					      cb->particle_size)*cb->particle_size_scale,
		      p->cd[0],p->cd[1],p->cd[2],color,cb->pov_info);
      /*
      fprintf(pov,"sphere{0.0,%f\n",
	      ks_get_gl_atom_size(p->atom->gl_atom,cb->particle_size)*cb->particle_size_scale);
      fprintf(pov," translate<% f,% f,% f>\n",p->cd[0],p->cd[1],p->cd[2]);
      fprintf(pov," texture{\n");
      get_color(p,color_mode,KS_GL_ATOM_BRIGHT_NORMAL,color);
      fprintf(pov,"  pigment{ color rgb <%f,%f %f>}\n",color[0],color[1],color[2]);
      fprintf(pov,"  finish{\n");
      fprintf(pov,"   diffuse 0.9\n");
      fprintf(pov,"   ambient 0.4\n");
      fprintf(pov,"  }\n");
      fprintf(pov," }\n");
      fprintf(pov,"}\n");
      */
    }
  }

  if(draw_bond_flg == KS_TRUE){
    draw_bond(gb,p,cb->color_mode,cb->draw_mode,KS_FALSE,KS_TRUE,bright_low,pov,cb->pov_info);
  }

}
static void draw_inside_face(double side[2][2][2][3], GLfloat color[4])
{
  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
  ks_draw_gl_box(side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		 side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],KS_DRAW_GL_BOX_FRONT);
}
static void draw_particle_inside(KS_GL_BASE *gb, CV_PARTICLE *p, CV_BASE *cb, CV_FRAME *fr,
				 BOOL bright_low, int trans_mode, BOOL draw_bond_flg, 
				 FILE *pov, int particle_order, 
				 GLfloat offset[3], double side[2][2][2][3])
{
  glEnable(GL_DEPTH_TEST);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glCullFace(GL_BACK);
  draw_inside_face(side,cb->periodic_inside_color);

  glDepthMask(GL_FALSE);
  glEnable(GL_STENCIL_TEST);
  glStencilFunc(GL_ALWAYS, 0, 0);
  glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
  glCullFace(GL_BACK);
  glPushMatrix();
  glTranslatef(offset[0],offset[1],offset[2]);
  draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);

  glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
  glCullFace(GL_FRONT);
  draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
  glPopMatrix();

  glDepthMask(GL_TRUE);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

  glStencilFunc(GL_NOTEQUAL, 0, 1);
  glDisable(GL_DEPTH_TEST);

  glCullFace(GL_BACK);
  draw_inside_face(side,cb->periodic_inside_color);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
}
static void draw_ribbon(KS_GL_BASE *gb, CV_PARTICLE *p, int color_mode, KS_GL_BUFFER_3D *ribbon,
			BOOL bright_low, int draw_mode)
{
  int i;
  GLfloat color[4];
  GLfloat alpha_color[4] = {1.0, 0.0, 0.0, 1.0};
  GLfloat beta_color[4]  = {0.0, 1.0, 1.0, 1.0};
  GLfloat loop_color[4]  = {0.7, 0.7, 0.7, 1.0};
  int color_type;

  glLoadName(CV_NAME_RIBBON);

  if(p->flags&CV_PARTICLE_PRIME){
    get_color(p,color_mode,KS_GL_ATOM_BRIGHT_NORMAL,color,KS_FALSE);
    if(color_mode == CV_COLOR_MODE_PARTICLE){
      color_type = KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR;
    } else if(color_mode == CV_COLOR_MODE_AMINO){
      color_type = KS_DRAW_GL_RIBBON_USER_COLOR;
    } else if(color_mode == CV_COLOR_MODE_AMINO_TYPE){
      color_type = KS_DRAW_GL_RIBBON_USER_COLOR;
    } else if(color_mode == CV_COLOR_MODE_AMINO_HYDROPATHY){
      color_type = KS_DRAW_GL_RIBBON_USER_COLOR;
    } else if(color_mode == CV_COLOR_MODE_CHAIN){
      color_type = KS_DRAW_GL_RIBBON_USER_COLOR;
    } else if(color_mode == CV_COLOR_MODE_AGGREGATE){
      color_type = KS_DRAW_GL_RIBBON_RAINBOW;
      color_type = KS_DRAW_GL_RIBBON_USER_COLOR;
    } else if(color_mode == CV_COLOR_MODE_RAINBOW){
      color_type = KS_DRAW_GL_RIBBON_RAINBOW;
    } else {
      color_type = KS_DRAW_GL_RIBBON_USER_COLOR;
    }

    if(bright_low == KS_TRUE){
      if(color_type == KS_DRAW_GL_RIBBON_USER_COLOR){
	for(i = 0; i < 3; i++) color[i] *= 0.5;
      } else if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
	for(i = 0; i < 3; i++) alpha_color[i] *= 0.5;
	for(i = 0; i < 3; i++) beta_color[i] *= 0.5;
	for(i = 0; i < 3; i++) loop_color[i] *= 0.5;
      } else if(color_type == KS_DRAW_GL_RIBBON_RAINBOW){
	color_type = KS_DRAW_GL_RIBBON_RAINBOW_LOW;
      }
    }
    if(draw_mode == CV_DRAW_MODE_RIBBON){
      ks_set_gl_buffer_3d(ribbon,
			  p->cd[0]+p->move[0],p->cd[1]+p->move[1],p->cd[2]+p->move[2],
			  p->property->residue->vec_C_O[0],
			  p->property->residue->vec_C_O[1],
			  p->property->residue->vec_C_O[2],color,p->label,
			  p->property->residue->flags);
    } else if(draw_mode == CV_DRAW_MODE_TUBE){
      ks_set_gl_buffer_3d(ribbon,
			  p->cd[0]+p->move[0],p->cd[1]+p->move[1],p->cd[2]+p->move[2],
			  p->property->residue->vec_C_O[0],
			  p->property->residue->vec_C_O[1],
			  p->property->residue->vec_C_O[2],color,p->label,
			  p->property->residue->flags&~(CV_RESIDUE_ALPHA|CV_RESIDUE_BETA));
    }
    /*
    printf("%d %s %d %s end %c\n"
	   ,p->label,p->property->name
	   ,p->property->residue->file_label
	   ,p->property->residue->name
	   ,p->property->residue->flags&CV_RESIDUE_END?'o':'x'
	   );
    */
    if(p->property->residue->flags&CV_RESIDUE_END){
      /*
      int i;
      for(i = 0; i < ribbon->num; i++){
	printf("%d %d %-5s %-5s %f %f %f %f %f %f\n",i,ribbon->label[i],
	       ribbon->flags[i]&KS_MD_RESIDUE_NORMAL ? "NORMAL":
	       ribbon->flags[i]&KS_MD_RESIDUE_START ? "START":
	       ribbon->flags[i]&KS_MD_RESIDUE_END ? "END":"NON",
	       ribbon->flags[i]&KS_MD_RESIDUE_ALPHA ? "ALPHA":
	       ribbon->flags[i]&KS_MD_RESIDUE_BETA  ? "BETA" :
	       ribbon->flags[i]&KS_MD_RESIDUE_LOOP  ? "LOOP" : "NON",
	       ribbon->x[i],ribbon->y[i],ribbon->z[i],
	       ribbon->nx[i],ribbon->ny[i],ribbon->nz[i]);
      }
      */
      ks_draw_gl_ribbon(gb,ribbon,color_type,alpha_color,beta_color,loop_color);
      ks_clear_gl_buffer_3d(ribbon);

    }
  }
  glLoadName(CV_NAME_DEFAULT);
}
BOOL is_selected_particle_range(CV_PARTICLE *p, CV_PARTICLE *q, double r)
{
  int i;
  double dd[3];

  if(q == NULL) return KS_FALSE;

  for(i = 0; i < 3; i++)
    dd[i] = p->cd[i] - q->cd[i];

  if(r*r > dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
static void draw_color_map(KS_GL_BASE *gb, CV_COLOR_MAP *color_map)
{
  int i;
  int x,y;
  int font = KS_GL_FONT_HELVETICA_12, font_height = 12, font_margin = 4;
  int len;
  static int len_max = -1;
  char value[64];
  int map_width = 10, margin = 25;
  /*
  x = ks_get_gl_viewport_2d_width(gb) - 25;
  y = ks_get_gl_viewport_2d_height(gb) - 200;
  */
  x = margin + map_width;
  y = ks_get_gl_viewport_2d_height(gb) -(font_height+font_margin)*color_map->num- 20;

  if(len_max < 0){
    for(i = 0; i < color_map->num; i++){
      ks_gl_base_foreground_color(gb);
      snprintf(value,sizeof(value),"%.4f",color_map->value[i]);
      ks_get_glut_font_width(gb,font,value,&len);
      if(len_max < len) len_max = len;
    }
    len_max -= 5;
  }
  for(i = 0; i < color_map->num; i++){
    ks_gl_base_foreground_color(gb);
    snprintf(value,sizeof(value),"%.4f",color_map->value[i]);
    ks_get_glut_font_width(gb,font,value,&len);
    glRasterPos2i(x-len+map_width+len_max,y+i*(font_height+font_margin)-.5*font_height);
    ks_use_glut_font(gb,font);
    glCallLists(strlen(value), GL_BYTE, value);
  }
  for(i = 0; i < color_map->num-1; i++){
    glBegin(GL_QUADS);
    glColor3fv(color_map->color[i]);
    glVertex2i(margin+map_width,y+i*(font_height+font_margin));
    glVertex2i(margin,y+i*(font_height+font_margin));
    glColor3fv(color_map->color[i+1]);
    glVertex2i(margin,y+(i+1)*(font_height+font_margin));
    glVertex2i(margin+map_width,y+(i+1)*(font_height+font_margin));
    glEnd();
  }
}
static void draw_additional_surface(CV_ADDITIONAL_SURFACE *sur)
{
  int i,j,k;
  GLboolean blend;
  blend = ks_gl_enable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  if(sur->cull_face == KS_FALSE){
    glDisable(GL_CULL_FACE);
  }

  glBegin(GL_TRIANGLES);
  for(i = 0; i < sur->face_num; i++){
    for(j = 0; j < 3; j++){
      k = sur->face[i*3+j];
      sur->color[k*4+3] = sur->transparency;
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,&sur->color[k*4]);
      glNormal3fv(&sur->normal[k*3]);
      glVertex3fv(&sur->vertex[k*3]);
    }
  }
  glEnd();
  if(blend == GL_TRUE)
    glDisable(GL_BLEND);

  if(sur->cull_face == KS_FALSE){
    glEnable(GL_CULL_FACE);
  }
}
static void draw_pdb_user_bond(KS_GL_BASE *gb, CV_BOND* pdb_user_bond, int color_mode, 
			       int draw_mode, int draw_water_mode, BOOL bright_low, 
			       FILE *pov, CV_POV_INFO pov_info)
{
  int i;
  CV_BOND *bond;
  CV_PARTICLE *p[2];
  int p_mode[2];
  for(bond = pdb_user_bond; bond != NULL; bond = bond->next->next){
    ks_assert(bond->next != NULL);
    p[0] = bond->p;
    p[1] = bond->next->p;
    for(i = 0; i < 2; i++){
      if(p[i]->flags&CV_PARTICLE_WATER){
	if(draw_water_mode == CV_DRAW_WATER_MODE_LINE){
	  p_mode[i] = CV_DRAW_MODE_LINE;
	} else if(draw_water_mode == CV_DRAW_WATER_MODE_PARTICLE){
	  p_mode[i] = CV_DRAW_MODE_PARTICLE;
	} else {
	  p_mode[i] = CV_DRAW_MODE_OFF;
	}
      } else {
	p_mode[i] = draw_mode;
      }
    }
    if((p_mode[0] == CV_DRAW_MODE_LINE &&     p_mode[1] == CV_DRAW_MODE_LINE) ||
       (p_mode[0] == CV_DRAW_MODE_PARTICLE && p_mode[1] == CV_DRAW_MODE_LINE) ||
       (p_mode[0] == CV_DRAW_MODE_LINE &&     p_mode[1] == CV_DRAW_MODE_PARTICLE)){
      if(bond->type == NULL){
	draw_bond_particles(gb,p[0],p[1],bond,color_mode,draw_mode,KS_TRUE,KS_FALSE,
			    bright_low,pov,pov_info);
	draw_bond_particles(gb,p[1],p[0],bond,color_mode,draw_mode,KS_TRUE,KS_FALSE,
			    bright_low,pov,pov_info);
      } else {
	draw_bond_particles(gb,p[0],p[1],bond,color_mode,draw_mode,KS_TRUE,KS_FALSE,
			    bright_low,pov,pov_info);
      }
    } else if(p_mode[0] == CV_DRAW_MODE_PARTICLE && p_mode[1] == CV_DRAW_MODE_PARTICLE){
      draw_bond_particles(gb,p[0],p[1],bond,color_mode,draw_mode,KS_FALSE,KS_TRUE,
			  bright_low,pov,pov_info);
    }
  }
}
void draw_frame(KS_GL_BASE *gb, CV_FRAME *fr, CV_BASE *cb,
		BOOL bright_low, int trans_mode, BOOL draw_bond_flg,
		CV_PARTICLE *selected_particle,	FILE *pov)
{
  CV_PARTICLE *p;
  int particle_order;
  /*
  printf("%p %p\n",fr,fr->particle_end);
  */
  /*
  printf("%p %p %f %f %f\n",fr,fr->particle_end
	 ,fr->particle_end->cd[0]
	 ,fr->particle_end->cd[1]
	 ,fr->particle_end->cd[2]);
  */
  /*  printf("draw_mode %d\n",draw_mode);*/
  if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_ORDER){
    particle_order = 1;
  } else {
    particle_order = -1;
  }
  for(p = fr->particle_end; p != NULL; p = p->prev){
    /*
    printf("%3d %s %s %s %f %f %f %c %c\n",
	   p->label,
	   p->property==NULL?"x":p->property->residue->name,
	   p->property==NULL?"x":p->property->name,
	   p->atom->name,p->cd[0],p->cd[1],p->cd[2],
	   p->flags&CV_PARTICLE_MAIN?'o':'x',
	   p->flags&CV_PARTICLE_PRIME?'o':'x');
    */
    if(!(p->flags&CV_PARTICLE_WATER) && !(p->flags&CV_PARTICLE_SOLVENT)){
      if(cb->draw_mode == CV_DRAW_MODE_LINE || cb->show_flags&CV_SHOW_LINE){
	if(cb->show_selected_particle_mode == CV_SHOW_SELECTED_PARTICLE_MODE_OFF || 
	   (cb->show_selected_particle_mode == CV_SHOW_SELECTED_PARTICLE_MODE_ON && 
	    p->flags&CV_PARTICLE_SELECTED) ||
	   (cb->show_selected_particle_mode == CV_SHOW_SELECTED_PARTICLE_MODE_RANGE && 
	    is_selected_particle_range(p,selected_particle,cb->show_selected_particle_range)
	    ==KS_TRUE)
	   ){
	  if(pov == NULL){
	    if(p->bond != NULL){
	      draw_bond(gb,p,cb->color_mode,cb->draw_mode,KS_TRUE,KS_FALSE,bright_low,NULL,
			cb->pov_info);
	    } else {
	      draw_particle_dot(gb,p,cb->color_mode,cb->label_mode,bright_low,&cb->continuous_color,
				&fr->c_color_info);
	    }
	  } else {
	    draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
			  
	  }
	}
      } 
      if(cb->draw_mode == CV_DRAW_MODE_PARTICLE || cb->show_flags&CV_SHOW_PARTICLE){
	if(cb->show_selected_particle_mode == CV_SHOW_SELECTED_PARTICLE_MODE_OFF || 
	   (cb->show_selected_particle_mode == CV_SHOW_SELECTED_PARTICLE_MODE_ON && 
	    p->flags&CV_PARTICLE_SELECTED) ||
	   (cb->show_selected_particle_mode == CV_SHOW_SELECTED_PARTICLE_MODE_RANGE && 
	    is_selected_particle_range(p,selected_particle,cb->show_selected_particle_range)
	    ==KS_TRUE)
	   ){
	  draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
			
	}
      } 
      if(cb->draw_mode == CV_DRAW_MODE_SOLVENT_EXCLUDE_SURFACE){
	if(p->property == NULL || p->property->chain->ses == NULL){
	  draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
	}
      } 
      if(cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN || 
		cb->draw_mode == CV_DRAW_MODE_TUBE ||
		cb->draw_mode == CV_DRAW_MODE_RIBBON){
	/*
	printf("%s %p %d %d\n",p->property->residue->name
	       ,p->property,p->property->chain->flags&CV_CHAIN_AMINO
	       ,p->property->residue->flags&CV_RESIDUE_AMINO);
	*/
	if(p->property == NULL){
	  draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
	} else if((p->property->chain->len > 1 && p->property->chain->flags&CV_CHAIN_AMINO && 
		   p->property->residue->flags&CV_RESIDUE_AMINO) || 
		  (fr->pdb_ext == KS_TRUE && cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN)){
	  if(cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN){
	    if(p->flags&CV_PARTICLE_MAIN){
	      draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
	    }
	  } else if(cb->draw_mode == CV_DRAW_MODE_TUBE || cb->draw_mode == CV_DRAW_MODE_RIBBON){
	    draw_ribbon(gb,p,cb->color_mode,cb->ribbon,bright_low,cb->draw_mode);
	  }
	} else {
	  draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
	}
      }
    } else {
      if(cb->draw_water_mode == CV_DRAW_WATER_MODE_LINE){
	if(p->bond != NULL){
	  draw_bond(gb,p,cb->color_mode,cb->draw_mode,KS_TRUE,KS_FALSE,bright_low,NULL,
		    cb->pov_info);
	} else {
	  draw_particle_dot(gb,p,cb->color_mode,cb->label_mode,bright_low,&cb->continuous_color,
			    &fr->c_color_info);
	}
      } else if(cb->draw_water_mode == CV_DRAW_WATER_MODE_PARTICLE){
	draw_particle(gb,p,cb,fr,bright_low,trans_mode,draw_bond_flg,pov,particle_order);
      }
    }
    if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_ORDER){
      ++particle_order;
    }
  }
  if(cb->draw_mode == CV_DRAW_MODE_SOLVENT_EXCLUDE_SURFACE){
    draw_solvent_exclude_surface(fr,cb->color_mode,bright_low);
  }
  if(cb->draw_additional_surface == KS_TRUE && fr->sur != NULL){
    draw_additional_surface(fr->sur);
  }
  if(fr->pdb_user_bond != NULL){
    draw_pdb_user_bond(gb,fr->pdb_user_bond,cb->color_mode,cb->draw_mode,cb->draw_water_mode,
		       bright_low,pov,cb->pov_info);
  }
}
static void draw_side_pov(FILE *pov, double side[2][2][2][3], double r, CV_POV_INFO pov_info)
{
  GLfloat color[3] = {1.0, 1.0, 1.0};
  int i,j,k;

  for(i = 0; i < 2; i++){
    for(j = 0; j < 2; j++){
      for(k = 0; k < 2; k++){
	draw_pov_sphere(pov,r,
			side[i][j][k][0],side[i][j][k][1],side[i][j][k][2],
			color,pov_info);
      }
    }
  }

  /* lower loop */
  draw_pov_bond(pov,
		side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		side[0][1][0][0],side[0][1][0][1],side[0][1][0][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[0][1][0][0],side[0][1][0][1],side[0][1][0][2],
		side[1][1][0][0],side[1][1][0][1],side[1][1][0][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[1][1][0][0],side[1][1][0][1],side[1][1][0][2],
		side[1][0][0][0],side[1][0][0][1],side[1][0][0][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[1][0][0][0],side[1][0][0][1],side[1][0][0][2],
		side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		r,color,pov_info);

  /* upper loop */
  draw_pov_bond(pov,
		side[0][0][1][0],side[0][0][1][1],side[0][0][1][2],
		side[0][1][1][0],side[0][1][1][1],side[0][1][1][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[0][1][1][0],side[0][1][1][1],side[0][1][1][2],
		side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],
		side[1][0][1][0],side[1][0][1][1],side[1][0][1][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[1][0][1][0],side[1][0][1][1],side[1][0][1][2],
		side[0][0][1][0],side[0][0][1][1],side[0][0][1][2],
		r,color,pov_info);
  /* side pillar */
  draw_pov_bond(pov,
		side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		side[0][0][1][0],side[0][0][1][1],side[0][0][1][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[0][1][0][0],side[0][1][0][1],side[0][1][0][2],
		side[0][1][1][0],side[0][1][1][1],side[0][1][1][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[1][1][0][0],side[1][1][0][1],side[1][1][0][2],
		side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],
		r,color,pov_info);
  draw_pov_bond(pov,
		side[1][0][0][0],side[1][0][0][1],side[1][0][0][2],
		side[1][0][1][0],side[1][0][1][1],side[1][0][1][2],
		r,color,pov_info);
}
static void draw_side(KS_GL_BASE *gb, GLfloat color[4], double side[2][2][2][3], GLfloat r)
{
  glDisable(GL_LIGHTING);
  //  ks_gl_base_foreground_color(gb);
  glColor4fv(color);

  if(r > 0){
    /* lower loop */
    ks_draw_gl_rod_pos(gb,
		       side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		       side[0][1][0][0],side[0][1][0][1],side[0][1][0][2],
		       r,KS_DRAW_GL_ROD_COVER_START_AND_END,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[0][1][0][0],side[0][1][0][1],side[0][1][0][2],
		       side[1][1][0][0],side[1][1][0][1],side[1][1][0][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[1][1][0][0],side[1][1][0][1],side[1][1][0][2],
		       side[1][0][0][0],side[1][0][0][1],side[1][0][0][2],
		       r,KS_DRAW_GL_ROD_COVER_START_AND_END,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[1][0][0][0],side[1][0][0][1],side[1][0][0][2],
		       side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);

    /* upper loop */
    ks_draw_gl_rod_pos(gb,
		       side[0][0][1][0],side[0][0][1][1],side[0][0][1][2],
		       side[0][1][1][0],side[0][1][1][1],side[0][1][1][2],
		       r,KS_DRAW_GL_ROD_COVER_START_AND_END,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[0][1][1][0],side[0][1][1][1],side[0][1][1][2],
		       side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],
		       side[1][0][1][0],side[1][0][1][1],side[1][0][1][2],
		       r,KS_DRAW_GL_ROD_COVER_START_AND_END,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[1][0][1][0],side[1][0][1][1],side[1][0][1][2],
		       side[0][0][1][0],side[0][0][1][1],side[0][0][1][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
    /* side pillar */
    ks_draw_gl_rod_pos(gb,
		       side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],
		       side[0][0][1][0],side[0][0][1][1],side[0][0][1][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[0][1][0][0],side[0][1][0][1],side[0][1][0][2],
		       side[0][1][1][0],side[0][1][1][1],side[0][1][1][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[1][1][0][0],side[1][1][0][1],side[1][1][0][2],
		       side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
    ks_draw_gl_rod_pos(gb,
		       side[1][0][0][0],side[1][0][0][1],side[1][0][0][2],
		       side[1][0][1][0],side[1][0][1][1],side[1][0][1][2],
		       r,KS_DRAW_GL_ROD_COVER_NON,KS_FALSE);
  }
  glLineWidth(1.0);
  glBegin(GL_LINE_LOOP);
  glVertex3dv(side[0][0][0]);
  glVertex3dv(side[0][1][0]);
  glVertex3dv(side[1][1][0]);
  glVertex3dv(side[1][0][0]);
  glEnd();
  glBegin(GL_LINE_LOOP);
  glVertex3dv(side[0][0][1]);
  glVertex3dv(side[0][1][1]);
  glVertex3dv(side[1][1][1]);
  glVertex3dv(side[1][0][1]);
  glEnd();
  glBegin(GL_LINES);
  glVertex3dv(side[0][0][0]);
  glVertex3dv(side[0][0][1]);
  glVertex3dv(side[0][1][0]);
  glVertex3dv(side[0][1][1]);
  glVertex3dv(side[1][0][0]);
  glVertex3dv(side[1][0][1]);
  glVertex3dv(side[1][1][0]);
  glVertex3dv(side[1][1][1]);
  glEnd();
  glEnable(GL_LIGHTING);
}
static void __KS_USED__ draw_particle_polygon(CV_BASE *cb, unsigned int *labels, int num, 
					      GLfloat color[4])
{
  int i;
  CV_PARTICLE *p;
  GLboolean blend;

  blend = ks_gl_enable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glColor4fv(color);
  glBegin(GL_POLYGON);
  for(i = 0; i < num; i++){
    p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,labels[i]);
    glVertex3dv(p->cd);
  }
  for(i = num-1; i >= 0; i--){
    p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,labels[i]);
    glVertex3dv(p->cd);
  }
  glEnd();
  if(blend == GL_TRUE)
    glDisable(GL_BLEND);
}
static void adjust_depth_range(KS_GL_BASE *gb, double side[2][2][2][3])
{
  int i,j,k;
  double mat[16];
  double m[3],min,max;
  /*
  glGetDoublev(GL_MODELVIEW_MATRIX,mat);
  ks_multi_gl_matrix(mat,side[0][0][0],m);
  printf("(% f % f % f)(% f % f % f)\n"
	 ,side[0][0][0][0],side[0][0][0][1],side[0][0][0][2],m[0],m[1],m[2]);
  ks_multi_gl_matrix(mat,side[1][1][1],m);
  printf("(% f % f % f)(% f % f % f)\n"
	 ,side[1][1][1][0],side[1][1][1][1],side[1][1][1][2],m[0],m[1],m[2]);
  for(i = 0; i < 4; i++){
    for(j = 0; j < 4; j++){
      printf("% f ",mat[i*4+j]);
    }
    printf("\n");
  }
  */

  glPushMatrix();
  ks_get_gl_modelview_matrix(gb,mat);
  glPopMatrix();
  /*
  for(i = 0; i < 4; i++){
    for(j = 0; j < 4; j++){
      printf("% f ",mat[i*4+j]);
    }
    printf("\n");
  }
  */
  ks_multi_gl_matrix(mat,side[0][0][0],m);
  min = m[2];
  max = m[2];
  for(i = 0; i < 2; i++){
    for(j = 0; j < 2; j++){
      for(k = 1; k < 2; k++){
	ks_multi_gl_matrix(mat,side[i][j][k],m);
	if(min > m[2]) min = m[2];
	if(max < m[2]) max = m[2];
      }
    }
  }
  //  printf("%f %f\n",min,max);

  //  ks_set_gl_base_depth_3d_near(gb, ks_max(1.0,-max*.5));
  ks_set_gl_base_depth_3d_far(gb, -min*2);
}
void cv_adjust_depth_range(KS_GL_BASE *gb, CV_BASE *cb)
{
  CV_FRAME *fr;
  fr = &cb->frame[cb->current_frame];

  if(cb->kabe_mode == CV_KABE_MODE_SIDE && fr->have_side == KS_TRUE){
    adjust_depth_range(gb,fr->side);
  } else {
    int i,j,k;
    double side[2][2][2][3];
    for(i = 0; i < 2; i++){
      for(j = 0; j < 2; j++){
	for(k = 0; k < 2; k++){
	  side[i][j][k][0] = (cb->reference_frame->center[0] + 
			      cb->reference_frame->system_size*(i == 0 ? -1:1));
	  side[i][j][k][1] = (cb->reference_frame->center[1] + 
			      cb->reference_frame->system_size*(j == 0 ? -1:1));
	  side[i][j][k][2] = (cb->reference_frame->center[2] + 
			      cb->reference_frame->system_size*(k == 0 ? -1:1));
	}
      }
    }
    adjust_depth_range(gb,side);
  }
}
static BOOL check_periodic(int p_cond, CV_PARTICLE *p, double r, double side[2][2][2][3], 
			   GLfloat *offset)
{
  int ix,iy,iz;
  ix = (p_cond/9);
  iy = (p_cond/3)%3;
  iz = p_cond%3;
  /*
  printf("%2d %f %f %f %f %d %d %d (%d %d %d)(%d %d %d)(%d %d %d)\n",p_cond,
	 p->cd[0],p->cd[1],p->cd[2],r,
	 ix,iy,iz,
	 ((ix == 0 && p->cd[0]-r < side[0][0][0][0]) || (ix == 1)|| 
	  (ix == 2 && p->cd[0]+r > side[1][1][1][0])),
	 ((iy == 0 && p->cd[1]-r < side[0][0][0][1]) || (iy == 1)|| 
	  (iy == 2 && p->cd[1]+r > side[1][1][1][1])),
	 ((iz == 0 && p->cd[2]-r < side[0][0][0][2]) || (iz == 1)|| 
	  (iz == 2 && p->cd[2]+r > side[1][1][1][2])),
	 p->cd[0]-r < side[0][0][0][0],
	 p->cd[1]-r < side[0][0][0][1],
	 p->cd[2]-r < side[0][0][0][2],
	 p->cd[0]+r > side[1][1][1][0],
	 p->cd[1]+r > side[1][1][1][1],
	 p->cd[2]+r > side[1][1][1][2]);
  */
  if(offset != NULL){
    double len[3];
    len[0] = side[1][1][1][0] - side[0][0][0][0];
    len[1] = side[1][1][1][1] - side[0][0][0][1];
    len[2] = side[1][1][1][2] - side[0][0][0][2];
    offset[0] = (ix == 0) ? len[0]: (ix == 2) ? -len[0]: 0; 
    offset[1] = (iy == 0) ? len[1]: (iy == 2) ? -len[1]: 0; 
    offset[2] = (iz == 0) ? len[2]: (iz == 2) ? -len[2]: 0; 
  }

  if(p != NULL){
    if(ix == 1 && iy == 1 && iz == 1){
      if(p->cd[0]-r < side[0][0][0][0] || p->cd[0]+r > side[1][1][1][0] ||
	 p->cd[1]-r < side[0][0][0][1] || p->cd[1]+r > side[1][1][1][1] ||
	 p->cd[2]-r < side[0][0][0][2] || p->cd[2]+r > side[1][1][1][2]){
	return KS_TRUE;
      } else {
	return KS_FALSE;
      }
    }
    if(((ix == 0 && p->cd[0]-r < side[0][0][0][0]) || (ix == 1)|| 
	(ix == 2 && p->cd[0]+r > side[1][1][1][0])) &&
       ((iy == 0 && p->cd[1]-r < side[0][0][0][1]) || (iy == 1)|| 
	(iy == 2 && p->cd[1]+r > side[1][1][1][1])) &&
       ((iz == 0 && p->cd[2]-r < side[0][0][0][2]) || (iz == 1)|| 
	(iz == 2 && p->cd[2]+r > side[1][1][1][2]))){
      return KS_TRUE;
    } else {
      return KS_FALSE;
    }
  }
  return KS_FALSE;

}
void cv_draw(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i,j;
  CV_FRAME *fr;
  GLfloat offset[3];
  GLenum gl_clip_plane[] = {GL_CLIP_PLANE0,GL_CLIP_PLANE1,GL_CLIP_PLANE2,
			    GL_CLIP_PLANE3,GL_CLIP_PLANE4,GL_CLIP_PLANE5};
  BOOL draw_bond_flg;
#if USE_GLSL
  float projection_matrix[16];
  float view_matrix[16];
  glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix);
  glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
#endif

  glLoadName(CV_NAME_DEFAULT);

  fr = &cb->frame[cb->current_frame];

  draw_bond_flg = (cb->particle_size == CV_PARTICLE_SIZE_SMALL || 
		   fr->flags&CV_FRAME_CDV) ? KS_TRUE:KS_FALSE;

  if(cb->clip_plane_flg != NULL){
    for(i = 0; i < 6; i++){
      if(cb->clip_plane_flg[i]== KS_TRUE){
	glClipPlane(gl_clip_plane[i], cb->clip_plane[i]);
	glEnable(gl_clip_plane[i]);
      }
    }
  }

  if(cb->kabe_mode == CV_KABE_MODE_HAKO){
#ifdef USE_GLSL
    ks_draw_gl_standard_shader(cb->sb->shader[SHADER_REGULAR],OBJ_TYPE_BOX,
			       projection_matrix, view_matrix);
#else
    draw_hako(cb->reference_frame->system_size,cb->reference_frame->center);
#endif
  }
  if(cb->overlap_reference == KS_TRUE && !(fr->flags&CV_FRAME_REFERENCE)){
    draw_frame(gb,cb->reference_frame,cb,KS_TRUE,KS_GL_ATOM_TRANS_OFF,draw_bond_flg,NULL,NULL);
  }

  glPushMatrix();

  if(cb->fit_reference == KS_TRUE){
    ks_add_gl_rotate(gb,fr->fit_rot);
    glTranslated(fr->fit_trans[0],fr->fit_trans[1],fr->fit_trans[2]);
  }

  if(cb->clip_plane_flg != NULL){
    for(i = 0 ; i < 6; i++){
      if(cb->clip_plane_flg[i] == KS_TRUE){
	glDisable(gl_clip_plane[i]);
      }
    }
  }
  if(cb->kabe_mode == CV_KABE_MODE_SIDE && fr->have_side == KS_TRUE){
    draw_side(gb,cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT],fr->side,fr->side_radius);
  }
  if(cb->clip_plane_flg != NULL){
    for(i = 0; i < 6; i++){
      if(cb->clip_plane_flg[i]== KS_TRUE){
	glClipPlane(gl_clip_plane[i], cb->clip_plane[i]);
	glEnable(gl_clip_plane[i]);
      }
    }
  }

#ifdef USE_GLSL
  ks_draw_gl_standard_shader(cb->sb->shader[SHADER_SPHERE],cb->detail,
			     projection_matrix, view_matrix);
  if(draw_bond_flg == KS_TRUE){
    int rod_type;
    if(cb->detail >= CV_SPHERE_DETAIL_NORMAL){
      rod_type = OBJ_TYPE_ROD_NORMAL;
    } else {
      rod_type = OBJ_TYPE_ROD_LOW;
    }
    ks_draw_gl_standard_shader(cb->sb->shader[SHADER_REGULAR],rod_type,
			       projection_matrix, view_matrix);
  }
#else
  {
    CV_PARTICLE *p;
    if(ks_count_int_list(cb->selected_label) == 1){
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
					cb->selected_label->value);
    } else if(ks_count_int_list(cb->selected_label) == 2){
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
					cb->selected_label->next->value);
    } else {
      p = NULL;
    }
    draw_frame(gb,fr,cb,KS_FALSE,KS_GL_ATOM_TRANS_OFF,draw_bond_flg,p,NULL);
  }
#endif

  /*
  {
    int labels[] = {3,4,15,13,12,7};
    int num = 6;
    GLfloat color[4];
    glDisable(GL_LIGHTING);
    color[0] = 0.5; color[1] = 0.0; color[2] = 0.5; color[3] = 0.5;
    draw_particle_polygon(cb,labels,num,color);
    glEnable(GL_LIGHTING);
  }
  {
    int labels[] = {2,9,13,4};
    int num = 4;
    GLfloat color[4];
    glDisable(GL_LIGHTING);
    color[0] = 0.0; color[1] = 0.5; color[2] = 0.5; color[3] = 0.5;
    draw_particle_polygon(cb,labels,num,color);
    glEnable(GL_LIGHTING);
  }
  */

  glPopMatrix();

  if(ks_gl_base_verbose_level(gb) >= 1 && fr->fv != NULL && fr->fv->field != NULL){
    fv_draw_field(gb,fr->fv->field,fr->fv->field_size,fr->fv->contour_size,fr->fv->contour_fan,
		  &cb->fv_target,NULL);
#ifdef CURSOR
    fv_draw_cursor();
#endif
  }

  if(cb->move_mode == KS_TRUE){
    ks_draw_gl_move_target_plane(gb,20,10,0.5,0.5,0.5);
  }

  if(cb->periodic_mode == KS_TRUE){
    for(i = 0; i < 27; i++){
      if(i != 13){
	check_periodic(i,NULL,0,fr->side,offset);
	//      printf("%d % f % f % f\n",i,offset[0],offset[1],offset[2]);
	glPushMatrix();
	glTranslatef(offset[0],offset[1],offset[2]);
	for(j = 0; j < fr->periodic_p_num[i]; j++){
	  draw_particle(gb,fr->periodic_p[i][j],cb,fr,KS_FALSE,KS_GL_ATOM_TRANS_OFF,draw_bond_flg,
			NULL,0);
	}
	glPopMatrix();
      }
    }
  }

  if(cb->clip_plane_flg != NULL){
    for(i = 0 ; i < 6; i++){
      if(cb->clip_plane_flg[i] == KS_TRUE){
	glDisable(gl_clip_plane[i]);
      }
    }
  }

  if(cb->periodic_mode == KS_TRUE){
    for(i = 0; i < 27; i++){
      check_periodic(i,NULL,0,fr->side,offset);
      //      printf("%d % f % f % f\n",i,offset[0],offset[1],offset[2]);
      for(j = 0; j < fr->periodic_p_num[i]; j++){
	draw_particle_inside(gb,fr->periodic_p[i][j],cb,fr,
			     KS_FALSE,KS_GL_ATOM_TRANS_OFF,
			     (cb->particle_size == CV_PARTICLE_SIZE_SMALL || 
			      fr->flags&CV_FRAME_CDV) ? KS_TRUE:KS_FALSE,NULL,
			     0,offset,fr->side);
      }
    }
  }
  /*
  ks_print_atom_list();
  ks_exit(EXIT_FAILURE);
  */
  /*
  {
    int i,j,c;
    double x,y,z,sep;
    x = 0; y = 0; z = 0; sep = 2.0;
    GLfloat color[4];
    for(i = 0,c = 0; i < KS_ATOM_LIST_NUM; i++){
      if(ks_atomic[i].radius < 10){
	for(j = 0; j < 3; j++){
	  color[j] = ks_atomic[i].color[j];
	}
	color[3] = 1.0;
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	glPushMatrix();
	glTranslated(0,sep*(c%10),sep*(int)(c/10));
	glutSolidSphere(0.5,10,5);
	glPopMatrix();
	glDisable(GL_LIGHTING);
	ks_gl_base_foreground_color(gb);
	glRasterPos3d(0,sep*(c%10),sep*(int)(c/10)-1.0);
	ks_draw_glut_text(gb,ks_atomic[i].symbol,KS_GL_FONT_8x13,0.5,1);
	glEnable(GL_LIGHTING);
	c++;
      }
    }
  }
  */
  /*
  {
    int i;
    float len = 1.4;
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
      glPushMatrix();
      glTranslated(0,(i-KS_GL_ATOM_COLOR_NUM*.5)*len,0);
      ks_draw_gl_atom(cb->frame[cb->current_frame].particle->atom->gl_atom,
		    i,
		    KS_GL_ATOM_SIZE_NORMAL,
		    KS_GL_ATOM_BRIGHT_NORMAL,
		    KS_GL_ATOM_TRANS_OFF);
      glPopMatrix();
    }
  }
  return ;
  */
}
double cv_get_system_size(CV_BASE *cb)
{
  return cb->frame[cb->current_frame].system_size;
}
static void reuse_cv_particle(CV_REUSE *reuse, CV_PARTICLE *pl)
{
  CV_PARTICLE *p;
  //  CV_BOND *bond;
  //  int bond_num, total_bond = 0;
  for(p = pl ;p != NULL; p = p->next){
    /*
    printf("%d %s %f %f %f:",p->label,p->atom->name,p->cd[0],p->cd[1],p->cd[2]);
    for(bond = p->bond, bond_num = 0; bond != NULL; bond = bond->next, bond_num++){
      printf(" %f",bond->len);
    }
    printf(" %d\n",bond_num);
    total_bond += bond_num;
    */
    if(p->property != NULL){
      if(p->property->alternative != NULL){
	reuse->particle = cat_cv_particle(reuse->particle,p->property->alternative);
	p->property->alternative = NULL;
      }
      reuse->property = cat_cv_particle_property(reuse->property,p->property);
      p->property = NULL;
    }
    if(p->bond != NULL){
      reuse->bond = cat_cv_bond(reuse->bond,p->bond);
      p->bond = NULL;
    }
  }
  /*  printf("total_bond %d\n",total_bond);*/
  reuse->particle = cat_cv_particle(reuse->particle,pl);
}
static void __KS_USED__ print_reuse(CV_REUSE *reuse)
{
  //  CV_PARTICLE *p;
  //  CV_PARTICLE_PROPERTY *pp;
  //  CV_BOND *pb;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  //  CV_RESIDUE *pr;

  printf("print reuse\n");
  /*
  for(p = reuse->particle; p != NULL; p = p->next){
    printf("%d %s %f %f %f\n",p->label,p->atom->name,p->cd[0],p->cd[1],p->cd[2]);
  }
  for(pp = reuse->property; pp != NULL; pp = pp->next){
    printf("%s\n",pp->name);
  }
  for(pb = reuse->bond; pb != NULL; pb = pb->next){
    printf("%f\n",pb->len);
  }
  */
  for(pa = reuse->aggregate; pa != NULL; pa = pa->next){
    printf("agg %d\n",pa->label);
  }
  for(pc = reuse->chain; pc != NULL; pc = pc->next){
    printf("chain %d %s\n",pc->label,pc->name);
  }
  /*
  for(pr = reuse->residue; pr != NULL; pr = pr->next){
    printf("residue %d %s\n",pr->label,pr->name);
  }
  */
}
static void reuse_cv_aggregate(CV_REUSE *reuse, CV_AGGREGATE *agg)
{
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  //  CV_RESIDUE *pr;

  for(pa = agg; pa != NULL; pa = pa->next){
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      reuse->residue = cat_cv_residue(pc->residue,reuse->residue);
      pc->residue = NULL;
    }
    reuse->chain = cat_cv_chain(pa->chain,reuse->chain);
    pa->chain = NULL;
  }
  reuse->aggregate = cat_cv_aggregate(agg,reuse->aggregate);
}
static void clear_frame(CV_FRAME *fr, CV_FIXED_SURFACE_INFO sur_fixed)
{
  reuse_cv_particle(fr->reuse,fr->particle);
  fr->particle = NULL;
  reuse_cv_aggregate(fr->reuse,fr->aggregate);

  if(fr->cdv_bond != NULL){
    fr->reuse->cdv_bond = cat_cv_cdv_bond_list(fr->reuse->cdv_bond,fr->cdv_bond);
    fr->cdv_bond = NULL;
  }
  if(fr->cdv_face != NULL){
    fr->reuse->cdv_face = cat_cv_cdv_face_list(fr->reuse->cdv_face,fr->cdv_face);
    fr->cdv_face = NULL;
  }
  if(fr->sur != NULL && fr->sur != sur_fixed.sur){
    ks_assert(fr->reuse->sur == NULL);
    fr->reuse->sur = fr->sur;
    fr->sur = NULL;
  }
  if(fr->color_map != NULL && fr->color_map != sur_fixed.color_map){
    ks_assert(fr->reuse->color_map == NULL);
    fr->reuse->color_map = fr->color_map;
    fr->color_map = NULL;
  }

  /*  print_reuse(fr->reuse);*/
  fr->aggregate = NULL;
  fr->particle_label_max = 0;
  fr->particle_file_label_max = 0;
  fr->residue_label_max = 0;
  fr->chain_label_max = 0;
  fr->aggregate_label_max = 0;

  fr->flags &= ~CV_FRAME_READ;
  /*  printf("%d\n",ks_output_memory_infomation(stdout,0));*/
}
static void set_selected_label(KS_INT_LIST *selected_label, 
					CV_PARTICLE **hash, int hash_size, BOOL select)
{
  KS_INT_LIST *il;
  CV_PARTICLE *p;
  for(il = selected_label; il != NULL; il = il->next){
    p = lookup_cv_particle_hash_label(hash,hash_size, il->value);
    if(p != NULL){
      if(select == KS_TRUE){
	set_selected_for_particle(p);
      } else {
	unset_selected_for_particle(p);
      }
    }
  }
}
static void set_selected_residue_label(KS_INT_LIST *selected_residue_label,
				       CV_RESIDUE **hash, int hash_size, BOOL select)
{
  KS_INT_LIST *il;
  CV_RESIDUE *pr;
  for(il = selected_residue_label; il != NULL; il = il->next){
    pr = lookup_cv_residue_hash_label(hash,hash_size, il->value);
    if(pr != NULL){
      set_select_residue(pr,select);
    }
  }
}
static void set_selected_label_all(KS_INT_LIST *selected_label, 
				   CV_PARTICLE **particle_hash, int particle_hash_size,
				   KS_INT_LIST *selected_residue_label,
				   CV_RESIDUE **residue_hash, int residue_hash_size, BOOL select)
{
  set_selected_label(selected_label,particle_hash,particle_hash_size,select);
  set_selected_residue_label(selected_residue_label,residue_hash,residue_hash_size,select);
}
static BOOL move_frame(KS_GL_BASE *gb, CV_BASE *cb, int move_frame)
{
  KS_INT_LIST *il;
  KS_LABEL_LIST *ll;
  CV_PARTICLE *p;
  int i;
  int selected_file_label[CV_SELECT_PARTICLE_MAX+1];
  for(i = 0; i < CV_SELECT_PARTICLE_MAX+1; ++i){
    selected_file_label[i] = -1;
  }
  /*
  printf("%d %s (%d %d %d) -> %d %s (%d %d %d)\n",
	 cb->current_frame,cb->frame[cb->current_frame].file_name,
	 cb->frame[cb->current_frame].particle_file_label_max,
	 cb->frame[cb->current_frame].particle_label_max,
	 cb->frame[cb->current_frame].residue_label_max,
	 move_frame,cb->frame[move_frame].file_name,
	 cb->frame[move_frame].particle_file_label_max,
	 cb->frame[move_frame].particle_label_max,
	 cb->frame[move_frame].residue_label_max);
  */
  set_selected_label_all(cb->selected_label,cb->particle_hash,cb->particle_hash_size,
			 cb->selected_residue_label,cb->residue_hash,cb->residue_hash_size,
			 KS_FALSE);
  i = 0;
  for(il = cb->selected_label; il != NULL; il = il->next){
    p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size, il->value);
    selected_file_label[i++] = p->file_label; // keep file label
  }

  if(!(cb->frame[cb->current_frame].flags&CV_FRAME_REFERENCE) && cb->no_mem == KS_TRUE){
    clear_frame(&cb->frame[cb->current_frame],cb->sur_fixed);
  }
  clear_hash(&cb->frame[cb->current_frame],
	     cb->particle_hash,cb->particle_hash_size,
	     cb->particle_label_hash,cb->particle_label_hash_size,
	     cb->residue_hash,cb->residue_hash_size,
	     cb->residue_label_hash,cb->residue_label_hash_size);

  cb->current_frame = move_frame;
  if(!(cb->frame[cb->current_frame].flags&CV_FRAME_READ)){
    if(init_frame(&cb->frame[cb->current_frame], &cb->fv_base,cb)
       == KS_FALSE){
      return KS_FALSE;
    }
  }

  if(cb->frame[cb->current_frame].atype_mode != cb->atype_mode){
    cb->frame[cb->current_frame].atype_mode = cb->atype_mode;
    set_atype(cb,&cb->frame[cb->current_frame]);
  }

  set_hash(&cb->frame[cb->current_frame],&cb->reuse_base.label,
	   &cb->particle_hash,&cb->particle_hash_size,
	   &cb->particle_label_hash,&cb->particle_label_hash_size,
	   &cb->residue_hash,&cb->residue_hash_size,
	   &cb->residue_label_hash,&cb->residue_label_hash_size);

  for(i = 0,il = cb->selected_label; selected_file_label[i] != -1; i++){
    ll = lookup_label_list(cb->particle_label_hash,
			   cb->particle_label_hash_size,selected_file_label[i]);
    if(ll == NULL){
      int value;
      value = il->value;
      il=il->next;
      cb->selected_label = ks_del_int_list(cb->selected_label,value);
    } else {
      il->value = ll->value; // convert from keeped file label to particle label
      il=il->next;
    }
  }
  set_selected_label_all(cb->selected_label,cb->particle_hash,cb->particle_hash_size,
			 cb->selected_residue_label,cb->residue_hash,cb->residue_hash_size,
			 KS_TRUE);
  if(cb->frame[cb->current_frame].cdv_bond != NULL){
    make_cdv_bond(cb->frame[cb->current_frame].cdv_bond,cb->bond_type,
		  cb->particle_hash,cb->particle_hash_size,
		  cb->particle_label_hash,cb->particle_label_hash_size,
		  &cb->frame[cb->current_frame].pdb_user_bond,
		  &cb->reuse_base.bond);
    free_cv_cdv_bond_list(cb->frame[cb->current_frame].cdv_bond);
    cb->frame[cb->current_frame].cdv_bond = NULL;
  }
  if(cb->frame[cb->current_frame].cdv_face != NULL){
    make_cdv_face(cb->frame[cb->current_frame].cdv_face,cb->face_type,
		  &cb->frame[cb->current_frame].sur,
		  cb->particle_hash,cb->particle_hash_size,
		  cb->particle_label_hash,cb->particle_label_hash_size,
		  &cb->reuse_base.sur);
  }
  if(cb->periodic_mode == KS_TRUE){
    set_periodic_frame(cb,&cb->frame[cb->current_frame]);
  }

#ifdef USE_GLSL
  update_shader_sphere_draw_info(cb,&cb->frame[cb->current_frame]);
  update_shader_rod_draw_info(cb,&cb->frame[cb->current_frame]);
#endif

  return KS_TRUE;
}
static void make_movie_from_bmp(CV_BASE *cb)
{
  int i,keta;
  char input_file_name[CV_NAME_MAX];
  char remove_file_name[CV_NAME_MAX];
  char command_in[CV_NAME_MAX];
  char command_out[CV_NAME_MAX];
  char *cp;
  BOOL remove_file = KS_TRUE;
  cp = getenv(CV_ENV_KEY_MOVIE_COMMAND);
  if(cp == NULL){
    return;
  }
  keta = log10(cb->frame_num)+1; // keta setting is the same as that in set_sequential_bmp_file_name
  snprintf(input_file_name,sizeof(input_file_name),  // make input file format for ffmpeg
	   "%s_%%0%dd.bmp",cb->file_name_base,keta);
  if(ks_replace_string(cp,sizeof(command_in),command_in,"%IN%",input_file_name) == KS_FALSE){
    draw_error(cb,"ERROR: can not replace %IN% of CDVIEW_MAKE_MOVIE_COMMAND");
    return;
  }
  if(ks_replace_string(command_in,sizeof(command_out),command_out,"%OUT%",cb->file_name_base)
     == KS_FALSE){
    draw_error(cb,"ERROR: can not replace %OUT% of CDVIEW_MAKE_MOVIE_COMMAND");
    return;
  }
  printf("making movie '%s\n",command_out); fflush(stdout);
  if(system(command_out) == -1){
    draw_error(cb,"ERROR: CDVIEW_MAKE_MOVIE_COMMAND failed");
  } else {
    draw_error(cb,"CDVIEW_MAKE_MOVIE_COMMAND succeeded");
  }

  cp = getenv(CV_ENV_KEY_REMOVE_BMP_MOVIE);
  if(cp != NULL){
    if(strcmp(cp,"1") == 0){
      for(i = 0; i < cb->output_bmp_file_cnt; i++){
	snprintf(remove_file_name,sizeof(remove_file_name),input_file_name,i+1);
	printf("remove %s\n",remove_file_name); fflush(stdout);
	if(remove(remove_file_name) != 0){
	  draw_error(cb,"ERROR: remove file failed %s",remove_file_name);
	  remove_file = KS_FALSE;
	}
      }
      if(remove_file == KS_TRUE){
	draw_error(cb,"CDVIEW_REMOVE_BMP_AFTER_MAKING_MOVIE succeeded");
      }
    }
  }

  fflush(stdout);
}
static void make_output_file_name(char *ext, char *in, char *out, size_t out_size)
{
  ks_change_file_ext(in,ext,out,out_size);
}
static void first_frame(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  for(i = 0; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE) break;
  }
  move_frame(gb,cb,i);
}
static void last_frame(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  for(i = cb->frame_num-1; i >= 0; i--){
    if(cb->frame[i].flags&CV_FRAME_ENABLE) break;
  }
  move_frame(gb,cb,i);
}
static void increment_frame(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
}
static void increment_frame_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  cb->idle = NULL;
}
static void set_increment_frame_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  ((CV_BASE*)vp)->idle = increment_frame_idle;
}
static void set_sequential_bmp_file_name(CV_BASE *cb, char* file_name, size_t file_name_size,
					 char* ext, int add)
{
  if(cb->order_bmp_counter > 0){
    int keta = log10(cb->frame_num)+1;
    snprintf(file_name,file_name_size,"%s_%0*d.%s",
	     cb->file_name_base,keta,cb->order_bmp_counter,ext);
    cb->order_bmp_counter += add;
  } else {
    if(cb->frame[cb->current_frame].file_name[0] != '\0'){
      make_output_file_name(ext,cb->frame[cb->current_frame].file_name,file_name,
			    file_name_size);
    } else if(cb->frame[cb->current_frame].file_name_fdv[0] != '\0'){
      make_output_file_name(ext,cb->frame[cb->current_frame].file_name_fdv,file_name,
			    file_name_size);
    } else {
      make_output_file_name(ext,"cdv_output",file_name,file_name_size);
    }
  }
}
static void increment_frame_bmp_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  char file_name[256];
  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      ks_draw_gl(gb);
      ks_draw_gl(gb);
      if(cb->output_file_mode == CV_OUTPUT_PICTURE_BMP){
	set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"bmp",1);
	cb->output_bmp_file_cnt++;
	printf("-> %s\n",file_name);
	fflush(stdout);
	ks_save_gl_bmp(file_name,KS_FALSE,ks_get_gl_viewport_3d(gb));
      } else if(cb->output_file_mode == CV_OUTPUT_PICTURE_POV || 
		cb->output_file_mode == CV_OUTPUT_PICTURE_POV_SHADOW){
	set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"pov",1);
	output_pov_file(gb,file_name,(void*)cb);
      }
      return;
    }
  }
  if(cb->order_bmp_counter > 0){ // set_increment_frame_order_bmp_idle is called to make movie
    make_movie_from_bmp(cb);
  }
  cb->idle = NULL;
  cb->order_bmp_counter = -1;
  cb->output_bmp_file_cnt = 0;
}
static void set_increment_frame_bmp_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  char file_name[256];
  cb = (CV_BASE*)vp;
  cb->idle = increment_frame_bmp_idle;
  if(cb->output_file_mode == CV_OUTPUT_PICTURE_BMP){
    ks_draw_gl(gb);
    ks_draw_gl(gb);
    set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"bmp",1);
    cb->output_bmp_file_cnt++;
    printf("-> %s\n",file_name);
    fflush(stdout);
    ks_save_gl_bmp(file_name,KS_FALSE,ks_get_gl_viewport_3d(gb));
  } else if(cb->output_file_mode == CV_OUTPUT_PICTURE_POV || 
	    cb->output_file_mode == CV_OUTPUT_PICTURE_POV_SHADOW){
    set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"pov",1);
    output_pov_file(gb,file_name,(void*)cb);
  }
}
static void set_increment_frame_order_bmp_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  make_common_file_name(cb,cb->file_name_base,sizeof(cb->file_name_base),"");
  if(cb->file_name_base[0] == '\0'){
    make_common_file_name_fdv(cb,cb->file_name_base,sizeof(cb->file_name_base),"");
  }
  cb->order_bmp_counter = 1;
  set_increment_frame_bmp_idle(gb,x,y,(void*)cb);
}
static void decrement_frame(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  for(i = cb->current_frame-1; i >= 0; i--){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
}
static void decrement_frame_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  for(i = cb->current_frame-1; i >= 0; i--){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  cb->idle = NULL;
}
static void set_decrement_frame_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  ((CV_BASE*)vp)->idle = decrement_frame_idle;
}
static void decrement_frame_bmp_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  char file_name[256];
  for(i = cb->current_frame-1; i >= 0; i--){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      ks_draw_gl(gb);
      ks_draw_gl(gb);
      if(cb->output_file_mode == CV_OUTPUT_PICTURE_BMP){
	set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"bmp",-1);
	cb->output_bmp_file_cnt++;
	printf("-> %s\n",file_name);
	fflush(stdout);
	ks_save_gl_bmp(file_name,KS_FALSE,ks_get_gl_viewport_3d(gb));
      } else if(cb->output_file_mode == CV_OUTPUT_PICTURE_POV || 
		cb->output_file_mode == CV_OUTPUT_PICTURE_POV_SHADOW){
	set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"pov",-1);
	output_pov_file(gb,file_name,(void*)cb);
      }
      return;
    }
  }
  if(cb->order_bmp_counter > 0){ // set_decrement_frame_order_bmp_idle is called to make movie
    make_movie_from_bmp(cb);
  }
  cb->idle = NULL;
  cb->order_bmp_counter = -1;
  cb->output_bmp_file_cnt = 0;
}
static void set_decrement_frame_bmp_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  char file_name[256];
  cb = (CV_BASE*)vp;
  cb->idle = decrement_frame_bmp_idle;
  if(cb->output_file_mode == CV_OUTPUT_PICTURE_BMP){
    ks_draw_gl(gb);
    ks_draw_gl(gb);
    set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"bmp",-1);
    cb->output_bmp_file_cnt++;
    printf("-> %s\n",file_name);
    fflush(stdout);
    ks_save_gl_bmp(file_name,KS_FALSE,ks_get_gl_viewport_3d(gb));
  } else if(cb->output_file_mode == CV_OUTPUT_PICTURE_POV || 
	    cb->output_file_mode == CV_OUTPUT_PICTURE_POV_SHADOW){
    set_sequential_bmp_file_name(cb,file_name,sizeof(file_name),"pov",-1);
    output_pov_file(gb,file_name,(void*)cb);
  }
}
static void set_decrement_frame_order_bmp_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  make_common_file_name(cb,cb->file_name_base,sizeof(cb->file_name_base),"");
  if(cb->file_name_base[0] == '\0'){
    make_common_file_name_fdv(cb,cb->file_name_base,sizeof(cb->file_name_base),"");
  }
  cb->order_bmp_counter = cb->current_frame+1;
  set_decrement_frame_bmp_idle(gb,x,y,(void*)cb);
}
static void reset_idle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  ((CV_BASE*)vp)->idle = NULL;
}
/*
static void add_move(CV_PARTICLE *pl)
{
  CV_PARTICLE *p;
  int i;
  for(p = pl; p != NULL; p = p->next){
    for(i = 0; i < 3; i++) p->cd[i] += p->move[i];
    for(i = 0; i < 3; i++) p->move[i] = 0;
  }
}
*/
static void add_move(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  for(p = fr->particle; p != NULL; p = p->next){
    for(i = 0; i < 3; i++) p->cd[i] += p->move[i];
    for(i = 0; i < 3; i++) p->move[i] = 0;
  }
}
static void move_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_PARTICLE *p;
  CV_FRAME *fr;
  char *move_on = {"move mode on"};
  char *move_off = {"move mode off"};

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  if(cb->move_mode == KS_FALSE){
    cb->move_mode = KS_TRUE;
    if(ks_count_int_list(cb->selected_label) == 1){
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
					cb->selected_label->value);
      ks_set_gl_move_target(gb,cb->move_add,p->cd,1.0);
      ks_set_gl_rotation_center(gb,p->cd);
    } else {
      ks_set_gl_move_target(gb,NULL,NULL,1.0);
    }
    cb->telop = ks_del_text_list(cb->telop,move_off);
    cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(move_on,2));
  } else {
    cb->move_mode = KS_FALSE;
    ks_set_gl_move_target(gb,NULL,NULL,1.0);
    cb->telop = ks_del_text_list(cb->telop,move_on);
    cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(move_off,2));
    add_move(gb,0,0,vp);
    ks_set_gl_rotation_center(gb,fr->center);
    if(cb->periodic_mode == KS_TRUE){
      set_periodic_frame(cb,&cb->frame[cb->current_frame]);
    }
  }
}
static void undo_move(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_PARTICLE *p;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  for(p = fr->particle; p != NULL; p = p->next){
    for(i = 0; i < 3; i++) p->move[i] = 0;
  }
  /*
  if(cb->move_mode == KS_TRUE){
    for(p = fr->particle; p != NULL; p = p->next){
      for(i = 0; i < 3; i++) p->move[i] = 0;
    }
  }
  */
}
static void print_residue(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc;
  CV_RESIDUE *pr;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  for(pa = fr->aggregate; pa != NULL; pa = pa->next){
    printf("agg %d %c\n",pa->label,pa->flags&CV_AGGREGATE_SELECTED? 'o':'x');
    for(pc = pa->chain; pc != NULL; pc = pc->next){
      printf("  chain %d %s %c\n",pc->label,pc->name,pc->flags&CV_CHAIN_SELECTED? 'o':'x');
      for(pr = pc->residue; pr != NULL; pr = pr->next){
	printf("    %d %d %s %c %s %s\n",pr->label,pr->file_label,pr->name,
	       pr->flags&CV_RESIDUE_SELECTED? 'o':'x',
	       pr->flags&CV_RESIDUE_START ? "START":
	       pr->flags&CV_RESIDUE_END ? "END":
	       pr->flags&CV_RESIDUE_NORMAL ? "NORMAL":"NON",
	       pr->flags&CV_RESIDUE_AMINO ? "AMINO":"NON"
	       );
      }
    }
  }
}
static void print_particle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_PARTICLE *p;
  CV_BASE *cb;
  CV_FRAME *fr;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  /*  for(p = fr->particle_end; p != NULL; p = p->prev){*/
  for(p = fr->particle; p != NULL; p = p->next){
    printf("%3d %3d %s %s %f %f %f %p %p %p\n"
	   ,p->label,p->file_label,p->atom->name,p->property->name
	   ,p->cd[0],p->cd[1],p->cd[2]
	   ,p->prev,p,p->next
	   );
  }
}
static void find_particle(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_PARTICLE *p;
  char *cp, c0[256];
  KS_LABEL_LIST *ll;

  cb = (CV_BASE*)vp;
  cp = text;
  while((cp = ks_get_str(", ",cp,c0,sizeof(c0))) != NULL){
    /*    printf("c0 %s\n",c0);*/
    if((ll = lookup_label_list(cb->particle_label_hash,
			       cb->particle_label_hash_size,atoi(c0))) == NULL){
      draw_error(cb,"ERROR: particle of label %d is not found",atoi(c0));
      return;
    }
    for(;ll != NULL;ll = ll->next){
      /*      printf("label %d\n",ll->value);*/
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,ll->value);
      /*      printf("%d\n",p->label);*/
      if(p != NULL){
	set_selected_for_particle(p);
	cb->selected_label=ks_add_int_list(cb->selected_label,ks_new_int_list(ll->value));
      }
      if(!(ll->flags&CV_LABEL_DUPLICATE)) break;
    }
  }
#ifdef USE_GLSL
  // update draw_info to change emissions of selected particles
  ks_update_gl_shader_sphere_draw_info_share(cb->sb->shader[SHADER_SPHERE],0,
					     CV_SPHERE_DETAIL_CNT,cb->sb->sphere_draw_info);
#endif
}
static void set_find_particle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  ks_set_gl_input_key(gb,"find particle > ",NULL,((CV_BASE*)vp)->input_key_font,
		      find_particle,KS_FALSE,cb);
}
static void find_residue(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_RESIDUE *p;
  char *cp, c0[256];
  KS_LABEL_LIST *ll;

  cb = (CV_BASE*)vp;
  cp = text;
  while((cp = ks_get_str(", ",cp,c0,sizeof(c0))) != NULL){
    /*    printf("c0 %s\n",c0);*/
    if((ll=lookup_label_list(cb->residue_label_hash,cb->residue_label_hash_size,atoi(c0)))==NULL){
      draw_error(cb,"ERROR: residue of label %d is not found",atoi(c0));
      return;
    }
    for(;ll != NULL;ll = ll->next){
      /*      printf("label %d\n",ll->value);*/
      p = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,ll->value);
      if(p != NULL){
	set_select_residue(p,KS_TRUE);
      }
      if(!(ll->flags&CV_LABEL_DUPLICATE)) break;
    }
  }
}
static void set_find_residue(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  ks_set_gl_input_key(gb,"find residue > ",NULL,((CV_BASE*)vp)->input_key_font,
		      find_residue,KS_FALSE,cb);
}
BOOL pos_hit(double pos[3], double x, double y, double z)
{
  if(pos[0] > x-.1 && pos[0] < x+.1 &&
     pos[1] > y-.1 && pos[1] < y+.1 &&
     pos[2] > z-.1 && pos[2] < z+.1){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
static BOOL is_selected_contour(FV_CONTOUR *c0, FV_CONTOUR *c1, FV_CONTOUR *c2)
{
  int i = 0;
  if(c0->flags&FV_CONTOUR_SELECTED) i++;
  if(c1->flags&FV_CONTOUR_SELECTED) i++;
  if(c2->flags&FV_CONTOUR_SELECTED) i++;
  if(i >= 2){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
static BOOL output_face_file_header(char *file_name, char *file_name_face, 
				    size_t file_name_face_size, FILE *fp)
{
  ks_remove_file_ext(file_name,file_name_face,file_name_face_size);
  if(strlen(file_name_face)+4 > file_name_face_size){
    return KS_FALSE;
  }
  strcat(file_name_face,".fac");
  fprintf(fp,"' face_file=%s face0_c=(1.0,0,0) face1_c=(1.0,1.0,0)\n",file_name_face);
  return KS_TRUE;
}
static BOOL output_contour_vertex(CV_BASE *cb, CV_FRAME *fr, FILE *fp)
{
  int i,j,k,m,n;
  FV_CONTOUR_SIZE cs;
  FV_TARGET tg;
  FV_CONTOUR_FAN ***fcf;
  FV_CONTOUR_FAN *fa;
  unsigned int label;
  int ii[3],ix,iy,iz;
  double r = 3.4,dd[3];
  //  BOOL hit;
  CV_PARTICLE_POS_HASH *ph;

  label = fr->particle->label+1;
  cs = fr->fv->contour_size;
  fcf = fr->fv->contour_fan;
  tg = cb->fv_target;

  if((ph = allocate_cv_particle_pos_hash(fr,r)) == NULL){
    draw_error(cb,"ERROR: memory error");
    return KS_FALSE;
  }
  printf("%d %d %d %f %f %f\n",ph->n[0],ph->n[1],ph->n[2],ph->len[0],ph->len[1],ph->len[2]);

#if 0
  for(i = 0; i < fr->field_size.n[0]; i++){
    for(j = 0; j < fr->field_size.n[1]; j++){
      for(k = 0; k < fr->field_size.n[2]; k++){
	if(fr->field[i][j][k] != NULL){
	  for(l = 0; l < 3; l++){
	    for(o = 0; o < 2; o++){
	      if(fr->field[i][j][k]->c[l][o] != NULL){
		hit = pos_hit(fr->field[i][j][k]->c[l][o]->cd,164.8,144.8,109.9);
		if(hit == KS_TRUE){
		  printf("%f %f %f\n"
			 ,fr->field[i][j][k]->c[l][o]->cd[0]
			 ,fr->field[i][j][k]->c[l][o]->cd[1]
			 ,fr->field[i][j][k]->c[l][o]->cd[2]);
		}
		calc_pos_hash(ph,fr->field[i][j][k]->c[l][o]->cd,ii);
		for(ix = -1; ix < 2; ix++){
		  for(iy = -1; iy < 2; iy++){
		    for(iz = -1; iz < 2; iz++){
		      if(ii[0]+ix >= 0 && ii[0]+ix < ph->n[0] && 
			 ii[1]+iy >= 0 && ii[1]+iy < ph->n[1] && 
			 ii[2]+iz >= 0 && ii[2]+iz < ph->n[2]){
			if(hit == KS_TRUE){
			  printf("(%d %d %d)(%d %d %d)(%d %d %d) %d\n"
				 ,ii[0],ii[1],ii[2]
				 ,ix,iy,iz,ii[0]+ix,ii[1]+iy,ii[2]+iz
				 ,ph->particle_num[ii[0]+ix][ii[1]+iy][ii[2]+iz]);
			}
			for(m = 0; m < ph->particle_num[ii[0]+ix][ii[1]+iy][ii[2]+iz]; m++){
			  if(hit == KS_TRUE){
			    printf("  %d %f %f %f\n",m
				   ,ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[0]
				   ,ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[1]
				   ,ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[2]);
			  }
			  if(ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->atom->atype == 4){
			    for(n = 0; n < 3; n++){
			      dd[n] = (fr->field[i][j][k]->c[l][o]->cd[n] - 
				       ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[n]);
			    }
			    if(dd[0]*dd[0]+dd[1]*dd[1]+dd[2]*dd[2] < r*r){
			      fr->field[i][j][k]->c[l][o]->flags |= FV_CONTOUR_SELECTED;
			    }
			  }
			}
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
#endif

  for(i = 0; i < cs.num; i++){
    if(tg.level[i] == 1){
      for(fa = fcf[i][tg.val],k = 0; fa != NULL; fa = fa->next,k++){
	for(j = 0; j < fa->num; j++){
	  /*
	    hit = pos_hit(fa->pcs[j]->cd,164.8,144.8,109.9);
	    if(hit == KS_TRUE){
	    printf("%f %f %f\n",fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2]);
	    }
	  */
	  calc_pos_hash(ph,fa->pcs[j]->cd,ii);
	  for(ix = -1; ix < 2; ix++){
	    for(iy = -1; iy < 2; iy++){
	      for(iz = -1; iz < 2; iz++){
		if(ii[0]+ix >= 0 && ii[0]+ix < ph->n[0] && 
		   ii[1]+iy >= 0 && ii[1]+iy < ph->n[1] && 
		   ii[2]+iz >= 0 && ii[2]+iz < ph->n[2]){
		  /*
		    if(hit == KS_TRUE){
		    printf("(%d %d %d)(%d %d %d)(%d %d %d) %d\n"
		    ,ii[0],ii[1],ii[2]
		    ,ix,iy,iz,ii[0]+ix,ii[1]+iy,ii[2]+iz
		    ,ph->particle_num[ii[0]+ix][ii[1]+iy][ii[2]+iz]);
		    }
		  */
		  for(m = 0; m < ph->particle_num[ii[0]+ix][ii[1]+iy][ii[2]+iz]; m++){
		    /*
		      if(hit == KS_TRUE){
		      printf("  %d %f %f %f\n",m
		      ,ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[0]
		      ,ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[1]
		      ,ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[2]);
		      }
		    */
		    if(ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->atom->atype == 10){
		      for(n = 0; n < 3; n++){
			dd[n] = (fa->pcs[j]->cd[n] - 
				 ph->particle[ii[0]+ix][ii[1]+iy][ii[2]+iz][m]->cd[n]);
		      }
		      if(dd[0]*dd[0]+dd[1]*dd[1]+dd[2]*dd[2] < r*r){
			fa->pcs[j]->flags |= FV_CONTOUR_SELECTED;
		      }
		    }
		  }
		}
	      }
	    }
	  }

	  glVertex3dv(fa->pcs[j]->cd);
	  if(fa->pcs[j]->flags&FV_CONTOUR_SELECTED){
	    fprintf(fp,"%d 2 %f %f %f 9 9\n",
		    label++,fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2]);
	  } else {
	    fprintf(fp,"%d 3 %f %f %f 9 9\n",
		    label++,fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2]);
	  }
	}
      }
    }
  }
  free_cv_particle_pos_hash(ph);
  return KS_FALSE;
}
static void output_contour_face(CV_BASE *cb, CV_FRAME *fr, char *file_name_face)
{
  int i,j,k;
  FILE *fp;
  FV_CONTOUR_SIZE cs;
  FV_TARGET tg;
  FV_CONTOUR_FAN ***fcf;
  FV_CONTOUR_FAN *fa;
  unsigned int label,label_center;
  enum {
    GAS,
    SOLID,
    AREA_NUM
  };
  double surface_area[AREA_NUM];

  if(fr->particle){
    label = fr->particle->label+1;
  } else {
    label = 0;
  }
  cs = fr->fv->contour_size;
  fcf = fr->fv->contour_fan;
  tg = cb->fv_target;

  for(i = 0; i < AREA_NUM; i++){
    surface_area[i] = 0;
  }

  if(file_name_face){
    if((fp = fopen(file_name_face,"wt")) == NULL){
      draw_error(cb,"ERROR: can not save %s",file_name_face);
      return;
    }
  }
  for(i = 0; i < cs.num; i++){
    if(tg.level[i] == 1){
      for(fa = fcf[i][tg.val],k = 0; fa != NULL; fa = fa->next,k++){
	label_center = label;
	label += 2;
	for(j = 2; j < fa->num; j++){
	  if(is_selected_contour(fa->pcs[0],fa->pcs[j-1],fa->pcs[j]) == KS_TRUE){
	    if(file_name_face){
	      fprintf(fp,"%d %d %d\n",label_center,label-1,label);
	    }
	    surface_area[SOLID] += ks_calc_triangle_area(fa->pcs[0]->cd,
							 fa->pcs[j-1]->cd,
							 fa->pcs[j]->cd);
	  } else {
	    if(file_name_face){
	      fprintf(fp,"%d %d %d 1\n",label_center,label-1,label);
	    }
	    surface_area[GAS] += ks_calc_triangle_area(fa->pcs[0]->cd,
						       fa->pcs[j-1]->cd,
						       fa->pcs[j]->cd);
	  }
	  label++;
	}
	if(fa->num > 3){
	  if(is_selected_contour(fa->pcs[0],fa->pcs[1],fa->pcs[fa->num-1]) == KS_TRUE){
	    if(file_name_face){
	      fprintf(fp,"%d %d %d\n",label_center,label_center+1,label-1);
	    }
	    surface_area[SOLID] += ks_calc_triangle_area(fa->pcs[0]->cd,
							 fa->pcs[1]->cd,
							 fa->pcs[fa->num-1]->cd);
	  } else {
	    if(file_name_face){
	      fprintf(fp,"%d %d %d 1\n",label_center,label_center+1,label-1);
	    }
	    surface_area[GAS] += ks_calc_triangle_area(fa->pcs[0]->cd,
						       fa->pcs[1]->cd,
						       fa->pcs[fa->num-1]->cd);
	  }
	}
      }
    }
  }
  printf("surface area %f + %f = %f %f:%f\n",
	 surface_area[GAS],surface_area[SOLID],
	 surface_area[GAS]+surface_area[SOLID],
	 surface_area[GAS]/(surface_area[GAS]+surface_area[SOLID]),
	 surface_area[SOLID]/(surface_area[GAS]+surface_area[SOLID]));
  fflush(stdout);
  if(file_name_face){
    fclose(fp);
  }
}
static void calc_contour_area(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i,j,k;
  CV_BASE *cb;
  CV_FRAME *fr;
  FV_CONTOUR_SIZE cs;
  FV_TARGET tg;
  FV_CONTOUR_FAN ***fcf;
  FV_CONTOUR_FAN *fa;
  enum {
    ALL,
    BOTTOM,
    AREA_NUM
  };
  double surface_area[AREA_NUM];
  //  double bottom_limit = 6.4+2.87;
  double bottom_limit = -1000;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  cs = fr->fv->contour_size;
  fcf = fr->fv->contour_fan;
  tg = cb->fv_target;

  for(i = 0; i < AREA_NUM; i++){
    surface_area[i] = 0;
  }

  for(i = 0; i < cs.num; i++){
    if(tg.level[i] == 1){
      for(fa = fcf[i][tg.val],k = 0; fa != NULL; fa = fa->next,k++){
	for(j = 2; j < fa->num; j++){
	  surface_area[ALL] += ks_calc_triangle_area(fa->pcs[0]->cd,
						     fa->pcs[j-1]->cd,
						     fa->pcs[j]->cd);
	  if(fa->pcs[0]->cd[2] < bottom_limit &&
	     fa->pcs[j-1]->cd[2] < bottom_limit &&
	     fa->pcs[j]->cd[2] < bottom_limit){
	    surface_area[BOTTOM] += ks_calc_triangle_area(fa->pcs[0]->cd,
							  fa->pcs[j-1]->cd,
							  fa->pcs[j]->cd);
	  }
	}
	if(fa->num > 3){
	  surface_area[ALL] += ks_calc_triangle_area(fa->pcs[0]->cd,
						     fa->pcs[1]->cd,
						     fa->pcs[fa->num-1]->cd);
	  if(fa->pcs[0]->cd[2] < bottom_limit &&
	     fa->pcs[1]->cd[2] < bottom_limit &&
	     fa->pcs[fa->num-1]->cd[2] < bottom_limit){
	    surface_area[BOTTOM] += ks_calc_triangle_area(fa->pcs[0]->cd,
							  fa->pcs[1]->cd,
							  fa->pcs[fa->num-1]->cd);
	  }
	}
      }
    }
  }
  if(bottom_limit > -100){
    printf("surface area %f %f %f\n",surface_area[ALL],surface_area[BOTTOM],
	   surface_area[ALL]-surface_area[BOTTOM]);
  } else {
    printf("surface area %f\n",surface_area[ALL]);
  }
  fflush(stdout);
}
static void output_line(char *line, size_t line_size, char *str, char *head, FILE *fp)
{
  if(strlen(line)+strlen(str) > line_size){
    strcat(line,"\n");
    fprintf(fp,"%s",line);
    strcpy(line,head);
  }
  strcat(line,str);
}
static void output_cdview_file(CV_BASE *cb, CV_FRAME *fr, FILE *fp)
{
  int i;
  CV_PARTICLE *p;
  CV_BOND *bond;
  CV_ATOM *atom;
  GLfloat *color;
  char line[101],str[101];

  strcpy(line,"'");
  for(atom = cb->atom; atom != NULL; atom = atom->next){
    if(atom->flags&CV_ATOM_SHOW){
      color = ks_get_gl_atom_color(atom->gl_atom,KS_GL_ATOM_COLOR_ATOM,KS_GL_ATOM_BRIGHT_NORMAL);
      /*
	printf("%d %d %f (%f %f %f)\n",atom->flags,atom->atype,
	ks_get_gl_atom_size(atom->gl_atom,KS_GL_ATOM_SIZE_NORMAL),
	color[0],color[1],color[2]);
      */
      sprintf(str," r%d=%.1f",
	      atom->atype,ks_get_gl_atom_size(atom->gl_atom,KS_GL_ATOM_SIZE_NORMAL));
      output_line(line,sizeof(line),str,"'",fp);
      sprintf(str," c%d=(%.2f,%.2f,%.2f)",atom->atype,color[0],color[1],color[2]);
      output_line(line,sizeof(line),str,"'",fp);
    }
  }
  strcat(line,"\n");
  fprintf(fp,"%s",line);

  for(p = fr->particle_end; p != NULL; p = p->prev){
    fprintf(fp,"%d %d %f %f %f",p->label,p->atom->atype,p->cd[0],p->cd[1],p->cd[2]);
    for(i = 1; i < cb->atype_mode_num; i++){
      fprintf(fp," %d",p->atype[i]);
    }
    fprintf(fp,"\n");
  }
  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(bond = p->bond; bond != NULL; bond = bond->next){
      if(p->bond->type == NULL){
	fprintf(fp,"CDVIEW_BOND %d %d\n",p->label,bond->p->label);
      } else {
	fprintf(fp,"CDVIEW_BOND %d %d %d\n",p->label,bond->p->label,p->bond->type->type);
      }
    }
  }
}
static void output_file(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  FILE *fp;
  KS_PDB_ATOM *pa;
  int file_label = 1;
  int next_residue_label = 0;
  char file_name_face[256];
  BOOL water_H_first = KS_TRUE;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  /*
  fp = stdout;
  */
  if((fp = fopen(text,"wt")) == NULL){
    draw_error(cb,"ERROR: can not save %s",text);
    return;
  }

  if(fr->fv != NULL){
    if(fr->fv->contour_fan != NULL){
      output_face_file_header(text,file_name_face,sizeof(file_name_face),fp);
    }
  }

  if(fr->flags&CV_FRAME_CDV){
    output_cdview_file(cb,fr,fp);
  }

  if(fr->flags&CV_FRAME_PDB){ // output REMARK header
    FILE *fph;
    char read_buf[256];
    char comment_key[] = {"REMARK"};
    if((fph = fopen(fr->file_name,"rt")) == NULL){
      ks_error_file(fr->file_name);
      return;
    }
    while(fgets(read_buf,sizeof(read_buf),fph) != NULL){
      if(strncmp(read_buf,comment_key,6) == 0){
	fputs(read_buf,fp);
      }
    }
    fclose(fph);
  }
  if(fr->flags&CV_FRAME_PDB || fr->flags&CV_FRAME_PREP){
    CV_PARTICLE *p;
    if((pa = ks_allocate_pdb_atom()) == NULL){
      draw_error(cb,"ERROR: can not save %s because of memory error",text);
      return;
    }
    for(p = fr->particle_end; p != NULL; p = p->prev){
      pa->label = p->file_label;
      pa->label = file_label++;
      pa->flags = 0;
      if(fr->pdb_ext == KS_TRUE){ // set particle flags
	pa->flags |= KS_PDB_ATOM_EXT;
	if(p->flags&CV_PARTICLE_HETATM){
	  pa->flags |= KS_PDB_ATOM_HETATM;
	}
	if(p->flags&CV_PARTICLE_CHAIN_END){
	  pa->flags |= KS_PDB_ATOM_CHAIN_END;
	}
	if(p->flags&CV_PARTICLE_WATER){
	  pa->flags |= KS_PDB_ATOM_WATER;
	}
	if(p->flags&CV_PARTICLE_MAIN){
	  pa->flags |= KS_PDB_ATOM_EXT_MAIN;
	}
	if(p->flags&CV_PARTICLE_PRIME){
	  pa->flags |= KS_PDB_ATOM_EXT_PRIME;
	}
      }
      if(p->property == NULL){
	/*	printf("%s\n",p->atom->name);*/
	strcpy(pa->name,p->atom->name);
	if(p->flags&CV_PARTICLE_WATER){
	  strcpy(pa->residue,"WAT");
	  if(strcmp(p->atom->name,"H") == 0){ // add 1 or 2 for water H
	    if(water_H_first == KS_TRUE){
	      strcat(pa->name,"1");
	      water_H_first = KS_FALSE;
	    } else {
	      strcat(pa->name,"2");
	      water_H_first = KS_TRUE;
	    }
	  } else { // water O
	    pa->residue_label = ++next_residue_label;
	  }
	} else {
	  strcpy(pa->residue,"???");
	  pa->residue_label = ++next_residue_label;
	}
      } else {
	strcpy(pa->name,p->property->name);
	strcpy(pa->residue,p->property->residue->name);
	pa->residue_label = p->property->residue->file_label;
	next_residue_label = pa->residue_label;
      }
      if(cb->fit_reference == KS_FALSE){
	for(i = 0; i < 3; i++){
	  pa->cd[i] = p->cd[i];
	}
      } else {
	for(i = 0; i < 3; i++){
	  pa->cd[i] = p->cd[i] + fr->fit_trans[i];
	}
      }
      if(fr->flags&CV_FRAME_PDB || (fr->flags&CV_FRAME_PREP && strcmp(pa->name,"DUMM") != 0)){
	ks_fprint_pdb_atom(fp,*pa);
      } else {
	file_label--;
      }
      if((p->flags&CV_PARTICLE_CHAIN_END) || 
	 ((p->flags&CV_PARTICLE_WATER) && strcmp(pa->name,"H2") == 0)){
	ks_fprint_pdb_ter(fp,*pa);
      }
    }
    fprintf(fp,"END\n");
    ks_free_pdb_atom(pa);
  }

  if(fr->fv != NULL){
    if(fr->fv->contour_fan != NULL){
      output_contour_vertex(cb,fr,fp);
    }
  }

  fclose(fp);

  if(fr->fv != NULL){
    if(fr->fv->contour_fan != NULL){
      output_contour_face(cb,fr,file_name_face);
    }
  }
  /* 28150 28151 28152 1 */
}
static void make_common_file_name(CV_BASE *cb, char *file_name, size_t size, char *add)
{
  int i,j;
  char *cp;
  char file_name_base[CV_NAME_MAX];
  int flgs[CV_NAME_MAX];

  for(i = 0; i < CV_NAME_MAX; i++)
    flgs[i] = 1;

  for(i = 1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      for(j = 0; cb->reference_frame->file_name[j]; j++){
	if(cb->frame[i].file_name[j] != cb->frame[0].file_name[j]) flgs[j] = 0;
      }
    }
    /*
    printf("%d %s %s ",i,name[i],name[0]);
    for(j = 0; j < max_len; j++)
      printf("%d",flgs[j]);
    printf("\n");
    */
  }
  for(i = 0,j = 0; cb->frame[0].file_name[i]; i++){
    if(flgs[i] == 1) file_name_base[j++] = cb->frame[0].file_name[i];
  }
  file_name_base[j] = '\0';

  for(cp = file_name_base; *cp; cp++);
  for(;cp != file_name_base && *cp != '.'; cp--);
  for(i = 0; &file_name_base[i] != cp && i < size+strlen(add); i++)
    file_name[i] = file_name_base[i];
  file_name[i] = '\0';
  strcat(file_name,add);
}
static void make_common_file_name_fdv(CV_BASE *cb, char *file_name, size_t size, char *add)
{
  int i,j;
  char *cp;
  char file_name_base[CV_NAME_MAX];
  int flgs[CV_NAME_MAX];

  for(i = 0; i < CV_NAME_MAX; i++)
    flgs[i] = 1;

  for(i = 1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      for(j = 0; cb->reference_frame->file_name_fdv[j]; j++){
	if(cb->frame[i].file_name_fdv[j] != cb->frame[0].file_name_fdv[j]) flgs[j] = 0;
      }
    }
    /*
    printf("%d %s %s ",i,name[i],name[0]);
    for(j = 0; j < max_len; j++)
      printf("%d",flgs[j]);
    printf("\n");
    */
  }
  for(i = 0,j = 0; cb->frame[0].file_name_fdv[i]; i++){
    if(flgs[i] == 1) file_name_base[j++] = cb->frame[0].file_name_fdv[i];
  }
  file_name_base[j] = '\0';

  for(cp = file_name_base; *cp; cp++);
  for(;cp != file_name_base && *cp != '.'; cp--);
  for(i = 0; &file_name_base[i] != cp && i < size+strlen(add); i++)
    file_name[i] = file_name_base[i];
  file_name[i] = '\0';
  strcat(file_name,add);
}
static void set_output_file(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  char *cp;
  CV_BASE *cb;
  CV_FRAME *fr;
  char file_name[256];
  char *add = {"_cv"};
  
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  for(cp = fr->file_name; *cp; cp++);
  for(;cp != fr->file_name && *cp != '.'; cp--);
  for(i = 0; &fr->file_name[i] != cp && i < sizeof(file_name)+strlen(add)+strlen(cp); i++)
    file_name[i] = fr->file_name[i];
  file_name[i] = '\0';
  strcat(file_name,add);
  if(fr->flags&CV_FRAME_PREP){
    strcat(file_name,".pdb");
  } else {
    strcat(file_name,cp);
  }

  /*
  printf("%s %s\n",fr->file_name,file_name);
  for(cp = file_name; *cp; cp++)
    printf("%x '%c'\n",*cp,*cp);
  ks_exit(EXIT_FAILURE);
  */
  ks_set_gl_input_key(gb,"file name > ",file_name,((CV_BASE*)vp)->input_key_font,
		      output_file,KS_FALSE,cb);
}
static void output_bmp_file(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  char message[256];

  cb = (CV_BASE*)vp;
  ks_draw_gl(gb);
  ks_draw_gl(gb);
  ks_save_gl_bmp(text,KS_FALSE,ks_get_gl_viewport_3d(gb));
  strcpy(message,"SAVE BMP: ");
  ks_strncat(message,text,sizeof(message));
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(message,5));
}
static void output_pov_file(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  FILE *fp;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  printf("-> %s\n",text);
  fflush(stdout);

  if((fp = fopen(text, "wt")) == NULL){
    draw_error(cb,"ERROR: can not save %s",text);
    return;
  }

  ks_save_gl_pov_header(gb,fp);

  draw_frame(gb,fr,cb,KS_FALSE,KS_GL_ATOM_TRANS_OFF,
	     (cb->particle_size == CV_PARTICLE_SIZE_SMALL || 
	      fr->flags&CV_FRAME_CDV) ? KS_TRUE:KS_FALSE,NULL,fp);

  if(cb->kabe_mode == CV_KABE_MODE_SIDE && fr->have_side == KS_TRUE){
    draw_side_pov(fp,fr->side,fr->side_radius,cb->pov_info);
  }

  if(ks_gl_base_verbose_level(gb) >= 1 && fr->fv != NULL && fr->fv->field != NULL){
    fv_draw_field(gb,fr->fv->field,fr->fv->field_size,fr->fv->contour_size,fr->fv->contour_fan,
		  &cb->fv_target,fp);
  }

  ks_save_gl_pov_footer(gb,fp,cb->output_file_mode == CV_OUTPUT_PICTURE_POV ? KS_TRUE:KS_FALSE);

  fclose(fp);
}
static void set_output_bmp_pov_file(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  char file_name[256];
  
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(cb->output_file_mode == CV_OUTPUT_PICTURE_BMP){
    if(fr->file_name[0] != '\0'){
      make_output_file_name("bmp",fr->file_name,file_name,sizeof(file_name));
    } else if(fr->file_name_fdv[0] != '\0'){
      make_output_file_name("bmp",fr->file_name_fdv,file_name,sizeof(file_name));
    } else {
      make_output_file_name("bmp","cdv_output",file_name,sizeof(file_name));
    }
    ks_set_gl_input_key(gb,"BMP file name > ",file_name,((CV_BASE*)vp)->input_key_font,
			output_bmp_file,KS_FALSE,cb);
  } else if(cb->output_file_mode == CV_OUTPUT_PICTURE_POV || 
	    cb->output_file_mode == CV_OUTPUT_PICTURE_POV_SHADOW){
    make_output_file_name("pov",fr->file_name,file_name,sizeof(file_name));
    ks_set_gl_input_key(gb,"POV file name > ",file_name,((CV_BASE*)vp)->input_key_font,
			output_pov_file,KS_FALSE,cb);
  }
}
static void look_selected_particle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_PARTICLE *p;
  cb = (CV_BASE*)vp;
  /*  printf("look %d\n",cb->look_selected_particle);*/
  if(cb->look_selected_particle == KS_FALSE){
    cb->look_selected_particle = KS_TRUE;
    if(ks_count_int_list(cb->selected_label) == 1){
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
					cb->selected_label->value);
      /*      printf("%s %f %f %f\n",p->atom->name,p->cd[0],p->cd[1],p->cd[2]);*/
      if(p != NULL){
	cv_set_lookat(gb,cb,p->cd);
      } else {
	printf("look_selected_particle else\n");
	ks_assert(0);
      }
    } else if(ks_count_int_list(cb->selected_label) == 2){
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
					cb->selected_label->next->value);
      /*      printf("%s %f %f %f\n",p->atom->name,p->cd[0],p->cd[1],p->cd[2]);*/
      if(p != NULL){
	cv_set_lookat(gb,cb,p->cd);
      } else {
	printf("look_selected_particle else\n");
	ks_assert(0);
      }
    }
  } else {
    cb->look_selected_particle = KS_FALSE;
    cv_set_lookat(gb,cb,cb->frame[cb->current_frame].center);
  }
}
static void del_particle(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  CV_AGGREGATE *pa;
  CV_CHAIN *pc,*pc_prev,*pc_next;
  CV_RESIDUE *pr,*pr_prev,*pr_next;
  CV_PARTICLE *p,*p_next;
  CV_BOND *bond;

  if(strcmp(text,"y") == 0){
    cb = (CV_BASE*)vp;
    fr = &cb->frame[cb->current_frame];
    /*    printf("del particle %s\n",text);*/
    ks_free_int_list(cb->selected_label);
    cb->selected_label = NULL;
    ks_free_int_list(cb->selected_residue_label);
    cb->selected_residue_label = NULL;
    /*
    for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      printf("agg %d\n",pa->label);
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	printf("  chain %d %s %d\n",pc->label,pc->name,pc->flags&CV_CHAIN_SELECTED);
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  printf("    %d %s %d\n",pr->label,pr->name,pr->flags&CV_RESIDUE_SELECTED);
	}
      }
    }
    */
    for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      pc_prev = NULL;
      for(pc = pa->chain; pc != NULL; pc = pc_next){
	pc_next = pc->next;
	if(pc->flags&CV_CHAIN_SELECTED){
	  if(pc_prev != NULL)
	    pc_prev->next = pc->next;
	  else
	    pa->chain = pc->next;
	  free_cv_residue(pc->residue);
	  ks_free(pc->name);
	  ks_free(pc);
	} else {
	  pr_prev = NULL;
	  for(pr = pc->residue; pr != NULL; pr = pr_next){
	    pr_next = pr->next;
	    if(pr->flags&CV_RESIDUE_SELECTED){
	      if(pr_prev != NULL){
		pr_prev->next = pr->next;
	      } else {
		if(pr->next != NULL){
		  pr->next->flags &= ~CV_RESIDUE_NORMAL;
		  pr->next->flags |= CV_RESIDUE_START;
		}
		pc->residue = pr->next;
	      }
	      if(pr->flags&CV_RESIDUE_END && pr_prev != NULL){
		pr_prev->flags &= ~CV_RESIDUE_NORMAL;
		pr_prev->flags |= CV_RESIDUE_END;
	      }
	      if(pr->particle != NULL){
		ks_free(pr->particle);
	      }
	      ks_free(pr);
	    } else {
	      pr_prev = pr;
	    }
	  }
	  pc_prev = pc;
	}
      }
    }
    /*
    for(pa = fr->aggregate; pa != NULL; pa = pa->next){
      printf("agg %d\n",pa->label);
      for(pc = pa->chain; pc != NULL; pc = pc->next){
	printf("  chain %d %s %d\n",pc->label,pc->name,pc->flags&CV_CHAIN_SELECTED);
	for(pr = pc->residue; pr != NULL; pr = pr->next){
	  printf("    %d %s %d\n",pr->label,pr->name,pr->flags&CV_RESIDUE_SELECTED);
	}
      }
    }
    */
    /*
    for(i = 0; i < cb->particle_hash_size; i++){
      printf("%d %p\n",i,cb->particle_hash[i]);
      for(p = cb->particle_hash[i]; p != NULL; p = p->hash_next){
	printf("(%p %p %d)",p,p->hash_next,p->label);
      }
      printf("\n");
    }
    */
    for(p = fr->particle; p != NULL; p = p_next){
      p_next = p->next;
      if(p->flags&CV_PARTICLE_SELECTED){
	/*	printf("%d %s\n",p->label,p->property->name);*/
	for(bond = p->bond; bond != NULL; bond = bond->next){
	  /*	  printf("bond %d %s\n",bond->p->label,bond->p->property->name);*/
	  bond->p->bond = del_cv_bond(bond->p->bond,p);
	}
	free_cv_bond(p->bond);

	del_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,p->label);
	
	if(p->prev != NULL)
	  p->prev->next = p->next;
	else
	  fr->particle = p->next;
	if(p->next != NULL)
	  p->next->prev = p->prev;
	else
	  fr->particle_end = p->prev;
	if(p->property != NULL){
	  free_cv_particle(p->property->alternative);
	  ks_free(p->property);
	}
	ks_free(p);
      }
    }
    /*
    for(i = 0; i < cb->particle_hash_size; i++){
      printf("%d %p\n",i,cb->particle_hash[i]);
      for(p = cb->particle_hash[i]; p != NULL; p = p->hash_next){
	printf("(%p %p %d)",p,p->hash_next,p->label);
      }
      printf("\n");
    }
    */
  }
}
static void set_periodic_frame(CV_BASE *cb, CV_FRAME *fr)
{
  int i;
  int size;
  int p_num[27];
  double r;
  CV_PARTICLE *p;
  GLfloat offset[3];

  if(fr->periodic_p == NULL){
    if((fr->periodic_p = (CV_PARTICLE***)ks_malloc(27*sizeof(CV_PARTICLE**),"fr->periodic_p")) 
       == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
    if((fr->periodic_p_num = ks_malloc_int_p(27,"fr->periodic_p_num")) == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
    for(i = 0; i < 27; i++){
      fr->periodic_p[i] = NULL;
      fr->periodic_p_num[i] = 0;
    }
  }
  for(i = 0; i < 27; i++){
    p_num[i] = 0;
  }
  for(i = 0; i < 27; i++){
    for(p = fr->particle; p != NULL; p = p->next){
      if(cb->particle_size == CV_PARTICLE_SIZE_NORMAL){
	size = KS_GL_ATOM_SIZE_NORMAL;
      } else if(cb->particle_size == CV_PARTICLE_SIZE_SMALL){
	size = KS_GL_ATOM_SIZE_SMALL;
      } else if(cb->particle_size == CV_PARTICLE_SIZE_LARGE){
	size = KS_GL_ATOM_SIZE_LARGE;
      }
      r = ks_get_gl_atom_size(p->atom->gl_atom,size)*cb->particle_size_scale;
      //      if(p->label == 2)
      //	printf("%d %f %f %f %f",p->label,p->cd[0],p->cd[1],p->cd[2],r);
      if(check_periodic(i,p,r,fr->side,offset) == KS_TRUE){
	p_num[i]++;
	//      if(p->label == 2)
	//	printf(" % f % f % f\n",offset[0],offset[1],offset[2]);
      } else {
	//      if(p->label == 2)
	//	printf("\n");
      }
    }
  }
  for(i = 0; i < 27; i++){
    /*    printf("%d %d\n",i,p_num[i]);*/
    if(fr->periodic_p_num[i] != p_num[i]){
      if(fr->periodic_p[i] != NULL){
	ks_free(fr->periodic_p[i]);
      }
      fr->periodic_p[i] = NULL;
      fr->periodic_p_num[i] = p_num[i];
    }
    if(fr->periodic_p[i] == NULL){
      if((fr->periodic_p[i] = (CV_PARTICLE**)ks_malloc(fr->periodic_p_num[i]*sizeof(CV_PARTICLE*),
						       "fr->periodic_p[i]")) == NULL){
	ks_error_memory();
	ks_exit(EXIT_FAILURE);
      }
    }
    p_num[i] = 0;
    for(p = fr->particle; p != NULL; p = p->next){
      if(cb->particle_size == CV_PARTICLE_SIZE_NORMAL){
	size = KS_GL_ATOM_SIZE_NORMAL;
      } else if(cb->particle_size == CV_PARTICLE_SIZE_SMALL){
	size = KS_GL_ATOM_SIZE_SMALL;
      } else if(cb->particle_size == CV_PARTICLE_SIZE_LARGE){
	size = KS_GL_ATOM_SIZE_LARGE;
      }
      r = ks_get_gl_atom_size(p->atom->gl_atom,size)*cb->particle_size_scale;
      if(check_periodic(i,p,r,fr->side,NULL) == KS_TRUE){
	fr->periodic_p[i][p_num[i]++] = p;
      }
    }
  }
}
static void set_periodic_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_FRAME *fr;
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  if(fr->have_side == KS_TRUE){
    if(cb->periodic_mode == KS_FALSE){
      cb->periodic_mode = KS_TRUE;
      set_periodic_frame(cb,fr);
      malloc_clip_plane(cb);
      for(i = 0; i < 6; i++){
	cb->clip_plane_flg[i] = KS_TRUE;
      }
      for(i = 0; i < 3; i++){
	cb->clip_plane[i][i]   = 1.0;
	cb->clip_plane[i][3]   = fr->side[0][0][0][i];
	cb->clip_plane[i+3][i] = -1.0;
	cb->clip_plane[i+3][3] = fr->side[1][1][1][i];
      }
    } else {
      cb->periodic_mode = KS_FALSE;
      for(i = 0; i < 6; i++){
	cb->clip_plane_flg[i] = KS_FALSE;
      }
    }
  }
}
static void set_del_particle(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  ks_set_gl_input_key(gb,"Rially delete selected particle? ",NULL,((CV_BASE*)vp)->input_key_font,
		      del_particle,KS_TRUE,cb);
}
static void bool_fit_reference(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  char *on = {"fit reference on"};
  char *off = {"fit reference off"};
  cb = (CV_BASE*)vp;
  if(cb->fit_reference == KS_TRUE){
    cb->fit_reference = KS_FALSE;
    cb->telop = ks_del_text_list(cb->telop,on);
    cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(off,2));
  } else {
    cb->fit_reference = KS_TRUE;
    cb->telop = ks_del_text_list(cb->telop,off);
    cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(on,2));
  }
}
static void bool_overlap_reference(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  char *on = {"overlap reference on"};
  char *off = {"overlap reference off"};
  cb = (CV_BASE*)vp;
  if(cb->overlap_reference == KS_TRUE){
    cb->overlap_reference = KS_FALSE;
    cb->telop = ks_del_text_list(cb->telop,on);
    cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(off,2));
  } else {
    cb->overlap_reference = KS_TRUE;
    cb->telop = ks_del_text_list(cb->telop,off);
    cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(on,2));
  }
}
static void calc_pdb_avrage(KS_GL_BASE *gb, CV_BASE *cb)
{
  static int c = 0;
  int i;
  CV_FRAME *fr;
  CV_PARTICLE *p,*q;
  double dd[3];

  fr = &cb->frame[cb->current_frame];
  /*  printf("calc_pdb_avrage %d %s\n",cb->current_frame,fr->file_name);*/
  for(p = cb->avrage_frame->particle, q = fr->particle;
      p != NULL && q != NULL; p = p->next, q = q->next){
    if((p->property != NULL && q->property != NULL) || cb->enable_cdv_rmsd_rmsf == KS_TRUE){
      dd[0] = (fr->fit_rot[0]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[4]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[8]* (q->cd[2]-fr->center[2]));
      dd[1] = (fr->fit_rot[1]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[5]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[9]* (q->cd[2]-fr->center[2]));
      dd[2] = (fr->fit_rot[2]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[6]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[10]*(q->cd[2]-fr->center[2]));
      for(i = 0; i < 3; i++){
	p->cd[i] = (p->cd[i]*c + dd[i]+cb->avrage_frame->center[i])/(c+1);
      }
      /*
      if(p->label == 0){
	printf("%d (%f %f %f)(%f %f %f)(%f %f %f)\n",q->label,q->cd[0],q->cd[1],q->cd[2]
	       ,fr->center[0],fr->center[1],fr->center[2]
	       ,p->cd[0]*(c+1),p->cd[1]*(c+1),p->cd[2]*(c+1));
      }
      */
      /*
      printf("%d %d %f %f %f %f %f %f\n",cb->current_frame,p->label
	     ,q->cd[0],q->cd[1],q->cd[2]
	     ,p->cd[0]*(c+1),p->cd[1]*(c+1),p->cd[2]*(c+1));
      */
    }
  }
  c++;
}
/*
static void calc_pdb_avrage_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  if(cb->current_frame < cb->frame_num-1){
    calc_pdb_avrage(gb,cb);
    move_frame(gb,cb,cb->current_frame+1);
  } else {
    cb->idle = NULL;
  }
}
*/
static BOOL init_avrage_frame(CV_BASE *cb)
{
  CV_PARTICLE *p;
  if(cb->avrage_frame == NULL){
    if((cb->avrage_frame = allocate_frame(1,cb->atype_mode,&cb->fi)) == NULL){
      return KS_FALSE;
    }
    if(cb->reference_frame->flags&CV_FRAME_PDB){
      cb->avrage_frame->flags |= CV_FRAME_PDB;
    } else if(cb->reference_frame->flags&CV_FRAME_CDV){
      cb->avrage_frame->flags |= CV_FRAME_CDV;
    } else {
      return KS_FALSE;
    }
    strcpy(cb->avrage_frame->file_name,cb->reference_frame->file_name);
    if(init_frame(cb->avrage_frame, &cb->fv_base,cb)
       == KS_FALSE){
      return KS_FALSE;
    }
    for(p = cb->avrage_frame->particle; p != NULL; p = p->next){
      /*    printf("%d %s %f %f %f\n",p->label,p->atom->name,p->cd[0],p->cd[1],p->cd[2]);*/
      p->cd[0] = 0; p->cd[1] = 0; p->cd[2] = 0;
      p->move[0] = 0; p->move[1] = 0; p->move[2] = 0;
    }
  }
  return KS_TRUE;
}
/*
static void set_calc_pdb_avrage(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  if(init_avrage_frame(cb) == KS_FALSE){
    draw_error(cb,"ERROR: init_frame %s",cb->avrage_frame->file_name);
    return;
  }

  move_frame(gb,cb,0);
  cb->idle = calc_pdb_avrage_idle;
}
*/
static void calc_rmsd_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i,c;
  FILE *fp;
  CV_FRAME *fr;
  CV_PARTICLE *p,*q;
  double dd[3];
  int n;

  fr = &cb->frame[cb->current_frame];

  n = 0;
  for(p = cb->reference_frame->particle, q = fr->particle; 
      p != NULL && q != NULL; p = p->next, q = q->next){
    if(((p->flags&CV_PARTICLE_PRIME) && (q->flags&CV_PARTICLE_PRIME)) || 
       cb->enable_cdv_rmsd_rmsf == KS_TRUE){
      dd[0] = (fr->fit_rot[0]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[4]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[8]* (q->cd[2]-fr->center[2]));
      dd[1] = (fr->fit_rot[1]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[5]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[9]* (q->cd[2]-fr->center[2]));
      dd[2] = (fr->fit_rot[2]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[6]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[10]*(q->cd[2]-fr->center[2]));
      for(i = 0; i < 3; i++){
	dd[i] -= p->cd[i]-cb->reference_frame->center[i];
      }
      fr->rmsd += dd[0]*dd[0]+dd[1]*dd[1]+dd[2]*dd[2];
      n++;
    }
  }
  fr->rmsd = sqrt(fr->rmsd/n);

  /*  printf("%s %d %f\n",fr->file_name,n,fr->rmsd);*/

  ks_set_gl_graph_2d(cb->rmsd_graph,fr->rmsd);

  calc_pdb_avrage(gb,cb);

  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  if((fp = fopen(cb->rmsd_file_name,"wt")) == NULL){
    draw_error(cb,"ERROR: file open error %s",cb->rmsd_file_name);
    return;
  }
  c = 1;
  for(i = 0; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      fr = &cb->frame[i];
      if(fr->have_time == KS_TRUE){
	fprintf(fp,"%e %f\n",fr->time,fr->rmsd);
      } else {
	fprintf(fp,"%d %f\n",c++,fr->rmsd);
      }
    }
  }
  fclose(fp);
  cb->idle = NULL;
}
static void draw_rmsd(KS_GL_BASE *gb, int id, int width, int height, void *vp)
{
  CV_BASE *cb;
  int margin = 10;
  int num_sep = 30;

  cb = (CV_BASE*)vp;
  if(cb->rmsd_graph != NULL){
    ks_draw_gl_graph_2d(gb,cb->rmsd_graph,
			margin+num_sep,margin,width-margin*2-num_sep,height-margin*2,
			0,
			(cb->current_frame-(cb->reference_frame->label))/cb->skip_frame);
    /*
    printf("%d %d\n",cb->current_frame,
	   (cb->current_frame-(cb->reference_frame->label%cb->skip_frame))/cb->skip_frame);
    */
  }
}
static void set_calc_rmsd_idle(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;

  cb = (CV_BASE*)vp;

  if(sizeof(cb->rmsd_file_name) < strlen(text)){
    draw_error(cb,"ERROR: file name is too long %s",text);
    return;
  }
  strcpy(cb->rmsd_file_name,text);
  first_frame(gb,0,0,(void*)cb);

  cb->rmsd_graph=ks_allocate_gl_graph_2d("RMSD",
					 (cb->frame_num-cb->reference_frame->label)/cb->skip_frame,
					 0,1,0.1,1,1,0,
					 4,1,0,0,
					 KS_GL_FONT_HELVETICA_10,
					 KS_GL_GRAPH_AUTO|
					 KS_GL_GRAPH_DRAW_SCALE);
  {
    int width = 150;
    int height = 120;
    cb->rmsd_window_id = ks_add_gl_base_window(gb,
					       KS_GL_WINDOW_SHOW,
					       "RMSD",
					       ks_get_gl_viewport_2d_width(gb)-width-30,
					       ks_get_gl_viewport_2d_height(gb)-
					       height-KS_GL_WINDOW_TITLE_BAR_SIZE-30
					       +(cb->rmsf_window_id >=0 ? -150:0)
					       +(cb->distance_window_id >=0 ? -150:0),
					       width,height,draw_rmsd,NULL,NULL,NULL,cb);
  }

  if(init_avrage_frame(cb) == KS_FALSE){
    draw_error(cb,"ERROR: init_frame %s",cb->avrage_frame->file_name);
    return;
  }
  cb->idle = calc_rmsd_idle;
}
static void set_calc_rmsd(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *add = {".rmsd"};
  char file_name[256];

  if(cb->rmsd_graph == NULL){
    make_common_file_name(cb,file_name,sizeof(file_name),add);
    ks_set_gl_input_key(gb,"rmsd file name > ",file_name,cb->input_key_font,
			set_calc_rmsd_idle,KS_FALSE,cb);
  } else {
    ks_show_gl_base_window(gb,cb->rmsd_window_id);
  }
}
static void calc_rmsf_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  static int c = 0;
  int i;
  CV_FRAME *fr;
  CV_PARTICLE *p,*q;
  double dd[6];
  FILE *fp;

  fr = &cb->frame[cb->current_frame];

  /*  printf("rmsf %d %s\n",cb->current_frame,fr->file_name);*/
  for(p = cb->avrage_frame->particle, q = fr->particle;
      p != NULL && q != NULL; p = p->next, q = q->next){
    if((p->property != NULL && q->property != NULL) || cb->enable_cdv_rmsd_rmsf == KS_TRUE){
      dd[0] = (fr->fit_rot[0]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[4]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[8]* (q->cd[2]-fr->center[2]));
      dd[1] = (fr->fit_rot[1]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[5]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[9]* (q->cd[2]-fr->center[2]));
      dd[2] = (fr->fit_rot[2]* (q->cd[0]-fr->center[0]) + 
	       fr->fit_rot[6]* (q->cd[1]-fr->center[1]) + 
	       fr->fit_rot[10]*(q->cd[2]-fr->center[2]));
      for(i = 0; i < 3; i++){
	dd[i+3] = dd[i]+cb->avrage_frame->center[i]-p->cd[i];
      }
      p->move[0] = (p->move[0]*c + sqrt(dd[3]*dd[3]+dd[4]*dd[4]+dd[5]*dd[5]))/(c+1);
      /*      printf("%d %f\n",c,p->move[0]);*/
      /*
      if(p->label == 0){
	printf("%d (%f %f %f)(%f %f %f)(%f %f %f)\n",q->label
	       ,q->cd[0],q->cd[1],q->cd[2],p->cd[0],p->cd[1],p->cd[2]
	       ,cb->avrage_frame->center[0]
	       ,cb->avrage_frame->center[1]
	       ,cb->avrage_frame->center[2]);
      }
      */
      /*
      if(p->label == 0)
	printf("%f (%f %f %f)(%f %f %f)(%f %f %f)(%f %f %f)\n"
	       ,sqrt(dd[3]*dd[3]+dd[4]*dd[4]+dd[5]*dd[5])
	       ,q->cd[0],q->cd[1],q->cd[2]
	       ,fr->center[0],fr->center[1],fr->center[2]
	       ,p->cd[0],p->cd[1],p->cd[2]
	       ,cb->avrage_frame->center[0]
	       ,cb->avrage_frame->center[1]
	       ,cb->avrage_frame->center[2]);
      */
    }
  }
  c++;

  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  if((fp = fopen(cb->rmsf_file_name,"wt")) == NULL){
    draw_error(cb,"ERROR: file open error %s",cb->rmsf_file_name);
    return;
  }
  for(p = cb->avrage_frame->particle_end; p != NULL; p = p->prev){
    if(p->flags&CV_PARTICLE_PRIME || cb->enable_cdv_rmsd_rmsf == KS_TRUE){
      if(p->property != NULL){
	fprintf(fp,"%d %f\n",p->property->residue->file_label,p->move[0]);
      } else if(cb->enable_cdv_rmsd_rmsf == KS_TRUE){
	fprintf(fp,"%d %f\n",p->label,p->move[0]);
      }
      ks_set_gl_graph_2d(cb->rmsf_graph,p->move[0]);
    }
  }
  fclose(fp);
  cb->idle = NULL;
}
static void draw_rmsf(KS_GL_BASE *gb, int id, int width, int height, void *vp)
{
  CV_BASE *cb;
  int margin = 10;
  int num_sep = 30;

  cb = (CV_BASE*)vp;
  if(cb->rmsf_graph != NULL){
    int residue;
    CV_RESIDUE *pr;
    if(cb->selected_residue_label != NULL){
      pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
					(unsigned int)cb->selected_residue_label->value);
      residue = pr->label;
    } else {
      residue = -1;
    }
    ks_draw_gl_graph_2d(gb,cb->rmsf_graph,
			margin+num_sep,margin,width-margin*2-num_sep,height-margin*2,
			0,residue);
  }
}
static void set_calc_rmsf_idle(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_BASE *cb;
  int size;

  cb = (CV_BASE*)vp;

  if(cb->avrage_frame == NULL){
    if(init_avrage_frame(cb) == KS_FALSE){
      draw_error(cb,"ERROR: init_frame %s",cb->avrage_frame->file_name);
      return;
    }
    first_frame(gb,0,0,(void*)cb);
    calc_pdb_avrage(gb,cb);

    for(i = 1; i < cb->frame_num; i++){
      if(cb->frame[i].flags&CV_FRAME_ENABLE){
	move_frame(gb,cb,i);
	calc_pdb_avrage(gb,cb);
      }
    }
    /*
    {
      CV_PARTICLE *p;
      for(p = cb->avrage_frame->particle; p->next != NULL; p = p->next);
      for(; p != NULL; p = p->prev){
	printf("%d %f %f %f\n",p->label,p->cd[0],p->cd[1],p->cd[2]);
      }
    }
    */
  }

  if(sizeof(cb->rmsf_file_name) < strlen(text)){
    draw_error(cb,"ERROR: file name is too long %s",text);
    return;
  }
  strcpy(cb->rmsf_file_name,text);
  first_frame(gb,0,0,(void*)cb);

  size = cb->reference_frame->residue_label_max;
  if(size == 0 && cb->enable_cdv_rmsd_rmsf){
    size = count_cv_particle(cb->frame[cb->current_frame].particle);
  }
  assert(size != 0);
  cb->rmsf_graph=ks_allocate_gl_graph_2d("RMSF",size,
					 0,1,0.1,0,1,1,
					 4,1,0,0,
					 KS_GL_FONT_HELVETICA_10,
					 KS_GL_GRAPH_AUTO|
					 KS_GL_GRAPH_DRAW_SCALE);

  {
    int width = 150;
    int height = 120;
    cb->rmsf_window_id = ks_add_gl_base_window(gb,
					       KS_GL_WINDOW_SHOW,
					       "RMSF",
					       ks_get_gl_viewport_2d_width(gb)-
					       width-30,
					       ks_get_gl_viewport_2d_height(gb)-
					       height-KS_GL_WINDOW_TITLE_BAR_SIZE-30
					       +(cb->rmsd_window_id >=0 ? -150:0)
					       +(cb->distance_window_id >=0 ? -150:0),
					       width,height,draw_rmsf,NULL,NULL,NULL,cb);
  }

  cb->idle = calc_rmsf_idle;
}
static void set_calc_rmsf(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *add = {".rmsf"};
  char file_name[256];

  if(cb->rmsf_graph == NULL){
    make_common_file_name(cb,file_name,sizeof(file_name),add);
    ks_set_gl_input_key(gb,"rmsf file name > ",file_name,cb->input_key_font,
			set_calc_rmsf_idle,KS_FALSE,cb);
  } else {
    ks_show_gl_base_window(gb,cb->rmsf_window_id);
  }
}
static void calc_distance_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i,c;
  FILE *fp;
  CV_FRAME *fr;
  CV_PARTICLE *p,*q;
  double dd[3];

  fr = &cb->frame[cb->current_frame];

  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				    (unsigned int)cb->selected_label->value);
  q = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				    (unsigned int)cb->selected_label->next->value);
  for(i = 0; i < 3; i++)
    dd[i] = (p->cd[i]+p->move[i])-(q->cd[i]+q->move[i]);
  fr->distance = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);

  ks_set_gl_graph_2d(cb->distance_graph,fr->distance);

  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  if((fp = fopen(cb->distance_file_name,"wt")) == NULL){
    draw_error(cb,"ERROR: file open error %s",cb->distance_file_name);
    return;
  }
  c = 1;
  for(i = 0; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      fr = &cb->frame[i];
      if(fr->have_time == KS_TRUE){
	fprintf(fp,"%e %f\n",fr->time,fr->distance);
      } else {
	fprintf(fp,"%d %f\n",c++,fr->distance);
      }
    }
  }
  fclose(fp);
  cb->idle = NULL;
}
static void draw_distance(KS_GL_BASE *gb, int id, int width, int height, void *vp)
{
  CV_BASE *cb;
  int margin = 10;
  int num_sep = 30;

  cb = (CV_BASE*)vp;
  if(cb->distance_graph != NULL){
    ks_draw_gl_graph_2d(gb,cb->distance_graph,
			margin+num_sep,margin,width-margin*2-num_sep,height-margin*2,
			0,
			(cb->current_frame-(cb->reference_frame->label))/cb->skip_frame);
    /*
    printf("%d %d\n",cb->current_frame,
	   (cb->current_frame-(cb->reference_frame->label%cb->skip_frame))/cb->skip_frame);
    */
  }
}
static void set_calc_distance_idle(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;

  cb = (CV_BASE*)vp;
  if(sizeof(cb->distance_file_name) < strlen(text)){
    draw_error(cb,"ERROR: file name is too long %s",text);
    return;
  }
  strcpy(cb->distance_file_name,text);
  first_frame(gb,0,0,(void*)cb);

  cb->distance_graph=ks_allocate_gl_graph_2d("DISTANCE",
					     (cb->frame_num-cb->reference_frame->label)/
					     cb->skip_frame,
					     0,1,0.1,0.9 ,0.6 ,1.0,
					     4,1,0,0,
					     KS_GL_FONT_HELVETICA_10,
					     KS_GL_GRAPH_AUTO|
					     KS_GL_GRAPH_DRAW_SCALE);
  {
    int width = 150;
    int height = 120;
    cb->distance_window_id = ks_add_gl_base_window(gb,
						   KS_GL_WINDOW_SHOW,
						   "DISTANCE",
						   ks_get_gl_viewport_2d_width(gb)-
						   width-30,
						   ks_get_gl_viewport_2d_height(gb)-
						   height-KS_GL_WINDOW_TITLE_BAR_SIZE-30
						   +(cb->rmsd_window_id >=0 ? -150:0)
						   +(cb->rmsf_window_id >=0 ? -150:0),
						   width,height,draw_distance,NULL,NULL,NULL,cb);
  }

  if(init_avrage_frame(cb) == KS_FALSE){
    draw_error(cb,"ERROR: init_frame %s",cb->avrage_frame->file_name);
    return;
  }
  cb->idle = calc_distance_idle;
}
static void set_calc_distance(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char *add = {".dis"};
  char file_name[256];

  if(ks_count_int_list(cb->selected_label) == 2){
    if(cb->distance_graph == NULL && ks_count_int_list(cb->selected_label) == 2){
      make_common_file_name(cb,file_name,sizeof(file_name),add);
      ks_set_gl_input_key(gb,"distance file name > ",file_name,cb->input_key_font,
			  set_calc_distance_idle,KS_FALSE,cb);
    } else {
      ks_show_gl_base_window(gb,cb->distance_window_id);
    }
  }
}
static void del_distance(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  if(cb->distance_window_id >= 0){
    ks_del_gl_base_window(gb,cb->distance_window_id);
    if(cb->distance_graph != NULL){
      ks_free_gl_graph_2d(cb->distance_graph);
      cb->distance_graph = NULL;
    }
    cb->distance_window_id = -1;
  }
}
static void store_memory_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  for(i = cb->current_frame+1; i < cb->frame_num; i++){
    if(cb->frame[i].flags&CV_FRAME_ENABLE){
      move_frame(gb,cb,i);
      return;
    }
  }
  cb->idle = NULL;
}
static void set_store_memory_idle(KS_GL_BASE *gb, char *text, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;

  cb->no_mem = KS_FALSE;
  /*  printf("%s\n",text);*/
  set_skip_frame(gb,cb,text);
  first_frame(gb,0,0,(void*)cb);
  cb->idle = store_memory_idle;
}
static void set_store_memory(KS_GL_BASE *gb, CV_BASE *cb, char *text)
{
  char message[64];
  sprintf(message,"skip number (total file num is %d) > ",cb->frame_num);
  ks_set_gl_input_key(gb,message,"1",cb->input_key_font,
		      set_store_memory_idle,KS_FALSE,cb);
}
static void command_mode(KS_GL_BASE *gb, char *text, void *vp)
{
  int i,len;
  BOOL hit;
  CV_BASE *cb;
  char command[CV_COMMAND_LEN_MAX];

  cb = (CV_BASE*)vp;
  if(strlen(text) > CV_COMMAND_LEN_MAX){
    draw_error(cb,"ERROR: command '%s' is too long",text);
    return;
  }

  for(i = strlen(text); i >= 0; i--){
    if(isalpha(((unsigned char*)text)[i]) || text[i] == '_'){ /* remove numerical values at tail */
      len = i;
      break;
    }
  }
  ++len;
  for(i = 0; i < len; i++){
    command[i] = text[i];
  }
  command[i] = '\0';

  hit = KS_FALSE;
  for(i = 0; i < CV_COMMAND_NUM; i++){
    /*
    printf("command '%s' %d  '%s' '%s' %s %d %d %d %d\n",text,
	   i,cb->command[i].key[CV_COMMAND_KEY_1],cb->command[i].key[CV_COMMAND_KEY_2],
	   cb->command[i].comment,
	   strncmp(text,cb->command[i].key[CV_COMMAND_KEY_1],
		   strlen(cb->command[i].key[CV_COMMAND_KEY_1])),
	   strlen(cb->command[i].key[CV_COMMAND_KEY_1]),
	   strncmp(text,cb->command[i].key[CV_COMMAND_KEY_2],
		   strlen(cb->command[i].key[CV_COMMAND_KEY_2])),
	   strlen(cb->command[i].key[CV_COMMAND_KEY_2]));
    */
    if(strcmp(command,cb->command[i].key[CV_COMMAND_KEY_1]) == 0 || 
       strcmp(command,cb->command[i].key[CV_COMMAND_KEY_2]) == 0){
      /*
      printf("hit %d %d %d\n",i,
	     strcmp(text,cb->command[i].key[CV_COMMAND_KEY_1]),
	     strcmp(text,cb->command[i].key[CV_COMMAND_KEY_2]));
      */
      (cb->command[i].process_command)(gb,cb,text);
      hit = KS_TRUE;
    }
  }
  if(hit == KS_FALSE){
    draw_error(cb,"ERROR: unknown command '%s'",command);
  }
  /*
  if(strcmp(text,"calc rmsd") == 0 || strcmp(text,"rmsd") == 0){
    set_calc_rmsd(gb,cb);
  } else if(strcmp(text,"calc rmsf") == 0 || strcmp(text,"rmsf") == 0){
    set_calc_rmsf(gb,cb);
  } else if(strcmp(text,"calc pdb_average") == 0 || strcmp(text,"av") == 0){
    set_calc_pdb_avrage(gb,cb);
  } else if(strcmp(text,"store memory") == 0 || strcmp(text,"mem") == 0){
    store_memory(cb);
  } else {
    draw_error(cb,"ERROR: unknown command");
  }
  */
}
static void set_command_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  ks_set_gl_input_key(gb,"command >",NULL,((CV_BASE*)vp)->input_key_font,
		      command_mode,KS_FALSE,(CV_BASE*)vp);
}
static void increment_selected_residue(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_RESIDUE *pr;
  cb = (CV_BASE*)vp;
  if(cb->selected_residue_label == NULL){
    cb->selected_residue_label=ks_add_int_list(cb->selected_residue_label,ks_new_int_list(0));
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,0);
    if(pr != NULL) pr->flags += CV_RESIDUE_SELECTED;
  } else if(cb->frame[cb->current_frame].residue_label_max-1 > cb->selected_residue_label->value){
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
				      (unsigned int)cb->selected_residue_label->value);
    if(pr != NULL) pr->flags &= ~CV_RESIDUE_SELECTED;
    cb->selected_residue_label->value++;
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
				      (unsigned int)cb->selected_residue_label->value);
    if(pr != NULL) pr->flags += CV_RESIDUE_SELECTED;
  }
}
static void decrement_selected_residue(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_RESIDUE *pr;
  cb = (CV_BASE*)vp;
  if(cb->selected_residue_label == NULL){
    cb->selected_residue_label=
      ks_add_int_list(cb->selected_residue_label,
		      ks_new_int_list(cb->frame[cb->current_frame].residue_label_max-1));
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
				      cb->frame[cb->current_frame].residue_label_max-1);
    if(pr != NULL) pr->flags += CV_RESIDUE_SELECTED;
  } else if(cb->selected_residue_label->value > 0){
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
				      (unsigned int)cb->selected_residue_label->value);
    if(pr != NULL) pr->flags &= ~CV_RESIDUE_SELECTED;
    cb->selected_residue_label->value--;
    pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
				      (unsigned int)cb->selected_residue_label->value);
    if(pr != NULL) pr->flags += CV_RESIDUE_SELECTED;
  }
}
static void change_background_color(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  static BOOL change = KS_FALSE;
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(change == KS_FALSE){
    ks_set_gl_base_background_color(gb,1.0,1.0,1.0,1.0);
    ks_set_gl_base_foreground_color(gb,0.0,0.0,0.0,0.0);
    for(i = 0; i < 4; i++){
      cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][i] = 
	cb->box_frame_color[CV_BOX_FRAME_COLOR_2][i];
    }
    change = KS_TRUE;
  } else {
    ks_set_gl_base_background_color(gb,0.0,0.0,0.0,0.0);
    ks_set_gl_base_foreground_color(gb,1.0,1.0,1.0,1.0);
    for(i = 0; i < 4; i++){
      cb->box_frame_color[CV_BOX_FRAME_COLOR_CURRENT][i] = 
	cb->box_frame_color[CV_BOX_FRAME_COLOR_1][i];
    }
    change = KS_FALSE;
  }
}
static void decrease_detail(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(cb->detail > CV_SPHERE_DETAIL_MIN){
    cb->detail--;
  }
}
static void increase_detail(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(cb->detail < CV_SPHERE_DETAIL_MAX){
    cb->detail++;
  }
}
static void change_color_mode(CV_BASE *cb, BOOL add)
{
  char add_text[64] = {"color mode: "};
  char del_text[64] = {"color mode: "};
  if(add == KS_TRUE){
    if(cb->color_mode == CV_COLOR_MODE_NUM-1){
      cb->color_mode = 0;
      /*      cb->telop = ks_del_text_list(cb->telop,cb->color_mode_name[CV_COLOR_MODE_NUM-1]);*/
      strcat(del_text,cb->color_mode_name[CV_COLOR_MODE_NUM-1]);
    } else {
      cb->color_mode++;
      /*      cb->telop = ks_del_text_list(cb->telop,cb->color_mode_name[cb->color_mode-1]);*/
      strcat(del_text,cb->color_mode_name[cb->color_mode-1]);
    }
  /*
  */
  } else {
    if(cb->color_mode == 0){
      cb->color_mode = CV_COLOR_MODE_NUM-1;
      /*      cb->telop = ks_del_text_list(cb->telop,cb->color_mode_name[0]);*/
      strcat(del_text,cb->color_mode_name[0]);
    } else {
      cb->color_mode--;
      /*      cb->telop = ks_del_text_list(cb->telop,cb->color_mode_name[cb->color_mode+1]);*/
      strcat(del_text,cb->color_mode_name[cb->color_mode+1]);
    }
  }
  cb->telop = ks_del_text_list(cb->telop,del_text);
  strcat(add_text,cb->color_mode_name[cb->color_mode]);
  /*
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(cb->color_mode_name[cb->color_mode],5));
  */
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(add_text,5));

#ifdef USE_GLSL
  set_shader_sphere_draw_info_color(cb,&cb->frame[cb->current_frame]);
  ks_update_gl_shader_sphere_draw_info_share(cb->sb->shader[SHADER_SPHERE],0,CV_SPHERE_DETAIL_CNT,
					     cb->sb->sphere_draw_info);
#endif
}
static void decrease_color_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  change_color_mode((CV_BASE*)vp,KS_FALSE);
}
static void increase_color_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  change_color_mode((CV_BASE*)vp,KS_TRUE);
}
static void change_draw_mode(CV_BASE *cb, BOOL add)
{
  if(cb->frame[cb->current_frame].flags&CV_FRAME_CDV){
    if(cb->draw_mode == CV_DRAW_MODE_PARTICLE){
      cb->draw_mode = CV_DRAW_MODE_LINE;
    } else {
      cb->draw_mode = CV_DRAW_MODE_PARTICLE;
    }
  } else {
    if(add == KS_TRUE){
      if(cb->draw_mode == CV_DRAW_MODE_NUM-1){
	cb->draw_mode = 0;
      } else {
	cb->draw_mode++;
      }
      if(cb->draw_mode == CV_DRAW_MODE_SOLVENT_EXCLUDE_SURFACE && 
	 cb->frame[cb->current_frame].have_solvent_exclude_surface == KS_FALSE){
	if(cb->draw_mode == CV_DRAW_MODE_NUM-1){
	  cb->draw_mode = 0;
	} else {
	  cb->draw_mode++;
	}
      }
    } else {
      if(cb->draw_mode == 0){
	cb->draw_mode = CV_DRAW_MODE_NUM-1;
      } else {
	cb->draw_mode--;
      }
      if(cb->draw_mode == CV_DRAW_MODE_SOLVENT_EXCLUDE_SURFACE && 
	 cb->frame[cb->current_frame].have_solvent_exclude_surface == KS_FALSE){
	if(cb->draw_mode == 0){
	  cb->draw_mode = CV_DRAW_MODE_NUM-1;
	} else {
	  cb->draw_mode--;
	}
      }
    }
  }
#ifdef USE_GLSL
  update_shader_sphere_draw_info(cb,&cb->frame[cb->current_frame]);
  update_shader_rod_draw_info(cb,&cb->frame[cb->current_frame]);
#endif
}
static void decrease_draw_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  change_draw_mode((CV_BASE*)vp,KS_FALSE);
}
static void increase_draw_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  change_draw_mode((CV_BASE*)vp,KS_TRUE);
}
static void toggle_projection_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(cb->projection_mode == CV_PROJECTION_MODE_PERSPECTIVE){
    cb->projection_mode = CV_PROJECTION_MODE_ORTHO;
    ks_gl_base_set_projection_mode(gb,KS_GL_BASE_ORTHO);
  } else if(cb->projection_mode == CV_PROJECTION_MODE_ORTHO){
    cb->projection_mode = CV_PROJECTION_MODE_PERSPECTIVE;
    ks_gl_base_set_projection_mode(gb,KS_GL_BASE_PERSPECTIVE);
  }
}
static void change_particle_size(CV_BASE *cb, BOOL increase)
{
  static int prev_detail;
  static int prev_detail_small_diff = 0;
  static int prev_detail_large_diff = 0;
  if(cb->frame[cb->current_frame].flags&CV_FRAME_PDB || 
     cb->frame[cb->current_frame].flags&CV_FRAME_PREP){
    if(increase == KS_TRUE){
      if(cb->particle_size == CV_PARTICLE_SIZE_NUM-1){
	cb->particle_size = 0;
      } else {
	cb->particle_size++;
      }
    } else {
      if(cb->particle_size == 0){
	cb->particle_size = CV_PARTICLE_SIZE_NUM-1;
      } else {
	cb->particle_size--;
      }
    }
    cb->detail = cb->detail + prev_detail_small_diff + prev_detail_large_diff; // recover detail
    if(cb->detail < CV_SPHERE_DETAIL_MIN) cb->detail = CV_SPHERE_DETAIL_MIN;   // check min
    if(cb->detail > CV_SPHERE_DETAIL_MAX) cb->detail = CV_SPHERE_DETAIL_MAX;   // check max
    prev_detail = cb->detail;
    if(cb->particle_size == CV_PARTICLE_SIZE_SMALL){
      if(cb->detail > CV_SPHERE_DETAIL_MIN){
	cb->detail--;                                     // decrease detail for small sphere
      }
      prev_detail_small_diff = prev_detail - cb->detail;
      prev_detail_large_diff = 0;
    } else if(cb->particle_size == CV_PARTICLE_SIZE_LARGE){
      cb->detail *= 2;                                    // increase detail for small sphere
      if(cb->detail > CV_SPHERE_DETAIL_MAX){
	cb->detail = CV_SPHERE_DETAIL_MAX;
      }
      prev_detail_small_diff = 0;
      prev_detail_large_diff = prev_detail - cb->detail;
    } else {
      prev_detail_small_diff = 0;
      prev_detail_large_diff = 0;
    }
  } else if(cb->frame[cb->current_frame].flags&CV_FRAME_CDV){
    if(increase == KS_TRUE){
      cb->particle_size_scale += 0.1;
    } else if((int)(cb->particle_size_scale*10+.5) > 1){
      cb->particle_size_scale -= 0.1;
    }
  }
  if(cb->periodic_mode == KS_TRUE){
    set_periodic_frame(cb,&cb->frame[cb->current_frame]);
  }
#ifdef USE_GLSL
  set_shader_sphere_draw_info_size(cb,&cb->frame[cb->current_frame]);
  ks_update_gl_shader_sphere_draw_info_share(cb->sb->shader[SHADER_SPHERE],0,CV_SPHERE_DETAIL_CNT,
					     cb->sb->sphere_draw_info);
#endif
}
static void decrease_particle_size(KS_GL_BASE *gb, int x, int y, void *vp)
{
  change_particle_size((CV_BASE*)vp,KS_FALSE);
}
static void increase_particle_size(KS_GL_BASE *gb, int x, int y, void *vp)
{
  change_particle_size((CV_BASE*)vp,KS_TRUE);
}
static void draw_command_help(KS_GL_BASE *gb, int id, int width, int height, void *vp)
{
  CV_BASE *cb;
  int i;
  int x,y;
  int font_height;
  int len, key_len, key_len_max = 0;

  cb = (CV_BASE*)vp;

  ks_get_glut_font_height(gb,cb->command_help_font,&font_height);
  x = 0;
  y = height-font_height;

  for(i = 0; i < CV_COMMAND_NUM; i++){
    ks_get_glut_font_width(gb,cb->command_help_font,cb->command[i].key[CV_COMMAND_KEY_1],&len);
    key_len = len;
    ks_get_glut_font_width(gb,cb->command_help_font,cb->command[i].key[CV_COMMAND_KEY_2],&len);
    key_len += len;
    if(key_len_max < key_len) key_len_max = key_len;
  }
  ks_gl_base_foreground_color(gb);
  for(i = 0; i < CV_COMMAND_NUM; i++){
    glRasterPos2i(x,y);
    ks_draw_glut_text(gb,cb->command[i].key[CV_COMMAND_KEY_1],cb->command_help_font,0.0,0);
    ks_draw_glut_text(gb," / ",cb->command_help_font,0.0,0);
    ks_draw_glut_text(gb,cb->command[i].key[CV_COMMAND_KEY_2],cb->command_help_font,0.0,0);
    glRasterPos2i(key_len_max+25,y);
    ks_draw_glut_text(gb,cb->command[i].comment,cb->command_help_font,0.0,0);
    y -= font_height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION;;
  }
}
static void show_command_help(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;

  cb = (CV_BASE*)vp;
  if(cb->command_help_window_id < 0){
    int key_len,comment_len;
    int key_len_max = 0, comment_len_max = 0;
    int len;
    int width = 0;
    int height = 0;
    int sep = 30;
    ks_get_glut_font_height(gb,cb->command_help_font,&height);
    height *= CV_COMMAND_NUM*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION;
    for(i = 0; i < CV_COMMAND_NUM; i++){
      ks_get_glut_font_width(gb,cb->command_help_font,cb->command[i].key[CV_COMMAND_KEY_1],&len);
      key_len = len;
      ks_get_glut_font_width(gb,cb->command_help_font,cb->command[i].key[CV_COMMAND_KEY_2],&len);
      key_len += len;
      if(key_len_max < key_len) key_len_max = key_len;
      ks_get_glut_font_width(gb,cb->command_help_font,cb->command[i].comment,&comment_len);
      if(comment_len_max < comment_len) comment_len_max = comment_len;
      /*
      printf("%d %d '%s' '%s' '%s'\n",key_len_max,comment_len_max,
	     cb->command[i].key[CV_COMMAND_KEY_1],
	     cb->command[i].key[CV_COMMAND_KEY_2],
	     cb->command[i].comment);
      */
    }
    width = key_len_max+sep+comment_len_max;
    cb->command_help_window_id = ks_add_gl_base_window(gb,
						       KS_GL_WINDOW_SHOW|KS_GL_WINDOW_FRAME_FIX,
						       "Command Help",
						       30,
						       ks_get_gl_viewport_2d_height(gb)-
						       height-KS_GL_WINDOW_TITLE_BAR_SIZE-30,
						       width,height,
						       draw_command_help,NULL,NULL,NULL,cb);
  } else {
    static BOOL show = KS_TRUE;
    if(show == KS_TRUE){
      ks_hide_gl_base_window(gb,cb->command_help_window_id);
      show = KS_FALSE;
    } else {
      ks_show_gl_base_window(gb,cb->command_help_window_id);
      show = KS_TRUE;
    }
  }
}
static void change_particle_name(KS_GL_BASE *gb, char *text, void *vp)
{
  int i;
  CV_PARTICLE *p;
  char name[5];
  p = (CV_PARTICLE*)vp;
  for(i = 0; text[i]; i++){
    name[i] = text[i];
  }
  for(; i < 4; i++){
    name[i] = ' ';
  }
  name[4] = '\0';
  /*  printf("name '%s' -> '%s' '%s'\n",p->property->name,text,name);*/
  strcpy(p->property->name,name);
}
static void set_change_particle_name(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_PARTICLE *p;
  cb = (CV_BASE*)vp;

  if(ks_count_int_list(cb->selected_label) == 1){
    p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				      cb->selected_label->value);
    if(p->property != NULL){
      ks_set_gl_input_key(gb,"new name >",p->property->name,cb->input_key_font,
			  change_particle_name,KS_FALSE,p);
    }
  }
}
static void copy_position(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i;
  CV_BASE *cb;
  CV_PARTICLE *p0,*p1;

  cb = (CV_BASE*)vp;
  if(ks_count_int_list(cb->selected_label) == 2){
    add_move(gb,0,0,vp);
    p0 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       (unsigned int)cb->selected_label->value);
    p1 = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
				       (unsigned int)cb->selected_label->next->value);
    ks_assert(p0 != NULL);
    ks_assert(p1 != NULL);
    /*
    printf("%s %f %f %f\n",p0->property->name,p0->cd[0],p0->cd[1],p0->cd[2]);
    printf("%s %f %f %f\n",p1->property->name,p1->cd[0],p1->cd[1],p1->cd[2]);
    */
    for(i = 0; i < 3; i++){
      p0->move[i]  = p1->cd[i] - p0->cd[i];
    }
  }
}
static void add_additional_surface_transparency(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  if(fr->sur != NULL){
    if(fr->sur->transparency > 0.001){
      fr->sur->transparency -= 0.1;
    } else {
      fr->sur->transparency = 1.0;
    }
  }
}
static void toggle_draw_additional_surface(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  if(cb->draw_additional_surface == KS_TRUE){
    cb->draw_additional_surface = KS_FALSE;
  } else {
    cb->draw_additional_surface = KS_TRUE;
  }
}
static void set_atype(CV_BASE *cb, CV_FRAME *fr)
{
  CV_PARTICLE *p;
  CV_ATOM *atom;
  for(p = fr->particle; p != NULL; p = p->next){
    atom = lookup_cv_atom_atype(cb->atom,p->atype[cb->atype_mode]);
    /*
    color = ks_get_gl_atom_color(atom->gl_atom,KS_GL_ATOM_COLOR_ATOM,KS_GL_ATOM_BRIGHT_NORMAL);
    printf("%d %d %f %f %f\n"
	   ,p->label,p->atype[cb->atype_mode],color[0],color[1],color[2]);
    */
    if(atom != NULL){
      p->atom = atom;
    }
  }
}
static void change_atype(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(cb->atype_mode == cb->atype_mode_num-1){
    if(cb->continuous_color.num == 0){
      cb->atype_mode = 0;
    } else {
      if(cb->continuous_color.mode == cb->continuous_color.num-1){
	cb->atype_mode = 0;
	cb->continuous_color.mode = -1;
      } else {
	cb->continuous_color.mode++;
      }
    }
  } else {
    cb->atype_mode++;
  }
  fr->atype_mode = cb->atype_mode;
  /*
  printf("after  %d %d %d %d\n",cb->atype_mode,cb->atype_mode_num,
	 cb->continuous_color.mode,cb->continuous_color.num);
  */
  set_atype(cb,fr);

#ifdef USE_GLSL
  update_shader_sphere_draw_info(cb,&cb->frame[cb->current_frame]);
  update_shader_rod_draw_info(cb,&cb->frame[cb->current_frame]);
#endif
}
static void show_all_atype(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_ATOM *p;
  cb = (CV_BASE*)vp;
  for(p = cb->atom; p != NULL; p = p->next){
    p->flags |= CV_ATOM_SHOW;
  }
}
static void toggle_all_atype(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_ATOM *p;
  cb = (CV_BASE*)vp;
  for(p = cb->atom; p != NULL; p = p->next){
    if(p->flags&CV_ATOM_SHOW){
      p->flags &= ~CV_ATOM_SHOW;
    } else {
      p->flags |= CV_ATOM_SHOW;
    }
  }
}
static void toggle_dark_atype(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_ATOM *p;
  cb = (CV_BASE*)vp;
  for(p = cb->atom; p != NULL; p = p->next){
    if(p->atype >= CV_DARK_ATOM_TYPE_ADD){
      if(p->flags&CV_ATOM_SHOW){
	p->flags &= ~CV_ATOM_SHOW;
      } else {
	p->flags |= CV_ATOM_SHOW;
      }
    }
  }
}
static void adjust_size_and_depth_range(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  cb->reference_frame->system_size = cb->frame[cb->current_frame].system_size;
  cb->reference_frame->center[0] = cb->frame[cb->current_frame].center[0];
  cb->reference_frame->center[1] = cb->frame[cb->current_frame].center[1];
  cb->reference_frame->center[2] = cb->frame[cb->current_frame].center[2];
  cv_init_lookat(gb,cb);
  cv_adjust_depth_range(gb,cb);
}
void cv_set_glut_key(KS_GL_BASE *gb, CV_BASE *cb)
{
  /*
  ks_set_gl_key_func(gb,'i',0, &cb->color_mode,0,CV_COLOR_MODE_NUM-1, 1,NULL,NULL,
		       "change color mode (+)");
  ks_set_gl_key_func(gb,'I',0, &cb->color_mode,0,CV_COLOR_MODE_NUM-1,-1,NULL,NULL,
		       "change color mode (-)");
  */
  ks_set_gl_key_func(gb,'i',0, NULL,0,0,0,increase_color_mode,(void*)cb,
		     "change color mode (+)");
  ks_set_gl_key_func(gb,'I',0, NULL,0,0,0,decrease_color_mode,(void*)cb,
		     "change color mode (-)");
  /*
  ks_set_gl_key_func(gb,'e',0, &cb->draw_mode,0,CV_DRAW_MODE_NUM-1, 1,NULL,NULL,
		     "change draw mode (+)");
  ks_set_gl_key_func(gb,'E',0, &cb->draw_mode,0,CV_DRAW_MODE_NUM-1,-1,NULL,NULL,
		     "change draw mode (-)");
  */
  ks_set_gl_key_func(gb,'b',0, NULL,0,0,0,toggle_projection_mode,(void*)cb,
		     "toggle projection mode (perspective or ortho)");

  ks_set_gl_key_func(gb,'e',0, NULL,0,0,0,increase_draw_mode,(void*)cb,
		     "change draw mode (+)");
  ks_set_gl_key_func(gb,'E',0, NULL,0,0,0,decrease_draw_mode,(void*)cb,
		     "change draw mode (-)");
  ks_set_gl_key_func(gb,'w',0, &cb->draw_water_mode,0,CV_DRAW_WATER_MODE_NUM-1, 1,NULL,NULL,
		     "change draw water mode (+)");
  ks_set_gl_key_func(gb,'W',0, &cb->draw_water_mode,0,CV_DRAW_WATER_MODE_NUM-1,-1,NULL,NULL,
		     "change draw water mode (-)");
  /*
  ks_set_gl_key_func(gb,'r',0, &cb->particle_size,0,CV_PARTICLE_SIZE_NUM-1, 1,NULL,NULL,
		     "particle size (+)");
  ks_set_gl_key_func(gb,'R',0, &cb->particle_size,0,CV_PARTICLE_SIZE_NUM-1,-1,NULL,NULL,
		     "particle size (-)");
  */
  ks_set_gl_key_func(gb,'r',0, NULL,0,0,0,decrease_particle_size,(void*)cb,
		     "change particle size (-)");
  ks_set_gl_key_func(gb,'R',0, NULL,0,0,0,increase_particle_size,(void*)cb,
		     "change particle size (+)");
  ks_set_gl_key_func(gb,'l',0, &cb->label_mode,0,CV_LABEL_MODE_NUM-1, 1,NULL,NULL,
		     "show label mode (+)");
  ks_set_gl_key_func(gb,'L',0, &cb->label_mode,0,CV_LABEL_MODE_NUM-1,-1,NULL,NULL,
		     "show label mode (+)");
  ks_set_gl_key_func(gb,'k',0, &cb->kabe_mode,0,CV_KABE_MODE_NUM-1, 1,NULL,NULL,
		     "change kabe mode (+)");
  ks_set_gl_key_func(gb,'K',0, &cb->kabe_mode,0,CV_KABE_MODE_NUM-1,-1,NULL,NULL,
		     "change kabe mode (-)");
  ks_set_gl_key_func(gb,'g',0, &cb->show_selected_particle_mode,0,
		     CV_SHOW_SELECTED_PARTICLE_MODE_NUM-1, 1,NULL,NULL,
		     "change show selected particle mode (+)");

  ks_set_gl_key_func(gb,'y',0, NULL,0,0, 0,bool_fit_reference,(void*)cb,
		     "fit reference frame ON/OFF");
  ks_set_gl_key_func(gb,'Y',0, NULL,0,0, 0,bool_overlap_reference,(void*)cb,
		     "overlap reference frame ON/OFF");

  ks_set_gl_key_func(gb,'c',0, NULL,0,0, 0,increment_frame,(void*)cb,"increment frame");
  ks_set_gl_key_func(gb,'s',0, NULL,0,0, 0,last_frame,(void*)cb,"move last frame");
  ks_set_gl_key_func(gb,'C',0, NULL,0,0, 0,set_increment_frame_idle,(void*)cb,
		     "increment frame (continue)");
  ks_set_gl_key_func(gb,'S',0, NULL,0,0, 0,set_increment_frame_bmp_idle,(void*)cb,
		     "increment frame with output bmp (continue)");
  ks_set_gl_key_func(gb,'s',KS_GL_ACTIVE_ALT, NULL,0,0, 0,set_increment_frame_order_bmp_idle,
		     (void*)cb, "increment frame with output bmp as order file name(continue)");
  ks_set_gl_key_func(gb,'z',0, NULL,0,0, 0,decrement_frame,(void*)cb,"decrement frame");
  ks_set_gl_key_func(gb,'a',0, NULL,0,0, 0,first_frame,(void*)cb,"move first frame");
  ks_set_gl_key_func(gb,'Z',0, NULL,0,0, 0,set_decrement_frame_idle,(void*)cb,
		     "decrement frame (continue)");
  ks_set_gl_key_func(gb,'A',0, NULL,0,0, 0,set_decrement_frame_bmp_idle,(void*)cb,
		     "decrement frame with output bmp (continue)");
  ks_set_gl_key_func(gb,'a',KS_GL_ACTIVE_ALT, NULL,0,0, 0,set_decrement_frame_order_bmp_idle,
		     (void*)cb, "decrement frame with output bmp as order file mame(continue)");
  ks_set_gl_key_func(gb,'X',0, NULL,0,0, 0,reset_idle,(void*)cb,"stop frame");
  ks_set_gl_key_func(gb,'x',0, NULL,0,0, 0,reset_idle,(void*)cb,"stop frame");

  ks_set_gl_key_func(gb,'m',0, NULL,0,0, 0,move_mode,(void*)cb,"move mode");
  ks_set_gl_key_func(gb,'?',0, NULL,0,0, 0,undo_move,(void*)cb,"undo move");
  ks_set_gl_key_func(gb,'/',KS_GL_ACTIVE_ALT, NULL,0,0, 0,add_move,(void*)cb,"add move");

  ks_set_gl_key_func(gb,'u',0, NULL,0,0, 0,fv_increase_draw_contour_mode,(void*)cb,
		     "toggle draw contour");
  ks_set_gl_key_func(gb,'U',0, NULL,0,0, 0,fv_increase_draw_surface_mode,(void*)cb,
		     "charge surface mode");

  ks_set_gl_key_func(gb,'f',0, NULL,0,0, 0,set_find_particle,(void*)cb,"find particle");
  ks_set_gl_key_func(gb,'F',0, NULL,0,0, 0,set_find_residue,(void*)cb,"find residue");
  ks_set_gl_key_func(gb,'o',0, NULL,0,0, 0,set_output_file,(void*)cb,"output file");
  ks_set_gl_key_func(gb,'O',0, NULL,0,0, 0,set_output_bmp_pov_file,(void*)cb,"output BMP file");
  /*
  ks_set_gl_key_func(gb,'h',0, NULL,0,0, 0,set_make_bond,(void*)cb,"make bond");
  */
  ks_set_gl_key_func(gb,'j',0, NULL,0,0, 0,look_selected_particle,(void*)cb,
		     "look selected particle");
  ks_set_gl_key_func(gb,'J',0, &cb->axis_mode,0,CV_AXIS_MODE_NUM-1, 1,NULL,NULL,"toggle axis");

  ks_set_gl_key_func(gb,'/',0, NULL,0,0, 0,set_command_mode,(void*)cb,"command mode");
  ks_set_gl_key_func(gb,'>',0, NULL,0,0, 0,increment_selected_residue,(void*)cb,
		     "increment selected residue");
  ks_set_gl_key_func(gb,'<',0, NULL,0,0, 0,decrement_selected_residue,(void*)cb,
		     "decrement selected residue");
  ks_set_gl_key_func(gb,'[',0, NULL,0,0, 0,change_background_color,(void*)cb,
		     "charge background color (black / white)");
  ks_set_gl_key_func(gb,' ',0, NULL,0,0, 0,change_atype,(void*)cb,"change atom type");
  ks_set_gl_key_func(gb,' ',KS_GL_ACTIVE_SHIFT, NULL,0,0, 0,fv_change_target_column,(void*)cb,
		     "change fdview data column");
  ks_set_gl_key_func(gb,'0',0, NULL,0,0, 0,show_all_atype,(void*)cb,"show all atom type");
  ks_set_gl_key_func(gb,'-',0, NULL,0,0, 0,toggle_all_atype,(void*)cb,"toggle all atom type");
  ks_set_gl_key_func(gb,'+',0, NULL,0,0, 0,toggle_dark_atype,(void*)cb,"toggle dark atom type");
  ks_set_gl_key_func(gb,'p',0, NULL,0,0, 0,set_periodic_mode,(void*)cb,"toggle periodic mode");
  ks_set_gl_key_func(gb,0x7f,0, NULL,0,0, 0,set_del_particle,(void*)cb,
		     "delete selected particle");
  ks_set_gl_key_func(gb,'d',0, NULL,0,0, 0,decrease_detail,(void*)cb,"decrease particle detail");
  ks_set_gl_key_func(gb,'D',0, NULL,0,0, 0,increase_detail,(void*)cb,"increase particle detail");
  ks_set_gl_key_func(gb,'v',0, NULL,0,0, 0,ks_decrease_gl_base_verbose_level,NULL,
		     "decrease verbose level");
  ks_set_gl_key_func(gb,'V',0, NULL,0,0, 0,ks_increase_gl_base_verbose_level,NULL,
		     "increase verbose level");

  ks_set_gl_key_func(gb,'n',0, NULL,0,0, 0,set_change_particle_name,(void*)cb,
		     "change particle name");

  ks_set_gl_key_func(gb,'t',0, NULL,0,0, 0,copy_position,(void*)cb,
		     "copy position of selected particle");

  ks_set_gl_key_func(gb,'T',0, NULL,0,0, 0,calc_contour_area,(void*)cb,
		     "calcuate are of contour surface");

  ks_set_gl_key_func(gb,'h',0, NULL,0,0, 0,toggle_draw_additional_surface,(void*)cb,
		     "toggle additional surface");
  ks_set_gl_key_func(gb,'H',0, NULL,0,0, 0,add_additional_surface_transparency,(void*)cb,
		     "add additional surface transparency");
  /*
  ks_print_gl_key_func(gb);
  ks_exit(EXIT_FAILURE);
  */
  ks_set_gl_base_key_help(gb,KS_GL_KEY_F1,KS_GL_FONT_HELVETICA_10);
  ks_set_gl_special_key_func(gb,KS_GL_KEY_F2,0, NULL,0,0, 0,show_command_help,(void*)cb,
			     "show command help");

  ks_set_gl_special_key_func(gb,KS_GL_KEY_F9,0, &cb->prep_change_mode,
			     0,CV_PREP_CHANGE_NUM-1, 1,NULL,NULL,
			     "change particle order mode");

  ks_set_gl_special_key_func(gb,KS_GL_KEY_UP,0, NULL,0,0, 0,fv_move_backward_contour_plane,
			     (void*)cb,"move backward contour plane");
  ks_set_gl_special_key_func(gb,KS_GL_KEY_DOWN,0, NULL,0,0, 0,fv_move_forward_contour_plane,
			     (void*)cb,"move forward contour plane");
  ks_set_gl_special_key_func(gb,KS_GL_KEY_RIGHT,0, NULL,0,0, 0,fv_increase_target_surface,
			     (void*)cb,"increase target surface");
  ks_set_gl_special_key_func(gb,KS_GL_KEY_LEFT,0, NULL,0,0, 0,fv_decrease_target_surface,
			     (void*)cb,"decrease target surface");
  ks_set_gl_special_key_func(gb,KS_GL_KEY_HOME,0, NULL,0,0, 0,adjust_size_and_depth_range,
			     (void*)cb,"adjust system size and depth range");

  ks_set_gl_key_func(gb,',',0, NULL,0,0, 0,print_residue,(void*)cb,"print residue");
  ks_set_gl_key_func(gb,'.',0, NULL,0,0, 0,print_particle,(void*)cb,"print particle");

  ks_set_gl_base_verbose_level(gb,4);
  ks_set_gl_base_verbose_level_max(gb,4);
}
void cv_draw_color_sample(KS_GL_BASE *gb, CV_BASE *cb, int width, int height)
{
  int len;
  GLfloat x,y;
  int font = KS_GL_FONT_HELVETICA_12;
  static char *charge_color_mode_name = {"Charge"};
  char *color_mode_name;

  if(cb->color_sample[cb->color_mode] != NULL){
    x = width - 25;
    y = 240;

    if(cb->color_mode_name[cb->color_mode][0] != '\0'){
      if(cb->frame[cb->current_frame].prep_block != NULL &&
	 cb->color_mode == CV_COLOR_MODE_RAINBOW){
	color_mode_name = charge_color_mode_name;
      } else {
	color_mode_name = cb->color_mode_name[cb->color_mode];
      }
      ks_gl_base_foreground_color(gb);
      ks_get_glut_font_width(gb,font,color_mode_name,&len);
      glRasterPos2i(x-len+10,y+15);
      ks_use_glut_font(gb,font);
      glCallLists(strlen(color_mode_name), GL_BYTE, color_mode_name);
    }
    ks_draw_gl_color_sample_right(gb, x,y,
				  cb->color_sample[cb->color_mode],
				  font,
				  11, 20, 10, cb->sphere_texture);
  }
}
static void set_select_chain(CV_CHAIN *pc, BOOL select)
{
  CV_RESIDUE *pr;
  if(select == KS_TRUE){
    pc->flags |= CV_CHAIN_SELECTED;
  } else {
    pc->flags &= ~CV_CHAIN_SELECTED;
  }
  for(pr = pc->residue; pr != NULL; pr = pr->next){
    set_select_residue(pr,select);
  }
}
static void set_select_aggregate(CV_AGGREGATE *pa, BOOL select)
{
  CV_CHAIN *pc;
  if(select == KS_TRUE){
    pa->flags |= CV_AGGREGATE_SELECTED;
  } else {
    pa->flags &= ~CV_AGGREGATE_SELECTED;
  }
  for(pc = pa->chain; pc != NULL; pc = pc->next){
    set_select_chain(pc,select);
  }
}
static void __KS_USED__ set_select_double(CV_PARTICLE *p, int hit_type)
{
  printf("double click\n");
  if(p->property->aggregate->flags&CV_AGGREGATE_SELECTED){
    printf("???\n");
  } else if(p->property->chain->flags&CV_CHAIN_SELECTED){
    printf("agg\n");
    set_select_aggregate(p->property->aggregate,KS_TRUE);
  } else if(p->property->residue->flags&CV_RESIDUE_SELECTED){
    printf("chain\n");
    set_select_chain(p->property->chain,KS_TRUE);
  } else {
    printf("residue\n");
    set_select_residue(p->property->residue,KS_TRUE);
  }
}
static void __KS_USED__ unset_select(CV_PARTICLE *p)
{
  /*  printf("unset\n");*/
  if(p->property != NULL){
    if(p->property->aggregate->flags&CV_AGGREGATE_SELECTED){
      printf("unset agg\n");
      set_select_aggregate(p->property->aggregate,KS_FALSE);
    } else if(p->property->chain->flags&CV_CHAIN_SELECTED){
      printf("unset chain\n");
      set_select_chain(p->property->chain,KS_FALSE);
    } else if(p->property->residue->flags&CV_RESIDUE_SELECTED){
      printf("unset residue\n");
      set_select_residue(p->property->residue,KS_FALSE);
    } else {
      /*      printf("unset else\n");*/
    }
  }
}
static void set_select_single(CV_PARTICLE *p, BOOL select, int hit_type)
{
  if(select == KS_TRUE){
    set_selected_for_particle(p);
  } else {
    unset_selected_for_particle(p);
  }
}
static void __KS_USED__ hit_first(CV_PARTICLE **particle_hash, int particle_hash_size, 
				  KS_INT_LIST **selected_label, int hit_name, int hit_type)
{
  CV_PARTICLE *p;
  
  p = lookup_cv_particle_hash_label(particle_hash,particle_hash_size,
				    (*selected_label)->value);
  if(p != NULL){ 
    set_select_single(p,KS_FALSE,hit_type);
    /*    p->flags &= ~CV_PARTICLE_SELECTED;*/
    /*    p->property->residue->flags &= ~CV_RESIDUE_SELECTED;*/
  }
  if((*selected_label)->value == hit_name){
    (*selected_label) = ks_del_int_list((*selected_label),hit_name);
  } else {
    (*selected_label)->value = hit_name;
  }
}
static void __KS_USED__ hit_second(CV_PARTICLE **particle_hash, int particle_hash_size, 
				   KS_INT_LIST **selected_label, int hit_name, int hit_type)
{
  CV_PARTICLE *p;
  
  p = lookup_cv_particle_hash_label(particle_hash,particle_hash_size,
				    (*selected_label)->next->value);
  if(p != NULL){ 
    set_select_single(p,KS_FALSE,hit_type);
    /*    p->flags &= ~CV_PARTICLE_SELECTED;*/
    /*    p->property->residue->flags &= ~CV_RESIDUE_SELECTED;*/
  }
  if((*selected_label)->next->value == hit_name){
    (*selected_label) = ks_del_int_list((*selected_label),hit_name);
  } else {
    (*selected_label)->next->value = hit_name;
  }
}
static void __KS_USED__  hit_clear(CV_PARTICLE **particle_hash, int particle_hash_size, 
				   KS_INT_LIST **selected_label, int hit_name, int hit_type)
{
  CV_PARTICLE *p;
  KS_INT_LIST *il;

  for(il = *selected_label; il != NULL; il = il->next){
    p = lookup_cv_particle_hash_label(particle_hash,particle_hash_size,il->value);
    if(p != NULL){ 
      set_select_single(p,KS_FALSE,hit_type);
      /*      p->flags &= ~CV_PARTICLE_SELECTED;*/
      /*      p->property->residue->flags &= ~CV_RESIDUE_SELECTED;*/
    }
  }
  (*selected_label) = ks_del_int_list((*selected_label),(*selected_label)->next->value);
  (*selected_label)->value = hit_name;
}
static void clear_selected_label(KS_INT_LIST **selected_label, CV_PARTICLE **hash, int hash_size)
{
  /*
  KS_INT_LIST *il;
  CV_PARTICLE *p;
  for(il = (*selected_label); il != NULL; il = il->next){
    p = lookup_cv_particle_hash_label(hash,hash_size, il->value);
    if(p != NULL){
      p->flags &= ~CV_PARTICLE_SELECTED;
    }
  }
  */
  set_selected_label(*selected_label,hash,hash_size,KS_FALSE);
  ks_free_int_list((*selected_label));
  (*selected_label) = NULL;
}
static void del_selected_label(unsigned int label, KS_INT_LIST **selected_label,
			       CV_PARTICLE **hash, int hash_size)
{
  CV_PARTICLE *p;
  p = lookup_cv_particle_hash_label(hash,hash_size,label);
  (*selected_label) = ks_del_int_list((*selected_label),label);
  if(p != NULL){
    unset_selected_for_particle(p);
  }
}
static void select_particle(KS_GL_BASE *gb, CV_BASE *cb)
{
  CV_PARTICLE *p;
  int selected_num;

  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,gb->hit_name[1]);
  if(p != NULL){
    if(cb->look_selected_particle == KS_TRUE){
      cv_set_lookat(gb,cb,p->cd);
    }
    if(ks_lookup_int_list(cb->selected_label,gb->hit_name[1]) != NULL){
      del_selected_label(gb->hit_name[1],
			 &cb->selected_label,cb->particle_hash,cb->particle_hash_size);
    } else if(p->property == NULL || (!(p->property->chain->flags&CV_CHAIN_SELECTED))){
      selected_num = ks_count_int_list(cb->selected_label);
      if(selected_num <= CV_SELECT_PARTICLE_MAX){
	if(ks_get_glut_shift(gb) == KS_FALSE){
	  clear_selected_label(&cb->selected_label,cb->particle_hash,cb->particle_hash_size);
	} else if(selected_num == CV_SELECT_PARTICLE_MAX){
	  ks_assert(cb->selected_label != NULL);
	  ks_assert(cb->selected_label->next != NULL);
	  del_selected_label(cb->selected_label->value,
			     &cb->selected_label,cb->particle_hash,cb->particle_hash_size);
	}
	set_selected_for_particle(p);
	cb->selected_label=ks_add_int_list(cb->selected_label,ks_new_int_list(gb->hit_name[1]));
      }
    }

  }
}
static void set_select_residue(CV_RESIDUE *p, BOOL select)
{
  int i;
  if(select == KS_TRUE){
    p->flags |= CV_RESIDUE_SELECTED;
  } else {
    p->flags &= ~CV_RESIDUE_SELECTED;
  }
  for(i = 0; i < p->particle_num; i++){
    if(select == KS_TRUE){
      set_selected_for_particle(p->particle[i]);
    } else {
      unset_selected_for_particle(p->particle[i]);
    }
  }
}
static void clear_selected_residue_label(KS_INT_LIST **selected_label, 
					 CV_RESIDUE **hash, int hash_size)
{
  KS_INT_LIST *il;
  CV_RESIDUE *p;
  for(il = (*selected_label); il != NULL; il = il->next){
    p = lookup_cv_residue_hash_label(hash,hash_size, il->value);
    if(p != NULL){
      set_select_residue(p,KS_FALSE);
    }
  }
  ks_free_int_list((*selected_label));
  (*selected_label) = NULL;
}
static void del_selected_residue_label(unsigned int label, KS_INT_LIST **selected_label, 
				       CV_RESIDUE **hash, int hash_size)
{
  CV_RESIDUE *p;
  p = lookup_cv_residue_hash_label(hash,hash_size,label);
  (*selected_label) = ks_del_int_list((*selected_label),label);
  if(p != NULL){
    set_select_residue(p,KS_FALSE);
  }
}
static void select_residue(KS_GL_BASE *gb, CV_BASE *cb, BOOL select_chain)
{
  KS_INT_LIST *il;
  CV_PARTICLE *p;
  CV_RESIDUE *pr,*pr_start,*pr_end;
  unsigned int label;

  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,gb->hit_name[1]);
  if(p != NULL && p->property != NULL && p->property->residue != NULL){
    clear_selected_label(&cb->selected_label,cb->particle_hash,cb->particle_hash_size);
    pr = p->property->residue;
    label = pr->label;
    /*
    printf("before:");
    for(il = cb->selected_residue_label; il != NULL; il = il->next){
      printf(" %d",il->value);
    }
    printf("\n");
    */
    if(select_chain == KS_TRUE){
      clear_selected_residue_label(&cb->selected_residue_label,
				   cb->residue_hash,cb->residue_hash_size);
      if(p->property->chain->flags&CV_CHAIN_SELECTED){
	set_select_chain(p->property->chain,KS_FALSE);
      } else {
	set_select_chain(p->property->chain,KS_TRUE);
      }
    } else if(ks_lookup_int_list(cb->selected_residue_label,label) != NULL){
      del_selected_residue_label(label,
				 &cb->selected_residue_label,
				 cb->residue_hash,cb->residue_hash_size);
    } else if(!(p->property->chain->flags&CV_CHAIN_SELECTED)){
      if(ks_get_glut_shift(gb) == KS_FALSE || cb->selected_residue_label == NULL){
	clear_selected_residue_label(&cb->selected_residue_label,
				     cb->residue_hash,cb->residue_hash_size);
	set_select_residue(pr,KS_TRUE);
	cb->selected_residue_label=ks_add_int_list(cb->selected_residue_label,
						   ks_new_int_list(label));
      } else {
	for(pr_end = pr; pr_end != NULL; pr_end = pr_end->next){
	  if(ks_lookup_int_list(cb->selected_residue_label,pr_end->label) != NULL){
	    /*	    printf("hit %d\n",pr_end->label);*/
	    break;
	  }
	}
	/*	printf("pr_end %p\n",pr_end);*/
	if(pr_end == NULL){
	  pr_start = NULL;
	  pr_end = pr->next;
	  for(pr = p->property->chain->residue; pr != NULL; pr = pr->next)
	    if(pr->flags&CV_RESIDUE_SELECTED) pr_start = pr;
	  for(pr = pr_start->next;pr != pr_end && pr != NULL; pr = pr->next){
	    set_select_residue(pr,KS_TRUE);
	    cb->selected_residue_label=ks_addend_int_list(cb->selected_residue_label,
							  ks_new_int_list(pr->label));
	  }
	} else {
	  /*	  printf("start %d end %d\n",pr->label,pr_end->label);*/
	  il = NULL;
	  for(;pr != pr_end && pr != NULL; pr = pr->next){
	    set_select_residue(pr,KS_TRUE);
	    il = ks_addend_int_list(il,ks_new_int_list(pr->label));
	  }
	  cb->selected_residue_label=ks_cat_int_list(cb->selected_residue_label,il);
	  il = NULL;
	}
      }
    }
    /*
    printf("after:");
    for(il = cb->selected_residue_label; il != NULL; il = il->next){
      printf(" %d",il->value);
    }
    printf("\n");
    */
  }
}
static void clear_move_selected_label(KS_INT_LIST **selected_label, 
				      CV_PARTICLE **hash, int hash_size)
{
  KS_INT_LIST *il;
  CV_PARTICLE *p;
  for(il = (*selected_label); il != NULL; il = il->next){
    p = lookup_cv_particle_hash_label(hash,hash_size, il->value);
    if(p != NULL && (p->property == NULL || 
		     (!(p->property->residue->flags&CV_RESIDUE_SELECTED) &&
		      !(p->property->chain->flags&CV_CHAIN_SELECTED)))){
      unset_selected_for_particle(p);
    }
  }
  ks_free_int_list((*selected_label));
  (*selected_label) = NULL;
}
static BOOL change_particle_order(CV_FRAME *fr, CV_PARTICLE *p0)
{
  int i;
  CV_PARTICLE *p1;
  CV_PARTICLE *p0_sw_prev,*p0_sw_next;
  CV_PARTICLE *p1_sw_prev,*p1_sw_next;
  /*
  for(p = fr->particle_end; p != NULL; p = p->prev){
    printf("%d %s %f %f %f\n"
	   ,p->label,p->property->name,p->cd[0],p->cd[1],p->cd[2]);
  }
  */
  for(p1 = fr->particle_end, i = 1; p1 != NULL; p1 = p1->prev,i++){
    if(i == change_particle_order_target){
      break;
    }
  }
  if(p0 != p1 && p1 != NULL){
    printf("chage %d %s <-> %d %s\n",p0->label,p0->property->name,p1->label,p1->property->name);
    if(p0->next == p1){
      ks_assert(p1->prev = p0);
      p0->next = p1->next;
      p1->prev = p0->prev;
      p0->prev = p1;
      p1->next = p0;
      if(p0->prev == NULL){
	fr->particle = p0;
      }
      if(p0->next == NULL){
	fr->particle_end = p0;
      } else {
	p0->next->prev = p0;
      }
      if(p1->prev == NULL){
	fr->particle = p1;
      } else {
	p1->prev->next = p1;
      }
      if(p1->next == NULL){
	fr->particle_end = p1;
      }
    } else if(p1->next == p0){
      ks_assert(p0->prev = p1);
      p1->next = p0->next;
      p0->prev = p1->prev;
      p1->prev = p0;
      p0->next = p1;
      if(p0->prev == NULL){
	fr->particle = p0;
      } else {
	p0->prev->next = p0;
      }
      if(p0->next == NULL){
	fr->particle_end = p0;
      }
      if(p1->prev == NULL){
	fr->particle = p1;
      }
      if(p1->next == NULL){
	fr->particle_end = p1;
      } else {
	p1->next->prev = p1;
      }
    } else {
      p0_sw_prev = p0->prev;
      p0_sw_next = p0->next;
      p1_sw_prev = p1->prev;
      p1_sw_next = p1->next;
      p0->prev = p1_sw_prev;
      p0->next = p1_sw_next;
      p1->prev = p0_sw_prev;
      p1->next = p0_sw_next;
      if(p1_sw_prev == NULL){
	fr->particle = p0;
      } else {
	p1_sw_prev->next = p0;
      }
      if(p1_sw_next == NULL){
	fr->particle_end = p0;
      } else {
	p1_sw_next->prev = p0;
      }
      if(p0_sw_prev == NULL){
	fr->particle = p1;
      } else {
	p0_sw_prev->next = p1;
      }
      if(p0_sw_next == NULL){
	fr->particle_end = p1;
      } else {
	p0_sw_next->prev = p1;
      }
    }
  } else {
    return KS_FALSE;
  }
  /*
  for(p = fr->particle_end; p != NULL; p = p->prev){
    printf("%d %s %f %f %f\n"
	   ,p->label,p->property->name,p->cd[0],p->cd[1],p->cd[2]);
  }
  */
  return KS_TRUE;
}
void cv_process_mouse(KS_GL_BASE *gb, CV_BASE *cb, int button, int state, int x, int y)
{
  CV_FRAME *fr;
  CV_PARTICLE *p;
  BOOL double_click,triple_click;
#ifdef USE_GLSL
  BOOL pick_enable;
#endif
  static BOOL double_click0;

  fr = &cb->frame[cb->current_frame];

#ifdef USE_GLSL
  pick_enable = ks_gl_base_mouse(gb,(void*)cb,button,state,x,y,display_3d);
  if(pick_enable == KS_TRUE){
    unsigned int pick_id;
    // pick mode ON
    ks_set_gl_shader_color_mode_all(cb->sb->shader,SHADER_TYPE_CNT,KS_GL_SHADER_COLOR_MODE_PICK);
    // draw in pick mode
    cv_process_display(gb,cb);
    // pick mode OFF
    ks_set_gl_shader_color_mode_all(cb->sb->shader,SHADER_TYPE_CNT,
				    KS_GL_SHADER_COLOR_MODE_LIGHTING);
    // get pick id
    pick_id = ks_get_gl_shader_pick_id(gb->window_size[KS_GL_WINDOW_SIZE_HEIGHT],x,y);
    process_shader_pick(gb,cb->sb,pick_id);
  } else {
    gb->hit_name_num = 0;
  }
#endif
  /*
  if(button == KS_GL_MOUSE_BUTTON_LEFT && state == KS_GL_MOUSE_STATE_UP){
    printf("%f\n"
	   ,gb->mouse_time[KS_GL_MOUSE_TIME_NOW][KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP]
	   -gb->mouse_time[KS_GL_MOUSE_TIME_PREV][KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP]);
  }
  */
  /*
  printf("gb->hit_num %d %s (%d %d) (%d %d)\n",gb->hit_name_num,
	 state == KS_GL_MOUSE_STATE_UP ? "UP":
	 state == KS_GL_MOUSE_STATE_DOWN ? "DOWN":"X",
	 gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_DOWN][0],
	 gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_DOWN][1],
	 gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP][0],
	 gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP][1]);
  */
#if 1

  if(button == KS_GL_MOUSE_BUTTON_LEFT){
    if((is_shown_fv_map(gb,fr) == KS_TRUE && 
	(state == KS_GL_MOUSE_STATE_DOWN && fv_hit_color_map_xy(x,y))) ||
       (state == KS_GL_MOUSE_STATE_UP && cb->fv_color_map_select == KS_TRUE)){ // check later !!
      if(state == KS_GL_MOUSE_STATE_DOWN && fv_hit_color_map_xy(x,y)){
	int level = fv_get_color_map_num(x,y,fr->fv->contour_size);
	//	printf("left down %d %d\n",level,fr->fv->contour_size.num);
	if(level < fr->fv->contour_size.num){
	  cb->fv_target.level[level] = cb->fv_target.level[level] == 1 ? 0:1;
	  gb->mouse_angle_change = KS_FALSE;
	  cb->fv_color_map_select = KS_TRUE;
	}
      } else if(state == KS_GL_MOUSE_STATE_UP){
	/*
	if(fv_hit_color_map_y(y)){
	  printf("left up %d\n",fv_get_color_map_num(x,y,fr->fv->contour_size));
	} else {
	  printf("left up X\n");
	}
	*/
	gb->mouse_angle_change = KS_TRUE;
	cb->fv_color_map_select = KS_FALSE;
      }
    } else if(gb->hit_name_num != 0){
      if(cb->move_mode == KS_TRUE){
	if(state == KS_GL_MOUSE_STATE_DOWN){
	  /*	printf("mouse %d %d\n",gb->hit_name_num, gb->hit_name[0]);*/
	  if(gb->hit_name[0] == CV_NAME_PARTICLE){
	    p=lookup_cv_particle_hash_label(cb->particle_hash,
					    cb->particle_hash_size,gb->hit_name[1]);
	    if(p != NULL){
	      ks_set_gl_move_target_motion(gb,KS_TRUE);
	      ks_set_gl_move_target(gb,cb->move_add,p->cd,1.0);
	      ks_set_gl_rotation_center(gb,p->cd);
	      clear_move_selected_label(&cb->selected_label,cb->particle_hash,
					cb->particle_hash_size);

	      set_selected_for_particle(p);
	      cb->selected_label=
		ks_add_int_list(cb->selected_label,ks_new_int_list(gb->hit_name[1]));

	      add_move(gb,0,0,(void*)cb);
	    }
	  }
	} else {
	  ks_set_gl_move_target_motion(gb,KS_FALSE);
	}
	/*
	  } else if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_ORDER &&
	  state == KS_GL_MOUSE_STATE_UP && 
	  gb->mouse_pos[button][KS_GL_MOUSE_STATE_DOWN][0] == 
	  gb->mouse_pos[button][KS_GL_MOUSE_STATE_UP][0]      &&
	  gb->mouse_pos[button][KS_GL_MOUSE_STATE_DOWN][1] == 
	  gb->mouse_pos[button][KS_GL_MOUSE_STATE_UP][1]){
	*/
      } else if(state == KS_GL_MOUSE_STATE_UP && 
		gb->mouse_pos[button][KS_GL_MOUSE_STATE_DOWN][0] == 
		gb->mouse_pos[button][KS_GL_MOUSE_STATE_UP][0]      &&
		gb->mouse_pos[button][KS_GL_MOUSE_STATE_DOWN][1] == 
		gb->mouse_pos[button][KS_GL_MOUSE_STATE_UP][1]){
	if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_ORDER){
	  if(gb->hit_name[0] == CV_NAME_PARTICLE){
	    p=lookup_cv_particle_hash_label(cb->particle_hash,
					    cb->particle_hash_size,gb->hit_name[1]);
	    if(p != NULL){
	      if(change_particle_order(fr,p) == KS_TRUE){
		cv_process_special_keyboard(gb,cb,KS_GL_KEY_PAGE_DOWN,0,0);
	      }
	    }
	  }
	} else if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_MAIN){
	  if(gb->hit_name[0] == CV_NAME_PARTICLE){
	    p=lookup_cv_particle_hash_label(cb->particle_hash,
					    cb->particle_hash_size,gb->hit_name[1]);
	    if(p != NULL){
	      p->flags ^= CV_PARTICLE_MAIN;
	    }
	  }
	} else {
	  double_click = ks_is_gl_double_click(gb,KS_GL_MOUSE_BUTTON_LEFT,KS_GL_MOUSE_STATE_UP);
	  if(double_click0 == KS_TRUE && double_click == KS_TRUE){
	    triple_click = KS_TRUE;
	  } else {
	    triple_click = KS_FALSE;
	  }
	  /*
	    printf("%d %d %s\n",
	    gb->hit_name[0],gb->hit_name[1],double_click==KS_TRUE?"double":"single");
	  */
	  if(gb->hit_name[0] == CV_NAME_PARTICLE){
	    if(double_click == KS_TRUE){
	      select_residue(gb,cb,triple_click);
	    } else if(cb->selected_residue_label == NULL){
	      select_particle(gb,cb);
	    }
	  } else if(gb->hit_name[0] == CV_NAME_RIBBON){
	    select_residue(gb,cb,double_click);
	  }
	  double_click0 = double_click;
	}
      }
#ifdef USE_GLSL
      // update draw_info to change emissions of selected particles
      ks_update_gl_shader_sphere_draw_info_share(cb->sb->shader[SHADER_SPHERE],0,
						 CV_SPHERE_DETAIL_CNT,cb->sb->sphere_draw_info);
#endif
    }
  }
  if(button == KS_GL_MOUSE_BUTTON_RIGHT &&
     state == KS_GL_MOUSE_STATE_UP && 
     gb->mouse_pos[button][KS_GL_MOUSE_STATE_DOWN][0] == 
     gb->mouse_pos[button][KS_GL_MOUSE_STATE_UP][0]      &&
     gb->mouse_pos[button][KS_GL_MOUSE_STATE_DOWN][1] == 
     gb->mouse_pos[button][KS_GL_MOUSE_STATE_UP][1]){
    clear_selected_label(&cb->selected_label,cb->particle_hash,cb->particle_hash_size);
    clear_selected_residue_label(&cb->selected_residue_label,
				 cb->residue_hash,cb->residue_hash_size);
    {
      CV_AGGREGATE *pa;
      CV_CHAIN *pc;
      CV_RESIDUE *pr;
      for(pa = fr->aggregate; pa != NULL; pa = pa->next){
	for(pc = pa->chain; pc != NULL; pc = pc->next){
	  set_select_chain(pc,KS_FALSE);
	  for(pr = pc->residue; pr != NULL; pr = pr->next){
	    set_select_residue(pr,KS_FALSE);
	  }
	}
      }
    }
#ifdef USE_GLSL
    // update draw_info to change emissions of unselected particles
    ks_update_gl_shader_sphere_draw_info_share(cb->sb->shader[SHADER_SPHERE],0,
					       CV_SPHERE_DETAIL_CNT,cb->sb->sphere_draw_info);
#endif
  }
#else
  if(gb->hit_name_num != 0  && button == KS_GL_MOUSE_BUTTON_LEFT){
    if(cb->move_mode == KS_TRUE){
      if(state == KS_GL_MOUSE_STATE_DOWN){
	/*	printf("%d %d\n",gb->hit_name_num, gb->hit_name[0]);*/
	if(gb->hit_name[0] == CV_NAME_PARTICLE){
	  p=lookup_cv_particle_hash_label(cb->particle_hash,
					  cb->particle_hash_size,gb->hit_name[1]);
	  if(p != NULL){
	    ks_set_gl_move_target_motion(gb,KS_TRUE);
	    set_selected_for_particle(p);
	    ks_set_gl_move_target(gb,p->add,p->cd);
	    /*	    ks_set_gl_rotation_center(gb,p->cd);*/
	    if(cb->selected_label == NULL){                /* no selected */
	      cb->selected_label=ks_add_int_list(cb->selected_label,
						 ks_new_int_list(gb->hit_name[1]));
	    } else if(cb->selected_label->next == NULL){   /* one selected */

	      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
						cb->selected_label->value);
	      if(cb->selected_label->value != gb->hit_name[1]){
		if(p != NULL){ 
		  unset_selected_for_particle(p);
		}
		cb->selected_label->value = gb->hit_name[1];
	      }
	    }
	  }
	}
      } else {
	ks_set_gl_move_target_motion(gb,KS_FALSE);
      }
    } else if(state == KS_GL_MOUSE_STATE_UP && 
       gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_DOWN][0] == 
       gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP][0]      &&
       gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_DOWN][1] == 
       gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP][1]){
      /*      printf("%d %d\n",gb->hit_name[0],gb->hit_name[1]);*/
      if(gb->hit_name[0] == CV_NAME_PARTICLE || gb->hit_name[0] == CV_NAME_RIBBON){
	//      printf("%d\n",gb->hit_name[1]);
	/*
	printf("%d\n",);
	*/
	double_click = ks_is_gl_double_click(gb,KS_GL_MOUSE_BUTTON_LEFT,KS_GL_MOUSE_STATE_UP);
	p=lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,gb->hit_name[1]);
	if(p != NULL){
	  /*	printf("%d %s %d\n",p->label,p->atom->name,cb->selected_label);*/
	  if(double_click == KS_FALSE){
	    unset_select(p);
	    /*	    change_lookat(cb,p->cd,2);*/
	    set_select_single(p,KS_TRUE,gb->hit_name[0]);
	    if(cb->selected_label == NULL){                /* no selected */
	      cb->selected_label=ks_add_int_list(cb->selected_label,
						 ks_new_int_list(gb->hit_name[1]));
	    } else if(cb->selected_label->next == NULL){   /* one selected */
	      if(ks_get_glut_shift(gb) == KS_TRUE){ /* shift */
		cb->selected_label=
		  ks_add_int_list(cb->selected_label,ks_new_int_list(gb->hit_name[1]));
	      } else {
		hit_first(cb->particle_hash,cb->particle_hash_size,&cb->selected_label,
			  gb->hit_name[1],gb->hit_name[0]);
	      }
	    } else if(cb->selected_label->next->next == NULL){   /* two selected */
	      if(ks_get_glut_shift(gb) == KS_TRUE){ /* shift */
		if(gb->hit_name[1] == cb->selected_label->next->value){
		  hit_second(cb->particle_hash,cb->particle_hash_size,&cb->selected_label,
			     gb->hit_name[1],gb->hit_name[0]);
		} else {
		  hit_first(cb->particle_hash,cb->particle_hash_size,&cb->selected_label,
			    gb->hit_name[1],gb->hit_name[0]);
		}
	      } else { /* no shift */
		if(gb->hit_name[1] == cb->selected_label->value){
		  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
						    cb->selected_label->next->value);
		  if(p != NULL){ 
		    set_select_single(p,KS_FALSE,gb->hit_name[0]);
		  }
		  cb->selected_label = ks_del_int_list(cb->selected_label,
						       cb->selected_label->next->value);
		} else if(gb->hit_name[1] == cb->selected_label->next->value){
		  p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
						    cb->selected_label->value);
		  if(p != NULL){ 
		    set_select_single(p,KS_FALSE,gb->hit_name[0]);
		  }
		  cb->selected_label = ks_del_int_list(cb->selected_label,
						       cb->selected_label->value);
		} else {
		  hit_clear(cb->particle_hash,cb->particle_hash_size,&cb->selected_label,
			    gb->hit_name[1],gb->hit_name[0]);
		}
	      }
	    }
	  } else {
	    set_select_double(p,gb->hit_name[0]);
	    ks_free_int_list(cb->selected_label);
	    cb->selected_label = NULL;
	  }
	}
      }
    }
  }
#endif

  /*
  if(button == KS_GL_MOUSE_BUTTON_LEFT && state == KS_GL_MOUSE_STATE_DOWN){
    cb->move_mouse_pos[0] = x;
    cb->move_mouse_pos[1] = y;
  }
  */

}
void cv_draw_info(KS_GL_BASE *gb, CV_BASE *cb, int width, int height)
{
  int x,y;
  int y_sep = 15;
  char text[256];
  int text_width;
  int font = KS_GL_FONT_HELVETICA_12;

  x = 10;
  y = 20;

  if(cb->frame[cb->current_frame].prep_block != NULL){
    snprintf(text,sizeof(text),"%s (%s)",cb->frame[cb->current_frame].file_name,
	    cb->frame[cb->current_frame].prep_block->type);
  } else {
    snprintf(text,sizeof(text),"%s",cb->frame[cb->current_frame].file_name);
  }
  ks_get_glut_font_width(gb,font,text,&text_width);
  ks_gl_base_foreground_color(gb);
  glRasterPos2i(width-x-text_width,height-y);
  ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
  y += y_sep;

  /*
  strcpy(text,"color mode: ");
  strcat(text,cb->color_mode_name[cb->color_mode]);
  ks_get_glut_font_width(gb,font,text,&text_width);
  ks_gl_base_foreground_color(gb);
  glRasterPos2i(width-x-text_width,height-y);
  ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
  y += y_sep;
  */
  /*
  ks_gl_base_foreground_color(gb);
  glRasterPos2i(width-x-text_width,height-y);
  sprintf(text,"%f",t-t0);
  t0 = t;
  ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
  y += y_sep;
  */
  /*
  ks_gl_base_foreground_color(gb);
  glRasterPos2i(width-x-text_width,height-y);
  sprintf(text,"%d",particle_mode);
  ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
  */

}
void cv_draw_text(KS_GL_BASE *gb, CV_BASE *cb, int width, int height)
{
  CV_FRAME *fr;
  CV_RESIDUE *pr;
  CV_PARTICLE *p;
  CV_PARTICLE *pp[CV_SELECT_PARTICLE_MAX];
  int x, y;
  int num ;
  char text[256];
  KS_INT_LIST *il;
  int i,j;
  double cd[3];
  double dd[3][3];
  int font;
  int font_height;

  /*  printf("draw_text\n");*/
  { /* upper */
    font = KS_GL_FONT_8x13;
    font_height = 13;
    x = 0;
    y = height-font_height;

    num = 0;
    fr = &cb->frame[cb->current_frame];
    for(il = cb->selected_label; il != NULL; il = il->next, num++){
      p = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
					(unsigned int)il->value);
      /*      printf("show %d %p\n",il->value,p);*/
      if(p != NULL){
	for(i = 0; i < 3; i++){
	  cd[i] = (p->cd[i] + p->move[i] /*+ 
		   p->property->aggregate->move[i]+
		   p->property->chain->move[i]+
		   p->property->residue->move[i]*/);
	}
	text[0] = '\0';
	if(cb->draw_mode == CV_DRAW_MODE_TUBE || 
	   cb->draw_mode == CV_DRAW_MODE_RIBBON){
	  sprintf(text,"%d %s %c",
		  p->property->residue->file_label,
		  p->property->residue->name,
		  p->property->residue->type
		  );
	} else if(cb->draw_mode == CV_DRAW_MODE_LINE || 
		  cb->draw_mode == CV_DRAW_MODE_PARTICLE || 
		  cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN){
	  if(p->property != NULL){
	    sprintf(text,"%4d %-4s %3d %-3s (%.1f %.1f %.1f)",p->file_label,
		    p->property->name,
		    p->property->residue->file_label,
		    p->property->residue->name,
		    cd[0],cd[1],cd[2]);
	  } else {
	    sprintf(text,"%4d %d (%.1f %.1f %.1f)",p->file_label,
		    p->atype[cb->atype_mode],
		    cd[0],cd[1],cd[2]);
	    /*
	    sprintf(text,"%4d %d %f %.1f %.1f %.1f",p->file_label,
		    p->atype[cb->atype_mode],
		    ks_get_gl_atom_size(p->atom->gl_atom,KS_GL_ATOM_SIZE_NORMAL),
		    cd[0],cd[1],cd[2]);
	    */
	  }
	}
	if(text[0] != '\0'){
	  ks_gl_base_foreground_color(gb);
	  glRasterPos2i(x,y);
	  ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
	  y -= font_height+1;
	}
      }
    }
    if(num > 1 && num <= CV_SELECT_PARTICLE_MAX && 
       cb->draw_mode != CV_DRAW_MODE_TUBE && cb->draw_mode != CV_DRAW_MODE_RIBBON){
      for(il = cb->selected_label,num=0; il != NULL; il = il->next, num++){
	ks_assert(num < CV_SELECT_PARTICLE_MAX);
	pp[num] = lookup_cv_particle_hash_label(cb->particle_hash,cb->particle_hash_size,
						(unsigned int)il->value);
      }
      for(i = 0; i < num; i++){
	for(j = 0; j < 3; j++){
	  dd[i][j] = pp[i]->cd[j] + pp[i]->move[j];
	}
      }
      if(num == 2){
	sprintf(text,"distance %.3f",calc_bond(dd[0],dd[1]));
      } else if(num == 3){
	sprintf(text,"angle %.1f",calc_angle(dd[0],dd[1],dd[2])/M_PI*180);
      } else if(num == 4){
	sprintf(text,"torsion %.1f",calc_torsion(dd[0],dd[1],dd[2],dd[3])/M_PI*180);
      }
      ks_gl_base_foreground_color(gb);
      glRasterPos2i(x,y);
      ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
      y -= font_height+1;
    }
    for(il = cb->selected_residue_label; il != NULL; il = il->next, num++){
      pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
					(unsigned int)il->value);
      if(pr != NULL){
	sprintf(text,"%d %s %c",pr->file_label, pr->name,pr->type);
	ks_gl_base_foreground_color(gb);
	glRasterPos2i(x,y);
	ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
	y -= font_height+1;
      }
    }
  }

  { /* lower */
    KS_TEXT_LIST *p;

    font = KS_GL_FONT_HELVETICA_12;
    font_height = 12;
    x = 6;
    y = 6;
    for(p = cb->telop; p != NULL; p = p->next){
      ks_gl_base_foreground_color(gb);
      glRasterPos2i(x,y);
      ks_draw_glut_text(gb,p->text,font,cb->background_transparency,1);
      y += font_height+1;
    }

    if(cb->prep_change_mode == CV_PREP_CHANGE_IMPROPER && 
       cb->frame[cb->current_frame].prep_block != NULL){
      KS_INT_LIST_LIST *pii;
      KS_AMBER_PREP_BLOCK *pb;
      int num,i,j;
      fr = &cb->frame[cb->current_frame];
      pb = fr->prep_block;
      num = ks_count_int_list_list(fr->prep_block->improper);
      for(pii = pb->improper,i=1; pii != NULL; pii = pii->next,i++){
	if(i == display_improper_target){
	  glRasterPos2i(x,y);
	  y += font_height+1;
	  sprintf(text,"%d/%d %s %s %s %s",i,num,
		  pb->atom[pii->p->value].name,
		  pb->atom[pii->p->next->value].name,
		  pb->atom[pii->p->next->next->value].name,
		  pb->atom[pii->p->next->next->next->value].name);
	  ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
	  for(j = 0; j < fr->aggregate->chain->residue->particle_num; j++){
	    if(j == pii->p->value ||
	       j == pii->p->next->value ||
	       j == pii->p->next->next->value ||
	       j == pii->p->next->next->next->value){
	      set_selected_for_particle(fr->aggregate->chain->residue->particle[j]);
	    } else {
	      unset_selected_for_particle(fr->aggregate->chain->residue->particle[j]);
	    }
	  }
	  /*
	  printf("%d/%d %s %s %s %s\n",i,num,
		 pb->atom[pii->p->value].name,
		 pb->atom[pii->p->next->value].name,
		 pb->atom[pii->p->next->next->value].name,
		 pb->atom[pii->p->next->next->next->value].name);
	  */
	}
      }
    }

    ks_gl_base_foreground_color(gb);
    glRasterPos2i(x,y);
    ks_draw_glut_input_key_text(gb,cb->background_transparency);
  }

  if(ks_gl_base_verbose_level(gb) >= 2){ /* draw time */
    CV_FRAME *fr;
    fr = &cb->frame[cb->current_frame];
    if(fr->have_time == KS_TRUE){
      font = KS_GL_FONT_TIMES_ROMAN_24;
      x = 24;
      y = 24;
      ks_gl_base_foreground_color(gb);
      glRasterPos2i(x,y);
      if(cb->draw_time_unit_type == CV_DRAW_TIME_UNIT_FS){
	sprintf(text,"%.1f(fs)",(fr->time+cb->time_offset)*1e+15);
      } else if(cb->draw_time_unit_type == CV_DRAW_TIME_UNIT_PS){
	sprintf(text,"%.1f(ps)",(fr->time+cb->time_offset)*1e+12);
      } else if(cb->draw_time_unit_type == CV_DRAW_TIME_UNIT_NS){
	sprintf(text,"%.1f(ns)",(fr->time+cb->time_offset)*1e+9);
      } else {
	ks_assert(0);
      }
      ks_draw_glut_text(gb,text,font,cb->background_transparency,1);
    }
  }

  if(ks_gl_base_verbose_level(gb) >= 2){ /* draw user text */
    CV_FRAME *fr;
    fr = &cb->frame[cb->current_frame];
    font = KS_GL_FONT_TIMES_ROMAN_24;
    if(fr->flags&CV_FRAME_CDV){
      for(i = 0; i < CV_USER_TEXT_MAX; i++){
	if(fr->user_text[i].text != NULL){
	  x = (fr->user_text[i].x/(2.7*2) + .5)*width;
	  y = (fr->user_text[i].y/(2.7*2) + .5)*height;
	  ks_gl_base_foreground_color(gb);
	  glRasterPos2i(x,y);
	  ks_draw_glut_text(gb,fr->user_text[i].text,font,0.0,1);
	  /*	  printf("%d %d %s\n",x,y,fr->user_text[i].text);*/
	}
      }
    }
  }

}
#if 0
static void shake_bond(CV_PARTICLE *pl)
{
  int i;
  int loop;
  enum{PREV,NOW,NUM};
  double dd[NUM][3];
  double r[NUM];
  CV_PARTICLE *p,*pb;
  CV_BOND *bond;
  double gij;
  int div;

  /*
  for(p = pl; p != NULL; p = p->next){
    printf("%d %s % f % f % f % f % f % f\n",p->label,p->atom->name
	   ,p->cd[0],p->cd[1],p->cd[2]
	   ,p->add[0],p->add[1],p->add[2]);
  }
  */
  for(loop = 0; loop < 5; loop++){
    for(p = pl; p != NULL; p = p->next){
      for(bond = p->bond; bond != NULL; bond = bond->next){
	pb = bond->p;
	if(p->label < pb->label){
	  for(i = 0; i < 3; i++) 
	    dd[NOW][i] = p->cd[i]+p->add[i] - pb->cd[i]-pb->add[i];
	  r[NOW] = sqrt(dd[NOW][0]*dd[NOW][0] + dd[NOW][1]*dd[NOW][1] + dd[NOW][2]*dd[NOW][2]);

	  /*
	  if((r-bond->len)/bond->len < 0.1){
	    continule;
	  }
	  */

	  for(i = 0; i < 3; i++) 
	    dd[PREV][i] = p->cd[i] - pb->cd[i];
	  r[PREV] = sqrt(dd[PREV][0]*dd[PREV][0]+dd[PREV][1]*dd[PREV][1]+dd[PREV][2]*dd[PREV][2]);

	  div = 0;
	  if(!(p->flags&CV_PARTICLE_SELECTED)) div += 5;
	  if(!(pb->flags&CV_PARTICLE_SELECTED)) div += 5;
	  if(div == 0) continue;

	  gij = ((r[NOW]*r[NOW]-bond->len*bond->len)/
		 (div*(dd[PREV][0]*dd[NOW][0] + dd[PREV][1]*dd[NOW][1] + dd[PREV][2]*dd[NOW][2])));
	  /*
	  printf("%d %d %s %d %s %f %f %f %f\n",loop
		 ,p->label,p->atom->name
		 ,pb->label,pb->atom->name
		 ,bond->len
		 ,r[PREV],r[NOW],gij);
	  */
	  if(!(p->flags&CV_PARTICLE_SELECTED)){
	    for(i = 0; i < 3; i++){
	      p->add[i] -= gij*dd[PREV][i];
	    }
	    /*
	    printf("p %d %s (%f %f %f)(%f %f %f)\n",p->label,p->atom->name
		   ,p->cd[0],p->cd[1],p->cd[2]
		   ,p->add[0],p->add[1],p->add[2]);
	    */
	  }
	  if(!(pb->flags&CV_PARTICLE_SELECTED)){
	    for(i = 0; i < 3; i++){
	      pb->add[i] += gij*dd[PREV][i];
	    }
	    /*
	    printf("pb %d %s (%f %f %f)(%f %f %f)\n",pb->label,pb->atom->name
		   ,pb->cd[0],pb->cd[1],pb->cd[2]
		   ,pb->add[0],pb->add[1],pb->add[2]);
	    */
	  }
	}
      }
    }
  }

  for(p = pl; p != NULL; p = p->next){
    /*
    printf("%d %s % f % f % f % f % f % f\n",p->label,p->atom->name
	   ,p->cd[0],p->cd[1],p->cd[2]
	   ,p->add[0],p->add[1],p->add[2]);
    */
    for(i = 0; i < 3; i++) p->cd[i] += p->add[i];
    for(i = 0; i < 3; i++) p->add[i] = 0;
  }
  /*  ks_exit(EXIT_FAILURE);*/
}
#endif
static void md_bond(CV_PARTICLE *pl, double *add)
{
  int i;
  int loop;
  double dd[3];
  double r;
  CV_PARTICLE *p,*pb;
  CV_BOND *bond;
  double dphir;
  double erg = 0.1;

  for(p = pl; p != NULL; p = p->next){
    if(p->flags&CV_PARTICLE_SELECTED){
      for(i = 0; i < 3; i++) p->move[i] += add[i];
    }
  }
  for(i = 0; i < 3; i++) add[i] = 0;

  for(loop = 0; loop < 5; loop++){
    for(p = pl; p != NULL; p = p->next){
      for(bond = p->bond; bond != NULL; bond = bond->next){
	pb = bond->p;
	if(p->label < pb->label){
	  for(i = 0; i < 3; i++) 
	    dd[i] = p->cd[i]+p->move[i] - pb->cd[i]-pb->move[i];
	  r = sqrt(dd[0]*dd[0] + dd[1]*dd[1] + dd[2]*dd[2]);

	  dphir =-2.0*erg*(r-bond->len)/r;

	  if(!(p->flags&CV_PARTICLE_SELECTED)){
	    for(i = 0; i < 3; i++){
	      p->move[i] += dphir*dd[i];
	    }
	  }
	  if(!(pb->flags&CV_PARTICLE_SELECTED)){
	    for(i = 0; i < 3; i++){
	      pb->move[i] -= dphir*dd[i];
	    }
	  }
	}
      }
    }
  }

  /*  ks_exit(EXIT_FAILURE);*/
}
void cv_process_motion(KS_GL_BASE *gb, CV_BASE *cb, int x, int y)
{
  if(ks_is_gl_move_target_motion(gb) == KS_TRUE){
    /*    shake_bond(cb->frame[cb->current_frame].particle);*/
    md_bond(cb->frame[cb->current_frame].particle,cb->move_add);
    /*    printf("%f %f %f\n",cb->move_add[0],cb->move_add[1],cb->move_add[2]);*/
  }
}
BOOL cv_process_idle(KS_GL_BASE *gb, CV_BASE *cb)
{
  KS_TEXT_LIST *telop_prev = cb->telop;
  if(cb->auto_command != NULL){
    if(cb->auto_command->current < cb->auto_command->num){
      if(cb->auto_command->command[cb->auto_command->current].key == 'q' ||
	 cb->auto_command->command[cb->auto_command->current].key == 'Q'){
	if(cb->idle == NULL){
	  return KS_FALSE;
	}
      } else {
	unsigned int active_flags;
	char key;
	active_flags = cb->auto_command->command[cb->auto_command->current].alt?KS_GL_ACTIVE_ALT:0;
	if(cb->auto_command->command[cb->auto_command->current].special == KS_FALSE){
	  key = cb->auto_command->command[cb->auto_command->current].key;
	  if(((key >= '1' && key <= '9') || key == '=')){
	    cv_process_keyboard(gb,cb,key,0,0);
	  } else {
	    ks_gl_call_key_func(gb,cb->auto_command->command[cb->auto_command->current].key,0,0,
				active_flags);
	  }
	} else {
	  ks_gl_call_special_key_func(gb,cb->auto_command->command[cb->auto_command->current].key,
				      0,0,active_flags);
	}
	cb->auto_command->current++;
	ks_redraw_gl(gb);
      }
    }
  }
  ks_gl_base_idle(gb);
  cb->telop = ks_del_text_list_time(cb->telop,ks_get_gl_time(gb));
  if(cb->idle != NULL){
    cb->idle(gb,cb);
  }
  if(cb->idle != NULL || cb->telop != telop_prev){
    ks_redraw_gl(gb);
  }
  return KS_TRUE;
}
void cv_init_lookat(KS_GL_BASE *gb, CV_BASE *cb)
{
  ks_set_gl_lookat_position(gb,cb->frame[cb->current_frame].center);
  ks_set_gl_rotation_center(gb,cb->frame[cb->current_frame].center);
  /*
  printf("%f %f %f\n"
	 ,cb->frame[cb->current_frame].center[0]
	 ,cb->frame[cb->current_frame].center[1]
	 ,cb->frame[cb->current_frame].center[2]);
  */
}
void cv_set_lookat(KS_GL_BASE *gb, CV_BASE *cb, double *pos)
{
  ks_change_gl_lookat(gb,pos,1);
  ks_set_gl_rotation_center(gb,pos);
}
void cv_draw_2d(KS_GL_BASE *gb, CV_BASE *cb, int width, int height)
{
  /*
  int x = 10,y = 50;
  int x_size = 100, y_size = 100;
  int y_sep = 20;
  if(cb->rmsd_graph != NULL){
    ks_draw_gl_graph_2d(gb,cb->rmsd_graph,
			width-x_size-x,
			height-y_size-y,100,100,0,cb->current_frame);
    y += y_size+y_sep;
  }
  if(cb->rmsf_graph != NULL){
    int residue;
    CV_RESIDUE *pr;
    if(cb->selected_residue_label != NULL){
      pr = lookup_cv_residue_hash_label(cb->residue_hash,cb->residue_hash_size,
					(unsigned int)cb->selected_residue_label->value);
      residue = pr->label;
    } else {
      residue = -1;
    }
    ks_draw_gl_graph_2d(gb,cb->rmsf_graph,
			width-x_size-x,
			height-y_size-y,100,100,0,residue);
    y += y_size+y_sep;
  }
  */
}
void cv_process_keyboard(KS_GL_BASE *gb, CV_BASE *cb, unsigned char key, int x, int y)
{
  int atype;
  CV_ATOM *atom;

  if(cb->frame[cb->current_frame].prep_block != NULL && 
     cb->prep_change_mode == CV_PREP_CHANGE_IMPROPER){
    if(key == 0x7f){  // DEL key
      KS_INT_LIST_LIST *pii,*prev;
      int i;
      CV_FRAME *fr;
      KS_AMBER_PREP_BLOCK *pb;
      fr = &cb->frame[cb->current_frame];
      pb = fr->prep_block;
      prev = NULL;
      for(pii = pb->improper,i=1; pii != NULL; pii = pii->next,i++){
	if(i == display_improper_target){
	  if(prev == NULL){
	    pb->improper = pii->next;
	  } else {
	    prev->next = pii->next;
	  }
	  ks_free_int_list(pii->p);
	  ks_free(pii);
	  break;
	}	
	prev = pii;
      }
      if(display_improper_target > ks_count_int_list_list(pb->improper)){
	display_improper_target = ks_count_int_list_list(pb->improper);
      }
    }
  } else {
    ks_gl_base_keyboard(gb,key,x,y);

    if(ks_is_gl_input_key_mode(gb) == KS_FALSE && ((key >= '1' && key <= '9') || key == '=')){
      if(key == '='){
	atype = 9;
      } else {
	atype = key-'1';
      }
      if(ks_get_glut_alt(gb) == KS_TRUE){
	atype += 10;
      }
      if(ks_get_glut_ctrl(gb) == KS_TRUE){
	atype += CV_DARK_ATOM_TYPE_ADD;
      }
      atom = lookup_cv_atom_atype(cb->atom,atype);
      if(atom != NULL){
	if(atom->flags&CV_ATOM_SHOW){
	  atom->flags &= ~CV_ATOM_SHOW;
	} else {
	  atom->flags |= CV_ATOM_SHOW;
	}
#ifdef USE_GLSL
	update_shader_sphere_draw_info(cb,&cb->frame[cb->current_frame]);
	update_shader_rod_draw_info(cb,&cb->frame[cb->current_frame]);
#endif
      }
    }
  }
}
void cv_process_special_keyboard(KS_GL_BASE *gb, CV_BASE *cb, unsigned char key, int x, int y)
{
  ks_gl_base_special_key(gb,key,x,y);
  if(cb->frame[cb->current_frame].prep_block != NULL){
    if(cb->prep_change_mode == CV_PREP_CHANGE_PARTICLE_ORDER){
      if(key == KS_GL_KEY_PAGE_UP){
	change_particle_order_target--;
	if(change_particle_order_target <= 0){
	  change_particle_order_target = count_cv_particle(cb->frame[cb->current_frame].particle);
	}
      }
      if(key == KS_GL_KEY_PAGE_DOWN){
	change_particle_order_target++;
	if(change_particle_order_target>=
	   count_cv_particle(cb->frame[cb->current_frame].particle)+1){
	  change_particle_order_target = 1;
	}
      }
      /*
	printf("%d %d\n",change_particle_order_target,
	count_cv_particle(cb->frame[cb->current_frame].particle));
      */
    } else if(cb->prep_change_mode == CV_PREP_CHANGE_IMPROPER){
      if(key == KS_GL_KEY_PAGE_UP){
	display_improper_target--;
	if(display_improper_target <= 0){
	  display_improper_target = 
	    ks_count_int_list_list(cb->frame[cb->current_frame].prep_block->improper);
	}
      }
      if(key == KS_GL_KEY_PAGE_DOWN){
	display_improper_target++;
	if(display_improper_target>=
	   ks_count_int_list_list(cb->frame[cb->current_frame].prep_block->improper)){
	  display_improper_target = 1;
	}
      }
      /*
      printf("%d %d\n",display_improper_target,
	     ks_count_int_list_list(cb->frame[cb->current_frame].prep_block->improper));
      */
    }
  }
}
static void draw_axis(KS_GL_BASE *gb)
{
  int i;
  GLfloat color[3][4] = {{1.0,0.0,0.0,1.0},{0.0,1.0,0.0,1.0},{0.0,0.0,1.0,1.0}};
  char text[3][2] = {"X","Y","Z"};
  double pos[3][3] = {{0.0,0.0,0.0},{0.0,0.0,0.0},{0.0,0.0,0.0}};
  GLfloat len = 10.0;
  GLfloat em[4] = {0,0,0,0};

  pos[0][0] = len;
  pos[1][1] = len;
  pos[2][2] = len;

  glViewport(10,10,
	     (GLsizei)gb->viewport_3d[KS_GL_VIEWPORT_WIDTH]*.2,
	     (GLsizei)gb->viewport_3d[KS_GL_VIEWPORT_HEIGHT]*.2);

  glClear(GL_DEPTH_BUFFER_BIT);

  glEnable(GL_LIGHT1);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(gb->eye_ang, 
		 (double)gb->viewport_3d[KS_GL_VIEWPORT_WIDTH]/
		 (double)gb->viewport_3d[KS_GL_VIEWPORT_HEIGHT], 
		 1.0,100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  gluLookAt(40.0, 0.0, 0.0, 
	    0.0, 0.0, 0.0,
	    0.0, 0.0, 1.0);

  glMaterialfv(GL_FRONT,GL_EMISSION,em);

  glPushMatrix();
  glMultMatrixd(gb->rot_mat);
  for(i = 0; i < 3; i++){
    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color[i]);
    ks_draw_gl_arrow_pos(gb,0.0,0.0,0.0,pos[i][0],pos[i][1],pos[i][2],
			 0.5,6,KS_DRAW_GL_ROD_COVER_START,KS_FALSE);
  }

  glDisable(GL_LIGHTING);

  ks_gl_base_foreground_color(gb);
  ks_use_glut_font(gb,KS_GL_FONT_HELVETICA_12);
  for(i = 0; i < 3; i++){
    glRasterPos3dv(pos[i]);
    glCallLists(strlen(text[i]), GL_BYTE, text[i]);
  }
  glPopMatrix();
  glDisable(GL_LIGHT1);
}
static void display_3d(KS_GL_BASE *gb, void *vp)
{
  CV_BASE *cb;
  cb = (CV_BASE*)vp;
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  cv_draw(gb,cb);
  //  printf("%d %d\n",cb->atype_mode,cb->frame[cb->current_frame].atype_mode);
  glDisable(GL_LIGHT0);
  if(cb->axis_mode == CV_AXIS_MODE_ON){
    draw_axis(gb);
  }
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);

}
static void __KS_USED__ draw_2d_arrow(KS_GL_BASE *gb, CV_BASE *cb)
{
  int start[2] = {350,200};
  //  float end[2] = {40,30};
  int len = 60;
  int width = 6;

  glRasterPos2i(start[0]-15,start[1]+len+10);
  ks_draw_glut_text(gb,"155(m/s)",KS_GL_FONT_TIMES_ROMAN_24,cb->background_transparency,1);

  glBegin(GL_POLYGON);
  glVertex2i(start[0]-width,start[1]);
  glVertex2i(start[0]-width*2,start[1]);
  glVertex2i(start[0],start[1]-width*2);
  glVertex2i(start[0]+width*2,start[1]);
  glVertex2i(start[0]+width,start[1]);
  glVertex2i(start[0]+width,start[1]+len);
  glVertex2i(start[0]-width,start[1]+len);
  glEnd();
}
static BOOL is_shown_fv_map(KS_GL_BASE *gb, CV_FRAME *fr)
{
  return (ks_gl_base_verbose_level(gb) >= 1 && fr->fv != NULL && fr->fv->field != NULL);
}
static void display_2d(KS_GL_BASE *gb, void *vp, int width, int height)
{
  CV_BASE *cb;
  CV_FRAME *fr;

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];

  if(ks_gl_base_verbose_level(gb) >= 4){
    cv_draw_info(gb,cb,width,height);
  }
  if(ks_gl_base_verbose_level(gb) >= 3){
    cv_draw_color_sample(gb,cb,width,height);
    /*    cv_draw_2d(gb,cb,width,height);*/

    if((cb->draw_additional_surface == KS_TRUE) && 
       fr->color_map != NULL && fr->color_map->num != 0){
      draw_color_map(gb,fr->color_map);
    } else if(fr->c_color_info.map != NULL && cb->continuous_color.mode >= 0){
      draw_color_map(gb,fr->c_color_info.map[cb->continuous_color.mode]);
    }

  }
  if(is_shown_fv_map(gb,fr)){
    fv_display_2d(gb,fr->fv->field,fr->fv->field_size,fr->fv->contour_size,cb->fv_target,
		  width,height,fr->fv->black_and_white);
  }
  cv_draw_text(gb,cb,width,height);
  /*  draw_2d_arrow(gb,cb);*/
}
void cv_process_display(KS_GL_BASE *gb, CV_BASE *cb)
{
  ks_gl_base_display(gb,(void*)cb,display_3d,display_2d);
}
static void __KS_USED__ set_eye_len(KS_GL_BASE *gb, CV_BASE *cb)
{
  int i;
  CV_FRAME *fr;
  CV_PARTICLE *p;
  double max,d;
  fr = &cb->frame[cb->current_frame];

  for(p = fr->particle; p != NULL; p = p->next){
    for(i = 1; i < 2; i++){
      d = fabs(p->cd[i] - fr->center[i]);
      if(max < d) max = d;
    }
  }
  printf("eye_len %f\n",(max+2)*tan(75./180.*M_PI)*1.3);
  ks_set_gl_base_eye_len(gb,(max+2)*tan(75./180.*M_PI)*1.3);
}
void cv_init_gl(KS_GL_BASE *gb, CV_BASE *cb)
{
  GLfloat light_position[] = {1.0, 1.1, 1.2, 0.0};
  GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};

  glShadeModel(GL_SMOOTH);

  glLightfv(GL_LIGHT0, GL_POSITION, cb->light_position);

  glLightfv(GL_LIGHT1, GL_POSITION, light_position); /* light for axis */
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.5);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);

  glCullFace(GL_BACK);

  glAlphaFunc(GL_GREATER,0.5);
  glClearStencil(0x0);
  glLoadName(CV_NAME_DEFAULT);

  ks_init_glut_font(gb);
  ks_init_gl_ribbon(gb,0.25);
  cv_set_glut_key(gb,cb);
  cv_init_lookat(gb,cb);

#ifdef USE_GLSL
  gb->hit_name = gb->select_buffer;  // use hit_name to set hit type
#endif

  /*
  if(cb->frame[cb->current_frame].flags&CV_FRAME_CDV){
    set_eye_len(gb,cb);
  }
  */
}
BOOL cv_have_side(CV_BASE *cb)
{
  return cb->frame[cb->current_frame].have_side;
}
BOOL cv_set_auto_command(CV_BASE *cb, char *auto_command)
{
  int i,c;
  if((cb->auto_command = (CV_AUTO_COMMAND*)ks_malloc(sizeof(CV_AUTO_COMMAND),"cb->auto_command")) 
     == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  cb->auto_command->current = 0;
  cb->auto_command->num = 0;
  for(i = 0; auto_command[i]; i++){
    if(auto_command[i] != '+' && auto_command[i] != '\\'){ // do not count postfix and prefix char
      cb->auto_command->num++;
    }
  }
  if((cb->auto_command->command = 
      (CV_AUTO_COMMAND_KEY*)ks_malloc(sizeof(CV_AUTO_COMMAND_KEY)*cb->auto_command->num,
				      "cb->auto_command->command"))
     == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(i = 0; i < cb->auto_command->num; i++){
    cb->auto_command->command[i].alt = KS_FALSE;
    cb->auto_command->command[i].special = KS_FALSE;
  }
  c = 0;
  for(i = 0; auto_command[i]; i++){
    if(auto_command[i] == '+'){
      if(c != 0){
	cb->auto_command->command[c-1].alt = KS_TRUE;
      }
    } else if(auto_command[i] == '\\'){                /* prefix char for spiecial key */
      if(c != cb->auto_command->num){
	cb->auto_command->command[c].special = KS_TRUE;
      }
    } else if(auto_command[i] == '_'){                 /* _ means space key */
      cb->auto_command->command[c++].key = ' ';
    } else if(cb->auto_command->command[c].special == KS_TRUE){
      if(auto_command[i] == 'u'){
	cb->auto_command->command[c++].key = KS_GL_KEY_UP;
      } else if(auto_command[i] == 'd'){
	cb->auto_command->command[c++].key = KS_GL_KEY_DOWN;
      } else if(auto_command[i] == 'l'){
	cb->auto_command->command[c++].key = KS_GL_KEY_LEFT;
      } else if(auto_command[i] == 'r'){
	cb->auto_command->command[c++].key = KS_GL_KEY_RIGHT;
      } else if(auto_command[i] == 'h'){
	cb->auto_command->command[c++].key = KS_GL_KEY_HOME;
      }
    } else {
      cb->auto_command->command[c++].key = auto_command[i];
    }
  }
  return KS_TRUE;
}
BOOL cv_is_set_disable_error_hit_enter()
{
  char *cp;
  cp = getenv(CV_ENV_KEY_DISABLE_ERROR_HIT_ENTER);
  if(cp == NULL){
    return KS_FALSE;
  }
  if(cp[0] == '1'){
    return KS_TRUE;
  }
  return KS_FALSE;
}

#ifdef USE_GLSL
static CV_SHADER_BASE* allocate_shader_base()
{
  int i;
  CV_SHADER_BASE *sb;
  
  if((sb = (CV_SHADER_BASE*)ks_malloc(sizeof(CV_SHADER_BASE),"CV_SHADER_BASE* sb")) == NULL){
    ks_error_memory();
    return NULL;
  }

  if((sb->shader_parm[SHADER_PARM_NORMAL] = ks_allocate_gl_standard_shader_parm()) == NULL){
    ks_exit(EXIT_FAILURE);
  }
  sb->shader_parm[SHADER_PARM_NORMAL]->light_position[0] = 0.3f;
  sb->shader_parm[SHADER_PARM_NORMAL]->light_position[1] = 0.8f;
  sb->shader_parm[SHADER_PARM_NORMAL]->light_position[2] = 1.2f;
  sb->shader_parm[SHADER_PARM_NORMAL]->light_ambient = 0.2f;
  sb->shader_parm[SHADER_PARM_NORMAL]->material_shininess = 5.0f;
  sb->shader_parm[SHADER_PARM_NORMAL]->material_specular = 0.2f;
  if((sb->shader_parm[SHADER_PARM_HIGH_AMBIENT] = ks_allocate_gl_standard_shader_parm()) == NULL){
    ks_exit(EXIT_FAILURE);
  }
  *sb->shader_parm[SHADER_PARM_HIGH_AMBIENT] = *sb->shader_parm[SHADER_PARM_NORMAL];
  sb->shader_parm[SHADER_PARM_HIGH_AMBIENT]->light_ambient = 1.0f;

  // make shader for sphere
  if((sb->shader[SHADER_SPHERE] = ks_allocate_gl_standard_shader_info(KS_GL_STANDARD_SHADER_SPHERE,
								      SPHERE_TYPE_CNT)) == NULL){
    ks_error_memory();
    return NULL;
  }
  // make spheres for each detail
  for(i = 0; i < CV_SPHERE_DETAIL_CNT; i++){
    if((sb->sphere_parm[i] = ks_allocate_gl_shader_sphere_parm(cv_sphere_detail_values[i]))==NULL){
      ks_error_memory();
      return NULL;
    }
    ks_init_gl_standard_shader_sphere(sb->shader[SHADER_SPHERE],i,sb->sphere_parm[i],-1);
  }

  if((sb->shader[SHADER_REGULAR] =
      ks_allocate_gl_standard_shader_info(KS_GL_STANDARD_SHADER_REGULAR,OBJ_TYPE_CNT)) == NULL){
    ks_error_memory();
    return NULL;
  }

  if((sb->shader[SHADER_POINT] = ks_allocate_gl_standard_shader_info(KS_GL_STANDARD_SHADER_POINT,1))
     == NULL){
    ks_error_memory();
    return NULL;
  }

  for(i = 0; i < OBJ_TYPE_CNT; i++){
    sb->obj_parm[i] = NULL;
    sb->obj_draw_info[i] = NULL;
  }

  return sb;
}
static void free_shader_base(CV_SHADER_BASE *sb)
{
  int i;
  for(i = 0; i < SHADER_TYPE_CNT; i++){
    ks_free_gl_standard_shader_info(sb->shader[i]);
  }
  for(i = 0; i < SHADER_PARM_CNT; i++){
    ks_free(sb->shader_parm[i]);
  }
  for(i = 0; i < CV_SPHERE_DETAIL_CNT; i++){
    ks_free_gl_shader_sphere_parm(sb->sphere_parm[i]);
  }
  for(i = 0; i < OBJ_TYPE_CNT; i++){
    if(sb->obj_parm[i] != NULL){
      ks_free_gl_shader_regular_parm(sb->obj_parm[i]);
    }
  }
  ks_free_gl_shader_sphere_draw_info(sb->sphere_draw_info);
  ks_free(sb->sphere_draw_info_particle);
  for(i = 0; i < OBJ_TYPE_CNT; i++){
    if(sb->obj_draw_info[i] != NULL){
      ks_free_gl_shader_regular_draw_info(sb->obj_draw_info[i]);
    }
  }
  ks_free(sb);
#ifndef NDEBUG
  printf("free_shader_base is successfully finished\n");
#endif
}
static void set_shader_sphere_draw_info(CV_BASE *cb, CV_FRAME *fr)
{
  int i;
  BOOL draw_flag;
  CV_PARTICLE *p;
  GLfloat color[4];
  int size;
  int bright;
  CV_SHADER_BASE *sb = cb->sb;
  
  if(cb->particle_size == CV_PARTICLE_SIZE_NORMAL){
    size = KS_GL_ATOM_SIZE_NORMAL;
  } else if(cb->particle_size == CV_PARTICLE_SIZE_SMALL){
    size = KS_GL_ATOM_SIZE_SMALL;
  } else if(cb->particle_size == CV_PARTICLE_SIZE_LARGE){
    size = KS_GL_ATOM_SIZE_LARGE;
  }

  for(p = fr->particle, i = 0; p != NULL; p = p->next){
    if(!(p->atom->flags&CV_ATOM_SHOW)) continue;
    draw_flag = KS_FALSE;
    if(cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN || 
       cb->draw_mode == CV_DRAW_MODE_TUBE ||
       cb->draw_mode == CV_DRAW_MODE_RIBBON){ // draw_mode conditions used in draw_frame
      if(p->property == NULL){
	draw_flag = KS_TRUE;
      } else if((p->property->chain->len > 1 && p->property->chain->flags&CV_CHAIN_AMINO && 
		 p->property->residue->flags&CV_RESIDUE_AMINO) || 
		(fr->pdb_ext == KS_TRUE && cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN)){
	if(cb->draw_mode == CV_DRAW_MODE_MAIN_CHAIN){
	  if(p->flags&CV_PARTICLE_MAIN){
	    draw_flag = KS_TRUE;
	  }
	} else if(cb->draw_mode == CV_DRAW_MODE_TUBE || cb->draw_mode == CV_DRAW_MODE_RIBBON){
	  //  draw_ribbon is not implemented
	}
      } else {
	draw_flag = KS_TRUE;
      }
    } else {
      draw_flag = KS_TRUE;
    }
    if(draw_flag == KS_TRUE){
      if(p->flags&CV_PARTICLE_SELECTED){
	bright = KS_GL_ATOM_BRIGHT_HIGH;
      } else {
	bright = KS_GL_ATOM_BRIGHT_NORMAL;
      }
      if(cb->continuous_color.num != 0 && cb->continuous_color.mode != -1){
	get_continuous_color(p,&cb->continuous_color,&fr->c_color_info,color);
      } else {
	get_color(p,cb->color_mode,bright,color,fr->prep_block != NULL);
      }
      sb->sphere_draw_info->elem[i].x = p->cd[0];
      sb->sphere_draw_info->elem[i].y = p->cd[1];
      sb->sphere_draw_info->elem[i].z = p->cd[2];
      sb->sphere_draw_info->elem[i].size = (ks_get_gl_atom_size(p->atom->gl_atom,size)*
					    cb->particle_size_scale);
      sb->sphere_draw_info->elem[i].r = color[0];
      sb->sphere_draw_info->elem[i].g = color[1];
      sb->sphere_draw_info->elem[i].b = color[2];
      sb->sphere_draw_info->elem[i].em = ((p->flags&CV_PARTICLE_SELECTED) ?
					  KS_GL_SELECTED_EMISSION: 0.0f);
      sb->sphere_draw_info_particle[i] = p;                // to refer from draw_info to CV_PARTICLE
      p->draw_info_elem_p=&(sb->sphere_draw_info->elem[i]);// to refer from CV_PARTICLE to draw_info
      i++;
    }
  }
  ks_assert(i <= fr->particle_cnt);
  sb->sphere_draw_info->elem_cnt = i;
}
static void set_shader_sphere_draw_info_size(CV_BASE *cb, CV_FRAME *fr)
{
  int i;
  CV_PARTICLE *p;
  int size;
  CV_SHADER_BASE *sb = cb->sb;
  
  if(cb->particle_size == CV_PARTICLE_SIZE_NORMAL){
    size = KS_GL_ATOM_SIZE_NORMAL;
  } else if(cb->particle_size == CV_PARTICLE_SIZE_SMALL){
    size = KS_GL_ATOM_SIZE_SMALL;
  } else if(cb->particle_size == CV_PARTICLE_SIZE_LARGE){
    size = KS_GL_ATOM_SIZE_LARGE;
  }

  for(p = fr->particle, i = 0; p != NULL; p = p->next){
    if(p->atom->flags&CV_ATOM_SHOW){
      sb->sphere_draw_info->elem[i].size = (ks_get_gl_atom_size(p->atom->gl_atom,size)*
					    cb->particle_size_scale);
      i++;
    }
  }
  ks_assert(i <= fr->particle_cnt);
}
static void set_shader_sphere_draw_info_color(CV_BASE *cb, CV_FRAME *fr)
{
  int i;
  CV_PARTICLE *p;
  GLfloat color[4];
  int bright;
  CV_SHADER_BASE *sb = cb->sb;
  
  for(p = fr->particle, i = 0; p != NULL; p = p->next){
    if(p->atom->flags&CV_ATOM_SHOW){
      if(p->flags&CV_PARTICLE_SELECTED){
	bright = KS_GL_ATOM_BRIGHT_HIGH;
      } else {
	bright = KS_GL_ATOM_BRIGHT_NORMAL;
      }
      if(cb->continuous_color.num != 0 && cb->continuous_color.mode != -1){
	get_continuous_color(p,&cb->continuous_color,&fr->c_color_info,color);
      } else {
	get_color(p,cb->color_mode,bright,color,fr->prep_block != NULL);
      }
      sb->sphere_draw_info->elem[i].r = color[0];
      sb->sphere_draw_info->elem[i].g = color[1];
      sb->sphere_draw_info->elem[i].b = color[2];
      sb->sphere_draw_info->elem[i].em = ((p->flags&CV_PARTICLE_SELECTED) ?
					  KS_GL_SELECTED_EMISSION: 0.0f);
      i++;
    }
  }
  ks_assert(i <= fr->particle_cnt);
  sb->sphere_draw_info->elem_cnt = i;
}
static BOOL init_shader_sphere_draw_info(CV_BASE *cb, CV_FRAME *fr)
{
  CV_SHADER_BASE *sb = cb->sb;
  // make draw info for sphere
  if((sb->sphere_draw_info = ks_allocate_gl_shader_sphere_draw_info(fr->particle_cnt)) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  // make array to refer from draw_info to CV_PARTICLE
  if((sb->sphere_draw_info_particle =
      (CV_PARTICLE**)ks_malloc(sizeof(CV_PARTICLE*)*fr->particle_cnt,
			       "sb->sphere_draw_info_particle")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  set_shader_sphere_draw_info(cb,fr);
  ks_set_gl_shader_sphere_draw_info_share(sb->shader[SHADER_SPHERE],0,CV_SPHERE_DETAIL_CNT,
					  sb->sphere_draw_info,sb->shader_parm[SHADER_PARM_NORMAL]);
  return KS_TRUE;
}
static BOOL update_shader_sphere_draw_info(CV_BASE *cb, CV_FRAME *fr)
{
  CV_SHADER_BASE *sb = cb->sb;
  if(fr->particle_cnt > sb->sphere_draw_info->elem_capacity){
    ks_resize_gl_shader_sphere_draw_info(sb->sphere_draw_info,fr->particle_cnt);
    if((sb->sphere_draw_info_particle =
	(CV_PARTICLE**)ks_realloc(sb->sphere_draw_info_particle,
				  sizeof(CV_PARTICLE*)*fr->particle_cnt,
				 "sb->sphere_draw_info_particle")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  set_shader_sphere_draw_info(cb,fr);
  ks_update_gl_shader_sphere_draw_info_share(sb->shader[SHADER_SPHERE],0,CV_SPHERE_DETAIL_CNT,
					     sb->sphere_draw_info);
  return KS_TRUE;
}
static void make_system_box_shader_obj_parm(KS_GL_SHADER_REGULAR_PARM *obj_parm,
					    double len, double *center)
{
  int i;
  int xyz,ix,iy,iz;
  i = 0;
  for(xyz = 0; xyz < 3; xyz++){
    for(ix = -1; ix < 2; ix += 2){
      for(iy = -1; iy < 2; iy += 2){
	for(iz = -1; iz < 2; iz += 2){
	  obj_parm->elem[i].x = center[0] + len*ix;
	  obj_parm->elem[i].y = center[1] + len*iy;
	  obj_parm->elem[i].z = center[2] + len*iz;
	  if(xyz == 0){
	    obj_parm->elem[i].nx = -1.0f*ix;
	    obj_parm->elem[i].ny =  0.0f;
	    obj_parm->elem[i].nz =  0.0f;
	  } if(xyz == 1){
	    obj_parm->elem[i].nx =  0.0f;
	    obj_parm->elem[i].ny = -1.0f*iy;
	    obj_parm->elem[i].nz =  0.0f;
	  } if(xyz == 2){
	    obj_parm->elem[i].nx =  0.0f;
	    obj_parm->elem[i].ny =  0.0f;
	    obj_parm->elem[i].nz = -1.0f*iz;
	  }
	  obj_parm->elem[i].r = 0.0f;
	  obj_parm->elem[i].g = 0.0f;
	  obj_parm->elem[i].b = 0.2f;
	  i++;
	}
      }
    }
  }
  
  ks_assert(obj_parm->elem_size/sizeof(KS_GL_SHADER_REGULAR_PARM_ELEM) == i);

  i = 0;
  xyz = 0;
  // x-low-down
  obj_parm->index[i++] = 0+xyz; // (-1,-1,-1)
  obj_parm->index[i++] = 2+xyz; // (-1, 1,-1)
  obj_parm->index[i++] = 1+xyz; // (-1,-1, 1)
  // x-low-up
  obj_parm->index[i++] = 1+xyz; // (-1,-1, 1)
  obj_parm->index[i++] = 2+xyz; // (-1, 1,-1)
  obj_parm->index[i++] = 3+xyz; // (-1, 1, 1)
  // x-high-down
  obj_parm->index[i++] = 4+xyz; // ( 1,-1,-1)
  obj_parm->index[i++] = 5+xyz; // ( 1,-1, 1)
  obj_parm->index[i++] = 6+xyz; // ( 1, 1,-1)
  // x-high-up
  obj_parm->index[i++] = 5+xyz; // ( 1,-1, 1)
  obj_parm->index[i++] = 7+xyz; // ( 1, 1, 1)
  obj_parm->index[i++] = 6+xyz; // ( 1, 1,-1)

  xyz += 8;
  // y-low-down
  obj_parm->index[i++] = 4+xyz; // ( 1,-1,-1)
  obj_parm->index[i++] = 0+xyz; // (-1,-1,-1)
  obj_parm->index[i++] = 5+xyz; // ( 1,-1, 1)
  // y-low-up
  obj_parm->index[i++] = 5+xyz; // ( 1,-1, 1)
  obj_parm->index[i++] = 0+xyz; // (-1,-1,-1)
  obj_parm->index[i++] = 1+xyz; // (-1,-1, 1)
  // y-high-down
  obj_parm->index[i++] = 6+xyz; // ( 1, 1,-1)
  obj_parm->index[i++] = 7+xyz; // ( 1, 1, 1)
  obj_parm->index[i++] = 2+xyz; // (-1, 1,-1)
  // y-high-up
  obj_parm->index[i++] = 7+xyz; // ( 1, 1, 1)
  obj_parm->index[i++] = 3+xyz; // (-1, 1, 1)
  obj_parm->index[i++] = 2+xyz; // (-1, 1,-1)

  xyz += 8;
  // z-low-down
  obj_parm->index[i++] = 0+xyz; // (-1,-1,-1)
  obj_parm->index[i++] = 4+xyz; // ( 1,-1,-1)
  obj_parm->index[i++] = 2+xyz; // (-1, 1,-1)
  // z-low-up
  obj_parm->index[i++] = 2+xyz; // (-1, 1,-1)
  obj_parm->index[i++] = 4+xyz; // ( 1,-1,-1)
  obj_parm->index[i++] = 6+xyz; // ( 1, 1,-1)
  // z-high-down
  obj_parm->index[i++] = 1+xyz; // (-1,-1, 1)
  obj_parm->index[i++] = 3+xyz; // (-1, 1, 1)
  obj_parm->index[i++] = 5+xyz; // ( 1,-1, 1)
  // z-high-up
  obj_parm->index[i++] = 3+xyz; // (-1, 1, 1)
  obj_parm->index[i++] = 7+xyz; // ( 1, 1, 1)
  obj_parm->index[i++] = 5+xyz; // ( 1,-1, 1)

  ks_assert(obj_parm->index_cnt == i);
}
static void set_model_matrix(KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info)
{
  int i;
  float model_matrix[16] = {1.0f,0.0f,0.0f,0.0f,
			    0.0f,1.0f,0.0f,0.0f,
			    0.0f,0.0f,1.0f,0.0f,
			    0.0f,0.0f,0.0f,1.0f};
  gmat4 *matp;
  for(i = 0; i < draw_info->elem_cnt; i++){
    matp = (gmat4*)&draw_info->model_view[16*i];
    glm_translate_matrix(*matp,model_matrix,draw_info->elem[i].trans);
    if(draw_info->elem[i].angle_euler_zyz[0] != 0.0){
      glm_rotate_matrix_z(*matp,*matp,draw_info->elem[i].angle_euler_zyz[0]);
    }
    if(draw_info->elem[i].angle_euler_zyz[1] != 0.0){
      glm_rotate_matrix_y(*matp,*matp,draw_info->elem[i].angle_euler_zyz[1]);
    }
    if(draw_info->elem[i].angle_euler_zyz[2] != 0.0){
      glm_rotate_matrix_z(*matp,*matp,draw_info->elem[i].angle_euler_zyz[2]);
    }
    glm_scale_matrix(*matp,*matp,draw_info->elem[i].scale);
  }
}
static BOOL init_system_box_draw_info(CV_BASE *cb)
{
  CV_SHADER_BASE *sb = cb->sb;
  sb->obj_parm[OBJ_TYPE_BOX] = ks_allocate_gl_shader_regular_parm(8*3,3*2*6);

  make_system_box_shader_obj_parm(sb->obj_parm[OBJ_TYPE_BOX],
				  cb->reference_frame->system_size*.5,
				  cb->reference_frame->center);

  ks_init_gl_standard_shader_regular(sb->shader[SHADER_REGULAR],
				     OBJ_TYPE_BOX,sb->obj_parm[OBJ_TYPE_BOX],-1);

  // make draw info for the system box
  if((sb->obj_draw_info[OBJ_TYPE_BOX] = ks_allocate_gl_shader_regular_draw_info(1)) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  set_model_matrix(sb->obj_draw_info[OBJ_TYPE_BOX]);
  ks_set_gl_shader_regular_draw_info(sb->shader[SHADER_REGULAR],OBJ_TYPE_BOX,
				     sb->obj_draw_info[OBJ_TYPE_BOX],
				     sb->shader_parm[SHADER_PARM_HIGH_AMBIENT],-1);
  return KS_TRUE;
}
static void make_rod_shader_obj_parm(CV_BOND_TYPE *bond_type,CV_SHADER_BASE *sb, int type)
{
  int i,j;
  float r, len = 1.0f;
  int div;      // number to divide the rod circle
  float angle;
  int ver_cnt;
  KS_GL_SHADER_REGULAR_PARM *obj_parm;
  CV_BOND_TYPE *hit;
  GLfloat color[3];

  hit = lookup_cv_bond_type(bond_type,0); // use only the rod setting bond_type 0
  if(hit == NULL){
    r = init_cv_bond_type.radius;
    color[0] = init_cv_bond_type.color[0];
    color[1] = init_cv_bond_type.color[1];
    color[2] = init_cv_bond_type.color[2];
  } else {
    r = hit->radius;
    color[0] = hit->color[0];
    color[1] = hit->color[1];
    color[2] = hit->color[2];
  }
  
  if(type == OBJ_TYPE_ROD_NORMAL || type == OBJ_TYPE_ROD_LOW){
    // settings for rod
    if(type == OBJ_TYPE_ROD_NORMAL){
      div = 10;
    } else if(type == OBJ_TYPE_ROD_LOW){
      div = 6;
    }
    angle = 2.0*M_PI/div;
    ver_cnt = div*2;
    // set rod vertices, normals, and colors
    sb->obj_parm[type] = ks_allocate_gl_shader_regular_parm(ver_cnt,ver_cnt+2);
    obj_parm = sb->obj_parm[type];
    for(i = 0; i < 2; i++){
      for(j = 0; j < div; j++){
	obj_parm->elem[i*div+j].x = r*cos(angle*j);
	obj_parm->elem[i*div+j].y = r*sin(angle*j);
	obj_parm->elem[i*div+j].z = i*len;
	obj_parm->elem[i*div+j].nx = cos(angle*j);
	obj_parm->elem[i*div+j].ny = sin(angle*j);
	obj_parm->elem[i*div+j].nz = 0.0f;
	obj_parm->elem[i*div+j].r = color[0];
	obj_parm->elem[i*div+j].g = color[1];
	obj_parm->elem[i*div+j].b = color[2];
      }
    }
    // set indices for rod faces
    for(i = 0; i < div; i++){
      obj_parm->index[i*2]   = i+div;
      obj_parm->index[i*2+1] = i;
    }
    obj_parm->index[i*2]   = div;
    obj_parm->index[i*2+1] = 0;
  } else {
    // setting for rod lines
    ks_assert(0); // not implemented
    sb->obj_parm[type] = ks_allocate_gl_shader_regular_parm(2,2);
    obj_parm = sb->obj_parm[type];
    for(i = 0; i < 2; i++){
      obj_parm->elem[i].x = 0.0f;
      obj_parm->elem[i].y = 0.0f;
      obj_parm->elem[i].z = i*len;
      obj_parm->elem[i].nx = 0.0f;
      obj_parm->elem[i].ny = 0.0f;
      obj_parm->elem[i].nz = 1.0f;
      obj_parm->elem[i].r = color[0];
      obj_parm->elem[i].g = color[1];
      obj_parm->elem[i].b = color[2];
      obj_parm->index[i] = i;
    }
  }
}
static void get_euler_angle(GLfloat sx, GLfloat sy, GLfloat sz, GLfloat ex, GLfloat ey, GLfloat ez,
			    float* angle, float* length)
{
  // set euler angle to connect two positions with a rod
  float v[3];
  float len,xy_len;
  v[0] = ex - sx;
  v[1] = ey - sy;
  v[2] = ez - sz;
  len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
  xy_len = sqrt(v[0]*v[0]+v[1]*v[1]);
  if(xy_len == 0.0){
    angle[0] = 0.0;
  } else {
    angle[0] = acos(v[0]/xy_len);
    angle[0] = v[1] > 0 ? angle[0] : 2.0*M_PI - angle[0];
  }
  angle[1] = acos(v[2]/len);
  angle[2] = 0.0;
  *length = len;
}
static BOOL make_rod_shader_draw_info(CV_SHADER_BASE *sb, CV_FRAME *fr, int draw_mode)
{
  CV_PARTICLE *p, *pb;
  CV_BOND *bond;
  BOOL rod_flg;
  GLfloat rod_color[4] = {0.0,0.0,0.0,1.0};
  float rod_r = 0.1;
  int i, bond_cnt = 0;
  // count number of bonds
  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(bond = p->bond; bond != NULL; bond = bond->next){
      pb = bond->p;
      if(bond->type != NULL && bond->type->radius < 0){
	/* line_flg = KS_TRUE; */
	rod_flg = KS_FALSE;
      } else {
	/* line_flg = line; */
	rod_flg = KS_TRUE;
      }
      if(rod_flg == KS_TRUE && p->label < pb->label && 
	 (draw_mode != CV_DRAW_MODE_MAIN_CHAIN || 
	  (draw_mode == CV_DRAW_MODE_MAIN_CHAIN && 
	   p->flags&CV_PARTICLE_MAIN && 
	   pb->flags&CV_PARTICLE_MAIN))){
	bond_cnt++;
      }
    }
  }
  // allocate draw info
  if(sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL] == NULL){
    sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL] = ks_allocate_gl_shader_regular_draw_info(bond_cnt);
  } else {
    if(ks_resize_gl_shader_regular_draw_info(sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL],bond_cnt)
       == KS_FALSE){
      return KS_FALSE;
    }
  }
  // set positions and euler angles of rods
  i = 0;
  for(p = fr->particle_end; p != NULL; p = p->prev){
    for(bond = p->bond; bond != NULL; bond = bond->next){
      pb = bond->p;
      if(bond->type != NULL && bond->type->radius < 0){
	/* line_flg = KS_TRUE; */
	rod_flg = KS_FALSE;
      } else {
	/* line_flg = line; */
	rod_flg = KS_TRUE;
      }
      set_rod_r_and_color(bond,KS_FALSE,rod_color,&rod_r);
      if(rod_flg == KS_TRUE && p->label < pb->label && 
	 (draw_mode != CV_DRAW_MODE_MAIN_CHAIN || 
	  (draw_mode == CV_DRAW_MODE_MAIN_CHAIN && 
	   p->flags&CV_PARTICLE_MAIN && 
	   pb->flags&CV_PARTICLE_MAIN))){
	sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]->elem[i].trans[0] = p->cd[0];
	sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]->elem[i].trans[1] = p->cd[1];
	sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]->elem[i].trans[2] = p->cd[2];
	get_euler_angle(p->cd[0],p->cd[1],p->cd[2],
			pb->cd[0],pb->cd[1],pb->cd[2],
			sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]->elem[i].angle_euler_zyz,
			&sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]->elem[i].scale[2]);
	i++;
      }
    }
  }
  assert(i == bond_cnt);
  set_model_matrix(sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]);
  return KS_TRUE;
}
static BOOL init_rod_draw_info(CV_BASE *cb, CV_FRAME *fr)
{
  CV_SHADER_BASE *sb = cb->sb;
  ks_set_gl_shader_draw_mode(sb->shader[SHADER_REGULAR],OBJ_TYPE_ROD_NORMAL,
			     GL_TRIANGLE_STRIP); // use GL_TRIANGLE_STRIP to draw rod
  make_rod_shader_obj_parm(cb->bond_type,sb,OBJ_TYPE_ROD_NORMAL);
  make_rod_shader_obj_parm(cb->bond_type,sb,OBJ_TYPE_ROD_LOW);

  // set vertices for rod
  ks_init_gl_standard_shader_regular(sb->shader[SHADER_REGULAR],
				     OBJ_TYPE_ROD_NORMAL,sb->obj_parm[OBJ_TYPE_ROD_NORMAL],-1);
  ks_init_gl_standard_shader_regular(sb->shader[SHADER_REGULAR],
				     OBJ_TYPE_ROD_LOW,sb->obj_parm[OBJ_TYPE_ROD_LOW],-1);
  if(make_rod_shader_draw_info(sb,fr,cb->draw_mode) == KS_FALSE){
    return KS_FALSE;
  }
  ks_set_gl_shader_regular_draw_info(sb->shader[SHADER_REGULAR],OBJ_TYPE_ROD_NORMAL,
				     sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL],
				     sb->shader_parm[SHADER_PARM_NORMAL],-1);
  ks_set_gl_shader_regular_draw_info(sb->shader[SHADER_REGULAR],OBJ_TYPE_ROD_LOW,
				     NULL,
				     sb->shader_parm[SHADER_PARM_NORMAL],OBJ_TYPE_ROD_NORMAL);
  return KS_TRUE;
}
static BOOL update_shader_rod_draw_info(CV_BASE *cb, CV_FRAME *fr)
{
  // update draw info of rods when number of rods is changed
  CV_SHADER_BASE *sb = cb->sb;
  if(make_rod_shader_draw_info(sb,fr,cb->draw_mode) == KS_FALSE){
    return KS_FALSE;
  }
  // update base draw info and shared draw info
  ks_update_gl_shader_regular_draw_info_share(sb->shader[SHADER_REGULAR],
					      OBJ_TYPE_ROD_NORMAL,OBJ_TYPE_ROD_LOW+1,
					      sb->obj_draw_info[OBJ_TYPE_ROD_NORMAL]);
  return KS_TRUE;
}
static void process_shader_pick(KS_GL_BASE *gb, CV_SHADER_BASE *sb, unsigned int pick_id)
{
  int i,j,n;
  for(i = 0; i < SHADER_TYPE_CNT; i++){
    if(i == SHADER_SPHERE){
      n = 1;
    } else if(i == SHADER_REGULAR){
      n = OBJ_TYPE_CNT;
    } else {
      n = 0;
    }
    for(j = 0; j < n; j++){
      if(ks_check_gl_shader_pick_id(sb->shader[i],j,&pick_id) == KS_TRUE){
	if(i == SHADER_SPHERE){
	  gb->hit_name_num = 2;
	  gb->hit_name[0] = CV_NAME_PARTICLE;
	  gb->hit_name[1] = sb->sphere_draw_info_particle[pick_id]->label;
	  /*
	  printf("hit id:%u file_label:%u label:%u\n",
		 pick_id,
		 sb->sphere_draw_info_particle[pick_id]->file_label,
		 sb->sphere_draw_info_particle[pick_id]->label);
	  */
	}
	goto HIT_PICK_OUT;
      }
    }
  }
 HIT_PICK_OUT:;
}
#endif

/*
  Field Viewer ver 0.33
*/

#define VER 0.33

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef ICL
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <sys/stat.h>
#include <stdarg.h>

#include <GL/glut.h>

#include "ks_std.h"
#include "ks_gl.h"
#include "ks_math.h"

#include "cv304.h"
#include "fv033.h"

struct { /* color map */
  int x,y;
  int w,h;
  int flg;
  int v[2];
} cm;
int color_edit = KS_FALSE;

struct { /* check cursor */
  int cd[3];
} cc;

#define HIT_SURFACE
#ifdef HIT_SURFACE
static int hit_surface[2] = {-1,-1};
#endif

/*
enum {
  DRAW_FRAME   = 0x01,
  DRAW_FIELD   = 0x02,
  DRAW_CONTOUR = 0x04,
  DRAW_SURFACE = 0x08
};
unsigned int draw_mode = DRAW_FRAME | DRAW_FIELD | DRAW_CONTOUR;
*/
enum {
  DRAW_FRAME    = 0,
  DRAW_FIELD    = 1,
  DRAW_CONTOUR  = 2,
  DRAW_SURFACE  = 3,
  DRAW_PARTICLE = 4,
  DRAW_NUM      = 5
};
unsigned int draw_mode[DRAW_NUM];

enum {
  SAVE_BMP = 0,
  SAVE_POVRAY = 1,
  SAVE_MAX = 1
};
unsigned int save_mode = SAVE_BMP;

int surface_calculated;
int idle_counter = 0;

char **file;
int file_pos = 0;
int file_num;

int water_flg = KS_FALSE;
int bubble_flg = KS_FALSE;

GLfloat conp[20][10][3];
GLfloat conpc[20][3];
int conpn = -1;
int conpm[20];

void save_povray(const char *name);
//static void set_field_color(double val, FV_CONTOUR_SIZE cs, GLfloat *color);
static int count_contour_fan(FV_CONTOUR_FAN *listp);
static void free_contour_fan(FV_CONTOUR_FAN *listp);
static void free_contour(FV_CONTOUR *listp);
static BOOL fv_gldraw_contour(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_CONTOUR_SIZE cn, FV_TARGET tg,
			      int dm);
static void fv_draw_frame(KS_GL_BASE *gb, FV_FIELD ****fi, FV_FIELD_SIZE fs);

void fv_increase_draw_contour_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  if(draw_mode[DRAW_CONTOUR] == FV_DRAW_FILL){
    draw_mode[DRAW_CONTOUR] = 0;
  } else {
    draw_mode[DRAW_CONTOUR]++;
  }
}
void fv_increase_draw_surface_mode(KS_GL_BASE *gb, int x, int y, void *vp)
{
  if(draw_mode[DRAW_SURFACE] == FV_DRAW_NUM-1){
    draw_mode[DRAW_SURFACE] = 0;
  } else {
    draw_mode[DRAW_SURFACE]++;
  }
}
void fv_increase_target_surface(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i,i0;
  CV_BASE *cb;
  CV_FRAME *fr;
  cb = (CV_BASE*)vp;
  if(cb->fv_target.level == NULL){
    return;
  }
  fr = &cb->frame[cb->current_frame];
  i0 = 0;
  for(i = fr->fv->contour_size.num-1; i >= 0; i--){
    if(cb->fv_target.level[i] == 1){
      if(i != fr->fv->contour_size.num-1){
	cb->fv_target.level[i+1] = 1;
	cb->fv_target.level[i] = 0;
      } else {
	cb->fv_target.level[fr->fv->contour_size.num-1] = 0;
      }
      i0 = 1;
    }
  }
  if(i0 == 0)
    cb->fv_target.level[0] = 1;
  /*
  for(i = 0; i < fr->fv->contour_size.num; i++){
    if(cb->fv_target.level[i] == 1){
      printf("%f\n",fr->fv->contour_size.level[i]);
    }
  }
  */
}
void fv_decrease_target_surface(KS_GL_BASE *gb, int x, int y, void *vp)
{
  int i,i0;
  CV_BASE *cb;
  CV_FRAME *fr;
  cb = (CV_BASE*)vp;
  if(cb->fv_target.level == NULL){
    return;
  }
  fr = &cb->frame[cb->current_frame];
  i0 = 0;
  for(i = 0; i < fr->fv->contour_size.num; i++){
    if(cb->fv_target.level[i] == 1){
      if(i != 0){
	cb->fv_target.level[i-1] = 1;
	cb->fv_target.level[i] = 0;
      } else {
	cb->fv_target.level[0] = 0;
      }
      i0 = 1;
    }
  }
  if(i0 == 0){
    cb->fv_target.level[fr->fv->contour_size.num-1] = 1;
  }
  /*
  for(i = 0; i < fr->fv->contour_size.num; i++){
    if(cb->fv_target.level[i] == 1){
      printf("%f\n",fr->fv->contour_size.level[i]);
    }
  }
  */
}
void fv_move_backward_contour_plane(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  if(cb->fv_target.pl_sign > 0){
    if(cb->fv_target.num[cb->fv_target.pl] == 0){
      cb->fv_target.num[cb->fv_target.pl] = fr->fv->field_size.n[cb->fv_target.pl]-1; 
    } else {
      cb->fv_target.num[cb->fv_target.pl]--;
    }
  } else {
    if(cb->fv_target.num[cb->fv_target.pl] == fr->fv->field_size.n[cb->fv_target.pl]-1){
      cb->fv_target.num[cb->fv_target.pl] = 0;
    } else {
      cb->fv_target.num[cb->fv_target.pl]++;
    }
  }
}
void fv_move_forward_contour_plane(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  if(cb->fv_target.pl_sign < 0){
    if(cb->fv_target.num[cb->fv_target.pl] == 0){
      cb->fv_target.num[cb->fv_target.pl] = fr->fv->field_size.n[cb->fv_target.pl]-1; 
    } else {
      cb->fv_target.num[cb->fv_target.pl]--;
    }
  } else {
    if(cb->fv_target.num[cb->fv_target.pl] == fr->fv->field_size.n[cb->fv_target.pl]-1){
      cb->fv_target.num[cb->fv_target.pl] = 0;
    } else {
      cb->fv_target.num[cb->fv_target.pl]++;
    }
  }
}
void fv_change_target_column(KS_GL_BASE *gb, int x, int y, void *vp)
{
  CV_BASE *cb;
  CV_FRAME *fr;
  char add_text[64] = {"fdv column: "};
  char del_text[64] = {"fdv column: "};

  cb = (CV_BASE*)vp;
  fr = &cb->frame[cb->current_frame];
  sprintf(del_text,"fdv column: %d",cb->fv_target.val+1);
  if(cb->fv_target.val == fr->fv->field_size.val_num-1){
    cb->fv_target.val = 0;
  } else {
    cb->fv_target.val++;
  }
  sprintf(add_text,"fdv column: %d",cb->fv_target.val+1);
  fv_set_max_min(fr->fv->field,&fr->fv->field_size,cb->fv_target.val);
  if(cb->fv_level[KS_RANGE_MAX][cb->fv_target.val] != NULL){
    fr->fv->field_size.max = *cb->fv_level[KS_RANGE_MAX][cb->fv_target.val];
  }
  if(cb->fv_level[KS_RANGE_MIN][cb->fv_target.val] != NULL){
    fr->fv->field_size.min = *cb->fv_level[KS_RANGE_MIN][cb->fv_target.val];
  }
  if(fv_init_field(fr->fv->field,fr->fv->field_size,&fr->fv->contour_size,&fr->fv->contour_fan,
		   &cb->fv_target,CV_FDVIEW_CONTOUR_LEVEL_NUM,fr->fv->black_and_white)
     == KS_FALSE){
    return;
  }
  cb->telop = ks_del_text_list(cb->telop,del_text);
  cb->telop = ks_add_text_list(cb->telop,ks_new_text_list(add_text,5));
}
void set_active_contour_plane(KS_GL_BASE *gb, FV_TARGET *tg)
{
  int i;
  double axis[3][3] = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}};
  double gl_axis[3][3];

  for(i = 0; i < 3; i++){
    ks_multi_gl_rotational_matrix(*gb,axis[i],gl_axis[i]);
    /*    printf("%d %f %f %f\n",i,gl_axis[i][0],gl_axis[i][1],gl_axis[i][2]);*/
  }
  if(fabs(gl_axis[0][0]) > fabs(gl_axis[1][0]) && fabs(gl_axis[0][0]) > fabs(gl_axis[2][0])){
    tg->pl = 0;
    if(gl_axis[0][0] > 0) tg->pl_sign = 1; else tg->pl_sign = -1;
  } else if(fabs(gl_axis[1][0]) > fabs(gl_axis[2][0])){
    tg->pl = 1;
    if(gl_axis[1][0] > 0) tg->pl_sign = 1; else tg->pl_sign = -1;
  } else {
    tg->pl = 2;
    if(gl_axis[2][0] > 0) tg->pl_sign = 1; else tg->pl_sign = -1;
  }
}
void fv_draw_field(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms,FV_CONTOUR_SIZE cn,
		   FV_CONTOUR_FAN ***csf, FV_TARGET *tg, FILE *pov)
{
  if(pov != NULL){
    fv_save_povray_mesh(gb,mn,ms,cn,csf,*tg,pov);
    return;
  }
  glPushMatrix();
  //  glTranslated(-ms.ccd[0],-ms.ccd[1],-ms.ccd[2]);
  /*
  printf("%f %f %f\n",ms.ccd[0],ms.ccd[1],ms.ccd[2]);
  */
  glDisable(GL_LIGHTING);
  glLineWidth(1.0);

  if(draw_mode[DRAW_FRAME] != FV_DRAW_OFF){
    fv_draw_frame(gb,mn,ms);
  }

  if(draw_mode[DRAW_FIELD] != FV_DRAW_OFF){
    glLoadName(1);
    fv_gldraw_field(mn,ms,*tg,cn,draw_mode[DRAW_FIELD]);
  }

  if(draw_mode[DRAW_CONTOUR] != FV_DRAW_OFF){
    glLoadName(2);
    set_active_contour_plane(gb,tg);
    fv_gldraw_contour(mn,ms,cn,*tg,draw_mode[DRAW_CONTOUR]);
  }

  if(draw_mode[DRAW_SURFACE] != FV_DRAW_OFF){
    if(draw_mode[DRAW_SURFACE] == FV_DRAW_FILL || draw_mode[DRAW_SURFACE] == FV_DRAW_TRANS)
      glEnable(GL_LIGHTING);
    fv_gldraw_contour_surface(cn,*tg,csf,draw_mode[DRAW_SURFACE]);
  }

  glEnable(GL_LIGHTING);

  glPopMatrix();
}
void draw_gl_sphere(double x, double y, double z, GLfloat c0, GLfloat c1, GLfloat c2, GLfloat c3,
		    double r, int ditail0, int ditail1)
{
  GLfloat color[4];
  color[0] = c0; color[1] = c1; color[2] = c2; color[3] = c3;
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glPushMatrix();
  glTranslated(x,y,z);
  ks_draw_gl_sphere(r,ditail1);
  glPopMatrix();
}
#if 0
static void __KS_USED__ draw_cursor(KS_GL_BASE *gb, FV_FIELD ****mn,FV_FIELD_SIZE ms,FV_TARGET tg)
{
  int i,j,k;
  GLfloat color[4];
  GLfloat co[10][4] = {
    {1.0, 0.0, 0.0, 1.0},
    {0.0, 1.0, 0.0, 1.0},
    {0.0, 0.0, 1.0, 1.0},
    {0.0, 1.0, 1.0, 1.0},
    {1.0, 0.0, 1.0, 1.0},
    {1.0, 1.0, 0.0, 1.0},
    {0.5, 0.0, 0.0, 1.0},
    {0.0, 0.5, 0.0, 1.0},
    {0.0, 0.0, 0.5, 1.0},
    {0.5, 0.5, 0.5, 1.0}
  };
  FV_FIELD *pf;//,*pfn;
  FV_CONTOUR *pc;
  FV_CONTOUR *pcs[10];
  int pcs_num;
  //  double dd[2][3];
  //  double vc[2][3],nv[3];
  char c0[256];
  double sr = .01;
  int cursor_font = KS_GL_FONT_HELVETICA_12;

  pf = mn[cc.cd[0]][cc.cd[1]][cc.cd[2]];

  glPushMatrix();
  glTranslated(-ms.ccd[0],-ms.ccd[1],-ms.ccd[2]);

  glPushMatrix();
  glTranslated(pf->cd[0],pf->cd[1],pf->cd[2]);
  color[0] = 1.0; color[1] = 1.0; color[2] = 1.0; color[3] = 1.0;
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glutSolidSphere(.1,10,5);
  glPopMatrix();

  /*
  glPopMatrix();
  for(i = 0; i < 3; i++){
    for(j = 0; j < 2; j++){
      if(pf->l[i][j] != NULL){
	glPushMatrix();
	glTranslated(pf->l[i][j]->cd[0],pf->l[i][j]->cd[1],pf->l[i][j]->cd[2]);
	color[0] = 0.0; color[1] = 1.0; color[2] = 1.0; color[3] = 1.0;
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
	glutSolidSphere(.1,10,5);
	glPopMatrix();
      }
    }
  }
  glPopMatrix();
  */
  pcs_num = 0;
  glDisable(GL_LIGHTING);
  for(i = 0; i < 3; i++){
    if(pf->l[i][1] != NULL){
      ks_draw_gl_line(pf->cd[0],pf->cd[1],pf->cd[2],
		      pf->l[i][1]->cd[0],pf->l[i][1]->cd[1],pf->l[i][1]->cd[2],1,1,1,1);
      for(pc = pf->c[i][1]; pc != NULL; pc = pc->next){
	if(pc->vn == tg.val){
	  if(tg.level[pc->ln] == 1){
	    glEnable(GL_LIGHTING);
	    draw_gl_sphere(pc->cd[0],pc->cd[1],pc->cd[2]
			   ,co[pcs_num][0],co[pcs_num][1],co[pcs_num][2],co[pcs_num][3]
			   ,sr,10,5);
	    glDisable(GL_LIGHTING);
	    ks_draw_gl_line(pc->cd[0],pc->cd[1],pc->cd[2],
			    pc->cd[0]+pc->nv[0],
			    pc->cd[1]+pc->nv[1],
			    pc->cd[2]+pc->nv[2],1,1,1,2);
	    ks_use_glut_font(gb,cursor_font);
	    sprintf(c0,"%d",pc);
	    glRasterPos3d(pc->cd[0],pc->cd[1],pc->cd[2]);
	    glCallLists(strlen(c0), GL_BYTE, c0);
	    pcs[pcs_num++] = pc;
	  }
	}
      }
      for(j = 0; j < 3; j++){
	if(pf->l[i][1]->l[j][1] != NULL && pf->l[i][1]->l[j][1] != pf && i != j){
	  ks_draw_gl_line(pf->l[i][1]->cd[0],pf->l[i][1]->cd[1],pf->l[i][1]->cd[2],
			  pf->l[i][1]->l[j][1]->cd[0],
			  pf->l[i][1]->l[j][1]->cd[1],
			  pf->l[i][1]->l[j][1]->cd[2],1,1,1,1);
	  for(pc = pf->l[i][1]->c[j][1]; pc != NULL; pc = pc->next){
	    if(pc->vn == tg.val){
	      if(tg.level[pc->ln] == 1){
		glEnable(GL_LIGHTING);
		draw_gl_sphere(pc->cd[0],pc->cd[1],pc->cd[2]
			       ,co[pcs_num][0],co[pcs_num][1],co[pcs_num][2],co[pcs_num][3]
			       ,sr,10,5);
		glDisable(GL_LIGHTING);
		ks_draw_gl_line(pc->cd[0],pc->cd[1],pc->cd[2],
				pc->cd[0]+pc->nv[0],
				pc->cd[1]+pc->nv[1],
				pc->cd[2]+pc->nv[2],1,1,1,2);
		ks_use_glut_font(gb,cursor_font);
		sprintf(c0,"%d",pc);
		glRasterPos3d(pc->cd[0],pc->cd[1],pc->cd[2]);
		glCallLists(strlen(c0), GL_BYTE, c0);
		pcs[pcs_num++] = pc;
	      }
	    }
	  }
	}
	if((i == 1 && j == 2) || (i == 2 && j == 0) || (i == 0 && j == 1) ){
	  if(i == 1 && j == 2) k = 0;
	  if(i == 2 && j == 0) k = 1;
	  if(i == 0 && j == 1) k = 2;
	  ks_draw_gl_line(pf->l[i][1]->l[j][1]->cd[0],
			  pf->l[i][1]->l[j][1]->cd[1],
			  pf->l[i][1]->l[j][1]->cd[2],
			  pf->l[i][1]->l[j][1]->l[k][1]->cd[0],
			  pf->l[i][1]->l[j][1]->l[k][1]->cd[1],
			  pf->l[i][1]->l[j][1]->l[k][1]->cd[2],1,1,1,1);
	  for(pc = pf->l[i][1]->l[j][1]->c[k][1]; pc != NULL; pc = pc->next){
	    if(pc->vn == tg.val){
	      if(tg.level[pc->ln] == 1){
		glEnable(GL_LIGHTING);
		draw_gl_sphere(pc->cd[0],pc->cd[1],pc->cd[2]
			       ,co[pcs_num][0],co[pcs_num][1],co[pcs_num][2],co[pcs_num][3]
			       ,sr,10,5);
		glDisable(GL_LIGHTING);
		ks_draw_gl_line(pc->cd[0],pc->cd[1],pc->cd[2],
				pc->cd[0]+pc->nv[0],
				pc->cd[1]+pc->nv[1],
				pc->cd[2]+pc->nv[2],1,1,1,2);
		ks_use_glut_font(gb,cursor_font);
		sprintf(c0,"%d",pc);
		glRasterPos3d(pc->cd[0],pc->cd[1],pc->cd[2]);
		glCallLists(strlen(c0), GL_BYTE, c0);
		pcs[pcs_num++] = pc;
	      }
	    }
	  }
	}
      }
    }
  }

  if(conpn > 0){
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    for(i = 0; i < conpn; i++){
      glColor3fv(conpc[i]);
      glBegin(GL_POLYGON);
      for(j = 0; j < conpm[i]; j++){
	//	printf("%d %d %f %f %f\n",i,j,conp[i][j][0],conp[i][j][1],conp[i][j][2]);
	glVertex3fv(conp[i][j]);
      }
      glEnd();
    }
    glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);
  }

#if 0
  if(pcs_num > 0){
    for(i = 0; i < pcs_num; i++){
      printf("%d %d %f %f %f\n",i,pcs[i],pcs[i]->cd[0],pcs[i]->cd[1],pcs[i]->cd[2]);
      c = 0;
      for(j = 0; j < 3; j++){
	for(k = 0; k < 2; k++){
	  if(pcs[i]->c[j][k] != NULL){
	    /*
	    printf("  %d %d %d %f %f %f\n",j,k,pcs[i]->c[j][k]
		   ,pcs[i]->c[j][k]->cd[0],pcs[i]->c[j][k]->cd[1],pcs[i]->c[j][k]->cd[2]);
	    */
	    for(i0 = 0; i0 < pcs_num; i0++){
	      if(pcs[i0] == pcs[i]->c[j][k]){
		/*		printf("hit\n");*/
		pcs[i]->loop[c++] = pcs[i]->c[j][k];
	      }
	    }
	  }
	}
      }
      for(j = 0; j < 2; j++){
	printf("  %d %d %f %f %f\n",j,pcs[i]->loop[j]
	       ,pcs[i]->loop[j]->cd[0]
	       ,pcs[i]->loop[j]->cd[1]
	       ,pcs[i]->loop[j]->cd[2]);
	for(k = 0; k < 3; k++)
	  vc[j][k] = pcs[i]->loop[j]->cd[k] - pcs[i]->cd[k];
      }
      nv[0] = vc[0][1]*vc[1][2] - vc[0][2]*vc[1][1];
      nv[1] = vc[0][2]*vc[1][0] - vc[0][0]*vc[1][2];
      nv[2] = vc[0][0]*vc[1][1] - vc[0][1]*vc[1][0];
      printf("  (% f % f % f)(% f % f % f) % f\n"
	     ,pcs[i]->nv[0],pcs[i]->nv[1],pcs[i]->nv[2],nv[0],nv[1],nv[2]
	     ,pcs[i]->nv[0]*nv[0]+pcs[i]->nv[1]*nv[1]+pcs[i]->nv[2]*nv[2]);
      if(pcs[i]->nv[0]*nv[0]+pcs[i]->nv[1]*nv[1]+pcs[i]->nv[2]*nv[2] > 0)
	printf("  -> %d\n",pcs[i]->loop[0]);
      else
	printf("  -> %d\n",pcs[i]->loop[1]);
    }

    if(pcs_num > 3){
      for(j = 0; j < 3; j++){
	dd[0][j] = 0;
	dd[1][j] = 0;
      }
      for(i = 0; i < pcs_num; i++){
	for(j = 0; j < 3; j++){
	  dd[0][j] += pcs[i]->cd[j];
	  dd[1][j] += pcs[i]->nv[j];
	}
      }
      for(j = 0; j < 3; j++){
	dd[0][j] /= pcs_num;
	dd[1][j] /= pcs_num;
      }

      glEnable(GL_LIGHTING);
      draw_gl_sphere(dd[0][0],dd[0][1],dd[0][2],1.0,1.0,1.0,1.0,.1,10,5);
      glDisable(GL_LIGHTING);
      ks_draw_gl_line(dd[0][0],dd[0][1],dd[0][2],
		      dd[0][0]+dd[1][0],
		      dd[0][1]+dd[1][1],
		      dd[0][2]+dd[1][2],1,1,1,2);
    }
  }
#endif

  glPopMatrix();
  glEnable(GL_LIGHTING);
}
#endif
BOOL fv_hit_color_map_xy(int x, int y)
{
  return x >= cm.x && x <= cm.x+10 && y <= cm.y && y >= cm.y - cm.h;
}
BOOL fv_hit_color_map_y(int y)
{
  return y <= cm.y && y >= cm.y - cm.h;
}
int fv_get_color_map_num(int x, int y,FV_CONTOUR_SIZE cn)
{
  return (cm.y-y)/(cm.h/(cn.num+1));
  /*
  if(x >= cm.x && x <= cm.x+10 && y <= cm.y && y >= cm.y - cm.h){
    return (cm.y-y)/(cm.h/(cn.num+1));
  } else {
    return -1;
  }
  */
}
void fv_display_2d(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn,
		   FV_TARGET tg, int width, int height, BOOL black_and_white)
{
  int i;
  int ix,iy;
  int len,len_max;
  int color_hight;
  double val_max,val_factor;
  double ret_len;
  double d0,d1,d2;
  char c0[256];
  GLfloat color[3];

  /*
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,width,0,height);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  */

  {
    /*    ix = width - cm.x;*/
    ix = cm.x;
    iy = height - cm.y;

    if(black_and_white == KS_TRUE){
      int bg_shift = -10;
      int bg_width = 90;
      int bg_height = 30;
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      //      glColor4f(0.0,0.0,0.0,0.8);
      glColor4f(gb->background_color[0],
		gb->background_color[1],
		gb->background_color[2],
		0.8);
      glBegin(GL_QUADS);
      glVertex2i(ix+bg_shift,iy-bg_height);
      glVertex2i(ix+bg_shift+bg_width,iy-bg_height);
      glVertex2i(ix+bg_shift+bg_width,iy+cm.h+bg_height);
      glVertex2i(ix+bg_shift,iy+cm.h+bg_height);
      glEnd();
      glDisable(GL_BLEND);
    }

    /*
    glBegin(GL_QUAD_STRIP);
    for(i = 0; i < 100; i++){
      ks_hsv2rgb(240-(double)i/100.*240,1.0,1.0,&d0,&d1,&d2);
      color[0] = d0; color[1] = d1; color[2] = d2;
      glColor3fv(color);
      glVertex2f(0.0+ix,(GLfloat)i/100*cm.h+iy);
      glVertex2f(10.0+ix,(GLfloat)i/100*cm.h+iy);
    }
    glEnd();
    */
#if 0
    glBegin(GL_LINES);
    for(i = 0; i < cm.h; i++){
      /*
      i0 = fv_get_level((ms.max-ms.min)/cm.h*i+ms.min,cn);
      if(i0 == 0)
	ks_hsv2rgb(240.0,1.0,1.0,&d0,&d1,&d2);
      else
	ks_hsv2rgb(240-(cn.level[i0-1]-ms.min)/(ms.max-ms.min)*240,1.0,1.0,&d0,&d1,&d2);
      color[0] = d0; color[1] = d1; color[2] = d2;
      */
      /*
      fv_get_level_color((ms.max-ms.min)/cm.h*i+ms.min,ms.max,ms.min,cn,color);
      glColor3fv(color);
      */
      /*
      for(i = 0; i < cm.h; i++){
	printf("%d %f %d\n",i,(ms.max-ms.min)/cm.h*i+ms.min,i0);
      }
      */
      /*
      printf("%d %f %d\n",i,(float)i/(cm.h)*(cn.num+1),(int)((float)i/(cm.h)*(cn.num+1)));
      */
      glColor3fv(cn.color[(int)((float)i/(cm.h)*(cn.num+1))]);
      glVertex2i(ix,   i+iy);
      glVertex2i(ix+10,i+iy);
    }
    glEnd();
#else
    color_hight = cm.h/(cn.num+1);
    glBegin(GL_QUAD_STRIP);
    for(i = 0; i < cn.num+1; i++){
      glColor3fv(cn.color[i]);
      glVertex2i(ix,   i*color_hight+iy);
      glVertex2i(ix+10,i*color_hight+iy);
      glVertex2i(ix,   (i+1)*color_hight+iy);
      glVertex2i(ix+10,(i+1)*color_hight+iy);
      //      printf("%d %d\n",i,i*color_hight);
    }
    glEnd();
#endif

    ks_gl_base_foreground_color(gb);
    ks_use_glut_font(gb,KS_GL_FONT_8x13);

    val_max = fabs(ms.max);
    if(val_max < fabs(ms.min)) val_max = fabs(ms.min);
    for(i = 0; i < cn.num; i++){
      if(val_max < fabs(cn.level[i])) val_max = fabs(cn.level[i]);
    }
    if(log10(val_max) > 4){
      val_factor = pow(10,-(int)log10(val_max));
      sprintf(c0,"[x%.0e]",1.0/val_factor);
      glRasterPos2i(ix+15,iy+cm.h+10);
      glCallLists(strlen(c0), GL_BYTE, c0);
    } else {
      val_factor = 1;
    }
    /*    printf("%f %f %e\n",val_max,log10(val_max),val_factor);*/

    len_max = 0;
    sprintf(c0,"%.2f",ms.max*val_factor);
    ks_get_glut_font_width(gb,KS_GL_FONT_8x13,c0,&len);
    if(len_max < len) len_max = len;
    sprintf(c0,"%.2f",ms.min*val_factor);
    ks_get_glut_font_width(gb,KS_GL_FONT_8x13,c0,&len);
    if(len_max < len) len_max = len;
    for(i = 0; i < cn.num; i++){
      sprintf(c0,"%.2f",cn.level[i]*val_factor);
      ks_get_glut_font_width(gb,KS_GL_FONT_8x13,c0,&len);
      if(len_max < len) len_max = len;
    }

    /*    ix -= 85;*/
    ix += len_max+20;
    iy -= 5;
    sprintf(c0,"%.2f",ms.max*val_factor);
    ks_get_glut_font_width(gb,KS_GL_FONT_8x13,c0,&len);
    glRasterPos2i(ix-len,(ms.max-ms.min)/(ms.max-ms.min)*cm.h+iy);
    glCallLists(strlen(c0), GL_BYTE, c0);
    sprintf(c0,"%.2f",ms.min*val_factor);
    ks_get_glut_font_width(gb,KS_GL_FONT_8x13,c0,&len);
    glRasterPos2i(ix-len,(ms.min-ms.min)/(ms.max-ms.min)*cm.h+iy);
    glCallLists(strlen(c0), GL_BYTE, c0);
    for(i = 0; i < cn.num; i++){
      if(tg.level[i] == 1){
	/*
	ks_hsv2rgb(240-(cn.level[i]-ms.min)/(ms.max-ms.min)*240,1.0,1.0,&d0,&d1,&d2);
	color[0] = d0; color[1] = d1; color[2] = d2;
	glColor3fv(color);
	*/
	glColor3fv(cn.color[i]);
      } else {
	ks_gl_base_foreground_color(gb);
      }
      sprintf(c0,"%.2f",cn.level[i]*val_factor);
      ks_get_glut_font_width(gb,KS_GL_FONT_8x13,c0,&len);
      glRasterPos2i(ix-len,(cn.level[i]-ms.min)/(ms.max-ms.min)*cm.h+iy);
      glCallLists(strlen(c0), GL_BYTE, c0);
    }
    if(cm.flg != 0){
      /*      ix = width - cm.x;*/
      ix = cm.x;
      iy = height - cm.y;
      ks_gl_base_foreground_color(gb);
      glBegin(GL_LINE_LOOP);
      glVertex2i(ix,cm.v[0]+iy);
      glVertex2i(ix+cm.w,cm.v[0]+iy);
      glVertex2i(ix+cm.w,cm.v[1]+iy);
      glVertex2i(ix,cm.v[1]+iy);
      glEnd();
    }
    if(color_edit == KS_TRUE){
      int len = 240;

      ix = width - cm.x - 100 - len;
      iy = height - cm.y;

      glBegin(GL_LINES);
      
      for(i = 0; i < len ; i++){
	ks_hsv2rgb(240-(double)i/len*240,1.0,1.0,&d0,&d1,&d2);
	color[0] = d0; color[1] = d1;color[2] = d2;
	glColor3fv(color);
	glVertex2i(ix+i,iy);
	glVertex2i(ix+i,iy+cm.h);
      }
      glEnd();
    }

    ret_len = 13+2;
    if(draw_mode[DRAW_CONTOUR] != FV_DRAW_OFF){
      ix = cm.x;
      iy = height - cm.y - 25;
      ks_gl_base_foreground_color(gb);
      if(tg.pl == 0){
	sprintf(c0,"x =%.2f",mn[tg.num[0]][0][0]->cd[0]);
      } else if(tg.pl == 1){
	sprintf(c0,"y =%.2f",mn[0][tg.num[1]][0]->cd[1]);
      } else if(tg.pl == 2){
	sprintf(c0,"z =%.2f",mn[0][0][tg.num[2]]->cd[2]);
      }
      ks_use_glut_font(gb,KS_GL_FONT_8x13);
      glRasterPos2i(ix,iy);
      glCallLists(strlen(c0), GL_BYTE, c0);
      iy -= ret_len;
    }

#ifdef CURSOR
    sprintf(c0,"%d %d %d",cc.cd[0],cc.cd[1],cc.cd[2]);
    ks_use_glut_font(gb,"TIMES_ROMAN_24");
    glRasterPos2i(ix,iy);
    glCallLists(strlen(c0), GL_BYTE, c0);
    iy -= ret_len;
#endif

  }

  /*
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0,width,0,height,-100,100);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);
  glPopMatrix();
  */
}
int a2p(int i0, int i1)
{
  int i;
  int flg[3] = {0,0,0};

  if(i0 > 2 || i1 > 2){
    fprintf(stderr,"error in a2p\n");
  } else {
    flg[i0] = 1;
    flg[i1] = 1;
  }
  for(i = 0; i < 3; i++){
    if(flg[i] == 0){
      return i;
    }
  }
  return -1;
}
void fv_save_povray(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn,
		    FV_CONTOUR_FAN ***csf, FV_TARGET tg, char *name)
{
  int i,j;
  int i1;
  double d3,d4,d5;
  double dd[9][3];
  char file_name[1024];
  FILE *fp;

  FV_CONTOUR_FAN *fa;

  strcpy(file_name,name);
  strcat(file_name,".pov");
  printf("save povray file %s\n",file_name);
  if ( ( fp = fopen( file_name, "wt" ) ) == NULL ){
    printf("file open error %s\n",file_name);
    ks_exit(EXIT_FAILURE);
  }

  fprintf(fp,"camera{\n");
  if(gb->projection == 1){
    /*
      fprintf(fp," location <%f,%f,%f>\n",-100.0,0.0,0.0);
    */
    fprintf(fp," orthographic\n");
    fprintf(fp," location <%f,%f,%f>\n",-gb->eye_len*100./570.+330*100./570.-100.0,0.0,0.0);
    fprintf(fp," right <%f,0,0>\n",fabs(-gb->eye_len*100./570.+330*100./570.-100.0));
    fprintf(fp," up <0,%f,0>\n",fabs(-gb->eye_len*100./570.+330*100./570.-100.0));
  } else {
    fprintf(fp," location <%f,%f,%f>\n",-gb->eye_len,0.0,0.0);
    /*      fprintf(fp," location <%f,%f,%f>\n",trans[0]-gb->eye_len,trans[1],trans[2]);*/
    fprintf(fp," right <1,0,0>\n");
    fprintf(fp," up <0,1,0>\n");
  }
  fprintf(fp," sky <0,0,1>\n");
  fprintf(fp," look_at <0,0,0>\n");
  if(gb->projection != 1)
    fprintf(fp," angle 30\n");
  fprintf(fp,"}\n");
  /*
  d3 = -100000; d4 = 100000; d5 = 100000;
  fprintf(fp,"light_source {<%f,%f,%f> color rgb<1,1,1>}\n",d3,d4,d5);
  */
  d3 = -1.0; d4 = 1.1; d5 = 1.2;
  fprintf(fp,"light_source {<%f,%f,%f> color rgb<1,1,1> parallel point_at <0,0,0>}\n",d3,d4,d5);

  fprintf(fp,"background {color rgb<%f,%f,%f>}\n"
	  ,gb->background_color[0],gb->background_color[1],gb->background_color[2]);

  if(draw_mode[DRAW_SURFACE] != FV_DRAW_OFF){

    for(i = 0; i < cn.num; i++){
      if(tg.level[i] == 1){

	if(bubble_flg == KS_TRUE){
	  /*
	  fprintf(fp,"plane{x,300\n");
	  fprintf(fp,"pigment{ checker color rgb<0.8,0.8,0.8>, color rgb<.6,.6,.6> scale 10.0}\n");
	  fprintf(fp,"}\n");
	  */
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[0]        [0]        [0]        ->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[0]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[ms.n[0]-1][0]        [0]        ->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[1]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[0]        [ms.n[1]-1][0]        ->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[2]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[ms.n[0]-1][ms.n[1]-1][0]        ->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[3]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[0]        [0]        [ms.n[2]-1]->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[4]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[ms.n[0]-1][0]        [ms.n[2]-1]->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[5]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[0]        [ms.n[1]-1][ms.n[2]-1]->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[6]);
	  for(i1 = 0; i1 < 3; i1++) dd[8][i1] = mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[i1];
	  ks_multi_gl_rotational_matrix(*gb,dd[8],dd[7]);

	  fprintf(fp,"mesh{\n");
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		  ,-dd[1][0]-gb->trans[0],dd[1][1]+gb->trans[1],dd[1][2]+gb->trans[2]
		  ,-dd[2][0]-gb->trans[0],dd[2][1]+gb->trans[1],dd[2][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[1][0]-gb->trans[0],dd[1][1]+gb->trans[1],dd[1][2]+gb->trans[2]
		  ,-dd[2][0]-gb->trans[0],dd[2][1]+gb->trans[1],dd[2][2]+gb->trans[2]
		  ,-dd[3][0]-gb->trans[0],dd[3][1]+gb->trans[1],dd[3][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		  ,-dd[1][0]-gb->trans[0],dd[1][1]+gb->trans[1],dd[1][2]+gb->trans[2]
		  ,-dd[4][0]-gb->trans[0],dd[4][1]+gb->trans[1],dd[4][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[1][0]-gb->trans[0],dd[1][1]+gb->trans[1],dd[1][2]+gb->trans[2]
		  ,-dd[4][0]-gb->trans[0],dd[4][1]+gb->trans[1],dd[4][2]+gb->trans[2]
		  ,-dd[5][0]-gb->trans[0],dd[5][1]+gb->trans[1],dd[5][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		  ,-dd[2][0]-gb->trans[0],dd[2][1]+gb->trans[1],dd[2][2]+gb->trans[2]
		  ,-dd[4][0]-gb->trans[0],dd[4][1]+gb->trans[1],dd[4][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[2][0]-gb->trans[0],dd[2][1]+gb->trans[1],dd[2][2]+gb->trans[2]
		  ,-dd[4][0]-gb->trans[0],dd[4][1]+gb->trans[1],dd[4][2]+gb->trans[2]
		  ,-dd[6][0]-gb->trans[0],dd[6][1]+gb->trans[1],dd[6][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[4][0]-gb->trans[0],dd[4][1]+gb->trans[1],dd[4][2]+gb->trans[2]
		  ,-dd[5][0]-gb->trans[0],dd[5][1]+gb->trans[1],dd[5][2]+gb->trans[2]
		  ,-dd[6][0]-gb->trans[0],dd[6][1]+gb->trans[1],dd[6][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[5][0]-gb->trans[0],dd[5][1]+gb->trans[1],dd[5][2]+gb->trans[2]
		  ,-dd[6][0]-gb->trans[0],dd[6][1]+gb->trans[1],dd[6][2]+gb->trans[2]
		  ,-dd[7][0]-gb->trans[0],dd[7][1]+gb->trans[1],dd[7][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[2][0]-gb->trans[0],dd[2][1]+gb->trans[1],dd[2][2]+gb->trans[2]
		  ,-dd[3][0]-gb->trans[0],dd[3][1]+gb->trans[1],dd[3][2]+gb->trans[2]
		  ,-dd[6][0]-gb->trans[0],dd[6][1]+gb->trans[1],dd[6][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[3][0]-gb->trans[0],dd[3][1]+gb->trans[1],dd[3][2]+gb->trans[2]
		  ,-dd[6][0]-gb->trans[0],dd[6][1]+gb->trans[1],dd[6][2]+gb->trans[2]
		  ,-dd[7][0]-gb->trans[0],dd[7][1]+gb->trans[1],dd[7][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[1][0]-gb->trans[0],dd[1][1]+gb->trans[1],dd[1][2]+gb->trans[2]
		  ,-dd[3][0]-gb->trans[0],dd[3][1]+gb->trans[1],dd[3][2]+gb->trans[2]
		  ,-dd[5][0]-gb->trans[0],dd[5][1]+gb->trans[1],dd[5][2]+gb->trans[2]);
	  fprintf(fp,"  triangle{<%f,%f,%f>,\n  <%f,%f,%f>,\n  <%f,%f,%f>}\n"
		  ,-dd[3][0]-gb->trans[0],dd[3][1]+gb->trans[1],dd[3][2]+gb->trans[2]
		  ,-dd[5][0]-gb->trans[0],dd[5][1]+gb->trans[1],dd[5][2]+gb->trans[2]
		  ,-dd[7][0]-gb->trans[0],dd[7][1]+gb->trans[1],dd[7][2]+gb->trans[2]);
	  fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .7}\n"
		    ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  /*	  fprintf(fp,"  interior{ ior 1.3 caustics 0.7}\n");*/
	  fprintf(fp,"}\n");
	}

	fprintf(fp,"mesh{\n");
	for(fa = csf[i][tg.val]; fa != NULL; fa = fa->next){

	  for(j = 1; j < fa->num; j++){
	    for(i1 = 0; i1 < 3; i1++){
	      dd[2][i1] = fa->pcs[0]->cd[i1]-ms.ccd[i1];
	    }
	    ks_multi_gl_rotational_matrix(*gb,dd[2],dd[0]);
	    ks_multi_gl_rotational_matrix(*gb,fa->pcs[0]->nv,dd[1]);
	    fprintf(fp,"  smooth_triangle {<%f,%f,%f>,<%f,%f,%f>,\n"
		    ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		    ,-dd[1][0],dd[1][1],dd[1][2]);
	    for(i1 = 0; i1 < 3; i1++){
	      dd[2][i1] = fa->pcs[j]->cd[i1]-ms.ccd[i1];
	    }
	    ks_multi_gl_rotational_matrix(*gb,dd[2],dd[0]);
	    ks_multi_gl_rotational_matrix(*gb,fa->pcs[j]->nv,dd[1]);
	    fprintf(fp,"                   <%f,%f,%f>,<%f,%f,%f>,\n"
		    ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		    ,-dd[1][0],dd[1][1],dd[1][2]);
	    if(j != fa->num-1 || fa->num == 3){
	      for(i1 = 0; i1 < 3; i1++){
		dd[2][i1] = fa->pcs[j+1]->cd[i1]-ms.ccd[i1];
	      }
	      ks_multi_gl_rotational_matrix(*gb,dd[2],dd[0]);
	      ks_multi_gl_rotational_matrix(*gb,fa->pcs[j+1]->nv,dd[1]);
	      fprintf(fp,"                   <%f,%f,%f>,<%f,%f,%f>}\n"
		      ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		      ,-dd[1][0],dd[1][1],dd[1][2]);
	      if(fa->num == 3) break;
	    } else {
	      for(i1 = 0; i1 < 3; i1++){
		dd[2][i1] = fa->pcs[1]->cd[i1]-ms.ccd[i1];
	      }
	      ks_multi_gl_rotational_matrix(*gb,dd[2],dd[0]);
	      ks_multi_gl_rotational_matrix(*gb,fa->pcs[1]->nv,dd[1]);
	      fprintf(fp,"                   <%f,%f,%f>,<%f,%f,%f>}\n"
		      ,-dd[0][0]-gb->trans[0],dd[0][1]+gb->trans[1],dd[0][2]+gb->trans[2]
		      ,-dd[1][0],dd[1][1],dd[1][2]);
	    }
	  }
	}
	if(water_flg == KS_TRUE){
	  fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .7}\n"
		    ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  fprintf(fp,"  finish{ diffuse 0.9 ambient 0.4 reflection 0.2 specular 0.5}\n");
	  fprintf(fp,"  interior{ ior 1.3 caustics 0.7}\n");
	} else if(bubble_flg == KS_TRUE){
	  fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .4}\n"
		  ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  fprintf(fp,"  finish{ diffuse 0.9 ambient 0.4 reflection 0.2 specular 0.5}\n");
	  fprintf(fp,"  interior{ ior .77 caustics 0.7}\n");
	} else {
	  if(draw_mode[DRAW_SURFACE] == FV_DRAW_TRANS)
	    fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .45}\n"
		    ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  else
	    fprintf(fp,"  pigment{ color rgb <%f,%f %f>}\n"
		    ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  fprintf(fp,"  finish{ diffuse 0.9 ambient 0.4}\n");
	}
	//	fprintf(fp,"  no_shadow\n");
	fprintf(fp,"}\n");
      }
    }
  }
  fclose(fp);
}
void fv_save_povray_mesh(KS_GL_BASE *gb, FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn,
			 FV_CONTOUR_FAN ***csf, FV_TARGET tg, FILE *fp)
{
  int i,j;

  FV_CONTOUR_FAN *fa;

  if(draw_mode[DRAW_SURFACE] != FV_DRAW_OFF){

    for(i = 0; i < cn.num; i++){
      if(tg.level[i] == 1){

	fprintf(fp,"mesh{\n");
	for(fa = csf[i][tg.val]; fa != NULL; fa = fa->next){

	  for(j = 1; j < fa->num; j++){
	    fprintf(fp,"  smooth_triangle {<%f,%f,%f>,<%f,%f,%f>,\n"
		    ,fa->pcs[0]->cd[0],fa->pcs[0]->cd[1],fa->pcs[0]->cd[2]
		    ,fa->pcs[0]->nv[0],fa->pcs[0]->nv[1],fa->pcs[0]->nv[2]);
	    fprintf(fp,"                   <%f,%f,%f>,<%f,%f,%f>,\n"
		    ,fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2]
		    ,fa->pcs[j]->nv[0],fa->pcs[j]->nv[1],fa->pcs[j]->nv[2]);
	    if(j != fa->num-1 || fa->num == 3){
	      fprintf(fp,"                   <%f,%f,%f>,<%f,%f,%f>}\n"
		      ,fa->pcs[j+1]->cd[0],fa->pcs[j+1]->cd[1],fa->pcs[j+1]->cd[2]
		      ,fa->pcs[j+1]->nv[0],fa->pcs[j+1]->nv[1],fa->pcs[j+1]->nv[2]);
	      if(fa->num == 3) break;
	    } else {
	      fprintf(fp,"                   <%f,%f,%f>,<%f,%f,%f>}\n"
		      ,fa->pcs[1]->cd[0],fa->pcs[1]->cd[1],fa->pcs[1]->cd[2]
		      ,fa->pcs[1]->nv[0],fa->pcs[1]->nv[1],fa->pcs[1]->nv[2]);
	    }
	  }
	}
	if(water_flg == KS_TRUE){
	  fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .7}\n"
		  ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  fprintf(fp,"  finish{ diffuse 0.9 ambient 0.4 reflection 0.2 specular 0.5}\n");
	  fprintf(fp,"  interior{ ior 1.3 caustics 0.7}\n");
	} else if(bubble_flg == KS_TRUE){
	  fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .4}\n"
		  ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  fprintf(fp,"  finish{ diffuse 0.9 ambient 0.4 reflection 0.2 specular 0.5}\n");
	  fprintf(fp,"  interior{ ior .77 caustics 0.7}\n");
	} else {
	  if(draw_mode[DRAW_SURFACE] == FV_DRAW_TRANS)
	    fprintf(fp,"  pigment{ color rgb <%f,%f %f> filter .45}\n"
		    ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  else
	    fprintf(fp,"  pigment{ color rgb <%f,%f %f>}\n"
		    ,cn.color[i][0],cn.color[i][1],cn.color[i][2]);
	  fprintf(fp,"  finish{ diffuse 0.9 ambient 0.4}\n");
	}
	//	fprintf(fp,"  no_shadow\n");
	fprintf(fp,"}\n");
      }
    }
  }
}
void copy_cd(double *cd, void* vp, int flg)
{
  int i;

  for(i = 0; i < 3; i++){
    if(flg == 0)
      cd[i] = ((FV_FIELD*)(vp))->cd[i];
    else
      cd[i] = ((FV_CONTOUR*)(vp))->cd[i];
  }
}
BOOL fv_init_field(FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE *cn, FV_CONTOUR_FAN ****csf, 
		   FV_TARGET *tg, int contour_level_num, BOOL black_and_white)
{
  int i,j;
  static BOOL init_flg = KS_FALSE;

  cm.x = 20;
  cm.y = 240;
  cm.w = 10;
  cm.h = 200;
  cm.flg = 0;

  /*
  {
    int ix,iy,iz;
    FV_FIELD *pf;
    for(ix = 0; ix < ms.n[0]; ix++){
      for(iy = 0; iy < ms.n[1]; iy++){
	for(iz = 0; iz < ms.n[2]; iz++){
	  if(mn[ix][iy][iz] != NULL){
	    pf = mn[ix][iy][iz];
	    printf("%d %d %d %p\n",ix,iy,iz,pf->c[0][0]);
	  }
	}
      }
    }
    ks_exit(EXIT_FAILURE);
  }
  */
  /*
  if((fp = fopen(file_name,"rt")) == NULL){
    printf("file open error %s\n",file_name);
    ks_exit(EXIT_FAILURE);
  }

  if(fv_read_field_file(fp,&mn,&ms) == 0){
    return 0;
  }

  fclose(fp);
  */
  /*  
  for(i2 = 0; i2 < ms.n[2]; i2++){
    for(i1 = 0; i1 < ms.n[1]; i1++){
      for(i0 = 0; i0 < ms.n[0]; i0++){
	printf("%d %d %d % f % f % f  % f % f % f\n",i0,i1,i2,
	       mn[i0][i1][i2].cd[0],
	       mn[i0][i1][i2].cd[1],
	       mn[i0][i1][i2].cd[2],
	       mn[i0][i1][i2].s[0],
	       mn[i0][i1][i2].s[1],
	       mn[i0][i1][i2].s[2]);
      }
    }
  }
  */
  /*
  for(p = &mn[0][0][0]; p != NULL; p = p->l[2][1]){
    printf("% f % f % f  % f % f % f\n",
	   p->cd[0], p->cd[1], p->cd[2],
	   p->s[0], p->s[1], p->s[2]);
  }
  */

  if(init_flg == KS_FALSE){
    tg->pl = 0;
    /*  tg->val = 0;  init in cv_allocate_base*/ 
    for(i = 0; i < 3; i++){
      tg->num[i] = ms.n[i]/2;
    }
  }
  /*
  printf("tg->num %d %d %d\n",tg->num[0],tg->num[1],tg->num[2]);
  printf("%d %d %d\n",ms.n[0]/2,ms.n[1]/2,ms.n[2]/2);
  ks_exit(EXIT_FAILURE);
  */
  /*
  if((ct = (FV_CONTOUR****)malloc(3*sizeof(FV_CONTOUR***))) == NULL){
    ks_error_memory();
    ks_exit(EXIT_FAILURE);
  }
  for(i = 0; i < 3; i++){
    if((ct[i] = (FV_CONTOUR***)malloc(ms.n[i]*sizeof(FV_CONTOUR**))) == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
  }
  for(i = 0; i < 3; i++){
    for(j = 0; j < ms.n[i]; j++){
      ct[i][j] = NULL;
    }
  }
  */

  cn->num = contour_level_num;
  if(cn->level == NULL){
    if((cn->level = (double*)ks_malloc(cn->num*sizeof(double),"cn->level")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    if((cn->color = (float**)ks_malloc((cn->num+1)*sizeof(float*),"cn->color")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < cn->num+1; i++){
      if((cn->color[i] = (float*)ks_malloc(3*sizeof(float),"cn->color")) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    }
  }
  /*
  if((*color_map) == NULL){
    if(((*color_map) = cv_allocate_color_map()) == NULL){
      return KS_FALSE;
    }
    (*color_map)->num = contour_level_num;
    if(cv_allocate_color_map_color((*color_map)) == KS_FALSE){
      return KS_FALSE;
    }
    fv_set_level(ms.min, ms.max,cn,(*color_map));
  } else {
    fv_set_level(ms.min, ms.max,cn,NULL);
  }
  */
  fv_set_level(ms.min, ms.max,cn,black_and_white);
  /*
  for(i = 0; i < cn->num; i++){
    printf("%d %f\n",i,cn->level[i]);
  }
  for(i = 0; i < cn->num+1; i++){
    printf("%2d %f %f %f\n",i,cn->color[i][0],cn->color[i][1],cn->color[i][2]);
  }
  */
  /*  ks_exit(EXIT_FAILURE);*/

  if(tg->level == NULL){
    if((tg->level = (int*)ks_malloc(cn->num*sizeof(int),"tg->level")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < cn->num; i++){
      tg->level[i] = 0;
    }
  }

  //  fv_init_range_history(RANGE_HIS_NUM,rh);
  /*
  fv_set_renge_history(ms.min,ms.max,cn->level);
  */

  /*
  printf("%f %f\n",cn->level[0]-d0,cn->level[cn->num-1]+d0);
  ks_exit(EXIT_FAILURE);
  */
  /*
  printf("%d %d\n",cn->num,ms.val_num);
  */
  if((*csf) == NULL){
    if(((*csf) = (FV_CONTOUR_FAN***)ks_malloc(cn->num*sizeof(FV_CONTOUR_FAN**),"csf")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < cn->num; i++){
      if(((*csf)[i]=(FV_CONTOUR_FAN**)ks_malloc(ms.val_num*sizeof(FV_CONTOUR_FAN*),"csf[i]"))
	 ==NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    }
    for(i = 0; i < cn->num; i++)
      for(j = 0; j < ms.val_num; j++)
	(*csf)[i][j] = NULL;
  }
  /*
  printf("calc_field_contour\n");
  */

  if(fv_calc_field_contour(mn,ms,*cn) == KS_FALSE){
    return KS_FALSE;
  }

  cc.cd[0] = ms.n[0]/2;
  cc.cd[1] = ms.n[1]/2+2;
  cc.cd[2] = 0;
  /*
  printf("calc_contour_surface\n");
  */
  fv_calc_contour_surface(mn,ms,*cn,*csf);

  if(init_flg == KS_FALSE){
    draw_mode[DRAW_FRAME] = FV_DRAW_LINE;
    draw_mode[DRAW_FIELD] = FV_DRAW_OFF;
    /*  draw_mode[DRAW_FIELD] = FV_DRAW_LINE;*/
    draw_mode[DRAW_CONTOUR] = FV_DRAW_LINE;
    draw_mode[DRAW_SURFACE] = FV_DRAW_OFF;
    draw_mode[DRAW_SURFACE] = FV_DRAW_FILL;
    draw_mode[DRAW_PARTICLE] = FV_DRAW_OFF;
  }
  init_flg = KS_TRUE;

  return KS_TRUE;
}
void fv_debug_mode(int mode)
{
  if(mode == 0){
    draw_mode[DRAW_FRAME] = FV_DRAW_LINE;
    draw_mode[DRAW_FIELD] = FV_DRAW_TRANS;
    draw_mode[DRAW_CONTOUR] = FV_DRAW_TRANS;
    draw_mode[DRAW_SURFACE] = FV_DRAW_OFF;
    draw_mode[DRAW_PARTICLE] = FV_DRAW_OFF;
  }
}
void fv_reconstruct_contour(FV_BASE *fv)
{
  FV_FIELD ****mn;
  FV_FIELD_SIZE ms;
  FV_CONTOUR_SIZE *cn;
  FV_CONTOUR_FAN ****csf;

  mn = fv->field;
  ms = fv->field_size;
  cn = &fv->contour_size;
  csf = &fv->contour_fan;
  fv_calc_field_contour(mn,ms,*cn);
  fv_calc_contour_surface(mn,ms,*cn,*csf);
}
void fv_free_target(FV_TARGET tg)
{
  if(tg.level != NULL)
    ks_free(tg.level);
}
void fv_free_contour_fan(FV_CONTOUR_FAN ***cf,FV_CONTOUR_SIZE cs, FV_FIELD_SIZE ms)
{
  int i,j;
  FV_CONTOUR_FAN *fa;

  for(i = 0; i < cs.num; i++){
    for(j = 0; j < ms.val_num; j++){
      for(fa = cf[i][j]; fa != NULL; fa = fa->next){
	if(fa->num != 3){
	  free_contour(fa->pcs[0]);
	}
	ks_free(fa->pcs);
      }
      free_contour_fan(cf[i][j]);
    }
  }
  for(i = 0; i < cs.num; i++){
    ks_free(cf[i]);
  }
  ks_free(cf);
}
void fv_free_contour_size(FV_CONTOUR_SIZE cs)
{
  int i;
  ks_free(cs.level);
  for(i = 0; i < cs.num+1; i++){
    ks_free(cs.color[i]);
  }
  ks_free(cs.color);
}
/*
static void fv_idle(FV_FIELD ****mn, FV_FIELD_SIZE ms, FV_CONTOUR_SIZE cn)
{

  ms.max -= 300;
  printf("%d %f\n",idle_counter,ms.max);
  fv_set_level(ms.min,ms.max,&cn,color_map);
  fv_calc_field_contour(mn,ms,cn);

  idle_counter++;
  if(idle_counter == 10)
    glutIdleFunc(NULL);

  glutPostRedisplay();
}
*/
#if 0
int main(int argc, char** argv)
{
  int i;
  double d0;

  int full_flg;
  int tx_flg = 0;
  int x_size,y_size;
  int x_pos,y_pos;
  int stereo_flg = 0;
  BOOL proj_flg = 0;
  double ax,ay,az;
  double tx,ty,tz;
  int max_flg = 0, min_flg = 0;
  double max = 0.0,min = 0.0;

  KS_OPTIONS *ops = NULL;

  x_pos = 50; y_pos = 0; 
  x_size = 450;  y_size = 450;
  ax = 0; ay = 0 ;az = 0;
  tx = 0; ty = 0; tz = 0;

  gb = ks_allocate_gl_base();
  /*  gb->eye_len = 120;*/
  cn.num = 10;

  ks_set_options("-full",0,KS_OPTIONS_BOOL,&full_flg,&ops,"full screen mode");
  ks_set_options("-stereo",1,KS_OPTIONS_INT,&stereo_flg,&ops,"stereo mode");
  ks_set_options("-j",1,KS_OPTIONS_BOOL,&proj_flg,&ops,"ortho mode");
  ks_set_options("-w",1,KS_OPTIONS_INT,&x_size,&ops,"width of window (default 450)");
  ks_set_options("-h",1,KS_OPTIONS_INT,&y_size,&ops,"height of window (default 450)");
  ks_set_options("-x",1,KS_OPTIONS_INT,&x_pos,&ops,"x position of window (default 50)");
  ks_set_options("-y",1,KS_OPTIONS_INT,&y_pos,&ops,"x position of window (default 0)");
  ks_set_options("-l",1,KS_OPTIONS_INT,&gb->eye_len,&ops,"distance of eye from (0,0,0)");
  ks_set_options("-ax",1,KS_OPTIONS_DOUBLE,&ax,&ops,"x angle (default 0)");
  ks_set_options("-ay",1,KS_OPTIONS_DOUBLE,&ay,&ops,"y angle (default 0)");
  ks_set_options("-az",1,KS_OPTIONS_DOUBLE,&az,&ops,"z angle (default 0)");
  ks_set_options("-tx",1,KS_OPTIONS_DOUBLE,&tx,&ops,"x trans (default 0)");
  ks_set_options("-ty",1,KS_OPTIONS_DOUBLE,&ty,&ops,"y trans (default 0)");
  ks_set_options("-tz",1,KS_OPTIONS_DOUBLE,&tz,&ops,"z trans (default 0)");

  ks_set_options("-max",1,KS_OPTIONS_DOUBLE,&max,&ops,"upper limit of color map");
  ks_set_options("-min",1,KS_OPTIONS_DOUBLE,&min,&ops,"lower limit of color map");
  ks_set_options("-level_num",1,KS_OPTIONS_INT,&cn.num,&ops,"number of contours (default 10)");
  ks_set_options("-water",0,KS_OPTIONS_BOOL,&water_flg,&ops,"water flag for povray");
  ks_set_options("-bubble",0,KS_OPTIONS_BOOL,&bubble_flg,&ops,"bubble flag for povray");

  if(argc == 1){
    printf("Particles coordination viewer \"fdview ver.%.2f\" by using OpenGL ",VER);
    printf("Copyright (C) 2005 Koishi\n");
    printf("Usage: fdview [FILENAME]\n");
    ks_output_options_comment(ops,stdout);
    ks_exit(EXIT_FAILURE);
  } else {
    ks_classify_options(argc,argv,ops,&file,&file_num,NULL);

    if(ks_get_options_hit("-max",ops) == KS_TRUE)
      max_flg = 1; else max_flg = 0;
    if(ks_get_options_hit("-min",ops) == KS_TRUE)
      min_flg = 1; else min_flg = 0;
  }

  if(stereo_flg == 0){
    ks_gl_base_set_stereo_mode(gb,KS_GL_BASE_STEREO_NOT_USE);
  } else if(stereo_flg == 1){
    ks_gl_base_set_stereo_mode(gb,KS_GL_BASE_STEREO_QUAD_BUFFER);
  } else if(stereo_flg == 2){
    ks_gl_base_set_stereo_mode(gb,KS_GL_BASE_STEREO_HORIZONTAL_SLIT);
  } else if(stereo_flg == 3){
    ks_gl_base_set_stereo_mode(gb,KS_GL_BASE_STEREO_VERTICAL_SLIT);
  } else {
    ks_error("unknown stereo type %d\n",stereo_flg);
    ks_exit(EXIT_FAILURE);
  }
  if(proj_flg == KS_TRUE){
    ks_gl_base_set_projection_mode(gb,KS_GL_BASE_ORTHO);
  } else {
    ks_gl_base_set_projection_mode(gb,KS_GL_BASE_PERSPECTIVE);
  }

  if(file_num == 0){
    printf("no file\n");
    ks_exit(EXIT_FAILURE);
  }

  /*
  printf("%d %d %d\n",gb->stereo_flg,max_flg,min_flg);
  */
  /*
  printf("%d\n",file_num);
  for(i = 0; i< file_num; i++){
    printf("%d %s\n",i,file[i]);
  }
  */
  /*
  printf("%d\n",cn.num);
  ks_exit(EXIT_FAILURE);
  */
  //  printf("read_file\n");
  if(fv_read_field_file(file[0],&mn,&ms) == 0){
    fprintf(stderr,"%s\n",error_msg);
    ks_exit(EXIT_FAILURE);
  }
  //  printf("set_max_min\n");
  fv_set_max_min(mn,&ms,tg.val);

  if(max_flg == 1)
    ms.max = max;
  if(min_flg == 1)
    ms.min = min;

  //  printf("init_field\n");

  if(init_field() == 0){
    fprintf(stderr,"%s\n",error_msg);
    ks_exit(EXIT_FAILURE);
  }

  //  printf("start_gl\n");

  glutInit(&argc, argv);
  if(stereo_flg == 1)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO);
  else 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
  glutInitWindowSize(x_size, y_size);
  glutInitWindowPosition(x_pos, y_pos);
  glutCreateWindow("fdview");
  init_gl();

  ks_gl_base_set_translational_array(gb,tx,ty,tz
				     ,(mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[0]-
				       mn[0][0][0]->cd[0])*1.2);
  ks_gl_base_set_rotational_matrix(gb,ax,ay,az);

  glutDisplayFunc(display); 
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  //  glutIdleFunc(idle);
  glutSpecialFunc(special_key);
  glutMainLoop();
  return KS_FALSE;
}
#endif
void fv_set_level(double min, double max, FV_CONTOUR_SIZE *cs, BOOL black_and_white)
{
  int i;
  double d0,d1,d2,d3;

  d0 = (max-min)/(cs->num+1);
  for(i = 0; i < cs->num; i++){
    cs->level[i] = d0*(i+1) + min;
    /*    printf("%d %f\n",i,cs->level[i]);*/
  }
  if(black_and_white == KS_TRUE){
    for(i = 0; i < cs->num+1; i++){
      cs->color[i][0] = (double)i/cs->num;
      cs->color[i][1] = (double)i/cs->num;
      cs->color[i][2] = (double)i/cs->num;
    }
  } else {
    for(i = 0; i < cs->num+1; i++){
      /*    printf("%d %f\n",i,240-(double)i/cs->num*240);*/
      ks_hsv2rgb(240-(double)i/cs->num*240,1.0,1.0,&d1,&d2,&d3);
      cs->color[i][0] = d1; cs->color[i][1] = d2; cs->color[i][2] = d3;
    }
  }
}
int fv_get_level(double val, FV_CONTOUR_SIZE cs)
{
  int i;

  if(val < cs.level[0]){
    return 0;
  } else if(val >= cs.level[cs.num-1]){
    return cs.num;
  } else {
    for(i = 0; i < cs.num-1; i++){
      if(val >= cs.level[i] && val < cs.level[i+1])
	return i+1;
    }
  }
  return -1;
}
void fv_get_level_color(double val, double max, double min, FV_CONTOUR_SIZE cs, GLfloat *color)
{
  int i0;
  double d0,d1,d2;
  i0 = fv_get_level(val,cs);
  if(i0 == 0)
    ks_hsv2rgb(240.0,1.0,1.0,&d0,&d1,&d2);
  else
    ks_hsv2rgb(240-(cs.level[i0-1]-min)/(max-min)*240,1.0,1.0,&d0,&d1,&d2);
  color[0] = d0; color[1] = d1; color[2] = d2;
}
void fv_set_max_min(FV_FIELD ****fi, FV_FIELD_SIZE *fs, int val_num)
{
  int i;
  int i0,i1,i2;

  if(val_num >= 0 && val_num < fs->val_num){
    fs->max = fi[0][0][0]->s[val_num];
    fs->min = fi[0][0][0]->s[val_num];
  } else {
    fs->max = fi[0][0][0]->s[0];
    fs->min = fi[0][0][0]->s[0];
  }

  for(i2 = 0; i2 < fs->n[2]; i2++){
    for(i1 = 0; i1 < fs->n[1]; i1++){
      for(i0 = 0; i0 < fs->n[0]; i0++){
	if(fi[i0][i1][i2] != NULL){
	  /*
	    printf("%d %d %d % f % f % f\n",i0,i1,i2,
	    fi[i0][i1][i2]->cd[0],
	    fi[i0][i1][i2]->cd[1],
	    fi[i0][i1][i2]->cd[2]);
	  */
	  if(val_num >= 0 && val_num < fs->val_num){
	    i = val_num;
	    if(fs->min > fi[i0][i1][i2]->s[i]) fs->min = fi[i0][i1][i2]->s[i];
	    if(fs->max < fi[i0][i1][i2]->s[i]) fs->max = fi[i0][i1][i2]->s[i];
	  } else {
	    for(i = 0; i < fs->val_num; i++){
	      if(fs->min > fi[i0][i1][i2]->s[i]) fs->min = fi[i0][i1][i2]->s[i];
	      if(fs->max < fi[i0][i1][i2]->s[i]) fs->max = fi[i0][i1][i2]->s[i];
	    }
	  }
	}
      }
    }
  }
  /*  printf("%d %f %f\n",val_num,fs->min,fs->max);*/
}
BOOL fv_read_field_file(char *file_name, FV_FIELD *****fi, FV_FIELD_SIZE *fs, char* file_name_cdv)
{
  int i,j,k;
  int i0,i1;
  int ii[3];
  char c0[1024],c1[256];
  char *cp;
  double d0;
  double offset[FV_MAX_COLUMN_NUM];
  char *n_name[] = {"fv_xn","fv_yn","fv_zn"};
  int line_num;
  FILE *fp;

  if((fp = fopen(file_name,"rt")) == NULL){
    ks_error_file(file_name);
    return KS_FALSE;
  }
  for(i = 0; i < 3; i++)
    fs->n[i] = 0;
  for(i = 0; i < FV_MAX_COLUMN_NUM; i++)
    offset[i] = 0;
  while(fgets(c0,sizeof(c0),fp) != NULL){
    /*    printf("%s",c0);*/
    if(ks_strval(c0,"fv_xn=",&d0)){
      fs->n[0] = (int)(d0+.5);
    }
    if(ks_strval(c0,"fv_yn=",&d0)){
      fs->n[1] = (int)(d0+.5);
    }
    if(ks_strval(c0,"fv_zn=",&d0)){
      fs->n[2] = (int)(d0+.5);
    }
    if(strstr(c0,"fv_offset") != NULL){
      for(i = 0; i < FV_MAX_COLUMN_NUM; i++){
	sprintf(c1,"fv_offset%d=",i+1);
	if(ks_strval(c0,c1,&d0)){
	  /*	  printf("%d %s %f\n",i,c1,d0);*/
	  offset[i] = d0;
	}
      }
    }
    cp = strstr(c0,"fv_cdv_file=");
    if(cp != NULL){
      cp += strlen("fv_cdv_file=");
      for(i = 0; cp[i] != '\n' && cp[i] != '\0' && i < sizeof(c1); i++)
	c1[i] = cp[i];
      c1[i] = '\0';
      strcpy(file_name_cdv,c1);
    }

    if(!ks_iscomment(c0)) break;
  }
  /*
  for(i = 0; i < FV_MAX_COLUMN_NUM; i++)
    printf("%d %f\n",i,offset[i]);
  ks_exit(EXIT_FAILURE);
  */
  /*
  printf("%d %d %d\n",fs->n[0],fs->n[1],fs->n[2]);
  */
  rewind(fp);

  if(fs->n[0] <= 0 || fs->n[1] <= 0 || fs->n[2] <= 0){
    ks_error("Illegal field data size (0)");
    return KS_FALSE;
  }

  if(strlen(c0) == 0){
    ks_error("Illegal field data file");
    return KS_FALSE;
  }
  fs->val_num = 0;
  for(i = 0; i < strlen(c0)-1; i++){
    /*    printf("%d %c %x %d\n",i,c0[i],c0[i],fs->val_num);*/
    if(ks_isfloat(c0[i]) && !ks_isfloat(c0[i+1])) fs->val_num++;
  }
  if(fs->val_num < 7){
    ks_error("Illegal field data structure (less than 7)");
    return KS_FALSE;
  }
  if(fs->val_num > FV_MAX_COLUMN_NUM){
    ks_error("Field data column is too large (max is %d)",FV_MAX_COLUMN_NUM-6);
    return KS_FALSE;
  }

  if((*fi) == NULL){
    if(((*fi) = (FV_FIELD****)ks_malloc(fs->n[0]*sizeof(FV_FIELD***),"fi")) == NULL){
      ks_error("memory error in fv_read_field_file");
      return KS_FALSE;
    }
    for(i = 0; i < fs->n[0]; i++)
      if(((*fi)[i] = (FV_FIELD***)ks_malloc(fs->n[1]*sizeof(FV_FIELD**),"fi[i]")) == NULL){
	ks_error("memory error in fv_read_field_file");
	return KS_FALSE;
      }
    for(i = 0; i < fs->n[0]; i++)
      for(j = 0; j < fs->n[1]; j++)
	if(((*fi)[i][j] = (FV_FIELD**)ks_malloc(fs->n[2]*sizeof(FV_FIELD*),"fi[i][j]")) == NULL){
	  ks_error("memory error in fv_read_field_file");
	  return KS_FALSE;
	}
    for(i = 0; i < fs->n[0]; i++)
      for(j = 0; j < fs->n[1]; j++)
	for(k = 0; k < fs->n[2]; k++)
	  (*fi)[i][j][k] = NULL;
    /*
    for(i = 0; i < fs->n[0]; i++)
      for(j = 0; j < fs->n[1]; j++)
	for(k = 0; k < fs->n[2]; k++)
	  if(((*fi)[i][j][k].s = (double*)malloc(fs->val_num*sizeof(double))) == NULL){
	    ks_error("memory error in ks_read_field_file");
	    return KS_FALSE;
	  }
    */
  }
  line_num = 0;
  while(fgets(c0,sizeof(c0),fp) != NULL){
    line_num++;
    if(!ks_iscomment(c0)){
      /*      printf("%s",c0);*/
      i0 = 0;
      for(i = 0; i < fs->val_num; i++){
	for(; c0[i0] == ' '; i0++);
	for(i1 = 0; ks_isfloat(c1[i1] = c0[i0]); i0++,i1++);
	c1[i1] = '\0';
	/*	printf("%d %s\n",i,c1);*/
	if(i >= 0 && i <= 2){
	  ii[i] = atoi(c1);
	} else if(i <= 5){
	  for(j = 0; j < 3; j++){
	    if(ii[0] < 0 || ii[0] > fs->n[0]){
	      ks_error("%d is not in range %s in line %d \n%s"
		      ,ii[j],n_name[j],line_num,c0);
	      return KS_FALSE;
	    }
	  }
	  if((*fi)[ii[0]][ii[1]][ii[2]] == NULL){
	    if(((*fi)[ii[0]][ii[1]][ii[2]] = (FV_FIELD*)ks_malloc(sizeof(FV_FIELD),"fi")) == NULL){
	      ks_error("memory error in fv_read_field_file");
	      return KS_FALSE;
	    }
	    (*fi)[ii[0]][ii[1]][ii[2]]->s = NULL;
	    for(j = 0; j < 3; j++)
	      for(k = 0; k < 2; k++)
		(*fi)[ii[0]][ii[1]][ii[2]]->c[j][k] = NULL;
	  }
	  (*fi)[ii[0]][ii[1]][ii[2]]->cd[i-3] = atof(c1);

	} else {
	  if((*fi)[ii[0]][ii[1]][ii[2]]->s == NULL){
	    if(((*fi)[ii[0]][ii[1]][ii[2]]->s=(double*)ks_malloc((fs->val_num-6)*sizeof(double),
								 "fi->s")) ==NULL){
	      ks_error("memory error in ks_read_field_file");
	      return KS_FALSE;
	    }
	  }
	  (*fi)[ii[0]][ii[1]][ii[2]]->s[i-6] = atof(c1)+offset[i-6];
	}
      }
      /*
      printf("%d %d %d %f %f %f  % f % f % f\n",ii[0],ii[1],ii[2],
	     (*fi)[ii[0]][ii[1]][ii[2]]->cd[0],
	     (*fi)[ii[0]][ii[1]][ii[2]]->cd[1],
	     (*fi)[ii[0]][ii[1]][ii[2]]->cd[2],
	     (*fi)[ii[0]][ii[1]][ii[2]]->s[0],
	     (*fi)[ii[0]][ii[1]][ii[2]]->s[1],
	     (*fi)[ii[0]][ii[1]][ii[2]]->s[2]);
      */
    }
  }

  fclose(fp);

  for(i = 0; i < fs->n[0]; i++){
    for(j = 0; j < fs->n[1]; j++){
      for(k = 0; k < fs->n[2]; k++){
	if((*fi)[i][j][k] == NULL){
	  ks_error("No data in %d %d %d",i,j,k);
	  return KS_FALSE;
	}
      }
    }
  }

  fs->val_num -= 6;

  fv_set_field_connection(fi,fs);

  return KS_TRUE;
}
void fv_set_field_connection(FV_FIELD *****fi, FV_FIELD_SIZE *fs)
{
  int i;
  int i0,i1,i2;

  for(i = 0; i < 3; i++)
    fs->ccd[i] = 0;
  /*
  fs->max = (*fi)[0][0][0]->s[0];
  fs->min = (*fi)[0][0][0]->s[0];
  */
  for(i2 = 0; i2 < fs->n[2]; i2++){
    for(i1 = 0; i1 < fs->n[1]; i1++){
      for(i0 = 0; i0 < fs->n[0]; i0++){
	if((*fi)[i0][i1][i2] != NULL){
	  /*
	    printf("%d %d %d % f % f % f\n",i0,i1,i2,
	    (*fi)[i0][i1][i2]->cd[0],
	    (*fi)[i0][i1][i2]->cd[1],
	    (*fi)[i0][i1][i2]->cd[2]);
	  */
	  for(i = 0; i < 3; i++)
	    fs->ccd[i] += (*fi)[i0][i1][i2]->cd[i];
	  /*
	  for(i = 0; i < fs->val_num; i++){
	    if(fs->min > (*fi)[i0][i1][i2]->s[i]) fs->min = (*fi)[i0][i1][i2]->s[i];
	    if(fs->max < (*fi)[i0][i1][i2]->s[i]) fs->max = (*fi)[i0][i1][i2]->s[i];
	  }
	  */
	  if(i0 == 0)
	    (*fi)[i0][i1][i2]->l[0][0] = NULL;
	  else
	    (*fi)[i0][i1][i2]->l[0][0] = (*fi)[i0-1][i1][i2];
	  if(i0 == fs->n[0]-1)
	    (*fi)[i0][i1][i2]->l[0][1] = NULL;
	  else
	    (*fi)[i0][i1][i2]->l[0][1] = (*fi)[i0+1][i1][i2];

	  if(i1 == 0)
	    (*fi)[i0][i1][i2]->l[1][0] = NULL;
	  else
	    (*fi)[i0][i1][i2]->l[1][0] = (*fi)[i0][i1-1][i2];
	  if(i1 == fs->n[1]-1)
	    (*fi)[i0][i1][i2]->l[1][1] = NULL;
	  else
	    (*fi)[i0][i1][i2]->l[1][1] = (*fi)[i0][i1+1][i2];

	  if(i2 == 0)
	    (*fi)[i0][i1][i2]->l[2][0] = NULL;
	  else
	    (*fi)[i0][i1][i2]->l[2][0] = (*fi)[i0][i1][i2-1];
	  if(i2 == fs->n[2]-1)
	    (*fi)[i0][i1][i2]->l[2][1] = NULL;
	  else
	    (*fi)[i0][i1][i2]->l[2][1] = (*fi)[i0][i1][i2+1];
	}
      }
    }
  }
  /*
  fs->min = floor(fs->min);
  fs->max = ceil(fs->max);
  */
  for(i = 0; i < 3; i++){
    fs->ccd[i] /= fs->n[0]*fs->n[1]*fs->n[2];
    /*    printf("ccd %d %f\n",i,fs->ccd[i]);*/
  }
}
void fv_set_center_and_size(FV_FIELD ****mn, FV_FIELD_SIZE ms, double *center, double *size)
{
  int i;
  for(i = 0; i < 3; i++)
    center[i] = ms.ccd[i];
  *size = ks_max(mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[0]-mn[0][0][0]->cd[0],
		 ks_max(mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[1]-mn[0][0][0]->cd[1],
			mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[2]-mn[0][0][0]->cd[2]));
  /*
  printf("%f %f %f\n",
	 mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[0]-mn[0][0][0]->cd[0],
	 mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[1]-mn[0][0][0]->cd[1],
	 mn[ms.n[0]-1][ms.n[1]-1][ms.n[2]-1]->cd[2]-mn[0][0][0]->cd[2]);
  */
}
void fv_free_field(FV_FIELD ****fi, FV_FIELD_SIZE fs)
{
  int i,j,k;
  int l,m;
  for(i = 0; i < fs.n[0]; i++){
    for(j = 0; j < fs.n[1]; j++){
      for(k = 0; k < fs.n[2]; k++){
	/*
	if(i == 11 && j == 11 & k == 21){
	  printf("%d %d %d %p\n",i,j,k,fi[i][j][k]->s);
	}
	*/
	ks_free(fi[i][j][k]->s);
	for(l = 0; l < 3; l++){
	  for(m = 1; m < 2; m++){
	    /*	    printf("  %d %d %d %d %d\n",i,j,k,l,m);*/
	    free_contour(fi[i][j][k]->c[l][m]);
	  }
	}
	ks_free(fi[i][j][k]);
      }
    }
  }
  for(i = 0; i < fs.n[0]; i++){
    for(j = 0; j < fs.n[1]; j++){
      ks_free(fi[i][j]);
    }
  }
  for(i = 0; i < fs.n[0]; i++){
    ks_free(fi[i]);
  }
  ks_free(fi);
}
static BOOL __KS_USED__ check_level(FV_FIELD *p0, FV_FIELD *p1, int tv, double val, double dd[3])
{
  int i;
  double sa[2];

  if(p0 != NULL && p1 != NULL/* && (p0->flg == 0 || p1->flg == 0)*/){
    if((p0->s[tv]-val)*(p1->s[tv]-val) < 0){
      sa[0] = fabs(p0->s[tv]-val);
      sa[1] = fabs(p1->s[tv]-val);
      for(i = 0; i < 3; i++){
	dd[i] = (sa[1]*p0->cd[i]+sa[0]*p1->cd[i])/(sa[0]+sa[1]);
      }
      return KS_TRUE;
    } else {
      return KS_FALSE;
    }
  } else {
    return KS_FALSE;
  }
}
/*
static void set_field_color(double val, FV_CONTOUR_SIZE cs, GLfloat *color)
{
  double h,s,v;
  double r,g,b;

  h = val*240.;
  if(h < 0) h = 0;
  if(h > 240) h = 240;
  h = 240. - h;
  s = 1.0;
  v = 1.0;

  ks_hsv2rgb(h,s,v,&r,&g,&b);
  color[0] = r;
  color[1] = g;
  color[2] = b;
}
static void set_field_color_vertex(FV_FIELD *m,int vn, double max, double min, int height_flg)
{
  double hm = 10.0;
  GLfloat color[3];

  set_field_color((m->s[vn]-min)/(max-min),color);

  glColor3fv(color);
  if(height_flg == 0)
    glVertex3d(m->cd[0],m->cd[1],m->cd[2]);
  else if(height_flg == 1)
    glVertex3d((m->s[vn]-min)/(max-min)*hm,m->cd[1],m->cd[2]);
  else
    glVertex3d(m->s[vn],m->cd[1],m->cd[2]);
}
*/
static void set_field_color_vertex(FV_FIELD *m, int vn, FV_CONTOUR_SIZE cs)
{
  
  glColor3fv(cs.color[fv_get_level(m->s[vn],cs)]);
  glVertex3d(m->cd[0],m->cd[1],m->cd[2]);
}
static void fv_draw_frame(KS_GL_BASE *gb, FV_FIELD ****fi, FV_FIELD_SIZE fs)
{
  int i,j,i0,in;
  double len = 0.9;
  char c0[256];
  GLfloat frame_color[3] = {0.6, 0.6, 0.6};
  GLfloat moji_color[3] = {1.0, 1.0, 1.0};
  int num_pos = 0;

  int div_num = 5;
  int dn[3];
  float dp[3],dl[3][3];

  glColor3fv(frame_color);
  /*
  printf("%d %d %d\n",fs.n[0],fs.n[1],fs.n[2]);
  printf("%f %f %f  %f %f %f\n"
	 ,fi[0][0][0]->cd[0],fi[0][0][0]->cd[1],fi[0][0][0]->cd[2],
	 fi[fs.n[0]-1][fs.n[1]-1][fs.n[2]-1]->cd[0],
	 fi[fs.n[0]-1][fs.n[1]-1][fs.n[2]-1]->cd[1],
	 fi[fs.n[0]-1][fs.n[1]-1][fs.n[2]-1]->cd[2]);
  */
  ks_draw_gl_box_line(fi[0][0][0]->cd[0],fi[0][0][0]->cd[1],fi[0][0][0]->cd[2],
		      fi[fs.n[0]-1][fs.n[1]-1][fs.n[2]-1]->cd[0],
		      fi[fs.n[0]-1][fs.n[1]-1][fs.n[2]-1]->cd[1],
		      fi[fs.n[0]-1][fs.n[1]-1][fs.n[2]-1]->cd[2]);

  /*
  for(i = 0; i < fs.n[1]; i += 40){
    glBegin(GL_LINE_LOOP);
    glVertex3dv(fi[0][i][0]->cd);
    glVertex3dv(fi[0][i][fs.n[2]-1]->cd);
    glVertex3dv(fi[fs.n[0]-1][i][fs.n[2]-1]->cd);
    glVertex3dv(fi[fs.n[0]-1][i][0]->cd);
    glEnd();
  }
  */

  ks_use_glut_font(gb,KS_GL_FONT_HELVETICA_10);

  for(i = 0; i < 3; i++){
    if(i == 0){
      i0 = 1;
    } else if(i == 1){
      i0 = 2;
    } else {
      i0 = 1;
    }
    for(j = 0; j < 3; j++){
      if(j == i0){
	dl[i][j] = len;
      } else {
	dl[i][j] = 0;
      }
    }
  }

  /*
  for(in = 0; in < 3; in++){
    dn[in] = fs.n[in]/div_num;
    printf("%d %d %d %d\n",in,fs.n[in],dn[in],div_num);
  }
  ks_exit(EXIT_FAILURE);
  */
  for(in = 0; in < 3; in++){
    dn[in] = fs.n[in]/div_num;
    if(dn[in] == 0) dn[in] = 1;
    /*    printf("%d %d %d\n",in,fs.n[in],dn[in]);*/
    for(i = 0; i < fs.n[in]; i += dn[in]){
      glColor3fv(frame_color);
      if(in == 0){
	for(j = 0; j < 3; j++)
	  dp[j] = fi[i][num_pos][fs.n[2]-1]->cd[j];
      } else if(in == 1){
	for(j = 0; j < 3; j++)
	  dp[j] = fi[fs.n[0]-1][i][num_pos]->cd[j];
      } else {
	for(j = 0; j < 3; j++)
	  dp[j] = fi[fs.n[0]-1][num_pos][i]->cd[j];
      }
      /*      printf("%d %d %f %f %f\n",in,i,dp[0],dp[1],dp[2]);*/
      glBegin(GL_LINES);
      glVertex3f(dp[0]-dl[in][0],dp[1]-dl[in][1],dp[2]-dl[in][2]);
      glVertex3f(dp[0]+dl[in][0],dp[1]+dl[in][1],dp[2]+dl[in][2]);
      /*
      printf("%d (%f %f %f)  (%f %f %f)\n",in,
	     dp[0]-dl[in][0],dp[1]-dl[in][1],dp[0]-dl[in][2],
	     dp[0]+dl[in][0],dp[1]+dl[in][1],dp[0]+dl[in][2]);
      */
      glEnd();
      {
	glColor3fv(moji_color);
	if(in == 0){
	  sprintf(c0,"%.1f",fi[i][0][0]->cd[0]);
	} else if(in == 1){
	  sprintf(c0,"%.1f",fi[0][i][0]->cd[1]);
	} else {
	  sprintf(c0,"%.1f",fi[0][0][i]->cd[2]);
	}
	glRasterPos3fv(dp);
	glCallLists(strlen(c0), GL_BYTE, c0);
      }
    }
  }

#if 0
  ks_exit(EXIT_FAILURE);

  for(i = 0; i < fs.n[0]; i += 1){
    glColor3fv(frame_color);
    glBegin(GL_LINES);
    glVertex3d(fi[i][num_pos][fs.n[2]-1]->cd[0],
	       fi[i][num_pos][fs.n[2]-1]->cd[1]-len,
	       fi[i][num_pos][fs.n[2]-1]->cd[2]);
    glVertex3d(fi[i][num_pos][fs.n[2]-1]->cd[0],
	       fi[i][num_pos][fs.n[2]-1]->cd[1]+len,
	       fi[i][num_pos][fs.n[2]-1]->cd[2]);
    glEnd();
    if(gb->vflg != 0){
      glColor3fv(moji_color);
      sprintf(c0,"%.1f",fi[i][0][0]->cd[0]);
      glRasterPos3d(fi[i][num_pos][fs.n[2]-1]->cd[0],
		    fi[i][num_pos][fs.n[2]-1]->cd[1],
		    fi[i][num_pos][fs.n[2]-1]->cd[2]);
      glCallLists(strlen(c0), GL_BYTE, c0);
    }
  }

  for(i = 0; i < fs.n[1]; i += 10){
    glColor3fv(frame_color);
    glBegin(GL_LINES);
    glVertex3d(fi[fs.n[0]-1][i][0]->cd[0],
	       fi[fs.n[0]-1][i][0]->cd[1],
	       fi[fs.n[0]-1][i][0]->cd[2]-len);
    glVertex3d(fi[fs.n[0]-1][i][0]->cd[0],
	       fi[fs.n[0]-1][i][0]->cd[1],
	       fi[fs.n[0]-1][i][0]->cd[2]+len);
    glEnd();
    if(gb->vflg != 0){
      glColor3fv(moji_color);
      sprintf(c0,"%.1f",fi[0][i][0]->cd[1]-80);
      glRasterPos3d(fi[fs.n[0]-1][i][0]->cd[0],
		    fi[fs.n[0]-1][i][0]->cd[1],
		    fi[fs.n[0]-1][i][0]->cd[2]);
      glCallLists(strlen(c0), GL_BYTE, c0);
    }
  }

  for(i = 0; i < fs.n[2]; i += 2){
    glColor3fv(frame_color);
    glBegin(GL_LINES);
    glVertex3d(fi[fs.n[0]-1][num_pos][i]->cd[0],
	       fi[fs.n[0]-1][num_pos][i]->cd[1]-len,
	       fi[fs.n[0]-1][num_pos][i]->cd[2]);
    glVertex3d(fi[fs.n[0]-1][num_pos][i]->cd[0],
	       fi[fs.n[0]-1][num_pos][i]->cd[1]+len,
	       fi[fs.n[0]-1][num_pos][i]->cd[2]);
    glEnd();

    if(gb->vflg != 0){
      glColor3fv(moji_color);
      sprintf(c0,"%.1f",fi[0][0][i]->cd[2]);
      glRasterPos3d(fi[fs.n[0]-1][num_pos][i]->cd[0],
		    fi[fs.n[0]-1][num_pos][i]->cd[1],
		    fi[fs.n[0]-1][num_pos][i]->cd[2]);
      glCallLists(strlen(c0), GL_BYTE, c0);
    }
  }
#endif
}
void fv_gldraw_field(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_TARGET ft, FV_CONTOUR_SIZE cs,
		     int dm)
{
  int i;
  int i0;
  FV_FIELD *sp;
  FV_FIELD *p0,*p1;
  int pl[2];
  char c0[64];

  if(ft.pl == 0){
    sp = fi[ft.num[0]][0][0];
    pl[0] = 1; pl[1] = 2;
  } else if(ft.pl == 1){
    sp = fi[0][ft.num[1]][0];
    pl[0] = 2; pl[1] = 0;
  } else if(ft.pl == 2){
    sp = fi[0][0][ft.num[2]];
    pl[0] = 0; pl[1] = 1;
  } else return;

  if(dm == FV_DRAW_LINE){
    /*  printf("min %f  max %f\n",fs.min,fs.max);*/
    for(i = 0; i < 2; i++){
      for(p0 = sp; p0 != NULL; p0 = p0->l[pl[0]][1]){
	glBegin(GL_LINE_STRIP);
	for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	  /*
	    printf("% f % f % f  % f % f % f\n",
	    p1->cd[0], p1->cd[1], p1->cd[2],
	    p1->s[0], p1->s[1], p1->s[2]);
	  */
	  /*
	  set_field_color_vertex(p1,ft.val, fs.max,  fs.min, 0);
	  */
	  set_field_color_vertex(p1,ft.val,cs);
	}
	glEnd();
      }
      i0 = pl[0];
      pl[0] = pl[1];
      pl[1] = i0;
    }
  } else if(dm == FV_DRAW_TRANS){ /* for debug */
    for(i = 0; i < 2; i++){
      for(p0 = sp; p0 != NULL; p0 = p0->l[pl[0]][1]){
	glBegin(GL_LINE_STRIP);
	for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	  set_field_color_vertex(p1,ft.val,cs);
	}
	glEnd();
	for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	  sprintf(c0,"%f",p1->s[0]);
	  glRasterPos3f(p1->cd[0],p1->cd[1],p1->cd[2]);
	  glCallLists(strlen(c0), GL_BYTE, c0);
	}
      }
      i0 = pl[0];
      pl[0] = pl[1];
      pl[1] = i0;
    }
  } else if(dm == FV_DRAW_FILL){

    for(p0 = sp; p0 != NULL; p0 = p0->l[pl[0]][1]){
      glPointSize(5.0);
      glBegin(GL_QUADS);
      for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	if(p1->l[pl[0]][1] != NULL && p1->l[pl[1]][1] != NULL){
	  /*
	  set_field_color_vertex(p1,ft.val, fs.max,  fs.min, 0);
	  set_field_color_vertex(p1->l[pl[0]][1],ft.val, fs.max,  fs.min, 0);
	  set_field_color_vertex(p1->l[pl[0]][1]->l[pl[1]][1],ft.val, fs.max,  fs.min, 0);
	  set_field_color_vertex(p1->l[pl[1]][1],ft.val, fs.max,  fs.min, 0);
	  */
	  set_field_color_vertex(p1,ft.val,cs);
	  set_field_color_vertex(p1->l[pl[0]][1],ft.val,cs);
	  set_field_color_vertex(p1->l[pl[0]][1]->l[pl[1]][1],ft.val,cs);
	  set_field_color_vertex(p1->l[pl[1]][1],ft.val,cs);
	}
      }
      glEnd();
    }
  }

}
/*
static void set_contour_color_vertex(FV_CONTOUR *m, double s, double max, double min, int height_flg)
{
  double hm = 10.0;
  GLfloat color[3];

  set_field_color((s-min)/(max-min),color);
  glColor3fv(color);
  if(height_flg == 0)
    glVertex3d(m->cd[0],m->cd[1],m->cd[2]);
  else if(height_flg == 1)
    glVertex3d((s-min)/(max-min)*hm,m->cd[1],m->cd[2]);
  else
    glVertex3d(s,m->cd[1],m->cd[2]);

}
*/
static void __KS_USED__ set_contour_color_vertex(FV_CONTOUR *m, FV_CONTOUR_SIZE cs)
{
  glColor3fv(cs.color[m->ln]);
  glVertex3d(m->cd[0],m->cd[1],m->cd[2]);
}
void fv_gldraw_contour_surface(FV_CONTOUR_SIZE cs, FV_TARGET tg, FV_CONTOUR_FAN ***fcf,
			       int dm)
{
  int i,j,c;
  FV_CONTOUR_FAN *fa;
  GLfloat color[4];

  glLoadName(3);

  glPushMatrix();

  glDisable(GL_CULL_FACE);
  
  /*  printf("cn.num %d\n",cn.num);*/
  if(dm == FV_DRAW_LINE){
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.0);
    glDisable(GL_LIGHTING);
    glEnable(GL_CULL_FACE);
  } else if(dm == FV_DRAW_TRANS){
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    color[3] = 0.7;
  } else {
    color[3] = 1.0;
  }

  for(i = 0; i < cs.num; i++){
    glPushName(i);
    /*    printf("%d %d\n",i,tg.level[i]);*/
    if(tg.level[i] == 1){
      c = 0;
      for(fa = fcf[i][tg.val]; fa != NULL; fa = fa->next){

	glPushName(c);

	/*
	if(i == hit_surface[0] && c == hit_surface[1]){
	  for(j = 0; j < fa->num; j++){
	    printf("  %d %d %d (%f %f %f) (%f %f %f)\n",i,j,fa->pcs[j]
		   ,fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2]
		   ,fa->pcs[j]->nv[0],fa->pcs[j]->nv[1],fa->pcs[j]->nv[2]
		   );
	  }
	  color[0] = 0.0; color[1] = 1.0; color[2] = 1.0; color[3] = 1.0;
	  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	} else {
	  set_field_color((cn.level[i]-ms.min)/(ms.max-ms.min),cs,color);
	  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	}
	*/

	/*	set_field_color((cn.level[i]-ms.min)/(ms.max-ms.min),color);*/
#ifdef HIT_SURFACE
	if(i == hit_surface[0] && c == hit_surface[1]){
	  if(dm != FV_DRAW_LINE){
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	    glLineWidth(1.0);
	    glDisable(GL_LIGHTING);
	    glEnable(GL_CULL_FACE);
	  }
	  glColor3f(0.0,1.0,0.0);
	  for(j = 0; j < fa->num; j++){
	    if(fa->pcs[j]->l[0] != NULL && fa->pcs[j]->l[1] != NULL){
	      glBegin(GL_LINES);
	      glVertex3dv(fa->pcs[j]->l[0]->cd);
	      glVertex3dv(fa->pcs[j]->l[1]->cd);
	      glEnd();
	    }
	  }
	  glColor3f(1.0,1.0,1.0);
	}
#endif

	if(dm == FV_DRAW_LINE){
	  glColor3fv(cs.color[i]);
	} else {
	  for(j = 0; j < 3; j++)
	    color[j] = cs.color[i][j];
	  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	}


	glBegin(GL_TRIANGLE_FAN);
	for(j = 0; j < fa->num; j++){
	  glNormal3dv(fa->pcs[j]->nv);
	  glVertex3dv(fa->pcs[j]->cd);
	}
	if(fa->num > 3){
	  glNormal3dv(fa->pcs[1]->nv);
	  glVertex3dv(fa->pcs[1]->cd);
	}
	glEnd();
	/*
	glBegin(GL_TRIANGLE_FAN);
	glNormal3d(-fa->pcs[0]->nv[0],-fa->pcs[0]->nv[1],-fa->pcs[0]->nv[2]);
	glVertex3dv(fa->pcs[0]->cd);
	for(j = fa->num-1; j >= 1; j--){
	  glNormal3d(-fa->pcs[j]->nv[0],-fa->pcs[j]->nv[1],-fa->pcs[j]->nv[2]);
	  glVertex3dv(fa->pcs[j]->cd);
	}
	if(fa->num > 3){
	  glNormal3d(-fa->pcs[fa->num-1]->nv[0],
		     -fa->pcs[fa->num-1]->nv[1],
		     -fa->pcs[fa->num-1]->nv[2]);
	  glVertex3dv(fa->pcs[fa->num-1]->cd);
	}
	glEnd();
	*/
	/*
	if(i == hit_surface[0] && c == hit_surface[1]){
	  glDisable(GL_LIGHTING);
	  for(j = 0; j < fa->num; j++){
	    ks_draw_gl_line(fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2],
			    fa->pcs[j]->cd[0]+fa->pcs[j]->nv[0],
			    fa->pcs[j]->cd[1]+fa->pcs[j]->nv[1],
			    fa->pcs[j]->cd[2]+fa->pcs[j]->nv[2],1,1,1,2);
	  }
	  glEnable(GL_LIGHTING);
	}
	*/
#ifdef HIT_SURFACE
	if(i == hit_surface[0] && c == hit_surface[1] && dm != FV_DRAW_LINE){
	  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	  glEnable(GL_LIGHTING);
	}
#endif

	glPopName();
	c++;
      }
    }
    glPopName();
  }
#if 1
  if(dm == FV_DRAW_LINE && draw_mode[DRAW_CONTOUR] == FV_DRAW_OFF){
    /* remove hidden line */
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0,1.0);

    glEnable(GL_STENCIL_TEST);
    glStencilFunc (GL_NOTEQUAL, 0x1, 0x1);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);

    for(i = 0; i < cs.num; i++){
      if(tg.level[i] == 1){
	c = 0;
	for(fa = fcf[i][tg.val]; fa != NULL; fa = fa->next){
	  glColor3f(0,0,0);
	  glBegin(GL_TRIANGLE_FAN);
	  for(j = 0; j < fa->num; j++){
	    glNormal3dv(fa->pcs[j]->nv);
	    glVertex3dv(fa->pcs[j]->cd);
	  }
	  if(fa->num > 3){
	    glNormal3dv(fa->pcs[1]->nv);
	    glVertex3dv(fa->pcs[1]->cd);
	  }
	  glEnd();
	  c++;
	}
      }
    }
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_POLYGON_OFFSET_FILL);
  }
#endif

  if(dm == FV_DRAW_LINE){
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
  } else if(dm == FV_DRAW_TRANS){
    glDisable(GL_BLEND);
  }

  glEnable(GL_CULL_FACE);

  glPopMatrix();

  glLoadName(0);
}
void fv_output_contour_surface(FV_CONTOUR_SIZE cs, FV_TARGET tg, FV_CONTOUR_FAN ***fcf,
			       int mode, FILE *fp)
{
  int i,j;
  int vertex_cnt;
  FV_CONTOUR_FAN *fa;
  fprintf(fp,"n_level %d\n",cs.num);
  for(i = 0; i < cs.num; i++){
    vertex_cnt = 0;
    for(fa = fcf[i][tg.val]; fa != NULL; fa = fa->next){
      vertex_cnt += fa->num;
    }
    fprintf(fp,"level %d n_surface %d n_vertex %d\n",i,count_contour_fan(fcf[i][tg.val]),
	    vertex_cnt);
    for(fa = fcf[i][tg.val]; fa != NULL; fa = fa->next){
      fprintf(fp,"%d\n",fa->num);
      for(j = 0; j < fa->num; j++){
	fprintf(fp,"%f %f %f\n",fa->pcs[j]->cd[0],fa->pcs[j]->cd[1],fa->pcs[j]->cd[2]);
      }
      for(j = 0; j < fa->num; j++){
	fprintf(fp,"%f %f %f\n",fa->pcs[j]->nv[0],fa->pcs[j]->nv[1],fa->pcs[j]->nv[2]);
      }
    }
  }
}
static BOOL fv_gldraw_contour(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_CONTOUR_SIZE cn, FV_TARGET tg,
			     int dm)
{
  int i,j,k;
  FV_FIELD *sp;
  FV_FIELD *p0,*p1;
  FV_FIELD *ps[4];
  int pl[2];
  FV_CONTOUR *pc;
  char c0[64];

  if(tg.pl == 0){
    sp = fi[tg.num[0]][0][0];
    pl[0] = 1; pl[1] = 2;
  } else if(tg.pl == 1){
    sp = fi[0][tg.num[1]][0];
    pl[0] = 2; pl[1] = 0;
  } else if(tg.pl == 2){
    sp = fi[0][0][tg.num[2]];
    pl[0] = 0; pl[1] = 1;
  } else {
    ks_error("unkown tg.pl type %d",tg.pl);
    return KS_FALSE;
  }

  /*  printf("%d %d %d\n",tg.num[0],tg.num[1],tg.num[2]);*/
  /*  printf("%d %p\n",tg,sp);*/
  if(dm == FV_DRAW_LINE){
    for(p0 = sp; p0 != NULL; p0 = p0->l[pl[0]][1]){
      /*      printf("p0 %p\n",p0);*/
      for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	/*	printf("p1 %p\n",p1);*/
	for(i = 0; i < 2; i++){
	  /*	  printf("%d %p\n",i,p1->l[pl[i]][1]);*/
	  if(p1->l[pl[i]][1] != NULL){
	    /*	    ks_exit(EXIT_FAILURE);*/
	    for(pc = p1->c[pl[i]][1]; pc != NULL; pc = pc->next){
	      if(pc->vn == tg.val){
		/*
		printf("     %d %d %f %f %f\n",pc->ln,pc->vn,pc->cd[0],pc->cd[1],pc->cd[2]);
		*/
		if(tg.level[pc->ln] == 1)
		  glLineWidth(2.0);
		else
		  glLineWidth(1.0);
		for(j = 0; j < 2; j++){
		  if(pc->c[tg.pl][j] != NULL){
		    glColor3fv(cn.color[pc->ln]);
		    glBegin(GL_LINES);
		    /*
		    set_contour_color_vertex(pc,cn.level[pc->ln],fs.max,fs.min,0);
		    set_contour_color_vertex(pc->c[tg.pl][j],cn.level[pc->ln],fs.max,fs.min,0);
		    */
		    glVertex3d(pc->cd[0],pc->cd[1],pc->cd[2]);
		    glVertex3d(pc->c[tg.pl][j]->cd[0],
			       pc->c[tg.pl][j]->cd[1],
			       pc->c[tg.pl][j]->cd[2]);
		    glEnd();
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  } else if(dm == FV_DRAW_TRANS){
    for(p0 = sp; p0 != NULL; p0 = p0->l[pl[0]][1]){
      /*      printf("p0 %p\n",p0);*/
      for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	/*	printf("p1 %p\n",p1);*/
	for(i = 0; i < 2; i++){
	  /*	  printf("%d %p\n",i,p1->l[pl[i]][1]);*/
	  if(p1->l[pl[i]][1] != NULL){
	    /*	    ks_exit(EXIT_FAILURE);*/
	    for(pc = p1->c[pl[i]][1]; pc != NULL; pc = pc->next){
	      glColor3fv(cn.color[pc->ln]);
	      glPointSize(4);
	      glBegin(GL_POINTS);
	      glVertex3d(pc->cd[0],pc->cd[1],pc->cd[2]);
	      glEnd();
	      sprintf(c0,"%.2f %.2f %.2f %d %f",pc->cd[0],pc->cd[1],pc->cd[2],pc->ln,cn.level[pc->ln]);
	      glRasterPos3f(pc->cd[0],pc->cd[1],pc->cd[2]);
	      glCallLists(strlen(c0), GL_BYTE, c0);
	    }
	  }
	}
      }
    }
  } else if(dm == FV_DRAW_FILL){
    {
      int i0,i1,i2,i3;
      int ln[4];
      FV_CONTOUR *pc2,*pc_prev;
      void *vp[11][10][2],*fvp,*cvp; /*,*vps[10];*/
      int vf[11][10][2];
      int vn[11];
      //      int vpsn;

      for(p0 = sp; p0 != NULL; p0 = p0->l[pl[0]][1]){
	for(p1 = p0; p1 != NULL; p1 = p1->l[pl[1]][1]){
	  if(p1->l[pl[0]][1] != NULL && p1->l[pl[1]][1]){
	    /*	    printf("%f %f %f\n",p1->cd[0],p1->cd[1],p1->cd[2]);*/
	    ps[0] = p1;
	    ps[1] = p1->l[pl[0]][1];
	    ps[2] = p1->l[pl[0]][1]->l[pl[1]][1];
	    ps[3] = p1->l[pl[1]][1];

	    for(i = 0; i < 4; i++)
	      ln[i] = fv_get_level(ps[i]->s[tg.val],cn);
	    for(i = 0; i < cn.num+1; i++){
	      vn[i] = 0;
	      for(j = 0; j < 10; j++){
		for(k = 0; k < 2; k++){
		  vp[i][j][k] = NULL;
		  vf[i][j][k] = -1;
		}
	      }
	    }
	    for(i = 0; i < 4; i++){
	      /*
	      printf("%d %d %d %f %f %f\n"
		     ,i,ln[i],ps[i],ps[i]->cd[0],ps[i]->cd[1],ps[i]->cd[2]);
	      */
	      vp[ln[i]][vn[ln[i]]][0] = ps[i];
	      vf[ln[i]][vn[ln[i]]][0] = 0;
	      vn[ln[i]]++;
	      if(i == 0){
		pc = ps[0]->c[pl[0]][1];
	      } else if(i == 1){
		pc = ps[1]->c[pl[1]][1];
	      } else if(i == 2){
		pc = ps[2]->c[pl[0]][0];
	      } else if(i == 3){
		pc = ps[3]->c[pl[1]][0];
	      }
	      i3 = 0;
	      for(pc_prev = NULL ; pc != NULL; pc = pc->next){
		if(pc->vn == tg.val){
		  i3 = 1;
		  /*
		  printf("* %d %d %f %f %f\n",pc->ln,pc,pc->cd[0],pc->cd[1],pc->cd[2]);
		  */
		  if(pc->ln == ln[i] || pc->ln == ln[i]-1){ /* feild to contour */
		    for(j = 0; j < vn[ln[i]]; j++){
		      /*
		      printf("  feild to contour %d %d %d %d %d %d\n",ln[i],j
			,vp[ln[i]][j][0],vf[ln[i]][j][0]
			,vp[ln[i]][j][1],vf[ln[i]][j][1]);
		      */
		      if(vf[ln[i]][j][0] == 0 && vf[ln[i]][j][1] == -1){
			vp[ln[i]][j][1] = pc;
			vf[ln[i]][j][1] = 1;
		      }
		    }
		  } 
		  if(pc->ln == ln[i==3?0:i+1] || pc->ln == ln[i==3?0:i+1]-1){/* contour to feild */
		    /*
		      printf("contour to field %d %d %d %d\n"
			     ,pc->ln,i==3?0:i+1,ln[i==3 ? 0:i+1],vn[ln[i==3 ? 0:i+1]]);
		    */
		    vp[ln[i==3?0:i+1]][vn[ln[i==3?0:i+1]]][1] = pc;
		    vf[ln[i==3?0:i+1]][vn[ln[i==3?0:i+1]]][1] = 1;
		    vp[ln[i==3?0:i+1]][vn[ln[i==3?0:i+1]]][0] = ps[i==3 ? 0:i+1];
		    vf[ln[i==3?0:i+1]][vn[ln[i==3?0:i+1]]][0] = 0;
		    vn[ln[i==3?0:i+1]]++;
		  }
		  if(pc_prev != NULL){
		    /*
		      printf("prev\n");
		      vp[pc->ln+1][vn[pc->ln+1]][0] = pc_prev;
		      vf[pc->ln+1][vn[pc->ln+1]][0] = 1;
		      vp[pc->ln+1][vn[pc->ln+1]][1] = pc;
		      vf[pc->ln+1][vn[pc->ln+1]][1] = 1;
		      vn[pc->ln+1]++;
		    */
		  } 
		  /*
		  if(pc->next != NULL && pc->vn == pc->next->vn){
		    printf("next\n");
		    vp[pc->ln][vn[pc->ln]][0] = pc;
		    vf[pc->ln][vn[pc->ln]][0] = 1;
		    vp[pc->ln][vn[pc->ln]][1] = pc->next;;
		    vf[pc->ln][vn[pc->ln]][1] = 1;
		    vn[pc->ln]++;
		  }
		  */

		  for(pc2 = pc->next; pc2 != NULL; pc2 = pc2->next){
		    if(pc->ln-1 == pc2->ln && pc->vn == pc2->vn){
		      vp[pc->ln][vn[pc->ln]][0] = pc;
		      vf[pc->ln][vn[pc->ln]][0] = 1;
		      vp[pc->ln][vn[pc->ln]][1] = pc2;
		      vf[pc->ln][vn[pc->ln]][1] = 1;
		      vn[pc->ln]++;
		    }
		  }

		  for(i2 = 0; i2 < 2; i2++){
		    i0 = 0;
		    for(j = 0; j < 3; j++){ /* contour to contour another side */
		      for(k = 0; k < 2; k++){
			/*
			printf("contour to contour    %d %d %d\n",j,k,pc->c[j][k]);
			*/
			for(i1 = 0; i1 < vn[pc->ln+i2]; i1++){
			  if(vp[pc->ln+i2][i1][0] == pc->c[j][k] && vf[pc->ln+i2][i1][1] == -1){
			    /*		    printf("    hit %d\n",i1);*/
			    vp[pc->ln+i2][i1][1] = pc;
			    vf[pc->ln+i2][i1][1] = 1;
			    i0 = 1;
			  }
			}
		      }
		    }
		    if(i0 == 0){
		      /*
		      printf("    create %d %d\n",pc->ln+i2,vn[pc->ln+i2]);
		      */
		      vp[pc->ln+i2][vn[pc->ln+i2]][0] = pc;
		      vf[pc->ln+i2][vn[pc->ln+i2]][0] = 1;
		      vn[pc->ln+i2]++;
		    }
		  }
		}
		pc_prev = pc;
	      }
	      if(i3 == 0){
		/*		printf("no contour %d %d\n",ln[i],vn[ln[i]]-1);*/
		vp[ln[i]][vn[ln[i]]-1][1] = ps[i==3?0:i+1];
		vf[ln[i]][vn[ln[i]]-1][1] = 0;
	      }
	    }
	    /*
	    for(i = 0; i < cn.num+1; i++){
	      printf("-> %d %d\n",i,vn[i]);
	      for(j = 0; j < vn[i]; j++){
		printf("  %d %d %d %d %d\n",j
		       ,vp[i][j][0],vf[i][j][0]
		       ,vp[i][j][1],vf[i][j][1]);
	      }
	    }
	    */
	    conpn = 0;
	    for(i = 0; i < cn.num+1; i++){
	      for(j = 0; j < vn[i]; j++){
		/*
		printf("%d %d %d %d %d %d\n",i,j
		       ,vp[i][j][0],vf[i][j][0]
		       ,vp[i][j][1],vf[i][j][1]);
		*/
		if(vf[i][j][0] != -1 && vf[i][j][1] != -1){
		  fvp = vp[i][j][0];
		  cvp = vp[i][j][1];
		  /* vpsn = 0; */
		  /* vps[vpsn++] = vp[i][j][0]; */
		  conpm[conpn] = 0;
		  if(vf[i][j][0] == 0){
		    for(i0 = 0; i0 < 3; i0++)
		      conp[conpn][conpm[conpn]][i0] = ((FV_FIELD*)(vp[i][j][0]))->cd[i0];
		  } else {
		    for(i0 = 0; i0 < 3; i0++)
		      conp[conpn][conpm[conpn]][i0] = ((FV_CONTOUR*)(vp[i][j][0]))->cd[i0];
		  }
		  for(i0 = 0; i0 < 3; i0++)
		    conpc[conpn][i0] = cn.color[i][i0];
		  conpm[conpn]++;
		  /*	  vps[vpsn++] = vp[i][j][1];*/
		  while(fvp != cvp){
		    /*		    printf("-- %d %d %d\n",i,j,cvp);*/
		    i3 = 0;
		    for(k = 0; k < vn[i]; k++){
		      if(j != k){
			/*			printf("j != k %d %d\n",j,k);*/
			if(cvp == vp[i][k][0]){
			  i3 = 1;
			  /*
			  printf("  %d %d %d %d %d %d a\n",j,k
				 ,vp[i][k][0],vf[i][k][0]
				 ,vp[i][k][1],vf[i][k][1]);
			  */
			  cvp = vp[i][k][1];
			  /* vps[vpsn++] = vp[i][k][0]; */
			  /* vps[vpsn++] = vp[i][k][1]; */
			  if(vf[i][k][0] == 0){
			    for(i0 = 0; i0 < 3; i0++)
			      conp[conpn][conpm[conpn]][i0] = ((FV_FIELD*)(vp[i][k][0]))->cd[i0];
			  } else {
			    for(i0 = 0; i0 < 3; i0++)
			      conp[conpn][conpm[conpn]][i0] = ((FV_CONTOUR*)(vp[i][k][0]))->cd[i0];
			  }
			  for(i0 = 0; i0 < 3; i0++)
			    conpc[conpn][i0] = cn.color[i][i0];
			  conpm[conpn]++;
			  vf[i][k][0] = -1;
			  vf[i][k][1] = -1;
			} else if(cvp == vp[i][k][1]){
			  i3 = 1;
			  /*
			  printf("  %d %d %d %d %d %d b\n",j,k
				 ,vp[i][k][0],vf[i][k][0]
				 ,vp[i][k][1],vf[i][k][1]);
			  */
			  cvp = vp[i][k][0];
			  /* vps[vpsn++] = vp[i][k][1]; */
			  /* vps[vpsn++] = vp[i][k][0]; */
			  if(vf[i][k][1] == 0){
			    for(i0 = 0; i0 < 3; i0++)
			      conp[conpn][conpm[conpn]][i0] = ((FV_FIELD*)(vp[i][k][1]))->cd[i0];
			  } else {
			    for(i0 = 0; i0 < 3; i0++)
			      conp[conpn][conpm[conpn]][i0] = ((FV_CONTOUR*)(vp[i][k][1]))->cd[i0];
			  }
			  for(i0 = 0; i0 < 3; i0++)
			    conpc[conpn][i0] = cn.color[i][i0];
			  conpm[conpn]++;
			  vf[i][k][0] = -1;
			  vf[i][k][1] = -1;
			} else {
			  /*
			  printf("%d not found. %d %d\n",cvp,i,j);
			  ks_exit(EXIT_FAILURE);
			  */
			}
		      }
		    }
		    if(i3 == 0){
		      ks_error("not found\n");
		      ks_exit(EXIT_FAILURE);
		    }
		  }
		  /*
		  for(k = 0; k < vpsn; k++){
		    printf("%d %d\n",k,vps[k]);
		  }
		  */
		  conpn++;
		}
	      }
	    }
	    /*
	    for(i = 0; i < conpn; i++){
	      printf("%d %f %f %f\n",i,conpc[i][0],conpc[i][1],conpc[i][2]);
	      for(j = 0; j < conpm[i]; j++)
		printf("%d %d %f %f %f\n",i,j,conp[i][j][0],conp[i][j][1],conp[i][j][2]);
		}
	    */
	    if(conpn > 0){
	      glDisable(GL_CULL_FACE);
	      glDisable(GL_LIGHTING);
	      for(i = 0; i < conpn; i++){
		glColor3fv(conpc[i]);
		glBegin(GL_POLYGON);
		for(j = 0; j < conpm[i]; j++){
		  //	printf("%d %d %f %f %f\n",i,j,conp[i][j][0],conp[i][j][1],conp[i][j][2]);
		  glVertex3fv(conp[i][j]);
		}
		glEnd();
	      }
	      glEnable(GL_CULL_FACE);
	      glEnable(GL_LIGHTING);
	    }
	  }
	}
      }
    }
  }

#ifdef CURSOR
  {
    char c0[256];
    FV_FIELD *pf;
    int ln[4];

    glDisable(GL_LIGHTING);
    glColor3f(1.0, 1.0, 1.0);
    ks_use_glut_font(gb,"8x13");

    pf = mn[cc.cd[0]][cc.cd[1]][cc.cd[2]];

    ps[0] = pf;
    ps[1] = pf->l[pl[0]][1];
    ps[2] = pf->l[pl[0]][1]->l[pl[1]][1];
    ps[3] = pf->l[pl[1]][1];

    for(i = 0; i < 4; i++)
      ln[i] = fv_get_level(ps[i]->s[tg.val],cn);
    for(i = 0; i < 4; i++){
      /*
      printf("%d %d %d %f %f %f\n"
	     ,i,ln[i],ps[i],ps[i]->cd[0],ps[i]->cd[1],ps[i]->cd[2]);
      */
      sprintf(c0,"%d",ps[i]);
      glRasterPos3f(ps[i]->cd[0],ps[i]->cd[1],ps[i]->cd[2]);
      glCallLists(strlen(c0), GL_BYTE, c0);
      if(i == 0){
	pc = ps[0]->c[pl[0]][1];
      } else if(i == 1){
	pc = ps[1]->c[pl[1]][1];
      } else if(i == 2){
	pc = ps[2]->c[pl[0]][0];
      } else if(i == 3){
	pc = ps[3]->c[pl[1]][0];
      }
      for(; pc != NULL; pc = pc->next){
	if(pc->vn == tg.val){
	  /*
	  printf("* %d %d %f %f %f\n",pc->ln,pc,pc->cd[0],pc->cd[1],pc->cd[2]);
	  */
	  sprintf(c0,"%d",pc);
	  glRasterPos3f(pc->cd[0],pc->cd[1],pc->cd[2]);
	  glCallLists(strlen(c0), GL_BYTE, c0);
	}
      }
    }
    glEnable(GL_LIGHTING);
  }
#endif
  return KS_TRUE;
}
static FV_CONTOUR *new_contour(int ln, int vn, double dd[3])
{
  int i,j;
  FV_CONTOUR *newp;

  newp = (FV_CONTOUR*)ks_malloc(sizeof(FV_CONTOUR),"FV_CONTOUR");
  newp->flags = 0;
  newp->ln = ln;
  newp->vn = vn;
  for(i = 0; i < 3; i++)
    newp->cd[i] = dd[i];
  for(i = 0; i < 3; i++)
    for(j = 0; j < 2; j++){
      newp->c[i][j] = NULL;
    }
  for(j = 0; j < 2; j++)
    newp->l[j] = NULL;
  newp->next = NULL;
  return newp;
}
static FV_CONTOUR *add_contour(FV_CONTOUR *listp, FV_CONTOUR *newp)
{
  newp->next = listp;
  return newp;
}
static void free_contour(FV_CONTOUR *listp)
{
  FV_CONTOUR *next;

  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static FV_CONTOUR_FAN *new_contour_fan(int num, FV_CONTOUR *pc, FV_CONTOUR **pcs, int loop_flg)
{
  int i;
  FV_CONTOUR_FAN *newp;

  newp = (FV_CONTOUR_FAN*)ks_malloc(sizeof(FV_CONTOUR_FAN),"newp");
  newp->num = num+1;
  newp->pcs = (FV_CONTOUR**)ks_malloc((num+1)*sizeof(FV_CONTOUR*),"newp->pcs");
  newp->pcs[0] = pc;
  if(loop_flg == 0){
    for(i = 0; i < num; i++)
      newp->pcs[i+1] = pcs[i];
  } else {
    for(i = 0; i < num; i++)
      newp->pcs[i+1] = pcs[num-1-i];
  }
  newp->next = NULL;
  return newp;
}
static FV_CONTOUR_FAN *add_contour_fan(FV_CONTOUR_FAN *listp, FV_CONTOUR_FAN *newp)
{
  newp->next = listp;
  return newp;
}
static void free_contour_fan(FV_CONTOUR_FAN *listp)
{
  FV_CONTOUR_FAN *next;

  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static int count_contour_fan(FV_CONTOUR_FAN *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
BOOL fv_calc_field_contour(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_CONTOUR_SIZE cs)
{
  int i0,i1,i2;
  int ix,iy,iz;
  int i,j,k,c;

  FV_FIELD *pf;
  FV_CONTOUR *pc,*pcn;
  double dd[3];
  double sa[2];

  static int free_flg = 0;
  /*
  printf("fv_calc_field_contour %d\n",free_flg);
  */
  if(free_flg == 1){
    for(i0 = 0; i0 < fs.n[0]; i0++){
      for(i1 = 0; i1 < fs.n[1]; i1++){
	for(i2 = 0; i2 < fs.n[2]; i2++){
	  if(fi[i0][i1][i2] != NULL){
	    pf = fi[i0][i1][i2];
	    for(i = 0; i < 3; i++){
	      for(j = 1; j < 2; j++){
		if(pf->c[i][j] != NULL){
		  /*
		  printf("%d %d %d  %d %d  %d %d\n",i0,i1,i2,i,j,pf->c[i][j],pf->c[i][j]->next);
		  */
		  free_contour(pf->c[i][j]);
		  pf->c[i][j] = NULL;
		}
	      }
	    }
	  }
	}
      }
    }
  }
  /*
  d0 = (fs.max-fs.min)/(cs->num+1);
  for(i = 0; i < cs->num; i++){
    cs->level[i] = d0*(i+1) + fs.min;
  }
  */
  /*  printf("%d %d %d\n",fs.n[0],fs.n[1],fs.n[2]);*/
  /*
  for(ix = 0; ix < fs.n[0]; ix++){
    for(iy = 0; iy < fs.n[1]; iy++){
      for(iz = 0; iz < fs.n[2]; iz++){
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  printf("%d %d %d %p\n",ix,iy,iz,pf->c[0][0]);
	}
      }
    }
  }
  ks_exit(EXIT_FAILURE);
  */

  for(i0 = 0; i0 < fs.n[0]; i0++){ /* set contour vertex */
    for(i1 = 0; i1 < fs.n[1]; i1++){
      for(i2 = 0; i2 < fs.n[2]; i2++){
	if(fi[i0][i1][i2] != NULL){
	  pf = fi[i0][i1][i2];
	  for(i = 0; i < 3; i++){
	    /*	    printf("%d %d %d %p %d %p\n",i0,i1,i2,pf,i,pf->l[i][1]);*/
	    if(pf->l[i][1] != NULL){
	      for(j = fs.val_num-1; j >= 0; j--){
		for(k = 0; k < cs.num; k++){
		  sa[0] = pf->s[j]         -cs.level[k];
		  sa[1] = pf->l[i][1]->s[j]-cs.level[k];
		  /*
		  if(i0 == 0 && i1 == 51 && i2 == 27){
		    printf("%d %d %d %d %d %d %f %f %f %f %f\n"
			   ,i0,i1,i2,i,j,k,pf->s[j],pf->l[i][1]->s[j],cs.level[k],sa[0],sa[1]);
		  }
		  */

		  if(sa[0] == 0.0 && sa[1] == 0.0){
		    sa[0] += 0.01*cs.level[k];
		    sa[1] -= 0.01*cs.level[k];
		  } else if(sa[0] == 0.0){
		    sa[0] += 0.01*cs.level[k];
		    /*
		    sa[0] += 0.01;
		    sa[1] -= 0.01;
		    */
		  } else if(sa[1] == 0.0){
		    sa[1] += 0.01*cs.level[k];
		    /*
		    sa[0] -= 0.01;
		    sa[1] += 0.01;
		    */
		  }
		  /*
		  if(sa[0] == 0.0 || sa[1] == 0.0){
		    ks_error("make contour error: "
			     "%d %d %d (%f %f %f) %f - (%f %f %f) %f at level %f",i0,i1,i2,
			     pf->cd[0],pf->cd[1],pf->cd[2],pf->s[j],
			     pf->l[i][1]->cd[0],
			     pf->l[i][1]->cd[1],
			     pf->l[i][1]->cd[2],
			     pf->l[i][1]->s[j],cs.level[k]);
		  }
		  */
		  if(sa[0]*sa[1] < 0){
		    sa[0] = fabs(sa[0]);
		    sa[1] = fabs(sa[1]);
		    for(c = 0; c < 3; c++){
		      dd[c] = (sa[1]*pf->cd[c]+sa[0]*pf->l[i][1]->cd[c])/(sa[0]+sa[1]);
		    }
		    /*
		    printf("%d %d %d  %d %d %d %f %f %f\n",i0,i1,i2,i,k,j,dd[0],dd[1],dd[2]);
		    */
		    pf->c[i][1] = add_contour(pf->c[i][1],new_contour(k,j,dd));
		    /*
		    if(i0 == 0 && i1 == 51 && i2 == 27){
		      printf("%d %p %d %d %f %f %f\n",i
			     ,pf->c[i][1],pf->c[i][1]->ln,pf->c[i][1]->vn
			     ,pf->c[i][1]->cd[0]
			     ,pf->c[i][1]->cd[1]
			     ,pf->c[i][1]->cd[2]);
		    }
		    */
		    pf->c[i][1]->l[0] = pf;
		    pf->c[i][1]->l[1] = pf->l[i][1];
		  }
		}
	      }
	      pf->l[i][1]->c[i][0] = pf->c[i][1];
	    }
	  }
	}
      }
    }
  }
  /*  ks_exit(EXIT_FAILURE);*/


  for(ix = 0; ix < fs.n[0]; ix++){ /* set contour connection */
    for(iy = 0; iy < fs.n[1]; iy++){
      for(iz = 0; iz < fs.n[2]; iz++){
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  /*
	  if(ix == 7 && iy == 15 && iz == 15){
	    printf("%d %d %d  %f %f %f  %f\n",ix,iy,iz,pf->cd[0],pf->cd[1],pf->cd[2],pf->s[0]);
	  }
	  */
	  for(i = 0; i < 3; i++){
	    for(j = 0; j < 2; j++){
	      if(pf->l[i][j] != NULL){
		/*
		printf("  lattice %d %d %f %f %f  %f %f %f\n",i,j,
		       pf->l[i][j]->cd[0],pf->l[i][j]->cd[1],pf->l[i][j]->cd[2]
		       ,pf->l[i][j]->s[0],pf->l[i][j]->s[1],pf->l[i][j]->s[2]);
		*/
		/*
		if(ix == 7 && iy == 15 && iz == 15){
		  printf("  lattice %d %d %f %f %f  %f\n",i,j,
			 pf->l[i][j]->cd[0],pf->l[i][j]->cd[1],pf->l[i][j]->cd[2],
			 pf->l[i][j]->s[0]);
		}
		*/
		for(pc = pf->c[i][j]; pc != NULL; pc = pc->next){
		  /*
		  printf("  lattice %d %d %f %f %f  %f %f %f\n",i,j
			 ,pf->cd[0],pf->cd[1],pf->cd[2],pf->s[0],pf->s[1],pf->s[2]);
		  printf("  lattice %d %d %f %f %f  %f %f %f\n",i,j
			 ,pf->l[i][j]->cd[0],pf->l[i][j]->cd[1],pf->l[i][j]->cd[2]
			 ,pf->l[i][j]->s[0],pf->l[i][j]->s[1],pf->l[i][j]->s[2]);
		  printf("    contour %d %d %f %f %f  %f\n",pc->ln,pc->vn
			 ,pc->cd[0],pc->cd[1],pc->cd[2],cn.level[pc->ln]);
		  */
		  /*
		  if(ix == 7 && iy == 15 && iz == 15){
		    printf("    contour %d %d %f %f %f  %f\n",pc->ln,pc->vn
			   ,pc->cd[0],pc->cd[1],pc->cd[2],cs.level[pc->ln]);
		  }
		  */
		  if(pf->s[pc->vn] < pf->l[i][j]->s[pc->vn]){
		    /*		    printf("up\n");*/
		    for(i0 = 0; i0 < 3; i0++){
		      if(i != i0){
			for(i1 = 0; i1 < 2; i1++){
			  if(pf->l[i][j]->l[i0][i1] != NULL){
			    /*
			    printf("      lattice %d %d  %f %f %f  %f\n",i0,i1
				   ,pf->l[i][j]->l[i0][i1]->cd[0]
				   ,pf->l[i][j]->l[i0][i1]->cd[1]
				   ,pf->l[i][j]->l[i0][i1]->cd[2]
				   ,pf->l[i][j]->l[i0][i1]->s[pc->vn]);
			    */
			    /*
			    printf("  lattice %d %d %f %f %f  %f %f %f\n",i0,i1
				   ,pf->l[i][j]->l[i0][i1]->cd[0]
				   ,pf->l[i][j]->l[i0][i1]->cd[1]
				   ,pf->l[i][j]->l[i0][i1]->cd[2]
				   ,pf->l[i][j]->l[i0][i1]->s[0]
				   ,pf->l[i][j]->l[i0][i1]->s[1]
				   ,pf->l[i][j]->l[i0][i1]->s[2]);
			    */
			    for(pcn = pf->l[i][j]->c[i0][i1]; pcn != NULL; pcn = pcn->next){
			      if(pc->vn == pcn->vn && pc->ln == pcn->ln){
				/*
				printf("hit0     contour %d %d %f %f %f  %f\n",pcn->ln,pcn->vn
				       ,pcn->cd[0],pcn->cd[1],pcn->cd[2]
				       ,cs.level[pcn->ln]);
				*/
				pc->c[a2p(i,i0)][i1] = pcn;
				goto hit;
			      }
			    }
			    /*
			    printf("  lattice %d %d %f %f %f  %f %f %f\n",i0,i1
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->cd[0]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->cd[1]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->cd[2]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->s[0]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->s[1]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->s[2]);
			    */
			    for(pcn = pf->l[i][j]->l[i0][i1]->c[i][j==0?1:0]; 
				pcn != NULL; pcn = pcn->next){
			      if(pc->vn == pcn->vn && pc->ln == pcn->ln){
				/*
				printf("hit1     contour %d %d %f %f %f  %f\n",pcn->ln,pcn->vn
				       ,pcn->cd[0],pcn->cd[1],pcn->cd[2]
				       ,cs.level[pcn->ln]);
				*/
				pc->c[a2p(i,i0)][i1] = pcn;
				goto hit;
			      }
			    }
			    /*
			    printf("  lattice %d %d %f %f %f  %f %f %f\n",i0,i1
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->l[i0][i1==0?1:0]->cd[0]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->l[i0][i1==0?1:0]->cd[1]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->l[i0][i1==0?1:0]->cd[2]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->l[i0][i1==0?1:0]->s[0]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->l[i0][i1==0?1:0]->s[1]
				   ,pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->l[i0][i1==0?1:0]->s[2]
				   );
			    */
			    for(pcn = pf->l[i][j]->l[i0][i1]->l[i][j==0?1:0]->c[i0][i1==0?1:0]; 
				pcn != NULL; pcn = pcn->next){
			      if(pc->vn == pcn->vn && pc->ln == pcn->ln){
				/*
				printf("hit2     contour %d %d %f %f %f  %f\n",pcn->ln,pcn->vn
				       ,pcn->cd[0],pcn->cd[1],pcn->cd[2]
				       ,cs.level[pcn->ln]);
				*/
				pc->c[a2p(i,i0)][i1] = pcn;
				goto hit;
			      }
			    }
			    ks_error("exit %d %d %d %d %d",ix,iy,iz,i,j);
			    return KS_FALSE;
			  hit:
			    ;
			  }
			}
		      }
		    }
		  } else {
		    /*		    printf("down\n");*/
		  }
		}
	      } else {
		/*		printf("  lattice %d %d NULL\n",i,j);*/
	      }
	    }
	  }
	}
      }
    }
  }

  /*
  for(ix = 0; ix < fs.n[0]; ix++){
    for(iy = 0; iy < fs.n[1]; iy++){
      for(iz = 0; iz < fs.n[2]; iz++){
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  printf("%d %d %d %p\n",ix,iy,iz,pf->c);
	}
      }
    }
  }
  */

  free_flg = 1;
  return KS_TRUE;
}
BOOL fv_calc_contour_surface(FV_FIELD ****fi, FV_FIELD_SIZE fs, FV_CONTOUR_SIZE cs,
			     FV_CONTOUR_FAN ***csf)
{
  int i0,i1,i2,i3,i4;
  int ix,iy,iz;
  int i,j,k,c,oc;

  FV_FIELD *pf;
  FV_CONTOUR *pc;//,*pcn;
  double lv[4],nv[4],vc[2][3];

  int il,iv;
  /*
  FV_CONTOUR *pcs[cs.num][fs.val_num][20],*pcs_next,*pcs_prev;
  FV_CONTOUR *pcp[cs.num][fs.val_num][2],*pcs_add[20],*pcs_new;
  int pcs_num[cs.num][fs.val_num];
  */
  enum {
    PCS_NUM_MAX = 20
  };
  FV_CONTOUR ****pcs,*pcs_next,*pcs_prev;
  FV_CONTOUR ****pcp,*pcs_add[20],*pcs_new;
  int **pcs_num;

  FV_CONTOUR_FAN *fa;

  static int free_flg = 0;

  /*
  printf("fv_calc_field_contour\n");
  */

  if((pcs = (FV_CONTOUR****)ks_malloc(cs.num*sizeof(FV_CONTOUR***),"pcs")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(i = 0; i < cs.num;i++){
    if((pcs[i] = (FV_CONTOUR***)ks_malloc(fs.val_num*sizeof(FV_CONTOUR**),"pcs[i]")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  for(i = 0; i < cs.num;i++){
    for(j = 0; j < fs.val_num; j++){
      if((pcs[i][j] = (FV_CONTOUR**)ks_malloc(PCS_NUM_MAX*sizeof(FV_CONTOUR*),"pcs[i][j]"))==NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    }
  }
  if((pcp = (FV_CONTOUR****)ks_malloc(cs.num*sizeof(FV_CONTOUR***),"pcp")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(i = 0; i < cs.num;i++){
    if((pcp[i] = (FV_CONTOUR***)ks_malloc(fs.val_num*sizeof(FV_CONTOUR**),"pcp[i]")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  for(i = 0; i < cs.num;i++){
    for(j = 0; j < fs.val_num; j++){
      if((pcp[i][j] = (FV_CONTOUR**)ks_malloc(2*sizeof(FV_CONTOUR*),"pcp[i][j]"))==NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    }
  }
  if((pcs_num = ks_malloc_int_pp(cs.num,fs.val_num,"pcs_num")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }

  if(free_flg == 1){

    for(i = 0; i < cs.num; i++){
      for(j = 0; j < fs.val_num; j++){
	for(fa = csf[i][j]; fa != NULL; fa = fa->next){
	  if(fa->num != 3){
	    free_contour(fa->pcs[0]);
	  }
	  ks_free(fa->pcs);
	  /*
	  for(k = 0; k < fa->num; k++){
	    if(fa->pcs[k] != NULL){
	      printf("%d %d %d\n",i,j,k);
	      for(pc = fa->pcs[k]; pc != NULL; pc = pcn){
		pcn = pc->next;
		free(pc);
		pc = NULL;
	      }
	    }
	  }
	  */
	}
	free_contour_fan(csf[i][j]);
	csf[i][j] = NULL;
      }
    }

  }

#if 1
  for(ix = 0; ix < fs.n[0]; ix++){ /* set normal vector */
    for(iy = 0; iy < fs.n[1]; iy++){
      for(iz = 0; iz < fs.n[2]; iz++){
	/*	ix = 5; iy = 86; iz = 0;*/
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  /*
	  printf("%d %d %d %d (%f %f %f) %f %f %f\n",ix,iy,iz,pf
		 ,pf->cd[0],pf->cd[1],pf->cd[2],pf->s[0],pf->s[1],pf->s[2]);
	  */
	  /*	  printf("%d %d %d %p\n",ix,iy,iz,pf->c);*/
	  /*	  printf("%d %d %d %p\n",ix,iy,iz,pf->c[0][0]);*/
	  for(i = 0; i < 3; i++){
	    for(j = 0; j < 2; j++){
	      for(pc = pf->c[i][j]; pc != NULL; pc = pc->next){
		/*
		printf("  %d %d %d (%f %f %f) %d %d\n",pc,i,j,pc->cd[0],pc->cd[1],pc->cd[2]
		       ,pc->ln,pc->vn);
		*/
		k = pc->l[0]->s[pc->vn] < pc->l[1]->s[pc->vn] ? 0:1;
		for(c = 0; c < 3; c++)
		  lv[c] = pc->l[k]->cd[c] - pc->cd[c];
		lv[3] = sqrt(lv[0]*lv[0]+lv[1]*lv[1]+lv[2]*lv[2]);
		for(c = 0; c < 3; c++)
		  lv[c] /= lv[3];

		for(c = 0; c < 3; c++)
		  pc->nv[c] = 0;
		c = 0;
		for(i0 = 0; i0 < 3; i0++){
		  for(i1 = 0; i1 < 2; i1++){
		    if(pc->c[i0][i1] != NULL){
		      for(i4 = 0; i4 < 3; i4++)
			vc[0][i4] = pc->c[i0][i1]->cd[i4] - pc->cd[i4];
		      for(i2 = i0+1; i2 < 3; i2++){
			for(i3 = 0; i3 < 2; i3++){
			  if(pc->c[i2][i3] != NULL){
			    for(i4 = 0; i4 < 3; i4++)
			      vc[1][i4] = pc->c[i2][i3]->cd[i4] - pc->cd[i4];
			    nv[0] = vc[0][1]*vc[1][2] - vc[0][2]*vc[1][1];
			    nv[1] = vc[0][2]*vc[1][0] - vc[0][0]*vc[1][2];
			    nv[2] = vc[0][0]*vc[1][1] - vc[0][1]*vc[1][0];
			    nv[3] = 1./sqrt(nv[0]*nv[0]+nv[1]*nv[1]+nv[2]*nv[2]);
			    for(i4 = 0; i4 < 3; i4++)
			      nv[i4] *= nv[3]; 
			    if(lv[0]*nv[0]+lv[1]*nv[1]+lv[2]*nv[2] < 0){
			      for(i4 = 0; i4 < 3; i4++)
				nv[i4] *= -1;
			    }
			    /*
			    printf("%d %d %d %d (% f % f % f)(% f % f % f)(% f % f % f)\n",
				   i0,i1,i2,i3
				   ,vc[0][0],vc[0][1],vc[0][2]
				   ,vc[1][0],vc[1][1],vc[1][2]
				   ,nv[0],nv[1],nv[2]);
			    */
			    for(i4 = 0; i4 < 3; i4++)
			      pc->nv[i4] += nv[i4];
			    c++;
			  }
			}
		      }		    
		    }
		  }
		}
		for(i4 = 0; i4 < 3; i4++)
		  pc->nv[i4] /= c;

		/*
		c = 0;
		for(i0 = 0; i0 < 3; i0++){
		  for(i1 = 0; i1 < 2; i1++){
		    pcn = pc->c[i0][i1];
		    if(pcn != NULL){
		      c++;
		      printf("    %d %d %d (%f %f %f) %d %d\n",i0,i1,pcn
			     ,pcn->cd[0],pcn->cd[1],pcn->cd[2]
			     ,pcn->ln,pcn->vn);
		    }
		  }
		}
		*/
	      }
	    }
	  }
	}
      }
    }
  }
#endif

#if 1
  /*
  for(ix = 0; ix < fs.n[0]; ix++){
    for(iy = 0; iy < fs.n[1]; iy++){
      for(iz = 0; iz < fs.n[2]; iz++){
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  for(i = 0; i < 3; i++){ 
	    if(pf->l[i][1] != NULL){
	      for(pc = pf->c[i][1]; pc != NULL; pc = pc->next){
		if(pc == 285483352){
		  printf("%d %d %d %d\n",ix,iy,iz,i);
		  for(j = 0; j < 3; j++){
		    for(k = 0; k < 2; k++){
		      printf("  %d %d %d\n",j,k,pc->c[j][k]);
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
  */
  for(ix = 0; ix < fs.n[0]-1; ix++){    /* set contour surface */
    for(iy = 0; iy < fs.n[1]-1; iy++){
      for(iz = 0; iz < fs.n[2]-1; iz++){
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  for(i = 0; i < cs.num; i++)
	    for(j = 0; j < fs.val_num; j++)
	      pcs_num[i][j] = 0;
	  for(i = 0; i < 3; i++){ /* make vertex list */
	    if(pf->l[i][1] != NULL){
	      for(pc = pf->c[i][1]; pc != NULL; pc = pc->next){
		/*
		if(pc->vn == tg.val){
		  pcs[pcs_num++] = pc;
		}
		*/
		if(pcs_num[pc->ln][pc->vn] < PCS_NUM_MAX){
		  pcs[pc->ln][pc->vn][pcs_num[pc->ln][pc->vn]++] = pc;
		}
	      }
	      for(j = 0; j < 3; j++){
		if(pf->l[i][1]->l[j][1] != NULL && pf->l[i][1]->l[j][1] != pf && i != j){
		  for(pc = pf->l[i][1]->c[j][1]; pc != NULL; pc = pc->next){
		    /*
		    if(pc->vn == tg.val){
		      pcs[pcs_num++] = pc;
		    }
		    */
		    if(pcs_num[pc->ln][pc->vn] < PCS_NUM_MAX){
		      pcs[pc->ln][pc->vn][pcs_num[pc->ln][pc->vn]++] = pc;
		    }
		  }
		}
		if((i == 1 && j == 2) || (i == 2 && j == 0) || (i == 0 && j == 1) ){
		  if(i == 1 && j == 2) k = 0;
		  if(i == 2 && j == 0) k = 1;
		  if(i == 0 && j == 1) k = 2;
		  for(pc = pf->l[i][1]->l[j][1]->c[k][1]; pc != NULL; pc = pc->next){
		    /*
		    if(pc->vn == tg.val){
		      pcs[pcs_num++] = pc;
		    }
		    */
		    if(pcs_num[pc->ln][pc->vn] < PCS_NUM_MAX){
		      pcs[pc->ln][pc->vn][pcs_num[pc->ln][pc->vn]++] = pc;
		    }
		  }
		}
	      }
	    }
	  }
	  /*
	  if(ix == 0 && iy == 2 && iz == 5)
	  for(il = 0; il < cs.num; il++){
	    for(iv = 0; iv < fs.val_num; iv++){
	      printf("%d %d %d\n",il,iv,pcs_num[il][iv]);
	      for(i = 0; i < pcs_num[il][iv]; i++){
		printf("%d %d %f %f %f\n",i,pcs[il][iv][i]
		       ,pcs[il][iv][i]->cd[0],pcs[il][iv][i]->cd[1],pcs[il][iv][i]->cd[2]);
		for(j = 0; j < 3; j++){
		  for(k = 0; k < 2; k++){
		    printf("   %d %d\n",pcs[il][iv][i]->c[j][k]);
		  }
		}
	      }
	    }
	  }
	  */
	  for(il = 0; il < cs.num; il++){ /* set neighbor vertex */
	    for(iv = 0; iv < fs.val_num; iv++){
	      for(i = 0; i < pcs_num[il][iv]; i++){
		/*
		if(pcs_num[il][iv] >= 7)
		  printf("%d %d %f %f %f\n",i,pcs[il][iv][i]
			 ,pcs[il][iv][i]->cd[0],pcs[il][iv][i]->cd[1],pcs[il][iv][i]->cd[2]);
		*/
		i1 = 0;
		for(j = 0; j < 3; j++){
		  for(k = 0; k < 2; k++){
		    if(pcs[il][iv][i]->c[j][k] != NULL){
		      /*
		      if(pcs_num[il][iv] >= 7)
			printf("  %d %d %d %f %f %f\n",j,k
			       ,pcs[il][iv][i]->c[j][k]
			       ,pcs[il][iv][i]->c[j][k]->cd[0]
			       ,pcs[il][iv][i]->c[j][k]->cd[1]
			       ,pcs[il][iv][i]->c[j][k]->cd[2]);
		      */
		      for(i0 = 0; i0 < pcs_num[il][iv]; i0++){
			if(pcs[il][iv][i0] == pcs[il][iv][i]->c[j][k]){
			  /*
			  if(pcs_num[il][iv] >= 7)
			    printf("  hit %d %d %d\n",j,k,pcs[il][iv][i]->c[j][k]);
			  */
			  pcp[il][iv][i1] = pcs[il][iv][i]->c[j][k];
			  for(i2 = 0; i2 < 3; i2++)
			    vc[i1][i2] = pcs[il][iv][i0]->cd[i2] - pcs[il][iv][i]->cd[i2];
			  i1++;
			}
		      }
		    }
		  }
		}
		/*
		if(i1 == 0){
		  printf("(%d %d %d) %d %d %d %d %f %f %f\n",ix,iy,iz,il,iv,i,pcs[il][iv][i]
			 ,pcs[il][iv][i]->cd[0],pcs[il][iv][i]->cd[1],pcs[il][iv][i]->cd[2]);
		  for(j = 0; j < 3; j++){
		    for(k = 0; k < 2; k++){
		      if(pcs[il][iv][i]->c[j][k] != NULL){
			printf("  %d %d %d %f %f %f\n",j,k
			       ,pcs[il][iv][i]->c[j][k]
			       ,pcs[il][iv][i]->c[j][k]->cd[0]
			       ,pcs[il][iv][i]->c[j][k]->cd[1]
			       ,pcs[il][iv][i]->c[j][k]->cd[2]);
		      }
		    }
		  }
		  printf("no hits\n");
		  ks_exit(EXIT_FAILURE);
		}
		*/
		/*
		  if(pcs_num > 5){
		  for(i0 = 0; i0 < 2; i0++){
		  printf("pcp %d %d",i0,pcp[i0]);
		  for(i1 = 0; i1 < 3; i1++){
		  printf(" %f",vc[i0][i1]);
		  }
		  printf("\n");
		  }
		  }
		*/

		nv[0] = vc[0][1]*vc[1][2] - vc[0][2]*vc[1][1];
		nv[1] = vc[0][2]*vc[1][0] - vc[0][0]*vc[1][2];
		nv[2] = vc[0][0]*vc[1][1] - vc[0][1]*vc[1][0];
		if(pcs[il][iv][i]->nv[0]*nv[0]+
		   pcs[il][iv][i]->nv[1]*nv[1]+
		   pcs[il][iv][i]->nv[2]*nv[2] > 0){
		  pcs[il][iv][i]->loop[0] = pcp[il][iv][0];
		  pcs[il][iv][i]->loop[1] = pcp[il][iv][1];
		} else {
		  pcs[il][iv][i]->loop[0] = pcp[il][iv][1];
		  pcs[il][iv][i]->loop[1] = pcp[il][iv][0];
		}
		/*
		pcs[il][iv][i]->loop[0] = pcp[il][iv][0];
		pcs[il][iv][i]->loop[1] = pcp[il][iv][1];
		*/
	      }
	      /*
	      if(pcs_num[il][iv] >= 7)
	      */
	      /*
	      if(ix == 0 && iy == 2 && iz == 5 && il == 0 && iv == 0){
		printf("%d %d %d %d %d %d\n",ix,iy,iz,il,iv,pcs_num[il][iv]);
		for(i = 0; i < pcs_num[il][iv]; i++){
		  printf("%d %d %d %d %f %f %f -> %d %d\n",il,iv
			 ,i,pcs[il][iv][i]
			 ,pcs[il][iv][i]->cd[0],pcs[il][iv][i]->cd[1],pcs[il][iv][i]->cd[2]
			 ,pcs[il][iv][i]->loop[0],pcs[il][iv][i]->loop[1]);
		  for(j = 0; j < 3; j++){
		    for(k = 0; k < 2; k++){
		      printf("   %d %d %d\n",j,k,pcs[il][iv][i]->c[j][k]);
		    }
		  }
		}
	      }
	      */

	      for(i = 0; i < pcs_num[il][iv]; i++){ /* set contour fan */
		if(pcs[il][iv][i]->loop[0] != NULL){
		  for(pc = pcs_prev = pcs[il][iv][i], c = 0, oc = 0; 
		      pc != pcs[il][iv][i] || c == 0; pc = pcs_next){
		    /*
		    if(pcs_num[il][iv] >= 7)
		    */
		    /*
		    if(idle_counter == 5 && ix == 2 && iy == 69 && iz == 5){
		      printf("%d %d\n",c,pc);
		      printf("%d %d %f %f %f %d %d\n",c,pc,pc->cd[0],pc->cd[1],pc->cd[2]
			     ,pc->loop[0],pc->loop[1]);
		    }
		    */
		    /*
		    if(pc == NULL){
		      printf("%d %d %d %d %d %d %d\n",ix,iy,iz,il,iv,c,pcs_num[il][iv]);
		      for(i = 0; i < pcs_num[il][iv]; i++){
			printf("%d %d %d %d %f %f %f -> %d %d\n",il,iv
			       ,i,pcs[il][iv][i]
			       ,pcs[il][iv][i]->cd[0],pcs[il][iv][i]->cd[1],pcs[il][iv][i]->cd[2]
			       ,pcs[il][iv][i]->loop[0],pcs[il][iv][i]->loop[1]);
			for(j = 0; j < 3; j++){
			  for(k = 0; k < 2; k++){
			    printf("   %d %d %d\n",j,k,pcs[il][iv][i]->c[j][k]);
			  }
			}
		      }
		      ks_exit(EXIT_FAILURE);
		    }
		    */
		    /*
		    if(ix == 0 && iy == 2 && iz == 5){
		      printf("* %d %d\n",c,pc);
		      printf("* %d %d %f %f %f %d %d\n",c,pc,pc->cd[0],pc->cd[1],pc->cd[2]
			     ,pc->loop[0],pc->loop[1]);
		    }
		    */
		    if(pc == NULL){
		      ks_error("Please change level min and min (now min %f max %f)\n",
			       fs.min,fs.max);
		      ks_exit(EXIT_FAILURE);
		    }
		    pcs_add[c] = pc;
		    if(c == 0 || pcs_prev != pc->loop[0]){
		      pcs_next = pc->loop[0];
		    } else {
		      oc++;
		      pcs_next = pc->loop[1]; /* opposite direction */
		    }
		    /*
		    if(pcs_next == NULL){
		      printf("pcs_next == NULL %d %d\n",c,pc);
		      printf("%d %d %f %f %f %d %d\n",c,pc,pc->cd[0],pc->cd[1],pc->cd[2]
			     ,pc->loop[0],pc->loop[1]);
		      		      ks_exit(EXIT_FAILURE);
		    }
		    */
		    pcs_prev = pc;
		    pc->loop[0] = NULL;
		    c++;
		  }
		  /*
		    if(pcs_num != c){
		    printf("error -----------------------------\n");
		    }
		  */
		  /*
		  if(idle_counter == 5 && ix == 2 && iy == 69 && iz == 5){
		    for(j = 0; j < c; j++)
		      printf("%d %d %d %d %d %d %f %f %f % f % f % f\n",oc,il,iv,i,j,c
			     ,pcs_add[j]->cd[0],pcs_add[j]->cd[1],pcs_add[j]->cd[2]
			     ,pcs_add[j]->nv[0],pcs_add[j]->nv[1],pcs_add[j]->nv[2]
			     );
		  }
		  */
		  if(c < 3){
		    printf("vertex num is too small\n");
		    ks_exit(EXIT_FAILURE);
		  } else if(c == 3){
		    csf[il][iv]=add_contour_fan(csf[il][iv],
						new_contour_fan(2,pcs_add[0],&pcs_add[1],0));
		  } else {
		    for(j = 0; j < 2; j++)
		      for(k = 0; k < 3; k++)
			vc[j][k] = 0;
		    for(j = 0; j < c; j++){
		      for(k = 0; k < 3; k++){
			vc[0][k] += pcs_add[j]->cd[k];
			vc[1][k] += pcs_add[j]->nv[k];
		      }
		    }
		    for(j = 0; j < 2; j++)
		      for(k = 0; k < 3; k++)
			vc[j][k] /= c;
		    /*
		    printf(" -> %f %f %f % f % f % f\n",vc[0][0],vc[0][1],vc[0][2]
			   ,vc[1][0],vc[1][1],vc[1][2]);
		    */
		    pcs_new = new_contour(pcs_add[0]->ln,pcs_add[0]->vn,vc[0]);
		    for(k = 0; k < 3; k++)
		      pcs_new->nv[k] = vc[1][k];
		    csf[il][iv]=add_contour_fan(csf[il][iv],
						new_contour_fan(c,pcs_new,pcs_add, oc>2?  1:0));
		  }
		  /*
		  csp[il][iv] = add_contour_polygon(csp[il][iv], new_contour_polygon(c,pcs_add));
		  */
		}
	      }
	    }
	  }
	}
      }
    }
  }
#endif
  /*
  for(il = 0; il < cs.num; il++){
    for(iv = 0; iv < fs.val_num; iv++){
      printf("%d %d %d\n",il,iv,csp[il][iv]);
    }
  }
  */
  /*
  {
    FV_CONTOUR_POLYGON *po;
    for(po = csp; po != NULL; po = po->next){
      printf("%d\n",po->num);
      for(i = 0; i < po->num; i++){
	printf("  %d (%f %f %f)\n",i,po->pcs[i]->cd[0],po->pcs[i]->cd[1],po->pcs[i]->cd[2]);
      }
    }
  }
  ks_exit(EXIT_FAILURE);
  */
  /*
  {
    FV_CONTOUR_FAN *po;
    for(po = csf[0][0]; po != NULL; po = po->next){
      printf("%d\n",po->num);
      for(i = 0; i < po->num; i++){
	printf("  %d (%f %f %f)\n",i,po->pcs[i]->cd[0],po->pcs[i]->cd[1],po->pcs[i]->cd[2]);
      }
    }
  }
  ks_exit(EXIT_FAILURE);
  */
#if 0
  for(ix = 0; ix < fs.n[0]; ix++){ /* remove overlap conture */
    for(iy = 0; iy < fs.n[1]; iy++){
      for(iz = 0; iz < fs.n[2]; iz++){
	if(fi[ix][iy][iz] != NULL){
	  pf = fi[ix][iy][iz];
	  for(i = 0; i < 3; i++){
	    for(j = 0; j < 2; j++){
	      for(pc = pf->c[i][j]; pc != NULL; pc = pc->next){
		for(i0 = 0; i0 < 3; i0++){
		  for(i1 = 0; i1 < 2; i1++){
		    pcn = pc->c[i0][i1];
		    if(pcn != NULL){
		      for(i2 = 0; i2 < 3; i2++){
			for(i3 = 0; i3 < 2; i3++){
			  if(pcn->c[i2][i3] != NULL){
			    if(pc == pcn->c[i2][i3]){
			      pcn->c[i2][i3] = NULL; /* remove */
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

  for(i = 0; i < cs.num;i++){
    for(j = 0; j < fs.val_num; j++){
      ks_free(pcs[i][j]);
    }
  }
  for(i = 0; i < cs.num;i++){
    ks_free(pcs[i]);
  }
  ks_free(pcs);

  for(i = 0; i < cs.num;i++){
    for(j = 0; j < fs.val_num; j++){
      ks_free(pcp[i][j]);
    }
  }
  for(i = 0; i < cs.num;i++){
    ks_free(pcp[i]);
  }
  ks_free(pcp);
  ks_free_int_pp(cs.num,pcs_num);

  free_flg = 1;
  return KS_TRUE;
}
void fv_free_base(FV_BASE *fv)
{
  if(fv->field != NULL){
    fv_free_field(fv->field,fv->field_size);
  }
  if(fv->contour_fan != NULL){
    fv_free_contour_fan(fv->contour_fan,fv->contour_size, fv->field_size);
    fv_free_contour_size(fv->contour_size);
  }
}
void fv_init_base(FV_BASE *fv)
{
  fv->field = NULL;
  fv->contour_size.level = NULL;
  fv->contour_size.color = NULL;
  fv->contour_fan = NULL;
  fv->black_and_white = KS_FALSE;
}
void fv_init_target(FV_TARGET *ft)
{
  ft->level = NULL;
}
BOOL fv_malloc_field(FV_FIELD *****fi, FV_FIELD_SIZE *fs)
{
  int i,j,k;
  int l,m;

  if((*fi) == NULL){
    if(((*fi) = (FV_FIELD****)ks_malloc(fs->n[0]*sizeof(FV_FIELD***),"fi")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < fs->n[0]; i++){
      if(((*fi)[i] = (FV_FIELD***)ks_malloc(fs->n[1]*sizeof(FV_FIELD**),"fi[i]")) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    }
    for(i = 0; i < fs->n[0]; i++){
      for(j = 0; j < fs->n[1]; j++){
	if(((*fi)[i][j] = (FV_FIELD**)ks_malloc(fs->n[2]*sizeof(FV_FIELD*),"fi[i][j]")) == NULL){
	  ks_error_memory();
	  return KS_FALSE;
	}
      }
    }
    for(i = 0; i < fs->n[0]; i++){
      for(j = 0; j < fs->n[1]; j++){
	for(k = 0; k < fs->n[2]; k++){
	  if(((*fi)[i][j][k] = (FV_FIELD*)ks_malloc(sizeof(FV_FIELD),"fi")) == NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }
	  if(((*fi)[i][j][k]->s=(double*)ks_malloc(fs->val_num*sizeof(double),"fi->s")) ==NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }

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
  return KS_TRUE;
}
BOOL fv_get_contour_level_color(FV_CONTOUR_SIZE *cs, int level, float color[3])
{
  int i;
  if(level >= 0 && level <= cs->num){
    for(i = 0; i < 3; i++){
      color[i] =  cs->color[level][i];
    }
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
void fv_set_contour_level_color(FV_CONTOUR_SIZE *cs, int level, float *color)
{
  int i;
  if(level >= 0 && level <= cs->num){
    for(i = 0; i < 3; i++){
      cs->color[level][i] = color[i];
    }
  }
}
BOOL fv_get_pov_water_flag()
{
  return water_flg;
}
BOOL fv_get_pov_bubble_flag()
{
  return bubble_flg;
}
void fv_set_pov_water_flag(BOOL value)
{
  water_flg = value;
}
void fv_set_pov_bubble_flag(BOOL value)
{
  bubble_flg = value;
}


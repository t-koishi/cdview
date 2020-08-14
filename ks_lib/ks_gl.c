
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#ifdef ICL
#include <mathimf.h>
#else
#include <math.h>
#endif

#ifdef MSVC
#pragma warning(disable:4305)
#pragma warning(disable:4244)
#pragma warning(disable:4101)
#include <windows.h>
#endif

#ifdef USE_GLSL
#include <glew.h>
#endif

#ifdef GLUT
#include <GL/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "ks_gl.h"
#include "ks_std.h"
#include "ks_atom.h"
#include "ks_math.h"

#ifdef ENABLE_DOME

enum {
  DOME_MODE_SYNRA_01,
  DOME_MODE_SYNRA_02,
  DOME_MODE_SYNRA_03,
  DOME_MODE_SYNRA_04,
  DOME_MODE_SYNRA_05,
  DOME_MODE_SYNRA_06,
  DOME_MODE_SYNRA_07,
  DOME_MODE_SYNRA_08,
  DOME_MODE_SYNRA_09,
  DOME_MODE_SYNRA_10,
  DOME_MODE_SYNRA_11,
  DOME_MODE_SYNRA_12,
  DOME_MODE_NUM
};
enum {
  NROTATE = 3,
};
//#define Z_FAR 100
#define Z_FAR 800
float dome_frustum[DOME_MODE_NUM][6] = {{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, 1e20},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, 1e20},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.0699895, 0.0699895, -0.0752679, 0.0752679, 0.1, Z_FAR},
					{-0.0699895, 0.0699895, -0.0752679, 0.0752679, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR},
					{-0.108422, 0.108422, -0.0693562, 0.0693562, 0.1, Z_FAR}};
float dome_rotate[DOME_MODE_NUM][NROTATE][4] = 
  {{{20, 0, 0, 1}, {-32.5, 1.0, 0.0, 0.0}, {-60.0, 0.0, 1.0, 0.0}},
   {{20, 0, 0, 1}, {-32.5, 1.0, 0.0, 0.0}, {-60.0, 0.0, 1.0, 0.0}},
   {{17.7, 0, 0, 1}, {-53.3, 1.0, 0.0, 0.0}, {-131.0, 0.0, 1.0, 0.0}},
   {{17.7, 0, 0, 1}, {-53.3, 1.0, 0.0, 0.0}, {-131.0, 0.0, 1.0, 0.0}},
   {{-21.705, 1, 0, 0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}},
   {{-21.705, 1, 0, 0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}},
   {{-71.968, 1, 0, 0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}},
   {{-71.968, 1, 0, 0}, {0.0, 1.0, 0.0, 0.0}, {0.0, 0.0, 1.0, 0.0}},
   {{-20.0, 0, 0, 1}, {-32.5, 1.0, 0.0, 0.0}, {60.0, 0.0, 1.0, 0.0}},
   {{-20.0, 0, 0, 1}, {-32.5, 1.0, 0.0, 0.0}, {60.0, 0.0, 1.0, 0.0}},
   {{-17.7, 0, 0, 1}, {-53.3, 1.0, 0.0, 0.0}, {131.0, 0.0, 1.0, 0.0}},
   {{-17.7, 0, 0, 1}, {-53.3, 1.0, 0.0, 0.0}, {131.0, 0.0, 1.0, 0.0}}};
int dome_eye_flag[DOME_MODE_NUM] = {1,-1,
				    1,-1,
				    1,-1,
				    1,-1,
				    1,-1,
				    1,-1};
#endif

static GLubyte *ks_read_ppm_file(char *file_name, int *w, int *h, KS_CHAR_LIST *search_path);
static KS_GL_WINDOW *ks_new_gl_window(int id, unsigned int flags, char *name,
				      int pos_x, int pos_y, int width, int height, 
				      void (*display)(KS_GL_BASE*,int,int,int,void*),
				      void (*mouse)(KS_GL_BASE*,int,int,int,int,int,int,int,void*),
				      void (*motion)(KS_GL_BASE*,int,int,int,int,int,void*),
				      void (*passive)(KS_GL_BASE*,int,int,int,int,int,void*),
				      void *vp)
{
  KS_GL_WINDOW *newp;

  if((newp = (KS_GL_WINDOW*)ks_malloc(sizeof(KS_GL_WINDOW),"KS_GL_WINDOW")) == NULL){
    ks_error("new_gl_window: memory error");
    return NULL;
  }
  newp->id = id;
  ks_strncpy(newp->name,name,sizeof(newp->name));
  newp->flags = flags;
  newp->viewport[KS_GL_VIEWPORT_X] = pos_x;
  newp->viewport[KS_GL_VIEWPORT_Y] = pos_y;
  newp->viewport[KS_GL_VIEWPORT_WIDTH] = width;
  newp->viewport[KS_GL_VIEWPORT_HEIGHT] = height;
  newp->vp = vp;
  newp->display = display;
  newp->mouse = mouse;
  newp->motion = motion;
  newp->passive = passive;
  newp->next = NULL;
  return newp;
}
static KS_GL_WINDOW *__KS_USED__ ks_add_gl_window(KS_GL_WINDOW *listp, KS_GL_WINDOW *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_GL_WINDOW *ks_addend_gl_window(KS_GL_WINDOW *listp, KS_GL_WINDOW *newp)
{
  KS_GL_WINDOW *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static KS_GL_WINDOW *__KS_USED__ ks_lookup_gl_window_name(KS_GL_WINDOW *listp, char *name)
{
  for(; listp != NULL; listp = listp->next){
    if(strcmp(listp->name,name) == 0)
      return listp;
  }
  return NULL;
}
static KS_GL_WINDOW *ks_lookup_gl_window(KS_GL_WINDOW *listp, int id)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->id == id)
      return listp;
  }
  return NULL;
}
static int __KS_USED__ ks_count_gl_window(KS_GL_WINDOW *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static KS_GL_WINDOW *ks_del_gl_window(KS_GL_WINDOW *listp, int id)
{
  KS_GL_WINDOW *p, *prev;

  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(p->id == id){
      if(prev == NULL)
	listp = p->next;
      else
	prev->next = p->next;
      ks_free(p);
      return listp;
    }
    prev = p;
  }
  return listp;
}
static void ks_free_gl_window(KS_GL_WINDOW *listp)
{
  KS_GL_WINDOW *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
static KS_GL_WINDOW *ks_move_end_gl_window(KS_GL_WINDOW *listp, int id)
{
  KS_GL_WINDOW *p;
  KS_GL_WINDOW *prev = NULL, *hit = NULL;
  for(p = listp; p->next != NULL; p = p->next){
    if(p->id == id){
      hit = p;
      if(prev != NULL){
	prev->next = p->next;
      } else {
	listp = p->next;
      }
    }
    prev = p;
  }
  if(hit != NULL){
    hit->next = NULL;
    p->next = hit;
  }
  /*
  for(p = listp; p != NULL; p = p->next){
    printf("b %d %s\n",p->id,p->name);
  }
  */
  return listp;
}
static void draw_gl_window_frame_upper_left(KS_GL_WINDOW *p)
{
  glVertex2i(0,p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_TITLE_BAR_SIZE);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE*.5-1,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_FRAME_SIZE*.5+KS_GL_WINDOW_TITLE_BAR_SIZE+1);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_TITLE_BAR_SIZE);
}
static void draw_gl_window_frame_lower_left(KS_GL_WINDOW *p)
{
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,0);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE*.5-1,KS_GL_WINDOW_FRAME_SIZE*.5-1);
  glVertex2i(0,KS_GL_WINDOW_FRAME_SIZE);
}
static void draw_gl_window_frame_lower_right(KS_GL_WINDOW *p)
{

  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*2,
	     KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*1.5+1,
	     KS_GL_WINDOW_FRAME_SIZE*.5-1);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,0);
}
static void draw_gl_window_frame_upper_right(KS_GL_WINDOW *p)
{
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_TITLE_BAR_SIZE+KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*1.5+1,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_TITLE_BAR_SIZE+KS_GL_WINDOW_FRAME_SIZE*.5+1);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*2,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_TITLE_BAR_SIZE);
}
static void draw_gl_window_frame(KS_GL_BASE *gb, KS_GL_WINDOW *p)
{
  int i;
  GLboolean blend;
  GLboolean lighting;
  GLfloat back_trans = 0.5;
  GLfloat frame_trans = 0.7;
  GLfloat edge_trans = 0.9;
  GLfloat edge = 0.3;
  GLfloat color[3];

  for(i = 0; i < 3; i++){
    if(gb->background_color[i] > 0.5){
      color[i] = gb->background_color[i]-0.3;
    } else {
      color[i] = gb->background_color[i]+0.3;
    }
  }

  /*
  printf("%f %f %f %f\n"
	 ,gb->background_color[0]
	 ,gb->background_color[1]
	 ,gb->background_color[2]
	 ,gb->background_color[3]);
  printf("%f %f %f\n",color[0],color[1],color[2]);
  */

  glViewport((GLint)(p->viewport[KS_GL_VIEWPORT_X]-KS_GL_WINDOW_FRAME_SIZE),
	     (GLint)(p->viewport[KS_GL_VIEWPORT_Y]-KS_GL_WINDOW_FRAME_SIZE),
	     (GLsizei)(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*2+1),
	     (GLsizei)(p->viewport[KS_GL_VIEWPORT_HEIGHT]+
		       KS_GL_WINDOW_TITLE_BAR_SIZE+KS_GL_WINDOW_FRAME_SIZE+1));
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0,(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*2),
	     0,(p->viewport[KS_GL_VIEWPORT_HEIGHT]+
		KS_GL_WINDOW_TITLE_BAR_SIZE+KS_GL_WINDOW_FRAME_SIZE));
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  lighting = ks_gl_disable(GL_LIGHTING);
  blend = ks_gl_enable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(1.0);

  /* back */
  glColor4f(color[0],color[1],color[2],back_trans);
  glBegin(GL_QUADS);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,KS_GL_WINDOW_FRAME_SIZE);
  glEnd();

  /* title bar */
  glColor4f(color[0],color[1],color[2],frame_trans);
  glBegin(GL_QUADS);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_TITLE_BAR_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_TITLE_BAR_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glEnd();

  glColor4f(color[0]+edge,color[1]+edge,color[2]+edge,edge_trans); /* upper edge */
  glBegin(GL_LINES);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_TITLE_BAR_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	     KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_TITLE_BAR_SIZE);
  glEnd();

  glColor4f(color[0]-edge,color[1]-edge,color[2]-edge,edge_trans); /* lower edge */
  glBegin(GL_LINES);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glEnd();

  /* bottom */

  glColor4f(color[0],color[1],color[2],frame_trans);
  glBegin(GL_QUADS);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,0);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,0);
  glEnd();

  glColor4f(color[0]+edge,color[1]+edge,color[2]+edge,edge_trans); /* upper edge */
  glBegin(GL_LINES);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     KS_GL_WINDOW_FRAME_SIZE);
  glEnd();

  glColor4f(color[0]-edge,color[1]-edge,color[2]-edge,edge_trans); /* lower edge */
  glBegin(GL_LINES);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,0);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,0);
  glEnd();

  /* left */

  glColor4f(color[0],color[1],color[2],frame_trans);
  glBegin(GL_POLYGON);
  glVertex2i(0,KS_GL_WINDOW_FRAME_SIZE);
  draw_gl_window_frame_upper_left(p);
  draw_gl_window_frame_lower_left(p);
  glEnd();
  glColor4f(color[0]+edge,color[1]+edge,color[2]+edge,edge_trans); /* left edge */
  glBegin(GL_LINE_STRIP);
  glVertex2i(0,KS_GL_WINDOW_FRAME_SIZE);
  draw_gl_window_frame_upper_left(p);
  glEnd();
  glColor4f(color[0]-edge,color[1]-edge,color[2]-edge,edge_trans); /* right edge */
  glBegin(GL_LINES);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(KS_GL_WINDOW_FRAME_SIZE,p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glEnd();

  /* right */
  glColor4f(color[0],color[1],color[2],frame_trans);
  glBegin(GL_POLYGON);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*2,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_TITLE_BAR_SIZE);
  draw_gl_window_frame_lower_right(p);
  draw_gl_window_frame_upper_right(p);
  glEnd();
  glColor4f(color[0]+edge,color[1]+edge,color[2]+edge,edge_trans); /* left edge */
  glBegin(GL_LINES);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,KS_GL_WINDOW_FRAME_SIZE);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE);
  glEnd();
  glColor4f(color[0]-edge,color[1]-edge,color[2]-edge,edge_trans); /* right edge */
  glBegin(GL_LINE_STRIP);
  glVertex2i(p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE*2,
	     p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_TITLE_BAR_SIZE);
  draw_gl_window_frame_lower_right(p);
  glEnd();

  { /* close mark */
    int ix,iy;
    glColor4f(color[0]+edge,color[1]+edge,color[2]+edge,edge_trans);
    ix = p->viewport[KS_GL_VIEWPORT_WIDTH]+KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_CLOSE_X;
    iy = p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE+KS_GL_WINDOW_CLOSE_Y;
    glBegin(GL_LINES);
    glVertex2i(ix,iy);
    glVertex2i(ix+KS_GL_WINDOW_CLOSE_WIDTH,iy+KS_GL_WINDOW_CLOSE_HEIGHT);
    glVertex2i(ix,iy+KS_GL_WINDOW_CLOSE_HEIGHT);
    glVertex2i(ix+KS_GL_WINDOW_CLOSE_WIDTH,iy);
    glEnd();
  }

#ifdef GLUT
  glColor4f(gb->foreground_color[0],gb->foreground_color[1],gb->foreground_color[2],1.0);
  ks_use_glut_font(gb,KS_GL_FONT_HELVETICA_10);
  glRasterPos2i(KS_GL_WINDOW_FRAME_SIZE,
		p->viewport[KS_GL_VIEWPORT_HEIGHT]+KS_GL_WINDOW_FRAME_SIZE+5);
  glCallLists((int)strlen(p->name), GL_BYTE, p->name);
#else
  ks_assert(0);
#endif

  if(blend == GL_TRUE)
    glDisable(GL_BLEND);

  if(lighting == GL_TRUE){
    glDisable(GL_LIGHTING);
  }
}
static void ks_draw_gl_window(KS_GL_BASE *gb, KS_GL_WINDOW *p)
{
  for(; p != NULL; p = p->next){
    if(p->flags&KS_GL_WINDOW_SHOW){
      if(!(p->flags&KS_GL_WINDOW_NO_FRAME)) draw_gl_window_frame(gb,p);
      glViewport((GLint)p->viewport[KS_GL_VIEWPORT_X],
		 (GLint)p->viewport[KS_GL_VIEWPORT_Y],
		 (GLsizei)(p->viewport[KS_GL_VIEWPORT_WIDTH]+1),
		 (GLsizei)(p->viewport[KS_GL_VIEWPORT_HEIGHT]+1));
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0,p->viewport[KS_GL_VIEWPORT_WIDTH],0,p->viewport[KS_GL_VIEWPORT_HEIGHT],
	      gb->depth_2d_near,gb->depth_2d_far);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glColor4f(gb->foreground_color[0],gb->foreground_color[1],gb->foreground_color[2],1.0);
      (p->display)(gb,p->id,p->viewport[KS_GL_VIEWPORT_WIDTH],p->viewport[KS_GL_VIEWPORT_HEIGHT],
		   p->vp);
    }
  }
}
static BOOL ks_is_gl_window_range(int width, int height, int x, int y)
{
  if(x >= 0 && x <= width && y >= 0 && y <= height){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
  /*
  if(x >= win_viewport[KS_GL_VIEWPORT_X] && 
     x <= win_viewport[KS_GL_VIEWPORT_X]+win_viewport[KS_GL_VIEWPORT_WIDTH] &&
     viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (win_viewport[KS_GL_VIEWPORT_Y]) && 
     viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (win_viewport[KS_GL_VIEWPORT_Y]+
						 win_viewport[KS_GL_VIEWPORT_HEIGHT])){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
  */
}
unsigned int ks_add_gl_base_window(KS_GL_BASE *gb, unsigned int flags, char *name,
				   int pos_x, int pos_y, int width, int height, 
				   void (*display)(KS_GL_BASE*,int,int,int,void*),
				   void (*mouse)(KS_GL_BASE*,int,int,int,int,int,int,int,void*),
				   void (*motion)(KS_GL_BASE*,int,int,int,int,int,void*),
				   void (*passive)(KS_GL_BASE*,int,int,int,int,int,void*),
				   void *vp)
{
  gb->window = ks_addend_gl_window(gb->window,ks_new_gl_window(gb->window_id_max++,flags,name,
							       pos_x,pos_y,width,height,
							       display,mouse,motion,passive,vp));
  return gb->window_id_max-1;
}
void ks_del_gl_base_window(KS_GL_BASE *gb, int id)
{
  gb->window = ks_del_gl_window(gb->window,id);
}
BOOL ks_toggle_gl_base_window(KS_GL_BASE *gb, int id)
{
  KS_GL_WINDOW *win;
  win = ks_lookup_gl_window(gb->window,id);
  if(win != NULL){
    win->flags ^= KS_GL_WINDOW_SHOW;
    gb->window = ks_move_end_gl_window(gb->window,win->id);
  } else {
    ks_error("window %d is not found");
    return KS_FALSE;
  }
  return KS_TRUE;
}
BOOL ks_show_gl_base_window(KS_GL_BASE *gb, int id)
{
  KS_GL_WINDOW *win;
  win = ks_lookup_gl_window(gb->window,id);
  if(win != NULL){
    win->flags |= KS_GL_WINDOW_SHOW;
    gb->window = ks_move_end_gl_window(gb->window,win->id);
  } else {
    ks_error("window %d is not found");
    return KS_FALSE;
  }
  return KS_TRUE;
}
BOOL ks_hide_gl_base_window(KS_GL_BASE *gb, int id)
{
  KS_GL_WINDOW *win;
  win = ks_lookup_gl_window(gb->window,id);
  if(win != NULL){
    win->flags &= ~KS_GL_WINDOW_SHOW;
  } else {
    ks_error("window %d is not found");
    return KS_FALSE;
  }
  return KS_TRUE;
}
BOOL ks_toggle_gl_base_window_frame(KS_GL_BASE *gb, int id)
{
  KS_GL_WINDOW *win;
  win = ks_lookup_gl_window(gb->window,id);
  if(win != NULL){
    if(win->flags&KS_GL_WINDOW_NO_FRAME){
      win->flags &= ~KS_GL_WINDOW_NO_FRAME;
    } else {
      win->flags |= KS_GL_WINDOW_NO_FRAME;
    }
    gb->window = ks_move_end_gl_window(gb->window,win->id);
  } else {
    ks_error("window %d is not found");
    return KS_FALSE;
  }
  return KS_TRUE;
}
static double calc_vec_len_dv(double *v)
{
  return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}
void ks_hsv2rgb(double h, double s, double v, double *r, double *g ,double *b)
{
    int i0;
    double fl;
    double m, n;

    i0 = (int)floor( h / 60. );
    fl = ( h / 60. ) - i0;
    if( !(i0 % 2)) fl = 1 - fl;

    m = v * ( 1 - s );
    n = v * ( 1 - s * fl );
    switch( i0 ){
       case 0: *r = v; *g = n; *b = m; break;
       case 1: *r = n; *g = v; *b = m; break;
       case 2: *r = m; *g = v; *b = n; break;
       case 3: *r = m; *g = n; *b = v; break;
       case 4: *r = n; *g = m; *b = v; break;
       case 5: *r = v; *g = m; *b = n; break;
    }
}
/*
static KS_GL_FONT *new_gl_font(GLuint base, char *name, void* font_type)
{
  KS_GL_FONT *newp;

  if((newp = (KS_GL_FONT*)ks_malloc(sizeof(KS_GL_FONT),"new_gl_font")) == NULL){
    ks_error("new_gl_font: memory error");
    return NULL;
  }
  if((newp->name = ks_malloc_char_copy(name,"new_gl_font")) == NULL){
    ks_error("new_gl_font: memory error");
    return NULL;
  }
  newp->base = base;
  newp->font_type = font_type;

  return newp;
}
static KS_GL_FONT *add_gl_font(KS_GL_FONT *listp, KS_GL_FONT *newp)
{
  newp->next = listp;
  return newp;
}
static KS_GL_FONT *lookup_gl_font(KS_GL_FONT* listp, char *name)
{
  for(; listp != NULL; listp = listp->next)
    if(strncmp(name,listp->name,strlen(listp->name)) == 0)
      return listp;
  return NULL;
}
static void free_gl_font(KS_GL_FONT *listp)
{
  KS_GL_FONT *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->name);
    next = listp->next;
    ks_free(listp);
  }
}
*/
static KS_GL_KEY_FUNC *new_gl_key_func(KS_GL_BASE *gb, unsigned char key, int active_flags, 
					   int *state, int state_min, int state_max, int state_add,
					   void (*func)(KS_GL_BASE*,int,int,void*),void *vp, 
					   char *comment)
{
  KS_GL_KEY_FUNC *newp;

  if((newp = (KS_GL_KEY_FUNC*)ks_malloc(sizeof(KS_GL_KEY_FUNC),"KS_GL_KEY_FUNC")) 
     == NULL){
    ks_error_memory();
    return NULL;
  }
  newp->gb = gb;
  newp->key = key;
  newp->active_flags = active_flags;
  newp->state = state;
  newp->state_range[KS_RANGE_MIN] = state_min;
  newp->state_range[KS_RANGE_MAX] = state_max;
  newp->state_add = state_add;
  newp->func = func;
  newp->vp = vp;
  if((newp->comment = ks_malloc_char_copy(comment,"comment")) == NULL){
    return NULL;
  }
  newp->next = NULL;
  return newp;
}
static KS_GL_KEY_FUNC *__KS_USED__ add_gl_key_func(KS_GL_KEY_FUNC *listp, KS_GL_KEY_FUNC *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_GL_KEY_FUNC *addend_gl_key_func(KS_GL_KEY_FUNC *listp, KS_GL_KEY_FUNC *newp)
{
  KS_GL_KEY_FUNC *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
static void free_gl_key_func(KS_GL_KEY_FUNC *listp)
{
  KS_GL_KEY_FUNC *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->comment);
    next = listp->next;
    ks_free(listp);
  }
}
BOOL ks_set_gl_key_func(KS_GL_BASE *gb, unsigned char key, int active_flags, 
			void *state, int state_min, int state_max, int state_add,
			void (*func)(KS_GL_BASE*,int,int,void*), void *vp, char *comment)
{
  if((gb->key_func = addend_gl_key_func(gb->key_func,new_gl_key_func(gb,key,active_flags,
								     state,
								     state_min,state_max,
								     state_add,
								     func,vp,comment)))==NULL){
    return KS_FALSE;
  }
  return KS_TRUE;
}
BOOL ks_set_gl_special_key_func(KS_GL_BASE *gb, unsigned char key, int active_flags, 
				void *state, int state_min, int state_max, int state_add,
				void (*func)(KS_GL_BASE*,int,int,void*), void *vp, char *comment)
{
  if((gb->special_key_func = addend_gl_key_func(gb->special_key_func,
						new_gl_key_func(gb,key,active_flags,
								state,
								state_min,state_max,
								state_add,
								func,vp,comment)))==NULL){
    return KS_FALSE;
  }
  return KS_TRUE;
}
static int comp_gl_key_func(const KS_GL_KEY_FUNC_SORT_BUF *sb0, const KS_GL_KEY_FUNC_SORT_BUF *sb1)
{
  return sb0->value - sb1->value;
}
BOOL ks_print_gl_key_func(KS_GL_BASE *gb)
{
  int i;
  KS_GL_KEY_FUNC *p;
  KS_GL_KEY_FUNC_SORT_BUF *sb;
  int num;

  for(p = gb->key_func, num = 0; p != NULL; p = p->next,num++){
    /*    printf("%c %s\n",p->key,p->comment);*/
  }
  /*  printf("%d\n",num);*/
  if((sb = (KS_GL_KEY_FUNC_SORT_BUF*)ks_malloc(num*sizeof(KS_GL_KEY_FUNC_SORT_BUF),"sb")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(p = gb->key_func, num = 0; p != NULL; p = p->next,num++){
    sb[num].p = p;
    if(isupper(p->key)){
      sb[num].value = .5 + (float)p->key;
    } else if(islower(p->key)){
      sb[num].value = (float)toupper(p->key);
    } else {
      sb[num].value = (float)p->key;
    }
    /*
    printf("%2d %c %4d %f\n",num,p->key,p->key,sb[num].value);
    */
  }
  /*
  for(i = 0; i < num; i++){
    printf("%2d %c %4d %f\n",i,sb[i].p->key,sb[i].p->key,sb[i].value);
  }
  */
  qsort(sb,num,sizeof(KS_GL_KEY_FUNC_SORT_BUF),(int (*)(const void*,const void*))comp_gl_key_func);

  for(i = 0; i < num; i++){
    /*    printf("%2d %c %4d %f\n",i,sb[i].p->key,sb[i].p->key,sb[i].value);*/
    printf("%c %s\n",sb[i].p->key,sb[i].p->comment);
  }
  ks_free(sb);
  return KS_TRUE;
}
#ifdef GLUT
void ks_init_glut_font(KS_GL_BASE *gb)
{
  int i;
  int fi;
  GLuint base;
  void *font_type[KS_GL_FONT_NUM];
  int font_height[KS_GL_FONT_NUM];

  font_type[KS_GL_FONT_TIMES_ROMAN_24] = GLUT_BITMAP_TIMES_ROMAN_24;
  font_type[KS_GL_FONT_TIMES_ROMAN_10] = GLUT_BITMAP_TIMES_ROMAN_10;
  font_type[KS_GL_FONT_HELVETICA_18] = GLUT_BITMAP_HELVETICA_18;
  font_type[KS_GL_FONT_HELVETICA_12] = GLUT_BITMAP_HELVETICA_12;
  font_type[KS_GL_FONT_HELVETICA_10] = GLUT_BITMAP_HELVETICA_10;
  font_type[KS_GL_FONT_8x13] = GLUT_BITMAP_8_BY_13;
  font_type[KS_GL_FONT_9x15] = GLUT_BITMAP_9_BY_15;

  font_height[KS_GL_FONT_TIMES_ROMAN_24] = 24;
  font_height[KS_GL_FONT_TIMES_ROMAN_10] = 10;
  font_height[KS_GL_FONT_HELVETICA_18] = 18;
  font_height[KS_GL_FONT_HELVETICA_12] = 12;
  font_height[KS_GL_FONT_HELVETICA_10] = 10;
  font_height[KS_GL_FONT_8x13] = 13;
  font_height[KS_GL_FONT_9x15] = 15;

  for(fi = 0; fi < KS_GL_FONT_NUM; fi++){
    base = glGenLists(128);
    for(i = 0; i < 128; i++){
      glNewList(base+i, GL_COMPILE);
      glutBitmapCharacter(font_type[fi], i);
      glEndList();
    }
    gb->font[fi].id = fi;
    gb->font[fi].base = base;
    gb->font[fi].type = font_type[fi];
    gb->font[fi].height = font_height[fi];
  }
}
#endif
void ks_init_gl_rot_mat(double mat[16])
{
  int i;
  for(i = 0; i < 16; i++){
    if(i == 0 || i == 5 || i == 10 || i == 15){
      mat[i] = 1;
    } else {
      mat[i] = 0;
    }
  }
}
KS_GL_BASE *ks_allocate_gl_base(void)
{
  int i,j,k;
  KS_GL_BASE *gb;

  if((gb = (KS_GL_BASE*)ks_malloc(sizeof(KS_GL_BASE),"ks_allocate_gl_base")) == NULL){
    ks_error("ks_allocate_gl_base: momory error\n");
    return NULL;
  }

  gb->active_flags = 0;
  if(KS_GL_ACTIVE_SHIFT == KS_GL_ACTIVE_NO_GET){
    ks_error("Waring!! The values of KS_GL_ACTIVE_SHIFT and KS_GL_ACTIVE_NO_GET are the same.\n");
  }
  if(KS_GL_ACTIVE_CTRL == KS_GL_ACTIVE_NO_GET){
    ks_error("Waring!! The values of KS_GL_ACTIVE_CTRL and KS_GL_ACTIVE_NO_GET are the same.\n");
  }
  if(KS_GL_ACTIVE_ALT == KS_GL_ACTIVE_NO_GET){
    ks_error("Waring!! The values of KS_GL_ACTIVE_ALT and KS_GL_ACTIVE_NO_GET are the same.\n");
  }

  gb->stereo = 0;
  gb->eye_flg = 1;
  gb->eye_width = .7;
  gb->eye_len = 20;
  gb->eye_ang = 30.;

  for(i = 0; i < KS_GL_MOUSE_BUTTON_NUM; i++)
    gb->mouse_state[i] = KS_GL_MOUSE_STATE_UP;

  for(i = 0; i < 3; i++){
    gb->trans[i] = 0.0;
    gb->angle[i] = 0.0;
  }

  gb->foreground_color[0] = 1.0;
  gb->foreground_color[1] = 1.0;
  gb->foreground_color[2] = 1.0;
  gb->foreground_color[3] = 1.0;

  gb->background_color[0] = 0.0;
  gb->background_color[1] = 0.0;
  gb->background_color[2] = 0.0;
  gb->background_color[3] = 0.0;

  /*
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetDoublev(GL_MODELVIEW_MATRIX,gb->rot_mat);
  glGetDoublev(GL_MODELVIEW_MATRIX,gb->rot_imat);
  */

  ks_init_gl_rot_mat(gb->rot_mat);
  ks_init_gl_rot_mat(gb->rot_imat);
  /*
  printf("rot mat %f %f %f %f %f %f %f %f %f\n"
	 ,gb->rot_mat[0],gb->rot_mat[4],gb->rot_mat[8]
	 ,gb->rot_mat[1],gb->rot_mat[5],gb->rot_mat[9]
	 ,gb->rot_mat[2],gb->rot_mat[6],gb->rot_mat[10]);
  */
  gb->projection = 0;
  gb->verbose_level = 1;
  gb->verbose_level_max = 1;

  gb->key_func = NULL;
  gb->special_key_func = NULL;

  gb->input.mode = KS_FALSE;

  gb->rod_vnum = 3;
  gb->rod_v = NULL;
  gb->arrow_n = NULL;
  gb->tube = NULL;
  gb->ribbon_a = NULL;
  gb->ribbon_b = NULL;
  gb->hokan = NULL;
  for(i = 0; i < KS_GL_BASE_VERTEX_BUF_NUM; i++){
    gb->verbuf[i] = NULL;
    gb->ribbon_v[i] = NULL;
    gb->ribbon_n[i] = NULL;
  }

  gb->double_click_time = 0.3;
  for(i = 0; i < KS_GL_MOUSE_TIME_NUM; i++)
    for(j = 0; j < KS_GL_MOUSE_BUTTON_NUM; j++)
      for(k = 0; k < KS_GL_MOUSE_STATE_NUM; k++)
	gb->mouse_time[i][j][k] = 0;
  gb->mouse_angle_change = KS_TRUE;

  gb->hits = 0;
  gb->hit_name_num = 0;

  for(i = 0; i < 3; i++)
    gb->rotation_center[i] = 0;

  for(i = 0; i < 3; i++)
    gb->rotation_center_offset[i] = 0;

  gb->lookat_moving = KS_FALSE;
  for(i = 0; i < KS_GL_LOOKAT_NUM; i++)
    for(j = 0; j < 3; j++)
      gb->lookat[i][j] = 0;
  for(i = 0; i < 3; i++){
    gb->lookat_stack[i] = 0;
    gb->trans_stack[i] = 0;
  }

  gb->move_target = NULL;

  for(i = 0; i < KS_GL_FONT_NUM; i++){
    gb->font[i].id = -1;
  }

  gb->display_func = NULL;
  gb->window = NULL;
  gb->pick_window = NULL;
  gb->window_id_max = 0;

  gb->key_help_key = -1;
  gb->key_help_font = -1;
  gb->key_help_window_id = -1;
  gb->key_help_point_pos = -1;

  /*
  gb->viewport[KS_GL_VIEWPORT_X] = 0;
  gb->viewport[KS_GL_VIEWPORT_Y] = 0;
  gb->viewport[KS_GL_VIEWPORT_WIDTH] = 0;
  gb->viewport[KS_GL_VIEWPORT_HEIGHT] = 0;
  */

  gb->viewport_3d[KS_GL_VIEWPORT_X] = 0;
  gb->viewport_3d[KS_GL_VIEWPORT_Y] = 0;
  gb->viewport_3d[KS_GL_VIEWPORT_WIDTH] = 0;
  gb->viewport_3d[KS_GL_VIEWPORT_HEIGHT] = 0;
  gb->viewport_2d[KS_GL_VIEWPORT_X] = 0;
  gb->viewport_2d[KS_GL_VIEWPORT_Y] = 0;
  gb->viewport_2d[KS_GL_VIEWPORT_WIDTH] = 0;
  gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT] = 0;

  gb->depth_3d_near = 1.0;
  gb->depth_3d_far = 800.0;
  gb->depth_2d_near = -1.0;
  gb->depth_2d_far   = 1.0;

#ifdef ENABLE_DOME
  gb->dome_id = -1;
#endif
  return gb;
}
void ks_free_gl_key_func(KS_GL_BASE *gb)
{
  free_gl_key_func(gb->key_func);
  free_gl_key_func(gb->special_key_func);
  gb->key_func = NULL;
  gb->special_key_func = NULL;
}
void ks_free_gl_base(KS_GL_BASE *gb)
{
  int i,j;

  ks_free_gl_key_func(gb);
  if(gb->rod_v != NULL){
    for(i = 0; i < gb->rod_vnum; i++)
      for(j = 0; j < gb->rod_vnum*4; j++)
	ks_free(gb->rod_v[i][j]);
    for(i = 0; i < gb->rod_vnum; i++)
      ks_free(gb->rod_v[i]);
  }
  ks_free(gb->rod_v);
  if(gb->arrow_n != NULL){
    for(i = 0; i < gb->rod_vnum*4; i++)
      ks_free(gb->arrow_n[i]);
    ks_free(gb->arrow_n);
  }
  for(i = 0; i < KS_GL_BASE_VERTEX_BUF_NUM; i++){
    if(gb->verbuf[i] != NULL){
      ks_free_double_pp(gb->tube->num,gb->verbuf[i]);
    }
  }
  if(gb->tube != NULL)
    ks_free_circle_buffer(gb->tube);
  if(gb->ribbon_a != NULL){
    ks_free_gl_buffer_3d(gb->ribbon_a);
  }
  if(gb->ribbon_b != NULL){
    ks_free_gl_buffer_3d(gb->ribbon_b);
  }
  if(gb->hokan != NULL){
    ks_free_gl_buffer_3d(gb->hokan);
  }

  for(i = 0; i < KS_GL_BASE_VERTEX_BUF_NUM; i++){
    if(gb->ribbon_v[i] != NULL){
      ks_free_double_pp(KS_GL_RIBBON_VERTEX_NUM,gb->ribbon_v[i]);
    }
  }
  for(i = 0; i < KS_GL_BASE_VERTEX_BUF_NUM; i++){
    if(gb->ribbon_n[i] != NULL){
      ks_free_double_pp(KS_GL_RIBBON_VERTEX_NUM,gb->ribbon_n[i]);
    }
  }

  gb->move_target = NULL;
  gb->move_target_motion = KS_FALSE;
  gb->move_target_motion_scale = 1.0;

  if(gb->window != NULL)
    ks_free_gl_window(gb->window);

  ks_free(gb);
}
/*
KS_GL_FONT *ks_lookup_gl_font(KS_GL_BASE *gb, char *name)
{
  return lookup_gl_font(gb->font,name);
}
*/
#ifdef GLUT
#if 0
BOOL ks_make_glut_font(KS_GL_BASE *gb, void *font_type, char *font_name)
{
  int i;
  GLuint base;
  /*
  {
    KS_GL_FONT *p;
    for(p = gl_font; p != NULL; p = p->next){
      printf("'%s'\n",p->name);
    }
  }
  */
  if(lookup_gl_font(gb->font,font_name) != NULL){
    ks_error("font name '%s' is already defined",font_name);
    return KS_FALSE;
  }
  base = glGenLists(128);
  for(i = 0; i < 128; i++){
    glNewList(base+i, GL_COMPILE);
    glutBitmapCharacter(font_type, i);
    glEndList();
  }

  if((gb->font = add_gl_font(gb->font,new_gl_font(base,font_name,font_type))) != NULL){
    return KS_TRUE;
  } else {
    return KS_FALSE;
  }
}
#endif
BOOL ks_use_glut_font(KS_GL_BASE *gb, int font)
{
  if(font >= 0 && font < KS_GL_FONT_NUM){
    if(gb->font[font].id == -1){
      ks_error("fonts are not initialized");
      return KS_FALSE;
    }
    glListBase(gb->font[font].base);
    return KS_TRUE;
  } else {
    ks_error("font id %d is not found",font);
    return KS_FALSE;
  }
}
BOOL ks_get_glut_font_width(KS_GL_BASE *gb, int font, char *str, int *len)
{
  int i;
  if(font >= 0 && font < KS_GL_FONT_NUM){
    *len = 0;
    for(i = 0; str[i]; i++){
      *len += glutBitmapWidth(gb->font[font].type,str[i]);
    }
    return KS_TRUE;
  } else {
    ks_error("font id %d is not found",font);
    return KS_FALSE;
  }
}
BOOL ks_get_glut_font_height(KS_GL_BASE *gb, int font, int *len)
{
  if(font >= 0 && font < KS_GL_FONT_NUM){
    *len = gb->font[font].height;
    return KS_TRUE;
  } else {
    ks_error("font id %d is not found",font);
    return KS_FALSE;
  }
}
void ks_draw_glut_sphere(double x0, double y0, double z0,
			 double cr, double cg, double cb, double ct, double r, int n0, int n1)
{
  GLfloat color[4];

  color[0] = cr; color[1] = cg; color[2] = cb; color[3] = ct;
  glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color);
  glPushMatrix();
  glTranslated(x0,y0,z0);
  glutSolidSphere(r,n0,n1);
  glPopMatrix();
}
#endif
void ks_draw_gl_line(double x0, double y0, double z0, double x1, double y1, double z1,
		     double cr, double cg, double cb, double width)
{
  glLineWidth(width);
  glColor4d(cr,cg,cb,1.0);
  glBegin(GL_LINES);
  glVertex3d(x0,y0,z0);
  glVertex3d(x1,y1,z1);
  glEnd();
}
void ks_draw_gl_box_line(double x0, double y0, double z0, double x1, double y1, double z1)
{
  glBegin(GL_LINE_LOOP);
  glVertex3d(x0,y0,z0);
  glVertex3d(x1,y0,z0);
  glVertex3d(x1,y1,z0);
  glVertex3d(x0,y1,z0);
  glEnd();

  glBegin(GL_LINE_LOOP);
  glVertex3d(x0,y0,z1);
  glVertex3d(x1,y0,z1);
  glVertex3d(x1,y1,z1);
  glVertex3d(x0,y1,z1);
  glEnd();

  glBegin(GL_LINES);
  glVertex3d(x0,y0,z0);
  glVertex3d(x0,y0,z1);
  glVertex3d(x1,y0,z0);
  glVertex3d(x1,y0,z1);
  glVertex3d(x0,y1,z0);
  glVertex3d(x0,y1,z1);
  glVertex3d(x1,y1,z0);
  glVertex3d(x1,y1,z1);
  glEnd();
}
void ks_draw_gl_box(double x0, double y0, double z0, double x1, double y1, double z1, int type)
{
  if(type == KS_DRAW_GL_BOX_FRONT || type == KS_DRAW_GL_BOX_FRONT_AND_BACK){
    glBegin(GL_QUADS);
    glNormal3d(0,0,-1);
    glVertex3d(x0,y0,z0);
    glVertex3d(x0,y1,z0);
    glVertex3d(x1,y1,z0);
    glVertex3d(x1,y0,z0);
    glNormal3d(0,0,1);
    glVertex3d(x0,y0,z1);
    glVertex3d(x1,y0,z1);
    glVertex3d(x1,y1,z1);
    glVertex3d(x0,y1,z1);
    glNormal3d(-1,0,0);
    glVertex3d(x0,y0,z0);
    glVertex3d(x0,y0,z1);
    glVertex3d(x0,y1,z1);
    glVertex3d(x0,y1,z0);
    glNormal3d(1,0,0);
    glVertex3d(x1,y0,z0);
    glVertex3d(x1,y1,z0);
    glVertex3d(x1,y1,z1);
    glVertex3d(x1,y0,z1);
    glNormal3d(0,-1,0);
    glVertex3d(x0,y0,z0);
    glVertex3d(x1,y0,z0);
    glVertex3d(x1,y0,z1);
    glVertex3d(x0,y0,z1);
    glNormal3d(0,1,0);
    glVertex3d(x0,y1,z0);
    glVertex3d(x0,y1,z1);
    glVertex3d(x1,y1,z1);
    glVertex3d(x1,y1,z0);
    glEnd();
  }
  if(type == KS_DRAW_GL_BOX_BACK || type == KS_DRAW_GL_BOX_FRONT_AND_BACK){
    glBegin(GL_QUADS);
    glNormal3d(0,0,1);
    glVertex3d(x0,y0,z0);
    glVertex3d(x1,y0,z0);
    glVertex3d(x1,y1,z0);
    glVertex3d(x0,y1,z0);
    glNormal3d(0,0,-1);
    glVertex3d(x0,y0,z1);
    glVertex3d(x0,y1,z1);
    glVertex3d(x1,y1,z1);
    glVertex3d(x1,y0,z1);
    glNormal3d(1,0,0);
    glVertex3d(x0,y0,z0);
    glVertex3d(x0,y1,z0);
    glVertex3d(x0,y1,z1);
    glVertex3d(x0,y0,z1);
    glNormal3d(-1,0,0);
    glVertex3d(x1,y0,z0);
    glVertex3d(x1,y0,z1);
    glVertex3d(x1,y1,z1);
    glVertex3d(x1,y1,z0);
    glNormal3d(0,1,0);
    glVertex3d(x0,y0,z0);
    glVertex3d(x0,y0,z1);
    glVertex3d(x1,y0,z1);
    glVertex3d(x1,y0,z0);
    glNormal3d(0,-1,0);
    glVertex3d(x0,y1,z0);
    glVertex3d(x1,y1,z0);
    glVertex3d(x1,y1,z1);
    glVertex3d(x0,y1,z1);
    glEnd();
  }
}
static void __KS_USED__ swap2(short *sbuf, char *cbuf)
{
  int i;
  for(i = 0; i < 2; i++){
    cbuf[1-i] = ((char*)sbuf)[i];
  }
}
static void swap2_u(unsigned short *sbuf, char *cbuf)
{
  int i;
  for(i = 0; i < 2; i++){
    cbuf[1-i] = ((char*)sbuf)[i];
  }
}
static void swap4(long *sbuf, char *cbuf)
{
  int i;
  for(i = 0; i < 4; i++){
    cbuf[3-i] = ((char*)sbuf)[i];
  }
}
static void swap4_u(unsigned long *sbuf, char *cbuf)
{
  int i;
  for(i = 0; i < 4; i++){
    cbuf[3-i] = ((char*)sbuf)[i];
  }
}
int ks_save_gl_bmp(char *file_name, BOOL swap_endian, int *viewport)
{
  int i,j;
  int x_pixel,y_pixel;
  int x_len; /*,x_add;*/
  char cbuf[4];
  char char_buf;
  char out_name[256];
  FILE *fp;

  GLubyte *pix;
  int bmp_info_size;

  struct BITMAPFILEHEADER {
    char                bfType[2];
    unsigned long       bfSize;
    unsigned short      bfReserved1;
    unsigned short      bfReserved2;
    unsigned long       bfOffBits;
  } bmp_header;

  struct BITMAPINFOHEADER {
    unsigned long       biSize;
    long                biWidth;
    long                biHeight;
    unsigned short      biPlanes;
    unsigned short      biBitCount;
    unsigned long       biCompression;
    unsigned long       biSizeImage;
    long                biXPixPerMeter;
    long                biYPixPerMeter;
    unsigned long       biClrUsed;
    unsigned long       biClrImporant;
  } bmp_info;

  /*
  GLint viewport[4];
  glGetIntegerv (GL_VIEWPORT, viewport);
  */
  /*  printf("%d %d %d %d\n",viewport[0],viewport[1],viewport[2],viewport[3]);*/
  x_pixel = viewport[2];
  y_pixel = viewport[3];
  /*
    x_pixel = glutGet(GLUT_WINDOW_WIDTH);
    y_pixel = glutGet(GLUT_WINDOW_HEIGHT);
  */
  /*  printf("%d %d\n",x_pixel,y_pixel);*/

  if((x_pixel*3) % 4 != 0){
    x_len = (x_pixel*3/4 + 1)*4;
    /* x_add = x_len - x_pixel*3; */
  } else {
    x_len = x_pixel*3;
    /* x_add = 0; */
  }

  if((pix=(GLubyte*)ks_malloc((x_pixel+3)*(y_pixel)*3*sizeof(GLubyte),"save_gl_bmp"))==NULL){
    ks_error("memory error in ks_save_gl_bmp");
    return 0;
  }
  glReadPixels(0, 0, x_pixel, y_pixel, GL_RGB, GL_UNSIGNED_BYTE, pix);

  for(i = 0; i < y_pixel; i++){
    for(j = 0; j < x_pixel*3; j += 3){
      char_buf = pix[i*x_len+j];
      pix[i*x_len+j] = pix[i*x_len+j+2];
      pix[i*x_len+j+2] = char_buf;
    }
  }
  /*
    printf("%d %d %d %d\n",x_pixel,y_pixel,x_len,x_add);
*/
  bmp_info_size = 40;  // because sizeof(bmp_info) is depend on environment

  bmp_header.bfType[0] = 'B';
  bmp_header.bfType[1] = 'M';
  bmp_header.bfSize = 14+bmp_info_size+sizeof(GLubyte)*x_len*y_pixel;

  bmp_header.bfReserved1 = 0;
  bmp_header.bfReserved2 = 0;
  bmp_header.bfOffBits = 14+bmp_info_size;

  bmp_info.biSize = bmp_info_size;
  bmp_info.biWidth = x_pixel;
  bmp_info.biHeight = y_pixel;
  bmp_info.biPlanes = 1;
  bmp_info.biBitCount = 24;
  bmp_info.biCompression = 0;
  bmp_info.biSizeImage = sizeof(GLubyte)*x_len*y_pixel;
  bmp_info.biXPixPerMeter = 0;
  bmp_info.biYPixPerMeter = 0;
  bmp_info.biClrUsed = 0;
  bmp_info.biClrImporant = 0;

  /*  printf("file_name %s\n",file_name);*/
  if(strstr(file_name,".bmp") != NULL || strstr(file_name,".BMP") != NULL){
    ks_strncpy(out_name,file_name,sizeof(out_name));
  } else {
    if(strstr(file_name,".") != NULL){
      for(i = 0; file_name[i]; i++);
      for(;file_name[i] != '.'; i--);
      for(j = 0; j < i; j++){
	out_name[j]=file_name[j];
      }
      out_name[j] = '\0';
      strcat(out_name,".bmp");
    } else {
      strcpy(out_name, file_name);
      strcat(out_name,".bmp");
    }
  }

  /*  printf("save %s\n",out_name);*/
  if((fp = fopen(out_name,"wb")) == NULL){
    ks_error("file open error in ks_save_gl_bmp");
    return 0;
  }
  if(swap_endian == KS_FALSE){
#if 0 // because sizeof(bmp_info) is depend on environment
    fwrite(bmp_header.bfType, sizeof(char),2,fp);
    fwrite(&bmp_header.bfSize, sizeof(bmp_header.bfSize),1,fp);
    fwrite(&bmp_header.bfReserved1, sizeof(bmp_header.bfReserved1),1,fp);
    fwrite(&bmp_header.bfReserved2, sizeof(bmp_header.bfReserved2),1,fp);
    fwrite(&bmp_header.bfOffBits, sizeof(bmp_header.bfOffBits),1,fp);

    fwrite(&bmp_info, sizeof(bmp_info),1,fp);
#else
    fwrite(bmp_header.bfType,2,1,fp);
    fwrite(&bmp_header.bfSize,4,1,fp);
    fwrite(&bmp_header.bfReserved1,2,1,fp);
    fwrite(&bmp_header.bfReserved2,2,1,fp);
    fwrite(&bmp_header.bfOffBits,4,1,fp);

    fwrite(&bmp_info.biSize,4,1,fp);
    fwrite(&bmp_info.biWidth,4,1,fp);
    fwrite(&bmp_info.biHeight,4,1,fp);
    fwrite(&bmp_info.biPlanes,2,1,fp);
    fwrite(&bmp_info.biBitCount,2,1,fp);
    fwrite(&bmp_info.biCompression,4,1,fp);
    fwrite(&bmp_info.biSizeImage,4,1,fp);
    fwrite(&bmp_info.biXPixPerMeter,4,1,fp);
    fwrite(&bmp_info.biYPixPerMeter,4,1,fp);
    fwrite(&bmp_info.biClrUsed,4,1,fp);
    fwrite(&bmp_info.biClrImporant,4,1,fp);
#endif
  } else {
    fwrite(bmp_header.bfType, sizeof(char),2,fp);
    swap4_u(&bmp_header.bfSize,cbuf);
    fwrite((long*)cbuf, sizeof(bmp_header.bfSize),1,fp);
    swap2_u(&bmp_header.bfReserved1,cbuf);
    fwrite((short*)cbuf, sizeof(bmp_header.bfReserved1),1,fp);
    swap2_u(&bmp_header.bfReserved2, cbuf);
    fwrite((short*)cbuf, sizeof(bmp_header.bfReserved2),1,fp);
    swap4_u(&bmp_header.bfOffBits, cbuf);
    fwrite((long*)cbuf, sizeof(bmp_header.bfOffBits),1,fp);

    swap4_u(&bmp_info.biSize, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4(&bmp_info.biWidth, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4(&bmp_info.biHeight, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap2_u(&bmp_info.biPlanes, cbuf);
    fwrite((short*)cbuf, sizeof(short),1,fp);
    swap2_u(&bmp_info.biBitCount, cbuf);
    fwrite((short*)cbuf, sizeof(short),1,fp);
    swap4_u(&bmp_info.biCompression, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4_u(&bmp_info.biSizeImage, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4(&bmp_info.biXPixPerMeter, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4(&bmp_info.biYPixPerMeter, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4_u(&bmp_info.biClrUsed, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
    swap4_u(&bmp_info.biClrImporant, cbuf);
    fwrite((long*)cbuf, sizeof(long),1,fp);
  }
  fwrite(pix,sizeof(GLubyte),x_len*y_pixel,fp);

  fclose(fp);
  ks_free(pix);

  return 1;
}
/*
void ks_multi_gl_mat(KS_GL_BASE gb, double *v, double *m)
{
  m[0] = gb.rot_mat[0]*v[0]+gb.rot_mat[4]*v[1]+gb.rot_mat[8]*v[2];
  m[1] = gb.rot_mat[1]*v[0]+gb.rot_mat[5]*v[1]+gb.rot_mat[9]*v[2];
  m[2] = gb.rot_mat[2]*v[0]+gb.rot_mat[6]*v[1]+gb.rot_mat[10]*v[2];
}
*/
void ks_multi_gl_matrix(double mat[16], double *v, double *m)
{
  m[0] = mat[0]*v[0]+mat[4]*v[1]+mat[8]*v[2] + mat[12];
  m[1] = mat[1]*v[0]+mat[5]*v[1]+mat[9]*v[2] + mat[13];
  m[2] = mat[2]*v[0]+mat[6]*v[1]+mat[10]*v[2]+ mat[14];
  /*  printf("%f %f %f\n",m[0],m[1],m[2]);*/
}
void ks_multi_gl_rotational_matrix(KS_GL_BASE gb, double *v, double *m)
{
  m[0] = gb.rot_mat[0]*v[0]+gb.rot_mat[4]*v[1]+gb.rot_mat[8]*v[2];
  m[1] = gb.rot_mat[1]*v[0]+gb.rot_mat[5]*v[1]+gb.rot_mat[9]*v[2];
  m[2] = gb.rot_mat[2]*v[0]+gb.rot_mat[6]*v[1]+gb.rot_mat[10]*v[2];
  /*  printf("%f %f %f\n",m[0],m[1],m[2]);*/
}
void ks_multi_gl_inverse_rotational_matrix(KS_GL_BASE gb, double *v, double *m)
{
  m[0] = gb.rot_imat[0]*v[0]+gb.rot_imat[4]*v[1]+gb.rot_imat[8]*v[2];
  m[1] = gb.rot_imat[1]*v[0]+gb.rot_imat[5]*v[1]+gb.rot_imat[9]*v[2];
  m[2] = gb.rot_imat[2]*v[0]+gb.rot_imat[6]*v[1]+gb.rot_imat[10]*v[2];
}
void ks_apply_gl_initial_matrix(KS_GL_BASE *gb)
{
  glMultMatrixd(gb->init_mat);
}
void ks_apply_gl_rotational_matrix(KS_GL_BASE *gb)
{
  glMultMatrixd(gb->rot_mat);
}
BOOL ks_draw_gl_ringo(double r, int flg)
{
  int i,j,k;
  double d0,d1,d2,d3,d4;
  GLfloat color[4];
  double rz;
  double phi;
  double v[2][4];
  static double ringo[21][2]= {
    {0.000000, 0.610000},
    {0.076434, 0.677688},
    {0.199017, 0.761057},
    {0.343990, 0.811007},
    {0.507785, 0.829017},
    {0.657107, 0.807107},
    {0.799017, 0.747785},
    {0.911007, 0.663990},
    {1.001057, 0.539017},
    {1.037688, 0.366434},
    {1.010000, 0.100000},
    {0.947688,-0.186434},
    {0.871057,-0.439017},
    {0.781007,-0.643990},
    {0.699017,-0.787785},
    {0.597107,-0.887107},
    {0.477785,-0.929017},
    {0.353990,-0.931007},
    {0.229017,-0.901057},
    {0.106434,-0.827688},
    {0.000000,-0.760000}};

  int r_num1 = 20+1;
  int r_num2 = 20+1;

  double ***rc = NULL;
  double ***rn = NULL;

  /*
  double rc[r_num1][r_num2][3];
  double rn[r_num1][r_num2][3];
  */

  phi = 2.0*M_PI/(r_num2-1);

  
  if((rc = ks_malloc_double_ppp(r_num1,r_num2,3,"ks_draw_gl_ringo")) == NULL){
    ks_error("ks_draw_gl_ringo: memory error\n");
    return KS_FALSE;
  }
  if((rn = ks_malloc_double_ppp(r_num1,r_num2,3,"ks_draw_gl_ringo")) == NULL){
    ks_error("ks_draw_gl_ringo: memory error\n");
    return KS_FALSE;
  }
  /*
  if((rc = (double***)ks_malloc(r_num1 *sizeof(double**),"ks_draw_gl_ringo")) == NULL){
    ks_error("ks_draw_gl_ringo: memory error\n");
    return KS_FALSE;
  }
  for(i = 0; i < r_num1; i++){
    if((rc[i] = (double**)ks_malloc(r_num2 *sizeof(double*),"ks_draw_gl_ringo")) == NULL){
      ks_error("ks_draw_gl_ringo: memory error\n");
      return KS_FALSE;
    }
  }
  for(i = 0; i < r_num1; i++)
    for(j = 0; j < r_num2; j++)
      if((rc[i][j] = (double*)ks_malloc(3 *sizeof(double),"ks_draw_gl_ringo")) == NULL){
	ks_error("ks_draw_gl_ringo: memory error\n");
	return KS_FALSE;
      }
  if((rn = (double***)ks_malloc(r_num1 *sizeof(double**),"ks_draw_gl_ringo")) == NULL){
    ks_error("ks_draw_gl_ringo: memory error\n");
    return KS_FALSE;
  }
  for(i = 0; i < r_num1; i++){
    if((rn[i] = (double**)ks_malloc(r_num2 *sizeof(double*),"ks_draw_gl_ringo")) == NULL){
      ks_error("ks_draw_gl_ringo: memory error\n");
      return KS_FALSE;
    }
  }
  for(i = 0; i < r_num1; i++)
    for(j = 0; j < r_num2; j++)
      if((rn[i][j] = (double*)ks_malloc(3 *sizeof(double),"ks_draw_gl_ringo")) == NULL){
	ks_error("ks_draw_gl_ringo: memory error\n");
	return KS_FALSE;
      }
  */
  for(i = 0; i < r_num1; i++){
    rz = ringo[i][0];
    for(j = 0; j < r_num2; j++){
      rc[i][j][0] = rz*cos(phi*j);
      rc[i][j][1] = rz*sin(phi*j);
      rc[i][j][2] = ringo[i][1];

    }
  }

    for(i = 0; i < r_num1-2; i++){
      for(j = 0; j < r_num2; j++){
	for(k = 0; k < 3; k++){
	  v[0][k] = rc[i+1][j][k] - rc[i][j][k];
	  v[1][k] = rc[i+2][j][k] - rc[i+1][j][k];
	}
	v[0][3] = calc_vec_len_dv(v[0]);
	v[1][3] = calc_vec_len_dv(v[1]);
	d0 = - v[0][3]*v[0][3]/(v[0][0]*v[1][0]+v[0][1]*v[1][1]+v[0][2]*v[1][2]);
	d1 = - v[1][3]*v[1][3]/(v[0][0]*v[1][0]+v[0][1]*v[1][1]+v[0][2]*v[1][2]);
	for(k = 0; k < 3; k++){
	  rn[i+1][j][k] = v[0][k] + d0*v[1][k] - v[1][k] - d1*v[0][k];
	}
	d0 = calc_vec_len_dv(rn[i+1][j]);
	for(k = 0; k < 3; k++)
	  rn[i+1][j][k] /= d0;
      }
    }


  /*
  glPolygonMode(GL_FRONT, GL_LINE);
  */
  color[0] = 1.0; color[1] = 1.0; color[2] = 1.0; color[3] = 1.0;
  glMaterialfv(GL_FRONT, GL_SPECULAR,color);
  glMaterialf(GL_FRONT, GL_SHININESS, 70.0);

  d0 = 0.4;
  d1 = 0.5;
  d2 = 0.8;
  switch(flg){
  case KS_GL_RINGO_TYPE_AKA:
    color[0] = 1.0; color[2] = 0.0; color[3] = 1.0;
    for(i = 0; i < r_num1-1; i++){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < r_num2; j++){
	if(d0 > (double)i/(r_num1-2))
	  color[1] = d1 - d1/d0*(double)i/(r_num1-2);
	else
	  color[1] =  1./(1-d0)*(double)i/(r_num1-2) - d0/(1-d0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
	glColor4fv(color);
	glNormal3d(rn[i][j][0],  rn[i][j][1],  rn[i][j][2]);
	glVertex3d(r*rc[i][j][0],  r*rc[i][j][1],  r*rc[i][j][2]);
	if(d0 > (double)(i+1)/(r_num1-2))
	  color[1] = d1 - d1/d0*(double)(i+1)/(r_num1-2);
	else
	  color[1] =  1./(1-d0)*(double)(i+1)/(r_num1-2) - d0/(1-d0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
	glColor4fv(color);
	glNormal3d(rn[i+1][j][0],rn[i+1][j][1],rn[i+1][j][2]);
	glVertex3d(r*rc[i+1][j][0],r*rc[i+1][j][1],r*rc[i+1][j][2]);
      }
      glEnd();
    }
    break;
  case KS_GL_RINGO_TYPE_AO:
    color[1] = 1.0; color[2] = 0.0; color[3] = 1.0;
    for(i = 0; i < r_num1-1; i++){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < r_num2; j++){
	if(d0 > (double)i/(r_num1-2))
	  color[0] = (d1 + d1/d0*(double)i/(r_num1-2))*d2;
	else
	  color[0] =  (-1./(1-d0)*(double)i/(r_num1-2) + 1./(1-d0))*d2;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
	glColor4fv(color);
	glNormal3d(rn[i][j][0],  rn[i][j][1],  rn[i][j][2]);
	glVertex3d(r*rc[i][j][0],  r*rc[i][j][1],  r*rc[i][j][2]);
	if(d0 > (double)(i+1)/(r_num1-2))
	  color[0] = (d1 + d1/d0*(double)(i+1)/(r_num1-2))*d2;
	else
	  color[0] = (-1./(1-d0)*(double)(i+1)/(r_num1-2) + 1./(1-d0))*d2;
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
	glColor4fv(color);
	glNormal3d(rn[i+1][j][0],rn[i+1][j][1],rn[i+1][j][2]);
	glVertex3d(r*rc[i+1][j][0],r*rc[i+1][j][1],r*rc[i+1][j][2]);
      }
      glEnd();
    }
    break;
  case KS_GL_RINGO_TYPE_BRIGHT:
    color[0] = 1.0; color[1] = 1.0; color[2] = 1.0; color[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
    glColor4fv(color);
    color[0] = 5.0; color[1] = 5.0; color[2] = 5.0; color[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT,color);
    glColor4fv(color);
    for(i = 0; i < r_num1-1; i++){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < r_num2; j++){
	glNormal3d(rn[i][j][0],  rn[i][j][1],  rn[i][j][2]);
	glVertex3d(r*rc[i][j][0],  r*rc[i][j][1],  r*rc[i][j][2]);
	glNormal3d(rn[i+1][j][0],rn[i+1][j][1],rn[i+1][j][2]);
	glVertex3d(r*rc[i+1][j][0],r*rc[i+1][j][1],r*rc[i+1][j][2]);
      }
      glEnd();
    }
    break;
    /*
    r1 = r*1.02;
    glDisable(GL_LIGHTING);
    glEnable (GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_POLYGON);
    glColor4f(1.0, 1.0, 1.0, 1.0);
    for(i = 0; i < c_num*3; i += 3){
      glVertex3d(0.0, r*cc[i],r*cc[i+1]);
    }
    glEnd();
    d0 = 1.0;
    d1 = 0.6;
    glBegin(GL_QUAD_STRIP);
    for(i = 0; i < c_num*3; i += 3){
      glColor4f(1.0, 1.0, 1.0, d0);
      glVertex3d(0.0, r*cc[i],r*cc[i+1]);
      glColor4f(1.0, 1.0, 1.0, d1);
      glVertex3d(0.0, r1*cc[i],r1*cc[i+1]);
    }
    glColor4f(1.0, 1.0, 1.0, d0);
    glVertex3d(0.0, r*cc[0],r*cc[1]);
    glColor4f(1.0, 1.0, 1.0, d1);
    glVertex3d(0.0, r1*cc[0],r1*cc[1]);
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for(i = 0; i < c_num*3; i += 3){
      glColor4f(1.0, 1.0, 1.0, d1);
      glVertex3d(0.0, r1*cc[i],r1*cc[i+1]);
      glColor4f(1.0, 1.0, 1.0, 0.0);
      glVertex3d(0.0, 2.0*r*cc[i],2.0*r*cc[i+1]);
    }
    glColor4f(1.0, 1.0, 1.0, d1);
    glVertex3d(0.0, r1*cc[0],r1*cc[1]);
    glColor4f(1.0, 1.0, 1.0, 0.0);
    glVertex3d(0.0, 2.0*r*cc[0],2.0*r*cc[1]);
    glEnd();
    glDisable (GL_BLEND);
    glEnable(GL_LIGHTING);
    break;
    */
  }

  color[0] = 0.0; color[1] = 0.0; color[2] = 0.0; color[3] = 1.0;
  glMaterialfv(GL_FRONT, GL_SPECULAR,color);
  glMaterialf(GL_FRONT, GL_SHININESS, 0.0);

  d0 = 0.03*r;
  d1 = 0.4*r+r*rc[0][0][2];
  d2 = 0.02*r;
  d3 = r*rc[0][0][2];
  d4 = 1./sqrt(2);

  switch(flg){
  case KS_GL_RINGO_TYPE_AKA:
  case KS_GL_RINGO_TYPE_AO:
    color[0] = 0.5; color[1] = 0.0; color[2] = 0.0; color[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
    break;
  case KS_GL_RINGO_TYPE_BRIGHT:
    color[0] = 1.0; color[1] = 1.0; color[2] = 1.0; color[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,color);
    color[0] = 5.0; color[1] = 5.0; color[2] = 5.0; color[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT,color);
    break;
  }

  glBegin(GL_QUADS);

  glNormal3d(0,0,1);
  glVertex3d(d0,d0,d1);
  glVertex3d(-d0,d0,d1);
  glVertex3d(-d0,-d0,d1);
  glVertex3d(d0,-d0,d1);

  glNormal3d(d4,d4,0);
  glVertex3d(d2,d2,d3);
  glVertex3d(d0,d0,d1);
  glNormal3d(d4,-d4,0);
  glVertex3d(d0,-d0,d1);
  glVertex3d(d2,-d2,d3);

  glNormal3d(-d4,-d4,0);
  glVertex3d(-d2,-d2,d3);
  glVertex3d(-d0,-d0,d1);
  glNormal3d(-d4,d4,0);
  glVertex3d(-d0,d0,d1);
  glVertex3d(-d2,d2,d3);

  glNormal3d(-d4,d4,0);
  glVertex3d(-d2,d2,d3);
  glVertex3d(-d0,d0,d1);
  glNormal3d(d4,d4,0);
  glVertex3d(d0,d0,d1);
  glVertex3d(d2,d2,d3);

  glNormal3d(d4,-d4,0);
  glVertex3d(d2,-d2,d3);
  glVertex3d(d0,-d0,d1);
  glNormal3d(-d4,-d4,0);
  glVertex3d(-d0,-d0,d1);
  glVertex3d(-d2,-d2,d3);

  glEnd();

  /*
  glDisable(GL_LIGHTING);
  glColor3f (0.0, 1.0, 0.0);
  for(i = 0; i < r_num1; i++){
    j = 0;
    {
      glBegin(GL_LINES);
      glVertex3d(r*rc[i][j][0],  r*rc[i][j][1],  r*rc[i][j][2]);
      glVertex3d(r*rc[i][j][0]+rn[i][j][0],  
		 r*rc[i][j][1]+rn[i][j][1],
		 r*rc[i][j][2]+rn[i][j][2]);
      glEnd();
    }
  }
  glEnable(GL_LIGHTING);
  */

  ks_free_double_ppp(r_num1,r_num2,rc);
  ks_free_double_ppp(r_num1,r_num2,rn);

  return KS_TRUE;
}
void ks_set_gl_rod_vertex_num(KS_GL_BASE *gb, int n)
{
  int i,j;
  if(n > 2){
    if(gb->rod_v != NULL){
      for(i = 0; i < gb->rod_vnum; i++)
	for(j = 0; j < gb->rod_vnum*3; j++)
	  ks_free(gb->rod_v[i][j]);
      for(i = 0; i < gb->rod_vnum; i++)
	ks_free(gb->rod_v[i]);
      ks_free(gb->rod_v);
    }
    gb->rod_vnum = n;
    gb->rod_v = NULL;
  }
}
void ks_draw_gl_rod(KS_GL_BASE *gb, GLfloat r, GLfloat len, int type, int line_flg)
{
  int i,j;
  float d0,d1,d2;

  if(gb->rod_v == NULL){
    if((gb->rod_v=(GLfloat***)ks_malloc(gb->rod_vnum *sizeof(GLfloat**),"ks_draw_gl_rod"))==NULL){
      ks_error("momory error in ks_draw_gl_rod");
      return;
    }
    for(i = 0; i < gb->rod_vnum; i++)
      if((gb->rod_v[i] = (GLfloat**)ks_malloc(gb->rod_vnum*4 *sizeof(GLfloat*),"ks_draw_gb->rod"))
	 == NULL){
	ks_error("momory error in ks_draw_gb->rod");
	return;
      }
    for(i = 0; i < gb->rod_vnum; i++)
      for(j = 0; j < gb->rod_vnum*4; j++)
	if((gb->rod_v[i][j] = (GLfloat*)ks_malloc(3 *sizeof(GLfloat),"ks_draw_gb->rod")) == NULL){
	  ks_error("momory error in ks_draw_gb->rod");
	  return;
	}
    d0 = 2.0*M_PI/gb->rod_vnum/4;
    d1 = 0.5*M_PI/gb->rod_vnum;
    /*    printf("%f %f\n",d0/M_PI*180,d1/M_PI*180);*/
    for(i = 0; i < gb->rod_vnum; i++){
      d2 = sin(d1*(i+1));
      for(j = 0; j < gb->rod_vnum*4; j++){
	gb->rod_v[i][j][0] = d2*cos(d0*j);
	gb->rod_v[i][j][1] = d2*sin(d0*j);
	gb->rod_v[i][j][2] = cos(d1*(i+1));
	/*
	printf("%d %d % f % f % f\n",i,j,gb->rod_v[i][j][0],gb->rod_v[i][j][1],gb->rod_v[i][j][2]);
	*/
      }
    }
  }

  if(line_flg >= 1){
    GLboolean lighting;
    if(line_flg == 2){
      lighting = ks_gl_disable(GL_LIGHTING);
    }
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, len);
    glVertex3f(0.0, 0.0, 0.0);
    glEnd();
    if(line_flg == 2){
      if(lighting == GL_TRUE)
	glDisable(GL_LIGHTING);
    }
  }

  if(type & KS_DRAW_GL_ROD_COVER_END){
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, r+len);
    for(j = 0; j < gb->rod_vnum*4; j++){
      glNormal3fv(gb->rod_v[0][j]);
      glVertex3f(r*gb->rod_v[0][j][0],r*gb->rod_v[0][j][1],r*gb->rod_v[0][j][2]+len);
    }
    glNormal3fv(gb->rod_v[0][0]);
    glVertex3f(r*gb->rod_v[0][0][0],r*gb->rod_v[0][0][1],r*gb->rod_v[0][0][2]+len);
    glEnd();
    for(i = 0; i < gb->rod_vnum-1; i++){
      glBegin(GL_TRIANGLE_STRIP);
      for(j = 0; j < gb->rod_vnum*4; j++){
	glNormal3fv(gb->rod_v[i][j]);
	glVertex3f(r*gb->rod_v[i][j][0],r*gb->rod_v[i][j][1],r*gb->rod_v[i][j][2]+len);
	glNormal3fv(gb->rod_v[i+1][j]);
	glVertex3f(r*gb->rod_v[i+1][j][0],r*gb->rod_v[i+1][j][1],r*gb->rod_v[i+1][j][2]+len);
      }
      glNormal3fv(gb->rod_v[i][0]);
      glVertex3f(r*gb->rod_v[i][0][0],r*gb->rod_v[i][0][1],r*gb->rod_v[i][0][2]+len);
      glNormal3fv(gb->rod_v[i+1][0]);
      glVertex3f(r*gb->rod_v[i+1][0][0],r*gb->rod_v[i+1][0][1],r*gb->rod_v[i+1][0][2]+len);
      glEnd();
    }

  }

  i = gb->rod_vnum-1;
  glBegin(GL_TRIANGLE_STRIP);
  for(j = 0; j < gb->rod_vnum*4; j++){
    glNormal3fv(gb->rod_v[i][j]);
    glVertex3f(r*gb->rod_v[i][j][0],r*gb->rod_v[i][j][1],r*gb->rod_v[i][j][2]+len);
    glNormal3fv(gb->rod_v[i][j]);
    glVertex3f(r*gb->rod_v[i][j][0],r*gb->rod_v[i][j][1],r*gb->rod_v[i][j][2]);
  }
  glNormal3fv(gb->rod_v[i][0]);
  glVertex3f(r*gb->rod_v[i][0][0],r*gb->rod_v[i][0][1],r*gb->rod_v[i][0][2]+len);
  glNormal3fv(gb->rod_v[i][0]);
  glVertex3f(r*gb->rod_v[i][0][0],r*gb->rod_v[i][0][1],r*gb->rod_v[i][0][2]);
  glEnd();

  if(type & KS_DRAW_GL_ROD_COVER_START){
    for(i = gb->rod_vnum-1; i > 0; i--){
      glBegin(GL_TRIANGLE_STRIP);
      for(j = 0; j < gb->rod_vnum*4; j++){
	glNormal3f(gb->rod_v[i][j][0],gb->rod_v[i][j][1],-gb->rod_v[i][j][2]);
	glVertex3f(r*gb->rod_v[i][j][0],r*gb->rod_v[i][j][1],-r*gb->rod_v[i][j][2]);
	glNormal3f(gb->rod_v[i-1][j][0],gb->rod_v[i-1][j][1],-gb->rod_v[i-1][j][2]);
	glVertex3f(r*gb->rod_v[i-1][j][0],r*gb->rod_v[i-1][j][1],-r*gb->rod_v[i-1][j][2]);
      }
      glNormal3f(gb->rod_v[i][0][0],gb->rod_v[i][0][1],-gb->rod_v[i][0][2]);
      glVertex3f(r*gb->rod_v[i][0][0],r*gb->rod_v[i][0][1],-r*gb->rod_v[i][0][2]);
      glNormal3f(gb->rod_v[i-1][0][0],gb->rod_v[i-1][0][1],-gb->rod_v[i-1][0][2]);
      glVertex3f(r*gb->rod_v[i-1][0][0],r*gb->rod_v[i-1][0][1],-r*gb->rod_v[i-1][0][2]);
      glEnd();
    }
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0, 0.0,-1.0);
    glVertex3f(0.0, 0.0,-r);
    glNormal3f(gb->rod_v[0][0][0],gb->rod_v[0][0][1],-gb->rod_v[0][0][2]);
    glVertex3f(r*gb->rod_v[0][0][0],r*gb->rod_v[0][0][1],-r*gb->rod_v[0][0][2]);
    for(j = gb->rod_vnum*4-1; j > -1 ; j--){
      glNormal3f(gb->rod_v[0][j][0],gb->rod_v[0][j][1],-gb->rod_v[0][j][2]);
      glVertex3f(r*gb->rod_v[0][j][0],r*gb->rod_v[0][j][1],-r*gb->rod_v[0][j][2]);
    }
    glEnd();
  }

}
static void exterior_product(float *v0, float *v1, float *v2)
{
  v2[0] = v0[1]*v1[2] - v0[2]*v1[1];
  v2[1] = v0[2]*v1[0] - v0[0]*v1[2];
  v2[2] = v0[0]*v1[1] - v0[1]*v1[0];
}
void ks_draw_gl_rod_pos(KS_GL_BASE *gb,
			GLfloat sx, GLfloat sy, GLfloat sz, GLfloat ex, GLfloat ey, GLfloat ez,
			GLfloat r, int type, int line_flg)
{
  float v0[3] = {0.0, 0.0, 1.0};
  float v[3],vr[3];
  float d0;
  float len;
  
  v[0] = ex - sx;
  v[1] = ey - sy;
  v[2] = ez - sz;
  len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
  d0 = acos(v[2]/len)/M_PI*180;
  exterior_product(v0,v,vr);
  /*  printf("%f %f (%f %f %f)\n",len,d0,vr[0],vr[1],vr[2]);*/

  glPushMatrix();
  glTranslated(sx,sy,sz);
  if(v[0] == 0 && v[1] == 0){
    glRotatef(d0,1.0,0.0,0.0);
  } else {
    glRotatef(d0,vr[0],vr[1],vr[2]);
  }
  ks_draw_gl_rod(gb,r,len,type,line_flg);
  glPopMatrix();
}
void ks_draw_gl_arrow(KS_GL_BASE *gb, GLfloat r, GLfloat len, int head_div, int type, int line_flg)
{
  int i,j;
  int i0;
  float d0,d1,d2;
  float head_angle = 20;
  float head_len, head_r, head_pos;
  
  if(gb->arrow_n == NULL){
    if((gb->arrow_n = (GLfloat**)ks_malloc(gb->rod_vnum*4 *sizeof(GLfloat*),"ks_draw_gb->arrow"))
       == NULL){
      ks_error("momory error in ks_draw_gb->arrow");
      return;
    }
    for(i = 0; i < gb->rod_vnum*4; i++)
      if((gb->arrow_n[i] = (GLfloat*)ks_malloc(3 *sizeof(GLfloat),"ks_draw_gb->rod")) == NULL){
	ks_error("momory error in ks_draw_gb->arrow");
	return;
      }
    d0 = 2.0*M_PI/gb->rod_vnum/4;
    for(i = 0; i < gb->rod_vnum*4; i++){
      gb->arrow_n[i][0] = cos(d0*i);
      gb->arrow_n[i][1] = sin(d0*i);
      gb->arrow_n[i][2] = tan(head_angle/180.*M_PI);

      d1 = 0;
      for(j = 0; j < 3; j++)
	d1 += gb->arrow_n[i][j]*gb->arrow_n[i][j];
      d1 = sqrt(d1);
      for(j = 0; j < 3; j++)
	gb->arrow_n[i][j] /= d1;
    }
  }

  head_r = r*3;
  head_len = head_r/tan(head_angle/180.*M_PI);

  if(head_len > len){
    head_pos = 0;
  } else {
    head_pos = len - head_len;
  }

  ks_draw_gl_rod(gb,r,head_pos,type,line_flg);

  i = gb->rod_vnum-1;
  glBegin(GL_POLYGON);
  for(j = gb->rod_vnum*4-1; j >= 0; j--){
    glNormal3f(0.0, 0.0, -1.0);
    glVertex3f(head_r*gb->rod_v[i][j][0],
	       head_r*gb->rod_v[i][j][1],
	       head_r*gb->rod_v[i][j][2]+head_pos);
  }
  glEnd();
  /*
  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  for(j = 0; j < gb->rod_vnum*4; j++){
    glVertex3f(head_r*gb->rod_v[i][j][0],
	       head_r*gb->rod_v[i][j][1],
	       head_r*gb->rod_v[i][j][2]+head_pos);
    glVertex3f(head_r*gb->rod_v[i][j][0]+gb->arrow_n[j][0],
	       head_r*gb->rod_v[i][j][1]+gb->arrow_n[j][1],
	       head_r*gb->rod_v[i][j][2]+head_pos+gb->arrow_n[j][2]);
  }
  glEnd();
  glEnable(GL_LIGHTING);
  */
  d2 = 1.0/head_div;
  for(i0 = 0; i0 < head_div; i0++){
    d0 = d2*i0;
    d1 = d2*(i0+1);
    glBegin(GL_TRIANGLE_STRIP);
    for(j = 0; j < gb->rod_vnum*4-1; j++){
      glNormal3fv(gb->arrow_n[j]);
      glVertex3f(d0*head_r*gb->rod_v[i][j][0],
		 d0*head_r*gb->rod_v[i][j][1],
		 head_r*gb->rod_v[i][j][2]+head_pos+head_len*(1.-d0));
      glVertex3f(d1*head_r*gb->rod_v[i][j][0],
		 d1*head_r*gb->rod_v[i][j][1],
		 head_r*gb->rod_v[i][j][2]+head_pos+head_len*(1.-d1));
      glNormal3fv(gb->arrow_n[j+1]);
      glVertex3f(d0*head_r*gb->rod_v[i][j+1][0],
		 d0*head_r*gb->rod_v[i][j+1][1],
		 head_r*gb->rod_v[i][j+1][2]+head_pos+head_len*(1.-d0));
      glVertex3f(d1*head_r*gb->rod_v[i][j+1][0],
		 d1*head_r*gb->rod_v[i][j+1][1],
		 head_r*gb->rod_v[i][j+1][2]+head_pos+head_len*(1.-d1));
    }  
    glNormal3fv(gb->arrow_n[j]);
    glVertex3f(d0*head_r*gb->rod_v[i][j][0],
	       d0*head_r*gb->rod_v[i][j][1],
	       head_r*gb->rod_v[i][j][2]+head_pos+head_len*(1.-d0));
    glVertex3f(d1*head_r*gb->rod_v[i][j][0],
	       d1*head_r*gb->rod_v[i][j][1],
	       head_r*gb->rod_v[i][j][2]+head_pos+head_len*(1.-d1));
    glNormal3fv(gb->arrow_n[0]);
    glVertex3f(d0*head_r*gb->rod_v[i][0][0],
	       d0*head_r*gb->rod_v[i][0][1],
	       head_r*gb->rod_v[i][0][2]+head_pos+head_len*(1.-d0));
    glVertex3f(d1*head_r*gb->rod_v[i][0][0],
	       d1*head_r*gb->rod_v[i][0][1],
	       head_r*gb->rod_v[i][0][2]+head_pos+head_len*(1.-d1));
    glEnd();
  }

  /*
  glBegin(GB->QUAD_STRIP);
  for(j = 0; j < gb->rod_vnum*4-1; j++){
    glNormal3fv(gb->arrow_n[j]);
    glVertex3f(0,0,head_pos+head_len);
    glVertex3f(head_r*gb->rod_v[i][j][0],
	       head_r*gb->rod_v[i][j][1],
	       head_r*gb->rod_v[i][j][2]+head_pos);
    glNormal3fv(gb->arrow_n[j+1]);
    glVertex3f(0,0,head_pos+head_len);
    glVertex3f(head_r*gb->rod_v[i][j+1][0],
	       head_r*gb->rod_v[i][j+1][1],
	       head_r*gb->rod_v[i][j+1][2]+head_pos);
  }  
  glNormal3fv(gb->arrow_n[j]);
  glVertex3f(0,0,head_pos+head_len);
  glVertex3f(head_r*gb->rod_v[i][j][0],
	     head_r*gb->rod_v[i][j][1],
	     head_r*gb->rod_v[i][j][2]+head_pos);
  glNormal3fv(gb->arrow_n[0]);
  glVertex3f(0,0,head_pos+head_len);
  glVertex3f(head_r*gb->rod_v[i][0][0],
	     head_r*gb->rod_v[i][0][1],
	     head_r*gb->rod_v[i][0][2]+head_pos);
  glEnd();
  */

}
void ks_draw_gl_arrow_pos(KS_GL_BASE *gb,
			  GLfloat sx, GLfloat sy, GLfloat sz, GLfloat ex, GLfloat ey, GLfloat ez,
			  GLfloat r, int head_div, int type, int line_flg)
{
  float v0[3] = {0.0, 0.0, 1.0};
  float v[3],vr[3];
  float d0;
  float len;
  
  v[0] = ex - sx;
  v[1] = ey - sy;
  v[2] = ez - sz;
  len = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
  d0 = acos(v[2]/len)/M_PI*180;
  exterior_product(v0,v,vr);
  /*  printf("%f %e (%e %e %e)\n",len,d0,vr[0],vr[1],vr[2]);*/

  glPushMatrix();
  glTranslated(sx,sy,sz);
  if(v[0] == 0 && v[1] == 0){
    if(v[2] < 0){
      glRotatef(180,1.0,0.0,0.0);
    } else {
    }
  } else {
    glRotatef(d0,vr[0],vr[1],vr[2]);
  }
  ks_draw_gl_arrow(gb,r,len,head_div,type,line_flg);
  glPopMatrix();
}
static GLubyte *ks_read_ppm_file(char *file_name, int *w, int *h, KS_CHAR_LIST *search_path)
{
  int i,j,i0;
  char c0[256];
  FILE *fp;
  GLubyte *image;

  if((fp = ks_open_file(file_name,"rb",search_path)) == NULL){
    ks_error("ks_read_ppm_file: file open error %s",file_name);
    return NULL;
  }
  fgets(c0,sizeof(c0),fp);
  fgets(c0,sizeof(c0),fp);
  if(c0[0] == '#'){
    fgets(c0,sizeof(c0),fp);
  }
  sscanf(c0,"%d %d",w,h);
  fgets(c0,sizeof(c0),fp);
  /*
  printf("%s %d %d\n",file_name,*w,*h);
  */
  if((image = (GLubyte*)ks_malloc((*w) * (*h) * 4 *sizeof(GLubyte),"ks_read_ppm_file")) == NULL){
    ks_error("ks_read_ppm_file: memory error\n");
    return NULL;
  }

  for(i = 0; i < *h; i++){
    for(j = 0; j < *w; j++){
      i0 = (*h-1-i)*4+j*4*(*h);
      image[i0+0] = fgetc(fp);
      image[i0+1] = fgetc(fp);
      image[i0+2] = fgetc(fp);
      if(image[i0+0] == 0 &&
         image[i0+1] == 0 &&
         image[i0+2] == 0)
	image[i0+3] = (GLubyte)0;
      else
	image[i0+3] = (GLubyte)255;
    }
  }

  fclose(fp);

  return image;
}
/*
KS_GL_TEXTURE *ks_new_gl_texture(char *file_name)
{
  KS_GL_TEXTURE *newp;

  if((newp = (KS_GL_TEXTURE*)ks_malloc(sizeof(KS_GL_TEXTURE))) == NULL){
    ks_error("new_gl_texture: memory error");
    return NULL;
  }
  if((newp->file_name = ks_malloc_char_copy(file_name,"ks_new_gl_texture")) == NULL){
    ks_error("new_gl_texture: memory error");
    return NULL;
  }
  newp->next = NULL;

  return newp;
}
KS_GL_TEXTURE *ks_add_gl_texture(KS_GL_TEXTURE *listp, KS_GL_TEXTURE *newp)
{
  newp->next = listp;
  return newp;
}
*/
static GLubyte *make_sphere_image(int w, GLfloat light_pos[3])
{
  int i,j;
  int i0;
  double d0;
  GLubyte *image;
  double x,y,z,r;
  double incident = 1.0;
  double reflection;
  double ambient = 0.3;
  double diffuse = 1.0;
  double inner[2];
  double eye[3] = {0.0, 0.0, 1.0};
  double ref_vec[3];

  if((image = (GLubyte*)ks_malloc(w * w * 4 *sizeof(GLubyte),"make_sphere_image")) == NULL){
    ks_error_memory();
    return NULL;
  }

  d0 = sqrt(light_pos[0]*light_pos[0] + light_pos[1]*light_pos[1] + light_pos[2]*light_pos[2]);
  for(i = 0; i < 3; i++)
    light_pos[i] /= d0;

  r = .5*w;
  for(i = 0; i < w; i++){
    for(j = 0; j < w; j++){
      i0 = i*4 + j*4*w;
      x = i-.5*w;
      y = j-.5*w;
      if(r*r > x*x + y*y){

	z = sqrt(r*r-x*x-y*y);
	ref_vec[0] = x-light_pos[0];
	ref_vec[1] = y-light_pos[1];
	ref_vec[2] = z-light_pos[2];
	ks_normalize_vector(ref_vec,3);

	inner[0] = (light_pos[0]*x/r+light_pos[1]*y/r+light_pos[2]*z/r);
	if(inner[0] < 0) inner[0] = 0;
	inner[1] = (ref_vec[0]*eye[0]+ref_vec[1]*eye[1]+ref_vec[2]*eye[2]);
	if(inner[1] < 0) inner[1] = 0;
	reflection = incident*(diffuse*inner[0]+.3*pow(inner[1],1.0));
	//	reflection = diffuse*incident*pow(inner,.3);
	if(reflection > 1.0) reflection = 1.0;
	if(reflection < ambient) reflection = ambient;
	/*
	printf("%d %d % f % f % f %f %f %d\n"
	       ,i,j,x/r,y/r,z/r,incident,reflection,(GLubyte)(255*reflection));
	*/
	image[i0+0] = (GLubyte)(255*reflection);
	image[i0+1] = (GLubyte)(255*reflection);
	image[i0+2] = (GLubyte)(255*reflection);
	image[i0+3] = (GLubyte)255;
      } else {
	image[i0+0] = 0;
	image[i0+1] = 0;
	image[i0+2] = 0;
	image[i0+3] = 0;
      }
    }
  }
  return image;
}
GLuint ks_set_gl_sphere_texture(int size, GLfloat light_pos[3])
{
  int i;
  int w,h;
  GLuint texture;
  GLubyte *image;

  w = 1;
  h = 1;
  for(i = 0; i < size; i++){
    w *= 2;
    h *= 2;
  }
  /*  printf("w %d h %d\n",w,h);*/

#ifdef GL_VERSION_1_1
  glGenTextures(1, &texture);
#endif
  if((image = make_sphere_image(w,light_pos)) == NULL){
    return -1;
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef GL_VERSION_1_1
  glBindTexture(GL_TEXTURE_2D, texture);
  /*  printf("bind tex %s %d\n",file_name,texture);*/
#endif
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    */
#ifdef GL_VERSION_1_1
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, h, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, 4, h, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#endif
  ks_free(image);

  return texture;
}
GLuint ks_set_gl_texture(char *file_name, KS_CHAR_LIST *search_path)
{
  GLuint texture;
  GLubyte *image;
  int w,h;

#ifdef GL_VERSION_1_1
  glGenTextures(1, &texture);
#endif
  if((image = ks_read_ppm_file(file_name,&w,&h,search_path)) == NULL){
    return -1;
  }
  //  printf("%d %d\n",w,h);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef GL_VERSION_1_1
  glBindTexture(GL_TEXTURE_2D, texture);
  /*  printf("bind tex %s %d\n",file_name,texture);*/
#endif
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    */
#ifdef GL_VERSION_1_1
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, h, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, 4, h, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#endif
  ks_free(image);

  return texture;
}
#if 0
KS_GL_TEXTURE *ks_set_gl_texture(char *file_name, KS_CHAR_LIST *search_path)
{
  KS_GL_TEXTURE *tex;
  GLuint texture;
  GLubyte *image;

  if((tex = (KS_GL_TEXTURE*)ks_malloc(sizeof(KS_GL_TEXTURE))) == NULL){
    return NULL;
  }

#ifdef GL_VERSION_1_1
  glGenTextures(1, &texture);
#endif
  if((image = ks_read_ppm_file(file_name,&tex->w,&tex->h,search_path)) == NULL){
    return NULL;
  }
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

#ifdef GL_VERSION_1_1
  glBindTexture(GL_TEXTURE_2D, texture);
  /*  printf("bind tex %s %d\n",file_name,texture);*/
#endif
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /*
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    */
#ifdef GL_VERSION_1_1
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex->h, tex->w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, 4, tex->h, tex->w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
#endif
  ks_free(image);
  tex->label = texture;
  return tex;
}
#endif
#define ICO_X .525731112119133606
#define ICO_Z .850650808352039932
static void ico_drawtriangle(float *v1, float *v2, float *v3, float r)
{
  int i;
  float rv1[3],rv2[3],rv3[3];

  for(i = 0; i < 3; i++){
    rv1[i] = r*v1[i];
    rv2[i] = r*v2[i];
    rv3[i] = r*v3[i];
  }
  glBegin(GL_TRIANGLES);
  glNormal3fv(v1); glVertex3fv(rv1);
  glNormal3fv(v2); glVertex3fv(rv2);
  glNormal3fv(v3); glVertex3fv(rv3);
  glEnd();
}
static void ico_normalize(float v[3])
{
  GLfloat d;

  d = sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
  /*
  if(d == 0.0){
    ks_error("ks_sphere: zero length vector\n");
    return;
  }
  */
  v[0] /= d; v[1] /= d; v[2] /= d;
}
static void ico_subdivide(float *v1, float *v2, float *v3, float r, long depth)
{
  GLfloat v12[3], v23[3], v31[3];
  int i;

  if(depth == 0){
    ico_drawtriangle(v1,v2,v3,r);
    return;
  }
  for(i = 0; i< 3; i++){
    v12[i] = v1[i]+v2[i];
    v23[i] = v2[i]+v3[i];
    v31[i] = v3[i]+v1[i];
  }
  ico_normalize(v12);
  ico_normalize(v23);
  ico_normalize(v31);
  ico_subdivide(v1 ,v12,v31,r,depth-1);
  ico_subdivide(v2 ,v23,v12,r,depth-1);
  ico_subdivide(v3 ,v31,v23,r,depth-1);
  ico_subdivide(v12,v23,v31,r,depth-1);
}
void ks_draw_gl_icosa(float r, int div_num)
{
  static GLfloat vdata[12][3] = {
    {-ICO_X, 0.0, ICO_Z},{ ICO_X, 0.0, ICO_Z},{-ICO_X, 0.0,-ICO_Z},{ ICO_X, 0.0,-ICO_Z},
    { 0.0, ICO_Z, ICO_X},{ 0.0, ICO_Z,-ICO_X},{ 0.0,-ICO_Z, ICO_X},{ 0.0,-ICO_Z,-ICO_X},
    { ICO_Z, ICO_X, 0.0},{-ICO_Z, ICO_X, 0.0},{ ICO_Z,-ICO_X, 0.0},{-ICO_Z,-ICO_X, 0.0}};
  static GLuint tindices[20][3] = {
    {0,4,1},{0,9,4},{9,5,4},{4,5,8},{4,8,1},
    {8,10,1},{8,3,10},{5,3,8},{5,2,3},{2,7,3},
    {7,10,3},{7,6,10},{7,11,6},{11,0,6},{0,1,6},
    {6,1,10},{9,0,11},{9,11,2},{9,2,5},{7,2,11}};
  int i;

  /*
  glDisable(GL_LIGHTING);
  glPolygonMode(GL_FRONT, GL_LINE);
  */
  glBegin(GL_TRIANGLES);
  for(i = 0; i < 20; i++){
    ico_subdivide(&vdata[tindices[i][2]][0],
		  &vdata[tindices[i][1]][0],
		  &vdata[tindices[i][0]][0],r,div_num);
  }
  glEnd();
  /*
  glEnable(GL_LIGHTING);
  */
}
void ks_draw_gl_sphere(float r, int n)
{
  int i,j;
  double x,y;
  double sz[2],cz[2];
  double theta,phi;
  int s_num1,s_num2;

  s_num1 = n;
  s_num2 = n*2;

  theta = M_PI/(s_num1);
  phi = 2.0*M_PI/(s_num2);

  for(i = 0; i < s_num1; i++){
    sz[0] = sin(theta*i);
    sz[1] = sin(theta*(i+1));
    cz[0] = cos(theta*i);
    cz[1] = cos(theta*(i+1));
    /*    printf("%d % f % f\n",i,sz[0],sz[1]);*/
    //    glBegin(GL_QUAD_STRIP);
    glBegin(GL_TRIANGLE_STRIP);
    for(j = 0; j < s_num2+1; j++){
      x = cos(phi*j); y = sin(phi*j);
      glNormal3d(  sz[0]*x,  sz[0]*y,  cz[0]);
      glTexCoord2f(1.-(double)i/(double)(s_num1),(double)j/(double)(s_num2));
      glVertex3d(r*sz[0]*x,r*sz[0]*y,r*cz[0]);
      glNormal3d(  sz[1]*x,  sz[1]*y,  cz[1]);
      glTexCoord2f(1.-(double)(i+1)/(double)(s_num1),(double)j/(double)(s_num2));
      glVertex3d(r*sz[1]*x,r*sz[1]*y,r*cz[1]);
    }
    glEnd();
  }

}
void ks_draw_gl_sphere_shadow_volume(float r, int n, 
				     double light_x, double light_y, double light_z, double len,
				     BOOL light_face)
{
  int i,j;
  double x,y;
  double sz[2],cz[2];
  double theta,phi;
  int s_num1,s_num1h,s_num2;
  double v0[3] = {0.0, 0.0, 1.0};
  double v[3],vr[3];
  double angle;

  v[0] = light_x;
  v[1] = light_y;
  v[2] = light_z;
  angle = acos(v[2]/sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]))/M_PI*180;
  ks_exterior_product(v0,v,vr);

  glPushMatrix();

  if(v[0] == 0 && v[1] == 0){
    glRotatef(angle,1.0,0.0,0.0);
  } else {
    glRotatef(angle,vr[0],vr[1],vr[2]);
  }

  s_num1 = n;
  s_num1h= n/2;
  s_num2 = n*2;

  theta = M_PI/(s_num1);
  phi = 2.0*M_PI/(s_num2);

  for(i = 0; i < s_num1h; i++){
    sz[0] = sin(theta*i);
    sz[1] = sin(theta*(i+1));
    cz[0] = cos(theta*i);
    cz[1] = cos(theta*(i+1));
    /*    printf("%d %f % f % f\n",i,theta*i/M_PI*180,sz[0],sz[1]);*/
    glBegin(GL_TRIANGLE_STRIP);
    for(j = 0; j < s_num2+1; j++){
      x = cos(phi*j); y = sin(phi*j);
      glNormal3d(  sz[0]*x,  sz[0]*y,  cz[0]);
      glVertex3d(r*sz[0]*x,r*sz[0]*y,r*cz[0]);
      glNormal3d(  sz[1]*x,  sz[1]*y,  cz[1]);
      glVertex3d(r*sz[1]*x,r*sz[1]*y,r*cz[1]);
    }
    glEnd();
  }
  if((i%2) == 1){
    sz[0] = sin(theta*i);
    sz[1] = 1.0;
    cz[0] = cos(theta*i);
    cz[1] = 0.0;
    glBegin(GL_TRIANGLE_STRIP);
    for(j = 0; j < s_num2+1; j++){
      x = cos(phi*j); y = sin(phi*j);
      glNormal3d(  sz[0]*x,  sz[0]*y,  cz[0]);
      glVertex3d(r*sz[0]*x,r*sz[0]*y,r*cz[0]);
      glNormal3d(  sz[1]*x,  sz[1]*y,  cz[1]);
      glVertex3d(r*sz[1]*x,r*sz[1]*y,r*cz[1]);
    }
    glEnd();
  }
  if(light_face == KS_TRUE){
  } else {
    glBegin(GL_TRIANGLE_STRIP);
    for(j = 0; j < s_num2+1; j++){
      x = cos(phi*j); y = sin(phi*j);
      glNormal3d(  x,  y,0.0);
      glVertex3d(r*x,r*y,0.0);
      glVertex3d(r*x,r*y,-len);
    }
    glEnd();
  }
  glPopMatrix();
}
static void mat_inv(double a[4][4])
{
  int i,j,k;
  double t, u, det;
  int n = 3;

  det = 1;
  for(k = 0; k < n; k++){
    t = a[k][k]; det *= t;
    for(i = 0; i < n; i++) a[k][i] /= t;
    a[k][k] = 1 / t;
    for(j = 0; j < n; j++)
      if(j != k){
        u = a[j][k];
        for(i = 0; i < n; i++)
          if(i != k) a[j][i] -= a[k][i] * u;
          else       a[j][i] = -u/t;
      }
  }
}
void ks_calc_gl_rot_inv_mat(double rmat[16], double imat[16])
{
  int i;
  for(i = 0; i < 16; i++)
    imat[i] = rmat[i];
  mat_inv((double(*)[4])imat);
}
static void ks_gl_base_lookat(KS_GL_BASE *gb, int stereo)
{
  double d0,d1,d3;
  if(stereo == KS_GL_STEREO_NOT_USE){
    d1 = 0;
    d0 = gb->eye_len;
  } else if(stereo == KS_GL_STEREO_LEFT){
    d3 = atan((-gb->eye_width*gb->eye_flg)/gb->eye_len);
    d1 = sin(d3)*gb->eye_len;
    d0 = cos(d3)*gb->eye_len;
  } else if(stereo == KS_GL_STEREO_RIGHT){
    d3 = atan((gb->eye_width*gb->eye_flg)/gb->eye_len);
    d1 = sin(d3)*gb->eye_len;
    d0 = cos(d3)*gb->eye_len;
  }
    
  //  printf("%f %f\n",d0,d1);
  /*  gluLookAt(d0, d1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);*/
#ifdef ENABLE_DOME
  if(gb->dome_id != -1){
    d3 = atan((gb->eye_width*gb->eye_flg*dome_eye_flag[gb->dome_id])/gb->eye_len);
    d1 = sin(d3)*gb->eye_len;
    d0 = cos(d3)*gb->eye_len;
  }
#endif

  gluLookAt(gb->lookat[KS_GL_LOOKAT_NOW][0]+d0, 
	    gb->lookat[KS_GL_LOOKAT_NOW][1]+d1, 
	    gb->lookat[KS_GL_LOOKAT_NOW][2], 
	    gb->lookat[KS_GL_LOOKAT_NOW][0],
	    gb->lookat[KS_GL_LOOKAT_NOW][1],
	    gb->lookat[KS_GL_LOOKAT_NOW][2],
	    0.0, 0.0, 1.0);
  /*
  gluLookAt(d0, 
	    d1, 
	    0, 
	    gb->lookat[KS_GL_LOOKAT_NOW][0],
	    gb->lookat[KS_GL_LOOKAT_NOW][1],
	    gb->lookat[KS_GL_LOOKAT_NOW][2],
	    0.0, 0.0, 1.0);
  */
}
/*
void ks_trans_gl(KS_GL_BASE *gb)
{
  glTranslated(gb->trans[0], gb->trans[1], gb->trans[2]);
}
*/
/*
static void rotate(KS_GL_BASE *gb)
{
  glTranslated(gb->rotation_center[0],gb->rotation_center[1],gb->rotation_center[2]);

  glPushMatrix();
  glLoadIdentity();
  glRotatef( gb->angle[0],1.0,0.0,0.0);
  glRotatef( gb->angle[1],0.0,1.0,0.0);
  glRotatef( gb->angle[2],0.0,0.0,1.0);
  glMultMatrixd(gb->rot_mat);
  glGetDoublev(GL_MODELVIEW_MATRIX, gb->rot_mat);
  glPopMatrix();

  ks_calc_rot_inv_mat(gb);

  glMultMatrixd(gb->rot_mat);

  gb->angle[0] = 0;
  gb->angle[1] = 0;
  gb->angle[2] = 0;

  glTranslated(-gb->rotation_center[0],-gb->rotation_center[1],-gb->rotation_center[2]);
}
*/
void ks_gl_base_set_stereo_mode(KS_GL_BASE *gb, unsigned int mode)
{
  gb->stereo = mode;
}
void ks_gl_base_set_translational_array(KS_GL_BASE *gb, double tx,double ty, double tz,
					double cell_size)
{
  gb->trans[0] = tx-(1.4*cell_size/tan(gb->eye_ang/180.*M_PI)-gb->eye_len);
  /*  printf("%f %f\n",1.0*cell_size/tan(gb->eye_ang/180.*M_PI),gb->eye_len);*/
  gb->trans[1] = ty;
  gb->trans[2] = tz;
}
void ks_set_gl_eye_position(KS_GL_BASE *gb, double pos[3])
{
  gb->trans[0] = gb->lookat[KS_GL_LOOKAT_NOW][0]+gb->eye_len-pos[0];
  gb->trans[1] = gb->lookat[KS_GL_LOOKAT_NOW][1]            -pos[1];
  gb->trans[2] = gb->lookat[KS_GL_LOOKAT_NOW][2]            -pos[2];
}
void ks_gl_base_set_rotational_matrix(KS_GL_BASE *gb, double ax,double ay, double az)
{
  glPushMatrix();
  glLoadIdentity();
  glRotatef( ax,1.0,0.0,0.0);
  glRotatef( ay,0.0,1.0,0.0);
  glRotatef( az,0.0,0.0,1.0);
  glGetDoublev(GL_MODELVIEW_MATRIX, gb->rot_mat);
  glPopMatrix();
}
void ks_gl_base_set_projection_mode(KS_GL_BASE *gb, unsigned int mode)
{
  gb->projection = mode;
}
unsigned int ks_gl_base_get_projection_mode(KS_GL_BASE *gb)
{
  return gb->projection;
}
static void single_display(KS_GL_BASE *gb, void *vp, int stereo, 
			   void (*display_3d)(KS_GL_BASE*,void*), double mat[16])
{
  glLoadIdentity();

#ifdef ENABLE_DOME
  if(gb->dome_id == -1){
    ks_gl_base_lookat(gb,stereo);
  } else {
    glRotatef(dome_rotate[gb->dome_id][0][0], dome_rotate[gb->dome_id][0][1], 
	      dome_rotate[gb->dome_id][0][2], dome_rotate[gb->dome_id][0][3]); 
    glRotatef(dome_rotate[gb->dome_id][1][0], dome_rotate[gb->dome_id][1][1], 
	      dome_rotate[gb->dome_id][1][2], dome_rotate[gb->dome_id][1][3]);
    glRotatef(dome_rotate[gb->dome_id][2][0], dome_rotate[gb->dome_id][2][1], 
	      dome_rotate[gb->dome_id][2][2], dome_rotate[gb->dome_id][2][3]);
    glRotatef(30.0, 1.0, 0.0, 0.0);
    ks_gl_base_lookat(gb,stereo);
  }
#else
  ks_gl_base_lookat(gb,stereo);
#endif

  glGetDoublev(GL_MODELVIEW_MATRIX, gb->init_mat);

  glPushMatrix(); /* 1 */

  glTranslated(gb->trans[0], gb->trans[1], gb->trans[2]);

  glPushMatrix(); /* 2 */

  glPushMatrix();
  glLoadIdentity();
  glRotatef( gb->angle[0],1.0,0.0,0.0);
  glRotatef( gb->angle[1],0.0,1.0,0.0);
  glRotatef( gb->angle[2],0.0,0.0,1.0);
  glMultMatrixd(gb->rot_mat);
  glGetDoublev(GL_MODELVIEW_MATRIX, gb->rot_mat);
  glPopMatrix();

  gb->angle[0] = 0;
  gb->angle[1] = 0;
  gb->angle[2] = 0;

  ks_calc_gl_rot_inv_mat(gb->rot_mat,gb->rot_imat);

  glTranslated(-gb->rotation_center_offset[0], 
	       -gb->rotation_center_offset[1], 
	       -gb->rotation_center_offset[2]);

  glTranslated(gb->rotation_center[0], gb->rotation_center[1], gb->rotation_center[2]);

  glMultMatrixd(gb->rot_mat);

  glTranslated(-(gb->rotation_center[0]),
	       -(gb->rotation_center[1]),
	       -(gb->rotation_center[2]));

  /*  display_3d(gb,vp);*/
  if(display_3d != NULL){
    display_3d(gb,vp);
  } else if(mat != NULL){
    glGetDoublev(GL_MODELVIEW_MATRIX,mat);
  }

  glPopMatrix(); /* 2 */

  glPopMatrix(); /* 1 */
}
void ks_gl_base_display(KS_GL_BASE *gb, void *vp,
			void (*display_3d)(KS_GL_BASE*, void*), 
			void (*display_2d)(KS_GL_BASE*, void*,int,int))
{
  GLboolean stencil;
  /*
  glViewport((GLint)gb->viewport[0],(GLint)gb->viewport[1], 
	     (GLsizei)gb->viewport[2], (GLsizei)gb->viewport[3]);
  */
  glViewport((GLint)gb->viewport_3d[KS_GL_VIEWPORT_X],
	     (GLint)gb->viewport_3d[KS_GL_VIEWPORT_Y], 
	     (GLsizei)gb->viewport_3d[KS_GL_VIEWPORT_WIDTH],
	     (GLsizei)gb->viewport_3d[KS_GL_VIEWPORT_HEIGHT]);
  ks_gl_base_clearcolor(gb);

  /* 3D */

  if(display_3d != NULL){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    ks_gl_base_scene(gb,gb->viewport_3d);

    glMatrixMode(GL_MODELVIEW);

    if(ks_gl_base_get_stereo_mode(gb) == KS_GL_BASE_STEREO_NOT_USE){
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      single_display(gb,vp,KS_GL_STEREO_NOT_USE,display_3d,NULL);
    } else if(ks_gl_base_get_stereo_mode(gb) == KS_GL_BASE_STEREO_QUAD_BUFFER){
      glDrawBuffer(GL_BACK_LEFT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      single_display(gb,vp,KS_GL_STEREO_LEFT,display_3d,NULL);
      glDrawBuffer(GL_BACK_RIGHT);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      single_display(gb,vp,KS_GL_STEREO_RIGHT,display_3d,NULL);
    } else if(ks_gl_base_get_stereo_mode(gb) == KS_GL_BASE_STEREO_HORIZONTAL_SLIT){
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      stencil = ks_gl_enable(GL_STENCIL_TEST);
      glStencilFunc (GL_EQUAL, 0x1, 0x1);
      glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
      single_display(gb,vp,KS_GL_STEREO_LEFT,display_3d,NULL);
      glStencilFunc (GL_NOTEQUAL, 0x1, 0x1);
      single_display(gb,vp,KS_GL_STEREO_RIGHT,display_3d,NULL);
      if(stencil == GL_TRUE)
	glDisable(GL_STENCIL_TEST);
    } else if(ks_gl_base_get_stereo_mode(gb) == KS_GL_BASE_STEREO_VERTICAL_SLIT){
      stencil = ks_gl_enable(GL_STENCIL_TEST);
      glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glStencilFunc (GL_EQUAL, 0x1, 0x1);
      glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
      single_display(gb,vp,KS_GL_STEREO_LEFT,display_3d,NULL);
      glStencilFunc (GL_NOTEQUAL, 0x1, 0x1);
      glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
      single_display(gb,vp,KS_GL_STEREO_LEFT,display_3d,NULL);
      glClear(GL_DEPTH_BUFFER_BIT);
      glStencilFunc (GL_NOTEQUAL, 0x1, 0x1);
      glColorMask(GL_TRUE, GL_FALSE, GL_TRUE, GL_TRUE);
      single_display(gb,vp,KS_GL_STEREO_RIGHT,display_3d,NULL);
      glStencilFunc (GL_EQUAL, 0x1, 0x1);
      glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_TRUE);
      single_display(gb,vp,KS_GL_STEREO_RIGHT,display_3d,NULL);
      glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
      if(stencil == GL_TRUE)
	glDisable(GL_STENCIL_TEST);
    }
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  /* 2D */
  /*
  glViewport((GLint)gb->viewport[0],(GLint)gb->viewport[1], 
	     (GLsizei)gb->viewport[2], (GLsizei)gb->viewport[3]);
  */
  glViewport((GLint)gb->viewport_2d[KS_GL_VIEWPORT_X],
	     (GLint)gb->viewport_2d[KS_GL_VIEWPORT_Y], 
	     (GLsizei)gb->viewport_2d[KS_GL_VIEWPORT_WIDTH],
	     (GLsizei)gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT]);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  /*  gluOrtho2D(0,gb->viewport[2],0,gb->viewport[3]);*/
  /*
  glOrtho(0,gb->viewport[KS_GL_VIEWPORT_WIDTH],0,gb->viewport[KS_GL_VIEWPORT_HEIGHT],
	  gb->depth_2d_near,gb->depth_2d_far);
  */
  glOrtho(gb->viewport_2d[KS_GL_VIEWPORT_X],
	  gb->viewport_2d[KS_GL_VIEWPORT_WIDTH],
	  gb->viewport_2d[KS_GL_VIEWPORT_Y],
	  gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT],
	  gb->depth_2d_near,
	  gb->depth_2d_far);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  /*
  printf("%d %d\n",gb->viewport[KS_GL_VIEWPORT_WIDTH],gb->viewport[KS_GL_VIEWPORT_HEIGHT]);
  */
  if(display_2d != NULL){
    /*
    display_2d(gb,vp,gb->viewport[KS_GL_VIEWPORT_WIDTH],gb->viewport[KS_GL_VIEWPORT_HEIGHT]);
    */
    display_2d(gb,vp,gb->viewport_2d[KS_GL_VIEWPORT_WIDTH],gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT]);
  }
  ks_draw_gl_window(gb,gb->window);

  /*
  glViewport((GLint)gb->viewport[0],(GLint)gb->viewport[1], 
	     (GLsizei)gb->viewport[2], (GLsizei)gb->viewport[3]);
  */
}
static void reshape_gl_window(KS_GL_WINDOW *win, int *viewport, int w, int h)
{
  for(;win != NULL; win = win->next){
    win->viewport[KS_GL_VIEWPORT_Y] += h-viewport[KS_GL_VIEWPORT_HEIGHT];
  }
}
/*
static void set_window_size(int w, int h, int *window_size, int *viewport, int *viewport)
{
  window_size[KS_GL_WINDOW_SIZE_WIDTH] = w;
  window_size[KS_GL_WINDOW_SIZE_HEIGHT] = h;
  viewport[KS_GL_VIEWPORT_WIDTH] = w;
  viewport[KS_GL_VIEWPORT_HEIGHT] = h;
  viewport[KS_GL_VIEWPORT_WIDTH] = w;
  viewport[KS_GL_VIEWPORT_HEIGHT] = h;
}
*/
void ks_gl_base_reshape(KS_GL_BASE *gb, int w, int h)
{
  int i;
  GLboolean stencil;

  reshape_gl_window(gb->window,gb->viewport_2d,w,h);

  /*  glViewport(0, 0, (GLsizei)w, (GLsizei)h);*/
  /*
  gb->viewport[KS_GL_VIEWPORT_WIDTH] = w;
  gb->viewport[KS_GL_VIEWPORT_HEIGHT] = h;
  */
  gb->window_size[KS_GL_WINDOW_SIZE_WIDTH] = w;
  gb->window_size[KS_GL_WINDOW_SIZE_HEIGHT] = h;
  gb->viewport_3d[KS_GL_VIEWPORT_WIDTH] = w;
  gb->viewport_3d[KS_GL_VIEWPORT_HEIGHT] = h;
  gb->viewport_2d[KS_GL_VIEWPORT_WIDTH] = w;
  gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT] = h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  if(ks_gl_base_get_stereo_mode(gb) == KS_GL_BASE_STEREO_HORIZONTAL_SLIT){
    stencil = ks_gl_enable(GL_STENCIL_TEST);
    gluOrtho2D(0,w,0,h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc (GL_ALWAYS, 0x1, 0x1);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_LINES);
    for(i = 0; i < h; i++){
      if(i % 2){
	glVertex2f(0,i);
	glVertex2f(w,i);
      }
    }
    glEnd();
    if(stencil == GL_TRUE)
      glDisable(GL_STENCIL_TEST);
  } else if(ks_gl_base_get_stereo_mode(gb) == KS_GL_BASE_STEREO_VERTICAL_SLIT){
    stencil = ks_gl_enable(GL_STENCIL_TEST);
    gluOrtho2D(0,w,0,h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilFunc (GL_ALWAYS, 0x1, 0x1);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_LINES);
    for(i = 0; i < w; i++){
      if(i % 2){
	glVertex2f(i,0);
	glVertex2f(i,h);
      }
    }
    glEnd();
    if(stencil == GL_TRUE)
      glDisable(GL_STENCIL_TEST);
  }
}
int ks_gl_base_verbose_level(KS_GL_BASE *gb)
{
  return gb->verbose_level;
}
void ks_gl_base_clearcolor(KS_GL_BASE *gb)
{
  glClearColor(gb->background_color[0],
	       gb->background_color[1],
	       gb->background_color[2],
	       gb->background_color[3]);
}
void ks_gl_base_foreground_color(KS_GL_BASE *gb)
{
  glColor4fv(gb->foreground_color);
}
GLfloat *ks_get_gl_base_foreground_color(KS_GL_BASE *gb)
{
  return gb->foreground_color;
}
void ks_set_gl_base_foreground_color(KS_GL_BASE *gb, 
				     GLfloat c0, GLfloat c1, GLfloat c2, GLfloat c3)
{
  gb->foreground_color[0] = c0;
  gb->foreground_color[1] = c1;
  gb->foreground_color[2] = c2;
  gb->foreground_color[3] = c3;
}
void ks_set_gl_base_background_color(KS_GL_BASE *gb, 
				     GLfloat c0, GLfloat c1, GLfloat c2, GLfloat c3)
{
  gb->background_color[0] = c0;
  gb->background_color[1] = c1;
  gb->background_color[2] = c2;
  gb->background_color[3] = c3;
}
void ks_gl_base_scene(KS_GL_BASE *gb, int viewport[4])
{
  double d0,d1;
  if(gb->projection == KS_GL_BASE_PERSPECTIVE){
    //    printf("%f %d %d\n",gb->eye_ang,viewport[2],viewport[3]);
#ifdef ENABLE_DOME
    if(gb->dome_id == -1){
      gluPerspective(gb->eye_ang, 
		     (double)viewport[KS_GL_VIEWPORT_WIDTH]/
		     (double)viewport[KS_GL_VIEWPORT_HEIGHT], 
		     gb->depth_3d_near,gb->depth_3d_far);
    } else {
      glFrustum(dome_frustum[gb->dome_id][0], dome_frustum[gb->dome_id][1], 
		dome_frustum[gb->dome_id][2], dome_frustum[gb->dome_id][3], 
		dome_frustum[gb->dome_id][4], dome_frustum[gb->dome_id][5]); 
    }
#else
    gluPerspective(gb->eye_ang, 
		   (double)viewport[KS_GL_VIEWPORT_WIDTH]/(double)viewport[KS_GL_VIEWPORT_HEIGHT], 
		   gb->depth_3d_near,gb->depth_3d_far);
#endif
  } else if(gb->projection == KS_GL_BASE_ORTHO){
    d0 = (gb->eye_len-gb->trans[0])*tan(gb->eye_ang*.5/180*M_PI);
    d1 = d0*(double)viewport[2] / (double)viewport[3];
    /*    glOrtho(-d1,d1,-d0,d0,-1200,1200);*/
    glOrtho(-d1,d1,-d0,d0,gb->depth_3d_near,gb->depth_3d_far);
  }
}
int ks_gl_base_get_stereo_mode(KS_GL_BASE *gb)
{
  return gb->stereo;
}
#ifndef USE_GLSL
void static processHits(KS_GL_BASE *gb)
{
  int i;
  unsigned int j;
  GLuint names, *ptr;
  GLuint *hit_min;

  /*  printf("hits = %d\n", gb->hits);*/
  if(gb->hits == 0){
    gb->hit_name_num = 0;
  } else {
    ptr = (GLuint *) gb->select_buffer;
    hit_min = (GLuint *)gb->select_buffer;
    for (i = 0; i < gb->hits; i++) {  /* for each hit  */
      names = *ptr;
      /*      printf(" number of names for hit = %d\n", names); */
      ptr++;
      /*      printf("  z1 is %g;", (float) *ptr/0x7fffffff); */
      if(*ptr < hit_min[1]) hit_min = ptr-1;
      ptr++;
      /*      printf(" z2 is %g\n", (float) *ptr/0x7fffffff); */
      ptr++;
      /*      printf("   the name is ");*/
      for (j = 0; j < names; j++) {
	/*	printf("%d ", *ptr);*/
	ptr++;
      }
      /*      printf("\n");*/
    }
    /*
    printf("%g %g %u %u\n"
	   ,(float)hit_min[1]/0x7fffffff,(float)hit_min[2]/0x7fffffff,hit_min[1],hit_min[2]);
    for(i = 0; i < hit_min[0]; i++)
      printf("%d ",hit_min[i+3]);
    printf("\n");
    */
    gb->hit_name_num = hit_min[0];
    gb->hit_name = &hit_min[3];
  }
  /*
  for(i = 0; i < gb->hit_name_num; i++)
    printf("%d ",gb->hit_name[i]);
  printf("\n");
  */

}
#endif
static void get_modifires(KS_GL_BASE *gb)
{
#ifdef GLUT
  /*
  if(glutGetModifiers()== GLUT_ACTIVE_SHIFT) gb->active_shift = 1; else gb->active_shift = 0;
  if(glutGetModifiers()== GLUT_ACTIVE_ALT) gb->active_alt = 1; else  gb->active_alt = 0;
  if(glutGetModifiers()== GLUT_ACTIVE_CTRL) gb->active_ctrl = 1; else  gb->active_ctrl = 0;
  printf("%x shirt %d alt %d ctrl %d\n",glutGetModifiers()
	 ,gb->active_shift,gb->active_alt,gb->active_ctrl);
  */
  if(!(gb->active_flags&KS_GL_ACTIVE_NO_GET)){
    gb->active_flags = glutGetModifiers();
  }
  /*
  printf("shift %d alt %d ctrf %d\n"
	 ,gb->active_flags&GLUT_ACTIVE_SHIFT
	 ,gb->active_flags&GLUT_ACTIVE_ALT
	 ,gb->active_flags&GLUT_ACTIVE_CTRL);
  */
#endif
}
BOOL ks_get_glut_shift(KS_GL_BASE *gb)
{
  return gb->active_flags&KS_GL_ACTIVE_SHIFT ? KS_TRUE:KS_FALSE;
}
BOOL ks_get_glut_ctrl(KS_GL_BASE *gb)
{
  return gb->active_flags&KS_GL_ACTIVE_CTRL ? KS_TRUE:KS_FALSE;
}
BOOL ks_get_glut_alt(KS_GL_BASE *gb)
{
  return gb->active_flags&KS_GL_ACTIVE_ALT ? KS_TRUE:KS_FALSE;
}
static unsigned int check_window_range(int *viewport, int *win_viewport, int x, int y)
{
  unsigned int flags = 0;
  if(x <= (win_viewport[KS_GL_VIEWPORT_X])){
    flags |= KS_GL_WINDOW_PICK_FRAME_LEFT;
  } else if(x < win_viewport[KS_GL_VIEWPORT_X]+win_viewport[KS_GL_VIEWPORT_WIDTH]){
    if(viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (win_viewport[KS_GL_VIEWPORT_Y]+
					     win_viewport[KS_GL_VIEWPORT_HEIGHT]) &&
       viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (win_viewport[KS_GL_VIEWPORT_Y]+
					     win_viewport[KS_GL_VIEWPORT_HEIGHT]+
					     KS_GL_WINDOW_TITLE_BAR_SIZE-
					     KS_GL_WINDOW_FRAME_SIZE+2)){
      flags |= KS_GL_WINDOW_PICK_TITLE_BAR;
    } else if(viewport[KS_GL_VIEWPORT_HEIGHT]-y>=win_viewport[KS_GL_VIEWPORT_Y] &&
	      viewport[KS_GL_VIEWPORT_HEIGHT]-y<=(win_viewport[KS_GL_VIEWPORT_Y]+
						  win_viewport[KS_GL_VIEWPORT_HEIGHT])
	      ){
      flags |= KS_GL_WINDOW_PICK_MAIN;
    }
  } else {
    flags |= KS_GL_WINDOW_PICK_FRAME_RIGHT;
  }
  if(viewport[KS_GL_VIEWPORT_HEIGHT]-y < win_viewport[KS_GL_VIEWPORT_Y]){
    flags |= KS_GL_WINDOW_PICK_FRAME_BOTTOM;
  } else if(viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (win_viewport[KS_GL_VIEWPORT_Y]+
						  win_viewport[KS_GL_VIEWPORT_HEIGHT]+
						  KS_GL_WINDOW_TITLE_BAR_SIZE-
						  KS_GL_WINDOW_FRAME_SIZE+2) &&
	    viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (win_viewport[KS_GL_VIEWPORT_Y]+
						  win_viewport[KS_GL_VIEWPORT_HEIGHT]+
						  KS_GL_WINDOW_TITLE_BAR_SIZE)){
    flags |= KS_GL_WINDOW_PICK_FRAME_TOP;
  }
  return flags;
}
BOOL ks_gl_base_mouse(KS_GL_BASE *gb, void *vp, int button, int state, int x, int y, 
		      void(*display_3d)(KS_GL_BASE*,void*))
{
  double t;
  int *viewport;
  BOOL pick_enable = KS_FALSE;

  viewport = gb->viewport_2d;

  if(button >= KS_GL_MOUSE_BUTTON_NUM){
    double len = 40+(gb->eye_len-gb->trans[0]);
    if(button==KS_GL_MOUSE_BUTTON_NUM){
      gb->trans[0] -= (len/100);
    }else{
      gb->trans[0] += (len/100);
    }
    ks_redraw_gl(gb);
    return pick_enable;
  }

  get_modifires(gb);
  /*
  gb->hits = 0;
  gb->hit_name_num = 0;
  */
  t = ks_get_time();
  gb->mouse_time[KS_GL_MOUSE_TIME_LAP][button][state] 
    = t-gb->mouse_time[KS_GL_MOUSE_TIME_NOW][button][state];
  gb->mouse_time[KS_GL_MOUSE_TIME_NOW][button][state] = t;
  gb->mouse_pos[button][state][0] = x;
  gb->mouse_pos[button][state][1] = y;
  gb->mouse_state[button] = state;
  if (state == KS_GL_MOUSE_STATE_DOWN) {
    gb->mouse_pos[button][KS_GL_MOUSE_STATE_MOTION][0] = x;
    gb->mouse_pos[button][KS_GL_MOUSE_STATE_MOTION][1] = y;
  }

  {
    KS_GL_WINDOW *win;
    for(win = gb->window; win != NULL; win = win->next){
      if(win->mouse != NULL && 
	 x >= win->viewport[KS_GL_VIEWPORT_X] && 
	 x <= win->viewport[KS_GL_VIEWPORT_X]+win->viewport[KS_GL_VIEWPORT_WIDTH] &&
	 viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (win->viewport[KS_GL_VIEWPORT_Y]) && 
	 viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (win->viewport[KS_GL_VIEWPORT_Y]+
						   win->viewport[KS_GL_VIEWPORT_HEIGHT])){
	(win->mouse)(gb,win->id,
		     win->viewport[KS_GL_VIEWPORT_WIDTH],win->viewport[KS_GL_VIEWPORT_HEIGHT],
		     button,state,x-win->viewport[KS_GL_VIEWPORT_X],
		     viewport[KS_GL_VIEWPORT_HEIGHT]-y-win->viewport[KS_GL_VIEWPORT_Y],
		     win->vp);
      }
    }
  }

  if(button == KS_GL_MOUSE_BUTTON_LEFT && state == KS_GL_MOUSE_STATE_UP){
    if(gb->pick_window != NULL){
      if(!(gb->pick_window->flags&KS_GL_WINDOW_NO_FRAME)){
	if(gb->pick_window->flags&KS_GL_WINDOW_CLOSE &&
	   x>=(gb->pick_window->viewport[KS_GL_VIEWPORT_X]+
	       gb->pick_window->viewport[KS_GL_VIEWPORT_WIDTH]+
	       KS_GL_WINDOW_CLOSE_X) &&
	   x<=(gb->pick_window->viewport[KS_GL_VIEWPORT_X]+
	       gb->pick_window->viewport[KS_GL_VIEWPORT_WIDTH]+
	       KS_GL_WINDOW_CLOSE_X+
	       KS_GL_WINDOW_CLOSE_WIDTH) &&
	   viewport[KS_GL_VIEWPORT_HEIGHT]-y>=
	   (gb->pick_window->viewport[KS_GL_VIEWPORT_Y]+
	    gb->pick_window->viewport[KS_GL_VIEWPORT_HEIGHT]+
	    KS_GL_WINDOW_CLOSE_Y) &&
	   viewport[KS_GL_VIEWPORT_HEIGHT]-y<=
	   (gb->pick_window->viewport[KS_GL_VIEWPORT_Y]+
	    gb->pick_window->viewport[KS_GL_VIEWPORT_HEIGHT]+
	    KS_GL_WINDOW_CLOSE_Y+KS_GL_WINDOW_CLOSE_HEIGHT)){ /* close */
	  gb->pick_window->flags &= ~KS_GL_WINDOW_CLOSE;
	  gb->pick_window->flags &= ~KS_GL_WINDOW_SHOW;
	} else {
	  gb->pick_window->flags &= ~KS_GL_WINDOW_CLOSE;
	}
	gb->pick_window = NULL;
      }
    } else if( // picking process
#ifdef USE_ALT_PICK
	      ks_get_glut_alt(gb) == KS_TRUE &&  // alt key is required
#endif
	      (gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_DOWN][0] ==
	       gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP][0]) &&
	      (gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_DOWN][1] ==
	       gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_UP][1])){
#ifdef USE_GLSL
      pick_enable = KS_TRUE;
#else
      glSelectBuffer (KS_GL_SELECT_BUFFER_SIZE, gb->select_buffer);
      (void) glRenderMode (GL_SELECT);

      glInitNames();
      glPushName(0);

      glMatrixMode (GL_PROJECTION);
      glPushMatrix ();
      glLoadIdentity ();

      gluPickMatrix ((GLdouble) x, (GLdouble) (gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT] - y), 
		     5.0, 5.0, gb->viewport_3d);
      /*
      gluPerspective(30.0,
		     (double)viewport[KS_GL_VIEWPORT_WIDTH]/
		     (double)viewport[KS_GL_VIEWPORT_HEIGHT], 1.0, 200.0);
      */
      ks_gl_base_scene(gb,gb->viewport_3d);

      glMatrixMode( GL_MODELVIEW );
      single_display(gb,vp,KS_GL_STEREO_NOT_USE,display_3d,NULL);

      glMatrixMode (GL_PROJECTION);
      glPopMatrix ();

      glMatrixMode( GL_MODELVIEW );

      gb->hits = glRenderMode (GL_RENDER);

      processHits(gb);
#endif
    }
  }
  if(button == KS_GL_MOUSE_BUTTON_LEFT && state == KS_GL_MOUSE_STATE_DOWN){
    KS_GL_WINDOW *p;
    for(p = gb->window; p != NULL; p = p->next){
      p->flags &= ~KS_GL_WINDOW_PICK_ALL;
      if(p->flags&KS_GL_WINDOW_SHOW && !(p->flags&KS_GL_WINDOW_NO_FRAME)){
	if(x >= (p->viewport[KS_GL_VIEWPORT_X]-KS_GL_WINDOW_FRAME_SIZE) &&
	   x <= (p->viewport[KS_GL_VIEWPORT_X]+p->viewport[KS_GL_VIEWPORT_WIDTH]+
		 KS_GL_WINDOW_FRAME_SIZE) &&
	   viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (p->viewport[KS_GL_VIEWPORT_Y]-
						     KS_GL_WINDOW_FRAME_SIZE) &&
	   viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (p->viewport[KS_GL_VIEWPORT_Y]+
						     p->viewport[KS_GL_VIEWPORT_HEIGHT]+
						     KS_GL_WINDOW_TITLE_BAR_SIZE)){/*pick */
	  /*
	    printf("%d %d %d\n"
	    ,viewport[KS_GL_VIEWPORT_HEIGHT]-y
	    ,(p->viewport[KS_GL_VIEWPORT_Y]-KS_GL_WINDOW_FRAME_SIZE)
	    ,(p->viewport[KS_GL_VIEWPORT_Y]+p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	    KS_GL_WINDOW_TITLE_BAR_SIZE));
	  */
	  gb->pick_window = p;

	  if(x>=(p->viewport[KS_GL_VIEWPORT_X]+p->viewport[KS_GL_VIEWPORT_WIDTH]+
		 KS_GL_WINDOW_CLOSE_X) &&
	     x<=(p->viewport[KS_GL_VIEWPORT_X]+p->viewport[KS_GL_VIEWPORT_WIDTH]+
		 KS_GL_WINDOW_CLOSE_X+
		 KS_GL_WINDOW_CLOSE_WIDTH) &&
	     viewport[KS_GL_VIEWPORT_HEIGHT]-y>=
	     (p->viewport[KS_GL_VIEWPORT_Y]+p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	      KS_GL_WINDOW_CLOSE_Y) &&
	     viewport[KS_GL_VIEWPORT_HEIGHT]-y<=
	     (p->viewport[KS_GL_VIEWPORT_Y]+p->viewport[KS_GL_VIEWPORT_HEIGHT]+
	      KS_GL_WINDOW_CLOSE_Y+KS_GL_WINDOW_CLOSE_HEIGHT)){ /* close */
	    /*	  p->flags &= ~KS_GL_WINDOW_SHOW;*/
	    p->flags |= KS_GL_WINDOW_CLOSE;
	  } else {

	    p->flags |= check_window_range(viewport,p->viewport,x,y);
	    /*
	    if(x <= (p->viewport[KS_GL_VIEWPORT_X])){
	      p->flags |= KS_GL_WINDOW_PICK_FRAME_LEFT;
	    } else if(x < p->viewport[KS_GL_VIEWPORT_X]+p->viewport[KS_GL_VIEWPORT_WIDTH]){
	      if(viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (p->viewport[KS_GL_VIEWPORT_Y]+
							   p->viewport[KS_GL_VIEWPORT_HEIGHT]) &&
		 viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (p->viewport[KS_GL_VIEWPORT_Y]+
							   p->viewport[KS_GL_VIEWPORT_HEIGHT]+
							   KS_GL_WINDOW_TITLE_BAR_SIZE-
							   KS_GL_WINDOW_FRAME_SIZE+2)){
		p->flags |= KS_GL_WINDOW_PICK_TITLE_BAR;
	      } else if(viewport[KS_GL_VIEWPORT_HEIGHT]-y>=p->viewport[KS_GL_VIEWPORT_Y] &&
			viewport[KS_GL_VIEWPORT_HEIGHT]-y<=(p->viewport[KS_GL_VIEWPORT_Y]+
								p->viewport[KS_GL_VIEWPORT_HEIGHT])
			){
		p->flags |= KS_GL_WINDOW_PICK_MAIN;
	      }
	    } else {
	      p->flags |= KS_GL_WINDOW_PICK_FRAME_RIGHT;
	    }
	    if(viewport[KS_GL_VIEWPORT_HEIGHT]-y < p->viewport[KS_GL_VIEWPORT_Y]){
	      p->flags |= KS_GL_WINDOW_PICK_FRAME_BOTTOM;
	    } else if(viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (p->viewport[KS_GL_VIEWPORT_Y]+
								p->viewport[KS_GL_VIEWPORT_HEIGHT]+
								KS_GL_WINDOW_TITLE_BAR_SIZE-
								KS_GL_WINDOW_FRAME_SIZE+2) &&
		      viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (p->viewport[KS_GL_VIEWPORT_Y]+
								p->viewport[KS_GL_VIEWPORT_HEIGHT]+
								KS_GL_WINDOW_TITLE_BAR_SIZE)){
	      p->flags |= KS_GL_WINDOW_PICK_FRAME_TOP;
	    }
	    */
	  }
	}
      }
    }
    if(gb->pick_window != NULL){
      gb->hits = 0;
      gb->hit_name_num = 0;
      /*
      printf("pick window '%s' MAIN %c BAR %c TOP %c BOTTOM %c LEFT %c RIGHT %c% f\n"
	     ,gb->pick_window->name
	     ,gb->pick_window->flags&KS_GL_WINDOW_PICK_MAIN ? 'o':'x'
	     ,gb->pick_window->flags&KS_GL_WINDOW_PICK_TITLE_BAR ? 'o':'x'
	     ,gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_TOP ? 'o':'x'
	     ,gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_BOTTOM ? 'o':'x'
	     ,gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_LEFT ? 'o':'x'
	     ,gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_RIGHT ? 'o':'x'
	     ,t
	     );
      */
      gb->window = ks_move_end_gl_window(gb->window,gb->pick_window->id);
    }
  }
  return pick_enable;
}
void ks_gl_base_motion(KS_GL_BASE *gb, int x, int y)
{
  int i;
  double scale;
  double len = 10;
  GLint viewport[4];
  double center[2];
  double mvec[2][2];
  int add_x,add_y;

  get_modifires(gb);

  len = 40+(gb->eye_len-gb->trans[0]);

  add_x = x-gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][0];
  add_y = y-gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][1];

#if 1

  {
    KS_GL_WINDOW *win;
    for(win = gb->window; win != NULL; win = win->next){
      if(win->motion != NULL){
	(win->motion)(gb,win->id,
		      win->viewport[KS_GL_VIEWPORT_WIDTH],win->viewport[KS_GL_VIEWPORT_HEIGHT],
		      x-win->viewport[KS_GL_VIEWPORT_X],
		      gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT]-y-win->viewport[KS_GL_VIEWPORT_Y],
		      win->vp);
      }
    }
  }

  if(gb->pick_window != NULL){                                             /* window move mode */
    if(!(gb->pick_window->flags&KS_GL_WINDOW_NO_FRAME)){
      if(gb->pick_window->flags&KS_GL_WINDOW_PICK_TITLE_BAR || 
	 (gb->pick_window->flags&KS_GL_WINDOW_FRAME_FIX && 
	  (gb->pick_window->flags&KS_GL_WINDOW_PICK_TITLE_BAR ||
	   gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_TOP ||
	   gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_BOTTOM ||
	   gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_LEFT ||
	   gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_RIGHT))){
	gb->pick_window->viewport[KS_GL_VIEWPORT_X] += add_x;
	gb->pick_window->viewport[KS_GL_VIEWPORT_Y] -= add_y;
      } else {
	if(gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_TOP && 
	   gb->pick_window->viewport[KS_GL_VIEWPORT_HEIGHT] - add_y > 1){
	  gb->pick_window->viewport[KS_GL_VIEWPORT_HEIGHT] -= add_y;
	}
	if(gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_BOTTOM && 
	   gb->pick_window->viewport[KS_GL_VIEWPORT_HEIGHT]+add_y > 1){
	  gb->pick_window->viewport[KS_GL_VIEWPORT_Y] -=  add_y;
	  gb->pick_window->viewport[KS_GL_VIEWPORT_HEIGHT] += add_y;
	}
	if(gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_LEFT &&
	   gb->pick_window->viewport[KS_GL_VIEWPORT_WIDTH]-add_x > 1){
	  gb->pick_window->viewport[KS_GL_VIEWPORT_X] += add_x;
	  gb->pick_window->viewport[KS_GL_VIEWPORT_WIDTH] -= add_x;
	}
	if(gb->pick_window->flags&KS_GL_WINDOW_PICK_FRAME_RIGHT &&
	   gb->pick_window->viewport[KS_GL_VIEWPORT_WIDTH]+add_x> 1){
	  gb->pick_window->viewport[KS_GL_VIEWPORT_WIDTH] += add_x;
	}
      }
    }
  } else if(gb->move_target_motion == KS_TRUE && gb->move_target != NULL){ /* particle move mode */
    double vec[3], rvec[3];
    scale = len*0.0005*gb->move_target_motion_scale;
    vec[0] = 0;
    vec[1] =  scale*(add_x);
    vec[2] = -scale*(add_y);
    ks_multi_gl_inverse_rotational_matrix(*gb,vec,rvec);
    if(gb->move_target_plane == KS_GL_PLANE_X){
      gb->move_target[1] += rvec[1];
      gb->move_target[2] += rvec[2];
    } else if(gb->move_target_plane == KS_GL_PLANE_Y){
      gb->move_target[0] += rvec[0];
      gb->move_target[2] += rvec[2];
    } else if(gb->move_target_plane == KS_GL_PLANE_Z){
      gb->move_target[0] += rvec[0];
      gb->move_target[1] += rvec[1];
    }
  } else if(gb->mouse_angle_change == KS_TRUE){
    if(gb->mouse_state[KS_GL_MOUSE_BUTTON_LEFT]   == KS_GL_MOUSE_STATE_DOWN && 
       gb->mouse_state[KS_GL_MOUSE_BUTTON_MIDDLE] == KS_GL_MOUSE_STATE_DOWN){
      gb->trans[0] += +(double)(add_y)*len/150;
      gb->angle[0] =  -(double)(add_x)*0.2;
    } else if(gb->mouse_state[KS_GL_MOUSE_BUTTON_MIDDLE] == KS_GL_MOUSE_STATE_DOWN){
      gb->trans[1] += 
	(double)(x-gb->mouse_pos[KS_GL_MOUSE_BUTTON_MIDDLE][KS_GL_MOUSE_STATE_MOTION][0])*len*.001;
      gb->trans[2] -= 
	(double)(y-gb->mouse_pos[KS_GL_MOUSE_BUTTON_MIDDLE][KS_GL_MOUSE_STATE_MOTION][1])*len*.001;
      /*
      printf("%f %f %f %d %d %d %d\n",len,gb->trans[1],gb->trans[2],
	     x,gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][0],
	     y,gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][1]);
      */
    } else if(gb->mouse_state[KS_GL_MOUSE_BUTTON_LEFT] == KS_GL_MOUSE_STATE_DOWN && 
	      gb->mouse_state[KS_GL_MOUSE_BUTTON_RIGHT] == KS_GL_MOUSE_STATE_DOWN){
      gb->trans[1] += (double)(add_x)*len*.001;
      gb->trans[2] -= (double)(add_y)*len*.001;
      /*
      printf("%f %f %f %d %d %d %d\n",len,gb->trans[1],gb->trans[2],
	     x,gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][0],
	     y,gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][1]);
      */
    } else if(gb->mouse_state[KS_GL_MOUSE_BUTTON_RIGHT] == KS_GL_MOUSE_STATE_DOWN){
      if(ks_get_glut_ctrl(gb) == KS_FALSE){
	gb->trans[0] -= 
	  (double)(y-gb->mouse_pos[KS_GL_MOUSE_BUTTON_RIGHT][KS_GL_MOUSE_STATE_MOTION][1])
	  *len*0.0067;
      } else {
	gb->trans[1] += 
	  (double)(x-gb->mouse_pos[KS_GL_MOUSE_BUTTON_RIGHT][KS_GL_MOUSE_STATE_MOTION][0])
	  *len*0.002;
	gb->trans[2] -= 
	  (double)(y-gb->mouse_pos[KS_GL_MOUSE_BUTTON_RIGHT][KS_GL_MOUSE_STATE_MOTION][1])
	  *len*0.002;
	/*      gb->angle[0] =  (double)(x-gb->mpos[0])*0.2;*/
	/*
	glGetIntegerv (GL_VIEWPORT, viewport);
	center[0] = .5*(viewport[2]-viewport[0]);
	center[1] = .5*(viewport[3]-viewport[1]);
	mvec[0][0]= gb->mouse_pos[KS_GL_MOUSE_BUTTON_RIGHT][KS_GL_MOUSE_STATE_MOTION][0]-center[0];
	mvec[0][1]= gb->mouse_pos[KS_GL_MOUSE_BUTTON_RIGHT][KS_GL_MOUSE_STATE_MOTION][1]-center[1];
	mvec[1][0] = x - center[0];
	mvec[1][1] = y - center[1];
	gb->angle[0] = -(mvec[0][0]*mvec[1][1] - mvec[0][1]*mvec[1][0])*.01;
	*/
      }
    } else if(gb->mouse_state[KS_GL_MOUSE_BUTTON_LEFT] == KS_GL_MOUSE_STATE_DOWN){
      if(ks_get_glut_ctrl(gb) == KS_FALSE){
	scale = len/50;
	if(scale > 1.0) scale = 1.0;
	gb->angle[1] = (double)(add_y)*scale;
	gb->angle[2] = (double)(add_x)*scale;
      } else {
	glGetIntegerv (GL_VIEWPORT, viewport);
	center[0] = .5*viewport[2];
	center[1] = .5*viewport[3];
	mvec[0][0]= gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][0]-center[0];
	mvec[0][1]= gb->mouse_pos[KS_GL_MOUSE_BUTTON_LEFT][KS_GL_MOUSE_STATE_MOTION][1]-center[1];
	mvec[1][0] = x - center[0];
	mvec[1][1] = y - center[1];
	gb->angle[0] = -(mvec[0][0]*mvec[1][1] - mvec[0][1]*mvec[1][0])*.01;
      }
    }
  }
  if(gb->mouse_state[KS_GL_MOUSE_BUTTON_LEFT] == KS_GL_MOUSE_STATE_DOWN || 
     gb->mouse_state[KS_GL_MOUSE_BUTTON_MIDDLE] == KS_GL_MOUSE_STATE_DOWN || 
     gb->mouse_state[KS_GL_MOUSE_BUTTON_RIGHT] == KS_GL_MOUSE_STATE_DOWN){
    ks_redraw_gl(gb);
  }
  for(i = 0; i < KS_GL_MOUSE_BUTTON_NUM; i++){
    if(gb->mouse_state[i] == KS_GL_MOUSE_STATE_DOWN){
      gb->mouse_pos[i][KS_GL_MOUSE_STATE_MOTION][0] = x;
      gb->mouse_pos[i][KS_GL_MOUSE_STATE_MOTION][1] = y;
    }
  }
#else
  if(gb->mouse_l == 1 && gb->mouse_m == 1){
    gb->trans[0] += (double)(y-gb->mpos[1])*len/150;
    gb->angle[0] = -(double)(x-gb->mpos[0])*0.2;
  } else  if(gb->mouse_m == 1 || (gb->mouse_l == 1 && gb->mouse_r == 1)){
    gb->trans[1] += (double)(x-gb->mpos[0])*len*.001;
    gb->trans[2] -= (double)(y-gb->mpos[1])*len*.001;
  } else if(gb->mouse_r == 1){
    if(ks_get_glut_ctrl(gb) == KS_FALSE)
      gb->trans[0] -= (double)(y-gb->mpos[1])*len/150;
    else {
      /*      gb->angle[0] =  (double)(x-gb->mpos[0])*0.2;*/
      glGetIntegerv (GL_VIEWPORT, viewport);
      center[0] = .5*viewport[2];
      center[1] = .5*viewport[3];
      mvec[0][0] = gb->mpos[0] - center[0];
      mvec[0][1] = gb->mpos[1] - center[1];
      mvec[1][0] = x - center[0];
      mvec[1][1] = y - center[1];
      gb->angle[0] = -(mvec[0][0]*mvec[1][1] - mvec[0][1]*mvec[1][0])*.01;
    }
  } else if(gb->mouse_l == 1){
    scale = len/50;
    if(scale > 1.0) scale = 1.0;
    gb->angle[1] = (double)(y-gb->mpos[1])*scale;
    gb->angle[2] = (double)(x-gb->mpos[0])*scale;
  }
  if(gb->mouse_l == 1 || gb->mouse_m == 1 || gb->mouse_r == 1){
    gb->mpos[0] = x;
    gb->mpos[1] = y;
    glutPostRedisplay();
  }
#endif
}
void ks_gl_base_passive(KS_GL_BASE *gb, int x, int y)
{
  KS_GL_WINDOW *win;
  int *viewport;
  viewport = gb->viewport_2d;
  for(win = gb->window; win != NULL; win = win->next){
#ifdef GLUT
    if(!(win->flags&KS_GL_WINDOW_FRAME_FIX) && !(win->flags&KS_GL_WINDOW_NO_FRAME) &&
       x >= (win->viewport[KS_GL_VIEWPORT_X]-KS_GL_WINDOW_FRAME_SIZE) &&
       x <= (win->viewport[KS_GL_VIEWPORT_X]+win->viewport[KS_GL_VIEWPORT_WIDTH]+
	     KS_GL_WINDOW_FRAME_SIZE) &&
       viewport[KS_GL_VIEWPORT_HEIGHT]-y >= (win->viewport[KS_GL_VIEWPORT_Y]-
					     KS_GL_WINDOW_FRAME_SIZE) &&
       viewport[KS_GL_VIEWPORT_HEIGHT]-y <= (win->viewport[KS_GL_VIEWPORT_Y]+
					     win->viewport[KS_GL_VIEWPORT_HEIGHT]+
					     KS_GL_WINDOW_TITLE_BAR_SIZE)){
      unsigned int flags;
      flags = check_window_range(viewport,win->viewport,x,y);
      if(flags&KS_GL_WINDOW_PICK_FRAME_TOP && flags&KS_GL_WINDOW_PICK_FRAME_LEFT){
	glutSetCursor(GLUT_CURSOR_TOP_LEFT_CORNER);
      } else if(flags&KS_GL_WINDOW_PICK_FRAME_TOP && flags&KS_GL_WINDOW_PICK_FRAME_RIGHT){
	glutSetCursor(GLUT_CURSOR_TOP_RIGHT_CORNER);
      } else if(flags&KS_GL_WINDOW_PICK_FRAME_BOTTOM && flags&KS_GL_WINDOW_PICK_FRAME_LEFT){
	glutSetCursor(GLUT_CURSOR_BOTTOM_LEFT_CORNER);
      } else if(flags&KS_GL_WINDOW_PICK_FRAME_BOTTOM && flags&KS_GL_WINDOW_PICK_FRAME_RIGHT){
	glutSetCursor(GLUT_CURSOR_BOTTOM_RIGHT_CORNER);
      } else if(flags&KS_GL_WINDOW_PICK_FRAME_TOP || flags&KS_GL_WINDOW_PICK_FRAME_BOTTOM){
	glutSetCursor(GLUT_CURSOR_UP_DOWN);
      } else if(flags&KS_GL_WINDOW_PICK_FRAME_LEFT || flags&KS_GL_WINDOW_PICK_FRAME_RIGHT){
	glutSetCursor(GLUT_CURSOR_LEFT_RIGHT);
      } else {
	glutSetCursor(GLUT_CURSOR_INHERIT);
      }
    } else {
      glutSetCursor(GLUT_CURSOR_INHERIT);
    }
#endif
    if(win->passive != NULL){
      (win->passive)(gb,win->id,
		     win->viewport[KS_GL_VIEWPORT_WIDTH],win->viewport[KS_GL_VIEWPORT_HEIGHT],
		     x-win->viewport[KS_GL_VIEWPORT_X],
		     viewport[KS_GL_VIEWPORT_HEIGHT]-y-win->viewport[KS_GL_VIEWPORT_Y],
		     win->vp);
    }
  }
}
static void process_key_func(KS_GL_KEY_FUNC *kf, int x, int y)
{
  if(kf->state != NULL){
    if(kf->state_add == 0 && 
       kf->state_range[KS_RANGE_MIN] == 0 && kf->state_range[KS_RANGE_MAX] == 0){
      if(*(BOOL*)kf->state == KS_TRUE){
	*(BOOL*)kf->state = KS_FALSE;
      } else {
	*(BOOL*)kf->state = KS_TRUE;
      }
      /*      printf("%d\n",*(BOOL*)kf->state);*/
    } else {
      *(int*)kf->state += kf->state_add;
      if(*(int*)kf->state < kf->state_range[KS_RANGE_MIN]) 
	*(int*)kf->state += kf->state_range[KS_RANGE_MAX]-kf->state_range[KS_RANGE_MIN]+1;
      else if(*kf->state > kf->state_range[KS_RANGE_MAX]) 
	*(int*)kf->state -= kf->state_range[KS_RANGE_MAX]-kf->state_range[KS_RANGE_MIN]+1;
    }
  }
  if(kf->func != NULL){
    kf->func(kf->gb,x,y,kf->vp);
  }
  /*
    printf("%c %d %d %d\n",kf->key,*kf->state
    ,kf->state_range[KS_RANGE_MIN],kf->state_range[KS_RANGE_MAX]);
  */
}
static void call_key_func(unsigned int key, int x, int y, KS_GL_KEY_FUNC *key_func, 
			  unsigned int active_flags)
{
  KS_GL_KEY_FUNC *kf;
  for(kf = key_func; kf != NULL; kf = kf->next){
    if(kf->key == key && ((kf->active_flags==0&&active_flags!=KS_GL_ACTIVE_ALT) || 
			  kf->active_flags&active_flags)){
      /*      printf("%c %d %d\n",kf->key,kf->active_flags,active_flags);*/
      process_key_func(kf,x,y);
    }
  }
}
void ks_gl_call_key_func(KS_GL_BASE *gb, unsigned int key, int x, int y, unsigned int active_flags)
{
  call_key_func(key,x,y,gb->key_func,active_flags);
}
void ks_gl_call_special_key_func(KS_GL_BASE *gb, unsigned int key, int x, int y,
				 unsigned int active_flags)
{
  call_key_func(key,x,y,gb->special_key_func,active_flags);
}
void ks_gl_base_keyboard(KS_GL_BASE *gb, unsigned char key, int x, int y)
{
  int i;
  get_modifires(gb);

  if(gb->input.mode == KS_TRUE){
    /*    printf("key = %x\n",key);*/
    if(gb->input.bool_flg == KS_TRUE){
      if(key == 'y' || key == 'Y' || key == 'n' || key == 'N'){
	if(key == 'y' || key == 'Y'){
	  strcpy(gb->input.text,"y");
	} else {
	  strcpy(gb->input.text,"n");
	}
	gb->input.mode = KS_FALSE;
	gb->input.return_func(gb->input.gb,gb->input.text,gb->input.vp);
	return;
      } else if(key == 0x1b || key == 0x07){ /* ESC or C-g */
	gb->input.mode = KS_FALSE;
	return;
      }
    } else {
      if(key == 0x0d){ /* RET */
	gb->input.mode = KS_FALSE;
	gb->input.return_func(gb->input.gb,gb->input.text,gb->input.vp);
	return;
      } else if(key == 0x1b){ /* ESC */
	gb->input.mode = KS_FALSE;
      } else if(key == 0x08){ /* BS */
	if(gb->input.cursor_pos > 0){
	  for(i = gb->input.cursor_pos-1; gb->input.text[i]; i++){
	    gb->input.text[i] = gb->input.text[i+1];
	  }
	  gb->input.cursor_pos--;
	}
      } else if(key == 0x7f || key == 0x04){ /* DEL */
	for(i = gb->input.cursor_pos; gb->input.text[i]; i++){
	  gb->input.text[i] = gb->input.text[i+1];
	}
      } else if(key == 0x02){ /* \C-b */
	if(gb->input.cursor_pos > 0){
	  gb->input.cursor_pos--;
	}
      } else if(key == 0x06){ /* \C-f */
	if(gb->input.text[gb->input.cursor_pos] != '\0'){
	  gb->input.cursor_pos++;
	}
      } else if(key == 0x01){ /* \C-a */
	gb->input.cursor_pos = 0;
      } else if(key == 0x05){ /* \C-e */
	gb->input.cursor_pos = (int)strlen(gb->input.text);
      } else {
	if(gb->input.cursor_pos < KS_GL_INPUT_KEY_MAX-2){
	  for(i = 0; gb->input.text[i]; i++);
	  for(;i >= gb->input.cursor_pos; i--){
	    gb->input.text[i+1] = gb->input.text[i];
	  }
	  gb->input.text[gb->input.cursor_pos] = key;
	  /*
	    gb->input.text[gb->input.cursor_pos+1] = '\0';
	  */
	  gb->input.cursor_pos++;
	}
      }
    }
    /*    printf("%s %s %d\n",gb->input.message,gb->input.text,gb->input.cursor_pos);*/
    /*
    for(i = 0; i < gb->input.text[i]; i++)
      printf("input %d '%c' %x\n",i,gb->input.text[i],gb->input.text[i]);
    */
  } else {
    /*
    if(key == '\033'|| key == 'q' || key == 'Q'){
      ks_free_gl_base(gb);
      ks_exit(0);
    }
    */
    if(key == '#'){
      GLint viewport[4];
      glGetIntegerv (GL_VIEWPORT, viewport);
      gb->eye_flg = (gb->eye_flg == -1 ? 1:-1);
      ks_gl_base_reshape(gb,viewport[2],viewport[3]);
    }
    if(key == '('){
      gb->eye_width -= 0.1;
    }
    if(key == ')'){
      gb->eye_width += 0.1;
    }
    /*    if(key == 'v' || key == 'V') gb->vflg = gb->vflg == 1 ? 0:gb->vflg+1;*/

    call_key_func(key,x,y,gb->key_func,gb->active_flags);

  }
}
static void draw_help_display_mouse_point(int y, int w, int h, int offset)
{
  GLboolean blend;
  GLfloat color[4];

  glGetFloatv(GL_CURRENT_COLOR,color);

  blend = ks_gl_enable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  glColor4f(0.5,0.5,0.5,0.5);
  glBegin(GL_QUADS);
  glVertex2f(0,y+offset);
  glVertex2f(0,y+h+offset);
  glVertex2f(w,y+h+offset);
  glVertex2f(w,y+offset);
  glEnd();
  glColor4fv(color);

  if(blend == GL_TRUE)
    glDisable(GL_BLEND);
}
static void process_help_display(KS_GL_BASE *gb,int id, int width, int height, void *vp)
{
#ifdef GLUT
  int x,y;
  int n;
  KS_GL_KEY_FUNC *kf;
  int len,len_max;
  char c[8];

  len_max = 45;

  x = 0;
  y = height-gb->font[gb->key_help_font].height;
  ks_use_glut_font(gb,gb->key_help_font);
  for(kf = gb->key_func,n = 0; kf != NULL; kf = kf->next,n++){
    if(n == gb->key_help_point_pos){
      /*      printf("%3d '%c' '%s' %d %d %d\n",kf->key,kf->key,c,len,n,y);*/
      draw_help_display_mouse_point(y,width,gb->font[gb->key_help_font].height,-2);
    }

    if(kf->key > ' ' && kf->key < 127){
      c[0] = (char)kf->key;
      c[1] = '\0';
    } else if(kf->key == ' '){
      strcpy(c,"SP");
    } else if(kf->key == 127){
      strcpy(c,"DEL");
    }
    ks_get_glut_font_width(gb,gb->key_help_font,c,&len);
    glRasterPos2i(x+(len_max-len)*.5,y);
    glCallLists(strlen(c), GL_BYTE, c);
    /*    printf("%3d '%c' '%s' %d %d %d\n",kf->key,kf->key,c,len,n,y);*/
    /*    printf("%d %d %d %s\n",gb->help_font,x,y,kf->comment);*/
    glRasterPos2i(x+KS_GL_KEY_HELP_KEY_LEN,y);
    glCallLists((int)strlen(kf->comment), GL_BYTE, kf->comment);
    y -= gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION;
  }
  for(kf = gb->special_key_func; kf != NULL; kf = kf->next,n++){
    if(n == gb->key_help_point_pos){
      draw_help_display_mouse_point(y,width,gb->font[gb->key_help_font].height,-2);
    }
    strcpy(c,ks_get_gl_key_name(kf->key));
    ks_get_glut_font_width(gb,gb->key_help_font,c,&len);
    glRasterPos2i(x+(len_max-len)*.5,y);
    glCallLists(strlen(c), GL_BYTE, c);
    /*    printf("%3d '%c' '%s' %d\n",kf->key,kf->key,c,len);*/
    /*    printf("%d %d %d %s\n",gb->help_font,x,y,kf->comment);*/
    glRasterPos2i(x+KS_GL_KEY_HELP_KEY_LEN,y);
    glCallLists((int)strlen(kf->comment), GL_BYTE, kf->comment);
    y -= gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION;
  }
#endif  
}
static void process_help_mouse(KS_GL_BASE *gb, int id, int width, int height, int button, 
			       int state, int x, int y, void *vp)
{
  int n;
  KS_GL_KEY_FUNC *kf;
  /*  printf("help mouse %d %d %d %d\n",button,state,x,y);*/
  if(button == KS_GL_MOUSE_BUTTON_LEFT && state == KS_GL_MOUSE_STATE_UP && 
     ks_is_gl_window_range(width,height,x,y) == KS_TRUE){
    for(kf = gb->key_func,n = 0; kf != NULL; kf = kf->next,n++){
      if(n == gb->key_help_point_pos){
	/*	printf("%3d '%c'\n",kf->key,kf->key);*/
	process_key_func(kf,x,y);
      }
    }
    for(kf = gb->special_key_func; kf != NULL; kf = kf->next,n++){
      if(n == gb->key_help_point_pos){
	process_key_func(kf,x,y);
      }
    }
  }
  /*  printf("help mouse %d\n",gb->key_help_select_pos);*/
}
static void process_help_motion(KS_GL_BASE *gb,int id, int width, int height, int x, int y, 
				void *vp)
{
  /*  printf("help motion %d %d\n",x,y);*/
  if(ks_is_gl_window_range(width,height,x,y) == KS_TRUE){
    gb->key_help_point_pos =
      (height-y)/(gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION);
  } else {
    gb->key_help_point_pos = -1;
  }
  /*  printf("help motion %d\n",gb->key_help_select_pos);*/
}
static void process_help_passive(KS_GL_BASE *gb, int id, int width, int height, int x, int y, 
				 void *vp)
{
  if(ks_is_gl_window_range(width,height,x,y) == KS_TRUE){
    /*    printf("help O %d %d\n",x,y);*/
    gb->key_help_point_pos =
      (height-y)/(gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION);
  } else {
    /*    printf("help X %d %d\n",x,y);*/
    gb->key_help_point_pos = -1;
  }
  /*
  printf("%d %d %d %f\n",y,height-y,gb->key_help_point_pos,
	 gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION);
  */
}
BOOL ks_set_gl_base_key_help(KS_GL_BASE *gb, int key, int font)
{
  if(font >= 0 && font < KS_GL_FONT_NUM){
    if(gb->font[font].id == -1){
      ks_error("fonts are not initialized");
      return KS_FALSE;
    }
  } else {
    ks_error("font id %d is not found",font);
    return KS_FALSE;
  }

  gb->key_help_key = key;
  gb->key_help_font = font;

  return KS_TRUE;
}
static void add_help_window(KS_GL_BASE *gb)
{
  int width = 0;
  int height = 0;
  int len;
  KS_GL_KEY_FUNC *kf;

  for(kf = gb->key_func; kf != NULL; kf = kf->next){
    /*    printf("%c : %s %d\n",kf->key,kf->comment,height);*/
#ifdef GLUT
    if(ks_get_glut_font_width(gb,gb->key_help_font,kf->comment,&len) == KS_TRUE){
      if(width < len) width = len;
    }
#else
    ks_assert(0);
#endif
    height += gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION;
  }
  for(kf = gb->special_key_func; kf != NULL; kf = kf->next){
#ifdef GLUT
    if(ks_get_glut_font_width(gb,gb->key_help_font,kf->comment,&len) == KS_TRUE){
      if(width < len) width = len;
    }
#else
    ks_assert(0);
#endif
    height += gb->font[gb->key_help_font].height*KS_GL_KEY_HELP_FONT_HEIGHT_MAGNIFICATION;
  }

  width += KS_GL_KEY_HELP_KEY_LEN+3;
  //  height += 3;
  /*  printf("%d %d %d\n",width,height,gb->viewport[KS_GL_VIEWPORT_HEIGHT]);*/

  gb->key_help_window_id = ks_add_gl_base_window(gb,
						 KS_GL_WINDOW_SHOW|KS_GL_WINDOW_FRAME_FIX,
						 "Key Help",
						 30,
						 gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT]-
						 height-KS_GL_WINDOW_TITLE_BAR_SIZE-30,
						 width,height,
						 process_help_display,
						 process_help_mouse,
						 process_help_motion,
						 process_help_passive,
						 NULL);

}
void ks_gl_base_special_key(KS_GL_BASE *gb, int key, int x, int y)
{
  get_modifires(gb);

  if(gb->input.mode == KS_TRUE){
    if(key == KS_GL_KEY_LEFT){
      if(gb->input.cursor_pos > 0){
	gb->input.cursor_pos--;
      }
    } else if(key == KS_GL_KEY_RIGHT){
      if(gb->input.text[gb->input.cursor_pos] != '\0'){
	gb->input.cursor_pos++;
      }
    }
  } else {
    if(key == gb->key_help_key){
      KS_GL_WINDOW *win;
      /*
      for(kf = gb->key_func; kf != NULL; kf = kf->next){
	printf("%c : %s\n",kf->key,kf->comment);
      }
      */
      if(gb->key_help_window_id < 0){
	add_help_window(gb);
      } else {
	win = ks_lookup_gl_window(gb->window,gb->key_help_window_id);
	if(win != NULL){
	  if(win->flags&KS_GL_WINDOW_SHOW){
	    win->flags &= ~KS_GL_WINDOW_SHOW;
	  } else {
	    win->flags |= KS_GL_WINDOW_SHOW;
	  }
	}
      }
    }
    call_key_func(key,x,y,gb->special_key_func,gb->active_flags);
  }
}
BOOL ks_set_gl_input_key(KS_GL_BASE *gb, char *message, char *before, int font,
			 void (*return_func)(KS_GL_BASE*,char*,void*), BOOL bool_flg, void *vp)
{
  int i;
  
  gb->input.mode = KS_TRUE;
  ks_strncpy(gb->input.message,message,sizeof(gb->input.message));
  if(bool_flg == KS_TRUE){
    if(strlen(gb->input.message) > KS_GL_INPUT_KEY_MAX+10){
      ks_error("input message is too long");
      return KS_FALSE;
    }
    strcat(gb->input.message," (y or n)");
  }
  gb->input.font_id = font;
  gb->input.return_func = return_func;
  for(i = 0; i < KS_GL_INPUT_KEY_MAX; i++)
    gb->input.text[i] = '\0';
  if(before != NULL){
    ks_strncpy(gb->input.text,before,sizeof(gb->input.text));
    gb->input.cursor_pos = (int)strlen(before);
  } else {
    gb->input.cursor_pos = 0;
  }
  gb->input.bool_flg = bool_flg;
  gb->input.gb = gb;
  gb->input.vp = vp;
  return KS_TRUE;
}
#ifdef GLUT
BOOL ks_draw_glut_text(KS_GL_BASE *gb, char *text, int font, GLfloat transparency, int margin)
{
  int i;
  char *cp;
  GLfloat pos[2][4];
  GLfloat color[4];
  float text_width;
  GLboolean blend;

  blend = ks_gl_enable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  ks_use_glut_font(gb,font);
  glGetFloatv(GL_CURRENT_RASTER_POSITION,pos[0]);
  glGetFloatv(GL_CURRENT_COLOR,color);
  pos[0][0] -= margin;
  pos[0][1] -= margin;

  text_width = 0;
  for(cp = text; *cp; cp++)
    text_width += glutBitmapWidth(gb->font[font].type,*cp);

  for(i = 0; i < 4; i++)
    pos[1][i] = pos[0][i];
  pos[1][0] += text_width+margin;
  pos[1][1] += gb->font[font].height+margin;
  glColor4f(gb->background_color[0],gb->background_color[1],gb->background_color[2],transparency);
  glBegin(GL_QUADS);
  glVertex4f(pos[0][0],pos[0][1],pos[0][2],pos[0][3]);
  glVertex4f(pos[0][0],pos[1][1],pos[0][2],pos[0][3]);
  glVertex4f(pos[1][0],pos[1][1],pos[0][2],pos[0][3]);
  glVertex4f(pos[1][0],pos[0][1],pos[0][2],pos[0][3]);
  glEnd();
  if(blend == GL_TRUE)
    glDisable(GL_BLEND);
  glCallLists(strlen(text), GL_BYTE, text);
  glColor4fv(color);
  return KS_TRUE;
}
void ks_draw_glut_input_key_text(KS_GL_BASE *gb, GLfloat transparency)
{
  if(gb->input.mode == KS_TRUE){
    int i;
    char *p;
    int message_len = 0;
    int cursor_pos = 0;
    GLfloat pos[4],color[4];
    char output[KS_GL_INPUT_KEY_MAX*2];

    /*    printf("draw_glut_input_key_text\n");*/
    glGetFloatv(GL_CURRENT_RASTER_POSITION,pos);
    glGetFloatv(GL_CURRENT_COLOR,color);
    ks_use_glut_font(gb,gb->input.font_id);
    strcpy(output,gb->input.message);
    strcat(output,gb->input.text);
    ks_draw_glut_text(gb,output,gb->input.font_id,transparency,1);
    /*    printf("%f %f %f %f\n",pos[0],pos[1],pos[2],pos[3]);*/
    for(p = gb->input.message; *p; p++){
      message_len += glutBitmapWidth(gb->font[gb->input.font_id].type,*p);
    }
    for(i = 0,p = gb->input.text; i < gb->input.cursor_pos; i++,p++){
      cursor_pos += glutBitmapWidth(gb->font[gb->input.font_id].type,*p);
    }
    pos[0] += message_len+cursor_pos;
    pos[1] -= 4;
    /*
    glRasterPos4fv(pos);
    glCallLists(strlen("_"), GL_BYTE, "_");
    printf("%d %x '%c'\n"
	   ,gb->input.cursor_pos
	   ,gb->input.text[gb->input.cursor_pos]
	   ,gb->input.text[gb->input.cursor_pos]);
    */
    glColor4fv(color);
    p = &gb->input.text[gb->input.cursor_pos];
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex4fv(pos);
    if(*p == '\0'){
      pos[0] += glutBitmapWidth(gb->font[gb->input.font_id].type,'_');
    } else {
      pos[0] += glutBitmapWidth(gb->font[gb->input.font_id].type,*p);
    }
    glVertex4fv(pos);
    glEnd();
  }
}
#endif
BOOL ks_is_gl_input_key_mode(KS_GL_BASE *gb)
{
  return gb->input.mode;
}
BOOL ks_get_gl_amino_color(int i, GLfloat *color)
{
  int j;

  /*  i -= KS_GL_ATOM_COLOR_AMINO;*/
  if(i >= 0 && i < 21){
    for(j = 0; j < 3; j++)
      color[j] = ks_amino[i].color[j];
  } else {
    /*    ks_error("Illegal amino range %d",i);*/
    return KS_FALSE;
  }

  return KS_TRUE;
}
BOOL ks_get_gl_charge_color(int i, GLfloat *color)
{
  /*  i -= KS_GL_ATOM_COLOR_CHARGE;*/
  if(i >= 0 && i < 5){
    color[0] = 1.0;
    color[1] = (float)i/5;
    color[2] = (float)i/5;
  } else if(i == 5){
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
  } else if(i > 5 && i < 11){
    color[0] = 1.0-(float)(i-5)/5;
    color[1] = 1.0-(float)(i-5)/5;
    color[2] = 1.0;
  } else {
    ks_error("Illegal color range %d",i);
    return KS_FALSE;
  }
  return KS_TRUE;
}
BOOL ks_get_gl_unknown_color(int i, GLfloat *color)
{
  float unknown_color[6][3] = { {0.0,0.5,0.5},
				{0.5,0.0,0.5},
				{0.5,0.5,0.0},
				{0.5,0.0,0.0},
				{0.0,0.5,0.0},
				{0.0,0.0,0.5}};

  /*  i -= KS_GL_ATOM_COLOR_UNKNOWN;*/
  if(i >= 0 && i <= 5){
    color[0] = unknown_color[i][0];
    color[1] = unknown_color[i][1];
    color[2] = unknown_color[i][2];
    /*
    color[0] = 0.25;
    color[1] = 0.25;
    color[2] = 0.25;
    color[0] = 0.90;
    color[1] = 0.80;
    color[2] = 0.60;
    */
  } else {
    ks_error("Illegal color range %d",i);
    return KS_FALSE;
  }
  return KS_TRUE;
}
KS_GL_ATOM *ks_allocate_gl_atom(float radius, float *color, int detail, float *scale, 
				BOOL use_display_list)
{
  int i,j,k,l;
  int i0;
  float r;
  int detail_add;
  GLfloat col[2][4],em[4] = {0.0,0.0,0.0,1.0};
  KS_GL_ATOM *gla;

  if(radius < 0 || color == NULL){
    ks_error("Illegal parameter");
    return NULL;
  }

  if((gla = (KS_GL_ATOM*)ks_malloc(sizeof(KS_GL_ATOM),"gla")) == NULL){
    ks_error_memory();
    return NULL;
  }

  if(use_display_list == KS_TRUE){
    if((gla->atom = (GLuint****)ks_malloc(KS_GL_ATOM_COLOR_NUM*sizeof(GLuint***),"gla->atom"))
       == NULL){
      ks_error_memory();
      return NULL;
    }
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
      if((gla->atom[i] = (GLuint***)ks_malloc(KS_GL_ATOM_SIZE_NUM*sizeof(GLuint**),"gla->atom[i]"))
	 == NULL){
	ks_error_memory();
	return NULL;
      }
    }
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
      for(j = 0; j < KS_GL_ATOM_SIZE_NUM; j++){
	if((gla->atom[i][j] = (GLuint**)ks_malloc(KS_GL_ATOM_BRIGHT_NUM*sizeof(GLuint*),
						  "gl->atom[i][j]")) == NULL){
	  ks_error_memory();
	  return NULL;
	}
      }
    }
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
      for(j = 0; j < KS_GL_ATOM_SIZE_NUM; j++){
	for(k = 0; k < KS_GL_ATOM_BRIGHT_NUM; k++){
	  /*	printf("%d %d %d %d\n",i,j,k,KS_GL_ATOM_TRANS_NUM);*/
	  if((gla->atom[i][j][k]=(GLuint*)ks_malloc(KS_GL_ATOM_TRANS_NUM*sizeof(GLuint),
						    "gla->atom[i][j][k]")) == NULL){
	    ks_error_memory();
	    return NULL;
	  }
	}
      }
    }
  } else {
    gla->atom = NULL;
  }

  if((gla->radius = (float*)ks_malloc(KS_GL_ATOM_SIZE_NUM*sizeof(float),"gla->radius")) == NULL){
    ks_error_memory();
    return NULL;
  }
  if((gla->color =(GLfloat***)ks_malloc(KS_GL_ATOM_COLOR_NUM*sizeof(GLfloat**),"gla->color"))
     ==NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
    if((gla->color[i] = (GLfloat**)ks_malloc(KS_GL_ATOM_BRIGHT_NUM*sizeof(GLfloat*),
					     "gla->color[i]")) == NULL){
      ks_error_memory();
      return NULL;
    }
  }
  for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
    for(j = 0; j < KS_GL_ATOM_BRIGHT_NUM; j++){
      if((gla->color[i][j] = (GLfloat*)ks_malloc(3*sizeof(GLfloat),"gla->color[i][j]")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
  }
  /*  printf("gl scale %p\n",scale);*/
  if(scale != NULL){
    if((gla->scale = ks_malloc_float_p(3,"gla->scale")) == NULL){
      ks_error_memory();
      return NULL;
    }
    for(i = 0; i < 3; i++){
      gla->scale[i] = scale[i];
    }
  } else {
    gla->scale = NULL;
  }

  if((gla->detail = ks_malloc_int_p(KS_GL_ATOM_SIZE_NUM,"gla->detail")) == NULL){
    ks_error_memory();
    return NULL;
  }
  if((gla->emission =(GLfloat**)ks_malloc(KS_GL_ATOM_BRIGHT_NUM*sizeof(GLfloat*),"gla->emission"))
     ==NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < KS_GL_ATOM_BRIGHT_NUM; i++){
    if((gla->emission[i] = (GLfloat*)ks_malloc(4*sizeof(GLfloat),"gla->emission[i]")) == NULL){
      ks_error_memory();
      return NULL;
    }
  }

  for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++){
    for(j = 0; j < KS_GL_ATOM_SIZE_NUM; j++){
      for(k = 0; k < KS_GL_ATOM_BRIGHT_NUM; k++){
	for(l = 0; l < KS_GL_ATOM_TRANS_NUM; l++){
	  if(use_display_list == KS_TRUE){
	    gla->atom[i][j][k][l] = glGenLists(1);
	    glNewList(gla->atom[i][j][k][l],GL_COMPILE);
	  }
	  if(i < KS_GL_ATOM_COLOR_ATOM + KS_GL_ATOM_COLOR_ATOM_NUM){
	    for(i0 = 0; i0 < 3; i0++) col[0][i0] = color[i0];
	  } else if(i < KS_GL_ATOM_COLOR_AMINO + KS_GL_ATOM_COLOR_AMINO_NUM){
	    /*
	    for(i0 = 0; i0 < 3; i0++) col[0][i0] = amino_color[i-KS_GL_ATOM_COLOR_ATOM_END][i0];
	    */
	    if(ks_get_gl_amino_color(i-KS_GL_ATOM_COLOR_AMINO,col[0]) == KS_FALSE){
	      return NULL;
	    }
	  } else if(i < KS_GL_ATOM_COLOR_CHARGE + KS_GL_ATOM_COLOR_CHARGE_NUM){
	    /*
	    for(i0 = 0; i0 < 3; i0++) col[0][i0] = charge_color[i-KS_GL_ATOM_COLOR_AMINO_END][i0];
	    */
	    if(ks_get_gl_charge_color(i-KS_GL_ATOM_COLOR_CHARGE,col[0]) == KS_FALSE){
	      return NULL;
	    }
	  } else if(i < KS_GL_ATOM_COLOR_UNKNOWN + KS_GL_ATOM_COLOR_UNKNOWN_NUM){
	    if(ks_get_gl_unknown_color(i-KS_GL_ATOM_COLOR_UNKNOWN,col[0]) == KS_FALSE){
	      return NULL;
	    }
	  }
	  if(k == KS_GL_ATOM_BRIGHT_NORMAL){
	    for(i0 = 0; i0 < 3; i0++) col[1][i0] = col[0][i0];
	    for(i0 = 0; i0 < 3; i0++) em[i0] = 0.0;
	  } else if(k == KS_GL_ATOM_BRIGHT_LOW){
	    for(i0 = 0; i0 < 3; i0++) col[1][i0] = col[0][i0]*.5;
	    for(i0 = 0; i0 < 3; i0++) em[i0] = 0.0;
	  } else if(k == KS_GL_ATOM_BRIGHT_HIGH){
	    for(i0 = 0; i0 < 3; i0++) col[1][i0] = col[0][i0];
	    for(i0 = 0; i0 < 3; i0++) em[i0] = KS_GL_SELECTED_EMISSION;
	  }
	  if(l == KS_GL_ATOM_TRANS_OFF){
	    col[1][3] = 1.0;
	  } if(l == KS_GL_ATOM_TRANS_NORMAL){
	    col[1][3] = 0.5;
	  }
	  if(j == KS_GL_ATOM_SIZE_NORMAL){
	    r = radius*.5;
	    detail_add = 0;
	  } else if(j == KS_GL_ATOM_SIZE_SMALL){
	    r = radius*.25;
	    detail_add = -1;
	  } else if(j == KS_GL_ATOM_SIZE_LARGE){
	    r = radius;
	    detail_add = detail;
	  }

	  gla->radius[j] = r;
	  gla->detail[j] = detail+detail_add;
	  for(i0 = 0; i0 < 3; i0++) gla->color[i][k][i0] = col[1][i0];
	  for(i0 = 0; i0 < 4; i0++) gla->emission[k][i0] = em[i0];

	  if(use_display_list == KS_TRUE){
	    GLboolean normalize;
	    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,col[1]);
	    glMaterialfv(GL_FRONT,GL_EMISSION,em);
	    if(scale != NULL){
	      /*	      printf("gl scale %f %f %f\n",scale[0],scale[1],scale[2]);*/
	      glScalef(scale[0],scale[1],scale[2]);
	      normalize = ks_gl_enable(GL_NORMALIZE);
	    }
#ifdef GLUT
	    glutSolidSphere(r,(detail+detail_add)*2,detail+detail_add);
#else
	    ks_draw_gl_sphere(r,detail+detail_add);
#endif
	    if(scale != NULL && normalize == GL_TRUE){
	      glDisable(GL_NORMALIZE);
	    }
	    glEndList();
	  }
	}
      }
    }
  }
  return gla;
}
void ks_free_gl_atom(KS_GL_ATOM *gla)
{
  int i,j,k;

  if(gla->atom != NULL){
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++)
      for(j = 0; j < KS_GL_ATOM_SIZE_NUM; j++)
	for(k = 0; k < KS_GL_ATOM_BRIGHT_NUM; k++)
	  ks_free(gla->atom[i][j][k]);
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++)
      for(j = 0; j < KS_GL_ATOM_SIZE_NUM; j++)
	ks_free(gla->atom[i][j]);
    for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++)
      ks_free(gla->atom[i]);
    ks_free(gla->atom);
  }

  for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++)
    for(j = 0; j < KS_GL_ATOM_BRIGHT_NUM; j++)
      ks_free(gla->color[i][j]);
  for(i = 0; i < KS_GL_ATOM_COLOR_NUM; i++)
    ks_free(gla->color[i]);
  ks_free(gla->color);
  for(i = 0; i < KS_GL_ATOM_BRIGHT_NUM; i++)
    ks_free(gla->emission[i]);
  ks_free(gla->emission);
  ks_free(gla->radius);
  if(gla->scale != NULL)
    ks_free(gla->scale);
  ks_free(gla->detail);
  ks_free(gla);
}
void ks_draw_gl_atom(KS_GL_ATOM *gla, int color, int size, int bright, int trans)
{
  if(gla->atom != NULL){
    glCallList(gla->atom[color][size][bright][trans]);
  } else {
    int i;
    GLfloat col[4];
    for(i = 0; i < 3; i++){
      col[i] = gla->color[color][bright][i];
    }
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE,col);
    glMaterialfv(GL_FRONT,GL_EMISSION,gla->emission[bright]);
#ifdef GLUT
    glutSolidSphere(gla->radius[size],gla->detail[size]*2,gla->detail[size]);
#else
    ks_draw_gl_sphere(gla->radius[size],gla->detail[size]);
#endif
  }
}
GLfloat *ks_get_gl_atom_color(KS_GL_ATOM *gla, int color, int bright)
{
  return gla->color[color][bright];
}
GLfloat *ks_get_gl_atom_emission(KS_GL_ATOM *gla, int bright)
{
  return gla->emission[bright];
}
float ks_get_gl_atom_size(KS_GL_ATOM *gla, int size)
{
  return gla->radius[size];
}
float ks_get_gl_atom_detail(KS_GL_ATOM *gla, int size)
{
  return gla->detail[size];
}
KS_GL_GRAPH_2D *ks_allocate_gl_graph_2d(char *name, int size, double min, double max, 
					double scale_unit, 
					GLfloat red, GLfloat green, GLfloat blue,
					GLfloat dot_size,
					GLfloat dot_red, GLfloat dot_green, GLfloat dot_blue,
					int font, unsigned int flags)
{
  int i;
  KS_GL_GRAPH_2D *gra;

  if((gra = (KS_GL_GRAPH_2D*)ks_malloc(sizeof(KS_GL_GRAPH_2D),"ks_allocate_gl_graph_2d")) == NULL){
    ks_error("ks_allocate_gl_graph_2d: memory error\n");
    return NULL;
  }
  if((gra->data = ks_malloc_double_p(size,"ks_allocate_gl_graph_2d")) == NULL){
    return NULL;
  }
  gra->size = size;
  gra->num = 0;
  gra->flags = flags;
  for(i = 0; i < 24 && name[i]; i++)
    gra->name[i] = name[i];
  gra->name[i] = '\0';
  gra->font_id = font;
  gra->current = 0;
  gra->range[KS_RANGE_MIN] = min;
  gra->range[KS_RANGE_MAX] = max;
  gra->scale_unit = scale_unit;
  gra->color[0] = red;
  gra->color[1] = green;
  gra->color[2] = blue;
  gra->dot_size = dot_size;
  gra->dot_color[0] = dot_red;
  gra->dot_color[1] = dot_green;
  gra->dot_color[2] = dot_blue;
  return gra;
}
void ks_free_gl_graph_2d(KS_GL_GRAPH_2D *gra)
{
  ks_free(gra->data);
  ks_free(gra);
}
void ks_set_gl_graph_2d(KS_GL_GRAPH_2D *gra, double data)
{
  gra->data[gra->current] = data;
  if(gra->current == gra->size-1)
    gra->current = 0;
  else
    gra->current++;
  if(gra->num < gra->size){
    gra->num++;
  }
}
static int set_dec_len(double x)
{
  int keta;
  keta = log10(fabs(x))+1;
  if(keta <= 1){
    return 2;
  } else if(keta == 2){
    return 1;
  }
  return 0;
}
#ifdef GLUT
void ks_draw_gl_graph_2d(KS_GL_BASE *gb, 
			 KS_GL_GRAPH_2D *gra, int x, int y, int width, int height, 
			 double line_value, int dot_pos)
{
  int i,i0;
  double data_range[2];
  char c0[256];
  int text_width;

  if(gra->flags&KS_GL_GRAPH_AUTO && gra->num > 1){
    data_range[KS_RANGE_MIN] = gra->data[0];
    data_range[KS_RANGE_MAX] = gra->data[0];
    for(i = 1; i < gra->num; i++){
      /*      printf("%d %f\n",i,gra->data[i]);*/
      if(data_range[KS_RANGE_MIN] > gra->data[i]) data_range[KS_RANGE_MIN] = gra->data[i];
      if(data_range[KS_RANGE_MAX] < gra->data[i]) data_range[KS_RANGE_MAX] = gra->data[i];
    }
    /*    printf("%f %f\n",data_range[KS_RANGE_MIN],data_range[KS_RANGE_MAX]);*/
    if(data_range[KS_RANGE_MIN] < gra->range[KS_RANGE_MIN] ||
       data_range[KS_RANGE_MAX] > gra->range[KS_RANGE_MAX] ||
       (data_range[KS_RANGE_MAX]-data_range[KS_RANGE_MIN]) <
       (gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*.5
       ){
      gra->range[KS_RANGE_MIN] = (int)(data_range[KS_RANGE_MIN]/gra->scale_unit-1)*gra->scale_unit;
      gra->range[KS_RANGE_MAX] = (int)(data_range[KS_RANGE_MAX]/gra->scale_unit+2)*gra->scale_unit;
    }
  }
  /*  printf("%f %f\n",gra->range[KS_RANGE_MIN],gra->range[KS_RANGE_MAX]);*/
  glLineWidth(1.0);
  ks_gl_base_foreground_color(gb);
  glBegin(GL_LINE_LOOP);
  glVertex2i(x,y);
  glVertex2i(x+width,y);
  glVertex2i(x+width,y+height);
  glVertex2i(x,y+height);
  glEnd();

  ks_use_glut_font(gb,gra->font_id);

  if(gra->flags&KS_GL_GRAPH_DRAW_SCALE){
    sprintf(c0,"%.*f",set_dec_len(gra->range[KS_RANGE_MIN]),gra->range[KS_RANGE_MIN]);
    ks_get_glut_font_width(gb,gra->font_id,c0,&text_width);
    glRasterPos2i(x-text_width-5,y-gb->font[gra->font_id].height*.5);
    glCallLists((int)strlen(c0), GL_BYTE, c0);

    sprintf(c0,"%.*f",set_dec_len(gra->range[KS_RANGE_MAX]),gra->range[KS_RANGE_MAX]);
    ks_get_glut_font_width(gb,gra->font_id,c0,&text_width);
    glRasterPos2i(x-text_width-5,height+y-gb->font[gra->font_id].height*.5);
    glCallLists((int)strlen(c0), GL_BYTE, c0);
  }

  if(gra->flags&KS_GL_GRAPH_DRAW_LINE){
    sprintf(c0,"%.*f",set_dec_len(line_value),line_value);
    ks_get_glut_font_width(gb,gra->font_id,c0,&text_width);
    glRasterPos2i(x-text_width-5,
		  (int)((line_value-gra->range[KS_RANGE_MIN])/
			(gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*height)+y
		  -gb->font[gra->font_id].height*.5);
    glCallLists((int)strlen(c0), GL_BYTE, c0);

    glBegin(GL_LINES);
    glVertex2i(x,(int)((line_value-gra->range[KS_RANGE_MIN])/
		       (gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*height)+y);
    glVertex2i(width+x,(int)((line_value-gra->range[KS_RANGE_MIN])/
			     (gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*height)+y);
    glEnd();
  }

  glColor4f(gra->color[0],gra->color[1],gra->color[2],1.0);
  /*  printf("%d\n",gra->num);*/
  glBegin(GL_LINE_STRIP);
  for(i0 = 0; i0 < gra->num; i0++){
    if(gra->num == gra->size){
      i = gra->current+i0;
      if(i >= gra->size) i -= gra->size;
    } else {
      i = i0;
    }
    /*
    printf("%d %d %d %d %d %f %d %d \n",i0,i,gra->current,gra->num,gra->size,gra->data[i]
	   ,i0+x,(int)((gra->data[i]-gra->range[KS_RANGE_MIN])/
		      (gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*height)+y);
    */
    glVertex2i(i0*width/gra->size+x,
	       (int)((gra->data[i]-gra->range[KS_RANGE_MIN])/
		     (gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*height)+y);
  }
  glEnd();

  if(dot_pos >= 0 && dot_pos < gra->num){ 
    glColor4f(gra->dot_color[0],gra->dot_color[1],gra->dot_color[2],1.0);
    glPointSize(gra->dot_size);
    glBegin(GL_POINTS);
    glVertex2i(dot_pos*width/gra->size+x,
	       (int)((gra->data[dot_pos]-gra->range[KS_RANGE_MIN])/
		     (gra->range[KS_RANGE_MAX]-gra->range[KS_RANGE_MIN])*height)+y);
    glEnd();
  }
}
#endif
KS_GL_BUFFER_3D *ks_allocate_gl_buffer_3d(int num, int grow, int size, unsigned int flags, 
					  char *msg)
{
  int i;
  KS_GL_BUFFER_3D *buf;
  if((buf = (KS_GL_BUFFER_3D*)ks_malloc(sizeof(KS_GL_BUFFER_3D),msg)) == NULL){
    ks_error_memory();
    return NULL;
  }
  buf->num = num;
  buf->grow = grow;
  buf->size = size;
  if(flags&KS_GL_BUFFER_3D_DEFAULT){
    if((buf->x = ks_malloc_double_p(buf->size,"x")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->y = ks_malloc_double_p(buf->size,"y")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->z = ks_malloc_double_p(buf->size,"z")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->label = ks_malloc_int_p(buf->size,"label")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->flags = ks_malloc_uint_p(buf->size,"flags")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->color = (GLfloat**)ks_malloc(buf->size*sizeof(GLfloat*),"color")) == NULL){
      ks_error_memory();
      return NULL;
    }
    for(i = 0; i < (int)buf->size; i++){
      if((buf->color[i] = (GLfloat*)ks_malloc(4*sizeof(GLfloat),"color[i]")) == NULL){
	ks_error_memory();
	return NULL;
      }
      buf->color[i][0] = 0;
      buf->color[i][1] = 0;
      buf->color[i][2] = 0;
      buf->color[i][3] = 1.0;
    }
  } else {
    buf->x = NULL;
    buf->y = NULL;
    buf->z = NULL;
    buf->label = NULL;
    buf->flags = NULL;
    buf->color = NULL;
  }
  if(flags&KS_GL_BUFFER_3D_NORMAL){
    if((buf->nx = ks_malloc_double_p(buf->size,"nx")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->ny = ks_malloc_double_p(buf->size,"ny")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->nz = ks_malloc_double_p(buf->size,"nz")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    buf->nx = NULL;
    buf->ny = NULL;
    buf->nz = NULL;
  }
  if(flags&KS_GL_BUFFER_3D_SPLINE){
    if((buf->d = ks_malloc_double_p(buf->size,"d")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->h = ks_malloc_double_p(buf->size,"h")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->p = ks_malloc_double_p(buf->size,"p")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->a = ks_malloc_double_p(buf->size,"a")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->b = ks_malloc_double_p(buf->size,"b")) == NULL){
      ks_error_memory();
      return NULL;
    }
    if((buf->c = ks_malloc_double_p(buf->size,"c")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    buf->d = NULL;
    buf->h = NULL;
    buf->p = NULL;
    buf->a = NULL;
    buf->b = NULL;
    buf->c = NULL;
  }
  return buf;
}
KS_GL_BUFFER_3D *ks_allocate_spline_buffer_3d(void)
{
  KS_GL_BUFFER_3D *spline;

  if((spline = ks_allocate_gl_buffer_3d(0,2,1,
					KS_GL_BUFFER_3D_DEFAULT|
					KS_GL_BUFFER_3D_SPLINE,"spline")) == NULL){
    return NULL;
  }
  return spline;
}
BOOL ks_realloc_gl_buffer_3d(KS_GL_BUFFER_3D *buf)
{
  size_t new_size;
  
  new_size = buf->num*buf->grow;

  if(buf->x != NULL){
    if((buf->x = ks_realloc_double_p(buf->x,new_size,"x")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->y != NULL){
    if((buf->y = ks_realloc_double_p(buf->y,new_size,"y")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->z != NULL){
    if((buf->z = ks_realloc_double_p(buf->z,new_size,"z")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->nx != NULL){
    if((buf->nx = ks_realloc_double_p(buf->nx,new_size,"nx")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->ny != NULL){
    if((buf->ny = ks_realloc_double_p(buf->ny,new_size,"ny")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->nz != NULL){
    if((buf->nz = ks_realloc_double_p(buf->nz,new_size,"nz")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->d != NULL){
    if((buf->d = ks_realloc_double_p(buf->d,new_size,"d")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->h != NULL){
    if((buf->h = ks_realloc_double_p(buf->h,new_size,"h")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->p != NULL){
    if((buf->p = ks_realloc_double_p(buf->p,new_size,"p")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->a != NULL){
    if((buf->a = ks_realloc_double_p(buf->a,new_size,"a")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->b != NULL){
    if((buf->b = ks_realloc_double_p(buf->b,new_size,"b")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->c != NULL){
    if((buf->c = ks_realloc_double_p(buf->c,new_size,"c")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->label != NULL){
    if((buf->label = ks_realloc_int_p(buf->label,new_size,"label")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->flags != NULL){
    if((buf->flags = ks_realloc_uint_p(buf->flags,new_size,"flags")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  if(buf->color != NULL){
    int i,j;
    GLfloat **color;
    if((color = (GLfloat**)ks_malloc(new_size*sizeof(GLfloat*),"color")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < (int)new_size; i++)
      if((color[i] = (GLfloat*)ks_malloc(4*sizeof(GLfloat),"color[i]")) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
    for(i = 0; i < (int)buf->size; i++){
      for(j = 0; j < 4; j++){
	color[i][j] = buf->color[i][j];
      }
      ks_free(buf->color[i]);
    }
    ks_free(buf->color);
    buf->color = color;
  }
  buf->size = new_size;
  return KS_TRUE;
}
void ks_free_gl_buffer_3d(KS_GL_BUFFER_3D *buf)
{
  int i;
  if(buf != NULL){
    if(buf->x != NULL) ks_free(buf->x);
    if(buf->y != NULL) ks_free(buf->y);
    if(buf->z != NULL) ks_free(buf->z);
    if(buf->nx != NULL) ks_free(buf->nx);
    if(buf->ny != NULL) ks_free(buf->ny);
    if(buf->nz != NULL) ks_free(buf->nz);
    if(buf->h != NULL) ks_free(buf->h);
    if(buf->d != NULL) ks_free(buf->d);
    if(buf->p != NULL) ks_free(buf->p);
    if(buf->a != NULL) ks_free(buf->a);
    if(buf->b != NULL) ks_free(buf->b);
    if(buf->c != NULL) ks_free(buf->c);
    if(buf->label != NULL) ks_free(buf->label);
    if(buf->flags != NULL) ks_free(buf->flags);
    if(buf->color != NULL){
      for(i = 0; i < (int)buf->size; i++)
	ks_free(buf->color[i]);
      ks_free(buf->color);
    }
    ks_free(buf);
  }
}
BOOL ks_set_spline_buffer_3d(KS_GL_BUFFER_3D *spline,double x, double y, double z,
			     GLfloat *color, int label)
{
  return ks_set_gl_buffer_3d(spline,x,y,z,0,0,0,color,label,0);
}
BOOL ks_set_gl_buffer_3d(KS_GL_BUFFER_3D *buf, 
			 double x, double y, double z, double nx, double ny, double nz,
			 GLfloat *color, int label, unsigned int flags)
{
  int i;
  if(buf->num >= buf->size){
    ks_realloc_gl_buffer_3d(buf);
  }
  buf->x[buf->num] = x;
  buf->y[buf->num] = y;
  buf->z[buf->num] = z;
  if(buf->nx != NULL) buf->nx[buf->num] = nx;
  if(buf->ny != NULL) buf->ny[buf->num] = ny;
  if(buf->nz != NULL) buf->nz[buf->num] = nz;

  if(color != NULL){
    for(i = 0; i < 4; i++)
      buf->color[buf->num][i] = color[i];
  }
  buf->label[buf->num] = label;
  buf->flags[buf->num] = flags;
  buf->num++;
  return KS_TRUE;
}
void ks_clear_gl_buffer_3d(KS_GL_BUFFER_3D *buf)
{
  buf->num = 0;
}
static void make_spline_table(double *x,double *y,double *z, double *h, double *d,int datan)
{
  int	i;
  double t;

  z[0] = 0;
  z[ datan - 1] = 0;
  for(i = 0;i < datan-1;i++){
    h[i    ] =  x[i + 1] - x[i];
    d[i + 1] = (y[i + 1] - y[i])/h[i];
    /*    printf("%d %f %f %f %f\n",i,x[i],x[i+1],h[i],d[i+1]);*/
  }
  /*  ks_exit(EXIT_FAILURE);*/
  z[1] = d[2] - d[1] - h[0] *z[0];
  d[1] = 2*(x[2] - x[0]);
  for(i = 1;i < datan - 2;i++){
    t = h[i]/d[i];
    z[i + 1] = d[i + 2] - d[i + 1] - z[i]*t;
    d[i + 1] = 2*(x[i + 2] - x[i]) - h[i]*t;
    /*    printf("%d %f %f %f %f %f %f %f\n",i,t,h[i],d[i],d[i+1],d[i+2],z[i],z[i+1]);*/
  }
  z[datan - 2] -= h[datan - 2]*z[datan - 1];
  /*
  printf("%f %f %f\n",z[datan - 2],h[datan - 2],z[datan - 1]);
  ks_exit(EXIT_FAILURE);
  */
  for(i = datan - 2;i > 0;i--){
    /*    printf("%d %f %f %f %f\n",i,z[i],z[i+1],h[i],d[i]);*/
    z[i] = (z[i] - h[i]*z[i + 1])/d[i];
  }
}
static void make_spline_table3(KS_GL_BUFFER_3D *sp)
{
  int i;
  double t1,t2,t3;

  sp->p[0] = 0;
  for(i = 1; i < (int)sp->num; i++){
    t1 = sp->x[i] - sp->x[i-1];
    t2 = sp->y[i] - sp->y[i-1];
    t3 = sp->z[i] - sp->z[i-1];
    sp->p[i] = sp->p[i-1] + sqrt(t1*t1 + t2*t2 + t3*t3);
    /*    printf("%d %f %f %f %f\n",i,sp->p[i],sp->x[i],sp->y[i],sp->z[i]);*/
  }
  /*  ks_exit(EXIT_FAILURE);*/
  for(i = 1;i < (int)sp->num; i++){
    sp->p[i] /= sp->p[sp->num-1];
    /*    printf("%d %f %f\n",i,sp->p[i],sp->p[i]-sp->p[i-1]);*/
  }

  make_spline_table(sp->p,sp->x,sp->a,sp->h,sp->d,(int)sp->num);
  make_spline_table(sp->p,sp->y,sp->b,sp->h,sp->d,(int)sp->num);
  make_spline_table(sp->p,sp->z,sp->c,sp->h,sp->d,(int)sp->num);
}
static double __KS_USED__ spline(double t,double *x,double *y,double *z, int *pi,int datan)
{
  int	i,j,k;
  double	d,h;

  i = 0;
  j = datan - 1;
  while(i < j){
    k = (i + j)/2;
    if (x[k] < t)
      i = k + 1;
    else
      j = k;
  }
  if(i > 0) i--;
  h = x[i + 1] - x[i];
  d = t - x[i];
  *pi = x[i+1]-t < t-x[i] ? i+1:i;
  return(((z[i + 1] - z[i])*d/h + z[i]*3)*d
	 + ((y[i + 1] - y[i])/h
	    - (z[i]*2 + z[i + 1])*h))*d + y[i];
}
static void spline3(double t, double *px, double *py, double *pz, int *pi, KS_GL_BUFFER_3D *sp)
{
  int	i,j,k;
  double	d,h;
  int datan;

  datan = (int)sp->num;

  i = 0;
  j = datan - 1;
  while(i < j){
    k = (i + j)/2;
    if (sp->p[k] < t)
      i = k + 1;
    else
      j = k;
  }
  if(i > 0) i--;
  h = sp->p[i + 1] - sp->p[i];
  d = t - sp->p[i];
  *pi = sp->p[i+1]-t < t-sp->p[i] ? i+1:i;

  *px = (((sp->a[i+1]-sp->a[i])*d/h+sp->a[i]*3)*d+
	 ((sp->x[i+1]-sp->x[i])/h-(sp->a[i]*2+sp->a[i+1])*h))*d+sp->x[i];
  /*  printf("%f %f %f %f %f\n",*px,sp->a[i],sp->a[i+1],sp->x[i],sp->x[i+1]);*/
  *py = (((sp->b[i+1]-sp->b[i])*d/h+sp->b[i]*3)*d+
	 ((sp->y[i+1]-sp->y[i])/h-(sp->b[i]*2+sp->b[i+1])*h))*d+sp->y[i];
  *pz = (((sp->c[i+1]-sp->c[i])*d/h+sp->c[i]*3)*d+
	 ((sp->z[i+1]-sp->z[i])/h-(sp->c[i]*2+sp->c[i+1])*h))*d+sp->z[i];
}
static void __KS_USED__ spline3_(double t, double *px, double *py, double *pz, int *pi, 
		     double *p, double *x, double *y, double *z, double *a, double *b, double *c,
		     int datan)
{
  int	i,j,k;
  double	d,h;

  i = 0;
  j = datan - 1;
  while(i < j){
    k = (i + j)/2;
    if (p[k] < t)
      i = k + 1;
    else
      j = k;
  }
  if(i > 0) i--;
  h = p[i + 1] - p[i];
  d = t - p[i];
  *pi = p[i+1]-t < t-p[i] ? i+1:i;

  *px = (((a[i+1]-a[i])*d/h+a[i]*3)*d+
	 ((x[i+1]-x[i])/h-(a[i]*2+a[i+1])*h))*d+x[i];
  *py = (((b[i+1]-b[i])*d/h+b[i]*3)*d+
	 ((y[i+1]-y[i])/h-(b[i]*2+b[i+1])*h))*d+y[i];
  *pz = (((c[i+1]-c[i])*d/h+c[i]*3)*d+
	 ((z[i+1]-z[i])/h-(c[i]*2+c[i+1])*h))*d+z[i];
}
/*
static void draw_tube(double r, double cd[2][3], double vec[2][3])
{
  int i,j;
  double rot_axis[3];
  double rot[3][3];
  double dd[3];

  glBegin(GL_LINES);
  glVertex3d(cd[0][0],cd[0][1],cd[0][2]);
  glVertex3d(cd[1][0],cd[1][1],cd[1][2]);
  glEnd();

  rot_axis[0] = -vec[0][1];
  rot_axis[1] = vec[0][0];
  rot_axis[2] = 0;
  ks_get_rotate_matrix(acos(vec[0][2]),rot_axis[0],rot_axis[1],rot_axis[2],rot);
  glBegin(GL_LINE_LOOP);
  for(i = 0; i < gl_tube->num; i++){
    dd[0] = gl_tube->x[i]*r;
    dd[1] = gl_tube->y[i]*r;
    dd[2] = 0.0;
    ks_multi_matrix_33x3(rot,dd);
    glVertex3d(dd[0]+cd[0][0],dd[1]+cd[0][1],dd[2]+cd[0][2]);
  }
  glEnd();
  for(j = 0; j < 3; j++) cd[0][j] = cd[1][j];
  for(j = 0; j < 3; j++) vec[0][j] = vec[1][j];
}
*/
static void init_vertex(double vec[3], double **vertex, KS_CIRCLE_BUFFER *cir)
{
  int i;
  double rot[3][3];
  /*
  double rot_axis[3];
  rot_axis[0] = -vec[1];
  rot_axis[1] = vec[0];
  rot_axis[2] = 0;
  ks_get_rotate_matrix(acos(vec[2]),rot_axis[0],rot_axis[1],rot_axis[2],rot);
  */
  ks_get_rotate_matrix(acos(vec[2]),-vec[1],vec[0],0.0,rot);
  for(i = 0; i < cir->num; i++){
    vertex[i][0] = cir->x[i];
    vertex[i][1] = cir->y[i];
    vertex[i][2] = 0.0;
    ks_multi_matrix_33x3(rot,vertex[i]);
  }
}
static void rot_ribbon_vertex(double rot[3][3], double **vertex, double **normal, 
			      KS_GL_BUFFER_3D *buf, int n)
{
  int i;
  for(i = 0; i < KS_GL_RIBBON_VERTEX_NUM; i++){
    vertex[i][0] = buf->x[n*KS_GL_RIBBON_VERTEX_NUM+i];
    vertex[i][1] = buf->y[n*KS_GL_RIBBON_VERTEX_NUM+i];
    vertex[i][2] = 0.0;
    normal[i][0] = buf->nx[n*KS_GL_RIBBON_VERTEX_NUM+i];
    normal[i][1] = buf->ny[n*KS_GL_RIBBON_VERTEX_NUM+i];
    normal[i][2] = 0.0;
    ks_multi_matrix_33x3(rot,vertex[i]);
    ks_multi_matrix_33x3(rot,normal[i]);
  }
}
static void get_rot_matrix(double vec[2][3], double rot[3][3])
{
  double rot_axis[3];
  double angle;
  angle = ks_calc_vectors_angle(vec[0],vec[1]);
  ks_exterior_product(vec[0],vec[1],rot_axis);
  ks_get_rotate_matrix(angle,rot_axis[0],rot_axis[1],rot_axis[2],rot);
}
static void rot_vertex(double angle, double vec[2][3], double **vertex, int num)
{
  int i;
  double rot_axis[3];
  double rot[3][3];
  ks_exterior_product(vec[0],vec[1],rot_axis);
  ks_get_rotate_matrix(angle,rot_axis[0],rot_axis[1],rot_axis[2],rot);
  for(i = 0; i < num; i++){
    ks_multi_matrix_33x3(rot,vertex[i]);
  }
}
static void draw_tube_poly(double pos0[3], double pos1[3], double **vertex[2], double r, int num)
{
  int i,j;
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < num; i++){
    glNormal3dv(vertex[1][i]);
    glVertex3d(vertex[1][i][0]*r+pos1[0],vertex[1][i][1]*r+pos1[1],vertex[1][i][2]*r+pos1[2]);
    glNormal3dv(vertex[0][i]);
    glVertex3d(vertex[0][i][0]*r+pos0[0],vertex[0][i][1]*r+pos0[1],vertex[0][i][2]*r+pos0[2]);
  }
  i = 0;
  glNormal3dv(vertex[1][i]);
  glVertex3d(vertex[1][i][0]*r+pos1[0],vertex[1][i][1]*r+pos1[1],vertex[1][i][2]*r+pos1[2]);
  glNormal3dv(vertex[0][i]);
  glVertex3d(vertex[0][i][0]*r+pos0[0],vertex[0][i][1]*r+pos0[1],vertex[0][i][2]*r+pos0[2]);
  glEnd();
  /*
  glDisable(GL_LIGHTING);
  glColor3f(1,1,1);
  glBegin(GL_LINE_LOOP);
  for(i = 0; i < num; i++){
    glVertex3d(vertex[0][i][0]*r+pos0[0],vertex[0][i][1]*r+pos0[1],vertex[0][i][2]*r+pos0[2]);
  }
  glEnd();
  glEnable(GL_LIGHTING);
  */
  for(i = 0; i < num; i++){
    for(j = 0; j < 3; j++){
      vertex[0][i][j] = vertex[1][i][j];
    }
  }
}
static void draw_ribbon_poly(double pos0[3], double pos1[3], double **vertex0, double **vertex1,
			     double **normal0, double **normal1,int num)
{
  int i;
  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < num; i++){
    glNormal3dv(normal0[i]);
    glVertex3d(vertex0[i][0]+pos0[0],vertex0[i][1]+pos0[1],vertex0[i][2]+pos0[2]);
    glNormal3dv(normal1[i]);
    glVertex3d(vertex1[i][0]+pos1[0],vertex1[i][1]+pos1[1],vertex1[i][2]+pos1[2]);
  }
  glEnd();
}
static void draw_ribbon_poly_color(double pos0[3], double pos1[3], 
				   double **vertex0, double **vertex1,
				   double **normal0, double **normal1,
				   GLfloat *color0, GLfloat *color1, GLfloat *em,int num)
{
  int i;

  glBegin(GL_QUAD_STRIP);
  for(i = 0; i < num; i++){
    glMaterialfv(GL_FRONT,GL_EMISSION,em);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color0);
    glNormal3dv(normal0[i]);
    glVertex3d(vertex0[i][0]+pos0[0],vertex0[i][1]+pos0[1],vertex0[i][2]+pos0[2]);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color1);
    glNormal3dv(normal1[i]);
    glVertex3d(vertex1[i][0]+pos1[0],vertex1[i][1]+pos1[1],vertex1[i][2]+pos1[2]);
  }
  glEnd();

}
void ks_draw_gl_tube(KS_GL_BASE *gb, KS_GL_BUFFER_3D *buf,
		     double r, double skip_angle, int circle_num)
{
  int i,j,k;
  double cd[3][3];
  double vec[2][3];
  double angle;

  if(gb->tube != NULL){
    if(circle_num != gb->tube->num){
      for(i = 0; i < 2; i++)
	for(j = 0; j < gb->tube->num; j++)
	  ks_free(gb->verbuf[i][j]);
      for(i = 0; i < 2; i++)
	ks_free(gb->verbuf[i]);
      ks_free_circle_buffer(gb->tube);
      gb->tube = NULL;
    }
  }
  if(gb->tube == NULL){
    gb->tube = ks_allocate_circle_buffer(circle_num);
    for(i = 0; i < 2; i++){
      if((gb->verbuf[i] = ks_malloc_double_pp(circle_num,3,"ks_draw_gl_tube")) == NULL){
      }
      /*      printf("%d %d\n",i,gb->verbuf[i]);*/
    }
  }

  for(i = 0; i < 2; i++){
    cd[i][0] = buf->x[i];
    cd[i][1] = buf->y[i];
    cd[i][2] = buf->z[i];
  }
  for(j = 0; j < 3; j++)
    vec[0][j] = cd[1][j] - cd[0][j];
  ks_normalize_vector(vec[0],3);
  init_vertex(vec[0],gb->verbuf[0],gb->tube);
  for(i = 2; i < (int)buf->num; i++){
    cd[2][0] = buf->x[i];
    cd[2][1] = buf->y[i];
    cd[2][2] = buf->z[i];
    if(buf->color != NULL)
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,buf->color[i]);
    /*
    {
      GLfloat color[4];
      if((i%3) == 0){
	color[0] = 1.0;	color[1] = 0.0;	color[2] = 0.0;	color[3] = 1.0;
      } else if((i%3) == 1){
	color[0] = 0.0;	color[1] = 1.0;	color[2] = 0.0;	color[3] = 1.0;
      } else {
	color[0] = 0.0;	color[1] = 0.0;	color[2] = 1.0;	color[3] = 1.0;
      }
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
    }
    */
    for(j = 0; j < 3; j++)
      vec[1][j] = cd[2][j] - cd[1][j];
    ks_normalize_vector(vec[1],3);
    angle = ks_calc_vectors_angle(vec[0],vec[1]);
    if(angle > skip_angle/180.*M_PI){
      for(j = 0; j < gb->tube->num; j++)
	for(k = 0; k < 3; k++)
	  gb->verbuf[1][j][k] = gb->verbuf[0][j][k];
      rot_vertex(angle,vec,gb->verbuf[1],gb->tube->num);
      if(buf->label != NULL)
	glPushName(buf->label[i]);
      draw_tube_poly(cd[0],cd[1],gb->verbuf,r,gb->tube->num);
      if(buf->label != NULL)
	glPopName();
      for(j = 0; j < 2; j++)
	for(k = 0; k < 3; k++)
	  cd[j][k] = cd[j+1][k];
      for(j = 0; j < 3; j++)
	vec[0][j] = vec[1][j];
    } else {
      for(k = 0; k < 3; k++)
	cd[1][k] = cd[2][k];
      /*
      for(j = 0; j < 3; j++)
	vec[0][j] = cd[1][j] - cd[0][j];
      ks_normalize_vector(vec[0],3);
      */
    }
  }
  draw_tube_poly(cd[0],cd[1],gb->verbuf,r,gb->tube->num);

}
BOOL ks_set_gl_tube_spline(KS_GL_BUFFER_3D *spline, int n, KS_GL_BUFFER_3D **buf)
{
  int i;
  double pos[3];
  int data_pos;
  GLfloat *color;

  if((*buf) == NULL){
    if(((*buf) = ks_allocate_gl_buffer_3d(0,2,1,
					  KS_GL_BUFFER_3D_DEFAULT|
					  KS_GL_BUFFER_3D_NORMAL|
					  KS_GL_BUFFER_3D_SPLINE,"buf")) == NULL){
      return KS_FALSE;
    }
  }
  make_spline_table3(spline);
  for(i = 0; i < n; i++){
    spline3((double)i/(n-1),&pos[0],&pos[1],&pos[2],&data_pos,spline);
    if(spline->color == NULL){
      color = NULL;
    } else {
      color = spline->color[data_pos];
    }
    ks_set_spline_buffer_3d((*buf),pos[0],pos[1],pos[2],color,spline->label[data_pos]);
  }
  return KS_TRUE;
}
BOOL ks_draw_gl_tube_spline(KS_GL_BASE *gb, KS_GL_BUFFER_3D *spline, double r, int div_num, 
			    double skip_angle, int circle_num)
{
  int n;

  if(spline->num == 0)
    return KS_FALSE;
  n = (int)spline->num*div_num;

  if(ks_set_gl_tube_spline(spline,n,&gb->hokan) == KS_FALSE){
    return KS_FALSE;
  }
  ks_draw_gl_tube(gb,gb->hokan,r,skip_angle,circle_num);

  ks_clear_gl_buffer_3d(gb->hokan);

  return KS_TRUE;
}
GLboolean ks_gl_enable(GLenum capability)
{
  if(glIsEnabled(capability) == GL_FALSE){
    glEnable(capability);
    return GL_TRUE;
  } else {
    return GL_FALSE;
  }
}
GLboolean ks_gl_disable(GLenum capability)
{
  if(glIsEnabled(capability) == GL_TRUE){
    glDisable(capability);
    return GL_TRUE;
  } else {
    return GL_FALSE;
  }
}
void ks_draw_gl_texture_2d(GLuint texture, GLfloat x, GLfloat y, GLfloat width, GLfloat height)
{
  GLboolean lighting,texture_2d,alpha;

  lighting = ks_gl_disable(GL_LIGHTING);
  texture_2d = ks_gl_enable(GL_TEXTURE_2D);
  alpha = ks_gl_enable(GL_ALPHA_TEST);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glBindTexture(GL_TEXTURE_2D, texture);

  glBegin(GL_QUADS);
  glTexCoord2f(1.0,1.0);
  glVertex2f(x+width,y+height);
  glTexCoord2f(1.0,0.0);
  glVertex2f(x,y+height);
  glTexCoord2f(0.0,0.0);
  glVertex2f(x,y);
  glTexCoord2f(0.0,1.0);
  glVertex2f(x+width,y);
  glEnd();
  if(lighting == GL_TRUE){
    glEnable(GL_LIGHTING);
  }
  if(texture_2d == GL_TRUE){
    glDisable(GL_TEXTURE_2D);
  }
  if(alpha == GL_TRUE){
    glDisable(GL_ALPHA_TEST);
  }
}
BOOL ks_init_gl_ribbon(KS_GL_BASE *gb, double scale)
{
  int rl;
  int i,j;
  double d0,d1,d2,d3;
  int ribbon_num;
  KS_GL_BUFFER_3D *ra,*rb,*r;
  double width;

  if((ra = ks_allocate_gl_buffer_3d(KS_GL_RIBBON_A_NUM*KS_GL_RIBBON_VERTEX_NUM,1,
				    KS_GL_RIBBON_A_NUM*KS_GL_RIBBON_VERTEX_NUM,
				    KS_GL_BUFFER_3D_DEFAULT|KS_GL_BUFFER_3D_NORMAL,"ra")) == NULL){
    return KS_FALSE;
  }
  if((rb = ks_allocate_gl_buffer_3d(KS_GL_RIBBON_B_NUM*KS_GL_RIBBON_VERTEX_NUM,1,
				    KS_GL_RIBBON_B_NUM*KS_GL_RIBBON_VERTEX_NUM,
				    KS_GL_BUFFER_3D_DEFAULT|KS_GL_BUFFER_3D_NORMAL,"rb")) == NULL){
    return KS_FALSE;
  }
  for(i = 0; i < KS_GL_BASE_VERTEX_BUF_NUM; i++){
    if((gb->ribbon_v[i] = ks_malloc_double_pp(KS_GL_RIBBON_VERTEX_NUM,3,"ribbon_v")) == NULL){
      return KS_FALSE;
    }
    if((gb->ribbon_n[i] = ks_malloc_double_pp(KS_GL_RIBBON_VERTEX_NUM,3,"ribbon_n")) == NULL){
      return KS_FALSE;
    }
    /*
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      printf("%d %d %p %p %p\n",i,j,
	     &gb->ribbon_ver[i][j][0],&gb->ribbon_ver[i][j][1],&gb->ribbon_ver[i][j][2]);
    }
    */
  }

  for(rl = 0; rl < 1; rl++){
    if(rl == 0){
      r = ra;
      ribbon_num = KS_GL_RIBBON_A_NUM;
    } else {
      r = rb;
      ribbon_num = KS_GL_RIBBON_B_NUM;
    }
    d0 = M_PI*2/(KS_GL_RIBBON_VERTEX_NUM-1);
    for(i = 0; i < (int)ribbon_num; i++){
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	r->x[i*KS_GL_RIBBON_VERTEX_NUM+j] = cos(-d0*j)*scale;
	r->y[i*KS_GL_RIBBON_VERTEX_NUM+j] = sin(-d0*j)*scale;
	r->z[i*KS_GL_RIBBON_VERTEX_NUM+j] = i;
      }
    }
    if(rl == 0){
      i = KS_GL_RIBBON_A_5;
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+1] = -4.2*scale;
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+2] = -4.9*scale;
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+3] = -5.0*scale;
    } else {
      i = KS_GL_RIBBON_B_3;
      d0 = 1.5;
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+1] = -4.2*d0*scale;
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+2] = -4.9*d0*scale;
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+3] = -5.0*d0*scale;
    }
    r->y[i*KS_GL_RIBBON_VERTEX_NUM+4] = r->y[i*KS_GL_RIBBON_VERTEX_NUM+2];
    r->y[i*KS_GL_RIBBON_VERTEX_NUM+5] = r->y[i*KS_GL_RIBBON_VERTEX_NUM+1];
    for(j = 7; j < KS_GL_RIBBON_VERTEX_NUM-1; j++)
      r->y[i*KS_GL_RIBBON_VERTEX_NUM+j] =
	-r->y[i*KS_GL_RIBBON_VERTEX_NUM+KS_GL_RIBBON_VERTEX_NUM-1-j];

    if(rl == 0){
      for(i = KS_GL_RIBBON_A_2; i < KS_GL_RIBBON_A_5; i++){
	if(i == KS_GL_RIBBON_A_2){
	  d0 = 0.1;
	} else if(i == KS_GL_RIBBON_A_3){
	  d0 = 0.3;
	} else if(i == KS_GL_RIBBON_A_4){
	  d0 = 0.6;
	}
	for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	  r->y[i*KS_GL_RIBBON_VERTEX_NUM+j]
	    =(r->y[KS_GL_RIBBON_A_1*KS_GL_RIBBON_VERTEX_NUM+j]*(1-d0)+
	      r->y[KS_GL_RIBBON_A_5*KS_GL_RIBBON_VERTEX_NUM+j]*d0);
	}
      }
    } else {
      for(i = KS_GL_RIBBON_B_2; i < KS_GL_RIBBON_A_3; i++){
	d0 = 0.5;
	for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	  r->y[i*KS_GL_RIBBON_VERTEX_NUM+j]
	    =(r->y[KS_GL_RIBBON_B_1*KS_GL_RIBBON_VERTEX_NUM+j]*(1-d0)+
	      r->y[KS_GL_RIBBON_B_3*KS_GL_RIBBON_VERTEX_NUM+j]*d0);
	}
      }
    }
    for(i = 0; i < (int)ribbon_num; i++){
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	if((j%3) == 0){
	  r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] = r->x[i*KS_GL_RIBBON_VERTEX_NUM+j];
	  r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] = r->y[i*KS_GL_RIBBON_VERTEX_NUM+j];
	  r->nz[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0;
	} else {
	  d0 = -(r->y[i*KS_GL_RIBBON_VERTEX_NUM+j] - r->y[i*KS_GL_RIBBON_VERTEX_NUM+j-1]);
	  d1 =  (r->x[i*KS_GL_RIBBON_VERTEX_NUM+j] - r->x[i*KS_GL_RIBBON_VERTEX_NUM+j-1]);
	  d2 = -(r->y[i*KS_GL_RIBBON_VERTEX_NUM+j+1] - r->y[i*KS_GL_RIBBON_VERTEX_NUM+j]);
	  d3 =  (r->x[i*KS_GL_RIBBON_VERTEX_NUM+j+1] - r->x[i*KS_GL_RIBBON_VERTEX_NUM+j]);
	  r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] = d0+d2;
	  r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] = d1+d3;
	  r->nz[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0;
	}
	d0  = r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j]*r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j];
	d0 += r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j]*r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j];
	d0 += r->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]*r->nz[i*KS_GL_RIBBON_VERTEX_NUM+j];
	d0 = sqrt(d0);
	r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] /= d0;
	r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] /= d0;
	r->nz[i*KS_GL_RIBBON_VERTEX_NUM+j] /= d0;
      }
    }
  }
  r = rb;
  ribbon_num = KS_GL_RIBBON_B_NUM;
  d0 = M_PI*2/(KS_GL_RIBBON_VERTEX_NUM-1);
  for(i = 0; i < (int)ribbon_num; i++){
    if(i == 0){
      width = scale*5;
    } else {
      width = scale*5*1.5*(1.-(double)(i-1)/(ribbon_num-2)) + scale*(double)(i-1)/(ribbon_num-2);
    }
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      if(j <= 2 || j >= 10){
	r->x[i*KS_GL_RIBBON_VERTEX_NUM+j] = scale;
      } else if(j >= 4 && j <= 8){
	r->x[i*KS_GL_RIBBON_VERTEX_NUM+j] =-scale;
      } else {
	r->x[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0;
      }
      if(j >= 1 && j <= 5){
	r->y[i*KS_GL_RIBBON_VERTEX_NUM+j] =-width;
      } else if(j >= 7 && j <= 11){
	r->y[i*KS_GL_RIBBON_VERTEX_NUM+j] = width;
      } else {
	r->y[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0;
      }
      r->z[i*KS_GL_RIBBON_VERTEX_NUM+j] = i;
      if(j <= 1 || j >= 11){
	r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] = 1.0;
	r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0.0;
      } else if(j >= 2 && j <= 4){
	r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0.0;
	r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] =-1.0;
      } else if(j >= 5 && j <= 7){
	r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] =-1.0;
	r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0.0;
      } else if(j >= 8 && j <= 10){
	r->nx[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0.0;
	r->ny[i*KS_GL_RIBBON_VERTEX_NUM+j] = 1.0;
      }
      r->nz[i*KS_GL_RIBBON_VERTEX_NUM+j] = 0;
    }
  }

  gb->ribbon_a = ra;
  gb->ribbon_b = rb;
  return KS_TRUE;
}
static void __KS_USED__ draw_normal_line(double x, double y, double z, double nx, double ny, double nz,
			     GLfloat c0, GLfloat c1, GLfloat c2)
{
  GLboolean lighting;
  lighting = ks_gl_disable(GL_LIGHTING);
  glColor4f(c0,c1,c2,1.0);
  glBegin(GL_LINES);
  glVertex3d(x,y,z);
  glVertex3d(x+nx,y+ny,z+nz);
  glEnd();
  if(lighting == GL_TRUE)
    glEnable(GL_LIGHTING);
}
void ks_draw_gl_ribbon_test(KS_GL_BASE *gb, int mode)
{
  int i,j;
  double z,len = 0.5;
  KS_GL_BUFFER_3D *ra,*rb;

  ra = gb->ribbon_a;
  rb = gb->ribbon_b;
  /*
  for(i = 0; i < KS_GL_RIBBON_A_NUM-1; i++){
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      glNormal3d(ra->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(ra->x[i*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 ra->y[i*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 ra->z[i*KS_GL_RIBBON_VERTEX_NUM+j]);
      glNormal3d(ra->nx[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->ny[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->nz[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(ra->x[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 ra->y[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 ra->z[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
    }
    glEnd();
  }
  glDisable(GL_LIGHTING);
  glBegin(GL_LINES);
  for(i = 0; i < KS_GL_RIBBON_A_NUM-1; i++){
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      glVertex3d(ra->x[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 ra->y[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 ra->z[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(ra->x[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r+ra->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->y[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r+ra->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->z[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]  +ra->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
    }
  }
  glEnd();
  glEnable(GL_LIGHTING);
  */
  /*
  for(i = 0; i < KS_GL_RIBBON_B_NUM-1; i++){
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      glNormal3d(rb->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 rb->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 rb->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(rb->x[i*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 rb->y[i*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 rb->z[i*KS_GL_RIBBON_VERTEX_NUM+j]);
      glNormal3d(rb->nx[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		 rb->ny[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		 rb->nz[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(rb->x[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 rb->y[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]*r,
		 rb->z[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
    }
    glEnd();
  }
  glDisable(GL_LIGHTING);
  */
  if(mode == 0){
    z = 0;
    for(i = 0; i < KS_GL_RIBBON_A_NUM-1; i++){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	glNormal3d(ra->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(ra->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   z);
	glNormal3d(ra->nx[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->ny[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->nz[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(ra->x[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->y[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   z+len);
      }
      glEnd();
      z += len;
    }
    i = KS_GL_RIBBON_A_5;
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      glNormal3d(ra->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(ra->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 z);
      glNormal3d(ra->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
      glVertex3d(ra->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 z+10);
    }
    glEnd();
    z += 10;
    for(i = KS_GL_RIBBON_A_NUM-1; i > 0 ; i--){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	glNormal3d(ra->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(ra->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   z);
	glNormal3d(ra->nx[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->ny[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->nz[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(ra->x[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   ra->y[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   z+len);
      }
      glEnd();
      z += len;
    }
  } else if(mode == 1){
    /*
    glBegin(GL_QUAD_STRIP);
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
      glNormal3d(0,0,-1);
      i = KS_GL_RIBBON_A_5;
      glVertex3d(ra->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 ra->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 z);
      i = KS_GL_RIBBON_B_3;
      glVertex3d(rb->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 rb->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		 z);
    }
    glEnd();

    for(i = KS_GL_RIBBON_B_NUM-1; i > 0 ; i--){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	glNormal3d(rb->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(rb->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   z);
	glNormal3d(rb->nx[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->ny[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->nz[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(rb->x[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->y[(i-1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   z+len);
      }
      glEnd();
      z += len;
    }
    */
    z = 0;
    for(i = 0; i < KS_GL_RIBBON_B_NUM-1; i++){
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	glNormal3d(rb->nx[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->ny[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->nz[i*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(rb->x[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->y[i*KS_GL_RIBBON_VERTEX_NUM+j],
		   z);
	glNormal3d(rb->nx[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->ny[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->nz[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j]);
	glVertex3d(rb->x[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   rb->y[(i+1)*KS_GL_RIBBON_VERTEX_NUM+j],
		   z+len);
      }
      glEnd();
      z += len;
    }
  }
}
static void __KS_USED__ calc_normal_vectors(KS_GL_BUFFER_3D *buf, int start, double t, double normal[3])
{
  int i;
  int end;
  double ax[3];
  double ex[3][3];
  double angle;
  double rot[3][3];

  end = start+1;
  /*
  printf("%f %d (%f %f %f) %d (%f %f %f)\n",t
	 ,start,buf->nx[start],buf->ny[start],buf->nz[start]
	 ,end,buf->nx[end],buf->ny[end],buf->nz[end]);
  */
  ax[0] = buf->x[end] - buf->x[start];
  ax[1] = buf->y[end] - buf->y[start];
  ax[2] = buf->z[end] - buf->z[start];
  ex[0][0] = buf->ny[start]*ax[2] - buf->nz[start]*ax[1];
  ex[0][1] = buf->nz[start]*ax[0] - buf->nx[start]*ax[2];
  ex[0][2] = buf->nx[start]*ax[1] - buf->ny[start]*ax[0];
  ex[1][0] = buf->ny[end]*ax[2] - buf->nz[end]*ax[1];
  ex[1][1] = buf->nz[end]*ax[0] - buf->nx[end]*ax[2];
  ex[1][2] = buf->nx[end]*ax[1] - buf->ny[end]*ax[0];
  angle = ks_calc_vectors_angle(ex[0],ex[1]);
    /*  printf("%f %f\n",angle,M_PI*.5);*/
  if(angle > M_PI*.5){
    for(i = 0; i < 3; i++) ex[1][i] *= -1;
    buf->nx[end] *= -1;
    buf->ny[end] *= -1;
    buf->nz[end] *= -1;
    angle -= M_PI;
  }
  ks_exterior_product(ex[0],ex[1],ex[2]);
  if(ks_calc_vectors_angle(ax,ex[2]) > M_PI*0.5){
    angle *= -1;
  }
  ks_exterior_product(ax,ex[0],normal);
  ks_get_rotate_matrix(angle*t,ax[0],ax[1],ax[2],rot);
  ks_multi_matrix_33x3(rot,normal);
}
static void rainbow_color(double h, GLfloat *color, double v)
{
  double dcolor[3];
  ks_hsv2rgb(h*240,1.0,v,&dcolor[0],&dcolor[1],&dcolor[2]);
  //  printf("%f %f %f %f\n",h,dcolor[0],dcolor[1],dcolor[2]);
  color[0] = (GLfloat)dcolor[0];
  color[1] = (GLfloat)dcolor[1];
  color[2] = (GLfloat)dcolor[2];
}
static void draw_ribbon(KS_GL_BASE *gb, KS_GL_BUFFER_3D *ribbon, int div, int color_type, 
			GLfloat *alpha_color, GLfloat *beta_color, GLfloat *loop_color)
{
  int i,j,k;
  int ri,di;
  double vec[2][3];
  double pos[3][3];
  double now[3][3],rot[3][3],tmp[3][3];
  double angle, angle_factor;
  double normal[3] = {1,0,0};
  double rn[3],re[3];
  KS_GL_BUFFER_3D *hokan;
  int ribbon_type;
  KS_GL_BUFFER_3D *ribbon_buf;
  BOOL beta_arrow_start;
  GLfloat color[2][4] = {{0.0,0.0,0.0,1.0},{0.0,0.0,0.0,1.0}};
  GLfloat em[4] = {0,0,0,0};

  hokan = gb->hokan;

  for(i = 0; i < 2; i++){
    pos[i][0] = hokan->x[i];
    pos[i][1] = hokan->y[i];
    pos[i][2] = hokan->z[i];
  }
  for(i = 0; i < 3; i++){
    vec[0][i] = pos[1][i] - pos[0][i];
  }
  ks_normalize_vector(vec[0],3);
  ks_get_rotate_matrix(acos(vec[0][2]),-vec[0][1],vec[0][0],0.0,now);
  rot_ribbon_vertex(now,gb->ribbon_v[0],gb->ribbon_n[0],gb->ribbon_a,KS_GL_RIBBON_A_1);
  ks_multi_matrix_33x3(now,normal);

  /* start cover */

  if(color_type == KS_DRAW_GL_RIBBON_USER_COLOR){
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,hokan->color[0]);
  } else if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,loop_color);
  } else if(color_type == KS_DRAW_GL_RIBBON_RAINBOW){
    rainbow_color(1.0,color[0],1.0);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color[0]);
  } else if(color_type == KS_DRAW_GL_RIBBON_RAINBOW_LOW){
    rainbow_color(1.0,color[0],0.5);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color[0]);
  }
  if(hokan->flags[0]&KS_GL_RESIDUE_SELECTED){
    for(j = 0; j < 3; j++) em[j] = KS_GL_SELECTED_EMISSION;
  }  else {
    for(j = 0; j < 3; j++) em[j] = 0.0;
  }
  glMaterialfv(GL_FRONT,GL_EMISSION,em);

  glPushName(hokan->label[0]);
  for(j = 0; j < 3; j++)
    rn[j] = -vec[0][j];
  ks_normalize_vector(rn,3);
  glBegin(GL_POLYGON);
  glNormal3dv(rn);
  for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
    glVertex3d(gb->ribbon_v[0][j][0]+pos[0][0],
	       gb->ribbon_v[0][j][1]+pos[0][1],
	       gb->ribbon_v[0][j][2]+pos[0][2]);
  }
  glEnd();
  glPopName();

  for(i = 1; i < (int)hokan->num-1; i++){
    ri = i/div;
    di = i-ri*div;
    beta_arrow_start = KS_FALSE;

    //    glPushName(ribbon->label[ri]);
    glPushName(hokan->label[i]);
    /*    printf("%d name %d\n",i,hokan->label[i]);*/
    /*
    printf("%d %d %d %-5s\n",i,ri,di,
	   ribbon->flags[ri]&KS_GL_RESIDUE_ALPHA ? "ALPHA":
	   ribbon->flags[ri]&KS_GL_RESIDUE_BETA ? "BETA":
	   ribbon->flags[ri]&KS_GL_RESIDUE_LOOP ? "LOOP":"NON");
    */
    pos[2][0] = hokan->x[i+1];
    pos[2][1] = hokan->y[i+1];
    pos[2][2] = hokan->z[i+1];

    for(j = 0; j < 3; j++)
      vec[1][j] = pos[2][j] - pos[1][j];
    /*
    printf("%d (% f % f % f) (% f % f % f) (% f % f % f) (% f % f % f)\n",i
	   ,pos[0][0],pos[0][1],pos[0][2]
	   ,pos[1][0],pos[1][1],pos[1][2]
	   ,vec[0][0],vec[0][1],vec[0][2]
	   ,vec[1][0],vec[1][1],vec[1][2]);
    */
    ks_normalize_vector(vec[1],3);
    /*
    printf("%d (% f % f % f) (% f % f % f)\n",i
	   ,vec[0][0],vec[0][1],vec[0][2]
	   ,vec[1][0],vec[1][1],vec[1][2]);
    */
    get_rot_matrix(vec,rot);

    ks_multi_matrix_33x3(rot,normal);
    ks_multi_matrix_33x33(rot,now,tmp);
    for(j = 0; j < 3; j++)
      memcpy(now[j],tmp[j],3*sizeof(double));

    /* set default shape and color */

    if(color_type == KS_DRAW_GL_RIBBON_USER_COLOR){
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,hokan->color[i]);
    } else if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
      glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,loop_color);
    }
    if(hokan->flags[i]&KS_GL_RESIDUE_SELECTED){
      for(j = 0; j < 3; j++) em[j] = KS_GL_SELECTED_EMISSION;
    }  else {
      for(j = 0; j < 3; j++) em[j] = 0.0;
    }
    glMaterialfv(GL_FRONT,GL_EMISSION,em);
    ribbon_type = KS_GL_RIBBON_A_1;
    ribbon_buf = gb->ribbon_a;

    if(ri < (int)ribbon->num-1){
      if(!(ribbon->flags[ri+1]&KS_GL_RESIDUE_BETA) && ribbon->flags[ri]&KS_GL_RESIDUE_BETA && 
	 di == 0){
	/* end of beta */
	ribbon_type = KS_GL_RIBBON_B_5;
	ribbon_buf = gb->ribbon_b;
	if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
	  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,beta_color);
	}
      } else if(ribbon->flags[ri]&KS_GL_RESIDUE_ALPHA){
	/* default of alpha */
	ribbon_type = KS_GL_RIBBON_A_5;
	if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
	  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,alpha_color);
	}
	if(!(ribbon->flags[ri-1]&KS_GL_RESIDUE_ALPHA)){
	  /* start of alpha */
	  if(di == 0){
	    ribbon_type = KS_GL_RIBBON_A_1;
	  } else if(di == 1){
	    ribbon_type = KS_GL_RIBBON_A_2;
	  } else if(di == 2){
	    ribbon_type = KS_GL_RIBBON_A_3;
	  } else if(di == 3){
	    ribbon_type = KS_GL_RIBBON_A_4;
	  }
	} else if(!(ribbon->flags[ri+1]&KS_GL_RESIDUE_ALPHA)){
	  /* end of alpha */
	  if(di == div-1){
	    ribbon_type = KS_GL_RIBBON_A_1;
	  } else if(di == div-2){
	    ribbon_type = KS_GL_RIBBON_A_2;
	  } else if(di == div-3){
	    ribbon_type = KS_GL_RIBBON_A_3;
	  } else if(di == div-4){
	    ribbon_type = KS_GL_RIBBON_A_4;
	  }
	}
      } else if(ribbon->flags[ri]&KS_GL_RESIDUE_BETA){
	/* default of beta */
	ribbon_type = KS_GL_RIBBON_B_1;
	if(ribbon->flags[ri+1]&KS_GL_RESIDUE_BETA){
	  ribbon_buf = gb->ribbon_b;
	  if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,beta_color);
	  }
	  if(!(ribbon->flags[ri-1]&KS_GL_RESIDUE_BETA) && di == 0){ 
	    beta_arrow_start = KS_TRUE;
	  } else if(ri < (int)ribbon->num-2 && !(ribbon->flags[ri+2]&KS_GL_RESIDUE_BETA)){
	    /* arrow of beta */
	    if(di == div-1){
	      ribbon_type = KS_GL_RIBBON_B_4;
	      ribbon_buf = gb->ribbon_b;
	    } else if(di == div-2){
	      ribbon_type = KS_GL_RIBBON_B_3;
	      ribbon_buf = gb->ribbon_b;
	    } else if(di == div-3){
	      ribbon_type = KS_GL_RIBBON_B_2;
	      ribbon_buf = gb->ribbon_b;
	      beta_arrow_start = KS_TRUE;
	    }
	  }
	} else {
	  ribbon_type = KS_GL_RIBBON_A_1;
	}
      }
      if(ribbon->flags[ri+1]&KS_GL_RESIDUE_ALPHA){ /* set normal vector of alpha */
	ks_exterior_product(vec[0],vec[1],re);
	ks_exterior_product(vec[1],re,rn);
	angle_factor = 0.5;
      } else if(ribbon->flags[ri+1]&KS_GL_RESIDUE_BETA){ /* set normal vector of beta */
	rn[0] = vec[1][1]*ribbon->nz[ri+1] - ribbon->ny[ri+1]*vec[1][2];
	rn[1] = vec[1][2]*ribbon->nx[ri+1] - ribbon->nz[ri+1]*vec[1][0];
	rn[2] = vec[1][0]*ribbon->ny[ri+1] - ribbon->nx[ri+1]*vec[1][1];
	angle_factor = 0.05;
      } else {
	goto DRAW_RIBBON_LOOP_JUMP;
      }
      angle = ks_calc_vectors_angle(normal,rn);
      if(angle > M_PI*.5) angle -= M_PI;
      ks_exterior_product(normal,rn,re);
      ks_get_rotate_matrix(angle*angle_factor,re[0],re[1],re[2],rot);
      ks_multi_matrix_33x3(rot,normal);
      ks_multi_matrix_33x33(rot,now,tmp);
      for(j = 0; j < 3; j++)
	for(k = 0; k < 3; k++)
	  now[j][k] = tmp[j][k];
    DRAW_RIBBON_LOOP_JUMP:
      ;
    }


    if(beta_arrow_start == KS_TRUE){ /* start of beta and that of beta arrow */
      double bpos[3];
      for(j = 0; j < 3; j++)bpos[j] = pos[0][j]*.1+pos[1][j]*.9;

      if(di == 0){
	if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
	  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,loop_color);
	}
	rot_ribbon_vertex(now,gb->ribbon_v[1],gb->ribbon_n[1],gb->ribbon_a,KS_GL_RIBBON_A_1);
      } else {
	rot_ribbon_vertex(now,gb->ribbon_v[1],gb->ribbon_n[1],gb->ribbon_b,KS_GL_RIBBON_B_1);
      }
      if(color_type == KS_DRAW_GL_RIBBON_RAINBOW || color_type == KS_DRAW_GL_RIBBON_RAINBOW_LOW){
	if(color_type == KS_DRAW_GL_RIBBON_RAINBOW){
	  rainbow_color(1-(double)i/hokan->num,color[0],1.0);
	  rainbow_color(1-(double)(i+1)/hokan->num,color[1],1.0);
	} else {
	  rainbow_color(1-(double)i/hokan->num,color[0],.5);
	  rainbow_color(1-(double)(i+1)/hokan->num,color[1],.5);
	}
	draw_ribbon_poly_color(pos[0],bpos,
			       gb->ribbon_v[0],gb->ribbon_v[1],
			       gb->ribbon_n[0],gb->ribbon_n[1],
			       color[0],color[1],em,
			       KS_GL_RIBBON_VERTEX_NUM);
      } else {
	draw_ribbon_poly(pos[0],bpos,
			 gb->ribbon_v[0],gb->ribbon_v[1],
			 gb->ribbon_n[0],gb->ribbon_n[1],KS_GL_RIBBON_VERTEX_NUM);
      }

      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	memcpy(gb->ribbon_v[0][j],gb->ribbon_v[1][j],3*sizeof(double));
      }
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	memcpy(gb->ribbon_n[0][j],gb->ribbon_n[1][j],3*sizeof(double));
      }

      rot_ribbon_vertex(now,gb->ribbon_v[1],gb->ribbon_n[1],ribbon_buf,ribbon_type);

      for(j = 0; j < 3; j++)
	rn[j] = -vec[1][j];
      ks_normalize_vector(rn,3);
      glNormal3dv(rn);

      if(color_type == KS_DRAW_GL_RIBBON_ALPHA_BETA_COLOR){
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,beta_color);
      } else if(color_type == KS_DRAW_GL_RIBBON_RAINBOW || 
		color_type == KS_DRAW_GL_RIBBON_RAINBOW_LOW){
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color[1]);
      }
      glBegin(GL_QUAD_STRIP);
      for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
	if((j >= 2 && j <= 4) || (j >= 8 && j <= 10)){
	  glNormal3dv(rn);
	} else {
	  glNormal3dv(gb->ribbon_n[0][j]);
	}
	glVertex3d(gb->ribbon_v[0][j][0]+bpos[0],
		   gb->ribbon_v[0][j][1]+bpos[1],
		   gb->ribbon_v[0][j][2]+bpos[2]);
	if((j >= 2 && j <= 4) || (j >= 8 && j <= 10)){
	  glNormal3dv(rn);
	} else {
	  glNormal3dv(gb->ribbon_n[0][j]);
	}
	glVertex3d(gb->ribbon_v[1][j][0]+pos[1][0],
		   gb->ribbon_v[1][j][1]+pos[1][1],
		   gb->ribbon_v[1][j][2]+pos[1][2]);
      }
      glEnd();
    } else {
      rot_ribbon_vertex(now,gb->ribbon_v[1],gb->ribbon_n[1],ribbon_buf,ribbon_type);
      /*
      printf("(%f %f %f)(%f %f %f)(%f %f %f)\n"
	     ,now[0][0],now[0][1],now[0][2]
	     ,now[1][0],now[1][1],now[1][2]
	     ,now[2][0],now[2][1],now[2][2]);
      */
      if(color_type == KS_DRAW_GL_RIBBON_RAINBOW || color_type == KS_DRAW_GL_RIBBON_RAINBOW_LOW){
	if(color_type == KS_DRAW_GL_RIBBON_RAINBOW){
	  rainbow_color(1-(double)i/hokan->num,color[0],1.0);
	  rainbow_color(1-(double)(i+1)/hokan->num,color[1],1.0);
	} else {
	  rainbow_color(1-(double)i/hokan->num,color[0],0.5);
	  rainbow_color(1-(double)(i+1)/hokan->num,color[1],0.5);
	}
	draw_ribbon_poly_color(pos[0],pos[1],
			       gb->ribbon_v[0],gb->ribbon_v[1],
			       gb->ribbon_n[0],gb->ribbon_n[1],
			       color[0],color[1],em,
			       KS_GL_RIBBON_VERTEX_NUM);
      } else {
	draw_ribbon_poly(pos[0],pos[1],
			 gb->ribbon_v[0],gb->ribbon_v[1],
			 gb->ribbon_n[0],gb->ribbon_n[1],KS_GL_RIBBON_VERTEX_NUM);
      }
    }
    /*
    for(j = 0; j < 3; j++) pos[0][j] = pos[1][j];
    for(j = 0; j < 3; j++) pos[1][j] = pos[2][j];
    for(j = 0; j < 3; j++) vec[0][j] = vec[1][j];
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++)
      for(k = 0; k < 3; k++)
	gb->ribbon_v[0][j][k] = gb->ribbon_v[1][j][k];
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++)
      for(k = 0; k < 3; k++)
	gb->ribbon_n[0][j][k] = gb->ribbon_n[1][j][k];
    */
    memcpy(pos[0],pos[1],3*sizeof(double));
    memcpy(pos[1],pos[2],3*sizeof(double));
    memcpy(vec[0],vec[1],3*sizeof(double));
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++)
      memcpy(gb->ribbon_v[0][j],gb->ribbon_v[1][j],3*sizeof(double));
    for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++)
      memcpy(gb->ribbon_n[0][j],gb->ribbon_n[1][j],3*sizeof(double));
    glPopName();
  }
  /* end cover */
  /*  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,ribbon->color[ribbon->num-1]);*/
  //  glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,hokan->color[hokan->num-1]);

  rot_ribbon_vertex(now,gb->ribbon_v[1],gb->ribbon_n[1],ribbon_buf,ribbon_type);

  glPushName(hokan->label[hokan->num-1]);
  draw_ribbon_poly(pos[0],pos[1],
		   gb->ribbon_v[0],gb->ribbon_v[1],
		   gb->ribbon_n[0],gb->ribbon_n[1],KS_GL_RIBBON_VERTEX_NUM);
  for(j = 0; j < 3; j++)
    rn[j] = vec[0][j];
  ks_normalize_vector(rn,3);
  glBegin(GL_POLYGON);
  glNormal3dv(rn);
  for(j = 0; j < KS_GL_RIBBON_VERTEX_NUM; j++){
    glVertex3d(gb->ribbon_v[1][KS_GL_RIBBON_VERTEX_NUM-1-j][0]+pos[1][0],
	       gb->ribbon_v[1][KS_GL_RIBBON_VERTEX_NUM-1-j][1]+pos[1][1],
	       gb->ribbon_v[1][KS_GL_RIBBON_VERTEX_NUM-1-j][2]+pos[1][2]);
  }
  glEnd();
  glPopName();

  for(j = 0; j < 3; j++) em[j] = 0.0;
  glMaterialfv(GL_FRONT,GL_EMISSION,em);
}
BOOL ks_draw_gl_ribbon(KS_GL_BASE *gb, KS_GL_BUFFER_3D *ribbon, int color_type, 
		       GLfloat *alpha_color, GLfloat *beta_color, GLfloat *loop_color)
{
  int i,j;
  int div = 8;
  enum {RIBBON_LOOP,RIBBON_ALPHA,RIBBON_BETA};
  double pos[3];
  int color_pos;
  int beta_c, beta_s, beta_num;

  if(ribbon->num == 0)
    return KS_FALSE;

  /*
  for(i = 0; i < ribbon->num; i++){
    printf("%d %d %-5s %-5s %f %f %f %f %f %f\n",i,ribbon->label[i],
	   ribbon->flags[i]&KS_GL_RESIDUE_NORMAL ? "NORMAL":
	   ribbon->flags[i]&KS_GL_RESIDUE_START ? "START":
	   ribbon->flags[i]&KS_GL_RESIDUE_END ? "END":"NON",
	   ribbon->flags[i]&KS_GL_RESIDUE_ALPHA ? "ALPHA":
	   ribbon->flags[i]&KS_GL_RESIDUE_BETA  ? "BETA" :
	   ribbon->flags[i]&KS_GL_RESIDUE_LOOP  ? "LOOP" : "NON",
	   ribbon->x[i],ribbon->y[i],ribbon->z[i],
	   ribbon->nx[i],ribbon->ny[i],ribbon->nz[i]);
	   
  }
  ks_exit(EXIT_FAILURE);
  */

  if(gb->hokan == NULL){
    if((gb->hokan = ks_allocate_gl_buffer_3d(0,2,1,
					  KS_GL_BUFFER_3D_DEFAULT|
					  KS_GL_BUFFER_3D_NORMAL|
					  KS_GL_BUFFER_3D_SPLINE,"buf")) == NULL){
      return KS_FALSE;
    }
  }

  make_spline_table3(ribbon);
  /*
  for(i = 0; i < ribbon->num; i++){
    printf("%d %f %f %f %f %f %f %f\n",i,ribbon->p[i]
	   ,ribbon->x[i],ribbon->y[i],ribbon->z[i]
	   ,ribbon->a[i],ribbon->b[i],ribbon->c[i]
	   );
  }
  ks_exit(EXIT_FAILURE);
  */
#if 0
  /*
  for(i = 0; i < ribbon->num; i++){
    if(ribbon->flags[i]&KS_GL_RESIDUE_LOOP){
	color_type = RIBBON_LOOP;
    } else if(ribbon->flags[i]&KS_GL_RESIDUE_ALPHA){
	color_type = RIBBON_ALPHA;
    } else if(ribbon->flags[i]&KS_GL_RESIDUE_BETA){
      color_type = RIBBON_BETA;
    }
    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,color[color_type]);
    glPushMatrix();
    glTranslated(ribbon->x[i],ribbon->y[i],ribbon->z[i]);
    glutSolidSphere(0.3,10,5);
    glPopMatrix();
  }
  */
  beta_num = 0; beta_c = 0; beta_s = -1;
  for(i = 0; i < (int)ribbon->num-1; i++){
    if(ribbon->flags[i-1]&KS_GL_RESIDUE_BETA && !(ribbon->flags[i]&KS_GL_RESIDUE_BETA)){
      beta_num = i-beta_s+1;
      for(beta_c = 0; beta_c < beta_num; beta_c++){
	/*	printf("%d %d %d\n",beta_c,beta_s,beta_num);*/
	ks_calc_Bezier(&ribbon->x[beta_s],&ribbon->y[beta_s],&ribbon->z[beta_s],beta_num,
		       (double)(beta_c)/(beta_num-1),pos);
	ribbon->x[beta_c+beta_s] = pos[0];
	ribbon->y[beta_c+beta_s] = pos[1];
	ribbon->z[beta_c+beta_s] = pos[2];
      }
    }
    if(ribbon->flags[i+1]&KS_GL_RESIDUE_BETA && !(ribbon->flags[i]&KS_GL_RESIDUE_BETA)){
      beta_s = i;
    }
  }

#endif
#if 1
  beta_num = 0; beta_c = 0; beta_s = -1;
  for(i = 0; i < (int)ribbon->num-1; i++){
    if(ribbon->flags[i]&KS_GL_RESIDUE_BETA && !(ribbon->flags[i+1]&KS_GL_RESIDUE_BETA)){
      beta_num = i-beta_s+1;
      for(beta_c = 0; beta_c < beta_num-1; beta_c++){
	/*	printf("beta %d %d %d %d\n",beta_c,beta_s,beta_num,beta_c+beta_s);*/
	for(j = 0; j < /*(beta_c == beta_num-1 ? div-1:div)*/div; j++){
	  ks_calc_Bezier(&ribbon->x[beta_s],&ribbon->y[beta_s],&ribbon->z[beta_s],beta_num,
			 (double)(beta_c*div+j)/((beta_num-1)*(div)),pos);
	  if(j < div*.5)
	    color_pos = beta_c+beta_s;
	  else
	    color_pos = beta_c+beta_s+1;
	  ks_set_gl_buffer_3d(gb->hokan,pos[0],pos[1],pos[2],
			      ribbon->nx[beta_c+beta_s],
			      ribbon->ny[beta_c+beta_s],
			      ribbon->nz[beta_c+beta_s],
			      ribbon->color[color_pos],
			      ribbon->label[color_pos],
			      ribbon->flags[color_pos/*beta_c+beta_s*/]);
	  /*
	  printf("b %d %d %d %f %f %f %f\n",beta_c+beta_s,i,j,
		 (double)(beta_c*div+j)/((beta_num-1)*div),pos[0],pos[1],pos[2]);
	  */
	}
      }
      beta_s = -1;
    }
    if(ribbon->flags[i]&KS_GL_RESIDUE_BETA && !(ribbon->flags[i-1]&KS_GL_RESIDUE_BETA)){
      beta_s = i;
    }
    /*
    printf("%d %d %-5s %-5s %d %d %d\n",i,ribbon->label[i],
	   ribbon->flags[i]&KS_GL_RESIDUE_NORMAL ? "NORMAL":
	   ribbon->flags[i]&KS_GL_RESIDUE_START ? "START":
	   ribbon->flags[i]&KS_GL_RESIDUE_END ? "END":"NON",
	   ribbon->flags[i]&KS_GL_RESIDUE_ALPHA ? "ALPHA":
	   ribbon->flags[i]&KS_GL_RESIDUE_BETA  ? "BETA" :
	   ribbon->flags[i]&KS_GL_RESIDUE_LOOP  ? "LOOP" : "NON",
	   beta_c,beta_s,beta_num);
    */
    if(beta_s == -1){
      double t;
      /*      printf("alpha %d\n",i);*/
      for(j = 0; j < div; j++){
	t = (double)j/(div);

	spline3(ribbon->p[i]*(1-t)+ribbon->p[i+1]*t,
		&pos[0],&pos[1],&pos[2],&color_pos,ribbon);
	/*
	printf("a %d %d %d %f %f %f %f\n",color_pos,i,j
	       ,ribbon->p[i]*(1-t)+ribbon->p[i+1]*t
	       ,pos[0],pos[1],pos[2]);
	*/
	if(j < div*.5)
	  color_pos = i;
	else
	  color_pos = i+1;
	ks_set_gl_buffer_3d(gb->hokan,pos[0],pos[1],pos[2],0,0,0,
			    ribbon->color[color_pos],
			    ribbon->label[color_pos],
			    ribbon->flags[color_pos/*i*/]);
	/*
	printf("%d %f %f %f\n",gb->hokan->num-1
	       ,gb->hokan->x[gb->hokan->num-1]
	       ,gb->hokan->y[gb->hokan->num-1]
	       ,gb->hokan->z[gb->hokan->num-1]
	       );
	*/
      }
    }
  }
  /*  ks_exit(EXIT_FAILURE);*/
#else
  for(i = 0; i < (int)ribbon->num; i++){
    if(ribbon->flags[i]&KS_GL_RESIDUE_LOOP){
      color_type = RIBBON_LOOP;
    } else if(ribbon->flags[i]&KS_GL_RESIDUE_ALPHA){
      color_type = RIBBON_ALPHA;
    } else if(ribbon->flags[i]&KS_GL_RESIDUE_BETA){
      color_type = RIBBON_BETA;
    }
    for(j = 0; j < div; j++){
      spline3((double)(i*div+j)/(n-1),&pos[0],&pos[1],&pos[2],&color_pos,ribbon);
      ks_set_spline_buffer_3d(gb->hokan,pos[0],pos[1],pos[2],ribbon->color[color_pos],
			      ribbon->label[i]);
    }
  }
#endif
  /*
  for(i = 0; i < gb->hokan->num; i++){
    printf("%d %-5s %f %f %f %f %f %f\n",i,
	   gb->hokan->flags[i]&KS_GL_RESIDUE_ALPHA ? "ALPHA":
	   gb->hokan->flags[i]&KS_GL_RESIDUE_BETA  ? "BETA" :
	   gb->hokan->flags[i]&KS_GL_RESIDUE_LOOP  ? "LOOP" : "NON",
	   gb->hokan->x[i],gb->hokan->y[i],gb->hokan->z[i],
	   gb->hokan->nx[i],gb->hokan->ny[i],gb->hokan->nz[i]);
  }
  ks_exit(EXIT_FAILURE);
  */

  draw_ribbon(gb,ribbon,div,color_type,alpha_color,beta_color,loop_color);

  /*
  glLoadName(2);
  for(i = 0; i < ribbon->num; i++){
    glPushName(i);
    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,ribbon->color[i]);
    glPushMatrix();
    glTranslated(ribbon->x[i],ribbon->y[i],ribbon->z[i]);
    glutSolidSphere(0.4,10,5);
    glPopMatrix();
    glPopName();
    draw_normal_line(ribbon->x[i],ribbon->y[i],ribbon->z[i],
		     ribbon->nx[i],ribbon->ny[i],ribbon->nz[i],1,1,1);
  }
  */
  /*
  glLoadName(3);
  for(i = 0; i < gb->hokan->num; i++){
    glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,gb->hokan->color[i]);
    glPushName(gb->hokan->label[i]);
    glPushMatrix();
    glTranslated(gb->hokan->x[i],gb->hokan->y[i],gb->hokan->z[i]);
    glutSolidSphere(0.1,10,5);
    glPopMatrix();
    glPopName();
    if(gb->hokan->flags[i]&KS_GL_RESIDUE_BETA)
      draw_normal_line(gb->hokan->x[i],gb->hokan->y[i],gb->hokan->z[i],
		       gb->hokan->nx[i],gb->hokan->ny[i],gb->hokan->nz[i],1,1,1);
  }
  */

  ks_clear_gl_buffer_3d(gb->hokan);
  /*  glLoadName(0);*/
  return KS_TRUE;
}
#ifdef GLUT
void ks_draw_gl_color_sample_right(KS_GL_BASE *gb, GLfloat x, GLfloat y, 
				   KS_SPHERE *sphere, int font,
				   int y_max, GLfloat y_sep, 
				   GLfloat sphere_size, GLuint sphere_texture)
{
  int sample_num;
  int x_num,y_num;
  int ix = 0,iy = 0;
  KS_SPHERE *p;
  int len, len_max, len_total;
  GLfloat pos[2];

  ks_use_glut_font(gb,font);
  sample_num = ks_count_sphere(sphere);
  x_num = sample_num/y_max;
  y_num = sample_num%y_max;
  if(y_num != 0) x_num++;
  p = sphere;
  /*    printf("%d %d\n",x_num,y_max);*/
  len_total = 0;
  for(ix = 0; ix < x_num; ix++){
    len_max = 0;
    for(iy = 0; iy < y_max && p != NULL; iy++){
      ks_get_glut_font_width(gb,font,p->name,&len);
      /*      printf("%d %d %s %d\n",ix,iy,p->name,len);*/
      if(len_max < len) len_max = len;
      p = p->next;
    }
    len_total += len_max;
    /*    printf("max %d %d\n",len_max,len_total);*/
  }
  /*  printf("total %d\n",len_total);*/
  len_total += sphere_size*3.0*(x_num-1);
  /*  printf("%d %f %d\n",len_total,sphere_size,x_num);*/

  p = sphere;
  for(ix = 0; ix < x_num; ix++){
    len_max = 0;
    for(iy = 0; iy < y_max && p != NULL; iy++){
      /*      printf("total %d\n",len_total);*/
      /*      printf("%s %f %f %f\n",p->name,p->color[0],p->color[1],p->color[2]);*/
      glColor4f(p->color[0],p->color[1],p->color[2],1.0);
      pos[0] = x-len_total;
      pos[1] = y+y_sep*(-iy);
      /*      printf("%f %f %d %d\n",pos[0],pos[1],len_total,y_sep*iy);*/
      ks_draw_gl_texture_2d(sphere_texture,
			    pos[0]-sphere_size*p->radius*.5,
			    pos[1]-sphere_size*p->radius*.5,
			    sphere_size*p->radius,sphere_size*p->radius);
      ks_gl_base_foreground_color(gb);
      glRasterPos2i(pos[0]+sphere_size*1.5,pos[1]-3);
      glCallLists((int)strlen(p->name), GL_BYTE, p->name);
      if(len_max < len) len_max = len;
      p = p->next;
    }
    len_total -= len_max+sphere_size*3.0;
  }
}
#endif
double ks_get_double_click_time(KS_GL_BASE *gb)
{
  return gb->double_click_time;
}
void ks_set_double_click_time(KS_GL_BASE *gb, double t)
{
  gb->double_click_time = t;
}
BOOL ks_check_double_click_time(KS_GL_BASE *gb, double t)
{
  if(gb->double_click_time > t) 
    return KS_TRUE;
  else
    return KS_FALSE;
}
BOOL ks_is_gl_double_click(KS_GL_BASE *gb, int button, int state)
{
  /*
  printf("%f %f\n"
	 ,gb->mouse_time[KS_GL_MOUSE_TIME_LAP][button][state]
	 ,gb->mouse_time[KS_GL_MOUSE_TIME_LAP][button][state]);
  */
  if((gb->mouse_time[KS_GL_MOUSE_TIME_LAP][button][state]) < gb->double_click_time)
    return KS_TRUE;
  else
    return KS_FALSE;
}
int ks_get_gl_mouse_state(KS_GL_BASE *gb, int button)
{
  return gb->mouse_state[button];
}
void ks_draw_gl_move_target_plane(KS_GL_BASE *gb, double plane_size, int line_num, 
				  GLfloat color0, GLfloat color1, GLfloat color2)
{
  int i;
  double axis[3][3] = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0}};
  double gl_axis[3][3];
  double angle[3];
  double line_sep;
  GLfloat line_color[4] = {0.5,0.5,0.5,1.0};
  GLfloat cross_color[4] = {0.9,0.9,0.9,1.0};
  double cross_size = 0.3;
  double tcd[3];
  GLboolean lighting;

  if(color0 >= 0){
    line_color[0] = color0;
  }
  if(color1 >= 0){
    line_color[1] = color1;
  }
  if(color2 >= 0){
    line_color[2] = color2;
  }

  line_sep = plane_size / line_num;
  plane_size *= 0.5;

  for(i = 0; i < 3; i++){
    ks_multi_gl_rotational_matrix(*gb,axis[i],gl_axis[i]);
    /*    printf("%d %f %f %f\n",i,gl_axis[i][0],gl_axis[i][1],gl_axis[i][2]);*/
    angle[i] = ks_calc_vectors_angle(axis[0],gl_axis[i]);
  }
  for(i = 0; i < 3; i++){
    if(angle[i] > M_PI*.5){
      angle[i] = M_PI-angle[i];
    }
  }
  /*  printf("%f %f %f\n",angle[0]/M_PI*180,angle[1]/M_PI*180,angle[2]/M_PI*180);*/
  if(gb->move_target != NULL){
    for(i = 0; i < 3; i++)
      tcd[i] = gb->move_target_initial[i];
    /*      tcd[i] = gb->move_target_pos[i] + gb->move_target_initial[i];*/
    /*
    for(i = 0; i < 3; i++)
      gb->rotation_center[i] = tcd[i];
    */
    lighting = ks_gl_disable(GL_LIGHTING);
    glLineWidth(1.0);
    glColor4fv(line_color);
    if(angle[0] < angle[1] && angle[0] < angle[2]){
      gb->move_target_plane = KS_GL_PLANE_X;
      glBegin(GL_LINE_LOOP);
      glVertex3f(tcd[0],tcd[1]+plane_size,tcd[2]+plane_size);
      glVertex3f(tcd[0],tcd[1]+plane_size,tcd[2]-plane_size);
      glVertex3f(tcd[0],tcd[1]-plane_size,tcd[2]-plane_size);
      glVertex3f(tcd[0],tcd[1]-plane_size,tcd[2]+plane_size);
      glEnd();
      glBegin(GL_LINES);
      for(i = 1; i < line_num; i++){
	glVertex3f(tcd[0],tcd[1]-plane_size+i*line_sep,tcd[2]-plane_size);
	glVertex3f(tcd[0],tcd[1]-plane_size+i*line_sep,tcd[2]+plane_size);
      }
      for(i = 1; i < line_num; i++){
	glVertex3f(tcd[0],tcd[1]-plane_size,tcd[2]-plane_size+i*line_sep);
	glVertex3f(tcd[0],tcd[1]+plane_size,tcd[2]-plane_size+i*line_sep);
      }
      glEnd();
    } else if(angle[1] < angle[2]){
      gb->move_target_plane = KS_GL_PLANE_Y;
      glBegin(GL_LINE_LOOP);
      glVertex3f(tcd[0]+plane_size,tcd[1],tcd[2]+plane_size);
      glVertex3f(tcd[0]+plane_size,tcd[1],tcd[2]-plane_size);
      glVertex3f(tcd[0]-plane_size,tcd[1],tcd[2]-plane_size);
      glVertex3f(tcd[0]-plane_size,tcd[1],tcd[2]+plane_size);
      glEnd();
      glBegin(GL_LINES);
      for(i = 1; i < line_num; i++){
	glVertex3f(tcd[0]-plane_size+i*line_sep,tcd[1],tcd[2]-plane_size);
	glVertex3f(tcd[0]-plane_size+i*line_sep,tcd[1],tcd[2]+plane_size);
      }
      for(i = 1; i < line_num; i++){
	glVertex3f(tcd[0]-plane_size,tcd[1],tcd[2]-plane_size+i*line_sep);
	glVertex3f(tcd[0]+plane_size,tcd[1],tcd[2]-plane_size+i*line_sep);
      }
      glEnd();
    } else {
      gb->move_target_plane = KS_GL_PLANE_Z;
      glBegin(GL_LINE_LOOP);
      glVertex3f(tcd[0]+plane_size,tcd[1]+plane_size,tcd[2]);
      glVertex3f(tcd[0]+plane_size,tcd[1]-plane_size,tcd[2]);
      glVertex3f(tcd[0]-plane_size,tcd[1]-plane_size,tcd[2]);
      glVertex3f(tcd[0]-plane_size,tcd[1]+plane_size,tcd[2]);
      glEnd();
      glBegin(GL_LINES);
      for(i = 1; i < line_num; i++){
	glVertex3f(tcd[0]-plane_size+i*line_sep,tcd[1]-plane_size,tcd[2]);
	glVertex3f(tcd[0]-plane_size+i*line_sep,tcd[1]+plane_size,tcd[2]);
      }
      for(i = 1; i < line_num; i++){
	glVertex3f(tcd[0]-plane_size,tcd[1]-plane_size+i*line_sep,tcd[2]);
	glVertex3f(tcd[0]+plane_size,tcd[1]-plane_size+i*line_sep,tcd[2]);
      }
      glEnd();
    }
    glLineWidth(4.0);
    glColor4fv(cross_color);
    glBegin(GL_LINES);
    glVertex3f(tcd[0]-cross_size,tcd[1],tcd[2]);
    glVertex3f(tcd[0]+cross_size,tcd[1],tcd[2]);
    glVertex3f(tcd[0],tcd[1]-cross_size,tcd[2]);
    glVertex3f(tcd[0],tcd[1]+cross_size,tcd[2]);
    glVertex3f(tcd[0],tcd[1],tcd[2]-cross_size);
    glVertex3f(tcd[0],tcd[1],tcd[2]+cross_size);
    glEnd();
    glLineWidth(1.0);
    if(lighting == GL_TRUE)
      glEnable(GL_LIGHTING);
  }
}
void ks_set_gl_move_target(KS_GL_BASE *gb, double *target, double *target_pos, double motion_scale)
{
  int i;
  gb->move_target = target;
  gb->move_target_pos = target_pos;
  if(target != NULL){
    for(i = 0; i < 3; i++)
      gb->move_target_initial[i] = target_pos[i];
  }
  gb->move_target_motion_scale = motion_scale;
}
void ks_clear_gl_move_target(KS_GL_BASE *gb)
{
  gb->move_target = NULL;
}
void ks_set_gl_move_target_motion(KS_GL_BASE *gb, BOOL mode)
{
  gb->move_target_motion = mode;
}
BOOL ks_is_gl_move_target_motion(KS_GL_BASE *gb)
{
  return gb->move_target_motion;
}
void ks_set_gl_rotation_center(KS_GL_BASE *gb, double *pos)
{

  double opos[3];
  double rpos[3];
  opos[0] = pos[0] - gb->rotation_center[0];
  opos[1] = pos[1] - gb->rotation_center[1];
  opos[2] = pos[2] - gb->rotation_center[2];
  ks_multi_gl_rotational_matrix(*gb, opos,rpos);

  gb->rotation_center_offset[0] += opos[0] - rpos[0];
  gb->rotation_center_offset[1] += opos[1] - rpos[1];
  gb->rotation_center_offset[2] += opos[2] - rpos[2];
  /*
  printf("(%f %f %f)(%f %f %f)(%f %f %f)\n",opos[0],opos[1],opos[2],rpos[0],rpos[1],rpos[2],
	 gb->rotation_center_offset[0],
	 gb->rotation_center_offset[1],
	 gb->rotation_center_offset[2]
	 );
  */
  gb->rotation_center[0] = pos[0];
  gb->rotation_center[1] = pos[1];
  gb->rotation_center[2] = pos[2];

}
void ks_set_gl_lookat_position(KS_GL_BASE *gb, double *pos)
{
  int i;
  /*
  double opos[3],rpos[3];
  for(i = 0; i < 3; i++)
    opos[i] = pos[i]-(gb->rotation_center[i]);
  ks_multi_gl_rotational_matrix(*gb, opos,rpos);
  for(i = 0; i < 3; i++)
    rpos[i] += gb->rotation_center[i]-gb->rotation_center_offset[i];

  if(gb->lookat_moving == KS_TRUE){
    for(i = 0; i < 3; i++)
      gb->lookat[KS_GL_LOOKAT_NEXT][i] = rpos[i];
  } else {
    for(i = 0; i < 3; i++)
      gb->lookat[KS_GL_LOOKAT_NOW][i] = rpos[i];
  }
  */
  if(gb->lookat_moving == KS_TRUE){
    for(i = 0; i < 3; i++){
      gb->lookat[KS_GL_LOOKAT_NEXT][i] = pos[i]-gb->rotation_center_offset[i];
    }
  } else {
    for(i = 0; i < 3; i++){
      gb->lookat[KS_GL_LOOKAT_NOW][i] = pos[i]-gb->rotation_center_offset[i];;
    }
  }
}
static void change_lookat(KS_GL_BASE *gb, double *pos, double *trans, double time)
{
  int i;
  double opos[3],rpos[3];
  gb->lookat_moving = KS_TRUE;
  gb->lookat_moving_start_time = ks_get_time();
  gb->lookat_moving_time = time;
  for(i = 0; i < 3; i++)
    gb->lookat[KS_GL_LOOKAT_PREV][i] = gb->lookat[KS_GL_LOOKAT_NOW][i];
  for(i = 0; i < 3; i++)
    opos[i] = pos[i]-(gb->rotation_center[i]);
  ks_multi_gl_rotational_matrix(*gb, opos,rpos);
  for(i = 0; i < 3; i++)
    rpos[i] += gb->rotation_center[i]-gb->rotation_center_offset[i];
  for(i = 0; i < 3; i++)
    gb->lookat[KS_GL_LOOKAT_NEXT][i] = rpos[i];
  for(i = 0; i < 3; i++)
    gb->trans_prev[i] = gb->trans[i];
  for(i = 0; i < 3; i++)
    gb->trans_next[i] = trans[i];
}
void ks_change_gl_lookat(KS_GL_BASE *gb, double *pos, double time)
{
  double trans[3] = {0,0,0};
  change_lookat(gb,pos,trans,time);
}
static void move_lookat(KS_GL_BASE *gb)
{
  int i;
  double t;

  t = (ks_get_time()-gb->lookat_moving_start_time)/gb->lookat_moving_time*M_PI;
  /*  printf("%f\n",t);*/
  if(t <= M_PI){
    t = (-cos(t)+1)*.5;
    for(i = 0; i < 3; i++){
      gb->lookat[KS_GL_LOOKAT_NOW][i] = ((1-t)*gb->lookat[KS_GL_LOOKAT_PREV][i] +
					 t*    gb->lookat[KS_GL_LOOKAT_NEXT][i]);
    }
    for(i = 0; i < 3; i++){
      gb->trans[i] = ((1-t)*gb->trans_prev[i] +
		      t    *gb->trans_next[i]);
    }
    /*
    printf("%f (%f %f %f) (%f %f %f)\n",1-t,
	   gb->trans[0],gb->trans[1],gb->trans[2],
	   gb->trans_prev[0],gb->trans_prev[1],gb->trans_prev[2]);
    */   
  } else {
    gb->lookat_moving = KS_FALSE;
  }
}
void ks_push_gl_lookat(KS_GL_BASE *gb)
{
  int i;
  for(i = 0; i < 3; i++){
    gb->lookat_stack[i] = gb->lookat[KS_GL_LOOKAT_NOW][i];
  }
  for(i = 0; i < 3; i++){
    gb->trans_stack[i] = gb->trans[i];
  }
}
void ks_pop_gl_lookat(KS_GL_BASE *gb)
{
  /*
  printf("%f %f %f  %f %f %f\n",
	 gb->lookat_stack[0],gb->lookat_stack[1],gb->lookat_stack[2],
	 gb->trans_stack[0],gb->trans_stack[1],gb->trans_stack[2]);
  */
  change_lookat(gb,gb->lookat_stack,gb->trans_stack,1.0);
}
void ks_gl_base_idle(KS_GL_BASE *gb)
{
  gb->time = ks_get_time();
  if(gb->lookat_moving == KS_TRUE){
    move_lookat(gb);
    ks_redraw_gl(gb);
  }
}
BOOL ks_is_gl_lookat_moving(KS_GL_BASE *gb)
{
  return gb->lookat_moving;
}
double ks_get_gl_time(KS_GL_BASE *gb)
{
  return gb->time;
}
void ks_redraw_gl(KS_GL_BASE *gb)
{
#ifdef GLUT
  glutPostRedisplay();
#elif defined(MSVC)
  /*  Invalidate(FALSE);*/
#else
  ks_assert(0);
#endif
}
void ks_add_gl_rotate(KS_GL_BASE *gb, double mat[16])
{
  glTranslated(gb->rotation_center[0],gb->rotation_center[1],gb->rotation_center[2]);
  glMultMatrixd(mat);
  glTranslated(-gb->rotation_center[0],-gb->rotation_center[1],-gb->rotation_center[2]);
}
void ks_gl_base_set_display_function(KS_GL_BASE *gb,void (*display)(void))
{
  gb->display_func = display;
}
void ks_draw_gl(KS_GL_BASE *gb)
{
  if(gb->display_func != NULL)
    (*gb->display_func)();
}
void ks_increase_gl_base_verbose_level(KS_GL_BASE *gb, int x, int y, void *vp)
{
  if(gb->verbose_level == gb->verbose_level_max){
    gb->verbose_level = 0;
  } else {
    gb->verbose_level++;
  }
}
void ks_decrease_gl_base_verbose_level(KS_GL_BASE *gb, int x, int y, void *vp)
{
  if(gb->verbose_level == 0){
    gb->verbose_level = gb->verbose_level_max;
  } else {
    gb->verbose_level--;
  }
}
void ks_set_gl_base_verbose_level(KS_GL_BASE *gb, unsigned int level)
{
  gb->verbose_level = level;
}
void ks_set_gl_base_verbose_level_max(KS_GL_BASE *gb, unsigned int max)
{
  gb->verbose_level_max = max;
}
char *ks_get_gl_key_name(int key)
{
  static char name[16];
  name[0] = '\0';
  if(     key == KS_GL_KEY_F1) 	      strcpy(name,"F1");
  else if(key == KS_GL_KEY_F2) 	      strcpy(name,"F2");
  else if(key == KS_GL_KEY_F3) 	      strcpy(name,"F3");
  else if(key == KS_GL_KEY_F4) 	      strcpy(name,"F4");
  else if(key == KS_GL_KEY_F5) 	      strcpy(name,"F5");
  else if(key == KS_GL_KEY_F6) 	      strcpy(name,"F6");
  else if(key == KS_GL_KEY_F7) 	      strcpy(name,"F7");
  else if(key == KS_GL_KEY_F8) 	      strcpy(name,"F8");
  else if(key == KS_GL_KEY_F9) 	      strcpy(name,"F9");
  else if(key == KS_GL_KEY_F10)       strcpy(name,"F10");
  else if(key == KS_GL_KEY_F11)       strcpy(name,"F11");
  else if(key == KS_GL_KEY_F12)       strcpy(name,"F12");
  else if(key == KS_GL_KEY_LEFT)      strcpy(name,"LEFT");
  else if(key == KS_GL_KEY_UP)        strcpy(name,"UP");
  else if(key == KS_GL_KEY_RIGHT)     strcpy(name,"RIGHT");
  else if(key == KS_GL_KEY_DOWN)      strcpy(name,"DOWN");
  else if(key == KS_GL_KEY_PAGE_UP)   strcpy(name,"PGUP");
  else if(key == KS_GL_KEY_PAGE_DOWN) strcpy(name,"PGDN");
  else if(key == KS_GL_KEY_HOME)      strcpy(name,"HOME");
  else if(key == KS_GL_KEY_END)       strcpy(name,"END");
  else if(key == KS_GL_KEY_INSERT)    strcpy(name,"INS");
  return name;
}
void ks_save_gl_pov_header(KS_GL_BASE *gb, FILE *fp)
{
  double light_pos[3] = {-100000,100000,100000};

  fprintf(fp,"camera{\n");
  if(ks_gl_base_get_projection_mode(gb) == KS_GL_BASE_ORTHO){
    /*
    double len = (gb->trans[0]-gb->eye_len)*100./570.+330*100./570.-100.0;
    */
    double len = (gb->eye_len-gb->trans[0])*tan(gb->eye_ang*.5/180*M_PI)*2;
    /*    printf("%f %f %f\n",gb->trans[0],gb->eye_len,len);*/
    fprintf(fp," orthographic\n");
    fprintf(fp," location <-%f,0,0>\n",len);
    fprintf(fp," right <%f,0,0>\n",len);
    fprintf(fp," up <0,%f,0>\n",len);
  } else if(ks_gl_base_get_projection_mode(gb) == KS_GL_BASE_PERSPECTIVE){
    fprintf(fp," location <%f,%f,%f>\n",-gb->eye_len,0.0,0.0);
    fprintf(fp," right <1,0,0>\n");
    fprintf(fp," up <0,1,0>\n");
    fprintf(fp," angle %f\n",gb->eye_ang);
  }
  fprintf(fp," sky <0,0,1>\n");
  fprintf(fp," look_at <0,0,0>\n");
  fprintf(fp,"}\n");
  fprintf(fp,"light_source {<%f,%f,%f> color rgb<1,1,1>}\n",
	  light_pos[0],light_pos[1],light_pos[2]);
  fprintf(fp,"background {color rgb<%f,%f,%f>}\n"
	  ,gb->background_color[0],gb->background_color[1],gb->background_color[2]);

  fprintf(fp,"union{\n");

}
void ks_save_gl_pov_footer(KS_GL_BASE *gb, FILE *fp, BOOL no_shadow)
{
  fprintf(fp,"translate<% f,% f,% f>\n"
	  ,-gb->rotation_center[0],-gb->rotation_center[1],-gb->rotation_center[2]);

  fprintf(fp," matrix<%f,%f,%f,\n",-gb->rot_mat[0],gb->rot_mat[1],gb->rot_mat[2]);
  fprintf(fp,"        %f,%f,%f,\n",-gb->rot_mat[4],gb->rot_mat[5],gb->rot_mat[6]);
  fprintf(fp,"        %f,%f,%f,\n",-gb->rot_mat[8],gb->rot_mat[9],gb->rot_mat[10]);
  fprintf(fp,"        %f,%f,%f>\n",-gb->trans[0],gb->trans[1],gb->trans[2]);
  if(no_shadow == KS_TRUE){
    fprintf(fp,"no_shadow\n");
  }
  fprintf(fp,"}\n");
}
void ks_gl_base_stop_x_rotate(KS_GL_BASE *gb)
{
  gb->angle[2] = 0;
}
void ks_gl_base_stop_y_rotate(KS_GL_BASE *gb)
{
  gb->angle[1] = 0;
}
void ks_gl_base_stop_z_rotate(KS_GL_BASE *gb)
{
  gb->angle[0] = 0;
}
int *ks_get_gl_viewport_3d(KS_GL_BASE *gb)
{
  return gb->viewport_3d;
}
int ks_get_gl_viewport_3d_width(KS_GL_BASE *gb)
{
  return gb->viewport_3d[KS_GL_VIEWPORT_WIDTH];
}
int ks_get_gl_viewport_3d_height(KS_GL_BASE *gb)
{
  return gb->viewport_3d[KS_GL_VIEWPORT_HEIGHT];
}
int *ks_get_gl_viewport_2d(KS_GL_BASE *gb)
{
  return gb->viewport_2d;
}
int ks_get_gl_viewport_2d_width(KS_GL_BASE *gb)
{
  return gb->viewport_2d[KS_GL_VIEWPORT_WIDTH];
}
int ks_get_gl_viewport_2d_height(KS_GL_BASE *gb)
{
  return gb->viewport_2d[KS_GL_VIEWPORT_HEIGHT];
}
void ks_io_gl_base(KS_IO io, KS_GL_BASE *gb)
{
  ks_io(io,gb->trans,sizeof(double),3);
  ks_io(io,gb->rot_mat,sizeof(double),16);
  ks_io(io,gb->rot_imat,sizeof(double),16);
  ks_io(io,gb->foreground_color,sizeof(GLfloat),4);
  ks_io(io,gb->background_color,sizeof(GLfloat),4);
  ks_io(io,gb->rotation_center,sizeof(double),3);
  ks_io(io,gb->rotation_center_offset,sizeof(double),3);
  ks_io(io,gb->lookat,sizeof(double),KS_GL_LOOKAT_NUM*3);
  ks_io(io,gb->trans_prev,sizeof(double),3);
}
void ks_set_gl_base_depth_3d_near(KS_GL_BASE *gb, GLdouble d)
{
  gb->depth_3d_near = d;
}
void ks_set_gl_base_depth_3d_far(KS_GL_BASE *gb, GLdouble d)
{
  gb->depth_3d_far = d;
}
void ks_set_gl_base_depth_2d_near(KS_GL_BASE *gb, GLdouble d)
{
  gb->depth_2d_near = d;
}
void ks_set_gl_base_depth_2d_far(KS_GL_BASE *gb, GLdouble d)
{
  gb->depth_2d_far = d;
}
void ks_get_gl_eye_pos(KS_GL_BASE *gb, double rpos[3])
{
  int i;
  double pos[3];
  for(i = 0;i < 3; i++){
    pos[i] = -gb->trans[i];
  }
  pos[0] += gb->eye_len;
  ks_multi_gl_inverse_rotational_matrix(*gb,pos,rpos);
}
double ks_get_gl_perspective_angle(KS_GL_BASE *gb)
{
  return gb->eye_ang;
}
void ks_set_gl_perspective_angle(KS_GL_BASE *gb, double angle)
{
  gb->eye_ang = angle;
}
void ks_set_gl_base_eye_len(KS_GL_BASE *gb, double len)
{
  gb->eye_len = len;
}
double ks_get_gl_base_eye_len(KS_GL_BASE *gb)
{
  return gb->eye_len;
}
void ks_set_gl_base_eye_width(KS_GL_BASE *gb, double width)
{
  gb->eye_width = width;
}
double ks_get_gl_base_eye_width(KS_GL_BASE *gb)
{
  return gb->eye_width;
}
double *ks_get_gl_modelview_matrix(KS_GL_BASE *gb, double mat[16])
{
  single_display(gb,NULL,KS_GL_STEREO_NOT_USE,NULL,mat);
  return mat;
}
KS_GL_SPHERE_OBJ *ks_allocate_gl_sphere_obj(int detail)
{
  int i,j;
  double x,y;
  double sz[2],cz[2];
  double theta,phi;
  KS_GL_SPHERE_OBJ *obj;

  if((obj = (KS_GL_SPHERE_OBJ*)ks_malloc(sizeof(KS_GL_SPHERE_OBJ),"sphere_obj")) == NULL){
    ks_error_memory();
    return NULL;
  }

  obj->s_num1 = detail;
  obj->s_num2 = detail*2;

  if((obj->cd = ks_malloc_double_ppp(obj->s_num1+1,obj->s_num2+1,3,"sphere_obj cd")) == NULL){
    ks_error_memory();
    return NULL;
  }

  theta = M_PI/(obj->s_num1);
  phi = 2.0*M_PI/(obj->s_num2);

  for(i = 0; i < obj->s_num1+1; i++){
    sz[0] = sin(theta*i);
    cz[0] = cos(theta*i);
    for(j = 0; j < obj->s_num2+1; j++){
      x = cos(phi*j); y = sin(phi*j);
      obj->cd[i][j][0] = sz[0]*x;
      obj->cd[i][j][1] = sz[0]*y;
      obj->cd[i][j][2] = cz[0];
    }
  }
  return obj;
}
void ks_free_gl_sphere_obj(KS_GL_SPHERE_OBJ* obj)
{
  ks_free_double_ppp(obj->s_num1+1,obj->s_num2+1,obj->cd);
  ks_free(obj);
}
void ks_draw_gl_sphere_obj(KS_GL_SPHERE_OBJ* obj, double r)
{
  int i,j;
  for(i = 0; i < obj->s_num1; i++){
    glBegin(GL_TRIANGLE_STRIP);
    for(j = 0; j < obj->s_num2+1; j++){
      glNormal3d(  obj->cd[i][j][0],  obj->cd[i][j][1],  obj->cd[i][j][2]);
      glVertex3d(r*obj->cd[i][j][0],r*obj->cd[i][j][1],r*obj->cd[i][j][2]);
      glNormal3d(  obj->cd[i+1][j][0],  obj->cd[i+1][j][1],  obj->cd[i+1][j][2]);
      glVertex3d(r*obj->cd[i+1][j][0],r*obj->cd[i+1][j][1],r*obj->cd[i+1][j][2]);
    }
    glEnd();
  }
}
#ifdef GLUT
void ks_glutInit(int* acp, char** av)
{
#ifdef FREEGLUT
  // wrapper of glutInit for freeglut
  int i;
  BOOL have_display = KS_FALSE;
  int ac_glut;
  char **av_glut = NULL;
  for(i = 1; i < *acp; i++){
    if(strcmp(av[i],"-display") == 0){
      have_display = KS_TRUE;
    }
  }
  if(have_display == KS_FALSE){
    ac_glut = *acp + 2;
    if((av_glut = (char**)ks_malloc(ac_glut*sizeof(char*),"av_glut")) == NULL){
      ks_error_memory();
    }
    for(i = 0; i < *acp; i++){
      av_glut[i] = av[i];
    }
    av_glut[i++] = "-display"; // add option to prevent warning of -display in freeglut
    av_glut[i++] = "DISPLAY";
    glutInit(&ac_glut, av_glut);
  }
  *acp = ac_glut;
  for(i = 0; i < ac_glut; i++){
    av[i] = av_glut[i];
  }
  if(av_glut != NULL){
    ks_free(av_glut);
  }
#else
  glutInit(acp, av);
#endif
}
#endif
#ifdef USE_GLSL
static char* read_shader_file(const char* filename)
{
  size_t size;
  FILE* fp;
  char *content;

  if((fp = fopen(filename, "rb")) == NULL){
    ks_error_file(filename);
  }

  if(fseek(fp, 0, SEEK_END) == -1) return NULL;
  size = ftell(fp);
  if(size == -1) return NULL;
  if(fseek(fp, 0, SEEK_SET) == -1) return NULL;

  if((content = (char*) ks_malloc( (size_t) size +1 ,"content")) == NULL){
    return NULL;
  }
  fread(content, 1, (size_t)size, fp);

  if(ferror(fp)) {
    ks_free(content);
    return NULL;
  }

  fclose(fp);
  content[size] = '\0';
  return content;
}
static void print_glsl_log(GLuint object)
{
  GLint log_length = 0;
  char *log;
  if(glIsShader(object)){
    glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
  } else if(glIsProgram(object)){
    glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
  } else {
    ks_error("printlog: Not a shader or a program\n");
    return;
  }

  if((log = (char*)ks_malloc(log_length,"log")) == NULL){
    ks_error_memory();
    return;
  }

  if(glIsShader(object)){
    glGetShaderInfoLog(object, log_length, NULL, log);
  } else if(glIsProgram(object)){
    glGetProgramInfoLog(object, log_length, NULL, log);
  }

  fprintf(stderr, "%s", log);
  ks_free(log);
}
GLuint ks_make_gl_shader(const GLchar *shader_file, GLenum type)
{
  GLuint program;
  program = glCreateShader(type);
  glShaderSource(program, 1, &shader_file, NULL);

  glCompileShader(program);
  GLint compile_result = GL_FALSE;
  glGetShaderiv(program, GL_COMPILE_STATUS, &compile_result);
  if (compile_result == GL_FALSE) {
    fprintf(stderr, "%s:", shader_file);
    print_glsl_log(program);
    glDeleteShader(program);
    return 0;
  }
  return program;
}
GLuint ks_make_gl_shader_from_file(char* file_name, GLenum type)
{
  const GLchar *shader_file;
  GLuint program;

  if((shader_file = read_shader_file(file_name)) == NULL){
    return 0;
  }
  program = ks_make_gl_shader(shader_file,type);
  ks_free((void*)shader_file);
  return program;
}
KS_GL_STANDARD_SHADER_PARM* ks_allocate_gl_standard_shader_parm()
{
  KS_GL_STANDARD_SHADER_PARM *shader_parm;
  if((shader_parm = (KS_GL_STANDARD_SHADER_PARM*)ks_malloc(sizeof(KS_GL_STANDARD_SHADER_PARM),
							   "shader_parm")) == NULL){
    ks_error_memory();
    return NULL;
  }
  // set default parameters for shaders in ks_gl
  shader_parm->light_position[0] = 1.0f;
  shader_parm->light_position[1] = 1.1f;
  shader_parm->light_position[2] = 1.2f;
  shader_parm->light_ambient = 0.2;
  shader_parm->material_shininess = 5.0f;
  shader_parm->material_specular = 0.2f;
  return shader_parm;
}
static BOOL output_shader_program(int standard_shader_type, int pick_type, int pick_id_offset,
				  char *ext, char *buf)
{
  char output_file_mode[256];
  FILE *fp;
  sprintf(output_file_mode,"ks_gl_shader_program_st%d_pt%d_po%d.%s",
	  standard_shader_type,pick_type,pick_id_offset,ext);
  printf("output OpenGL shader program  -> %s\n",output_file_mode); fflush(stdout);
  if((fp = fopen(output_file_mode,"w")) == NULL){
    ks_error_file(output_file_mode);
    return KS_FALSE;
  }
  fprintf(fp,buf);
  fclose(fp);
  return KS_TRUE;
}
static GLuint make_gl_standard_shader_program(int standard_shader_type,
					      int color_mode, int pick_id_offset,
					      KS_GL_STANDARD_SHADER_PARM* shader_parm)
{
  KS_BUF_CHAR *bc;
  char str[256];
  GLuint program;
    enum {
    VERTEX_SHADER,
    FRAGMENT_SHADER,
    SHADER_CNT
  };
  GLuint shader_src[SHADER_CNT];

  if((bc = ks_allocate_buf_char(100,"bc")) == NULL){
    ks_error_memory();
  }
  ks_cat_buf_char(bc,"#version 330 core\n");
  ks_cat_buf_char(bc,"layout(location = 0) in vec3 vertexPosition;\n");
  if(standard_shader_type == KS_GL_STANDARD_SHADER_SPHERE ||
     standard_shader_type == KS_GL_STANDARD_SHADER_POINT){
     // (x,y,z,size) for instanced draw
    ks_cat_buf_char(bc,"layout(location = 1) in vec4 spherePosition;\n");
    // (r,g,b,emission) for instanced draw
    ks_cat_buf_char(bc,"layout(location = 2) in vec4 vertexColor;\n");
  } else if(standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
    ks_cat_buf_char(bc,"layout(location = 1) in vec2 uv;\n");
     // model view matrix for instanced draw
    ks_cat_buf_char(bc,"layout(location = 2) in mat4 modelMatrix;\n");
  } else if(standard_shader_type == KS_GL_STANDARD_SHADER_REGULAR){
    ks_cat_buf_char(bc,"layout(location = 1) in vec3 vertexNormal;\n");
    ks_cat_buf_char(bc,"layout(location = 2) in vec3 vertexColor;\n");
    ks_cat_buf_char(bc,"layout(location = 3) in mat4 modelMatrix;\n");
  }
  ks_cat_buf_char(bc,"uniform mat4 projectionMatrix;\n");
  ks_cat_buf_char(bc,"uniform mat4 viewMatrix;\n");
  ks_cat_buf_char(bc,"out vec3 fragmentColor;\n");
  if(standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
    ks_cat_buf_char(bc,"out vec2 fragmentUV;\n");
    ks_cat_buf_char(bc,"vec4 vertexColor = vec4(1.0,1.0,1.0,1.0);\n");
  }
  ks_cat_buf_char(bc,"void main(){\n");
  if(standard_shader_type == KS_GL_STANDARD_SHADER_SPHERE ||
     standard_shader_type == KS_GL_STANDARD_SHADER_POINT){
    ks_cat_buf_char(bc,"mat4 VP = projectionMatrix*viewMatrix;\n");
    ks_cat_buf_char(bc,"gl_Position =  VP*(vec4(vertexPosition*spherePosition.w,1)+"
		    "vec4(spherePosition.xyz,0));\n");
    if(color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
      ks_cat_buf_char(bc,"vec4 position = viewMatrix*vec4(vertexPosition,1);\n");
      ks_cat_buf_char(bc,"mat3 normalMatrix = mat3( transpose( inverse(viewMatrix) ) );\n");
      ks_cat_buf_char(bc,"vec3 normal = normalize(normalMatrix*vertexPosition);\n");
    }
  } else if(standard_shader_type == KS_GL_STANDARD_SHADER_REGULAR ||
	    standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
    ks_cat_buf_char(bc,"mat4 modelViewMatrix = viewMatrix*modelMatrix;\n");
    ks_cat_buf_char(bc,"mat4 MVP = projectionMatrix*modelViewMatrix;\n");
    ks_cat_buf_char(bc,"gl_Position =  MVP*vec4(vertexPosition,1);\n");
    if(color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
      ks_cat_buf_char(bc,"vec4 position = modelViewMatrix*vec4(vertexPosition,1);\n");
      ks_cat_buf_char(bc,"mat3 normalMatrix = mat3( transpose( inverse(modelViewMatrix) ) );\n");
      if(standard_shader_type == KS_GL_STANDARD_SHADER_REGULAR){
	ks_cat_buf_char(bc,"vec3 normal = normalize(normalMatrix*vertexNormal);\n");
      } else {
	ks_cat_buf_char(bc,"vec3 normal = normalize(normalMatrix*vertexPosition);\n");
      }
    }
  }
  if(standard_shader_type != KS_GL_STANDARD_SHADER_POINT &&
     color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
    sprintf(str,"vec4 lightPosition = vec4(%.1f,%.1f,%.1f,0.0);\n",
	    shader_parm->light_position[0],
	    shader_parm->light_position[1],
	    shader_parm->light_position[2]);
    ks_cat_buf_char(bc,str);
    ks_cat_buf_char(bc,"vec3 light = normalize((lightPosition*position.w - "
		    "lightPosition.w*position).xyz);\n");
    ks_cat_buf_char(bc,"float diffuse = max(dot(light, normal), 0.0);\n");
    ks_cat_buf_char(bc,"vec3 view = -normalize(position.xyz);\n");
    ks_cat_buf_char(bc,"vec3 halfway = normalize(light + view);\n");
    sprintf(str,"float materialShininess = %.1f;\n",shader_parm->material_shininess);
    ks_cat_buf_char(bc,str);
    ks_cat_buf_char(bc,"float specular = pow(max(dot(normal, halfway), 0.0),materialShininess);\n");
    sprintf(str,"vec3 lightAmbient = vec3(%.1f,%.1f,%.1f);\n",
	    shader_parm->light_ambient,shader_parm->light_ambient,shader_parm->light_ambient);
    ks_cat_buf_char(bc,str);
    sprintf(str,"vec3 materialSpecular = vec3(%.1f,%.1f,%.1f);\n",
	    shader_parm->material_specular,
	    shader_parm->material_specular,
	    shader_parm->material_specular);
    ks_cat_buf_char(bc,str);
    if(standard_shader_type == KS_GL_STANDARD_SHADER_SPHERE){
      ks_cat_buf_char(bc,"vec3 emission = vec3(vertexColor.w,vertexColor.w,vertexColor.w);\n");
    }
    ks_cat_buf_char(bc,"fragmentColor = vertexColor.xyz*diffuse +\n");
    ks_cat_buf_char(bc,"materialSpecular * specular +\n");
    if(standard_shader_type == KS_GL_STANDARD_SHADER_SPHERE){
      ks_cat_buf_char(bc,"lightAmbient * vertexColor.xyz+\n");
      ks_cat_buf_char(bc,"emission;\n");
    } else if(standard_shader_type == KS_GL_STANDARD_SHADER_REGULAR ||
	      standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
      ks_cat_buf_char(bc,"lightAmbient * vertexColor.xyz;\n");
    }
    if(standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
      ks_cat_buf_char(bc,"fragmentUV = uv;\n");
    }
  } else if(color_mode == KS_GL_SHADER_COLOR_MODE_DIRECT ||
	    (standard_shader_type == KS_GL_STANDARD_SHADER_POINT &&
	     color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING)){
    ks_cat_buf_char(bc,"fragmentColor = vertexColor.rgb;\n");
    if(standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
      ks_cat_buf_char(bc,"fragmentUV = uv;\n");
    }
  } else if(color_mode == KS_GL_SHADER_COLOR_MODE_PICK){
    sprintf(str,"int pick_id = gl_InstanceID + %d + %d;\n",pick_id_offset,KS_GL_SHADER_PICK_ADJUST);
    ks_cat_buf_char(bc,str);
    ks_cat_buf_char(bc,"int r = (pick_id & 0x000000FF) >>  0;\n");
    ks_cat_buf_char(bc,"int g = (pick_id & 0x0000FF00) >>  8;\n");
    ks_cat_buf_char(bc,"int b = (pick_id & 0x00FF0000) >> 16;\n");
    ks_cat_buf_char(bc,"fragmentColor = vec3(r/255.0f, g/255.0f, b/255.0f);\n");
  }
  if(standard_shader_type == KS_GL_STANDARD_SHADER_POINT){
    if(color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
      ks_cat_buf_char(bc,"gl_PointSize = spherePosition.w*1500.0/gl_Position.w;\n");
    } else if(color_mode == KS_GL_SHADER_COLOR_MODE_DIRECT){
      ks_cat_buf_char(bc,"gl_PointSize = 5;\n");
    }
  }

  ks_cat_buf_char(bc,"}\n");

  shader_src[VERTEX_SHADER] = ks_make_gl_shader((const GLchar*)bc->buf,GL_VERTEX_SHADER);

  if(0){ // output a vertex shader program to check it
    output_shader_program(standard_shader_type,color_mode,pick_id_offset,"vert",bc->buf);
  }

  ks_clear_buf_char(bc);

  ks_cat_buf_char(bc,"#version 330 core\n");
  ks_cat_buf_char(bc,"in vec3 fragmentColor;\n");
  if(standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE &&
     (color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING ||
      color_mode == KS_GL_SHADER_COLOR_MODE_DIRECT)){
    ks_cat_buf_char(bc,"in vec2 fragmentUV;\n");
    ks_cat_buf_char(bc,"uniform sampler2D textureSampler;\n");
  } else if(standard_shader_type == KS_GL_STANDARD_SHADER_POINT &&
	    color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
    ks_cat_buf_char(bc,"uniform sampler2D textureSampler;\n");
  }
  ks_cat_buf_char(bc,"out vec3 color;\n");
  ks_cat_buf_char(bc,"void main()\n");
  ks_cat_buf_char(bc,"{\n");
  if(standard_shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE &&
     (color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING ||
      color_mode == KS_GL_SHADER_COLOR_MODE_DIRECT)){
    if(color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
      ks_cat_buf_char(bc,"color = texture( textureSampler, fragmentUV ).rgb*fragmentColor;\n");
    } else if(color_mode == KS_GL_SHADER_COLOR_MODE_DIRECT){
      ks_cat_buf_char(bc,"color = texture( textureSampler, fragmentUV ).rgb;\n");
    }
  } else if(standard_shader_type == KS_GL_STANDARD_SHADER_POINT){
    if(color_mode == KS_GL_SHADER_COLOR_MODE_LIGHTING){
      ks_cat_buf_char(bc,"vec3 texColor = texture( textureSampler, gl_PointCoord ).rgb*fragmentColor;\n");
      ks_cat_buf_char(bc,"vec2 n;\n");
      ks_cat_buf_char(bc,"n.xy = gl_PointCoord * 2.0 - 1.0;\n");
      ks_cat_buf_char(bc,"if (dot(n.xy, n.xy) > 0.95) discard;\n");
      ks_cat_buf_char(bc,"color = texColor;\n");
    } else if(color_mode == KS_GL_SHADER_COLOR_MODE_DIRECT){
      ks_cat_buf_char(bc,"vec2 n;\n");
      ks_cat_buf_char(bc,"n.xy = gl_PointCoord * 2.0 - 1.0;\n");
      ks_cat_buf_char(bc,"if (dot(n.xy, n.xy) > 1.0) discard;\n");
      ks_cat_buf_char(bc,"color = fragmentColor;\n");
    }
  } else {
    ks_cat_buf_char(bc,"color = fragmentColor;\n");
  }
  ks_cat_buf_char(bc,"}\n");
  
  if(0){ // output a frag shader program to check it
    output_shader_program(standard_shader_type,color_mode,pick_id_offset,"frag",bc->buf);
  }

  shader_src[FRAGMENT_SHADER] = ks_make_gl_shader((const GLchar*)bc->buf,GL_FRAGMENT_SHADER);

  program = ks_make_gl_shader_program(shader_src,SHADER_CNT);

  glDeleteShader(shader_src[VERTEX_SHADER]);
  glDeleteShader(shader_src[FRAGMENT_SHADER]);

  ks_free_buf_char(bc);
  return program;
}
static void init_gl_shader_sphere_attribute(KS_GL_SHADER_INFO* info)
{
  int i;
  for(i = 0; i < KS_GL_SHADER_COLOR_MODE_CNT; i++){
    // set location indices for shader program for KS_GL_STANDARD_SHADER_SPHERE
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][i] = 0;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][i] = 1; // for instanced draw
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][i]    = 2; // for instanced draw
  }
}
static void init_gl_shader_textured_sphere_attribute(KS_GL_SHADER_INFO* info)
{
  int i;
  for(i = 0; i < KS_GL_SHADER_COLOR_MODE_CNT; i++){
    // set location indices for shader program for KS_GL_STANDARD_SHADER_SPHERE
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][i] = 0;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_UV][i]       = 1;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][i]    = 2; // for instanced draw
  }
}
static void init_gl_shader_regular_attribute(KS_GL_SHADER_INFO* info)
{
  int i;
  for(i = 0; i < KS_GL_SHADER_COLOR_MODE_CNT; i++){
    // set location indices of attribute variables for KS_GL_STANDARD_SHADER_REGULAR
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][i]  = 0;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_NORMAL][i]    = 1;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][i]     = 2;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][i]     = 3; // for instanced draw
  }
}
static void init_gl_shader_point(KS_GL_SHADER_INFO* info)
{
  int i;
  for(i = 0; i < KS_GL_SHADER_COLOR_MODE_CNT; i++){
    // set location indices of attribute variables for KS_GL_STANDARD_SHADER_POINT
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][i] = 0;
    info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][i] = 1; // for instanced draw
    info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][i]    = 2; // for instanced draw
  }
}
static void init_gl_shader_uniform(KS_GL_SHADER_INFO* info, int obj_id, int color_mode)
{
  // set location indices of uniform variables
  info->uniform_loc[obj_id][KS_GL_SHADER_UNIFORM_LOCATION_PROJECTION_MATRIX][color_mode]
    = glGetUniformLocation(info->program[obj_id][color_mode],"projectionMatrix");
  info->uniform_loc[obj_id][KS_GL_SHADER_UNIFORM_LOCATION_VIEW_MATRIX][color_mode]
    = glGetUniformLocation(info->program[obj_id][color_mode],"viewMatrix");
  if(info->shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
    info->uniform_loc[obj_id][KS_GL_SHADER_UNIFORM_LOCATION_SAMPLER2D][color_mode]
      = glGetUniformLocation(info->program[obj_id][color_mode],"textureSampler");
  }
}
static void set_gl_standard_shader_program(KS_GL_SHADER_INFO* info, int obj_id,
					   KS_GL_STANDARD_SHADER_PARM *shader_parm,
					   BOOL update_pick_id_offset)
{
  static int pick_id_offset;
  int i;
  for(i = 0; i < KS_GL_SHADER_COLOR_MODE_CNT; i++){
    // set shader program
    if(update_pick_id_offset == KS_TRUE){
      info->obj[obj_id].pick_id_offset = pick_id_offset;
    }
    info->program[obj_id][i] = make_gl_standard_shader_program(info->shader_type,i,
							       info->obj[obj_id].pick_id_offset,
							       shader_parm);
    // bind uniform location
    init_gl_shader_uniform(info,obj_id,i);
  }
  if(update_pick_id_offset == KS_TRUE){
    pick_id_offset += info->obj[obj_id].draw_elem_cnt;
  }
}
GLuint ks_make_gl_shader_program(GLuint *shader, int shader_cnt)
{
  int i;
  GLuint program; // shader program
  GLint link_result;
  program = glCreateProgram();

  for(i = 0; i < shader_cnt; i++){
    glAttachShader(program, shader[i]);
  }
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &link_result);
  if(link_result == GL_FALSE){
    print_glsl_log(program);
    return 0;
  }
  return program;
}
KS_GL_SHADER_INFO *ks_allocate_gl_standard_shader_info(int standard_shader_type, int object_cnt)
{
  int i,j;
  KS_GL_SHADER_INFO *info;
  if((info = (KS_GL_SHADER_INFO*)ks_malloc(sizeof(KS_GL_SHADER_INFO),"info")) == NULL){
    ks_error_memory();
    return NULL;
  }
  info->shader_type = standard_shader_type;
  if((info->program = (GLuint**)ks_malloc(sizeof(GLuint*)*object_cnt,"program")) == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < object_cnt; i++){
    if((info->program[i] = (GLuint*)ks_malloc(sizeof(GLuint)*KS_GL_SHADER_COLOR_MODE_CNT,
					      "program[i]")) == NULL){
      ks_error_memory();
      return NULL;
    }
  }
  if((info->uniform_loc=(GLuint***)ks_malloc(sizeof(GLuint**)*object_cnt,"uniform_loc")) == NULL){
    ks_error_memory();
    return NULL;
  }
  for(i = 0; i < object_cnt; i++){
    if((info->uniform_loc[i]=(GLuint**)ks_malloc(sizeof(GLuint*)*KS_GL_SHADER_UNIFORM_LOCATION_CNT,
						 "uniform_loc[i]")) == NULL){
      ks_error_memory();
      return NULL;
    }
    for(j = 0; j < KS_GL_SHADER_UNIFORM_LOCATION_CNT; j++){
      if((info->uniform_loc[i][j]=(GLuint*)ks_malloc(sizeof(GLuint)*KS_GL_SHADER_COLOR_MODE_CNT,
						     "uniform_loc[i][j]")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
  }
  if((info->obj = (KS_GL_SHADER_OBJECT*)ks_malloc(sizeof(KS_GL_SHADER_OBJECT)*object_cnt,
						  "info->obj")) == NULL){
    ks_error_memory();
    return NULL;
  }
  info->obj_cnt = object_cnt;
  for(i = 0; i < info->obj_cnt; i++){
    info->obj[i].color_mode = KS_GL_SHADER_COLOR_MODE_LIGHTING;
    info->obj[i].pick_id_offset = 0;
    info->obj[i].draw_elem_cnt = 0;
    info->obj[i].index_cnt = 0;
  }
  if(info->shader_type == KS_GL_STANDARD_SHADER_SPHERE){
    for(i = 0; i < info->obj_cnt; i++){
      info->obj[i].draw_mode = GL_TRIANGLE_STRIP;
      if((info->obj[i].vbo = (GLuint*)ks_malloc(KS_GL_SHADER_SPHERE_VBO_CNT*sizeof(GLuint),
						"info->obj[i].vbo")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
    init_gl_shader_sphere_attribute(info);
  } else if(info->shader_type == KS_GL_STANDARD_SHADER_TEXTURED_SPHERE){
    for(i = 0; i < info->obj_cnt; i++){
      info->obj[i].draw_mode = GL_TRIANGLES;
      if((info->obj[i].vbo = (GLuint*)ks_malloc(KS_GL_SHADER_TEXTURED_SPHERE_VBO_CNT*sizeof(GLuint),
						"info->obj[i].vbo")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
    init_gl_shader_textured_sphere_attribute(info);
  } else if(info->shader_type == KS_GL_STANDARD_SHADER_REGULAR){
    for(i = 0; i < info->obj_cnt; i++){
      info->obj[i].draw_mode = GL_TRIANGLES;
      if((info->obj[i].vbo = (GLuint*)ks_malloc(KS_GL_SHADER_REGULAR_VBO_CNT*sizeof(GLuint),
						"info->obj[i].vbo")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
    init_gl_shader_regular_attribute(info);
  } else if(info->shader_type == KS_GL_STANDARD_SHADER_POINT){
    for(i = 0; i < info->obj_cnt; i++){
      info->obj[i].draw_mode = GL_POINTS;
      if((info->obj[i].vbo = (GLuint*)ks_malloc(KS_GL_SHADER_POINT_VBO_CNT*sizeof(GLuint),
						"info->obj[i].vbo")) == NULL){
	ks_error_memory();
	return NULL;
      }
    }
    init_gl_shader_point(info);
  }
  return info;
}
void ks_free_gl_standard_shader_info(KS_GL_SHADER_INFO *info)
{
  int i,j;
  for(i = 0; i < info->obj_cnt; i++){
    ks_free(info->program[i]);
    for(j = 0; j < KS_GL_SHADER_UNIFORM_LOCATION_CNT; j++){
      ks_free(info->uniform_loc[i][j]);
    }
    ks_free(info->uniform_loc[i]);
  }
  ks_free(info->program);
  ks_free(info->uniform_loc);
  for(i = 0; i < info->obj_cnt; i++){
    ks_free(info->obj[i].vbo);
  }
  ks_free(info->obj);
  ks_free(info);
}
void ks_set_gl_shader_color_mode(KS_GL_SHADER_INFO *info, int color_mode)
{
  int i;
  for(i = 0; i < info->obj_cnt; i++){
    info->obj[i].color_mode = color_mode;
  }
}
void ks_set_gl_shader_color_mode_all(KS_GL_SHADER_INFO **info_array, int cnt, int color_mode)
{
  int i;
  for(i = 0; i < cnt; i++){
    ks_set_gl_shader_color_mode(info_array[i],color_mode);
  }
}
unsigned int ks_get_gl_shader_pick_id(int window_height, int x, int y)
{
  unsigned char data[4];
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glReadPixels(x,  window_height - y,1,1, GL_RGBA, GL_UNSIGNED_BYTE, data);
  return data[0] + data[1] * 256 + data[2] * 256*256 - KS_GL_SHADER_PICK_ADJUST;
}
unsigned int ks_get_gl_shader_pick_id_offset(KS_GL_SHADER_INFO *info, int obj_id)
{
  return info->obj[obj_id].pick_id_offset;
}
BOOL ks_check_gl_shader_pick_id(KS_GL_SHADER_INFO *info, int obj_id, unsigned int *pick_id_p)
{
  *pick_id_p -= info->obj[obj_id].pick_id_offset;
  if(*pick_id_p >= 0 && *pick_id_p <  info->obj[obj_id].draw_elem_cnt){
    return KS_TRUE;
  }
  *pick_id_p += info->obj[obj_id].pick_id_offset;
  return KS_FALSE;
}   
KS_GL_SHADER_SPHERE_DRAW_INFO *ks_allocate_gl_shader_sphere_draw_info(int elem_cnt)
{
  KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info;
  if((draw_info =
      (KS_GL_SHADER_SPHERE_DRAW_INFO*)ks_malloc(sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO),
						"draw_info")) == NULL){
    ks_error_memory();
    return NULL;
  }
  draw_info->elem_cnt = elem_cnt;
  draw_info->elem_capacity = elem_cnt;
  if(elem_cnt > 0){
    if((draw_info->elem =
	(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM*)ks_malloc(elem_cnt*
						       sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
						       "draw_info->elem")) == NULL){
      ks_error_memory();
      return NULL;
    }
  } else {
    draw_info->elem = NULL;
  }
  return draw_info;
}
BOOL ks_resize_gl_shader_sphere_draw_info(KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info, int elem_cnt)
{
  if(elem_cnt < 0){
    ks_error("ks_resize_gl_shader_sphere_draw_info, error: elem_cnt is a negative value\n");
    return KS_FALSE;
  }
  if(draw_info->elem == NULL){
    if((draw_info->elem =
	(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM*)ks_malloc(elem_cnt*
						       sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
						       "draw_info->elem")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    draw_info->elem_capacity = elem_cnt;
  } else if(draw_info->elem_capacity < elem_cnt){
    if((draw_info->elem =
	(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM*)ks_realloc(draw_info->elem,elem_cnt*
							sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
							"draw_info->elem")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    draw_info->elem_capacity = elem_cnt;
  }
  draw_info->elem_cnt = elem_cnt;
  return KS_TRUE;
}
void ks_free_gl_shader_sphere_draw_info(KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info)
{
  if(draw_info->elem != NULL){
    ks_free(draw_info->elem);
  }
  ks_free(draw_info);
}
static KS_GL_SHADER_SPHERE_PARM* allocate_shader_sphere_parm(int vertex_cnt, int index_cnt)
{
  KS_GL_SHADER_SPHERE_PARM *parm;
  if((parm = (KS_GL_SHADER_SPHERE_PARM*)ks_malloc(sizeof(KS_GL_SHADER_SPHERE_PARM),
						  "parm of KS_GL_SHADER_SPHERE_PARM")) == NULL){
    ks_error_memory();
    return NULL;
  }
  parm->vertex_cnt = vertex_cnt;  // set total number of vertices
  if((parm->vertex=(KS_GL_FLOAT_ELEM3*)ks_malloc(sizeof(KS_GL_FLOAT_ELEM3)*parm->vertex_cnt,
						 "parm->vertex of KS_GL_FLOAT_ELEM3")
      ) == NULL){
    ks_error_memory();
    return NULL;
  }

  parm->index_cnt = index_cnt; // set total number of indices
  if((parm->index=(GLushort*)ks_malloc(sizeof(GLushort)*parm->index_cnt,
					 "parm->index")) == NULL){
    ks_error_memory();
    return NULL;
  }
  return parm;
}
static void set_sphere_parm_vertex(int type, int index,
				     KS_GL_SHADER_SPHERE_PARM *s_parm,
				     KS_GL_SHADER_TEXTURED_SPHERE_PARM *t_parm,
				     float x, float y, float z, float u, float v)
{
  if(type == GENERATE_SPHERE_PARM_SPHERE){
    s_parm->vertex[index].elem[0] = x;
    s_parm->vertex[index].elem[1] = y;
    s_parm->vertex[index].elem[2] = z;
  } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
    t_parm->elem[index].x = x;
    t_parm->elem[index].y = y;
    t_parm->elem[index].z = z;
    t_parm->elem[index].u = u;
    t_parm->elem[index].v = v;
  } else {
    assert(KS_FALSE && "unknown type");
  }
}
static void set_sphere_parm_index(int type, int index,
				    KS_GL_SHADER_SPHERE_PARM *s_parm,
				    KS_GL_SHADER_TEXTURED_SPHERE_PARM *t_parm,
				    int value)
{
  if(type == GENERATE_SPHERE_PARM_SPHERE){
    s_parm->index[index] = value;
  } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
    t_parm->index[index] = value;
  } else {
    assert(KS_FALSE && "unknown type");
  }
}
static KS_GL_SHADER_TEXTURED_SPHERE_PARM* allocate_shader_textured_sphere_parm(int vertex_cnt,
									       int index_cnt)
{
  KS_GL_SHADER_TEXTURED_SPHERE_PARM *parm;
  if((parm = (KS_GL_SHADER_TEXTURED_SPHERE_PARM*)
      ks_malloc(sizeof(KS_GL_SHADER_TEXTURED_SPHERE_PARM),
		"parm of KS_GL_SHADER_TEXTURED_SPHERE_PARM")) == NULL){
    ks_error_memory();
    return NULL;
  }

  parm->elem_size = sizeof(KS_GL_SHADER_TEXTURED_SPHERE_PARM_ELEM)*vertex_cnt;
  if((parm->elem = (KS_GL_SHADER_TEXTURED_SPHERE_PARM_ELEM*)
      ks_malloc(parm->elem_size,"parm->elem of KS_GL_SHADER_TEXTURED_SPHERE_PARM_ELEM")) == NULL){
    ks_error_memory();
    return NULL;
  }

  parm->index_cnt = index_cnt; // set total number of indices
  if((parm->index=(GLushort*)ks_malloc(sizeof(GLushort)*parm->index_cnt,
					 "parm->index")) == NULL){
    ks_error_memory();
    return NULL;
  }
  return parm;
}
static void *generate_sphere_parm(int detail, int type)
{
  int i,j,index;
  int i_start,i_end;
  double x,y;
  double sz,cz;
  double theta,phi;
  int div_cnt1, div_cnt2;
  int v_cnt1,v_cnt2;
  int vertex_cnt, index_cnt;

  KS_GL_SHADER_SPHERE_PARM *s_parm;
  KS_GL_SHADER_TEXTURED_SPHERE_PARM *t_parm;

  div_cnt1 = detail;   // division for latitude  (north to south)
  div_cnt2 = detail*2; // division for longitude (west to east)
  theta = M_PI/(div_cnt1);
  phi = 2.0*M_PI/(div_cnt2);
  if(type == GENERATE_SPHERE_PARM_SPHERE){
    v_cnt1 = div_cnt1;
    v_cnt2 = div_cnt2;
    vertex_cnt = (v_cnt1-1)*v_cnt2 + 2;  // use minimum vertices number to use GL_TRIANGLE_STRIP
    index_cnt = ((detail-1)*2+2+(detail-1)*2)*detail+1;
  } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
    v_cnt1 = div_cnt1;
    v_cnt2 = div_cnt2+1;                 // add 1 to use other uv at the start and the of longitude
    vertex_cnt = (v_cnt1+1)*v_cnt2;      // use all vertices to map uv with GL_TRIANGLES
    index_cnt = div_cnt1*div_cnt2*6;
  } else {
    ks_assert(KS_FALSE && "unknown sphere type");
  }
  if(type == GENERATE_SPHERE_PARM_SPHERE){
    s_parm = allocate_shader_sphere_parm(vertex_cnt,index_cnt);
  } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
    t_parm = allocate_shader_textured_sphere_parm(vertex_cnt,index_cnt);
  }

  index = 0;
  for(j = 0; j < v_cnt2; j++){               // longitude loop (west to east)
    x = cos(phi*j); y = sin(phi*j);
    if(type == GENERATE_SPHERE_PARM_SPHERE){
      if(j == 0){
	i_start = 0;              // start point（north pole）
	i_end = v_cnt1;
      } else if(j == v_cnt2-1){
	i_start = 1;
	i_end = v_cnt1+1;         // end point (south pole)
      } else {
	i_start = 1;
	i_end = v_cnt1;
      }
    } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
      i_start = 0;
      i_end = v_cnt1+1;
    }
    for(i = i_start; i < i_end; i++){         // latitude loop (north to south)
      sz = sin(theta*i);
      cz = cos(theta*i);
      set_sphere_parm_vertex(type,index++,s_parm,t_parm, sz*x, sz*y, cz,
			       1.0f-(float)i/(v_cnt1),(float)j/(div_cnt2));
    }
  }
  ks_assert(index == vertex_cnt);

  index = 0;
  if(type == GENERATE_SPHERE_PARM_SPHERE){
    for(j = 0; j < v_cnt2; j += 2){
      set_sphere_parm_index(type,index++,s_parm,NULL,0);  // start point
      for(i = 1; i < v_cnt1; i++){
	// down latitude, longitude+0
	set_sphere_parm_index(type,index++,s_parm,NULL,i+(v_cnt1-1)*j);
	//                longitude+1
	set_sphere_parm_index(type,index++,s_parm,NULL,i+(v_cnt1-1)*(j+1));
      }
      set_sphere_parm_index(type,index++,s_parm,NULL,vertex_cnt-1);     // end point
      if(j < v_cnt2-2){
	for(i = v_cnt1-1; i > 0; i--){
	  // up latitude, longitude+1
	  set_sphere_parm_index(type,index++,s_parm,NULL,i+(v_cnt1-1)*(j+1));
	  //              longitude+2
	  set_sphere_parm_index(type,index++,s_parm,NULL,i+(v_cnt1-1)*(j+2));
	}
      } else {
	for(i = v_cnt1-1; i > 0; i--){                   // the last up latitude
	  // up latitude, longitude+1
	  set_sphere_parm_index(type,index++,s_parm,NULL,i+(v_cnt1-1)*(j+1));
	  //              longitude start point
	  set_sphere_parm_index(type,index++,s_parm,NULL,i);
	}
	set_sphere_parm_index(type,index++,s_parm,NULL,0);   // start point
      }
    }
  } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
    for(j = 0; j < div_cnt2; j++){
      for(i = 0; i < div_cnt1; i++){             // latitude loop (north to south)
	set_sphere_parm_index(type,index++,NULL,t_parm,(div_cnt1+1)*j+0+i);
	set_sphere_parm_index(type,index++,NULL,t_parm,(div_cnt1+1)*j+1+i);
	set_sphere_parm_index(type,index++,NULL,t_parm,(div_cnt1+1)*j+v_cnt1+2+i);
	set_sphere_parm_index(type,index++,NULL,t_parm,(div_cnt1+1)*j+0+i);
	set_sphere_parm_index(type,index++,NULL,t_parm,(div_cnt1+1)*j+v_cnt1+2+i);
	set_sphere_parm_index(type,index++,NULL,t_parm,(div_cnt1+1)*j+v_cnt1+1+i);
      }
    }
  }
  ks_assert(index == index_cnt);

  if(type == GENERATE_SPHERE_PARM_SPHERE){
    return s_parm;
  } else if(type == GENERATE_SPHERE_PARM_TEXTURED_SPHERE){
    return t_parm;
  }
  return NULL;
}
KS_GL_SHADER_SPHERE_PARM *ks_allocate_gl_shader_sphere_parm(int detail)
{
  return (KS_GL_SHADER_SPHERE_PARM*)generate_sphere_parm(detail,GENERATE_SPHERE_PARM_SPHERE);
}
void ks_free_gl_shader_sphere_parm(KS_GL_SHADER_SPHERE_PARM *parm)
{
  if(parm->vertex != NULL){
    ks_free(parm->vertex);
  }
  if(parm->index != NULL){
    ks_free(parm->index);
  }
  ks_free(parm);
}
void ks_init_gl_standard_shader_sphere(KS_GL_SHADER_INFO *info, int obj_id,
				       KS_GL_SHADER_SPHERE_PARM *sphere_parm, int share_obj_id)
{
  int color_mode;
  int use_obj_id;
  // make vertices of sphere
  if(share_obj_id < 0){ // default setting
    info->obj[obj_id].index_cnt = sphere_parm->index_cnt;
  } else {              // shape share setting
    info->obj[obj_id].index_cnt = info->obj[share_obj_id].index_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
  }

  // make VAO for sphere
  glGenVertexArrays(KS_GL_SHADER_COLOR_MODE_CNT, info->obj[obj_id].vao);

  if(share_obj_id < 0){
    // make VBO
    // vertices           KS_GL_SHADER_SPHERE_VBO_VERTEX:0
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_VERTEX]);
    // setting of vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sphere_parm->vertex_cnt*sizeof(GLfloat)*3,
		 (GLfloat*)sphere_parm->vertex,
		 GL_STATIC_DRAW); // vertices of sphere

    glGenBuffers(1, &info->obj[obj_id].vbi); // buffer for indices
    // setting of indices buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[obj_id].vbi);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sphere_parm->index_cnt*sizeof(GLushort),
		 sphere_parm->index,GL_STATIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }
  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[use_obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_VERTEX]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[use_obj_id].vbi);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode]);
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode],
			  3,GL_FLOAT,GL_FALSE,0,(void*)0);
  }

  glBindVertexArray(0);
}
void ks_set_gl_shader_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
				       KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
				       KS_GL_STANDARD_SHADER_PARM* shader_parm,
				       int share_obj_id)
{
  int color_mode;
  int use_obj_id;
  GLsizei stride;

  // keep number of elements for instanced rendering
  if(share_obj_id < 0){ // default setting
    info->obj[obj_id].draw_elem_cnt = draw_info->elem_cnt;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_TRUE);
  } else {              // draw info shear setting
    info->obj[obj_id].draw_elem_cnt = info->obj[share_obj_id].draw_elem_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
    info->obj[obj_id].pick_id_offset = info->obj[share_obj_id].pick_id_offset;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_FALSE);
  }

  if(share_obj_id < 0){
    // make VBO
    // position and color KS_GL_SHADER_SPHERE_VBO_PROPERTY:1
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_PROPERTY]);
    // setting of position and color buffer
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_PROPERTY]);
    glBufferData(GL_ARRAY_BUFFER, draw_info->elem_cnt*sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
		 draw_info->elem, GL_DYNAMIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }
  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[use_obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_PROPERTY]);
    // set location of position and size
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][color_mode]);
    // set location of color and emission
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode]);
    stride = sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM);
    // set attribute of position and size
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][color_mode],
			  4,                                // size : x + y + z + size => 4
			  GL_FLOAT,                         // type
			  GL_FALSE,                         // normalized?
			  stride,                           // stride
			  (void*)0                          // array buffer offset
			  );
    // set attribute of color and emission
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode],
			  4,                                // size : r + g + b + em => 4
			  GL_FLOAT,                         // type
			  GL_FALSE,                         // normalized?
			  stride,                           // stride
			  (void*)(sizeof(GLfloat)*4)        // array buffer offset
			  );
    // set divisor of position and size for instanced rendering
    glVertexAttribDivisor(info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][color_mode], 1);
    // set divisor of color and emission for instanced rendering
    glVertexAttribDivisor(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode], 1);
  }
  glBindVertexArray(0);
}
void ks_set_gl_shader_sphere_draw_info_share(KS_GL_SHADER_INFO *info,
					     int obj_id_start, int obj_id_end,
					     KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
					     KS_GL_STANDARD_SHADER_PARM* shader_parm)
{
  int i;
  if(obj_id_start >= obj_id_end){
    ks_error("ks_set_gl_shader_sphere_draw_info_share, error: "
	     "obj_id_start is larger than obj_id_end\n");
    return;
  }
  // bind draw info to the shader
  ks_set_gl_shader_sphere_draw_info(info,obj_id_start,draw_info,shader_parm,-1);
  // share the draw info to share it
  for(i = obj_id_start+1; i < obj_id_end; i++){
    ks_set_gl_shader_sphere_draw_info(info,i,NULL,shader_parm,obj_id_start);
  }
}
void ks_update_gl_shader_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					  KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
					  int share_obj_id)
{
  // keep number of elements for instanced rendering
  if(share_obj_id < 0){ // default setting
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_PROPERTY]);
    if(info->obj[obj_id].draw_elem_cnt < draw_info->elem_cnt){
      glBufferData(GL_ARRAY_BUFFER, draw_info->elem_cnt*sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
		   draw_info->elem, GL_DYNAMIC_DRAW);
    } else {
      glBufferSubData(GL_ARRAY_BUFFER, 0,
		      draw_info->elem_cnt*sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
		      draw_info->elem);
    }
    info->obj[obj_id].draw_elem_cnt = draw_info->elem_cnt;
  } else {              // draw info shear setting
    info->obj[obj_id].draw_elem_cnt = info->obj[share_obj_id].draw_elem_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
    info->obj[obj_id].pick_id_offset = info->obj[share_obj_id].pick_id_offset;
  }
}
void ks_update_gl_shader_sphere_draw_info_share(KS_GL_SHADER_INFO *info,
						int obj_id_start, int obj_id_end,
						KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info)
{
  int i;
  if(obj_id_start >= obj_id_end){
    ks_error("ks_update_gl_shader_sphere_draw_info_share, error: "
	     "obj_id_start is larger than obj_id_end\n");
    return;
  }

  // bind draw info to the shader
  ks_update_gl_shader_sphere_draw_info(info,obj_id_start,draw_info,-1);
  // share the draw info to share it
  for(i = obj_id_start+1; i < obj_id_end; i++){
    ks_update_gl_shader_sphere_draw_info(info,i,NULL,obj_id_start);
  }
}
void ks_draw_gl_standard_shader(KS_GL_SHADER_INFO *info, int obj_id,
				GLfloat* projection_matrix, GLfloat* view_matrix)
{
  int color_mode = info->obj[obj_id].color_mode;

  glBindVertexArray(info->obj[obj_id].vao[color_mode]);

  // use shader program
  glUseProgram(info->program[obj_id][color_mode]);

  // send uniform variables
  glUniformMatrix4fv(info->uniform_loc[obj_id][KS_GL_SHADER_UNIFORM_LOCATION_PROJECTION_MATRIX][color_mode],
		     1, GL_FALSE, projection_matrix);
  glUniformMatrix4fv(info->uniform_loc[obj_id][KS_GL_SHADER_UNIFORM_LOCATION_VIEW_MATRIX][color_mode],
		     1, GL_FALSE, view_matrix);

  // instanced rendering
  glDrawElementsInstanced(info->obj[obj_id].draw_mode, info->obj[obj_id].index_cnt,
			  GL_UNSIGNED_SHORT,
			  (GLubyte*)NULL,info->obj[obj_id].draw_elem_cnt);

  glBindVertexArray(0);
  glUseProgram(0);
}
void ks_init_gl_standard_shader_regular(KS_GL_SHADER_INFO *info, int obj_id,
					KS_GL_SHADER_REGULAR_PARM *parm, int share_obj_id)
{
  int color_mode;
  int use_obj_id;
  // keep number of indices for instanced rendering for regular shader
  if(share_obj_id < 0){   // default setting
    info->obj[obj_id].index_cnt = parm->index_cnt;
  } else {                // shape share setting
    info->obj[obj_id].index_cnt = info->obj[share_obj_id].index_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
  }

  // make VAO
  glGenVertexArrays(KS_GL_SHADER_COLOR_MODE_CNT, info->obj[obj_id].vao);

  if(share_obj_id < 0){
    // make VBO
    // vertices      KS_GL_SHADER_REGULAR_VBO_VERTEX:0
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_VERTEX]);
    // setting of vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, parm->elem_size,parm->elem, GL_STATIC_DRAW);

    glGenBuffers(1, &info->obj[obj_id].vbi); // buffer for indices
    // setting of indices buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[obj_id].vbi);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, parm->index_size,parm->index,GL_STATIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }

  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[use_obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_VERTEX]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[use_obj_id].vbi);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode]);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode]);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_NORMAL][color_mode]);
    GLsizei stride = sizeof(GLfloat)*(3+3+3);  // vertex, normal, color
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode],
			  3,GL_FLOAT,GL_FALSE,stride,(void*)0);
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode],
			  3,GL_FLOAT,GL_FALSE,stride,(void*)(sizeof(GLfloat)*6));
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_NORMAL][color_mode],
			  3,GL_FLOAT,GL_FALSE,stride,(void*)(sizeof(GLfloat)*3));
  }

  glBindVertexArray(0);
}
void ks_set_gl_shader_regular_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
					KS_GL_STANDARD_SHADER_PARM* shader_parm,
					int share_obj_id)
{
  int i;
  int color_mode;
  int use_obj_id;

  // keep number of elements for instanced rendering
  if(share_obj_id < 0){ // default setting
    info->obj[obj_id].draw_elem_cnt = draw_info->elem_cnt;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_TRUE);
  } else {              // draw info shear setting
    info->obj[obj_id].draw_elem_cnt = info->obj[share_obj_id].draw_elem_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
    info->obj[obj_id].pick_id_offset = info->obj[share_obj_id].pick_id_offset;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_FALSE);
  }

  if(share_obj_id < 0){
    // make VBO
    // model  matrix KS_GL_SHADER_REGULAR_VBO_MODEL_MATRIX:1
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_MODEL_VIEW_MATRIX]);
    // setting of model view buffer
    glBindBuffer(GL_ARRAY_BUFFER,info->obj[obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_MODEL_VIEW_MATRIX]);
    glBufferData(GL_ARRAY_BUFFER, draw_info->model_view_size,draw_info->model_view,GL_DYNAMIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }
  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER,
		 info->obj[use_obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_MODEL_VIEW_MATRIX]);
    for(i = 0; i < 4; i++){   // get from 03-instancing2.cpp in OpenGL Programing Guide
      // set attribute of position and size model matrix
      glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][color_mode] + i,
			    4, GL_FLOAT, GL_FALSE,            // vec4
			    sizeof(GLfloat)*16,               // matrix size
			    (void *)(sizeof(GLfloat)*4 * i)); // offset
      glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][color_mode] + i);
      // set divisor of model matrix for instanced rendering
      glVertexAttribDivisor(info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][color_mode] + i, 1);
    }
  }
  glBindVertexArray(0);
}
void ks_update_gl_shader_regular_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
					   KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
					   int share_obj_id)
{
  if(share_obj_id < 0){ // default setting
    glBindBuffer(GL_ARRAY_BUFFER,
		 info->obj[obj_id].vbo[KS_GL_SHADER_REGULAR_VBO_MODEL_VIEW_MATRIX]);
    if(info->obj[obj_id].draw_elem_cnt < draw_info->elem_cnt){
      glBufferData(GL_ARRAY_BUFFER, draw_info->model_view_size,draw_info->model_view,
		   GL_DYNAMIC_DRAW);
    } else {
      glBufferSubData(GL_ARRAY_BUFFER, 0, draw_info->model_view_size, draw_info->model_view);
    }
    info->obj[obj_id].draw_elem_cnt = draw_info->elem_cnt;
  } else {             // draw info shear setting
    info->obj[obj_id].draw_elem_cnt = info->obj[share_obj_id].draw_elem_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
    info->obj[obj_id].pick_id_offset = info->obj[share_obj_id].pick_id_offset;
  }
}
void ks_update_gl_shader_regular_draw_info_share(KS_GL_SHADER_INFO *info,
						 int obj_id_start, int obj_id_end,
						 KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info)
{
  int i;
  if(obj_id_start >= obj_id_end){
    ks_error("ks_update_gl_shader_regular_draw_info_share, error: "
	     "obj_id_start is larger than obj_id_end\n");
    return;
  }

  // bind draw info to the shader
  ks_update_gl_shader_regular_draw_info(info,obj_id_start,draw_info,-1);
  // share the draw info to share it
  for(i = obj_id_start+1; i < obj_id_end; i++){
    ks_update_gl_shader_regular_draw_info(info,i,NULL,obj_id_start);
  }
}
void ks_set_gl_shader_draw_mode(KS_GL_SHADER_INFO *info,int obj_id, GLenum draw_mode)
{
  info->obj[obj_id].draw_mode = draw_mode;
}
KS_GL_SHADER_REGULAR_PARM *ks_allocate_gl_shader_regular_parm(int vertex_cnt,
							      int index_cnt)
{
  KS_GL_SHADER_REGULAR_PARM *parm;
  if((parm = (KS_GL_SHADER_REGULAR_PARM*)ks_malloc(sizeof(KS_GL_SHADER_REGULAR_PARM),
						   "KS_GL_SHADER_REGULAR_PARM")) == NULL){
    ks_error_memory();
    return NULL;
  }
  parm->elem_size = sizeof(KS_GL_SHADER_REGULAR_PARM_ELEM)*vertex_cnt;
  if((parm->elem =
      (KS_GL_SHADER_REGULAR_PARM_ELEM*)ks_malloc(parm->elem_size,
						 "KS_GL_SHADER_REGULAR_PARM_ELEM")) == NULL){
    ks_error_memory();
    return NULL;
  }

  parm->index_cnt = index_cnt;
  parm->index_size = sizeof(GLushort)*index_cnt;
  if((parm->index =
      (GLushort*)ks_malloc(parm->index_size,"parm->index")) == NULL){
    ks_error_memory();
    return NULL;
  }
  return parm;
}
void ks_free_gl_shader_regular_parm(KS_GL_SHADER_REGULAR_PARM *parm)
{
  ks_free(parm->elem);
  ks_free(parm->index);
  ks_free(parm);
}
KS_GL_SHADER_REGULAR_DRAW_INFO* ks_allocate_gl_shader_regular_draw_info(int elem_cnt)
{
  int i,j;
  KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info;
  if((draw_info = (KS_GL_SHADER_REGULAR_DRAW_INFO*)ks_malloc(sizeof(KS_GL_SHADER_REGULAR_DRAW_INFO),
						      "draw_info")) == NULL){
    ks_error_memory();
    return NULL;
  }
  draw_info->elem_cnt = elem_cnt;
  draw_info->elem_capacity = elem_cnt;
  if((draw_info->elem =
      (KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM*)ks_malloc(elem_cnt*
						      sizeof(KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM),
						      "draw_info->elem"))
     == NULL){
    ks_error_memory();
    ks_exit(EXIT_FAILURE);
  }
  for(i = 0; i < elem_cnt; i++){
    for(j = 0; j < 3; j++){
      draw_info->elem[i].trans[j] = 0.0f;
      draw_info->elem[i].angle_euler_zyz[j] = 0.0f;
      draw_info->elem[i].scale[j] = 1.0f;
    }
  }
  draw_info->model_view_size = draw_info->elem_cnt*sizeof(GLfloat)*16;
  if((draw_info->model_view = (GLfloat*)ks_malloc(draw_info->model_view_size,
							"draw_info->model_view")) == NULL){
    ks_error_memory();
    return NULL;
  }
  return draw_info;
}
BOOL ks_resize_gl_shader_regular_draw_info(KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info, int elem_cnt)
{
  int i,j;
  if(elem_cnt < 0){
    ks_error("ks_resize_gl_shader_regular_draw_info, error: elem_cnt is a negative value\n");
    return KS_FALSE;
  }
  if(draw_info->elem == NULL){
    if((draw_info->elem =
	(KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM*)ks_malloc(elem_cnt*
							sizeof(KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM),
							"draw_info->elem"))
       == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    if((draw_info->model_view = (GLfloat*)ks_malloc(elem_cnt*sizeof(GLfloat)*16,
						    "draw_info->model_view")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    draw_info->elem_capacity = elem_cnt;
  } else if(draw_info->elem_capacity < elem_cnt){
    if((draw_info->elem =
	(KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM*)ks_realloc(draw_info->elem,
							 elem_cnt*
							 sizeof(KS_GL_SHADER_REGULAR_DRAW_INFO_ELEM),
							 "draw_info->elem"))
       == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    if((draw_info->model_view = (GLfloat*)ks_realloc(draw_info->model_view,
						     elem_cnt*sizeof(GLfloat)*16,
						     "draw_info->model_view")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    draw_info->elem_capacity = elem_cnt;
  }
  draw_info->elem_cnt = elem_cnt;
  draw_info->model_view_size = elem_cnt*sizeof(GLfloat)*16;
  for(i = 0; i < elem_cnt; i++){
    for(j = 0; j < 3; j++){
      draw_info->elem[i].trans[j] = 0.0f;
      draw_info->elem[i].angle_euler_zyz[j] = 0.0f;
      draw_info->elem[i].scale[j] = 1.0f;
    }
  }
  return KS_TRUE;
}
void ks_free_gl_shader_regular_draw_info(KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info)
{
  ks_free(draw_info->elem);
  ks_free(draw_info->model_view);
  ks_free(draw_info);
}
KS_GL_SHADER_TEXTURED_SPHERE_PARM *ks_allocate_gl_shader_textured_sphere_parm(int detail)
{
  return (KS_GL_SHADER_TEXTURED_SPHERE_PARM*)generate_sphere_parm(detail,GENERATE_SPHERE_PARM_TEXTURED_SPHERE);
}
void ks_free_gl_shader_textured_sphere_parm(KS_GL_SHADER_TEXTURED_SPHERE_PARM *parm)
{
  if(parm->elem != NULL){
    ks_free(parm->elem);
  }
  if(parm->index != NULL){
    ks_free(parm->index);
  }
  ks_free(parm);
}
void ks_init_gl_standard_shader_textured_sphere(KS_GL_SHADER_INFO *info, int obj_id,
						KS_GL_SHADER_TEXTURED_SPHERE_PARM* sphere_parm,
						int share_obj_id)
{
  int color_mode;
  int use_obj_id;

  // make vertices of sphere
  if(share_obj_id < 0){ // default setting
    info->obj[obj_id].index_cnt = sphere_parm->index_cnt;
  } else {              // shape share setting
    info->obj[obj_id].index_cnt = info->obj[share_obj_id].index_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
  }

  // make VAO for sphere
  glGenVertexArrays(KS_GL_SHADER_COLOR_MODE_CNT, info->obj[obj_id].vao);

  if(share_obj_id < 0){
    // make VBO
    // vertices           KS_GL_SHADER_TEXTURED_SPHERE_VBO_VERTEX:0
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_VERTEX]);
    // setting of vertices buffer
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_VERTEX]);
    glBufferData(GL_ARRAY_BUFFER, sphere_parm->elem_size, (GLfloat*)sphere_parm->elem,
		 GL_STATIC_DRAW); // vertices of sphere

    glGenBuffers(1, &info->obj[obj_id].vbi); // buffer for indices
    // setting of indices buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[obj_id].vbi);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 sphere_parm->index_cnt*sizeof(GLushort),
		 sphere_parm->index,GL_STATIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }
  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER,
		 info->obj[use_obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_VERTEX]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[use_obj_id].vbi);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode]);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_UV][color_mode]);
    GLsizei stride = sizeof(GLfloat)*(3+2);  // vertex, uv
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode],
			  3,GL_FLOAT,GL_FALSE,stride,(void*)0);
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_UV][color_mode],
			  2,GL_FLOAT,GL_FALSE,stride,(void*)(sizeof(GLfloat)*3));

  }

  glBindVertexArray(0);
}
void ks_set_gl_shader_textured_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
						KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info,
						KS_GL_STANDARD_SHADER_PARM* shader_parm,
						int share_obj_id)
{
  int i;
  int color_mode;
  int use_obj_id;

  // keep number of elements for instanced rendering
  if(share_obj_id < 0){ // default setting
    info->obj[obj_id].draw_elem_cnt = draw_info->elem_cnt;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_TRUE);
  } else {              // draw info shear setting
    info->obj[obj_id].draw_elem_cnt = info->obj[share_obj_id].draw_elem_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
    info->obj[obj_id].pick_id_offset = info->obj[share_obj_id].pick_id_offset;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_FALSE);
  }

  if(share_obj_id < 0){
    // make VBO
    // model  matrix KS_GL_SHADER_TEXTURED_SPHERE_VBO_PROPERTY:1
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_MODEL_VIEW_MATRIX]);
    // setting of model view buffer
    glBindBuffer(GL_ARRAY_BUFFER,
		 info->obj[obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_MODEL_VIEW_MATRIX]);
    glBufferData(GL_ARRAY_BUFFER, draw_info->model_view_size,draw_info->model_view,GL_DYNAMIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }
  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER,
		 info->obj[use_obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_MODEL_VIEW_MATRIX]);
    for(i = 0; i < 4; i++){   // get from 03-instancing2.cpp in OpenGL Programing Guide
      // set attribute of position and size model matrix
      glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][color_mode] + i,
			    4, GL_FLOAT, GL_FALSE,            // vec4
			    sizeof(GLfloat)*16,               // matrix size
			    (void *)(sizeof(GLfloat)*4 * i)); // offset
      glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][color_mode] + i);
      // set divisor of model matrix for instanced rendering
      glVertexAttribDivisor(info->in_loc[KS_GL_SHADER_IN_LOCATION_MODEL_MATRIX][color_mode] + i, 1);
    }
  }
  glBindVertexArray(0);
}
void ks_update_gl_shader_textured_sphere_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
						   KS_GL_SHADER_REGULAR_DRAW_INFO *draw_info)
{
  glBindBuffer(GL_ARRAY_BUFFER,
	       info->obj[obj_id].vbo[KS_GL_SHADER_TEXTURED_SPHERE_VBO_MODEL_VIEW_MATRIX]);
  glBufferSubData(GL_ARRAY_BUFFER, 0, draw_info->model_view_size, draw_info->model_view);
}
void ks_use_gl_shader_texture(GLuint uniform_loc, KS_GL_SHADER_TEXTURE_INFO *tex_info)
{
  glActiveTexture(GL_TEXTURE0+tex_info->unit);        // set texture unit
  glBindTexture(GL_TEXTURE_2D, tex_info->texture_id); // bind texture
  glUniform1i(uniform_loc,tex_info->unit);            // set texture unit for shader
}
void ks_draw_gl_standard_shader_with_texture(KS_GL_SHADER_INFO *info, int obj_id,
					     GLfloat* projection_matrix, GLfloat *view_matrix,
					     KS_GL_SHADER_TEXTURE_INFO *tex_info)
{
  ks_use_gl_shader_texture(info->uniform_loc[obj_id][KS_GL_SHADER_UNIFORM_LOCATION_SAMPLER2D]
			   [info->obj[obj_id].color_mode],tex_info);
  ks_draw_gl_standard_shader(info,obj_id,projection_matrix,view_matrix);
}
static KS_GL_SHADER_TEXTURE_INFO* allocate_gl_shader_texture_info(GLint unit, GLubyte *image,
								  int w, int h)
{
  GLuint sampler;
  int max_texture_units;

  KS_GL_SHADER_TEXTURE_INFO *tex_info;

  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
  if(unit > max_texture_units){
    ks_error("number of unit is larger than GL_MAX_TEXTURE_UNITS\n");
    return NULL;
  }

  if((tex_info = (KS_GL_SHADER_TEXTURE_INFO*)ks_malloc(sizeof(KS_GL_SHADER_TEXTURE_INFO),
						       "tex_info")) == NULL){
    ks_error_memory();
    return NULL;
  }

  tex_info->unit = unit;
  glActiveTexture(GL_TEXTURE0+tex_info->unit);

  glGenTextures(1, &tex_info->texture_id);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, tex_info->texture_id);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, h, w, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

  ks_free(image);

  glGenSamplers(1, &sampler);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindSampler(tex_info->unit, sampler);

  glBindTexture(GL_TEXTURE_2D, 0);

  return tex_info;
}
KS_GL_SHADER_TEXTURE_INFO* ks_allocate_gl_shader_file_texture(GLint unit, char *file_name,
							      KS_CHAR_LIST *search_path)
{
  GLubyte *image;
  int w,h;

  if((image = ks_read_ppm_file(file_name,&w,&h,search_path)) == NULL){
    return NULL;
  }
  return allocate_gl_shader_texture_info(unit,image,w,h);
}
KS_GL_SHADER_TEXTURE_INFO* ks_allocate_gl_shader_sphere_texture(GLint unit, int size,
								GLfloat light_pos[3])
{
  int i;
  int w,h;
  GLubyte *image;
  w = 1;
  h = 1;
  for(i = 0; i < size; i++){
    w *= 2;
    h *= 2;
  }
  light_pos[1] *= -1;
  if((image = make_sphere_image(w,light_pos)) == NULL){
    return NULL;
  }
  light_pos[1] *= -1;
  return allocate_gl_shader_texture_info(unit,image,w,h);
}
void ks_free_gl_shader_texture_info(KS_GL_SHADER_TEXTURE_INFO *tex_info)
{
  ks_free(tex_info);
}
void ks_init_gl_standard_shader_point(KS_GL_SHADER_INFO *info, int obj_id)
{
  int color_mode;
  // just set a vertex in the origin to draw a point
  int index_cnt = 1;
  KS_GL_FLOAT_ELEM3 vertex = {{0.0f,0.0f,0.0f}};
  GLushort index = 0;

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_POINT_SPRITE);

  // make vertices of sphere
  info->obj[obj_id].index_cnt = index_cnt;

  // make VAO for sphere
  glGenVertexArrays(KS_GL_SHADER_COLOR_MODE_CNT, info->obj[obj_id].vao);

  // make VBO
  // vertices           KS_GL_SHADER_POINT_VBO_VERTEX:0
  glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_POINT_VBO_VERTEX]);
  // setting of vertices buffer
  glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_POINT_VBO_VERTEX]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3,(GLfloat*)&vertex,GL_STATIC_DRAW); // set vertex

  glGenBuffers(1, &info->obj[obj_id].vbi); // buffer for indices
  // setting of indices buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[obj_id].vbi);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort), &index,GL_STATIC_DRAW); // set index

  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_POINT_VBO_VERTEX]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info->obj[obj_id].vbi);
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode]);
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_POSITION][color_mode],
			  3,GL_FLOAT,GL_FALSE,0,(void*)0);
  }

  glBindVertexArray(0);

}
void ks_set_gl_shader_point_draw_info(KS_GL_SHADER_INFO *info, int obj_id,
				      KS_GL_SHADER_SPHERE_DRAW_INFO *draw_info,
				      KS_GL_STANDARD_SHADER_PARM* shader_parm,
				      int share_obj_id)
{
  int color_mode;
  int use_obj_id;
  GLsizei stride;

  // keep number of elements for instanced rendering
  if(share_obj_id < 0){ // default setting
    info->obj[obj_id].draw_elem_cnt = draw_info->elem_cnt;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_TRUE);
  } else {              // draw info shear setting
    info->obj[obj_id].draw_elem_cnt = info->obj[share_obj_id].draw_elem_cnt;
    info->obj[obj_id].draw_mode = info->obj[share_obj_id].draw_mode;
    info->obj[obj_id].pick_id_offset = info->obj[share_obj_id].pick_id_offset;
    set_gl_standard_shader_program(info,obj_id,shader_parm,KS_FALSE);
  }

  if(share_obj_id < 0){
    // make VBO
    // position and color KS_GL_SHADER_SPHERE_VBO_PROPERTY:1
    glGenBuffers(1, &info->obj[obj_id].vbo[KS_GL_SHADER_POINT_VBO_PROPERTY]);
    // setting of position and color buffer
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[obj_id].vbo[KS_GL_SHADER_POINT_VBO_PROPERTY]);
    glBufferData(GL_ARRAY_BUFFER, draw_info->elem_cnt*sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM),
		 draw_info->elem, GL_DYNAMIC_DRAW);
    use_obj_id = obj_id;
  } else {
    use_obj_id = share_obj_id;
  }
  for(color_mode = 0; color_mode < KS_GL_SHADER_COLOR_MODE_CNT; color_mode++){
    glBindVertexArray(info->obj[obj_id].vao[color_mode]);
    glBindBuffer(GL_ARRAY_BUFFER, info->obj[use_obj_id].vbo[KS_GL_SHADER_SPHERE_VBO_PROPERTY]);
    // set location of position and size
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][color_mode]);
    // set location of color and emission
    glEnableVertexAttribArray(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode]);
    stride = sizeof(KS_GL_SHADER_SPHERE_DRAW_INFO_ELEM);
    // set attribute of position and size
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][color_mode],
			  4,                                // size : x + y + z + size => 4
			  GL_FLOAT,                         // type
			  GL_FALSE,                         // normalized?
			  stride,                           // stride
			  (void*)0                          // array buffer offset
			  );
    // set attribute of color and emission
    glVertexAttribPointer(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode],
			  4,                                // size : r + g + b + em => 4
			  GL_FLOAT,                         // type
			  GL_FALSE,                         // normalized?
			  stride,                           // stride
			  (void*)(sizeof(GLfloat)*4)        // array buffer offset
			  );
    // set divisor of position and size for instanced rendering
    glVertexAttribDivisor(info->in_loc[KS_GL_SHADER_IN_LOCATION_SPHERE_POSITION][color_mode], 1);
    // set divisor of color and emission for instanced rendering
    glVertexAttribDivisor(info->in_loc[KS_GL_SHADER_IN_LOCATION_VERTEX_COLOR][color_mode], 1);
  }
  glBindVertexArray(0);
}

#endif

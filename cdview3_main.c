/*
  cdview3 main file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <stdarg.h>

#include <GL/glut.h>

#include "cv304.h"
#include <ks_std.h>
#include <ks_gl.h>

CV_BASE *cb;
KS_GL_BASE *gb;

KS_OPTIONS *ops = NULL;
char **input_file_name = NULL;
int input_file_num = 0;

void init_gl(void)
{
  cv_init_gl(gb,cb);
}
void finalize(void)
{
  cv_free_base(cb);
  ks_free_gl_base(gb);
  ks_free_options(ops,input_file_name,input_file_num);
  ks_exit(EXIT_SUCCESS);
}
#if 0
void display_3d(KS_GL_BASE *gb, void *vp)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  cv_draw(gb,cb);

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
}
void display_2d(KS_GL_BASE *gb, void *vp, int width, int height)
{
  int i,j,k;
  int ix,iy;
  int i0,i1;
  double ret_len;
  double d0,d1,d2,d3;
  char c0[256];
  GLfloat color[3];

  if(ks_gl_base_verbose_level(gb) >= 2){
    cv_draw_color_sample(gb,cb,width,height);
    cv_draw_info(gb,cb,width,height);
    /*    cv_draw_2d(gb,cb,width,height);*/
  }
  cv_draw_text(gb,cb,width,height);
}
void display(void)
{
  ks_gl_base_display(gb,(void*)cb,display_3d,display_2d);
  glutSwapBuffers();
}
#else
void display(void)
{
  cv_process_display(gb,cb);
  glutSwapBuffers();
}
#endif
void reshape(int w, int h)
{
  ks_gl_base_reshape(gb,w,h);
}
void mouse(int button, int state, int x, int y)
{
  cv_process_mouse(gb,cb,button,state,x,y);
  glutPostRedisplay();
}
void motion(int x, int y)
{
  ks_gl_base_motion(gb,x,y);
  cv_process_motion(gb,cb,x,y);
}
void passive(int x, int y)
{
  ks_gl_base_passive(gb,x,y);
}
void keyboard(unsigned char key, int x, int y)
{
  /*
  printf("shift %d ctrl %d alt %d\n"
	 ,ks_get_glut_shift(gb)
	 ,ks_get_glut_ctrl(gb)
	 ,ks_get_glut_alt(gb));
  */
  if(ks_is_gl_input_key_mode(gb) == KS_FALSE){
    if(/*key == '\033'|| */key == 'q' || key == 'Q') finalize();
  }

  cv_process_keyboard(gb,cb,key,x,y);

  /*
  if(key == 'l'){
    double pos[3] = {5,5,5};
    //    ks_set_gl_lookat_position(gb,pos);
    ks_set_gl_center_of_rotation(gb,pos);
  }
  */
  /*
  if(key == 'w'){
    glutCreateWindow("second window");
    glutDisplayFunc(display); 
  }
  */

  glutPostRedisplay();
}
void special_key(int key, int x, int y)
{
  cv_process_special_keyboard(gb,cb,key,x,y);
  glutPostRedisplay();
}
/*
void init(BOOL no_mem, BOOL no_water, int initial_detail, char *header_file_name)
{
  if((cb = cv_allocate_base(input_file_name,input_file_num,no_mem,no_water,initial_detail,
			    header_file_name)) 
     == NULL){
    ks_exit(EXIT_FAILURE);
  }
}
*/
void idle(void)
{
  if(!cv_process_idle(gb,cb)){
    finalize();
  }
}
BOOL set_levels_all(double value, double **levels)
{
  int i;
  for(i = 0; i < FV_MAX_COLUMN_NUM; i++){
    if((levels[i] = ks_malloc_double_p(1,"levels[i]")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    *levels[i] = value;
  }

  return KS_TRUE;
}
BOOL set_levels(char *level_c, double **levels)
{
  int i,j;
  char *cp;
  char str[64];

  cp = level_c;
  i = 0;
  for(;;){
    for(j = 0; *cp && *cp != ',' && j < sizeof(str); cp++,j++){
      //      printf("%d %d %c\n",i,j,*cp);
      str[j] = *cp;
    }
    str[j] = '\0';
    //    printf("out %d %s\n",j,str);
    if(j != 0){
      if(ks_isfloat_all(str) == KS_FALSE){
	ks_error("'%s' is not float value in option -level_min_c or -level_max_c\n",str);
	return KS_FALSE;
      }
      if((levels[i] = ks_malloc_double_p(1,"levels[i]")) == NULL){
	ks_error_memory();
	return KS_FALSE;
      }
      *levels[i] = atof(str);
    }
    i++;
    if(!(*cp && i < FV_MAX_COLUMN_NUM)) break;
    cp++;
  } 

  /*
  for(j = 0; j < KS_RANGE; j++){
    for(i = 0; i < FV_MAX_COLUMN_NUM; i++){
      printf("%d %d ",j,i);
      if(level_min_max[j][i] == NULL){
	printf("NULL\n");
      } else {
	printf("%f\n",*level_min_max[j][i]);
      }
    }
  }
  */
  //  ks_exit(EXIT_FAILURE);

  return KS_TRUE;
}
int main(int argc, char** argv)
{
  int i,j;

  int full_flg = 0;
  int x_size,y_size;
  int x_pos,y_pos;
  int stereo_flg = 0;
  BOOL proj_flg = KS_FALSE;
  int proj_mode;
  double ax,ay,az;
  double tx,ty,tz;
  double eye_pos[3],rot_pos[3];
  double eye_angle = 30.0;
  char *header_file_name = NULL;
  int initial_detail = CV_SPHERE_DETAIL_NORMAL;
  double level_max,level_min;
  int level_num = CV_FDVIEW_CONTOUR_LEVEL_NUM;
  char *level_min_c = NULL;
  char *level_max_c = NULL;
  double ***level_min_max;
  char *auto_command = NULL;

  BOOL no_mem = KS_FALSE;
  BOOL no_water = KS_FALSE;
  BOOL fv_bw = KS_FALSE;
  BOOL enable_cdv_rmsd_rmsf = KS_FALSE;

  double stereo_len, stereo_width;

  x_pos = 50; y_pos = 0; 
  x_size = 448;  y_size = 448;
  ax = 0; ay = 0; az = 0;
  tx = 0; ty = 0; tz = 0;

  ks_check_memory();
  //  ks_set_exe_path(argv[0]);

  ks_glutInit(&argc, argv);

  gb = ks_allocate_gl_base();

  ks_set_options("-level_num",1,KS_OPTIONS_INT,&level_num,&ops,
		 "set contour level num (default %d)",level_num);
  ks_set_options("-level_max",1,KS_OPTIONS_DOUBLE,&level_max,&ops,"set contour level max");
  ks_set_options("-level_min",1,KS_OPTIONS_DOUBLE,&level_min,&ops,"set contour level min");
  ks_set_options("-level_max_c",1,KS_OPTIONS_CHAR,&level_max_c,&ops,
		 "set contour level max for each column (e.g. 1.0,3.3,,5.0)");
  ks_set_options("-level_min_c",1,KS_OPTIONS_CHAR,&level_min_c,&ops,
		 "set contour level min for each column (e.g. ,-3.3,,0.0)");
  ks_set_options("-detail",1,KS_OPTIONS_INT,&initial_detail,&ops,"sphere detail (default %d)",
		 initial_detail);
  ks_set_options("-c",1,KS_OPTIONS_CHAR,&header_file_name,&ops,"set header for cdview");
  ks_set_options("-eye_pos",3,KS_OPTIONS_DOUBLE,eye_pos,&ops,"set eye position");
  ks_set_options("-eye_angle",1,KS_OPTIONS_DOUBLE,&eye_angle,&ops,"set eye perspective angle (default %.1f)",eye_angle);
  ks_set_options("-rot_pos",3,KS_OPTIONS_DOUBLE,rot_pos,&ops,"set position of rotational center");
  ks_set_options("-no_water",0,KS_OPTIONS_BOOL,&no_water,&ops,"do not read water");
  ks_set_options("-no_mem",0,KS_OPTIONS_BOOL,&no_mem,&ops,"do not read all files in memory");
  ks_set_options("-fv_bw",0,KS_OPTIONS_BOOL,&fv_bw,&ops,"black and white mode in fdview");
  ks_set_options("-cdv_rmsd",0,KS_OPTIONS_BOOL,&enable_cdv_rmsd_rmsf,&ops,
		 "enable calculation of RMSD and RMSF for cdview file");

  ks_set_options("-full",0,KS_OPTIONS_BOOL,&full_flg,&ops,"full screen mode");
  ks_set_options("-stereo",1,KS_OPTIONS_INT,&stereo_flg,&ops,"stereo mode");
  ks_set_options("-s_len",1,KS_OPTIONS_DOUBLE,&stereo_len,&ops,
		 "eye length to target for stereo (default %.2f)",ks_get_gl_base_eye_len(gb));
  ks_set_options("-s_width",1,KS_OPTIONS_DOUBLE,&stereo_width,&ops,
		 "eye width for stereo (default %.2f)",ks_get_gl_base_eye_width(gb));
  ks_set_options("-j",1,KS_OPTIONS_BOOL,&proj_flg,&ops,"ortho mode");
  ks_set_options("-w",1,KS_OPTIONS_INT,&x_size,&ops,"width of window (default %d)",x_size);
  ks_set_options("-h",1,KS_OPTIONS_INT,&y_size,&ops,"height of window (default %d)",y_size);
  ks_set_options("-x",1,KS_OPTIONS_INT,&x_pos,&ops,"x position of window (default %d)",x_pos);
  ks_set_options("-y",1,KS_OPTIONS_INT,&y_pos,&ops,"x position of window (default %d)",y_pos);
  ks_set_options("-ax",1,KS_OPTIONS_DOUBLE,&ax,&ops,"x angle (default %d)",ax);
  ks_set_options("-ay",1,KS_OPTIONS_DOUBLE,&ay,&ops,"y angle (default %d)",ay);
  ks_set_options("-az",1,KS_OPTIONS_DOUBLE,&az,&ops,"z angle (default %d)",az);
  ks_set_options("-tx",1,KS_OPTIONS_DOUBLE,&tx,&ops,"x trans (default %d)",tx);
  ks_set_options("-ty",1,KS_OPTIONS_DOUBLE,&ty,&ops,"y trans (default %d)",ty);
  ks_set_options("-tz",1,KS_OPTIONS_DOUBLE,&tz,&ops,"z trans (default %d)",tz);
  ks_set_options("-auto",1,KS_OPTIONS_CHAR,&auto_command,&ops,"auto command");

  if(cv_is_set_disable_error_hit_enter() == KS_FALSE){
    ks_enable_hit_enter_key();
    ks_enable_error_hit_enter_key();
  }

  if(argc == 1){
    printf("cdview ver.%d.%d.%d ",CV_VER_MAJOR,CV_VER_MINOR,CV_VER_TEENY);
    printf("Copyright (C) 2005-2021 Koishi\n");
    printf("Usage: cdview [FILENAME]\n");
    ks_output_options_comment(ops,stdout);
    ks_hit_enter_key();
    ks_exit(EXIT_FAILURE);
  } else {
    if(ks_classify_options(argc,argv,ops,&input_file_name,&input_file_num,NULL) == KS_FALSE)
      ks_exit(EXIT_FAILURE);
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
    proj_mode = CV_PROJECTION_MODE_ORTHO;
    ks_gl_base_set_projection_mode(gb,KS_GL_BASE_ORTHO);
  } else {
    proj_mode = CV_PROJECTION_MODE_PERSPECTIVE;
    ks_gl_base_set_projection_mode(gb,KS_GL_BASE_PERSPECTIVE);
  }
  if(ks_get_options_hit("-s_len",ops) == KS_TRUE){
    ks_set_gl_base_eye_len(gb,stereo_len);
  }
  if(ks_get_options_hit("-s_width",ops) == KS_TRUE){
    ks_set_gl_base_eye_width(gb,stereo_width);
  }

  if((level_min_max = (double***)ks_malloc(KS_RANGE*sizeof(double**),"level_min_max")) == NULL){
    ks_error_memory();
    ks_exit(EXIT_FAILURE);
  }
  for(i = 0; i < KS_RANGE; i++){
    if((level_min_max[i] = (double**)ks_malloc(FV_MAX_COLUMN_NUM*sizeof(double*),
					       "level_min_max[i]")) == NULL){
      ks_error_memory();
      ks_exit(EXIT_FAILURE);
    }
    for(j = 0; j < FV_MAX_COLUMN_NUM; j++){
      level_min_max[i][j] = NULL;
    }
  }
  if(ks_get_options_hit("-level_max",ops) == KS_TRUE){
    if(set_levels_all(level_max,level_min_max[KS_RANGE_MAX]) == KS_FALSE){
      ks_exit(EXIT_FAILURE);
    }
  }
  if(ks_get_options_hit("-level_min",ops) == KS_TRUE){
    if(set_levels_all(level_min,level_min_max[KS_RANGE_MIN]) == KS_FALSE){
      ks_exit(EXIT_FAILURE);
    }
  }
  if(ks_get_options_hit("-level_max_c",ops) == KS_TRUE){
    if(set_levels(level_max_c,level_min_max[KS_RANGE_MAX]) == KS_FALSE){
      ks_exit(EXIT_FAILURE);
    }
  }
  if(ks_get_options_hit("-level_min_c",ops) == KS_TRUE){
    if(set_levels(level_min_c,level_min_max[KS_RANGE_MIN]) == KS_FALSE){
      ks_exit(EXIT_FAILURE);
    }
  }

  ks_gl_base_set_display_function(gb,display);

  if(stereo_flg == 1)
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO);
  else 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
  glutInitWindowSize(x_size, y_size);
  glutInitWindowPosition(x_pos, y_pos);
  glutCreateWindow("cdview3");

  if(full_flg == KS_TRUE)
    glutFullScreen();

  /*
  init(no_mem,no_water,initial_detail,
  ks_get_options_hit("-c",ops) == KS_TRUE ? header_file_name:NULL);
  */
  if((cb = cv_allocate_base(input_file_name,input_file_num,no_mem,no_water,proj_mode,
			    initial_detail,
			    ks_get_options_hit("-c",ops) == KS_TRUE ? header_file_name:NULL,
			    level_min_max,level_num,fv_bw,enable_cdv_rmsd_rmsf))
     == NULL){
    ks_exit(EXIT_FAILURE);
  }
  init_gl();

  if(ks_get_options_hit("-eye_pos",ops) == KS_TRUE){
    /*    printf("%f %f %f\n",eye_pos[0],eye_pos[1],eye_pos[2]);*/
    ks_set_gl_eye_position(gb,eye_pos);
  } else {
    if(cv_have_side(cb) == KS_TRUE){
      ks_gl_base_set_translational_array(gb,tx,ty,tz,cv_get_system_size(cb)*1.5);
    } else {
      ks_gl_base_set_translational_array(gb,tx,ty,tz,cv_get_system_size(cb));
    }
  }
  if(ks_get_options_hit("-rot_pos",ops) == KS_TRUE){
    ks_set_gl_rotation_center(gb,rot_pos);
  }

  ks_set_gl_perspective_angle(gb,eye_angle);

  if(ks_get_options_hit("-auto",ops) == KS_TRUE){
    if(cv_set_auto_command(cb,auto_command) == KS_FALSE){
      ks_exit(EXIT_FAILURE);
    }
  }
  /*
  printf("%f %f %f\n",gb->trans[0],gb->trans[1],gb->trans[2]);
  printf("%f %f %f\n"
	 ,gb->lookat[KS_GL_LOOKAT_NOW][0]
	 ,gb->lookat[KS_GL_LOOKAT_NOW][1]
	 ,gb->lookat[KS_GL_LOOKAT_NOW][2]);
  */
  ks_gl_base_set_rotational_matrix(gb,ax,ay,az);

  cv_adjust_depth_range(gb,cb);

  glutDisplayFunc(display); 
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutPassiveMotionFunc(passive);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_key);
  glutIdleFunc(idle);
  glutMainLoop();
  return 0;
}

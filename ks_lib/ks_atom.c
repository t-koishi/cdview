
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "ks_std.h"
#include "ks_atom.h"

KS_AMINO_LIST ks_amino[KS_AMINO_LIST_NUM] = {
  { "ALA", 'A', {0.9 ,0.6 ,1.0 },  1.8, KS_AMINO_NO_CHARGE}, /*  0 */
  { "CYS", 'C', {1.0, 1.0, 0.0 },  2.5, KS_AMINO_NO_CHARGE}, /*  1 */
  { "ASP", 'D', {0.8 ,0.1 ,0.6 }, -3.5, KS_AMINO_ACIDIC   }, /*  2 */
  { "GLU", 'E', {0.95,0.1 ,0.1 }, -3.5, KS_AMINO_ACIDIC   }, /*  3 */
  { "PHE", 'F', {0.8 ,0.8 ,0.8 },  2.8, KS_AMINO_NO_CHARGE}, /*  4 */
  { "GLY", 'G', {1.0 ,1.0 ,1.0 }, -0.4, KS_AMINO_NO_CHARGE}, /*  5 */
  { "HIS", 'H', {0.4 ,0.35,0.7 }, -3.2, KS_AMINO_BASIC    }, /*  6 */
  { "ILE", 'I', {0.6 ,0.9 ,0.6 },  4.5, KS_AMINO_NO_CHARGE}, /*  7 */
  { "LYS", 'K', {0.35,0.41,0.98}, -3.9, KS_AMINO_BASIC    }, /*  8 */
  { "LEU", 'L', {0.6 ,0.6 ,0.6 },  3.8, KS_AMINO_NO_CHARGE}, /*  9 */
  { "MET", 'M', {0.9 ,0.8, 0.1 },  1.9, KS_AMINO_NO_CHARGE}, /* 10 */
  { "ASN", 'N', {0.98,0.5 ,0.45}, -3.5, KS_AMINO_POLAR    }, /* 11 */
  { "PRO", 'P', {0.86,0.59,0.51}, -1.9, KS_AMINO_NO_CHARGE}, /* 12 */
  { "GLN", 'Q', {1.0 ,0.08,0.58}, -3.5, KS_AMINO_POLAR    }, /* 13 */
  { "ARG", 'R', {0.08,0.35,1.00}, -4.5, KS_AMINO_BASIC    }, /* 14 */
  { "SER", 'S', {0.88,0.39,0.28}, -0.8, KS_AMINO_POLAR    }, /* 15 */
  { "THR", 'T', {0.98,0.59,0.0 }, -0.7, KS_AMINO_POLAR    }, /* 16 */
  { "VAL", 'V', {0.7, 0.9, 0.7 },  4.2, KS_AMINO_NO_CHARGE}, /* 17 */
  { "TRP", 'W', {1.0 ,0.2 ,0.9 }, -0.9, KS_AMINO_NO_CHARGE}, /* 18 */
  { "TYR", 'Y', {0.9 ,0.3 ,0.7 }, -1.3, KS_AMINO_POLAR    }, /* 19 */
};

BOOL ks_get_atom_number(char *s, int *n)
{
  int i;
  //  char *upper;
  /*
  for(i = 0; ks_atomic[i].number != KS_FALSE; i++){
    printf("%3d %3d %d %-15s %-2s %s\n",i
	   ,ks_atomic[i].number
	   ,ks_atomic[i].number-i
	   ,ks_atomic[i].name
	   ,ks_atomic[i].symbol
	   ,ks_atomic[i].weight);
  }
  exit(0);
  */
#if 0
  {
    char cc[3][100];
    for(i = 0; ks_atomic[i].number != KS_FALSE; i++){
      sprintf(cc[0],"\"%s\",",ks_atomic[i].name);
      sprintf(cc[1],"\"%s\",",ks_atomic[i].symbol);
      sprintf(cc[2],"\"%s\",",ks_atomic[i].weight);
      printf("/* %3d */ {%-16s%-7s%-17s ",i,cc[0],cc[1],cc[2]);
      k = 0;
      for(j = 0; j < ATYPE_NUM; j++){
	if(strcmp(atype_name[j],ks_atomic[i].symbol) == 0){
	  printf("%5.2f,{%5.2f,%5.2f,%5.2f}},\n",atype_radius[j]
		 ,atype_color[j][0],atype_color[j][1],atype_color[j][2]);
	  k = 1;
	}
      }
      if(k == 0){
	printf("10.00,{ 0.00, 0.00, 0.00}},\n");
      }
    }
    for(i = 114; i < 220; i++){
      sprintf(cc[0],"\"\",");
      sprintf(cc[1],"\"\",");
      sprintf(cc[2],"\"\",");
      printf("/* %3d */ {%-16s%-7s%-17s ",i,cc[0],cc[1],cc[2]);
      printf("10.00,{ 0.00, 0.00, 0.00}},\n");
    }
    exit(0);
  }
#endif
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    if(strcmp(s,ks_atomic[i].symbol) == 0){
      /*      printf("ks_get_atomic_number %s %d\n",s,i);*/
      *n = i;
      return KS_TRUE;
    }
  }
  /*
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    upper = ks_upper_all(ks_atomic[i].symbol);
    if(strcmp(s,upper) == 0){
      ks_free(upper);
      return i;
    }
    ks_free(upper);
  }
  */
  return KS_FALSE;
}
BOOL ks_get_atom_radius(char *s, float *r)
{
  int i;
  //  char *upper;
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    if(strcmp(s,ks_atomic[i].symbol) == 0){
      *r =  ks_atomic[i].radius;
      return KS_TRUE;
    }
  }
  /*
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    upper = ks_upper_all(ks_atomic[i].symbol);
    if(strcmp(s,upper) == 0){
      ks_free(upper);
      return ks_atomic[i].radius;
    }
    ks_free(upper);
  }
  */
  return KS_FALSE;
}
BOOL ks_get_atom_color(char *s, float *color)
{
  int i,j;
  //  char *upper;
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    if(strcmp(s,ks_atomic[i].symbol) == 0){
      for(j = 0; j < 3; j++)
	color[j] =  ks_atomic[i].color[j];
      return KS_TRUE;
    }
  }
  /*
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    upper = ks_upper_all(ks_atomic[i].symbol);
    if(strcmp(s,upper) == 0){
      ks_free(upper);
      return ks_atomic[i].color;
    }
    ks_free(upper);
  }
  */
  return KS_FALSE;
}
BOOL ks_get_atom_weight(char *s, double *w)
{
  int i,j;
  char weight[32];
  char *cp;
  //  char *upper;
  /*
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    printf("%3d %d %-15s %-2s %s\n",i
	   ,i
	   ,ks_atomic[i].name
	   ,ks_atomic[i].symbol
	   ,ks_atomic[i].weight);
  }
  exit(0);
  */
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    if(strcmp(s,ks_atomic[i].symbol) == 0){
      if(ks_atomic[i].weight[0] == '[') return KS_FALSE;
      if(ks_atomic[i].weight[0] == '\0'){  // set zero when mass is blank
	*w = 0.0;
	return KS_TRUE;
      }
      for(cp = ks_atomic[i].weight,j=0; *cp != '(' && j < 32; cp++,j++)
	weight[j] = *cp;
      weight[j] = '\0';
      /*      printf("ks_get_atomic_number %s %d %s %s\n",s,i,ks_atomic[i].weight,weight);*/
      *w = atof(weight);
      return KS_TRUE;
    }
  }
  /*
  for(i = 0; strcmp(ks_atomic[i].name,"END") != 0; i++){
    upper = ks_upper_all(ks_atomic[i].symbol);
    if(strcmp(s,upper) == 0){
      if(ks_atomic[i].weight[0] == '[') return 0;
      for(cp = ks_atomic[i].weight,j=0; *cp != '(' && j < 32; cp++,j++)
	weight[j] = *cp;
      weight[j] = '\0';
      ks_free(upper);
      return atof(weight);
    }
    ks_free(upper);
  }
  */
  return KS_FALSE;
}
BOOL ks_get_amino_id(char *s, int *id)
{
  int i;
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(s,ks_amino[i].name) == 0){
      *id = i;
      return KS_TRUE;
    }
  }
  if(strcmp(s,"HID") == 0 || strcmp(s,"HIE") == 0 || strcmp(s,"HIP") == 0){
    *id = 6;
    return KS_TRUE;
  }
  return KS_FALSE;
}
BOOL ks_get_amino_color(char *s, float color[3])
{
  int i;
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(s,ks_amino[i].name) == 0){
      color[0] = ks_amino[i].color[0];
      color[1] = ks_amino[i].color[1];
      color[2] = ks_amino[i].color[2];
      return KS_TRUE;
    }
  }
  if(strcmp(s,"HID") == 0 || strcmp(s,"HIE") == 0 || strcmp(s,"HIP") == 0){
    color[0] = ks_amino[6].color[0];
    color[1] = ks_amino[6].color[1];
    color[2] = ks_amino[6].color[2];
    return KS_TRUE;
  }
  return KS_FALSE;
}
void ks_set_amino_color(char *s, float color[3])
{
  int i;
  if(strcmp(s,"HID") == 0 || strcmp(s,"HIE") == 0 || strcmp(s,"HIP") == 0){
    ks_amino[6].color[0] = color[0];
    ks_amino[6].color[1] = color[1];
    ks_amino[6].color[2] = color[2];
    return;
  }
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(s,ks_amino[i].name) == 0){
      ks_amino[i].color[0] = color[0];
      ks_amino[i].color[1] = color[1];
      ks_amino[i].color[2] = color[2];
    }
  }
}
BOOL ks_get_amino_hydropathy_index(char *s, double *h)
{
  int i;
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(s,ks_amino[i].name) == 0){
      *h = ks_amino[i].hydropathy;
      return KS_TRUE;
    }
  }
  if(strcmp(s,"HID") == 0 || strcmp(s,"HIE") == 0 || strcmp(s,"HIP") == 0){
    *h = ks_amino[6].hydropathy;
    return KS_TRUE;
  }
  return KS_FALSE;
}
BOOL ks_get_amino_type(char *s, int *type)
{
  int i;
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(s,ks_amino[i].name) == 0){
      *type = ks_amino[i].type;
      return KS_TRUE;
    }
  }
  if(strcmp(s,"HID") == 0 || strcmp(s,"HIE") == 0 || strcmp(s,"HIP") == 0){
    *type = ks_amino[6].type;
    return KS_TRUE;
  }
  return KS_FALSE;
}
static void rgb2color(char *rgb, float color[3])
{
  int i,j;
  int h;

  for(i = 0; i < 3; i++){
    h = 0;
    for(j = 0; j < 2; j++){
      h += (isalpha(((unsigned char*)rgb)[i*2+j])?rgb[i*2+j]-'A'+10:rgb[i*2+j]-'0') << (j==0?4:0);
      /*
      printf("%d %d %c %d %d\n",i,j,rgb[i*2+j]
	     ,isalpha(rgb[i*2+j])?rgb[i*2+j]-'A'+10:rgb[i*2+j]-'0',h);
      */
    }
    /*    printf("%x %d\n",h,h);*/
    color[i] = (float)h/255;
  }
}
void ks_print_atom_list(void)
{
  int i;
  char cc[3][100];
  float color[3];

  for(i = 0; i < KS_ATOM_LIST_NUM; i++){
#if 0
    sprintf(cc[0],"\"%s\",",ks_atomic[i].name);
    sprintf(cc[1],"\"%s\",",ks_atomic[i].symbol);
    sprintf(cc[2],"\"%s\",",ks_atomic[i].weight);
    printf("/* %3d */ {%-16s%-7s%-17s ",i,cc[0],cc[1],cc[2]);
    printf("%5.2f,{%5.2f,%5.2f,%5.2f}},\n"
	   ,ks_atomic[i].radius
	   ,ks_atomic[i].color[0],ks_atomic[i].color[1],ks_atomic[i].color[2]);
#endif
    sprintf(cc[0],"\"%s\",",ks_atomic[i].name);
    sprintf(cc[1],"\"%s\",",ks_atomic[i].symbol);
    sprintf(cc[2],"\"%s\",",ks_atomic[i].weight);
    printf("/* %3d */ {%-16s%-7s%-17s ",i,cc[0],cc[1],cc[2]);
    if(i > 0 && i < 109){
      if(strcmp(_atom_colors[i-1].Rasmol_color,"FF1493") == 0){
	rgb2color(_atom_colors[i-1].Jmol_color,color);
      } else {
	rgb2color(_atom_colors[i-1].Rasmol_color,color);
      }
      printf("%5.2f,{%5.2f,%5.2f,%5.2f}},\n"
	     ,ks_atomic[i].radius,color[0],color[1],color[2]);
    } else {
      printf("%5.2f,{%5.2f,%5.2f,%5.2f}},\n"
	     ,ks_atomic[i].radius
	     ,ks_atomic[i].color[0],ks_atomic[i].color[1],ks_atomic[i].color[2]);
    }
  }
}
BOOL ks_get_amino_name_to_char(char *amino_name, char *amino_char)
{
  int i;
  BOOL hit = KS_FALSE;
  *amino_char = '\0';
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(strcmp(ks_amino[i].name,amino_name) == 0){
      *amino_char = ks_amino[i].c;
      hit = KS_TRUE;
    }
  }
  return hit;
}
BOOL ks_get_amino_char_to_name(char amino_char, char *amino_name)
{
  int i;
  BOOL hit = KS_FALSE;
  *amino_name = '\0';
  for(i = 0; i < KS_AMINO_LIST_NUM; i++){
    if(ks_amino[i].c == amino_char){
      strcpy(amino_name,ks_amino[i].name);
      hit = KS_TRUE;
    }
  }
  return hit;
}

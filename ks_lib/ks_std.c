
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#ifdef ICL
#include <mathimf.h>
#else
#include <math.h>
#endif
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#ifdef MSVC
#pragma warning(disable:4101)
#include <windows.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include <time.h>
#ifdef MPI
#include <mpi.h>
#endif
#ifdef VT
#include <VT.h>
#endif

#include "ks_std.h"
#include "ks_math.h"

/*
#if defined(MDGRAPE2) && defined(MDM)
void wine2_free_board(void);
#endif
*/

/*static KS_CHAR_LIST *search_path = NULL;*/
static int ks_mem_hash_size = 1000000;
static KS_MEM_LIST **ks_mem_hash = NULL;
static char ks_error_msg[KS_ERROR_MSG_LEN];
static char ks_exe_path[KS_EXE_PATH_LEN];

BOOL enable_error_hit_enter_key = KS_FALSE;  // used in ks_index_error()
BOOL enable_hit_enter_key = KS_FALSE;        // used in ks_hit_enter_key()

KS_CHAR_LIST *ks_cm_new_char_list(char *name, int value, KS_INDEX_PROTO)
{
  KS_CHAR_LIST *newp;

  if((newp = (KS_CHAR_LIST*)ks_cm_malloc(sizeof(KS_CHAR_LIST),KS_INDEX_ARGS,
					    "KS_CHAR_LIST")) == NULL){
    ks_error("new_char_list: memory error");
    return NULL;
  }
  if((newp->name = ks_malloc_char_copy(name,"new_char_list")) == NULL)
    return NULL;
  newp->value = value;
  newp->next = NULL;
  return newp;
}
KS_CHAR_LIST *ks_add_char_list(KS_CHAR_LIST *listp, KS_CHAR_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
KS_CHAR_LIST *ks_addend_char_list(KS_CHAR_LIST *listp, KS_CHAR_LIST *newp)
{
  KS_CHAR_LIST *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
KS_CHAR_LIST *ks_lookup_char_list(KS_CHAR_LIST *listp, char *name)
{
  for(; listp != NULL; listp = listp->next){
    if(strcmp(listp->name,name) == 0)
      return listp;
  }
  return NULL;
}
KS_CHAR_LIST *ks_lookup_char_list_value(KS_CHAR_LIST *listp, int value)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->value == value)
      return listp;
  }
  return NULL;
}
int ks_count_char_list(KS_CHAR_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
KS_CHAR_LIST *ks_del_char_list_value(KS_CHAR_LIST *listp, int value)
{
  KS_CHAR_LIST *p, *prev;

  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(p->value == value){
      if(prev == NULL)
	listp = p->next;
      else
	prev->next = p->next;
      ks_free(p->name);
      ks_free(p);
      return listp;
    }
    prev = p;
  }
  return listp;
}
void ks_free_char_list(KS_CHAR_LIST *listp)
{
  KS_CHAR_LIST *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->name);
    next = listp->next;
    ks_free(listp);
  }
}
KS_CHAR_LIST_LIST *ks_cm_new_char_list_list(KS_CHAR_LIST *p, KS_INDEX_PROTO)
{
  KS_CHAR_LIST_LIST *newp;
  if((newp = (KS_CHAR_LIST_LIST*)ks_cm_malloc(sizeof(KS_CHAR_LIST_LIST),KS_INDEX_ARGS,
						 "ks_new_char_list_list")) 
     == NULL){
    ks_error("new_char_list_list: memory error");
    return NULL;
  }
  newp->p = p;
  newp->next = NULL;
  return newp;
}
KS_CHAR_LIST_LIST *ks_add_char_list_list(KS_CHAR_LIST_LIST *listp, KS_CHAR_LIST_LIST *newp)
{
  newp->next = listp;
  return newp;
}
void ks_free_char_list_list(KS_CHAR_LIST_LIST *listp)
{
  KS_CHAR_LIST_LIST *next;
  for(; listp != NULL; listp = next){
    ks_free_char_list(listp->p);
    next = listp->next;
    ks_free(listp);
  }
}
KS_INT_LIST *ks_cm_new_int_list(int val, KS_INDEX_PROTO)
{
  KS_INT_LIST *newp;
  if((newp=(KS_INT_LIST*)ks_cm_malloc(sizeof(KS_INT_LIST),KS_INDEX_ARGS,"KS_INT_LIST"))==NULL){
    ks_error("new_int_list: memory error");
    return NULL;
  }
  newp->value = val;
  newp->next = NULL;
  return newp;
}
KS_INT_LIST *ks_add_int_list(KS_INT_LIST *listp, KS_INT_LIST *newp)
{
  newp->next = listp;
  return newp;
}
KS_INT_LIST *ks_cat_int_list(KS_INT_LIST *listp, KS_INT_LIST *newp)
{
  KS_INT_LIST *p;
  for(p = newp; p->next != NULL; p = p->next);
  p->next = listp;
  return newp;
}
KS_INT_LIST *ks_addend_int_list(KS_INT_LIST *listp, KS_INT_LIST *newp)
{
  KS_INT_LIST *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
KS_INT_LIST *ks_lookup_int_list(KS_INT_LIST *listp, int val)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->value == val)
      return listp;
  }
  return NULL;
}
int ks_count_int_list(KS_INT_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
KS_INT_LIST *ks_del_int_list(KS_INT_LIST *listp, int value)
{
  KS_INT_LIST *p, *prev;

  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(p->value == value){
      if(prev == NULL)
	listp = p->next;
      else
	prev->next = p->next;
      ks_free(p);
      return listp;
    }
    prev = p;
  }
  ks_error("%d is not found",value);
  return NULL;
}
void ks_free_int_list(KS_INT_LIST *listp)
{
  KS_INT_LIST *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
KS_INT_LIST2 *ks_cm_new_int_list2(int key, int val, KS_INDEX_PROTO)
{
  KS_INT_LIST2 *newp;
  if((newp=(KS_INT_LIST2*)ks_cm_malloc(sizeof(KS_INT_LIST2),KS_INDEX_ARGS,"KS_INT_LIST2"))==NULL){
    ks_error("new_int_list2: memory error");
    return NULL;
  }
  newp->key = key;
  newp->value = val;
  newp->next = NULL;
  return newp;
}
KS_INT_LIST2 *ks_add_int_list2(KS_INT_LIST2 *listp, KS_INT_LIST2 *newp)
{
  newp->next = listp;
  return newp;
}
KS_INT_LIST2 *ks_cat_int_list2(KS_INT_LIST2 *listp, KS_INT_LIST2 *newp)
{
  KS_INT_LIST2 *p;
  for(p = newp; p->next != NULL; p = p->next);
  p->next = listp;
  return newp;
}
KS_INT_LIST2 *ks_addend_int_list2(KS_INT_LIST2 *listp, KS_INT_LIST2 *newp)
{
  KS_INT_LIST2 *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
KS_INT_LIST2 *ks_lookup_int_list2(KS_INT_LIST2 *listp, int key)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->key == key)
      return listp;
  }
  return NULL;
}
int ks_count_int_list2(KS_INT_LIST2 *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_int_list2(KS_INT_LIST2 *listp)
{
  KS_INT_LIST2 *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
KS_INT_LIST_LIST *ks_cm_new_int_list_list(KS_INT_LIST *p, KS_INDEX_PROTO)
{
  KS_INT_LIST_LIST *newp;
  if((newp = (KS_INT_LIST_LIST*)ks_cm_malloc(sizeof(KS_INT_LIST_LIST),KS_INDEX_ARGS,
						"KS_INT_LIST_LIST")) == NULL){
    ks_error("new_int_list_list: memory error");
    return NULL;
  }
  newp->p = p;
  newp->next = NULL;
  return newp;
}
KS_INT_LIST_LIST *ks_add_int_list_list(KS_INT_LIST_LIST *listp, KS_INT_LIST_LIST *newp)
{
  newp->next = listp;
  return newp;
}
int ks_count_int_list_list(KS_INT_LIST_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_int_list_list(KS_INT_LIST_LIST *listp)
{
  KS_INT_LIST_LIST *next;
  for(; listp != NULL; listp = next){
    ks_free_int_list(listp->p);
    next = listp->next;
    ks_free(listp);
  }
}
KS_WIDEUSE_LIST *ks_cm_new_wideuse_list(char *name, int ip_num, int *ip, int dp_num, double *dp,
					KS_INDEX_PROTO)
{
  int i;
  KS_WIDEUSE_LIST *newp;

  if((newp = (KS_WIDEUSE_LIST*)ks_cm_malloc(sizeof(KS_WIDEUSE_LIST),KS_INDEX_ARGS,
					    "KS_WIDEUSE_LIST")) == NULL){
    ks_error("new_wideuse_list: memory error");
    return NULL;
  }
  if(name == NULL){
    newp->name = NULL;
  } else {
    if((newp->name = ks_malloc_char_copy(name,"new_wideuse_list")) == NULL)
      return NULL;
  }
  if(ip_num == 0){
    newp->ip_num = 0;
    newp->ip = NULL;
  } else {
    newp->ip_num = ip_num;
    if((newp->ip = ks_malloc_int_p(ip_num,"ip")) == NULL){
      return NULL;
    }
    for(i = 0; i < ip_num; i++)
      newp->ip[i] = ip[i];
  }
  if(dp_num == 0){
    newp->dp_num = 0;
    newp->dp = NULL;
  } else {
    newp->dp_num = dp_num;
    if((newp->dp = ks_malloc_double_p(dp_num,"dp")) == NULL){
      return NULL;
    }
    for(i = 0; i < dp_num; i++)
      newp->dp[i] = dp[i];
  }
  newp->next = NULL;
  return newp;
}
KS_WIDEUSE_LIST *ks_add_wideuse_list(KS_WIDEUSE_LIST *listp, KS_WIDEUSE_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
KS_WIDEUSE_LIST *ks_addend_wideuse_list(KS_WIDEUSE_LIST *listp, KS_WIDEUSE_LIST *newp)
{
  KS_WIDEUSE_LIST *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
KS_WIDEUSE_LIST *ks_lookup_wideuse_list(KS_WIDEUSE_LIST *listp, char *name)
{
  for(; listp != NULL; listp = listp->next){
    if(strcmp(listp->name,name) == 0)
      return listp;
  }
  return NULL;
}
int ks_count_wideuse_list(KS_WIDEUSE_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_wideuse_list(KS_WIDEUSE_LIST *listp)
{
  KS_WIDEUSE_LIST *next;
  for(; listp != NULL; listp = next){
    if(listp->name != NULL)
      ks_free(listp->name);
    if(listp->ip != NULL)
      ks_free(listp->ip);
    if(listp->dp != NULL)
      ks_free(listp->dp);
    next = listp->next;
    ks_free(listp);
  }
}
KS_RANGE_LIST *ks_cm_new_range_list(unsigned int flags, double range[2][3], KS_INDEX_PROTO)
{
  KS_RANGE_LIST *newp;

  if((newp = (KS_RANGE_LIST*)ks_cm_malloc(sizeof(KS_RANGE_LIST),KS_INDEX_ARGS,
					    "KS_RANGE_LIST")) == NULL){
    ks_error("new_range_list: memory error");
    return NULL;
  }
  newp->flags = flags;
  ks_copy_range(newp->dpp,range);
  newp->next = NULL;
  return newp;
}
KS_RANGE_LIST *ks_add_range_list(KS_RANGE_LIST *listp, KS_RANGE_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
KS_RANGE_LIST *ks_addend_range_list(KS_RANGE_LIST *listp, KS_RANGE_LIST *newp)
{
  KS_RANGE_LIST *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
int ks_count_range_list(KS_RANGE_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
KS_RANGE_LIST *ks_divide_range_list_periodic_cell(KS_RANGE_LIST *listp, double cell[KS_RANGE][3])
{
  int i,j;
  KS_RANGE_LIST *rg,*rg_next,*new_rg;
  KS_RANGE_LIST *drg,*drg_next;
  double div_range[2][KS_RANGE][3];

  for(rg = listp; rg != NULL; rg = rg_next){
    rg_next = rg->next;
    for(i = 0; i < 3; i++){
      /*
      printf("%d %f %f  %f %f\n",i,cell[KS_RANGE_MIN][i],cell[KS_RANGE_MAX][i],
	     rg->dpp[KS_RANGE_MIN][i],rg->dpp[KS_RANGE_MAX][i]);
      */
      if(rg->dpp[KS_RANGE_MIN][i] < cell[KS_RANGE_MIN][i]){
	/*	printf("div min %d\n",i);*/
	for(drg = rg; drg != NULL && drg != rg_next; drg = drg_next){
	  drg_next = drg->next;
	  /*
	    printf("d(%f %f %f)-(%f %f %f) %p %p\n"
	    ,drg->dpp[KS_RANGE_MIN][0],drg->dpp[KS_RANGE_MIN][1]
	    ,drg->dpp[KS_RANGE_MIN][2],drg->dpp[KS_RANGE_MAX][0]
	    ,drg->dpp[KS_RANGE_MAX][1],drg->dpp[KS_RANGE_MAX][2],drg,drg_next);
	  */
	  for(j = 0; j < 3; j++){
	    if(i == j){
	      div_range[0][KS_RANGE_MIN][j] = cell[KS_RANGE_MIN][j];
	      div_range[0][KS_RANGE_MAX][j] = drg->dpp[KS_RANGE_MAX][j];
	      div_range[1][KS_RANGE_MIN][j] =(drg->dpp[KS_RANGE_MIN][j] + 
					      (cell[KS_RANGE_MAX][j]-cell[KS_RANGE_MIN][j]));
	      div_range[1][KS_RANGE_MAX][j] = cell[KS_RANGE_MAX][j];
	    } else {
	      div_range[0][KS_RANGE_MIN][j] = drg->dpp[KS_RANGE_MIN][j];
	      div_range[0][KS_RANGE_MAX][j] = drg->dpp[KS_RANGE_MAX][j];
	      div_range[1][KS_RANGE_MIN][j] = drg->dpp[KS_RANGE_MIN][j];
	      div_range[1][KS_RANGE_MAX][j] = drg->dpp[KS_RANGE_MAX][j];
	    }
	  }
	  if((new_rg = ks_new_range_list(drg->flags,div_range[0])) == NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }
	  new_rg->next = drg->next;
	  ks_copy_range(drg->dpp,div_range[1]);
	  drg->next = new_rg;
	}
      } else if(rg->dpp[KS_RANGE_MAX][i] >= cell[KS_RANGE_MAX][i]){
	/*	printf("div max %d %f %f\n",i,rg->dpp[KS_RANGE_MAX][i],cell[KS_RANGE_MAX][i]);*/
	for(drg = rg; drg != NULL && drg != rg_next; drg = drg_next){
	  drg_next = drg->next;
	  for(j = 0; j < 3; j++){
	    if(i == j){
	      div_range[0][KS_RANGE_MIN][j] = drg->dpp[KS_RANGE_MIN][j];
	      div_range[0][KS_RANGE_MAX][j] = cell[KS_RANGE_MAX][j];
	      div_range[1][KS_RANGE_MIN][j] = cell[KS_RANGE_MIN][j];
	      div_range[1][KS_RANGE_MAX][j] =(drg->dpp[KS_RANGE_MAX][j] - 
					      (cell[KS_RANGE_MAX][j]-cell[KS_RANGE_MIN][j]));
	    } else {
	      div_range[0][KS_RANGE_MIN][j] = drg->dpp[KS_RANGE_MIN][j];
	      div_range[0][KS_RANGE_MAX][j] = drg->dpp[KS_RANGE_MAX][j];
	      div_range[1][KS_RANGE_MIN][j] = drg->dpp[KS_RANGE_MIN][j];
	      div_range[1][KS_RANGE_MAX][j] = drg->dpp[KS_RANGE_MAX][j];
	    }
	  }
	  if((new_rg = ks_new_range_list(drg->flags,div_range[0])) == NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }
	  new_rg->next = drg->next;
	  ks_copy_range(drg->dpp,div_range[1]);
	  drg->next = new_rg;
	}
      }
    }
  }
  return listp;
}
void ks_free_range_list(KS_RANGE_LIST *listp)
{
  KS_RANGE_LIST *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
#if 0
BOOL ks_error(char *fmt, ...)
{
  int ret;
  va_list argp;
  va_start(argp, fmt);
  if(vfprintf(stderr,fmt, argp) < KS_ERROR_MSG_LEN){
    vsprintf(ks_error_msg,fmt, argp);
    fprintf(stderr,"\n");
#ifdef MSVC
    OutputDebugString(ks_error_msg);
#endif
    ret = KS_TRUE;
  } else {
    fprintf(stderr,"ks_error: error message is too long!!\n");
    ret = KS_FALSE;
  }
  va_end(argp);
  return ret;
}
#endif
#ifdef MSVC
BOOL ks_error(char *fmt, ...)
{
  BOOL ret;
  char msg[KS_ERROR_MSG_LEN];
  va_list argp;
  va_start(argp, fmt);
  if(strlen(fmt) < KS_ERROR_MSG_LEN){
    vsprintf(ks_error_msg,fmt,argp);
    fprintf(stderr,"%s\n",ks_error_msg);
    OutputDebugString(ks_error_msg);
    ret = KS_TRUE;
  } else {
    fprintf(stderr,"ks_error: error message is too long!!\n");
    ret = KS_FALSE;
  }
  va_end(argp);
  if(enable_error_hit_enter_key == KS_TRUE){
    ks_hit_enter_key();
  }
  return ret;
}
#endif
BOOL ks_index_error(KS_INDEX_PROTO, char *fmt, ...)
{
  BOOL ret;
#ifndef NDEBUG
  char msg[KS_ERROR_MSG_LEN];
#endif

  va_list argp;
  va_start(argp, fmt);
  if(strlen(_file_)+strlen(fmt)+(int)log10((double)_line_) < KS_ERROR_MSG_LEN){
#ifdef NDEBUG
    vsprintf(ks_error_msg,fmt,argp);
#else
    vsprintf(msg,fmt,argp);
#ifdef MSVC
    sprintf(ks_error_msg,"%s:%d: ",_file_,_line_);
#else
    sprintf(ks_error_msg,"%s:%d:%s: ",_file_,_line_,_func_);
#endif
    strcat(ks_error_msg,msg);
#endif
    fprintf(stderr,"%s\n",ks_error_msg);
    ret = KS_TRUE;
  } else {
    fprintf(stderr,"ks_error: error message is too long!!\n");
    ret = KS_FALSE;
  }
  va_end(argp);
  if(enable_error_hit_enter_key == KS_TRUE){
    ks_hit_enter_key();
  }
  return ret;
}

void ks_clear_error_message(void)
{
  ks_error_msg[0] = '\0';
}
char *ks_get_error_message(void)
{
  return ks_error_msg;
}
void ks_verbose(int verbose_level, int level, char *fmt, ...)
{
  char s[256];
  va_list argp;
  va_start(argp, fmt);
  if(verbose_level >= level){
    if(vprintf(fmt, argp) < sizeof(s)){
      fflush(stdout);
#ifdef MSVC
      vsprintf(s,fmt, argp);
      OutputDebugString(s);
#endif
    }
  }
  va_end(argp);
}
BOOL ks_iscomment(char *c)
{
  int i;

  for(i = 0; c[i] == ' '; i++);
  if(c[i] == '\'' || c[i] == '#')
    return KS_TRUE;
  else
    return KS_FALSE;
}
BOOL ks_strval(char *buf, char *key, double *v)
{
  int i;
  char c0[256];
  char *cp;

  cp = strstr(buf,key);
  if(cp == NULL){
    return KS_FALSE;
  }
  cp += strlen(key);
  for(; *cp && *cp == ' ' && *cp != 0x0d && *cp != 0x0a;cp++);
  for(i = 0; *cp && *cp != ' ' && *cp != 0x0d && *cp != 0x0a;i++,cp++){
    c0[i] = *cp;
  }
  c0[i] = '\0';
  *v = atof(c0);
  return KS_TRUE;
}
BOOL ks_isfloat(char c)
{
  if((c >= '0' && c <= '9') || c == 'E' || c == 'e' || c == '+' || c == '-' || c == '.')
    return KS_TRUE;
  else
    return KS_FALSE;
}
BOOL ks_isfloat_all(char *str)
{
  int i;
  if((int)strlen(str) == 0 || ks_isreturn(str[0])) return KS_FALSE;
  for(i = 0; str[i]; i++)
    if(ks_isfloat(str[i]) == KS_FALSE)
      return KS_FALSE;
  return KS_TRUE;
}
BOOL ks_isalpha_all(char *str)
{
  int i;
  if((int)strlen(str) == 0 || ks_isreturn(str[0])) return KS_FALSE;
  for(i = 0; str[i]; i++)
    if(isalpha(((unsigned char*)str)[i]) == KS_FALSE)
      return KS_FALSE;
  return  KS_TRUE;
}
BOOL ks_isblank_all(char *str)
{
  int i;
  if((int)strlen(str) == 0 || ks_isreturn(str[0])) return KS_TRUE;
  for(i = 0; str[i]; i++)
    if(str[i] != ' ' && str[i] != 0x0a && str[i] != 0x0d)
      return KS_FALSE;
  return  KS_TRUE;
}
BOOL ks_isdigit_all(char *str)
{
  int i;
  if((int)strlen(str) == 0 || ks_isreturn(str[0])) return KS_FALSE;
  for(i = 0; str[i]; i++)
    if(isdigit(((unsigned char*)str)[i]) == KS_FALSE)
      return KS_FALSE;
  return  KS_TRUE;
}
BOOL ks_store_file_name(KS_STORED_FILE_NAME *sn, char *name)
{
  size_t i;
  size_t el;
  size_t len;

  len = (int)strlen(name);
  if(len == 0){
    ks_error("ks_store_file_name: no file name");
    return KS_FALSE;
  }
  strcpy(sn->all_name,name);
  if(strstr(name,".")){
    for(el = len-1; name[el] != '.'; el--);
    for(i = 0; i < el; i++)
      sn->name[i] = name[i];
    sn->name[i] = '\0';
    for(i = el+1; i < len; i++)
      sn->ext[i-el-1] = name[i];
    sn->ext[i-el-1] = '\0';
  } else {
    strcpy(sn->name,name);
    sn->ext[0] = '\0';
  }

  return KS_TRUE;
}
char *ks_cm_malloc_char_p(size_t n0, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  char *p;

  if((p=(char*)ks_cm_malloc(n0*sizeof(char),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_char_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
char **ks_cm_malloc_char_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg)
{
  int i;
  char err[KS_ERROR_MSG_LEN];
  char **p;

  if((p=(char**)ks_cm_malloc(n0*sizeof(char*),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_char_pp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return (char**)NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(char*)ks_cm_malloc(n1*sizeof(char),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_char_pp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return (char**)NULL;
    }

  return p;
}
int *ks_cm_malloc_int_p(size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  int *p;

  if((p=(int*)ks_cm_malloc(n*sizeof(int),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_int_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }

  return p;
}
unsigned int *ks_cm_malloc_uint_p(size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  unsigned int *p;

  if((p=(unsigned int*)ks_cm_malloc(n*sizeof(unsigned int),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_uint_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }

  return p;
}
char *ks_cm_realloc_char_p(char *op, size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  char *p;

  if((p=(char*)ks_cm_realloc(op,n*sizeof(char),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_char_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }

  return p;
}
int *ks_cm_realloc_int_p(int *op, size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  int *p;

  if((p=(int*)ks_cm_realloc(op,n*sizeof(int),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"realloc_int_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }

  return p;
}
unsigned int *ks_cm_realloc_uint_p(unsigned int *op, size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  unsigned int *p;

  if((p=(unsigned int*)ks_cm_realloc(op,n*sizeof(unsigned int),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"realloc_uint_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }

  return p;
}
int **ks_cm_malloc_int_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg)
{
  int i;
  char err[KS_ERROR_MSG_LEN];
  int **p;

  if((p=(int**)ks_cm_malloc(n0*sizeof(int*),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_int_pp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return (int**)NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(int*)ks_cm_malloc(n1*sizeof(int),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_int_pp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return (int**)NULL;
    }

  return p;
}
int ***ks_cm_malloc_int_ppp(size_t n0, size_t n1, size_t n2, KS_INDEX_PROTO, char *msg)
{
  int i,j;
  char err[KS_ERROR_MSG_LEN];
  int ***p;

  if((p=(int***)ks_cm_malloc(n0*sizeof(int**),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_int_ppp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(int**)ks_cm_malloc(n1*sizeof(int*),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_int_ppp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return NULL;
    }
  for(i = 0; i < n0; i++)
    for(j = 0; j < n1; j++)
      if((p[i][j]=(int*)ks_cm_malloc(n2*sizeof(int),KS_INDEX_ARGS,msg))==NULL){
	if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	  strcpy(err,msg);
	  strcat(err,": memory error\n");
	} else {
	  strcpy(err,"malloc_int_pp: memory error\n");
	}
	ks_index_error(KS_INDEX_ARGS,err);
	return NULL;
      }

  return p;
}
void ks_free_int_ppp(size_t n0, size_t n1, int ***p)
{
  int i,j;
  for(i = 0; i < n0; i++)
    for(j = 0; j < n1; j++)
      ks_free(p[i][j]);
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
/*
unsigned int *ks_malloc_unsigned_int_p(int n, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  unsigned int *p;

  if((p=(unsigned int*)ks_malloc(n*sizeof(unsigned int),msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_unsigned_int_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
*/
/*
unsigned int *ks_realloc_unsigned_int_p(unsigned int *op, int n, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  unsigned int *p;

  if((p=(unsigned int*)ks_realloc(op,n*sizeof(unsigned int),msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_unsigned_int_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
*/
/*
unsigned int **ks_malloc_unsigned_int_pp(int n0, int n1, char *msg)
{
  int i;
  char err[KS_ERROR_MSG_LEN];
  unsigned int **p;

  if((p=(unsigned int**)ks_malloc(n0*sizeof(unsigned int*),msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_unsigned_int_pp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return (unsigned int**)NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(unsigned int*)ks_malloc(n1*sizeof(unsigned int),msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_unsigned_int_pp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return (unsigned int**)NULL;
    }

  return p;
}
*/
float *ks_cm_malloc_float_p(size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  float *p;

  if((p=(float*)ks_cm_malloc(n*sizeof(float),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_float_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
float *ks_cm_realloc_float_p(float *op, size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  float *p;

  if((p=(float*)ks_cm_realloc(op,n*sizeof(float),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_float_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
float **ks_cm_malloc_float_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg)
{
  int i;
  char err[KS_ERROR_MSG_LEN];
  float **p;

  if((p=(float**)ks_cm_malloc(n0*sizeof(float*),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_float_pp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return (float**)NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(float*)ks_cm_malloc(n1*sizeof(float),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_float_pp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return (float**)NULL;
    }

  return p;
}
double *ks_cm_malloc_double_p(size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  double *p;

  if((p=(double*)ks_cm_malloc(n*sizeof(double),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_double_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
double *ks_cm_realloc_double_p(double *op, size_t n, KS_INDEX_PROTO, char *msg)
{
  char err[KS_ERROR_MSG_LEN];
  double *p;

  if((p=(double*)ks_cm_realloc(op,n*sizeof(double),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_double_p: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  return p;
}
double **ks_cm_malloc_double_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg)
{
  int i;
  char err[KS_ERROR_MSG_LEN];
  double **p;

  if((p=(double**)ks_cm_malloc(n0*sizeof(double*),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_double_pp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return (double**)NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(double*)ks_cm_malloc(n1*sizeof(double),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_double_pp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return (double**)NULL;
    }

  return p;
}
double **ks_cm_realloc_double_pp(double **op, size_t n0, size_t n1, KS_INDEX_PROTO, char *msg)
{
  int i;
  char err[KS_ERROR_MSG_LEN];
  double **p;

  if((p=(double**)ks_cm_malloc(n0*sizeof(double*),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error");
    } else {
      strcpy(err,"malloc_double_pp: memory error");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return (double**)NULL;
  }
  for(i = 0; i < n0; i++){
    if((p[i]=ks_cm_realloc_double_p(op[i],n1*sizeof(double),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error");
      } else {
	strcpy(err,"malloc_double_pp: memory error");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return (double**)NULL;
    }
  }
  ks_free(op);
  return p;
}
double ***ks_cm_malloc_double_ppp(size_t n0, size_t n1, size_t n2, KS_INDEX_PROTO, char *msg)
{
  int i,j;
  char err[KS_ERROR_MSG_LEN];
  double ***p;

  if((p=(double***)ks_cm_malloc(n0*sizeof(double**),KS_INDEX_ARGS,msg))==NULL){
    if((int)strlen(msg) < KS_ERROR_MSG_LEN){
      strcpy(err,msg);
      strcat(err,": memory error\n");
    } else {
      strcpy(err,"malloc_double_pp: memory error\n");
    }
    ks_index_error(KS_INDEX_ARGS,err);
    return NULL;
  }
  for(i = 0; i < n0; i++)
    if((p[i]=(double**)ks_cm_malloc(n1*sizeof(double*),KS_INDEX_ARGS,msg))==NULL){
      if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	strcpy(err,msg);
	strcat(err,": memory error\n");
      } else {
	strcpy(err,"malloc_double_pp: memory error\n");
      }
      ks_index_error(KS_INDEX_ARGS,err);
      return NULL;
    }
  for(i = 0; i < n0; i++)
    for(j = 0; j < n1; j++)
      if((p[i][j]=(double*)ks_cm_malloc(n2*sizeof(double),KS_INDEX_ARGS,msg))==NULL){
	if((int)strlen(msg) < KS_ERROR_MSG_LEN){
	  strcpy(err,msg);
	  strcat(err,": memory error\n");
	} else {
	  strcpy(err,"malloc_double_pp: memory error\n");
	}
	ks_index_error(KS_INDEX_ARGS,err);
	return NULL;
      }

  return p;
}
void ks_free_char_pp(size_t n0, char **p)
{
  int i;
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
void ks_free_unsigned_int_pp(size_t n0, unsigned int **p)
{
  int i;
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
void ks_free_int_pp(size_t n0, int **p)
{
  int i;
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
void ks_free_float_pp(size_t n0, float **p)
{
  int i;
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
void ks_free_double_pp(size_t n0, double **p)
{
  int i;
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
void ks_free_double_ppp(size_t n0, size_t n1, double ***p)
{
  int i,j;
  for(i = 0; i < n0; i++)
    for(j = 0; j < n1; j++)
      ks_free(p[i][j]);
  for(i = 0; i < n0; i++)
    ks_free(p[i]);
  ks_free(p);
}
static KS_OPTIONS *new_options(char *key, char *comment, int arg_num, int type, void *vp)
{
  KS_OPTIONS *newp;

  if((newp = (KS_OPTIONS*)ks_malloc(sizeof(KS_OPTIONS),"new_option")) == NULL){
    ks_error("new_options: memory error\n");
    return (KS_OPTIONS*)NULL;
  }
  strcpy(newp->key,key);
  if(comment == NULL)
    newp->comment[0] = KS_FALSE;
  else
    strcpy(newp->comment,comment);
  newp->arg_num = arg_num;
  newp->type = type;
  newp->vp = vp;
  newp->hit = KS_FALSE;
  newp->next = NULL;
  return newp;
}
static KS_OPTIONS *add_options(KS_OPTIONS *listp, KS_OPTIONS *newp)
{
  newp->next = listp;
  return newp;
}
static KS_OPTIONS *lookup_options(KS_OPTIONS *listp, char *name)
{
  int i;
  char buf[KS_OPTIONS_MAX_CHAR_LEN];

  for(i = 0; name[i] != '\0' && !isdigit(((unsigned char*)name)[i]) && name[i] != '.' && 
	(name[i]!='-' || i==0); i++)
    buf[i] = name[i];
  buf[i] = '\0';
  for(; listp != NULL; listp = listp->next){
    if(strcmp(listp->key,buf) == 0)
      return listp;
  }
  return NULL;
}
void ks_free_options(KS_OPTIONS *listp, char **file_name, int file_num)
{
  int i;
  KS_OPTIONS *next;
  for(; listp != NULL; listp = next){
    if(listp->type == KS_OPTIONS_CHAR){
      if(listp->arg_num == 1){
	ks_free(*(char**)listp->vp);
      } else {
	for(i = 0; i < listp->arg_num; i++){
	  ks_free(((char**)listp->vp)[i]);
	}
      }
    }
    next = listp->next;
    ks_free(listp);
  }
  if(file_name != NULL){
    for(i = 0; i < file_num; i++){
      ks_free(file_name[i]);
    }
    ks_free(file_name);
  }
}
BOOL ks_set_options(char *key, int arg_num, int type, void *vp, KS_OPTIONS **op, char *fmt, ...)
{
  int i,len;
  KS_OPTIONS *newp;
  char comment[256];

  va_list argp;
  va_start(argp, fmt);

#if defined(ICL) || defined(MSVC)
  if(strlen(fmt) > sizeof(comment)){
    ks_error("output format is too long");
    return KS_FALSE;
  }
  vsprintf(comment,fmt,argp);
#else
  vsnprintf(comment,sizeof(comment),fmt, argp);
#endif

  len = (int)strlen(key);
  if(len > KS_OPTIONS_MAX_CHAR_LEN){
    ks_error("option key is too long\n");
    return KS_FALSE;
  }
  for(i = 0; i < len; i++){
    if(isdigit(((unsigned char*)key)[i])){
      ks_error("please don't use digit character for option key\n");
      return KS_FALSE;
    }
  }
  if(key[0] != '-'){
    /*
    if(type == KS_OPTIONS_CHAR){
      fprintf(stderr,"can't set KS_OPTIONS_CHAR in ks_set_options\n");
      return KS_FALSE;
    }
    */
    /*
    if(arg_num != 0 && arg_num != 1){
      ks_error("ks_set_options: can't set args '%s'",key);
      return KS_FALSE;
    }
    */
  }
  if(type == KS_OPTIONS_CHAR){
    if(arg_num == 1){
      if(*(char**)vp == NULL){
	if((*(char**)vp = (char*)ks_malloc(KS_OPTIONS_MAX_CHAR_LEN*sizeof(char),"vp")) == NULL){
	  ks_error_memory();
	  return KS_FALSE;
	}
      }
    } else {
      int i;
      for(i = 0; i < arg_num; i++){
	if(((char**)vp)[i] == NULL){
	  if((((char**)vp)[i]=(char*)ks_malloc(KS_OPTIONS_MAX_CHAR_LEN*sizeof(char),"vp"))==NULL){
	    ks_error_memory();
	    return KS_FALSE;
	  }
	}
      }
    }
  }
  if((newp = new_options(key,comment,arg_num,type,vp)) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  *op = add_options(*op,newp);
  return KS_TRUE;
}
static void set_options_value(KS_OPTIONS *op, char **value)
{
  int i;
  void *vp;

  vp = op->vp;
  if(op->type == KS_OPTIONS_BOOL){
    if(op->arg_num == 0)
      *(int*)vp = KS_TRUE;
    else
      *(int*)vp = op->arg_num;
  } else {
    for(i = 0; i < op->arg_num; i++){
      switch(op->type){
      case KS_OPTIONS_CHAR:
	/*	*(char**)vp = *value;*/
	strcpy(*(char**)vp,*value);
	/*
	printf("%p %p %p\n",&vp,vp,value);
	printf("value = %s %p %x %p %s %p\n",*value,*value,(*value)[0]
	       ,*(char**)vp,*(char**)vp,(char**)vp);
	*/
	/*	printf("%d %d\n",((char**)vp)[i],*value);*/
	vp = (char**)vp+1;
	break;
      case KS_OPTIONS_INT:
	if(*value != NULL)
	  *(int*)vp = atoi(*value);
	else
	  *(int*)vp = 0;
	/*	printf("%d '%s'\n",*(int*)vp,*value);*/
	/*	((int*)vp)++;*/
	vp = (int*)vp+1;
	/*	vp += sizeof(int*);*/
	/*	(vp)++;*/
	break;
      case KS_OPTIONS_DOUBLE:
	if(*value != NULL)
	  *(double*)vp = atof(*value);
	else
	  *(double*)vp = 0;
	/*	printf("%f %p '%s' %d\n",*(double*)vp,vp,*value,sizeof(double*));*/
	vp = (double*)vp+1;
	/*	vp += sizeof(double*);*/
	/*	(vp)++;*/
	break;
      }
      if(*value != NULL)
	value++;
      /*
      printf("%p %d\n",value,*value);
      printf("%s\n",*value);
      */
    }
  }
}
static BOOL set_options_value2(KS_OPTIONS *op, char *value)
{

  if(op->type == KS_OPTIONS_BOOL || op->type == KS_OPTIONS_CHAR){
    if(op->type == KS_OPTIONS_BOOL && strcmp(value,op->key) == 0){
      if(op->arg_num == 0)
	*(int*)op->vp = KS_TRUE;
      else
	*(int*)op->vp = op->arg_num;
    } else if(op->type == KS_OPTIONS_CHAR){
      /*      *(char**)op->vp = value;*/
      strcpy(*(char**)op->vp,value);
    }
  } else {
    int i;
    char *cp;
    char c0[256];
    char **vals;
    void *vp;

    if((vals = (char**)ks_malloc(op->arg_num*sizeof(char*),"vals")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    for(i = 0; i < op->arg_num; i++)
      vals[i] = NULL;

    cp = &value[(int)strlen(op->key)];
    for(i = 0; (cp = ks_get_str(",",cp,c0,sizeof(c0))) != NULL; i++){
      /*    printf("%d %s\n",i,c0);*/
      if(i < op->arg_num){
	if((vals[i] = ks_malloc_char_copy(c0,"vals[i]")) == NULL){
	  ks_error_memory();
	  return KS_FALSE;
	}
      } else {
	ks_error("number of args is too large in '%s'",op->key);
	return KS_FALSE;
      }
    }
    /*
    if(i != op->arg_num){
      ks_error("number of args is not match in '%s'",op->key);
      return KS_FALSE;
    }
    */
    vp = op->vp;
    for(i = 0; i < op->arg_num; i++){
      /*      printf("%d %s %p\n",i,vals[i],vp);*/
      if(op->type == KS_OPTIONS_INT){
	if(vals[i] != NULL){
	  *(int*)vp = atoi(vals[i]);
	} else {
	  *(int*)vp = 0;
	}
	/*	((int*)vp)++;*/
	vp = (int*)vp+1;
	/*	vp += sizeof(int*);*/
	/*	printf(" %d %s\n",*(int*)(vp-1),vals[i]);*/
      } else if(op->type == KS_OPTIONS_DOUBLE){
	if(vals[i] != NULL){
	  *(double*)vp = atof(vals[i]);
	} else {
	  *(double*)vp = 0;
	}
	vp = (double*)vp+1;
	/*	((double*)vp)++;*/
	/*	vp += sizeof(double*);*/
      }
    }

    ks_free_char_pp(op->arg_num,vals);

  }
  return KS_TRUE;
}
BOOL ks_get_options_hit(char *key, KS_OPTIONS *op)
{
  KS_OPTIONS *p;

  if((int)strlen(key) > KS_OPTIONS_MAX_CHAR_LEN){
    ks_error("option is too long\n");
    return KS_FALSE;
  }
  if((p = lookup_options(op,key)) == NULL){
    return KS_FALSE;
  } else {
    return p->hit;
  }
}
BOOL ks_set_options_hit(char *key, KS_OPTIONS *op)
{
  KS_OPTIONS *p;

  if((int)strlen(key) > KS_OPTIONS_MAX_CHAR_LEN){
    ks_error("option is too long\n");
    return KS_FALSE;
  }
  if((p = lookup_options(op,key)) == NULL){
    return KS_FALSE;
  } else {
    return p->hit = KS_TRUE;
  }
}
static BOOL analysis_ops_file(char *file_name, int *ac, char **av)
{
  FILE *fp;
  char *cp;
  char read_buf[256];
  char c0[256];

  if((fp = fopen(file_name,"rt")) == NULL){
    ks_error("file open error %s",file_name);
    return KS_FALSE;
  }
  while(fgets(read_buf,sizeof(read_buf),fp) != NULL){
    /*    printf("%s",read_buf);*/
    cp = read_buf;
    while((cp = ks_get_str(" \t",cp,c0,sizeof(c0))) != NULL){
      if(c0[0] == '#') break;
      /*      printf("%d '%s'\n",*ac,c0);*/
      if(av == NULL){
	(*ac)++;
      } else {
	strcpy(av[(*ac)++],c0);
      }
    }
  }

  return KS_TRUE;
}
void ks_print_options_list(KS_OPTIONS *op)
{
  KS_OPTIONS *p;
  for(p = op; p != NULL; p = p->next){
    if(p->type == KS_OPTIONS_CHAR)
      printf("'%s' %p\n",p->key,*(char**)p->vp);
    /*
    else
      printf("'%s'\n",p->key);
    */
  }
}
static BOOL check_file_ext(KS_CHAR_LIST *file_ext, char *file_name)
{
  KS_CHAR_LIST *p;
  char ext[KS_OPTIONS_MAX_FILE_EXT+1];
  char *cp;

  /*  printf("%s\n",file_name);*/
  for(p = file_ext; p != NULL; p = p->next){
    if(strlen(p->name) > KS_OPTIONS_MAX_FILE_EXT){
      ks_error("length of file extention is too long. (max %d)",KS_OPTIONS_MAX_FILE_EXT);
      return KS_FALSE;
    }
    strcpy(ext,".");
    strcat(ext,p->name);
    cp = strstr(file_name,ext);
    if(cp != NULL){
      /*
      printf("%s %s '%s' %d %c\n",p->name,ext,cp,cp[(int)strlen(ext)],cp[(int)strlen(ext)]);
      */
      if(cp[(int)strlen(ext)] == ':' || cp[(int)strlen(ext)] == '\0')
	return KS_TRUE;
    }
  }

  return KS_FALSE;
}
BOOL ks_classify_options(int argc, char **argv, KS_OPTIONS *op, char ***file, int *file_num,
			 KS_CHAR_LIST *file_ext)
{
  int i;
  KS_OPTIONS *p;
  char **file_name;
  int fn;
  int error_flg = 0;
  int ac = 0;
  char **av = NULL;
  /*
  for(p = op; p != NULL; p = p->next){
    printf("%s %d\n",p->key,p->vp);
  }
  */

  if(ks_lookup_char_list(file_ext,"ops") != NULL || ks_lookup_char_list(file_ext,"OPS") != NULL){
    ks_error("Please don't use '%s' for file extention list. It is reserved.");
    return KS_FALSE;
  }

  for(i = 1; i < argc; i++){
    if(strcmp(ks_file_ext(argv[i]),"OPS") == 0 || strcmp(ks_file_ext(argv[i]),"ops") == 0){
      if(analysis_ops_file(argv[i],&ac,av) == KS_FALSE)
	return KS_FALSE;
    } else {
      ac++;
    }
  }

  if((av = ks_malloc_char_pp(ac,KS_OPTIONS_MAX_CHAR_LEN,"av")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }

  ac = 0;
  for(i = 1; i < argc; i++){
    if(strcmp(ks_file_ext(argv[i]),"OPS") == 0 || strcmp(ks_file_ext(argv[i]),"ops") == 0){
      if(analysis_ops_file(argv[i],&ac,av) == KS_FALSE)
	return KS_FALSE;
    } else {
      strcpy(av[ac++],argv[i]);
    }
  }
  /*
  for(i = 0; i < ac; i++){
    printf("%d '%s'\n",i,av[i]);
  }
  */

  fn = 0;
  for(i = 0; i < ac; i++){
    /*    printf("%d %s\n",i,av[i]);*/
    if((int)strlen(av[i]) > KS_OPTIONS_MAX_CHAR_LEN){
      ks_error("length of arg %d is too long\n",i);
      return KS_FALSE;
    }
    if((p = lookup_options(op,av[i])) != NULL){
      /*      printf("  %d %s %d\n",i,av[i],p->arg_num);*/
      if(p->type != KS_OPTIONS_BOOL && p->key[0] == '-')
	i += p->arg_num;
    } else {
      fn++;
    }
  }
  /*
  printf("%d\n",fn);
  ks_exit(0);
  */
  if(file != NULL){
    if((file_name = ks_malloc_char_pp(fn,KS_FILE_MAX_NAME_LEN,"ks_classify_option")) == NULL){
      return KS_FALSE;
    }
  }

  fn = 0;
  for(i = 0; i < ac; i++){
    /*    printf("%d %s\n",i,av[i]);*/
    if((p = lookup_options(op,av[i])) != NULL){
      p->hit = KS_TRUE;
      /*      printf("hit %d %s\n",i,av[i]);*/
      if(p->key[0] == '-'){
	/*	printf("%d %p\n",i,&av[i+1]);*/
	/*	printf("++ %d %s\n",i,av[i+1]);*/
	/*	printf("%d %s %d %d\n",i,av[i],ac-i-1,p->arg_num);*/
	if(ac-i-1 < p->arg_num && p->type != KS_OPTIONS_BOOL){
	  ks_error("arg num of '%s' is too small",p->key);
	  return KS_FALSE;
	}
	set_options_value(p,&av[i+1]);
      } else {
	/*	printf("%d '%s' '%s' '%s'\n",i,p->key,av[i],&av[i][(int)strlen(p->key)]);*/
	if(set_options_value2(p,av[i]) == KS_FALSE){
	  ks_error("opetion error in '%s'",p->key);
	  return KS_FALSE;
	}
      }
      if(p->type != KS_OPTIONS_BOOL && p->key[0] == '-')
	i += p->arg_num;
    } else if(file != NULL){
      /*      printf("%d %s %d\n",i,av[i],fn);*/
      /*      printf("%s\n",file_name[fn]);*/
      if(file_ext != NULL){
	/*
	  {
	  KS_CHAR_LIST *cl;
	  for(cl = file_ext; cl != NULL; cl = cl->next){
	  printf("%s\n",cl->name);
	  }
	  cl = ks_lookup_char_list(file_ext,"pdb");
	  printf("%s\n",cl->name);
	  printf("ext '%s' '%s'\n",ks_file_ext(av[i]),av[i]);
	  }
	*/
	/*	printf("%s\n",ks_file_ext(av[i]));*/
	if(check_file_ext(file_ext,av[i]) == KS_TRUE){
	  strcpy(file_name[fn],av[i]);
	  fn++;
	} else {
	  ks_error("ERROR: unknown file type '%s'",av[i]);
	  error_flg = 1;
	}
      } else {
	strcpy(file_name[fn],av[i]);
	fn++;
      }
    } else {
      ks_error("ERROR: unknown option '%s'",av[i]);
      error_flg = 1;
    }
  }
  if(file != NULL){
    *file = file_name;
    *file_num = fn;
  }

  ks_free_char_pp(ac,av);

  if(error_flg == 0)
    return KS_TRUE;
  else
    return KS_FALSE;

}
BOOL ks_output_options_comment(KS_OPTIONS *op, FILE *fp)
{
  int i,j;
  KS_OPTIONS *p;
  char ***list;
  char *types[4];
  int list_num;
  size_t len,max_len;

  types[KS_OPTIONS_BOOL] = "[b]";
  types[KS_OPTIONS_CHAR] = "[c]";
  types[KS_OPTIONS_INT] = "[i]";
  types[KS_OPTIONS_DOUBLE] = "[d]";

  list_num = 0; max_len = 0;
  for(p = op; p != NULL; p = p->next){
    if(p->comment[0] != KS_FALSE){
      len = (int)strlen(p->key);
      if(p->type != KS_OPTIONS_BOOL)
	len += p->arg_num*(int)strlen(types[p->type]);
      if(p->key[0] == '-'){
	len++;
      } 
      if(p->arg_num > 1){
	len += p->arg_num-1;
      }
      if(max_len < len) max_len = len;
      /*      printf("%s %s %d %d\n",p->key,p->comment,list_num,max_len);*/
      list_num++;
    }
  }
  /*  printf("%d %d\n",list_num,max_len);*/

  if((list = (char***)ks_malloc(list_num*sizeof(char**),"ks_output_options_comment")) == NULL){
    ks_error("ks_output_options_comment: memory error\n");
    return KS_FALSE;
  }
  for(i = 0; i < list_num; i++)
    if((list[i] = (char**)ks_malloc(2*sizeof(char*),"ks_output_options_comment")) == NULL){
      ks_error("ks_output_options_comment: memory error\n");
      return KS_FALSE;
    }
  for(i = 0; i < list_num; i++)
    for(j = 0; j < 2; j++)
      if((list[i][j] = (char*)ks_malloc(KS_OPTIONS_MAX_CHAR_LEN*sizeof(char),
					"ks_output_options_comment")) == NULL){
	ks_error("ks_output_options_comment: memory error\n");
	return KS_FALSE;
      }
  i = list_num-1;
  for(p = op; p != NULL; p = p->next){
    if(p->comment[0] != KS_FALSE){
      strcpy(list[i][0],p->key);
      if(p->type != KS_OPTIONS_BOOL){
	if(p->key[0] == '-')
	  strcat(list[i][0]," ");
	for(j = 0; j < p->arg_num; j++){
	  strcat(list[i][0],types[p->type]);
	  if(j < p->arg_num-1){
	    if(p->key[0] != '-'){
	      strcat(list[i][0],",");
	    } else {
	      strcat(list[i][0]," ");
	    }
	  }
	}
      }
      strcpy(list[i][1],p->comment);
      i--;
    }
  }
  fprintf(fp,"argument: %s:bool %s:char %s:int %s:double\n"
	  ,types[KS_OPTIONS_BOOL]
	  ,types[KS_OPTIONS_CHAR]
	  ,types[KS_OPTIONS_INT]
	  ,types[KS_OPTIONS_DOUBLE]);

  for(i = 0; i < list_num; i++){
    fprintf(fp,"  %-*s : %s\n",(int)max_len,list[i][0],list[i][1]);
    /*    fprintf(fp,"<tr><td>%s</td><td>%s</td></tr>\n",list[i][0],list[i][1]);*/
  }
  for(i = 0; i < list_num; i++)
    for(j = 0; j < 2; j++)
      ks_free(list[i][j]);
  for(i = 0; i < list_num; i++)
    ks_free(list[i]);
  ks_free(list);

  return KS_TRUE;
}
BOOL ks_read_column_data(FILE *fp, KS_COLUMN_DATA *co)
{
  int i;
  int i0,i1;
  char c0[KS_MAX_FILE_DATA_ONE_LINE],c1[KS_MAX_FILE_DATA_ONE_LINE];
  int comment_num,data_num,column_num;

  if(fp == NULL){
    ks_error("ks_read_column_data: file not open");
    return KS_FALSE;
  }

  comment_num = 0;
  data_num = 0;
  while(fgets(c0,sizeof(c0),fp) != NULL){
    if(ks_iscomment(c0)){
      comment_num++;
    } else {
      if(data_num == 0){
	column_num = 0;
	for(i = 0; i < (int)strlen(c0)-1; i++){
	  if(ks_isfloat(c0[i]) && !ks_isfloat(c0[i+1])) column_num++;
	}
      }
      /*      printf("%d %s",data_num,c0);*/
      data_num++;
    }
  }
  rewind(fp);
  /*
  printf("%d %d %d\n",comment_num,column_num,data_num);
  printf("%d %p\n",co->comment_num,co->comment);
  ks_exit(EXIT_FAILURE);
  */
  if(co->comment != NULL && co->comment_num != comment_num){
    ks_free_char_pp(co->comment_num,co->comment);
    co->comment = NULL;
  } 
  if(co->comment == NULL){
    if((co->comment=ks_malloc_char_pp(comment_num,KS_MAX_FILE_DATA_ONE_LINE,
				      "comment"))==NULL){
      return KS_FALSE;
    }
  }
  if(co->d != NULL && (co->num != data_num || co->column_num != column_num)){
    ks_free_double_pp(co->column_num,co->d);
    co->d = NULL;
  }
  if(co->d == NULL){
    if((co->d=ks_malloc_double_pp(column_num,data_num,"data"))==NULL){
      return KS_FALSE;
    }
  }
  co->column_num = column_num;

  co->comment_num = 0;
  co->num = 0;
  while(fgets(c0,sizeof(c0),fp) != NULL){
    if(ks_iscomment(c0)){
      for(i = 0; (co->comment[co->comment_num][i] = c0[i]) != '\n'; i++);
      co->comment[co->comment_num][i] = '\0';
      co->comment_num++;
    } else {
      i0 = 0;
      for(i = 0; i < co->column_num; i++){
	for(; c0[i0] == ' '; i0++);
	for(i1 = 0; ks_isfloat(c1[i1] = c0[i0]); i0++,i1++);
	c1[i1] = '\0';
	co->d[i][co->num] = atof(c1);
      }
      /*      printf("%d\n",co->num);*/
      co->num++;
    }
  }
  return KS_TRUE;
}
void ks_free_column_data(KS_COLUMN_DATA *co)
{
  int i;
  if(co->comment != NULL){
    for(i = 0; i < co->comment_num; i++)
      ks_free(co->comment[i]);
    ks_free(co->comment);
  }
  if(co->d != NULL){
    for(i = 0; i < co->column_num; i++)
      ks_free(co->d[i]);
    ks_free(co->d);
  }
}
void ks_print_error(FILE *fp)
{
  fprintf(fp,"%s",ks_error_msg);
}
BOOL ks_get_common_name(char **name, int name_num, char *common, int size)
{
  int i,j;
  int i0;
  int max_len;
  int *flgs;

  max_len = 0;
  for(i = 0; i < name_num; i++){
    /*    printf("%d %s %d\n",i,name[i],max_len);*/
    i0 = (int)(int)strlen(name[i]);
    if(max_len < i0) max_len = i0;
  }
  if(max_len > size){
    ks_error("file name is too long");
    return KS_FALSE;
  }

  if((flgs = (int*)ks_malloc(max_len*sizeof(int),"flgs")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(i = 0; i < max_len; i++)
    flgs[i] = 1;

  for(i = 1; i < name_num; i++){
    for(j = 0; name[0][j]; j++){
      if(name[i][j] != name[0][j]) flgs[j] = 0;
    }
    /*
    printf("%d %s %s ",i,name[i],name[0]);
    for(j = 0; j < max_len; j++)
      printf("%d",flgs[j]);
    printf("\n");
    */
  }
  for(i = 0,j = 0; name[0][i]; i++){
    if(flgs[i] == 1) common[j++] = name[0][i];
  }
  common[j] = '\0';

  ks_free(flgs);
  return KS_TRUE;
}
char *ks_file_ext(char *s)
{
  if(strstr(s,".")){
    for(; *s; s++);
    for(s--; *s != '.'; s--);
    return ++s;
  } else {
    for(; *s; s++);
    return s;
  }
}
BOOL ks_remove_file_ext(char *name, char *base, size_t size)
{
  int i;
  int el;
  int len;

  len = (int)strlen(name);
  if(len == 0){
    ks_error("ks_remove_file_ext: no file name\n");
    return KS_FALSE;
  }
  if(len > size){
    ks_error("file name %s is too long",name);
    return KS_FALSE;
  }

  if(strstr(name,".")){
    for(el = len-1; name[el] != '.'; el--);
    for(i = 0; i < el; i++)
      base[i] = name[i];
    base[i] = '\0';
  } else {
    strcpy(base,name);
  }

  return KS_TRUE;
}
BOOL ks_change_file_ext(char *in, char *ext, char *out, size_t out_size)
{
  if(ks_remove_file_ext(in,out,out_size) == KS_FALSE){
    return KS_FALSE;
  }
  if(strlen(out)+strlen(ext)+1 > out_size){
    ks_error("size is too short");
    return KS_FALSE;
  }
  strcat(out,".");
  strcat(out,ext);
  return KS_TRUE;
}
double ks_get_time(void)
{
#ifdef MSVC
  static int ini = 0;
  LARGE_INTEGER freq,t;
  static double dfreq;
  if(ini == 0){
    QueryPerformanceFrequency(&freq);
    dfreq = (double)freq.QuadPart;
    ini = 1;
  }
  QueryPerformanceCounter(&t);
  return (double)t.QuadPart/dfreq;
#else
  struct timeval time_v;
  struct timezone time_z;
  gettimeofday(&time_v,&time_z);
  return (time_v.tv_sec + time_v.tv_usec / 1000000.0);
#endif
}

KS_TIMER *ks_cm_allocate_timer(KS_INDEX_PROTO)
{
  KS_TIMER *timer;
  if((timer = (KS_TIMER*)ks_cm_malloc(sizeof(KS_TIMER),KS_INDEX_ARGS,"timer")) == NULL){
    ks_error("ks_allocate_timer: memory error\n");
    return NULL;
  }
  timer->lap = 0;
  timer->sum = 0;
  timer->now = ks_get_time();
#ifdef VT
  VT_classdef("ks_lib",&timer->vt_class);
#endif
  return timer;
}
void ks_index_start_timer(KS_INDEX_PROTO, KS_TIMER *timer)
{
  timer->lap = 0;
  timer->now = ks_get_time();
#ifdef VT
  VT_scldef(_file_,_line_,&timer->vt_scl);
  VT_funcdef(timer->vt_name,timer->vt_class,&timer->vt_func);
  VT_enter(timer->vt_func,timer->vt_scl);
#endif
}
void ks_record_timer(KS_TIMER *timer)
{
  double now;
  now = ks_get_time();
  timer->lap = now - timer->now;
  timer->sum += timer->lap;
  timer->now = now;
#ifdef VT
  VT_leave(timer->vt_scl);
#endif
}
void ks_free_timer(KS_TIMER *timer)
{
  ks_free(timer);
}
KS_COMMINUCATION_INFO *ks_cm_allocate_comminucation_info(KS_INDEX_PROTO)
{
  KS_COMMINUCATION_INFO *info;
  if((info = (KS_COMMINUCATION_INFO*)ks_cm_malloc(sizeof(KS_COMMINUCATION_INFO),KS_INDEX_ARGS,
						     "comm_info")) == NULL){
    ks_error_memory();
    return NULL;
  }
  if((info->timer = ks_allocate_timer()) == NULL){
    return NULL;
  }
  return info;
}
void ks_free_comminucation_info(KS_COMMINUCATION_INFO *info)
{
  ks_free_timer(info->timer);
  ks_free(info);
}
BOOL ks_set_flags_range(int num, int *flg, char *types)
{
  int i,j;
  char *c0;
  int i0,i1,i2;

  if((c0 = (char*)ks_malloc((int)strlen(types)*sizeof(char),"ks_set_flags_range")) == NULL){
    ks_error("ks_set_flags_range: memory error\n");
    return KS_FALSE;
  }

  for(i = 0; i < num; i++)
    flg[i] = KS_FALSE;

  i = 0;
  do{
    for(j = 0; types[i] != ',' && types[i] != '\0'; i++,j++)
      c0[j] = types[i];
    c0[j] = '\0';
    /*      printf("%s\n",c0);*/
    if(strstr(c0,"-")){
      sscanf(c0,"%d-%d",&i0,&i1);
      if(i0 > i1){
	i2 = i0;
	i0 = i1;
	i1 = i2;
      }
      /*	printf("%d %d\n",i0,i1);*/
      if(i0 < 0 || i0 >= num || i1 < 0 || i1 >=num){
	ks_error("ks_set_flags_range: paramter is out of range\n");
	return KS_FALSE;
      }
      for(i = i0; i <= i1; i++)
	flg[i] = KS_TRUE;
    } else {
      i0 = atoi(c0);
      if(i0 < 0 || i0 >= num){
	ks_error("ks_set_flags_range: paramter is out of range\n");
	return KS_FALSE;
      }
      flg[i0] = KS_TRUE;
    }
    if(types[i] == ',') i++;
  } while(types[i] != '\0');

  ks_free(c0);
  return KS_TRUE;
}
KS_BUF_CHAR *ks_cm_allocate_buf_char(size_t size, KS_INDEX_PROTO, char *msg)
{
  KS_BUF_CHAR *b;
  if(size == 0){
    ks_error("ks_allocate_buf_char: size is zero\n");
    return NULL;
  }
  if((b = (KS_BUF_CHAR*)ks_cm_malloc(sizeof(KS_BUF_CHAR),KS_INDEX_ARGS,msg)) == NULL){
    ks_error("ks_allocate_buf_char: memory error\n");
    return NULL;
  }
  b->size = size;
  b->len = 0;
  if((b->buf = ks_cm_malloc_char_p(b->size,KS_INDEX_ARGS,msg)) == NULL){
    return NULL;
  }
  b->buf[0] = '\0';
  return b;
}
BOOL ks_cm_grow_buf_char(KS_BUF_CHAR *b, const size_t new_size, KS_INDEX_PROTO)
{
  if(b->size <= new_size){
    if(ks_cm_realloc_buf_char(b,new_size+1,KS_INDEX_ARGS,"grow_buf_char") == KS_FALSE){
      return KS_FALSE;
    }
    b->size = new_size+1;
  }
  return KS_TRUE;
}
BOOL ks_cm_realloc_buf_char(KS_BUF_CHAR *b, const size_t new_size, KS_INDEX_PROTO, char *msg)
{
  char *bb;

  if((bb = ks_cm_realloc_char_p(b->buf,new_size,KS_INDEX_ARGS,msg)) == NULL){
    return KS_FALSE;
  }
  b->size  = new_size;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_set_buf_char(KS_BUF_CHAR *b, const char *cp, KS_INDEX_PROTO)
{
  int len;
  len = (int)strlen(cp);

  if(len+1 >= b->size){
    if(ks_cm_realloc_buf_char(b,len+1,KS_INDEX_ARGS,"set_buf_char") == KS_FALSE){
      return KS_FALSE;
    }
  }
  strcpy(b->buf,cp);
  b->len = len;
  return KS_TRUE;
}
BOOL ks_cm_cat_buf_char(KS_BUF_CHAR *b, const char *cp, KS_INDEX_PROTO)
{
  int len;
  len = (int)strlen(cp);

  if(b->len+len+1 >= b->size){
    if(ks_cm_realloc_buf_char(b,b->len*2,KS_INDEX_ARGS,"cat_buf_char") == KS_FALSE){
      return KS_FALSE;
    }
  }
  strcat(b->buf,cp);
  b->len = (int)strlen(b->buf);
  return KS_TRUE;
}
void ks_clear_buf_char(KS_BUF_CHAR *b)
{
  ks_assert(b->size > 0);
  b->len = 0;
  b->buf[0] = '\0';
}
void ks_free_buf_char(KS_BUF_CHAR *b)
{
  ks_free(b->buf);
  ks_free(b);
}
KS_BUF_CHAR_PP *ks_cm_allocate_buf_char_pp(int size, int grow, int len,KS_INDEX_PROTO,char *msg)
{
  KS_BUF_CHAR_PP *b;

  if((b = (KS_BUF_CHAR_PP*)ks_cm_malloc(sizeof(KS_BUF_CHAR_PP),KS_INDEX_ARGS,msg)) == NULL){
    ks_error("ks_allocate_buf_char_pp: memory error\n");
    return NULL;
  }
  b->size = size;
  b->grow = grow;
  b->len = len;
  b->num = 0;
  /*  printf("allocate %d %f %d\n",b->size,b->grow,b->len);*/
  if((b->buf = ks_malloc_char_pp(b->size,b->len,"ks_allocate_buf_char_pp")) == NULL){
    return NULL;
  }
  return b;
}
BOOL ks_cm_grow_buf_char_pp(KS_BUF_CHAR_PP *b, KS_INDEX_PROTO)
{
  int i;
  char **bb = NULL;
  /*
  printf("%d\n",b->buf);
  for(i = 0; i < b->size; i++)
    printf("grow %d %d '%s' %f\n",i,&b->buf[i],b->buf[i],b->grow);
  */
  /*
  printf("grow %d -> %d %f\n",b->size,(int)((float)b->size*b->grow),b->grow);
  printf("%d\n",bb);
  printf("%d\n",ks_malloc((int)((float)b->size*b->grow)*sizeof(char*)));
  printf("%d\n",bb = (char**)ks_malloc((int)((float)b->size*b->grow)*sizeof(char*)));
  if(bb == NULL){
    exit(0);
  }
  printf("end malloc\n");
  */
  if((bb = (char**)ks_cm_malloc((int)((float)b->size*b->grow)*sizeof(char*),KS_INDEX_ARGS,
				   "ks_cm_grow_buf_char_pp")) == NULL){
    ks_error("ks_grow_buf_char_pp: memory error\n");
    return KS_FALSE;
  }
  for(i = 0; i < (int)((float)b->size*b->grow); i++)
    if((bb[i] = (char*)ks_cm_malloc(b->len*sizeof(char),KS_INDEX_ARGS,
				       "ks_grow_buf_char_pp")) == NULL){
      ks_error("ks_grow_buf_char_pp: memory error\n");
      return KS_FALSE;
    }
  for(i = 0; i < b->size; i++)
    strcpy(bb[i],b->buf[i]);
  for(i = 0; i < b->size; i++)
    ks_free(b->buf[i]);
  ks_free(b->buf);
  b->size *= b->grow;
  b->buf = bb;
  /*  printf("new size %d\n",b->size);*/
  return KS_TRUE;
}
BOOL ks_cm_set_buf_char_pp(KS_BUF_CHAR_PP *b, int i, char *s, KS_INDEX_PROTO)
{
  if(i >= b->size){
    if(ks_cm_grow_buf_char_pp(b,KS_INDEX_ARGS) == KS_FALSE){
      return KS_FALSE;
    }
  }
  strcpy(b->buf[i],s);
  /*  printf("set %f %d %d '%s'\n",b->grow,i,&b->buf[i],b->buf[i]);*/
  return KS_TRUE;
}
void ks_free_buf_char_pp(KS_BUF_CHAR_PP *b)
{
  int i;
  for(i = 0; i < b->size; i++)
    ks_free(b->buf[i]);
  ks_free(b->buf);
  ks_free(b);
}
KS_BUF_INT *ks_cm_allocate_buf_int(int size, int grow, KS_INDEX_PROTO, char *msg)
{
  KS_BUF_INT *b;

  if((b = (KS_BUF_INT*)ks_cm_malloc(sizeof(KS_BUF_INT),KS_INDEX_ARGS,msg)) == NULL){
    ks_error("ks_allocate_buf_int: memory error\n");
    return NULL;
  }
  b->size = size;
  b->grow = grow;
  b->num = 0;
  if((b->buf = ks_malloc_int_p(b->size,"ks_allocate_buf_int")) == NULL){
    return NULL;
  }
  return b;
}
void ks_free_buf_int(KS_BUF_INT *b)
{
  ks_free(b->buf);
  ks_free(b);
}
BOOL ks_cm_grow_buf_int(KS_BUF_INT *b, KS_INDEX_PROTO, char *msg)
{
  int *bb;

  if((bb = ks_cm_realloc_int_p(b->buf,b->size*b->grow,KS_INDEX_ARGS,msg))==NULL){
    return KS_FALSE;
  }
  b->size *= b->grow;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_realloc_buf_int(KS_BUF_INT *b, size_t size, KS_INDEX_PROTO, char *msg)
{
  int *bb;
  if((bb = ks_cm_realloc_int_p(b->buf,size,KS_INDEX_ARGS,msg)) == NULL){
    return KS_FALSE;
  }
  b->size = size;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_set_buf_int(KS_BUF_INT *b, int i, int v, KS_INDEX_PROTO, char *msg)
{
  if(i >= b->size){
    if(ks_cm_grow_buf_int(b,KS_INDEX_ARGS,msg) == KS_FALSE){
      return KS_FALSE;
    }
  }
  b->buf[i] = v;
  return KS_TRUE;
}
KS_BUF_UINT *ks_cm_allocate_buf_uint(int size, int grow, KS_INDEX_PROTO, char *msg)
{
  KS_BUF_UINT *b;

  if((b = (KS_BUF_UINT*)ks_cm_malloc(sizeof(KS_BUF_UINT),KS_INDEX_ARGS,msg)) == NULL){
    ks_error("ks_allocate_buf_uint: memory error\n");
    return NULL;
  }
  b->size = size;
  b->grow = grow;
  b->num = 0;
  if((b->buf = ks_malloc_uint_p(b->size,"ks_allocate_buf_uint")) == NULL){
    return NULL;
  }
  return b;
}
void ks_free_buf_uint(KS_BUF_UINT *b)
{
  ks_free(b->buf);
  ks_free(b);
}
BOOL ks_cm_grow_buf_uint(KS_BUF_UINT *b, KS_INDEX_PROTO, char *msg)
{
  unsigned int *bb;

  if((bb = ks_cm_realloc_uint_p(b->buf,b->size*b->grow,KS_INDEX_ARGS,msg))==NULL){
    return KS_FALSE;
  }
  b->size *= b->grow;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_realloc_buf_uint(KS_BUF_UINT *b, size_t size, KS_INDEX_PROTO, char *msg)
{
  unsigned int *bb;
  if((bb = ks_cm_realloc_uint_p(b->buf,size,KS_INDEX_ARGS,msg)) == NULL){
    return KS_FALSE;
  }
  b->size = size;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_set_buf_uint(KS_BUF_UINT *b, int i, int v, KS_INDEX_PROTO, char *msg)
{
  if(i >= b->size){
    if(ks_cm_grow_buf_uint(b,KS_INDEX_ARGS,msg) == KS_FALSE){
      return KS_FALSE;
    }
  }
  b->buf[i] = v;
  return KS_TRUE;
}
KS_BUF_FLOAT *ks_cm_allocate_buf_float(int size, int grow, KS_INDEX_PROTO, char *msg)
{
  KS_BUF_FLOAT *b;

  if((b = (KS_BUF_FLOAT*)ks_cm_malloc(sizeof(KS_BUF_FLOAT),KS_INDEX_ARGS,msg)) == NULL){
    ks_error("ks_allocate_buf_float: memory error\n");
    return NULL;
  }
  b->size = size;
  b->grow = grow;
  b->num = 0;
  if((b->buf = ks_malloc_float_p(b->size,"ks_allocate_buf_float")) == NULL){
    return NULL;
  }
  return b;
}
BOOL ks_cm_grow_buf_float(KS_BUF_FLOAT *b, KS_INDEX_PROTO)
{
  float *bb;

  if((bb = ks_cm_realloc_float_p(b->buf,b->size*b->grow,KS_INDEX_ARGS,
				     "ks_grow_buf_float")) == NULL){
    return KS_FALSE;
  }
  b->size *= b->grow;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_realloc_buf_float(KS_BUF_FLOAT *b, size_t size, KS_INDEX_PROTO, char *msg)
{
  float *bb;
  if((bb = ks_cm_realloc_float_p(b->buf,size,KS_INDEX_ARGS,msg)) == NULL){
    return KS_FALSE;
  }
  b->size = size;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_set_buf_float(KS_BUF_FLOAT *b, int i, float v, KS_INDEX_PROTO)
{
  if(i >= b->size){
    if(ks_cm_grow_buf_float(b,KS_INDEX_ARGS) == KS_FALSE){
      return KS_FALSE;
    }
  }
  b->buf[i] = v;
  return KS_TRUE;
}
void ks_free_buf_float(KS_BUF_FLOAT *b)
{
  ks_free(b->buf);
  ks_free(b);
}
KS_BUF_DOUBLE *ks_cm_allocate_buf_double(int size, int grow, KS_INDEX_PROTO, char *msg)
{
  KS_BUF_DOUBLE *b;

  if((b = (KS_BUF_DOUBLE*)ks_cm_malloc(sizeof(KS_BUF_DOUBLE),KS_INDEX_ARGS,msg)) == NULL){
    ks_error("ks_allocate_buf_double: memory error\n");
    return NULL;
  }
  b->size = size;
  b->grow = grow;
  b->num = 0;
  if((b->buf = ks_malloc_double_p(b->size,"ks_allocate_buf_double")) == NULL){
    return NULL;
  }
  return b;
}
BOOL ks_cm_grow_buf_double(KS_BUF_DOUBLE *b, KS_INDEX_PROTO)
{
  double *bb;

  if((bb = ks_cm_realloc_double_p(b->buf,b->size*b->grow,KS_INDEX_ARGS,
				     "ks_grow_buf_double")) == NULL){
    return KS_FALSE;
  }
  b->size *= b->grow;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_realloc_buf_double(KS_BUF_DOUBLE *b, size_t size, KS_INDEX_PROTO, char *msg)
{
  double *bb;
  if((bb = ks_cm_realloc_double_p(b->buf,size,KS_INDEX_ARGS,msg)) == NULL){
    return KS_FALSE;
  }
  b->size = size;
  b->buf = bb;
  return KS_TRUE;
}
BOOL ks_cm_set_buf_double(KS_BUF_DOUBLE *b, int i, double v, KS_INDEX_PROTO)
{
  if(i >= b->size){
    if(ks_cm_grow_buf_double(b,KS_INDEX_ARGS) == KS_FALSE){
      return KS_FALSE;
    }
  }
  b->buf[i] = v;
  return KS_TRUE;
}
void ks_free_buf_double(KS_BUF_DOUBLE *b)
{
  ks_free(b->buf);
  ks_free(b);
}
char* ks_cm_malloc_char_copy(char *c, KS_INDEX_PROTO, char *msg)
{
  char *p;
  if(c == NULL) return NULL;
  if((p = ks_cm_malloc_char_p((int)strlen(c)+1,KS_INDEX_ARGS,msg)) == NULL){
    return NULL;
  }
  strcpy(p,c);
  return p;
}
BOOL ks_add_search_path(KS_CHAR_LIST **search_path, char *path)
{
  char *cp;
  int len;
  if(path == NULL) return KS_FALSE;
  len = (int)strlen(path);
  if(len == 0) return KS_FALSE;
  if(path[len-1] == '/' || path[len-1] == '\\'){
    if((cp = ks_malloc_char_copy(path,"ks_add_search_path")) == NULL)
      return KS_FALSE;
  } else {
    if((cp = ks_malloc_char_p(len+2,"ks_add_search_path")) == NULL)
      return KS_FALSE;
    strcpy(cp,path);
    if(strstr(path,"\\"))
      strcat(cp,"\\");
    else
      strcat(cp,"/");
  }
  if(((*search_path) = ks_addend_char_list((*search_path),ks_new_char_list(cp,0))) == NULL)
    return KS_FALSE;

  ks_free(cp);

  return KS_TRUE;
}
FILE *ks_open_file(char *file_name, char *mode, KS_CHAR_LIST *search_path)
{
  KS_CHAR_LIST *cl;
  KS_BUF_CHAR *cb;
  FILE *fp;

  /*  printf("search_path %p\n",search_path);*/
  if(search_path == NULL){
    if((fp = fopen(file_name,mode)) != NULL){
      return fp;
    } else {
      ks_error("ks_open_file: file open error %s\n",file_name);
      return NULL;
    }
  } else {
    if((cb = ks_allocate_buf_char((int)strlen(file_name)+1,"ks_open_file")) == NULL)
      return NULL;
    for(cl = search_path; cl != NULL; cl = cl->next){
      ks_set_buf_char(cb,cl->name);
      ks_cat_buf_char(cb,file_name);
      /*      printf("'%s'+'%s'='%s' %d\n",cl->name,file_name,cb->buf,(int)strlen(cb->buf));*/
      /*
      ks_verbose(0,0,"'%s'+'%s'='%s' %d\n",cl->name,file_name,cb->buf,(int)strlen(cb->buf));
      */
      if((fp = fopen(cb->buf,mode)) != NULL){
	ks_free_buf_char(cb);
	return fp;
      }
      /*      printf("'%s' fail\n",cb->buf);*/
    }
    ks_error("ks_open_file: file open error %s",file_name);
    ks_free_buf_char(cb);
    return NULL;
  }
}
BOOL ks_isreturn(char c)
{
  if(c == 0x0a || c == 0x0d)
    return KS_TRUE;
  else
    return KS_FALSE;
}
char* ks_get_str(char *list, char *s, char *d, int size)
{
  char *p;
  int i;

  *d = '\0';
  if(s == NULL || *s == '\0' || ks_isreturn(*s) || ks_isblank_all(s) == KS_TRUE) return NULL;
  for(; *s; s++){
    for(p = list; *p; p++){
      if(*s == *p)
	goto KS_GET_STR_SKIP;
    }
    break;
  KS_GET_STR_SKIP:
    ;
  }
  for(i = 0; *s; s++){
    for(p = list; *p; p++){
      if(*s == *p || ks_isreturn(*s) == KS_TRUE){
	d[i] = '\0';
	return s;
      }
    }
    d[i++] = *s;
    if(i >= size){
      d[--i] = '\0';
      return s;
    }
  }
  d[i] = '\0';
  return s;
}
#if 0
char* ks_get_chars(char *list, char *s, char *d, int size)
{
  char *p;
  int i;

  if(s == NULL || *s == '\0' || ks_isreturn(*s)) return NULL;
  for(i = 0; *s; s++){
    for(p = list; *p; p++){
      if(*s == *p){
	d[i] = '\0';
	return s;
      } else if(ks_isreturn(*s) == KS_TRUE){
	d[i] = '\0';
	return s;
      } else if(i >= size){
	d[i] = '\0';
	return s;
      }
      d[i] = *s;
      i++;
    }
  }
  return s;
  /*
  int i,j,len;
  if(s == NULL || (int)strlen(s) == 0 || ks_isreturn(s[0])) return NULL;
  len = (int)strlen(list);
  for(i = 0; s[i]; i++){
    for(j = 0; j < len; j++){
      if(s[i] == list[j]){
	d[i] = '\0';
	s += i;
	return s;
      } else if(ks_isreturn(s[i]) == KS_TRUE){
	d[i] = '\0';
	s += i;
	return s;
      }
      d[i] = s[i];
    }
  }
  s += i;
  return s;
  */
}
#endif
char* ks_skip_chars(char *list, char *s)
{
  int i,j,len;
  if(s == NULL || (int)strlen(s) == 0 || ks_isreturn(s[0])) return NULL;
  len = (int)strlen(list);
  for(i = 0; s[i]; i++){
    for(j = 0; j < len; j++){
      /*      printf("skip %d %d '%c' '%c'\n",i,j,s[i],list[j]);*/
      if(s[i] == list[j]){
	goto KS_SKIP_CHARS_SKIP;
      }
    }
    s += i;
    return s;
  KS_SKIP_CHARS_SKIP:;
  }
  s += i;
  return s;
}
void ks_remove_return(char *s)
{
  /*  printf("remove_ret_code %s %c %c\n",s,*s,*(s+1));*/
  for(; *s ; s++){
    if(*s == 0x0a || *s == 0x0d)
      *s = '\0';
    /*    printf("%c %x\n",*s,*s);*/
  }
}
void ks_remove_end_blank(char *s)
{
  if(*s == '\0') return;
  for(; *s ; s++);
  for(s--; *s == ' '; s--)
    *s = '\0';
}
void ks_init_average_buffer(KS_AVERAGE_BUFFER *av)
{
  av->average = 0;
  av->average2 = 0;
  av->count = 0;
}
void ks_add_average_buffer(KS_AVERAGE_BUFFER *av, double val)
{
  av->average  = (av->average*av->count + val)/(av->count+1);
  av->average2 = (av->average2*av->count + val*val)/(av->count+1);
  av->count++;
}
KS_CIRCLE_BUFFER *ks_cm_allocate_circle_buffer(int num, KS_INDEX_PROTO)
{
  int i;
  KS_CIRCLE_BUFFER *cir;

  if((cir = (KS_CIRCLE_BUFFER*)ks_cm_malloc(sizeof(KS_CIRCLE_BUFFER),KS_INDEX_ARGS,
					       "ks_allocate_circle_buffer")) == NULL){
    ks_error("ks_allocate_circle_buffer: memory error\n");
    return NULL;
  }
  
  if((cir->x = ks_malloc_double_p(num,"ks_allocate_circle_buffre")) == NULL){
    return NULL;
  }
  if((cir->y = ks_malloc_double_p(num,"ks_allocate_circle_buffre")) == NULL){
    return NULL;
  }
  cir->angle = 2.0*M_PI/num;

  for(i = 0; i < num; i++){
    cir->x[i] = cos(cir->angle*i);
    cir->y[i] = sin(cir->angle*i);
  }
  cir->num = num;
  return cir;
}
void ks_free_circle_buffer(KS_CIRCLE_BUFFER *cir)
{
  ks_free(cir->x);
  ks_free(cir->y);
  ks_free(cir);
}
size_t ks_io(KS_IO io, void *p, size_t size, size_t num)
{
  if((io.flags&KS_IO_OUTPUT) &&(io.flags&KS_IO_FILE)){
    return fwrite(p,size,num,io.fp);
  }
  if((io.flags&KS_IO_INPUT) &&(io.flags&KS_IO_FILE)){
    return fread(p,size,num,io.fp);
  }
#ifdef SOCK
  if((io.flags&KS_IO_OUTPUT) &&(io.flags&KS_IO_SOCK)){
    return ks_sock_send(io.sock,p,size*num);
  }
  if((io.flags&KS_IO_INPUT) &&(io.flags&KS_IO_SOCK)){
    return ks_sock_recv(io.sock,p,size*num);
  }
#endif
  ks_assert(0);
  return 0;
}
KS_SPHERE *ks_new_sphere(unsigned int label, char *name, float radius, float color[3])
{
  int i;
  KS_SPHERE *newp;

  if((newp = (KS_SPHERE*)ks_malloc(sizeof(KS_SPHERE),"ks_new_sphere")) == NULL){
    ks_error("new_sphere: memory error");
    return NULL;
  }
  newp->label = label;
  for(i = 0; i < sizeof(newp->name)-1 && name[i]; i++)
    newp->name[i] = name[i];
  newp->name[i] = '\0';
  newp->radius = radius;
  for(i = 0; i < 3; i++)
    newp->color[i] = color[i];
  newp->next = NULL;
  return newp;
}
KS_SPHERE *ks_add_sphere(KS_SPHERE *listp, KS_SPHERE *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
KS_SPHERE *ks_addend_sphere(KS_SPHERE *listp, KS_SPHERE *newp)
{
  KS_SPHERE *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
KS_SPHERE *ks_lookup_sphere(KS_SPHERE *listp, char *name)
{
  for(; listp != NULL; listp = listp->next){
    if(strcmp(listp->name,name) == 0)
      return listp;
  }
  return NULL;
}
int ks_count_sphere(KS_SPHERE *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_sphere(KS_SPHERE *listp)
{
  KS_SPHERE *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
char* ks_upper_all(char *s)
{
  char *up,*p;
  if((up = ks_malloc_char_p((int)strlen(s)+1,"ks_upper_all")) == NULL){
    return NULL;
  }
  for(p = up; *s; ){
    if(isupper(*(unsigned char*)s))
      *(p++) = *(s++);
    else
      *(p++) = toupper(*(unsigned char*)(s++));
  }
  *p = '\0';
  return up;
}
void ks_strncpy(char *d, const char *s, const size_t size)
{
  size_t i;
  for(i = 0; i < size && s[i]; i++)
    d[i] = s[i];
  d[i] = '\0';
}
void ks_strncat(char *d, const char *s, const size_t size)
{
  size_t i,len;
  len = strlen(d);
  for(i = 0; i < size-len-1 && s[i]; i++){
    d[i+len] = s[i];
  }
  d[i+len] = '\0';
}
void ks_marge_range(double total_range[2][3], double range[2][3])
{
  int i;
  for(i = 0; i < 3; i++){
    total_range[KS_RANGE_MIN][i] = ks_min(range[KS_RANGE_MIN][i],total_range[KS_RANGE_MIN][i]);
    total_range[KS_RANGE_MAX][i] = ks_max(range[KS_RANGE_MAX][i],total_range[KS_RANGE_MAX][i]);
  }
}
void ks_copy_range(double d_range[2][3], double s_range[2][3])
{
  int i,j;
  for(i = 0; i < 2; i++)
    for(j = 0; j < 3; j++)
      d_range[i][j] = s_range[i][j];
}
BOOL ks_is_overlap_range(double range0[2][3], double range1[2][3])
{
  BOOL flg = KS_FALSE;

  if(range0[KS_RANGE_MIN][0] < range1[KS_RANGE_MAX][0] && 
     range0[KS_RANGE_MAX][0] > range1[KS_RANGE_MIN][0] &&
     range0[KS_RANGE_MIN][1] < range1[KS_RANGE_MAX][1] && 
     range0[KS_RANGE_MAX][1] > range1[KS_RANGE_MIN][1] &&
     range0[KS_RANGE_MIN][2] < range1[KS_RANGE_MAX][2] && 
     range0[KS_RANGE_MAX][2] > range1[KS_RANGE_MIN][2]){
    flg = KS_TRUE;
  }
  /*
  if(flg == KS_TRUE)
  printf("(%f %f %f)-(%f %f %f)\n(%f %f %f)-(%f %f %f) %d\n"
	 ,range0[KS_RANGE_MIN][0],range0[KS_RANGE_MIN][1],range0[KS_RANGE_MIN][2]
	 ,range0[KS_RANGE_MAX][0],range0[KS_RANGE_MAX][1],range0[KS_RANGE_MAX][2]
	 ,range1[KS_RANGE_MIN][0],range1[KS_RANGE_MIN][1],range1[KS_RANGE_MIN][2]
	 ,range1[KS_RANGE_MAX][0],range1[KS_RANGE_MAX][1],range1[KS_RANGE_MAX][2],flg);
  */
  return flg;
}
BOOL ks_is_overlap_range_list(KS_RANGE_LIST *range0, KS_RANGE_LIST *range1)
{
  KS_RANGE_LIST *r0,*r1;
  BOOL flg = KS_FALSE;
  for(r0 = range0; r0 != NULL; r0 = r0->next){
    for(r1 = range1; r1 != NULL; r1 = r1->next){
      if(ks_is_overlap_range(r0->dpp,r1->dpp) == KS_TRUE)
	flg = KS_TRUE;
    }
  }
  return flg;
}
char *ks_get_date(void)
{
  time_t tm;
  time(&tm);
  return ctime(&tm);
}
KS_LABEL_LIST *ks_new_label_list(unsigned int label, unsigned int value, unsigned int flags)
{
  KS_LABEL_LIST *newp;

  if((newp = (KS_LABEL_LIST*)ks_malloc(sizeof(KS_LABEL_LIST),"new_label_list"))
     == NULL){
    return NULL;
  }
  newp->label = label;
  newp->value = value;
  newp->flags = flags;
  newp->next = NULL;
  return newp;
}
KS_LABEL_LIST *ks_add_label_list(KS_LABEL_LIST *listp, KS_LABEL_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
KS_LABEL_LIST *ks_lookup_label_list(KS_LABEL_LIST* listp, unsigned int label)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->label == label)
      return listp;
  }
  return NULL;
}
int ks_count_label_list(KS_LABEL_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
void ks_free_label_list(KS_LABEL_LIST *listp)
{
  KS_LABEL_LIST *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    ks_free(listp);
  }
}
KS_TEXT_LIST *ks_new_text_list(char *text, double lifetime)
{
  KS_TEXT_LIST *newp;

  if((newp = (KS_TEXT_LIST*)ks_malloc(sizeof(KS_TEXT_LIST),"KS_TEXT_LIST")) == NULL){
    ks_error_memory();
    return NULL;
  }
  newp->lifetime = lifetime;
  if((newp->text = ks_malloc_char_copy(text,"new_text")) == NULL)
    return NULL;
  newp->start_time = ks_get_time();
  newp->next = NULL;
  return newp;
}
KS_TEXT_LIST *ks_add_text_list(KS_TEXT_LIST *listp, KS_TEXT_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
KS_TEXT_LIST *ks_addend_text_list(KS_TEXT_LIST *listp, KS_TEXT_LIST *newp)
{
  KS_TEXT_LIST *p;
  if(listp == NULL)
    return newp;
  for(p = listp; p->next != NULL; p = p->next);
  p->next = newp;
  return listp;
}
KS_TEXT_LIST *ks_lookup_text_list(KS_TEXT_LIST *listp, char *text, int size)
{
  for(; listp != NULL; listp = listp->next){
    if(strncmp(listp->text,text,size) == 0)
      return listp;
  }
  return NULL;
}
int ks_count_text_list(KS_TEXT_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
KS_TEXT_LIST *ks_del_text_list(KS_TEXT_LIST *listp, char *text)
{
  KS_TEXT_LIST *p, *prev;

  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(strcmp(p->text,text) == 0){
      if(prev == NULL)
	listp = p->next;
      else
	prev->next = p->next;
      ks_free(p->text);
      ks_free(p);
      return listp;
    }
    prev = p;
  }
  return listp;
}
KS_TEXT_LIST *ks_del_text_list_time(KS_TEXT_LIST *listp, double time)
{
  KS_TEXT_LIST *p, *prev;

  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(p->lifetime < time-p->start_time){
      if(prev == NULL)
	listp = p->next;
      else
	prev->next = p->next;
      ks_free(p->text);
      ks_free(p);
    }
    prev = p;
  }
  return listp;
}
void ks_free_text_list(KS_TEXT_LIST *listp)
{
  KS_TEXT_LIST *next;
  for(; listp != NULL; listp = next){
    ks_free(listp->text);
    next = listp->next;
    ks_free(listp);
  }
}
BOOL ks_read_dir_first(char *path, KS_DIR_HANDLE *handle, KS_DIR_DATA *data)
{
#ifdef MSVC
  char *search;
  if((search = (char*)ks_malloc((strlen(path)+4)*sizeof(char),"search")) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  strcpy(search,path);
  strcat(search,"\\*");
  *handle = FindFirstFile(search, data);
  if(*handle == INVALID_HANDLE_VALUE){
    return KS_FALSE;
  }
  ks_free(search);
#else
  if((*handle = opendir(path)) == NULL){
    return KS_FALSE;
  }
  *data = readdir(*handle);
#endif
  return KS_TRUE;
}
BOOL ks_read_dir_next(KS_DIR_HANDLE handle, KS_DIR_DATA *data)
{
#ifdef MSVC
  if(!FindNextFile(handle,data)){
    return KS_FALSE;
  }
#else
  if((*data = readdir(handle)) == NULL){
    return KS_FALSE;
  }
#endif
  return KS_TRUE;
}
void ks_close_dir(KS_DIR_HANDLE handle)
{
#ifdef MSVC
  FindClose(handle);
#else
  closedir(handle);
#endif
}
BOOL ks_search_file(char *path, char *file_name)
{
  KS_DIR_HANDLE dir_handle;
  KS_DIR_DATA dir_data;
  char file_name_exe[256];

  ks_strncpy(file_name_exe,file_name,sizeof(file_name_exe));
  if(strlen(file_name_exe)+4 < sizeof(file_name_exe)){
    strcat(file_name_exe,".exe");
  }
  if(ks_read_dir_first(path,&dir_handle, &dir_data) == KS_FALSE){
    ks_error("open dir error %s",path);
    return KS_FALSE;
  }
  do{
    /*    printf("'%s' '%s'\n",file_name,ks_get_dir_file_name(dir_data));*/
    if(strcmp(file_name,ks_get_dir_file_name(dir_data)) == 0 ||
       strcmp(file_name_exe,ks_get_dir_file_name(dir_data)) == 0){
      return KS_TRUE;
    }
  }while(ks_read_dir_next(dir_handle,&dir_data) != KS_FALSE);
  return KS_FALSE;
}
BOOL ks_set_exe_path(char *argv)
{
  int i,j;
  char path[KS_EXE_PATH_LEN];
  char *c0,*c1;
  /*  printf("%s\n",argv);*/
  if(strstr(argv,"/") != NULL || strstr(argv,"\\") != NULL){
    for(i = 0; argv[i]; i++);
    for(;argv[i] != '/' && argv[i] != '\\'; i--);
    for(j = 0; j < i; j++){
      ks_exe_path[j] = argv[j];
    }
    if(strstr(argv,"/") != NULL)
      ks_exe_path[j++] = '/';
    else
      ks_exe_path[j++] = '\\';
    ks_exe_path[j] = '\0';
  } else {
    if(ks_search_file(".",argv) == KS_TRUE){
      strcpy(ks_exe_path,".");
      return KS_TRUE;
    }
    ks_error("stop in ks_set_exe_path");
    ks_exit(EXIT_FAILURE);
    /*    printf("ENV\n%s\n",getenv("PATH"));*/
    c0 = getenv("PATH");
    for(;;){
      c1 = strstr(c0,":");
      /*
      printf("c0 '%s'\n",c0);
      printf("c1 '%s'\n",c1);
      */
      for(i = 0; c0 != c1 && *c0 && i < sizeof(path); c0++, i++){
	/*	printf("%d %c\n",i,*c0);*/
	path[i] = *c0;
      }
      path[i] = '\0';
      /*      printf("path '%s' %d\n",path,ks_search_file(path,argv));*/
      if(ks_search_file(path,argv) == KS_TRUE){
	strcpy(ks_exe_path,path);
	if(ks_exe_path[strlen(ks_exe_path)-1]!='/' && ks_exe_path[strlen(ks_exe_path)-1] != '\\'){
	  if(strlen(ks_exe_path)+1 < KS_EXE_PATH_LEN){
	    if(strstr(ks_exe_path,"/") != NULL){
	      strcat(ks_exe_path,"/");
	    } else {
	      strcat(ks_exe_path,"\\");
	    }
	  } else {
	    ks_error("path name %s is too long",path);
	    return KS_FALSE;
	  }
	}
	return KS_TRUE;
      }
      if(c1 == NULL) break;
      c0 = c1+1;
    }
  }
  return KS_FALSE;
}
char *ks_get_exe_path(void)
{
  return ks_exe_path;
}
#ifdef SOCK
/* ks_sock */
#if defined(_WIN32) && !defined(__CYGWIN__)
static BOOL check_winsock_ver(void)
{
  WSADATA wsaData;

  if(WSAStartup(0x0101, &wsaData) != 0){
    ks_error("Winsock is not installed.");
    return KS_FALSE;
  }
  if(wsaData.wVersion != 0x0101){
    ks_error("Winsock version is not match.\n");
    return KS_FALSE;
  }
  return KS_TRUE;
}
#endif
static KS_SOCK_BASE *allocate_sock_base(int mode)
{
  KS_SOCK_BASE *sb;

  if((sb = (KS_SOCK_BASE*)ks_malloc(sizeof(KS_SOCK_BASE),"KS_SOCK_BASE")) == NULL){
    ks_error_memory();
    return NULL;
  }
  if(mode == KS_SOCK_TCP){
    sb->protocol = IPPROTO_TCP;
    sb->type = SOCK_STREAM;
  } else if(mode == KS_SOCK_UDP){
    sb->protocol = IPPROTO_UDP;
    sb->type = SOCK_DGRAM;
  } else {
    ks_error("Unknown sock type");
    return NULL;
  }
  return sb;
}
static void free_sock_base(KS_SOCK_BASE *sb)
{
  ks_free(sb);
}
KS_SOCK_BASE *ks_sock_open_server(unsigned short int port, int mode)
{
  int ls;                         /* Socket descriptor for server */
  int cs = -1;                    /* Socket descriptor for client */
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  struct sockaddr_in s_address;   /* Local address */
  struct sockaddr_in c_address;   /* Client address */
#else /* WinSock */
  SOCKADDR_IN s_address;   /* Local address */
  SOCKADDR_IN c_address;   /* Client address */
#endif
  unsigned int cl;                /* Length of client address data structure */

  KS_SOCK_BASE *sb;

  if((sb = allocate_sock_base(mode)) == NULL){
    return NULL;
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  if(check_winsock_ver() == KS_FALSE){
    ks_error("winsock version error");
    return NULL;
  }
#endif

  ls = socket(PF_INET, sb->type, sb->protocol);

#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  if (ls < 0) {
    ks_error("socket fail");
    return NULL;
  }
#else /* WinSock */
  if(ls == INVALID_SOCKET){
    ks_error("socket fail");
    return NULL;
  }
#endif

  memset((char *) &s_address, 0, sizeof(s_address));
  s_address.sin_family = AF_INET;
  s_address.sin_port = htons(port);
  s_address.sin_addr.s_addr = htonl(INADDR_ANY);

  {
    int on = 1;
    setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof (on));
  }

#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  if(bind(ls, (struct sockaddr *)&s_address, sizeof(s_address)) < 0) {
    ks_error("sock bind fail");
    close(ls);
    return NULL;
  }
#else /* WinSock */
  if(bind(ls, (LPSOCKADDR)&s_address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR) {
    ks_error("sock bind fail");
    closesocket(ls);
    return NULL;
  }
#endif

  if(sb->protocol == IPPROTO_TCP){
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
    if (listen(ls, 1) < 0) {
      ks_error("sock listen fail");
      return NULL;
    }
#else /* WinSock */
    if (listen(ls, SOMAXCONN) == SOCKET_ERROR) {
      ks_error("sock listen fail");
      return NULL;
    }
#endif

#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
    while(cs < 0) {
      cl = sizeof(c_address);
      if( (cs = accept(ls, (struct sockaddr *)&c_address, &cl)) < 0){
	ks_error("sock accept fail");
      }
    }
    close(ls);
#else /* WinSock */
    while(cs == INVALID_SOCKET) {
      cl = sizeof(SOCKADDR_IN);
      if( (cs = accept(ls, (LPSOCKADDR)&c_address, &cl)) < 0){
	ks_error("sock accept fail");
      }
    }
    closesocket(ls);
#endif
  } else if(sb->protocol == IPPROTO_UDP){
    cs = ls;
  } else {
    ks_error("illegal protocol type");
    return NULL;
  }

  sb->id = cs;

  return sb;
}
static struct in_addr *get_addr(char* address)
{
  struct hostent *host;
  static struct in_addr saddr;

  saddr.s_addr = inet_addr(address);
  /*
  printf("inet_addr\n");
  getchar();
  */
  if (saddr.s_addr != -1) {
    return &saddr;
  }
  host = gethostbyname(address);
  /*
  printf("gethostbyname\n");
  getchar();
  */
  if (host != NULL) {
    return (struct in_addr *) *host->h_addr_list;
  }
  return NULL;
}
KS_SOCK_BASE *ks_sock_open_client(char *host, unsigned short int port, int mode)
{
  int sock, connected;
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  struct sockaddr_in address;
#else /* WinSock */
  SOCKADDR_IN address;
#endif

  struct in_addr *addr;

  KS_SOCK_BASE *sb;

  if((sb = allocate_sock_base(mode)) == NULL){
    return NULL;
  }

#if defined(_WIN32) && !defined(__CYGWIN__)
  if(check_winsock_ver() == KS_FALSE){
    ks_error("winsock version error");
    return NULL;
  }
#endif

  addr = get_addr(host);
  if (addr == NULL) {
    ks_error("make_connection:  Invalid network address.");
    /*    getchar();*/
    return NULL;
  }
  memset((char *) &address, 0, sizeof(address));
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  address.sin_addr.s_addr = addr->s_addr;
  
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  if ((sock = socket(PF_INET, sb->type, sb->protocol)) < 0) {
    ks_error("open socket fail");
    return NULL;
  }
#else /* WinSock */
  if ((sock = socket(PF_INET, sb->type, sb->protocol)) == INVALID_SOCKET) {
    ks_error("open socket fail");
    return NULL;
  }
#endif

  if(sb->protocol == IPPROTO_TCP){
    /*    printf("Connecting to %s on port %d.\n",inet_ntoa(*addr),port);*/
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
    if((connected=connect(sock,(struct sockaddr *)&address,sizeof(address)))<0){
      ks_error("sock connect fail");
      return NULL;
    }
#else /* WinSock */
    if((connected=connect(sock,(LPSOCKADDR)&address,sizeof(SOCKADDR_IN))) == SOCKET_ERROR){
      ks_error("sock connect fail");
      return NULL;
    }
#endif
  } else if(sb->protocol == IPPROTO_UDP){
    memset((char *) &sb->udp_c_address, 0, sizeof(sb->udp_c_address));
    sb->udp_c_address.sin_family = address.sin_family;
    sb->udp_c_address.sin_port = address.sin_port;
    sb->udp_c_address.sin_addr.s_addr = address.sin_addr.s_addr;
  } else {
    ks_error("Illegal protocol number");
    return NULL;
  }
  sb->id = sock;
  return sb;

}
size_t ks_sock_recv(KS_SOCK_BASE *sb, char *buf, size_t count)
{
  size_t bytes_read = 0;
  int this_read;

  if(sb->protocol == IPPROTO_TCP){
    while (bytes_read < count) {
      do
	this_read = recv(sb->id, buf, count - bytes_read,0);
#if !defined(_WIN32) || defined(__CYGWIN__)
      while ( (this_read < 0) && (errno == EINTR) );
#else
      while ( (this_read < 0) );
#endif
      /*      printf("this_read %d\n",this_read);*/
      if (this_read < 0){
	return this_read;
      }
      else if (this_read == 0)
	return bytes_read;
      bytes_read += this_read;
      buf += this_read;
    }
  } else {
    unsigned int address_size;
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
    address_size = sizeof(sb->udp_c_address);
    count = recvfrom(sb->id,buf,count,0,(struct sockaddr *) &sb->udp_c_address, &address_size);
#else /* WinSock */
    address_size = sizeof(SOCKADDR_IN);
    count = recvfrom(sb->id,buf,count,0,(LPSOCKADDR) &sb->udp_c_address, &address_size);
#endif
  }
  return count;
}
size_t ks_sock_send(KS_SOCK_BASE *sb, const char *buf, size_t count)
{
  size_t bytes_sent = 0;
  int this_write;

  if(sb->protocol == IPPROTO_TCP){
    while (bytes_sent < count) {
      do
	this_write = send(sb->id, buf, count - bytes_sent,0);
#if !defined(_WIN32) || defined(__CYGWIN__)
      while ( (this_write < 0) && (errno == EINTR) );
#else
      while ( (this_write < 0));
#endif
      /*      printf("this_write %d\n",this_write);*/
      if (this_write <= 0){
	return this_write;
      }
      bytes_sent += this_write;
      buf += this_write;
    }
  } else {
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
    count = sendto(sb->id,buf,count,0,(struct sockaddr *) &sb->udp_c_address, 
		   sizeof(sb->udp_c_address));
#else /* WinSock */
    count = sendto(sb->id,buf,count,0,(LPSOCKADDR) &sb->udp_c_address, sizeof(SOCKADDR_IN));
#endif
  }
  return count;
}
size_t ks_sock_send_char(KS_SOCK_BASE *sb, char* buf, size_t size)
{
  return ks_sock_send(sb,buf,sizeof(char)*size);
}
size_t ks_sock_send_int(KS_SOCK_BASE *sb, int* buf, size_t size)
{
  int count;
#ifdef SWAP_ENDIAN
  int i,j;
  char c_buf[SIZE_OF_INT];
  int *send_buf_int;

  if((send_buf_int = malloc(size * sizeof(int))) == NULL){
    printf("memory error\n");
    return -1;
  }

  for(i = 0; i < size; i++){
    for(j = 0; j < SIZE_OF_INT; j++){
      c_buf[SIZE_OF_INT-1-j] = ((char*)&buf[i])[j];
      send_buf_int[i] = *(int*)c_buf;
    }
  }
  count = ks_sock_send(sb,send_buf_int,sizeof(int)*size);

  free(send_buf_int);
#else
  count = ks_sock_send(sb,(char*)buf,sizeof(int)*size);
#endif
  return count;
}
size_t ks_sock_send_double(KS_SOCK_BASE *sb, double* buf, size_t size)
{
  int count;
#ifdef SWAP_ENDIAN
  int i,j;
  char c_buf[SIZE_OF_DOUBLE];
  double *send_buf_double;

  if((send_buf_double = malloc(size*sizeof(double))) == NULL){
    printf("memory error\n");
    return -1;
  }

  for(i = 0; i < size; i++){
    for(j = 0; j < SIZE_OF_DOUBLE; j++){
      c_buf[SIZE_OF_DOUBLE-1-j] = ((char*)&buf[i])[j];
      send_buf_double[i] = *(double*)c_buf;
    }
  }
  count = ks_sock_send(sb,send_buf_double,sizeof(double)*size);

  free(send_buf_double);
#else
  count = ks_sock_send(sb,(char*)buf,sizeof(double)*size);
#endif
  return count;
}
size_t ks_sock_recv_char(KS_SOCK_BASE *sb, char* buf, size_t size)
{
  return ks_sock_recv(sb,buf,sizeof(char)*size);
}
size_t ks_sock_recv_int(KS_SOCK_BASE *sb, int* buf, size_t size)
{
  int count;
#ifdef SWAP_ENDIAN
  int i,j;
  char c_buf[SIZE_OF_INT];
  int *send_buf_int;

  if((send_buf_int = malloc(size * sizeof(int))) == NULL){
    printf("memory error\n");
    return -1;
  }

  count = ks_sock_recv(sb,send_buf_int,sizeof(int)*size);

  for(i = 0; i < size; i++){
    for(j = 0; j < SIZE_OF_INT; j++){
      c_buf[SIZE_OF_INT-1-j] = ((char*)&send_buf_int[i])[j];
      buf[i] = *(int*)c_buf;
    }
  }

  free(send_buf_int);
#else
  count = ks_sock_recv(sb,(char*)buf,sizeof(int)*size);
#endif
  return count;
}
size_t ks_sock_recv_double(KS_SOCK_BASE *sb, double* buf, size_t size)
{
  int count;
#ifdef SWAP_ENDIAN
  int i,j;
  char c_buf[SIZE_OF_DOUBLE];
  double *send_buf_double;

  if((send_buf_double = malloc(size*sizeof(double))) == NULL){
    printf("memory error\n");
    return -1;
  }

  count = ks_sock_recv(sb,send_buf_double,sizeof(double)*size);

  for(i = 0; i < size; i++){
    for(j = 0; j < SIZE_OF_DOUBLE; j++){
      c_buf[SIZE_OF_DOUBLE-1-j] = ((char*)&send_buf_double[i])[j];
      buf[i] = *(double*)c_buf;
    }
  }

  free(send_buf_double);
#else
  count = ks_sock_recv(sb,(char*)buf,sizeof(double)*size);
#endif
  return count;
}
void ks_sock_close(KS_SOCK_BASE *sb)
{
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  /*  printf("sock_close\n");*/
  close(sb->id);
#else
  closesocket(sb->id);
#endif
  free_sock_base(sb);
}
/* ks_sock end */
#endif /* SOCK */

KS_INT_LIST *ks_expand_sequence(char *c)
{
  int i;
  int val[2];
  int sw;
  BOOL range = KS_FALSE;
  KS_INT_LIST *il = NULL;
  char *cp;
  char c0[64];

  for(cp = c; (cp = ks_get_str(",-",cp,c0,sizeof(c0))) != NULL;){
    /*    printf("'%s' '%s'\n",c0,cp);*/
    if(range == KS_TRUE){
      range = KS_FALSE;
      val[1] = atoi(c0);
      /*      printf("range %d %d\n",val[0],val[1]);*/
      if(val[0] > val[1]){
	sw = val[0];
	val[0] = val[1];
	val[1] = sw;
      }
      for(i = val[0]; i <= val[1]; i++){
	if(ks_lookup_int_list(il,i) == NULL){
	  il = ks_add_int_list(il,ks_new_int_list(i));
	}
      }
    } else {
      val[0] = atoi(c0);
      if(*cp == '-'){
	range = KS_TRUE;
      } else {
	if(ks_lookup_int_list(il,val[0]) == NULL){
	  il = ks_add_int_list(il,ks_new_int_list(val[0]));
	}
      }
    }
  }
  return il;
}
void ks_get_file_path(char *file_name, char *path, size_t path_size)
{
  int i;
  char *cp;

  if(strstr(file_name,"\\") == NULL && strstr(file_name,"/") == NULL){
    path[0] = '\0';
    return;
  }
  for(cp = file_name; *cp; cp++);
  for(;*cp != '\\' && *cp != '/'; cp--);
  for(i = 0; i < path_size-1; i++){
    path[i] = file_name[i];
    if(&file_name[i] == cp){
      if(i < path_size) i++;
      break;
    }
  }
  path[i] = '\0';
}
int ks_fprintf2(FILE *fp1, FILE *fp2, const char *fmt,...)
{
  int ret;
  va_list argp;
  va_start(argp, fmt);
  ret = vfprintf(fp1,fmt, argp);
  ret = vfprintf(fp2,fmt, argp);
  return ret;
}
void ks_enable_error_hit_enter_key()
{
  enable_error_hit_enter_key = KS_TRUE;
}
void ks_disable_error_hit_enter_key()
{
  enable_error_hit_enter_key = KS_FALSE;
}
void ks_enable_hit_enter_key()
{
  enable_hit_enter_key = KS_TRUE;
}
void ks_disable_hit_enter_key()
{
  enable_hit_enter_key = KS_FALSE;
}
void ks_hit_enter_key()
{
  if(enable_hit_enter_key == KS_TRUE){
    printf("\n--- Hit Enter Key ---\n");
    fflush(stderr);
    fflush(stdout);
    getchar();
  }
}
BOOL ks_replace_string(char *input, int size, char *output,
		       const char *search, const char *replace)
{
  int oi,i;
  int search_size,replace_size;
  char *hit;
  search_size = strlen(search);
  replace_size = strlen(replace);
  if(replace_size > size){
    return KS_FALSE;
  }
  if(strstr(input,search) == NULL){
    return KS_FALSE;
  }
  oi = 0;
  while((hit = strstr(input,search)) != NULL){
    for(; input != hit; oi++,input++){
      if(oi >= size){ output[0] = '\0';	return KS_FALSE; }
      output[oi] = *input;         // copy characters before the search key
    }
    for(i = 0; i < replace_size; i++,oi++){
      if(oi >= size){ output[0] = '\0';	return KS_FALSE; }
      output[oi] = replace[i];     // copy characters from the replace key
    }
    input += search_size;          // move pointer to the end of search key
  }
  for(; *input != '\0'; oi++,input++){
    if(oi >= size){ output[0] = '\0';	return KS_FALSE; }
    output[oi] = *input;           // copy characters after the search key
  }
  output[oi] = '\0';
  return KS_TRUE;
}
int ks_printf_flush_stdout(const char* fmt, ...)
{
  int ret;
  va_list argp;
  va_start(argp, fmt);
  ret = vprintf(fmt, argp);
  fflush(stdout);
  va_end(argp);
  return ret;
}
static long long int get_file_size(const char* file_name)
{
  struct stat st;
  if(stat(file_name, &st) != 0){
    return -1LL;
  }
  if((st.st_mode & S_IFMT) != S_IFREG){
    return -1LL;
  }
  return st.st_size;
}
KS_FILE_LINES* ks_allocate_file_lines()
{
  KS_FILE_LINES *fi;
  if((fi = (KS_FILE_LINES*)ks_malloc(sizeof(KS_FILE_LINES),"KS_FILE_LINES fi")) == NULL){
    ks_error_memory();
    return NULL;
  }
  fi->read_buf = NULL;
  fi->file_lines = NULL;
  fi->line_cnt = 0;
  fi->line_capacity = 0;
  fi->size = 0;
  return fi;
}
void ks_free_file_lines(KS_FILE_LINES *fi)
{
  if(fi->read_buf != NULL){
    ks_free(fi->read_buf);
  }
  if(fi->file_lines != NULL){
    ks_free(fi->file_lines);
  }
  ks_free(fi);
}
BOOL ks_read_file_lines(const char* file_name, KS_FILE_LINES* fi)
{
  long long int i;
  int ret_flag;
  FILE *fp;
  long long int size;
  long long int line_cnt;
  size_t lf_cnt, cr_cnt;

  size = get_file_size(file_name);
  if(size == -1LL){
    return KS_FALSE;
  }

  if(fi->read_buf == NULL){
    fi->size = size;
    if((fi->read_buf = (char*)ks_malloc(sizeof(char)*fi->size,"fi->read_buf")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  } else if(size > fi->size){
    fi->size = size;
    if((fi->read_buf = (char*)ks_realloc(fi->read_buf,sizeof(char)*size,"fi->read_buf")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }
  memset(fi->read_buf,0,sizeof(char)*fi->size); // clear memory

  if((fp = fopen(file_name,"rb")) == NULL){
    ks_error_file(file_name);
    return KS_FALSE;
  }
  fread(fi->read_buf,fi->size,1,fp);
  fclose(fp);

  // count line number
  lf_cnt = 0; cr_cnt = 0;
  for(i = 0; i < fi->size; i++){
    if(fi->read_buf[i] == 0x0a) lf_cnt++;   // LF
    if(fi->read_buf[i] == 0x0d) cr_cnt++;   // CR
  }
  ks_assert((lf_cnt == 0 && cr_cnt != 0) || // Mac
	    (lf_cnt != 0 && cr_cnt == 0) || // UNIX
	    (lf_cnt == cr_cnt));            // Windows
  line_cnt = ks_max(lf_cnt,cr_cnt);

  if(fi->file_lines == NULL){
    fi->line_capacity = line_cnt;
    if((fi->file_lines = (char**)ks_malloc(sizeof(char*)*fi->line_capacity,
					   "fi->file_lines")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  } else if(line_cnt > fi->line_capacity){
    fi->line_capacity = line_cnt;
    if((fi->file_lines = (char**)ks_realloc(fi->file_lines,sizeof(char*)*fi->line_capacity,
					    "fi->file_lines")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }

  // replace LF and CR to '\0' and store the head pointer to file_lines
  ret_flag = 1;
  fi->line_cnt = 0;
  for(i = 0; i < fi->size; i++){
    if(fi->read_buf[i] == '\0') break;
    if(fi->read_buf[i] == 0x0d || fi->read_buf[i] == 0x0a){
      fi->read_buf[i] = '\0';
      ret_flag = 1;
    } else if(ret_flag == 1){
      fi->file_lines[fi->line_cnt++] = &fi->read_buf[i];
      ret_flag = 0;
    }
  }
  ks_assert(line_cnt >= fi->line_cnt);

  return KS_TRUE;
}

/**************** following funcsions must be bottom of this file ********************/

#undef malloc
#undef realloc
#undef free
#undef exit

static void avoid_output_mem_list(void)
{
  ks_mem_hash_size = 0;
}
void ks_exit(int i)
{
#ifdef MPI
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize(); 
#endif
#if defined(MDGRAPE2) && defined(MDM)
  m2_gm_finalize();
  /*  wine2_free_board();*/
#endif
  if(i == EXIT_FAILURE)
    atexit(avoid_output_mem_list);
  exit(i);
}
void ks_wait_debug(void)
{
  int i = 1;
  while(i);
}
#ifndef NO_CHECK_MEMORY
static KS_MEM_LIST *new_mem_list(void *p, size_t size, KS_INDEX_PROTO, char *comment)
{
  KS_MEM_LIST *newp;
  if((newp = (KS_MEM_LIST*)malloc(sizeof(KS_MEM_LIST))) == NULL){
    ks_error("new_mem_list: memory error");
    return NULL;
  }
  newp->ptr = p;
  newp->size = size;
  newp->file = _file_;
  newp->line = _line_;
#ifndef MSVC
  newp->func = _func_;
#endif
  newp->comment = comment;
  newp->next = NULL;
  return newp;
}
static KS_MEM_LIST *add_mem_list(KS_MEM_LIST *listp, KS_MEM_LIST *newp)
{
  if(newp == NULL) return listp;
  newp->next = listp;
  return newp;
}
static KS_MEM_LIST* __KS_USED__ lookup_mem_list(KS_MEM_LIST *listp, void *p)
{
  for(; listp != NULL; listp = listp->next){
    if(listp->ptr == p)
      return listp;
  }
  return NULL;
}
static int count_mem_list(KS_MEM_LIST *listp)
{
  int c = 0;
  for(; listp != NULL; listp = listp->next)
    c++;
  return c;
}
static KS_MEM_LIST *del_mem_list(KS_MEM_LIST *listp, void *ptr, KS_INDEX_PROTO)
{
  KS_MEM_LIST *p, *prev;
  prev = NULL;
  for(p = listp; p != NULL; p = p->next){
    if(p->ptr == ptr){
      if(prev == NULL){
	listp = p->next;
      } else {
	prev->next = p->next;
      }
      free(p);
      return listp;
    }
    prev = p;
  }
  /*
  printf("%p\n",listp);
  for(p = listp; p != NULL; p = p->next){
    fprintf(stderr,"%p %s %d %s\n",p->ptr,p->_file_,p->_line_,p->comment);
  }
  */
#ifdef MSVC
  ks_error("del_mem_list: %p not found %s %d\n",ptr,_file_,_line_);
#else
  ks_error("del_mem_list: %p not found %s %d %s\n",ptr,_file_,_line_,_func_);
#endif
  ks_abort();
  return NULL;
}
#endif
void ks_check_mem_list(void)
{
  KS_MEM_LIST *p;
  for(p = ks_mem_hash[608074]; p != NULL; p = p->next){
#if __SIZEOF_SIZE_T__ == 4
    printf("ks_check_mem_list %p: %s %d %s %u\n",p,p->file,p->line,p->comment,p->size);
#else
# ifdef _WIN32
    printf("ks_check_mem_list %p: %s %d %s %I64u\n",p,p->file,p->line,p->comment,p->size);
# else
    printf("ks_check_mem_list %p: %s %d %s %llu\n",p,p->file,p->line,p->comment,p->size);
# endif
#endif
  }
}
#ifndef NO_CHECK_MEMORY
static void output_mem_list(void)
{
  int i;
  KS_MEM_LIST *p;
  for(i = 0; i < ks_mem_hash_size; i++){
    /*    printf("%d %d\n",i,count_mem_list(ks_mem_hash[i]));*/
    for(p = ks_mem_hash[i]; p != NULL; p = p->next){
      if(p->comment != NULL)
	fprintf(stderr,"MEMORY LEAK  %s:%d:%s: %s\n",p->file,p->line,p->func,p->comment);
      else
	fprintf(stderr,"MEMORY LEAK  %s:%d:%s: \n",p->file,p->line,p->func);
    }
  }
}
static void free_mem_list(KS_MEM_LIST *listp)
{
  KS_MEM_LIST *next;
  for(; listp != NULL; listp = next){
    next = listp->next;
    free(listp);
  }
}
static unsigned int mem_hash(size_t p)
{
  return (p/8) % ks_mem_hash_size;
}
#endif
BOOL ks_check_memory(void)
{
#ifndef NO_CHECK_MEMORY
  atexit(output_mem_list);
  if((ks_mem_hash = (KS_MEM_LIST**)malloc(ks_mem_hash_size*sizeof(KS_MEM_LIST*))) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  {
    int i;
    for(i = 0; i < ks_mem_hash_size; i++)
      ks_mem_hash[i] = NULL;
  }
#endif
  return KS_TRUE;
}
#ifndef NO_CHECK_MEMORY
#define HASH_CHAR_MULTIPLITER 31
static unsigned int hash_char(char *str, size_t size)
{
  unsigned int h;
  unsigned char *p;
  h = 0;
  for(p = (unsigned char*)str; *p != '\0'; p++){
    h += HASH_CHAR_MULTIPLITER * h + *p;
  }
  return h % size;
}
static KS_MEM_LIST *lookup_mem_info(KS_MEM_LIST *listp, KS_MEM_LIST *p)
{
  for(; listp != NULL; listp = listp->next){
    if(strcmp(p->file,listp->file) == 0 && p->line == listp->line && 
       strcmp(p->func,listp->func) == 0){
      return listp;
    }
  }
  return NULL;
}
#endif
BOOL ks_combsort_int_buf(int *data, int num, int **sb)
{
  int i;
  int d;
  int flg = 1;
  int t;

  if(num <= 0){
    ks_error("number of sort array is %d",num);
    return KS_FALSE;
  }

  if(*sb == NULL){
    if((*sb = ks_malloc_int_p(num,"ks_combsort_int_buf")) == NULL){
      return KS_FALSE;
    }
  }

  for(i = 0; i < num; i++)
    (*sb)[i] = i;

  for(d = num - 1; d >= 1 || flg; d = (d*10 + 3)/ 13){
    flg = 0;
    for(i = 0; i < num-d; i++){
      if(data[(*sb)[i]] > data[(*sb)[i+d]]){
        t = (*sb)[i];
        (*sb)[i] = (*sb)[i+d];
        (*sb)[i+d] = t;
        flg = 1;
      }
    }
    if(flg == 0 && d == 1) d  = 0;
  }

  return KS_TRUE;
}
BOOL ks_combsort_double_buf(double *data, int num, int **sb)
{
  int i;
  int d;
  int flg = 1;
  int t;

  if(*sb == NULL){
    if((*sb = ks_malloc_int_p(num,"sb")) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
  }

  for(i = 0; i < num; i++)
    (*sb)[i] = i;

  for(d = num - 1; d >= 1 || flg; d = (d*10 + 3)/ 13){
    flg = 0;
    for(i = 0; i < num-d; i++){
      if(data[(*sb)[i]] > data[(*sb)[i+d]]){
        t = (*sb)[i];
        (*sb)[i] = (*sb)[i+d];
        (*sb)[i+d] = t;
        flg = 1;
      }
    }
    if(flg == 0 && d == 1) d  = 0;
  }

  return KS_TRUE;
}
BOOL ks_inssort_int_buf(int *data, int num, int **sb)
{
  int i,j;
  int x;

  if(num <= 0){
    ks_error("number of sort array is %d",num);
    return KS_FALSE;
  }

  if(*sb == NULL){
    if((*sb = ks_malloc_int_p(num,"ks_combsort_int_buf")) == NULL){
      return KS_FALSE;
    }
  }

  for(i = 0; i < num; i++)
    (*sb)[i] = i;

  for(i = 1; i < num; i++){
    x = (*sb)[i];
    for(j = i-1; j >= 0 && data[(*sb)[j]] > data[x]; j--){
      (*sb)[j+1] = (*sb)[j];
    }
    (*sb)[j+1] = x;
  }

  return KS_TRUE;
}
BOOL ks_inssort_inv_int_buf(int *data, int num, int **sb)
{
  int i,j;
  int x;

  if(num <= 0){
    ks_error("number of sort array is %d",num);
    return KS_FALSE;
  }

  if(*sb == NULL){
    if((*sb = ks_malloc_int_p(num,"ks_combsort_int_buf")) == NULL){
      return KS_FALSE;
    }
  }

  for(i = 0; i < num; i++)
    (*sb)[i] = i;

  for(i = 1; i < num; i++){
    x = (*sb)[i];
    for(j = i-1; j >= 0 && data[(*sb)[j]] < data[x]; j--){
      (*sb)[j+1] = (*sb)[j];
    }
    (*sb)[j+1] = x;
  }

  return KS_TRUE;
}
size_t ks_output_memory_infomation(FILE *fp, int output_lim)
{
  size_t total_size = 0;
#ifndef NO_CHECK_MEMORY
  int i;
  KS_MEM_LIST *p, *hit;
  KS_MEM_LIST **mem_info;
  unsigned int h;

  if((mem_info = (KS_MEM_LIST**)malloc(ks_mem_hash_size*sizeof(KS_MEM_LIST*))) == NULL){
    ks_error_memory();
    return KS_FALSE;
  }
  for(i = 0; i < ks_mem_hash_size; i++)
    mem_info[i] = NULL;

  for(i = 0; i < ks_mem_hash_size; i++){
    for(p = ks_mem_hash[i]; p != NULL; p = p->next){
      total_size += p->size;
      h = hash_char((char*)p->func,ks_mem_hash_size);
      hit = lookup_mem_info(mem_info[h],p);

      if(hit == NULL){
	/*
	printf("%d %p %d %s %d %s %s\n",i,p->ptr,p->size,p->file,p->line,p->func,p->comment);
	*/
	mem_info[h] = add_mem_list(mem_info[h],new_mem_list(p->ptr,p->size,p->file,
							    p->line,p->func,p->comment));
      } else {
	/*
	printf("%d %p %d %s %d %s %s %p\n",i,p->ptr,p->size,p->file,p->line,p->func,p->comment,
	       hit->ptr);
	printf("hit %s %d %s %d %p <- %s %d %s %d %p\n"
	       ,hit->file,hit->line,hit->func,hit->size,hit->ptr
	       ,p->file,p->line,p->func,p->size,p->ptr);
	*/
	hit->size += p->size;
      }
    }
  }

  if(output_lim >= 0 && fp != NULL){
    int num;
    int *sort_size,*sort_buf = NULL;
    KS_MEM_LIST **sort_mem;
    num = 0;
    for(i = 0; i < ks_mem_hash_size; i++){
      num += count_mem_list(mem_info[i]);
    }
    if((sort_size = (int*)malloc(num*sizeof(int))) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    if((sort_mem = (KS_MEM_LIST**)malloc(num*sizeof(KS_MEM_LIST*))) == NULL){
      ks_error_memory();
      return KS_FALSE;
    }
    num = 0;
    for(i = 0; i < ks_mem_hash_size; i++){
      for(p = mem_info[i]; p != NULL; p = p->next){
	/*
	printf("%d %p %d %s %d %s %s\n",i,p->ptr,p->size,p->file,p->line,p->func,p->comment);
	*/
	sort_size[num] = p->size;
	sort_mem[num] = p;
	num++;
      }
    }
    ks_combsort_int_buf(sort_size,num,&sort_buf);

    if(output_lim == 0 || output_lim > num)
      output_lim = num;
    for(i = 0; i < output_lim; i++){
      p = sort_mem[sort_buf[num-1-i]];
      /*
      printf("%d %p %d %s %d %s %s\n",i,p->ptr,p->size,p->file,p->line,p->func,p->comment);
      */
      if(p->comment != NULL){
#if __SIZEOF_SIZE_T__ == 4
	fprintf(fp,"MEMORY_INFOMATION: %u %s:%d:%s: %s\n"
		,p->size,p->file,p->line,p->func,p->comment);
#else
# ifdef _WIN32
	fprintf(fp,"MEMORY_INFOMATION: %I64u %s:%d:%s: %s\n"
		,p->size,p->file,p->line,p->func,p->comment);
# else
	fprintf(fp,"MEMORY_INFOMATION: %llu %s:%d:%s: %s\n"
		,p->size,p->file,p->line,p->func,p->comment);
# endif
#endif
      } else {
#if __SIZEOF_SIZE_T__ == 4
	fprintf(fp,"MEMORY_INFOMATION: %u %s:%d:%s:\n",p->size,p->file,p->line,p->func);
#else
# ifdef _WIN32
	fprintf(fp,"MEMORY_INFOMATION: %I64u %s:%d:%s:\n",p->size,p->file,p->line,p->func);
# else
	fprintf(fp,"MEMORY_INFOMATION: %llu %s:%d:%s:\n",p->size,p->file,p->line,p->func);
# endif
#endif
      }
    }
    free(sort_size);
    free(sort_mem);
    ks_free(sort_buf);
  }

  if(fp != NULL){
#if __SIZEOF_SIZE_T__ == 4
    fprintf(fp,"MEMORY_INFOMATION: total_size %u\n",total_size);
#else
# ifdef _WIN32
    fprintf(fp,"MEMORY_INFOMATION: total_size %I64u\n",total_size);
# else
    fprintf(fp,"MEMORY_INFOMATION: total_size %llu\n",total_size);
# endif
#endif
  }

  for(i = 0; i < ks_mem_hash_size; i++){
    free_mem_list(mem_info[i]);
  }
  free(mem_info);
#endif
  return total_size;
}
void *ks_cm_malloc(size_t size, KS_INDEX_PROTO,char *comment)
{
  void *ptr;
#ifndef NO_CHECK_MEMORY
  unsigned int h;
#endif
  ptr = malloc(size);
#ifndef NO_CHECK_MEMORY
  if(ptr != NULL && ks_mem_hash != NULL){
    h = mem_hash((size_t)ptr);
    ks_mem_hash[h]=add_mem_list(ks_mem_hash[h],
				new_mem_list(ptr,size,KS_INDEX_ARGS,comment));
    /*    printf("ks_malloc : %s %d %s\n",_file_,_line_,comment);*/
  }
#endif
  return ptr;
}
void *ks_cm_realloc(void *ptr, size_t size, KS_INDEX_PROTO, char *comment)
{
  void *re_ptr;
#ifndef NO_CHECK_MEMORY
  unsigned int h;
#endif
  re_ptr = realloc(ptr,size);
#ifndef NO_CHECK_MEMORY
  if(re_ptr != NULL && ks_mem_hash != NULL){
    h = mem_hash((size_t)ptr);
    ks_mem_hash[h] = del_mem_list(ks_mem_hash[h],ptr,KS_INDEX_ARGS);
    h = mem_hash((size_t)re_ptr);
    ks_mem_hash[h]=add_mem_list(ks_mem_hash[h],
				new_mem_list(re_ptr,size,KS_INDEX_ARGS,comment));
    /*    printf("ks_realloc : %s %d %s\n",_file_,_line_,comment);*/
  }
#endif
  return re_ptr;
}
void ks_cm_free(void *ptr, KS_INDEX_PROTO)
{
#ifndef NO_CHECK_MEMORY
  unsigned int h;
  if(ptr != NULL && ks_mem_hash != NULL){
    h = mem_hash((size_t)ptr);
    ks_mem_hash[h] = del_mem_list(ks_mem_hash[h],ptr,KS_INDEX_ARGS);
  }
#endif
  free(ptr);
}
/**************** above funcsions must be bottom of this file ********************/

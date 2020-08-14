
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef ICL
#include <mathimf.h>
#else
#include <math.h>
#endif
#include "ks_std.h"
#include "ks_math.h"

#ifdef MSVC
/*
double erfc(double x)
{
  if(x < 3)
  return( +9.9878326e-01
	  -1.0979871e+00*x
	  -1.7561843e-01*x*x
	  +7.8573930e-01*x*x*x
	  -4.5243056e-01*x*x*x*x
	  +1.0899079e-01*x*x*x*x*x
	  -9.8458465e-03*x*x*x*x*x*x);
  else 
    return 0.0;
}
*/
#endif
int ks_round(double d)
{
  if(d >= 0)
    return (int)(d+.5);
  else
    return (int)(d-.5);
}
double ks_integrate(int num, double h, double *f)
{
  int i;
  double wa;

  if(num <= 1){
    ks_error("integral num <= 0 (ks_integrate)");
    return 0.0;
  } else if(num < 4){ /* trapezoidal integration */
    wa = (f[0]+f[num-1])*.5;
    for(i = 1; i < num-1; i++)
      wa += f[i];
    return wa*h;
  } else if(num < 8){ /* 1/N^3 */
    wa  = (f[0]+f[num-1])*5/12;
    wa += (f[1]+f[num-2])*13/12;
    for(i = 2; i < num-2; i++)
      wa += f[i];
    return wa*h;
  } else {            /* alternative exteded simpson's rule */
    wa  = (f[0]+f[num-1])*17/48;
    wa += (f[1]+f[num-2])*59/48;
    wa += (f[2]+f[num-3])*43/48;
    wa += (f[3]+f[num-4])*49/48;
    for(i = 4; i < num-4; i++)
      wa += f[i];
    return wa*h;
  }
}
int ks_gcd(int x, int y)
{
  int t;
  while(y != 0){
    t = x % y;
    x = y;
    y = t;
  }
  return x;
}
int ks_ngcd(int n, int *a)
{
  int i,d;

  d = a[0];
  for(i = 1; i < n && d != 1; i++)
    d = ks_gcd(a[i],d);

  return d;
}
int ks_lcm(int m, int n)
{
  return abs(m*n)/ks_gcd(m,n);
}
int ks_nlcm(int n, int *a)
{
  int i,d;

  d = a[0];
  for(i = 1; i < n; i++)
    d = ks_lcm(a[i],d);

  return d;
}

double ks_inner_product(const int n, const double *u, const double *v)
{
  int i, n5;
  double s;

  s = 0;  n5 = n % 5;
  for (i = 0; i < n5; i++) s += u[i]*v[i];
  for (i = n5; i < n; i += 5)
    s += u[i]*v[i] + u[i+1]*v[i+1] + u[i+2]*v[i+2]
      + u[i+3]*v[i+3] + u[i+4]*v[i+4];
  return s;
}
void ks_exterior_product(const double *v0, const double *v1, double *v2)
{
  v2[0] = v0[1]*v1[2] - v0[2]*v1[1];
  v2[1] = v0[2]*v1[0] - v0[0]*v1[2];
  v2[2] = v0[0]*v1[1] - v0[1]*v1[0];
}

/* least squar fitting in C algorithm dictinaly */
#define calg_lsq_swap(a, i, j, t)  t = a[i];  a[i] = a[j];  a[j] = t
#define calg_lsq_EPS   1e-6    /* least error */
static int calg_lsq(const int n, const int m, double **x, double *b,
		    int *col, double *initnormsq, double *normsq)
{
  int i, j, r;
  double s, t, u;
  double *v, *w;

  for (j = 0; j < m; j++) {
    col[j] = j;
    normsq[j] = ks_inner_product(n, x[j], x[j]);
    initnormsq[j] = (normsq[j] != 0) ? normsq[j] : -1;
  }
  for (r = 0; r < m; r++) {
    if (r != 0) {
      j = r;  u = 0;
      for (i = r; i < m; i++) {
	t = normsq[i] / initnormsq[i];
	if (t > u) {  u = t;  j = i;  }
      }
      calg_lsq_swap(col, j, r, i);
      calg_lsq_swap(normsq, j, r, t);
      calg_lsq_swap(initnormsq, j, r, t);
      calg_lsq_swap(x, j, r, v);
    }
    v = x[r];  u = ks_inner_product(n - r, &v[r], &v[r]);
    if (u / initnormsq[r] < calg_lsq_EPS * calg_lsq_EPS) break;
    u = sqrt(u);  if (v[r] < 0) u = -u;
    v[r] += u;  t = 1 / (v[r] * u);
    for (j = r + 1; j <= m; j++) {
      w = x[j];
      s = t * ks_inner_product(n - r, &v[r], &w[r]);
      for (i = r; i < n; i++) w[i] -= s * v[i];
      if (j < m) normsq[j] -= w[r] * w[r];
    }
    v[r] = -u;
  }
  for (j = r - 1; j >= 0; j--) {
    s = x[m][j];
    for (i = j + 1; i < r; i++) s -= x[i][j] * b[i];
    b[j] = s / x[j][j];
  }
  return r;  /* rank */
}
static void invr(const int r, double **x)
{
  int i, j, k;
  double s;

  for (k = 0; k < r; k++) {
    x[k][k] = 1 / x[k][k];
    for (j = k - 1; j >= 0; j--) {
      s = 0;
      for (i = j + 1; i <= k; i++)
	s -= x[i][j] * x[i][k];
      x[j][k] = s * x[j][j];
    }
  }
}

BOOL ks_fit_polynomial(const double *x, const double *y, const int num, const int order,
		       const BOOL origin_flg, double *res, double *err)
{
  int i,j;
  int p,r;
  double s, t, rss;
  static BOOL origin_flg_prev;
  static int num_prev,order_prev;
  static double **mat = NULL;
  static int *col = NULL;
  static double *b,*initnormsq,*normsq;

  if(num <= 1){
    ks_error("ks_fit_polynomial: data size is too small\n");
    return KS_FALSE;
  }
  if(origin_flg == KS_TRUE){
    if(mat != NULL && (origin_flg_prev != origin_flg || num_prev != num || order_prev != order)){
      ks_free_double_pp(order_prev+2,mat);
      mat = NULL;
    }
    if(mat == NULL){
      if((mat = ks_malloc_double_pp(order+2,num,"ks_fit_ploynomial")) == NULL){
	return KS_FALSE;
      }
    }
    for(i = 0; i < num; i++){
      mat[0][i] = 1;
      for(j = 1; j < order+1; j++){
	mat[j][i] = mat[j-1][i]*x[i];
      }
      mat[order+1][i] = y[i];
    }
    p = order+1;
  } else {
    if(mat != NULL && (origin_flg_prev != origin_flg || num_prev != num || order_prev != order)){
      ks_free_double_pp(order_prev+1,mat);
      mat = NULL;
    }
    if(mat == NULL){
      if((mat = ks_malloc_double_pp(order+1,num,"ks_fit_ploynomial")) == NULL){
	return KS_FALSE;
      }
    }
    for(i = 0; i < num; i++){
      mat[0][i] = x[i];
      for(j = 1; j < order; j++)
	mat[j][i] = mat[j-1][i]*x[i];
      mat[order][i] = y[i];
    }
    p = order;
  }
  if(col != NULL && (origin_flg_prev != origin_flg || num_prev != num || order_prev != order)){
    ks_free(col);
    ks_free(b);
    ks_free(initnormsq);
    ks_free(normsq);
    col = NULL;
  }
  if(col == NULL){
    if((col = ks_malloc_int_p(num,"ks_fit_ploynomial")) == NULL){
      return KS_FALSE;
    }
    if((b = ks_malloc_double_p(num,"ks_fit_ploynomial")) == NULL){
      return KS_FALSE;
    }
    if((initnormsq = ks_malloc_double_p(num,"ks_fit_ploynomial")) == NULL){
      return KS_FALSE;
    }
    if((normsq = ks_malloc_double_p(num,"ks_fit_ploynomial")) == NULL){
      return KS_FALSE;
    }
  }
  /*
  for(i = 0; i < num; i++){
    printf("%d",i);
    for(j = 0; j < p+1; j++)
      printf(" %f",mat[j][i]);
    printf("\n");
  }
  */
  r = calg_lsq(num, p, mat, b, col, initnormsq, normsq);
  rss = ks_inner_product(num - r, &mat[p][r], &mat[p][r]);
  invr(r, mat);
  for (j = 0; j < r; j++) {
    t = ks_inner_product(r - j, &mat[j][j], &mat[j][j]);
    s = sqrt(t * rss / (num - r));
    res[col[j]] = b[j];
    if(err != NULL)
      err[col[j]] = s;
    /*
    printf("%4d  % #-14g % #-14g", col[j] + 1, b[j], s);
    if (s > 0) printf("  % #-11.3g", fabs(b[j] / s));
    printf("\n");
    */
  }
  /*
  printf("%g / %d = %g\n", rss, num - r, rss / (num - r));
  */
  /*
  printf("r= %d\n",r);
  for (j = 0; j < r; j++) 
    printf("%d %d %f\n",j,col[j],b[j]);
  */
  origin_flg_prev = origin_flg;
  num_prev = num;
  order_prev = order;

  return KS_TRUE;
}
double ks_polynomial(const double *c, const double x, const int order)
{
  int i;
  double v;

  v = c[order];
  for(i = order-1; i >= 0; i--)
    v = v*x+c[i];
  return v;
}
void ks_get_rotate_matrix(const double ang, const double ax, const double ay, const double az, 
			  double m[3][3])
{
  int i,j;
  double d0;
  double s[3][3];
  double u[3];

  d0 = sqrt(pow(ax,2)+pow(ay,2)+pow(az,2));
  u[0] = ax/d0;
  u[1] = ay/d0;
  u[2] = az/d0;

  s[0][0] = 0;    s[0][1] =-u[2]; s[0][2] = u[1];
  s[1][0] = u[2]; s[1][1] = 0;    s[1][2] =-u[0];
  s[2][0] =-u[1]; s[2][1] = u[0]; s[2][2] = 0;

  for(i = 0; i < 3; i++)
    for(j = 0; j < 3; j++)
      m[i][j] = u[i]*u[j];

  d0 = cos(ang);
  for(i = 0; i < 3; i++)
    for(j = 0; j < 3; j++)
      m[i][j] += d0*((i==j ? 1:0)-u[i]*u[j]);

  d0 = sin(ang);
  for(i = 0; i < 3; i++)
    for(j = 0; j < 3; j++)
      m[i][j] += d0*s[i][j];
}
void ks_multi_matrix_33x3(double m[3][3], double v[3])
{
  int i,j;
  double temp[3];

  for(i = 0; i < 3; i++){
    temp[i] = 0;
    for(j = 0; j < 3; j++){
      temp[i] += m[i][j]*v[j];
    }
  }
  for(i = 0; i < 3; i++)
    v[i] = temp[i];
}
void ks_multi_matrix_44x4(double m[4][4], double v[4])
{
  int i,j;
  double temp[4];

  for(i = 0; i < 4; i++){
    temp[i] = 0;
    for(j = 0; j < 4; j++){
      temp[i] += m[i][j]*v[j];
    }
  }
  for(i = 0; i < 4; i++)
    v[i] = temp[i];
}
void ks_multi_matrix_33x33(double m0[3][3], double m1[3][3], double m2[3][3])
{
  int i,j,k;

  for(i = 0; i < 3; i++){
    for(j = 0; j < 3; j++){
      m2[i][j] = 0;
      for(k = 0; k < 3; k++){
	m2[i][j] += m0[i][k]*m1[k][j];
      }
    }
  }
}
void ks_multi_matrix_44x44(double m0[4][4], double m1[4][4], double m2[4][4])
{
  int i,j,k;

  for(i = 0; i < 4; i++){
    for(j = 0; j < 4; j++){
      m2[i][j] = 0;
      for(k = 0; k < 4; k++){
	m2[i][j] += m0[i][k]*m1[k][j];
      }
    }
  }
}
void ks_multi_matrix_44x44f(float m0[4][4], float m1[4][4], float m2[4][4])
{
  int i,j,k;

  for(i = 0; i < 4; i++){
    for(j = 0; j < 4; j++){
      m2[i][j] = 0;
      for(k = 0; k < 4; k++){
	m2[i][j] += m0[i][k]*m1[k][j];
      }
    }
  }
}
double ks_calc_vector_length3(double v[3])
{
  int i;
  double len = 0;
  for(i = 0; i < 3; i++){
    len += v[i]*v[i];
  }
  return sqrt(len);
}
double ks_calc_vector_length(const double *v, const int n)
{
  int i;
  double len = 0;
  for(i = 0; i < n; i++)
    len += v[i]*v[i];
  return sqrt(len);
}
BOOL ks_normalize_vector(double *v, const int n)
{
  int i;
  double len;
  len = ks_calc_vector_length(v,n);
  if(len == 0){
    return KS_FALSE;
  }
  for(i = 0; i < n; i++)
    v[i] /= len;
  return KS_TRUE;
}
void ks_print_mat_33(double m[3][3])
{
  int i,j;
  for(i = 0; i < 3; i++){
    for(j = 0; j < 3; j++){
      printf("% f ",m[i][j]);
    }
    printf("\n");
  }
}
BOOL ks_calc_inverse_matrix(int n, double **a)
{
  int i,j,k;
  double t, u, det;
  det = 1;
  for(k = 0; k < n; k++){
    t = a[k][k]; det *= t;
    if(t == 0){
      ks_error("can not calculate inverse matrix");
      return KS_FALSE;
    }
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
  return KS_TRUE;
}
BOOL ks_calc_inverse_matrixf(int n, float **a)
{
  int i,j,k;
  float t, u, det;
  det = 1;
  for(k = 0; k < n; k++){
    t = a[k][k]; det *= t;
    if(t == 0){
      ks_error("can not calculate inverse matrix");
      return KS_FALSE;
    }
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
  return KS_TRUE;
}
BOOL ks_calc_inverse_matrix_33(double a[3][3])
{
  double m[3][3];
  double det,det_inv;
  det  = a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1]);
  det += a[0][1]*(a[1][2]*a[2][0]-a[1][0]*a[2][2]);
  det += a[0][2]*(a[1][0]*a[2][1]-a[1][1]*a[2][0]);
  if(det == 0){
    ks_error("can not calculate inverse matrix");
    return KS_FALSE;
  }
  det_inv = 1.0/det;
  m[0][0] = a[0][0];
  m[0][1] = a[0][1];
  m[0][2] = a[0][2];
  m[1][0] = a[1][0];
  m[1][1] = a[1][1];
  m[1][2] = a[1][2];
  m[2][0] = a[2][0];
  m[2][1] = a[2][1];
  m[2][2] = a[2][2];

  a[0][0] = (m[1][1]*m[2][2]-m[1][2]*m[2][1])*det_inv;
  a[0][1] = (m[0][2]*m[2][1]-m[0][1]*m[2][2])*det_inv;
  a[0][2] = (m[0][1]*m[1][2]-m[0][2]*m[1][1])*det_inv;
  a[1][0] = (m[1][2]*m[2][0]-m[1][0]*m[2][2])*det_inv;
  a[1][1] = (m[0][0]*m[2][2]-m[0][2]*m[2][0])*det_inv;
  a[1][2] = (m[0][2]*m[1][0]-m[0][0]*m[1][2])*det_inv;
  a[2][0] = (m[1][0]*m[2][1]-m[1][1]*m[2][0])*det_inv;
  a[2][1] = (m[0][1]*m[2][0]-m[0][0]*m[2][1])*det_inv;
  a[2][2] = (m[0][0]*m[1][1]-m[0][1]*m[1][0])*det_inv;
  return KS_TRUE;
}
BOOL ks_calc_inverse_matrix_33f(float a[3][3])
{
  float m[3][3];
  float det,det_inv;
  det  = a[0][0]*(a[1][1]*a[2][2]-a[1][2]*a[2][1]);
  det += a[0][1]*(a[1][2]*a[2][0]-a[1][0]*a[2][2]);
  det += a[0][2]*(a[1][0]*a[2][1]-a[1][1]*a[2][0]);
  if(det == 0){
    ks_error("can not calculate inverse matrix");
    return KS_FALSE;
  }
  det_inv = 1.0/det;
  m[0][0] = a[0][0];
  m[0][1] = a[0][1];
  m[0][2] = a[0][2];
  m[1][0] = a[1][0];
  m[1][1] = a[1][1];
  m[1][2] = a[1][2];
  m[2][0] = a[2][0];
  m[2][1] = a[2][1];
  m[2][2] = a[2][2];

  a[0][0] = (m[1][1]*m[2][2]-m[1][2]*m[2][1])*det_inv;
  a[0][1] = (m[0][2]*m[2][1]-m[0][1]*m[2][2])*det_inv;
  a[0][2] = (m[0][1]*m[1][2]-m[0][2]*m[1][1])*det_inv;
  a[1][0] = (m[1][2]*m[2][0]-m[1][0]*m[2][2])*det_inv;
  a[1][1] = (m[0][0]*m[2][2]-m[0][2]*m[2][0])*det_inv;
  a[1][2] = (m[0][2]*m[1][0]-m[0][0]*m[1][2])*det_inv;
  a[2][0] = (m[1][0]*m[2][1]-m[1][1]*m[2][0])*det_inv;
  a[2][1] = (m[0][1]*m[2][0]-m[0][0]*m[2][1])*det_inv;
  a[2][2] = (m[0][0]*m[1][1]-m[0][1]*m[1][0])*det_inv;
  return KS_TRUE;
}
void ks_calc_transpose_matrix_33(double a[3][3])
{
  int i,j;
  double m[3][3];
  int n = 3;
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      m[i][j] = a[i][j];
    }
  }
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      a[i][j] = m[j][i];
    }
  }
}
void ks_calc_transpose_matrix_33f(float a[3][3])
{
  int i,j;
  float m[3][3];
  int n = 3;
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      m[i][j] = a[i][j];
    }
  }
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      a[i][j] = m[j][i];
    }
  }
}
void ks_get_3x3_submatrix_4x4(double m[4][4], double s[3][3])
{
  int i,j;
  int n = 3;
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      s[i][j] = m[i][j];
    }
  }
}
void ks_get_3x3_submatrix_4x4f(float m[4][4], float s[3][3])
{
  int i,j;
  int n = 3;
  for(i = 0; i < n; i++){
    for(j = 0; j < n; j++){
      s[i][j] = m[i][j];
    }
  }
}
double ks_calc_vectors_angle(double v0[3], double v1[3])
{
  double ip;
  if(ks_normalize_vector(v0,3) == KS_FALSE){
    return 0;
  }
  if(ks_normalize_vector(v1,3) == KS_FALSE){
    return 0;
  }
  ip = v0[0]*v1[0]+v0[1]*v1[1]+v0[2]*v1[2];
  if(ip > 1.0) ip = 1.0;
  if(ip <-1.0) ip =-1.0;
  return acos(ip);
}
BOOL ks_factorize(int x, int **factor, int *factor_num)
{
  int i;
  int d, q;
  KS_INT_LIST *il = NULL, *p;

  /*  printf("%5d = ", x);*/
  while (x >= 4 && x % 2 == 0) {
    /*    printf("2 * ");*/
    x /= 2;
    il = ks_addend_int_list(il,ks_new_int_list(2));
  }
  d = 3;  q = x / d;
  while (q >= d) {
    if (x % d == 0) {
      /*      printf("%d * ", d);*/
      x = q;
      il = ks_addend_int_list(il,ks_new_int_list(d));
    } else d += 2;
    q = x / d;
  }
  /*  printf("%d\n", x);*/
  il = ks_addend_int_list(il,ks_new_int_list(x));

  *factor_num = ks_count_int_list(il);
  if(((*factor) = ks_malloc_int_p(*factor_num,"ks_factorize")) == NULL){
    return KS_FALSE;
  }

  for(i = 0, p = il; p != NULL; i++, p = p->next){
    (*factor)[i] = p->value;
  }

  /*
  {
    int j,i0;
    int *fac,fac_num;

    for(i = 0; i < 1000; i++){
      i0 = i;
      ks_factorize(i0,&fac,&fac_num);
      printf("%d = ",i0);
      for(j = 0; j < fac_num-1; j++)
	printf("%d x ",fac[j]);
      printf("%d\n",fac[fac_num-1]);
    }
    ks_exit(0);
  }
  */	

  ks_free_int_list(il);

  return KS_TRUE;
}
double ks_calc_positions_angle(double cd0[3], double cd1[3], double cd2[3])
{
  int i;
  double vec[2][3];

  for(i = 0; i < 3; i++) vec[0][i] = cd0[i] - cd1[i];
  for(i = 0; i < 3; i++) vec[1][i] = cd2[i] - cd1[i];
  if(ks_normalize_vector(vec[0],3) == KS_FALSE){
    return 0;
  }
  if(ks_normalize_vector(vec[1],3) == KS_FALSE){
    return 0;
  }
  return acos(vec[0][0]*vec[1][0] + vec[0][1]*vec[1][1] + vec[0][2]*vec[1][2]);
}
static void make_spline_table(KS_SPLINE_BUFFER *sp)
{
  int i;
  double t;

  sp->z[0] = 0;  sp->z[sp->num - 1] = 0;
  for(i = 0; i < sp->num-1; i++){
    sp->h[i]   =  sp->x[i+1] - sp->x[i];
    sp->d[i+1] = (sp->y[i+1] - sp->y[i]) / sp->h[i];
  }
  sp->z[1] = sp->d[2] - sp->d[1] - sp->h[0]*sp->z[0];
  sp->d[1] = 2.0*(sp->x[2] - sp->x[0]);
  for(i = 1; i < sp->num - 2; i++){
    t = sp->h[i] / sp->d[i];
    sp->z[i+1] = sp->d[i+2] - sp->d[i+1] - sp->z[i]*t;
    sp->d[i+1] = 2.0*(sp->x[i+2] - sp->x[i]) - sp->h[i]*t;
  }
  sp->z[sp->num - 2] -= sp->h[sp->num-2]*sp->z[sp->num-1];
  for(i = sp->num - 2; i > 0; i--)
    sp->z[i] = (sp->z[i] - sp->h[i] * sp->z[i+1]) / sp->d[i];
}
KS_SPLINE_BUFFER *ks_allocate_spline_buffer(int num, double *x, double *y)
{
  int i;
  KS_SPLINE_BUFFER *sp;

  if((sp = (KS_SPLINE_BUFFER*)ks_malloc(sizeof(KS_SPLINE_BUFFER),"ks_allocae_spline_buffer"))
     ==NULL){
    ks_error("ks_allocate_spline_buffer: memory error\n");
    return NULL;
  }
  sp->num = num;
  if((sp->x = ks_malloc_double_p(sp->num,"ks_allocae_spline_buffer")) == NULL)
    return NULL;
  if((sp->y = ks_malloc_double_p(sp->num,"ks_allocae_spline_buffer")) == NULL)
    return NULL;
  if((sp->z = ks_malloc_double_p(sp->num,"ks_allocae_spline_buffer")) == NULL)
    return NULL;
  if((sp->d = ks_malloc_double_p(sp->num,"ks_allocae_spline_buffer")) == NULL)
    return NULL;
  if((sp->h = ks_malloc_double_p(sp->num,"ks_allocae_spline_buffer")) == NULL)
    return NULL;

  for(i = 0; i < sp->num; i++) sp->x[i] = x[i];
  for(i = 0; i < sp->num; i++) sp->y[i] = y[i];

  make_spline_table(sp);

  return sp;
}
void ks_free_spline_buffer(KS_SPLINE_BUFFER *sp)
{
  ks_free(sp->x);
  ks_free(sp->y);
  ks_free(sp->z);
  ks_free(sp->d);
  ks_free(sp->h);
  ks_free(sp);
}
double ks_calc_spline(KS_SPLINE_BUFFER *sp, double t)
{
  int i, j, k;
  double d, h;

  i = 0;  j = sp->num - 1;
  while (i < j) {
    k = (i + j) / 2;
    if (sp->x[k] < t) i = k + 1;  else j = k;
  }
  if (i > 0) i--;
  h = sp->x[i+1] - sp->x[i];
  d = t - sp->x[i];
  return (((sp->z[i+1] - sp->z[i]) * d / h + sp->z[i] * 3) * d
	  + ((sp->y[i + 1] - sp->y[i]) / h
	     - (sp->z[i] * 2 + sp->z[i+1]) * h)) * d + sp->y[i];
}
double ks_differentiate_array(int i, double h, int num, double *f)
{
  double df;
  /* 5 and 3 pointes approximation */
  
  if(i > 1 && i < num-2){
    df = (f[i-2]-8.0*f[i-1]+8.0*f[i+1]-f[i+2])/(h*12.0);
    /*    df = (f[i+1]-f[i-1])/h*0.5;*/
  } else if(i == 1 || i == num-2){
    df = (f[i+1]-f[i-1])/h*0.5;
  } else if(i == 0){
    df = (f[1]-f[0])/h;
  } else if(i == num-1){
    df = (f[num-1]-f[num-2])/h;
  } else {
    df = sqrt(-1);
  }
  return df;
}
static double Bezire_factor(int n, int m)
{
  int i;
  double nf = 1;
  double mf = 1;
  double nmf =1;

  for(i = 0; i < n; i++){
    nf *= i+1;
  }
  for(i = 0; i < m; i++){
    mf *= i+1;
  }
  for(i = 0; i < n-m; i++){
    nmf *= i+1;
  }
  return nf/(mf*nmf);
}
void ks_calc_Bezier(double *x, double *y, double *z, int n, double t, double *pos)
{
  int i;

  for(i = 0; i < 3; i++){
    pos[i] = 0;
  }
  for(i = 0; i < n; i++){
    pos[0] += Bezire_factor(n-1,i)*pow(t,i)*pow(1-t,n-1-i)*x[i];
    pos[1] += Bezire_factor(n-1,i)*pow(t,i)*pow(1-t,n-1-i)*y[i];
    pos[2] += Bezire_factor(n-1,i)*pow(t,i)*pow(1-t,n-1-i)*z[i];
    /*    printf("%d %f %f %f %f %f\n",i,t,Bezire_factor(n-1,i),pos[0],pos[1],pos[2]);*/
  }
}
static double __KS_USED__ lu(int n, double **a, int *ip)
{
  int i, j, k, ii, ik;
  double t, u, det;
  double *weight;

  if((weight = ks_malloc_double_p(n,"weight")) == NULL){
    ks_error_memory();
    return 0.0;
  }

  det = 0;
  for (k = 0; k < n; k++) {
    ip[k] = k;
    u = 0;
    for (j = 0; j < n; j++) {
      t = fabs(a[k][j]);  if (t > u) u = t;
    }
    if (u == 0) goto EXIT;
    weight[k] = 1 / u;
  }
  det = 1;
  for (k = 0; k < n; k++) {
    u = -1;
    for (i = k; i < n; i++) {
      ii = ip[i];
      t = fabs(a[ii][k]) * weight[ii];
      if (t > u) {  u = t;  j = i;  }
    }
    ik = ip[j];
    if (j != k) {
      ip[j] = ip[k];  ip[k] = ik;
      det = -det;
    }
    u = a[ik][k];  det *= u;
    if (u == 0) goto EXIT;
    for (i = k + 1; i < n; i++) {
      ii = ip[i];
      t = (a[ii][k] /= u);
      for (j = k + 1; j < n; j++)
	a[ii][j] -= t * a[ik][j];
    }
  }
 EXIT:
  ks_free(weight);
  return det;
}
static double innerproduct(int n, double *u, double *v)
{
  int i, n5;
  double s;

  s = 0;  n5 = n % 5;
  for (i = 0; i < n5; i++) s += u[i]*v[i];
  for (i = n5; i < n; i += 5)
    s += u[i]*v[i] + u[i+1]*v[i+1] + u[i+2]*v[i+2]
      + u[i+3]*v[i+3] + u[i+4]*v[i+4];
  return s;
}
static double house(int n, double *x)  /* Householder transform */
{
  int i;
  double s, t;

  s = sqrt(innerproduct(n, x, x));
  if (s != 0) {
    if (x[0] < 0) s = -s;
    x[0] += s;  t = 1 / sqrt(x[0] * s);
    for (i = 0; i < n; i++) x[i] *= t;
  }
  return -s;
}
static void tridiagonalize(int n, double **a, double *d, double *e)
{
  int i, j, k;
  double s, t, p, q;
  double *v, *w;

  for (k = 0; k < n - 2; k++) {
    v = a[k];  d[k] = v[k];
    e[k] = house(n - k - 1, &v[k + 1]);
    if (e[k] == 0) continue;
    for (i = k + 1; i < n; i++) {
      s = 0;
      for (j = k + 1; j < i; j++) s += a[j][i] * v[j];
      for (j = i;     j < n; j++) s += a[i][j] * v[j];
      d[i] = s;
    }
    t = innerproduct(n-k-1, &v[k+1], &d[k+1]) / 2;
    for (i = n - 1; i > k; i--) {
      p = v[i];  q = d[i] - t * p;  d[i] = q;
      for (j = i; j < n; j++)
	a[i][j] -= p * d[j] + q * v[j];
    }
  }
  if (n >= 2) {  d[n - 2] = a[n - 2][n - 2];
  e[n - 2] = a[n - 2][n - 1];  }
  if (n >= 1)    d[n - 1] = a[n - 1][n - 1];
  for (k = n - 1; k >= 0; k--) {
    v = a[k];
    if (k < n - 2) {
      for (i = k + 1; i < n; i++) {
	w = a[i];
	t = innerproduct(n-k-1, &v[k+1], &w[k+1]);
	for (j = k + 1; j < n; j++)
	  w[j] -= t * v[j];
      }
    }
    for (i = 0; i < n; i++) v[i] = 0;
    v[k] = 1;
  }
}
BOOL ks_diagonalize(int n, double **a, double *d, double *e)
{
  int i, j, k, h, iter;
  double c, s, t, w, x, y;
  double *v;

  tridiagonalize(n, a, d, &e[1]);
  e[0] = 0;
  for (h = n - 1; h > 0; h--) {
    j = h;
    while (fabs(e[j]) > KS_DIAGONALIZE_EPS * (fabs(d[j - 1]) + fabs(d[j])))
      j--;
    if (j == h) continue;
    iter = 0;
    do {
      if (++iter > KS_DIAGONALIZD_MAX_ITER){
	ks_error("not converge");
	return KS_FALSE;
      }
      w = (d[h - 1] - d[h]) / 2;
      t = e[h] * e[h];
      s = sqrt(w * w + t);  if (w < 0) s = -s;
      x = d[j] - d[h] + t / (w + s);  y = e[j + 1];
      for (k = j; k < h; k++) {
	if (fabs(x) >= fabs(y)) {
	  t = -y / x;  c = 1 / sqrt(t * t + 1);
	  s = t * c;
	} else {
	  t = -x / y;  s = 1 / sqrt(t * t + 1);
	  c = t * s;
	}
	w = d[k] - d[k + 1];
	t = (w * s + 2 * c * e[k + 1]) * s;
	d[k] -= t;  d[k + 1] += t;
	if (k > j) e[k] = c * e[k] - s * y;
	e[k + 1] += s * (c * w - 2 * s * e[k + 1]);

	for (i = 0; i < n; i++) {
	  x = a[k][i];  y = a[k + 1][i];
	  a[k    ][i] = c * x - s * y;
	  a[k + 1][i] = s * x + c * y;
	}
	if (k < h - 1) {
	  x = e[k + 1];  y = -s * e[k + 2];
	  e[k + 2] *= c;
	}
      }
    } while (fabs(e[h]) >
	     KS_DIAGONALIZE_EPS * (fabs(d[h - 1]) + fabs(d[h])));
  }
  for (k = 0; k < n - 1; k++) {
    h = k;  t = d[h];
    for (i = k + 1; i < n; i++)
      if (d[i] > t) {  h = i;  t = d[h];  }
    d[h] = d[k];  d[k] = t;
    v = a[h];  a[h] = a[k];  a[k] = v;
  }
  return KS_TRUE;
}
double ks_calc_triangle_area(double *p0, double *p1, double *p2)
{
  int i;
  double vec[3][3];

  for(i = 0; i < 3; i++){
    vec[0][i] = p1[i] - p0[i];
    vec[1][i] = p2[i] - p0[i];
  }
  ks_exterior_product(vec[0],vec[1],vec[2]);
  return sqrt(vec[2][0]*vec[2][0]+vec[2][1]*vec[2][1]+vec[2][2]*vec[2][2])*.5;
}
BOOL ks_calc_segment_area(double radius, double len, double *area)
{
  double x,y;
  double angle;
  if(len > radius){
    *area = 0;
    ks_error("len %f is larger than radus %f",len,radius);
    return KS_FALSE;
  }
  x = fabs(len);
  y = sqrt(radius*radius - x*x);
  angle = acos(x/radius);
  /*
  printf("calc seg %f %f %f %f %f %f\n",x,y,angle,radius,x/radius,acos(x/radius)/(M_PI)*180);
  */
  *area = (radius*radius*angle - x*y);

  return KS_TRUE;
}
int ks_ipow(int base, int pow)
{
  int i;
  int v = 1;
  for(i = 0; i < pow; i++){
    v *= base;
  }
  return v;
}

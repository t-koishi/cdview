#ifndef KS_MATH_H
#define KS_MATH_H

#ifdef ICL
#include <mathimf.h>
#else
#include <math.h>
#endif

#define KS_PI_PI  9.86960440108935861883
#define KS_TWO_PI 6.28318530717958647696

#define KS_PI_POW12 924269.1815233738
#define KS_PI_POW10  93648.0474760830
#define KS_PI_POW6     961.3891935753
#define KS_PI_POW4      97.4090910340

#define ks_get_sign(x) ((x) < 0 ? -1.:1.)
#define ks_swap_int(x,y) {int _ks_swap_int; _ks_swap_int = (x); (x) = (y); (y) = _ks_swap_int;}

#define KS_DIAGONALIZE_EPS         1E-6
#define KS_DIAGONALIZD_MAX_ITER    100

typedef struct KS_SPLINE_BUFFER KS_SPLINE_BUFFER;
struct KS_SPLINE_BUFFER{
  double *x, *y, *z;
  double *h, *d;
  int num;
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MSVC
  //  double erfc(double x);
#endif

  int ks_round(double d);
  double ks_integrate(int num, double h, double *f);
  int ks_gcd(int x, int y);
  int ks_ngcd(int n, int *a);
  int ks_lcm(int m, int n);
  int ks_nlcm(int n, int *a);
  double ks_inner_product(const int n, const double *u, const double *v);
  void ks_exterior_product(const double *v0, const double *v1, double *v2);
  BOOL ks_fit_polynomial(const double *x, const double *y, const int num, const int order,
			 const BOOL origin_flg, double *res, double *err);
  double ks_polynomial(const double *c, const double x, const int order);
  void ks_get_rotate_matrix(const double ang, const double ax, const double ay, const double az, 
			    double m[3][3]);
  void ks_multi_mat_33x3(double m[3][3], double v[3]);
  double ks_calc_vector_length(const double *v, const int n);
  BOOL ks_normalize_vector(double *v, const int n);
  void ks_print_matrix_33(double m[3][3]);
  BOOL ks_calc_inverse_matrix(int n, double **a);
  BOOL ks_calc_inverse_matrixf(int n, float **a);
  BOOL ks_calc_inverse_matrix_33(double a[3][3]);
  BOOL ks_calc_inverse_matrix_33f(float a[3][3]);
  void ks_multi_matrix_33x3(double m[3][3], double v[3]);
  void ks_multi_matrix_44x4(double m[4][4], double v[4]);
  double ks_calc_vectors_angle(double v0[3], double v1[3]);
  BOOL ks_factorize(int x, int **factor, int *factor_num);
  double ks_calc_positions_angle(double cd0[3], double cd1[3], double cd2[3]);
  KS_SPLINE_BUFFER *ks_allocate_spline_buffer(int num, double *x, double *y);
  void ks_free_spline_buffer(KS_SPLINE_BUFFER *sp);
  double ks_calc_spline(KS_SPLINE_BUFFER *sp, double t);
  double ks_differentiate_array(int i, double h, int num, double *f);
  void ks_calc_Bezier(double *x, double *y, double *z, int n, double t, double *pos);
  void ks_multi_matrix_33x33(double m0[3][3], double m1[3][3], double m2[3][3]);
  void ks_multi_matrix_44x44(double m0[4][4], double m1[4][4], double m2[4][4]);
  void ks_multi_matrix_44x44f(float m0[4][4], float m1[4][4], float m2[4][4]);
  BOOL ks_diagonalize(int n, double **a, double *d, double *e);
  double ks_calc_triangle_area(double *p0, double *p1, double *p2);
  BOOL ks_calc_segment_area(double radius, double len, double *area);
  int ks_ipow(int base, int pow);
  double ks_calc_vector_length3(double v[3]);
  void ks_calc_transpose_matrix_33(double a[3][3]);
  void ks_calc_transpose_matrix_33f(float a[3][3]);
  void ks_get_3x3_submatrix_4x4(double m[4][4], double s[3][3]);
  void ks_get_3x3_submatrix_4x4f(float m[4][4], float s[3][3]);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif

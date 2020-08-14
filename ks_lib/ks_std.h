#ifndef KS_STD_H
#define KS_STD_H

#include <stdio.h>
#include <assert.h>

#if defined(MSVC) || defined(ICL)
#include <windows.h>
#elif defined(WIN32)
//#include <sys/dirent.h>
#include <dirent.h>
#else
#include <dirent.h>
#include <unistd.h>
#endif

#ifdef SOCK
/* ks_sock */
#if !defined(_WIN32) || defined(__CYGWIN__)
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#else
#include <winsock.h>
#endif
/* ks_sock end */
#endif

#define ks_assert(flg) assert(flg)
#define ks_abort() abort()

#ifndef DEBUG
#ifndef NDEBUG
#define DEBUG
#endif
#endif

#ifdef MSVC
#ifndef BOOL
#define BOOL int
#endif
#define KS_TRUE 1
#define KS_FALSE 0
#else
#ifndef __cplusplus
typedef enum {
  false = 0,
  true  = 1
} bool;
#endif
#define BOOL bool
#define KS_FALSE false
#define KS_TRUE true
#endif

enum {
  KS_RANGE_MIN,
  KS_RANGE_MAX,
  KS_RANGE
};

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif
#ifndef M_2_SQRTPI
#define M_2_SQRTPI 1.128379167
#endif
#ifndef M_1_PI
#define M_1_PI 0.318309886
#endif

#define ks_max(x,y) ((x) > (y) ? (x):(y))
#define ks_min(x,y) ((x) < (y) ? (x):(y))

#if defined(__GNUC__)
    #define __KS_USED__  __attribute__((__used__))
#else
    #define __KS_USED__
#endif

#ifdef MSVC
#define KS_INDEX_PROTO char const *_file_, int _line_
#define KS_INDEX_ARGS _file_,_line_
#define KS_INDEX_MACRO __FILE__,__LINE__
#define chdir _chdir
#define mkdir(x,y) _mkdir(x)
#define snprintf _snprintf
#else
#define KS_INDEX_PROTO char const *_file_, int _line_, char const* _func_
#define KS_INDEX_ARGS _file_,_line_,_func_
#define KS_INDEX_MACRO __FILE__,__LINE__,__func__
#endif

#if defined(MSVC)
/* for MSVC */
#elif defined(__GNUC__)
#define ks_error(fmt,ARGS...) ks_index_error(KS_INDEX_MACRO,fmt,##ARGS)
#else
#define ks_error(fmt,...) ks_index_error(KS_INDEX_MACRO,fmt,__VA_ARGS__)
#endif

#define ks_error_memory() ks_error("memory error")
#define ks_error_file(name) ks_error("file open error %s",name)

#if !defined(DEBUG) || defined(MSVC)
#define NO_CHECK_MEMORY
#endif

#ifndef NO_CHECK_MEMORY
#define ks_malloc(size,comment) ks_cm_malloc((size),KS_INDEX_MACRO,(comment))
#define ks_realloc(ptr,size,comment) ks_cm_realloc((ptr),(size),KS_INDEX_MACRO,(comment))
#define ks_free(ptr) ks_cm_free((ptr),KS_INDEX_MACRO)
#define malloc orignal_malloc
#define realloc orignal_realloc
#define free orignal_free
#define exit orignal_exit
#define GLUT_DISABLE_ATEXIT_HACK
#else
#define ks_malloc(size,comment) malloc((size))
#define ks_realloc(ptr,size,comment) realloc((ptr),(size))
#define ks_free(ptr) free((ptr))
#endif

#ifdef USE_PRINTF_FFLUSH_STDOUT_ALWAYS
#define printf(fmt,...) ks_printf_flush_stdout(fmt,##__VA_ARGS__)
#endif

#define ks_malloc_char_copy(size,msg) ks_cm_malloc_char_copy((size),KS_INDEX_MACRO,(msg))
#define ks_malloc_char_p(n0,msg) ks_cm_malloc_char_p((n0),KS_INDEX_MACRO,(msg))
#define ks_malloc_char_pp(n0,n1,msg) ks_cm_malloc_char_pp((n0),(n1),KS_INDEX_MACRO,(msg))
#define ks_realloc_char_p(op,n,msg) ks_cm_realloc_char_p((op),(n),KS_INDEX_MACRO,(msg))
#define ks_malloc_int_p(n,msg) ks_cm_malloc_int_p((n), KS_INDEX_MACRO,(msg))
#define ks_malloc_int_pp(n0,n1,msg) ks_cm_malloc_int_pp((n0),(n1), KS_INDEX_MACRO, (msg))
#define ks_malloc_int_ppp(n0,n1,n2,msg) \
        ks_cm_malloc_int_ppp((n0),(n1),(n2),KS_INDEX_MACRO,(msg))
#define ks_malloc_uint_p(n,msg) ks_cm_malloc_uint_p((n),KS_INDEX_MACRO,(msg))
#define ks_malloc_uint_pp(n0,n1,msg) ks_cm_malloc_uint_pp((n0),(n1), KS_INDEX_MACRO, (msg))
#define ks_malloc_float_p(n,msg) ks_cm_malloc_float_p((n), KS_INDEX_MACRO,(msg))
#define ks_malloc_float_pp(n0,n1,msg) ks_cm_malloc_float_pp((n0),(n1), KS_INDEX_MACRO, (msg))
#define ks_malloc_double_p(n,msg) ks_cm_malloc_double_p((n), KS_INDEX_MACRO,(msg))
#define ks_malloc_double_pp(n0,n1,msg) ks_cm_malloc_double_pp((n0),(n1), KS_INDEX_MACRO, (msg))
#define ks_malloc_double_ppp(n0,n1,n2,msg) \
        ks_cm_malloc_double_ppp((n0),(n1),(n2),KS_INDEX_MACRO,(msg))
#define ks_realloc_int_p(op,n,msg) ks_cm_realloc_int_p((op),(n),KS_INDEX_MACRO,(msg))
#define ks_realloc_uint_p(op,n,msg) ks_cm_realloc_uint_p((op),(n),KS_INDEX_MACRO,(msg))
#define ks_realloc_float_p(op,n,msg) ks_cm_realloc_float_p((op),(n),KS_INDEX_MACRO,(msg))
#define ks_realloc_double_p(op,n,msg) ks_cm_realloc_double_p((op),(n),KS_INDEX_MACRO,(msg))
#define ks_realloc_double_pp(op,n0,n1,msg) ks_cm_realloc_double_pp((op),(n0),(n1),KS_INDEX_MACRO, \
        (msg))
#define ks_realloc_buf_char(b,size,msg) ks_cm_realloc_buf_char((b),(size),KS_INDEX_MACRO,(msg))
#define ks_realloc_buf_int(b,size,msg) ks_cm_realloc_buf_int((b),(size),KS_INDEX_MACRO,(msg))
#define ks_realloc_buf_uint(b,size,msg) ks_cm_realloc_buf_uint((b),(size),KS_INDEX_MACRO,(msg))
#define ks_realloc_buf_double(b,size,msg) \
        ks_cm_realloc_buf_double((b),(size),KS_INDEX_MACRO,(msg))
#define ks_allocate_timer() ks_cm_allocate_timer(KS_INDEX_MACRO)
#define ks_allocate_comminucation_info() ks_cm_allocate_comminucation_info(KS_INDEX_MACRO)
#define ks_allocate_buf_uint(size,grow,msg) \
        ks_cm_allocate_buf_uint((size),(grow),KS_INDEX_MACRO,(msg))
#define ks_allocate_buf_char(size,msg) ks_cm_allocate_buf_char((size),KS_INDEX_MACRO,(msg))
#define ks_allocate_buf_char_pp(size,grow,len,msg) \
        ks_cm_allocate_buf_char_pp((size),(grow),(len),KS_INDEX_MACRO,(msg))
#define ks_allocate_buf_int(size,grow,msg) \
        ks_cm_allocate_buf_int((size),(grow),KS_INDEX_MACRO,(msg))
#define ks_allocate_buf_uint(size,grow,msg) \
        ks_cm_allocate_buf_uint((size),(grow),KS_INDEX_MACRO,(msg))
#define ks_allocate_buf_double(size,grow,msg) \
        ks_cm_allocate_buf_double((size),(grow),KS_INDEX_MACRO,(msg))
#define ks_allocate_circle_buffer(num) ks_cm_allocate_circle_buffer((num),KS_INDEX_MACRO)
#define ks_grow_buf_char(b,size) ks_cm_grow_buf_char((b),(size),KS_INDEX_MACRO)
#define ks_grow_buf_char_pp(b) ks_cm_grow_buf_char_pp((b),KS_INDEX_MACRO)
#define ks_grow_buf_int(b,msg) ks_cm_grow_buf_int((b),KS_INDEX_MACRO,(msg))
#define ks_grow_buf_uint(b,msg) ks_cm_grow_buf_uint((b),KS_INDEX_MACRO,(msg))
#define ks_grow_buf_float(b) ks_cm_grow_buf_float((b),KS_INDEX_MACRO)
#define ks_grow_buf_double(b) ks_cm_grow_buf_double((b),KS_INDEX_MACRO)
#define ks_set_buf_char(b,cp) ks_cm_set_buf_char((b),(cp),KS_INDEX_MACRO)
#define ks_cat_buf_char(b,cp) ks_cm_cat_buf_char((b),(cp),KS_INDEX_MACRO)
#define ks_set_buf_char_pp(b,i,s) ks_cm_set_buf_char_pp((b),(i),(s),KS_INDEX_MACRO)
#define ks_set_buf_int(b,i,v,msg) ks_cm_set_buf_int((b),(i),(v),KS_INDEX_MACRO,(msg))
#define ks_set_buf_uint(b,i,v,msg) ks_cm_set_buf_uint((b),(i),(v),KS_INDEX_MACRO,(msg))
#define ks_set_buf_float(b,i,v) ks_cm_set_buf_float((b),(i),(v),KS_INDEX_MACRO)
#define ks_set_buf_double(b,i,v) ks_cm_set_buf_double((b),(i),(v),KS_INDEX_MACRO)

#define ks_new_char_list(name,value) ks_cm_new_char_list((name),(value),KS_INDEX_MACRO)
#define ks_new_char_list_list(p) ks_cm_new_char_list_list((p),KS_INDEX_MACRO)
#define ks_new_int_list(val) ks_cm_new_int_list((val),KS_INDEX_MACRO)
#define ks_new_int_list2(key,val) ks_cm_new_int_list2((key),(val),KS_INDEX_MACRO)
#define ks_new_int_list_list(p) ks_cm_new_int_list_list((p),KS_INDEX_MACRO)
#define ks_new_wideuse_list(name,value) ks_cm_new_wideuse_list((name),(value),KS_INDEX_MACRO)
#define ks_new_range_list(name,value) ks_cm_new_range_list((name),(value),KS_INDEX_MACRO)

#define ks_start_timer(timer) ks_index_start_timer(KS_INDEX_MACRO,timer)

typedef struct KS_MEM_LIST KS_MEM_LIST;
struct KS_MEM_LIST{
  void *ptr;
  size_t size;
  char const* file;
  char const* func;
  int line;
  char *comment;
  KS_MEM_LIST *next;
};

typedef struct KS_CHAR_LIST KS_CHAR_LIST;
struct KS_CHAR_LIST{
  char *name;
  int value;
  KS_CHAR_LIST *next;
};
typedef struct KS_CHAR_LIST_LIST KS_CHAR_LIST_LIST;
struct KS_CHAR_LIST_LIST{
  KS_CHAR_LIST *p;
  KS_CHAR_LIST_LIST *next;
};

typedef struct KS_INT_LIST KS_INT_LIST;
struct KS_INT_LIST{
  int value;
  KS_INT_LIST *next;
};
typedef struct KS_INT_LIST2 KS_INT_LIST2;
struct KS_INT_LIST2{
  int key;
  int value;
  KS_INT_LIST2 *next;
};
typedef struct KS_INT_LIST_LIST KS_INT_LIST_LIST;
struct KS_INT_LIST_LIST{
  KS_INT_LIST *p;
  KS_INT_LIST_LIST *next;
};

typedef struct KS_WIDEUSE_LIST KS_WIDEUSE_LIST;
struct KS_WIDEUSE_LIST{
  char *name;
  int ip_num,dp_num;
  int *ip;
  double *dp;
  KS_WIDEUSE_LIST *next;
};

typedef struct KS_LABEL_LIST KS_LABEL_LIST;
struct KS_LABEL_LIST{
  unsigned int flags;
  unsigned int label;
  int value;
  KS_LABEL_LIST *next;
};

#define KS_RANGE_LIST_SELECTED (unsigned int)0x01
typedef struct KS_RANGE_LIST KS_RANGE_LIST;
struct KS_RANGE_LIST{
  unsigned int flags;
  double dpp[2][3];
  KS_RANGE_LIST *next;
};

typedef struct KS_TEXT_LIST KS_TEXT_LIST;
struct KS_TEXT_LIST{
  char *text;
  double lifetime;
  double start_time;
  KS_TEXT_LIST *next;
};

#define KS_STOERD_MAX_FILE_NAME 256
typedef struct KS_STORED_FILE_NAME KS_STORED_FILE_NAME;
struct KS_STORED_FILE_NAME{
  char all_name[KS_STOERD_MAX_FILE_NAME];
  char name[KS_STOERD_MAX_FILE_NAME];
  char ext[KS_STOERD_MAX_FILE_NAME];
};

#define KS_OPTIONS_MAX_FILE_EXT 256
#define KS_OPTIONS_BOOL 0
#define KS_OPTIONS_CHAR 1
#define KS_OPTIONS_INT 2
#define KS_OPTIONS_DOUBLE 3
#define KS_OPTIONS_MAX_CHAR_LEN 256
typedef struct KS_OPTIONS KS_OPTIONS;
struct KS_OPTIONS{
  int arg_num;
  int type;
  BOOL hit;
  void *vp;
  char key[KS_OPTIONS_MAX_CHAR_LEN];
  /*  char value[KS_OPTIONS_MAX_CHAR_LEN];*/
  char comment[KS_OPTIONS_MAX_CHAR_LEN];
  KS_OPTIONS *next;
};

#define KS_FILE_MAX_NAME_LEN 1024
typedef struct KS_FILES KS_FILES;
struct KS_FILES{
  int num;
  char **name;
};

#define KS_MAX_FILE_DATA_ONE_LINE 1024
typedef struct KS_COLUMN_DATA KS_COLUMN_DATA;
struct KS_COLUMN_DATA{
  int comment_num,column_num,num;
  char **comment;
  double **d;
};

typedef struct KS_BUF_CHAR KS_BUF_CHAR;
struct KS_BUF_CHAR{
  int size;
  int len;
  char *buf;
};
typedef struct KS_BUF_CHAR_PP KS_BUF_CHAR_PP;
struct KS_BUF_CHAR_PP{
  int size;
  int len;
  int num;
  int grow;
  char **buf;
};
typedef struct KS_BUF_INT KS_BUF_INT;
struct KS_BUF_INT{
  int size;
  int num;
  int *buf;
  int grow;
};
typedef struct KS_BUF_UINT KS_BUF_UINT;
struct KS_BUF_UINT{
  int size;
  int num;
  unsigned int *buf;
  int grow;
};
typedef struct KS_BUF_FLOAT KS_BUF_FLOAT;
struct KS_BUF_FLOAT{
  int size;
  int num;
  float *buf;
  int grow;
};
typedef struct KS_BUF_DOUBLE KS_BUF_DOUBLE;
struct KS_BUF_DOUBLE{
  int size;
  int num;
  double *buf;
  int grow;
};

typedef struct KS_AVERAGE_BUFFER KS_AVERAGE_BUFFER;
struct KS_AVERAGE_BUFFER{
  double average;
  double average2;
  unsigned int count;
};

typedef struct KS_CIRCLE_BUFFER KS_CIRCLE_BUFFER;
struct KS_CIRCLE_BUFFER{
  int num;
  double angle;
  double *x;
  double *y;
};

typedef struct KS_TIMER KS_TIMER;
struct KS_TIMER{
  double lap;
  double sum;
  double now;
  char vt_name[64];
  int vt_scl,vt_class,vt_func;
};

typedef struct KS_COMMINUCATION_INFO KS_COMMINUCATION_INFO;
struct KS_COMMINUCATION_INFO{
  KS_TIMER *timer;
  size_t size;
};

typedef struct KS_SPHERE KS_SPHERE;
struct KS_SPHERE{
  unsigned int label;
  char name[32];
  float radius;
  float color[3];
  KS_SPHERE *next;
};

enum {
  KS_ERROR_MSG_LEN = 256,
  KS_EXE_PATH_LEN = 256
};

#ifdef MSVC
#define KS_DIR_HANDLE HANDLE
#define KS_DIR_DATA WIN32_FIND_DATA
#define ks_get_dir_file_name(data) data.cFileName
#else
#define KS_DIR_HANDLE DIR*
#define KS_DIR_DATA struct dirent*
#define ks_get_dir_file_name(data) data->d_name
#endif

#ifdef SOCK
/* ks_sock */
#if 0
#if defined(__ICC)
// icc doesn't like GNU __extension__ functions
// this has to happen AFTER <netinet/in.h> !!
#undef htons
#undef ntohs
#undef htonl
#undef ntohl

# if __BYTE_ORDER == __BIG_ENDIAN
/* The host byte order is the same as network byte order,
   so these functions are all just identity.  */
# define ntohl(x)	(x)
# define ntohs(x)	(x)
# define htonl(x)	(x)
# define htons(x)	(x)
# else
#  if __BYTE_ORDER == __LITTLE_ENDIAN
#   define htons(x) __bswap_constant_16(x)
#   define ntohs(x) __bswap_constant_16(x)
#   define htonl(x) __bswap_constant_32(x)
#   define ntohl(x) __bswap_constant_32(x)
  /*
#   define ntohl(x)	__bswap_32 (x)
#   define ntohs(x)	__bswap_16 (x)
#   define htonl(x)	__bswap_32 (x)
#   define htons(x)	__bswap_16 (x)
  */
#  endif
# endif
#endif
#endif

enum {
  KS_SOCK_TCP,
  KS_SOCK_UDP
};

typedef struct{
  int id;
  int protocol; /* IPPROTO_TCP or IPPROTO_UDP */
  int type;     /* SOCK_STREAM or SOCK_DGRAM */
#if !defined(_WIN32) || defined(__CYGWIN__)  /* Linux or Cygwin */
  struct sockaddr_in udp_c_address;
#else /* WinSock */
  SOCKADDR_IN udp_c_address;
#endif
} KS_SOCK_BASE;
/* ks_sock end */
#endif

#define KS_IO_OUTPUT (unsigned int)0x01
#define KS_IO_INPUT  (unsigned int)0x02
#define KS_IO_FILE   (unsigned int)0x04
#define KS_IO_SOCK   (unsigned int)0x08
typedef struct KS_IO KS_IO;
struct KS_IO{
  unsigned int flags;
  FILE *fp;
#ifdef SOCK
  KS_SOCK_BASE *sock;
#endif
};

typedef struct {
  char *read_buf;
  char **file_lines;
  long long int line_cnt;
  long long int line_capacity;
  long long int size;
} KS_FILE_LINES;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MDM
  void m2_gm_finalize(void);
#endif

  void *ks_cm_malloc(size_t size, KS_INDEX_PROTO, char *comment);
  void *ks_cm_realloc(void *ptr, size_t size, KS_INDEX_PROTO, char *comment);
  void ks_cm_free(void *ptr, KS_INDEX_PROTO);
  void ks_wait_debug(void);
  /*  BOOL ks_error(char *fmt, ...);*/
#ifdef MSVC
  BOOL ks_error(char *fmt, ...);
  BOOL ks_index_error(KS_INDEX_PROTO, char *fmt, ...);
#else
  BOOL ks_index_error(KS_INDEX_PROTO, char *fmt, ...);
#endif
  void ks_clear_error_message(void);
  char *ks_get_error_message(void);
  size_t ks_output_memory_infomation(FILE *fp, int output_lim);
  void ks_check_mem_list(void);

  void ks_verbose(int verbose_level, int level, char *fmt, ...);
  BOOL ks_iscomment(char *c);
  BOOL ks_strval(char *buf, char *key, double *v);
  BOOL ks_isfloat(char c);
  BOOL ks_isfloat_all(char *str);
  BOOL ks_isalpha_all(char *str);
  BOOL ks_isblank_all(char *str);
  BOOL ks_isdigit_all(char *str);
  BOOL ks_store_file_name(KS_STORED_FILE_NAME *sn, char *name);
  BOOL ks_set_options(char *key, int arg_num, int type, void *vp, KS_OPTIONS **op, char *fmt, ...);
  BOOL ks_classify_options(int ac, char **av, KS_OPTIONS *op, char ***file, int *file_num,
			   KS_CHAR_LIST *file_ext);
  BOOL ks_get_options_hit(char *key, KS_OPTIONS *op);
  BOOL ks_set_options_hit(char *key, KS_OPTIONS *op);
  void ks_free_options(KS_OPTIONS *listp, char **file_name, int file_num);
  BOOL ks_output_options_comment(KS_OPTIONS *op, FILE *fp);
  void ks_print_options_list(KS_OPTIONS *op);
  BOOL ks_read_column_data(FILE *fp, KS_COLUMN_DATA *co);
  void ks_free_column_data(KS_COLUMN_DATA *co);
  void ks_print_error(FILE *fp);
  BOOL ks_get_common_name(char **name, int name_num, char *common, int size);
  char *ks_file_ext(char *s);
  BOOL ks_remove_file_ext(char *name, char *base, size_t size);
  BOOL ks_change_file_ext(char *in, char *ext, char *out, size_t out_size);
  double ks_get_time(void);
  KS_TIMER *ks_cm_allocate_timer(KS_INDEX_PROTO);
  void ks_index_start_timer(KS_INDEX_PROTO, KS_TIMER *timer);
  void ks_record_timer(KS_TIMER *timer);
  void ks_free_timer(KS_TIMER *timer);
  BOOL ks_set_flags_range(int num, int *flg, char *types);
  char *ks_cm_malloc_char_p(size_t n0, KS_INDEX_PROTO, char *msg);
  char *ks_cm_realloc_char_p(char *op, size_t n, KS_INDEX_PROTO, char *msg);
  char **ks_cm_malloc_char_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg);
  void ks_free_char_pp(size_t n0, char **p);
  int *ks_cm_malloc_int_p(size_t n, KS_INDEX_PROTO, char *msg);
  KS_BUF_UINT *ks_cm_allocate_buf_uint(int size, int grow, KS_INDEX_PROTO, char *msg);
  void ks_free_buf_uint(KS_BUF_UINT *b);
  BOOL ks_cm_grow_buf_uint(KS_BUF_UINT *b, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_realloc_buf_uint(KS_BUF_UINT *b, size_t size, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_set_buf_uint(KS_BUF_UINT *b, int i, int v, KS_INDEX_PROTO, char *msg);
  unsigned int *ks_cm_malloc_uint_p(size_t n, KS_INDEX_PROTO, char *msg);
  int *ks_cm_realloc_int_p(int *op, size_t n, KS_INDEX_PROTO, char *msg);
  int **ks_cm_malloc_int_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg);
  int ***ks_cm_malloc_int_ppp(size_t n0, size_t n1, size_t n2, KS_INDEX_PROTO, char *msg);
  unsigned int *ks_cm_realloc_uint_p(unsigned int *op, size_t n, KS_INDEX_PROTO, char *msg);
  void ks_free_int_pp(size_t n0, int **p);
  void ks_free_int_ppp(size_t n0, size_t n1, int ***p);
  /*
  unsigned int *ks_cm_malloc_unsigned_int_p(int n, KS_INDEX_PROTO, char *msg);
  unsigned int **ks_cm_malloc_unsigned_int_pp(int n0, int n1, KS_INDEX_PROTO, char *msg);
  unsigned int *ks_cm_realloc_unsigned_int_p(unsigned int *op, int n,KS_INDEX_PROTO, char *msg);
  void ks_free_unsigned_int_pp(int n0, unsigned int **p);
  */
  float *ks_cm_malloc_float_p(size_t n, KS_INDEX_PROTO, char *msg);
  float *ks_cm_realloc_float_p(float *op, size_t n, KS_INDEX_PROTO, char *msg);
  float **ks_cm_malloc_float_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg);
  double *ks_cm_malloc_double_p(size_t n, KS_INDEX_PROTO, char *msg);
  double *ks_cm_realloc_double_p(double *op, size_t n, KS_INDEX_PROTO, char *msg);
  double **ks_cm_malloc_double_pp(size_t n0, size_t n1, KS_INDEX_PROTO, char *msg);
  double **ks_cm_realloc_double_pp(double **op, size_t n1, size_t n2, KS_INDEX_PROTO, char *msg);
  double ***ks_cm_malloc_double_ppp(size_t n0, size_t n1, size_t n2, KS_INDEX_PROTO, char *msg);
  void ks_free_float_pp(size_t n0, float **p);
  void ks_free_double_pp(size_t n0, double **p);
  void ks_free_double_ppp(size_t n0, size_t n1, double ***p);
  KS_BUF_CHAR *ks_cm_allocate_buf_char(size_t size, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_grow_buf_char(KS_BUF_CHAR *b, const size_t new_size, KS_INDEX_PROTO);
  BOOL ks_cm_realloc_buf_char(KS_BUF_CHAR *b, const size_t new_size,KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_set_buf_char(KS_BUF_CHAR *b, const char *cp, KS_INDEX_PROTO);
  BOOL ks_cm_cat_buf_char(KS_BUF_CHAR *b, const char *cp, KS_INDEX_PROTO);
  void ks_clear_buf_char(KS_BUF_CHAR *b);
  void ks_free_buf_char(KS_BUF_CHAR *b);
  KS_BUF_CHAR_PP *ks_cm_allocate_buf_char_pp(int size, int grow, int len, KS_INDEX_PROTO,
						char *msg);
  BOOL ks_cm_grow_buf_char_pp(KS_BUF_CHAR_PP *b, KS_INDEX_PROTO);
  BOOL ks_cm_set_buf_char_pp(KS_BUF_CHAR_PP *b, int i, char *s, KS_INDEX_PROTO);
  void ks_free_buf_char_pp(KS_BUF_CHAR_PP *b);
  KS_BUF_INT *ks_cm_allocate_buf_int(int size, int grow, KS_INDEX_PROTO, char *msg);
  void ks_free_buf_int(KS_BUF_INT *b);
  BOOL ks_cm_grow_buf_int(KS_BUF_INT *b, KS_INDEX_PROTO,char *msg);
  BOOL ks_cm_realloc_buf_int(KS_BUF_INT *b, size_t size, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_set_buf_int(KS_BUF_INT *b, int i, int v, KS_INDEX_PROTO, char *msg);
  KS_BUF_FLOAT *ks_cm_allocate_buf_float(int size, int grow, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_grow_buf_float(KS_BUF_FLOAT *b, KS_INDEX_PROTO);
  BOOL ks_cm_realloc_buf_float(KS_BUF_FLOAT *b, size_t size, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_set_buf_float(KS_BUF_FLOAT *b, int i, float v, KS_INDEX_PROTO);
  void ks_free_buf_float(KS_BUF_FLOAT *b);
  KS_BUF_DOUBLE *ks_cm_allocate_buf_double(int size, int grow, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_grow_buf_double(KS_BUF_DOUBLE *b, KS_INDEX_PROTO);
  BOOL ks_cm_realloc_buf_double(KS_BUF_DOUBLE *b, size_t size, KS_INDEX_PROTO, char *msg);
  BOOL ks_cm_set_buf_double(KS_BUF_DOUBLE *b, int i, double v, KS_INDEX_PROTO);
  void ks_free_buf_double(KS_BUF_DOUBLE *b);
  char* ks_cm_malloc_char_copy(char *c, KS_INDEX_PROTO, char *msg);
  BOOL ks_add_search_path(KS_CHAR_LIST **search_path, char *path);
  FILE *ks_open_file(char *file_name, char *mode, KS_CHAR_LIST *search_path);
  /*
    char* ks_get_chars(char *list, char *s, char *d, int size);
  */
  char* ks_skip_chars(char *list, char *s);
  BOOL ks_isreturn(char c);
  KS_CHAR_LIST *ks_cm_new_char_list(char *name, int value, KS_INDEX_PROTO);
  KS_CHAR_LIST *ks_add_char_list(KS_CHAR_LIST *listp, KS_CHAR_LIST *newp);
  KS_CHAR_LIST *ks_addend_char_list(KS_CHAR_LIST *listp, KS_CHAR_LIST *newp);
  KS_CHAR_LIST *ks_lookup_char_list(KS_CHAR_LIST *listp, char *name);
  KS_CHAR_LIST *ks_lookup_char_list_value(KS_CHAR_LIST *listp, int value);
  KS_CHAR_LIST *ks_del_char_list_value(KS_CHAR_LIST *listp, int value);
  int ks_count_char_list(KS_CHAR_LIST *listp);
  void ks_free_char_list(KS_CHAR_LIST *listp);
  KS_CHAR_LIST_LIST *ks_cm_new_char_list_list(KS_CHAR_LIST *p, KS_INDEX_PROTO);
  KS_CHAR_LIST_LIST *ks_add_char_list_list(KS_CHAR_LIST_LIST *listp, KS_CHAR_LIST_LIST *newp);
  void ks_free_char_list_list(KS_CHAR_LIST_LIST *listp);
  KS_INT_LIST *ks_cm_new_int_list(int val, KS_INDEX_PROTO);
  KS_INT_LIST *ks_add_int_list(KS_INT_LIST *listp, KS_INT_LIST *newp);
  KS_INT_LIST *ks_cat_int_list(KS_INT_LIST *listp, KS_INT_LIST *newp);
  KS_INT_LIST *ks_addend_int_list(KS_INT_LIST *listp, KS_INT_LIST *newp);
  KS_INT_LIST *ks_lookup_int_list(KS_INT_LIST *listp, int val);
  int ks_count_int_list(KS_INT_LIST *listp);
  KS_INT_LIST *ks_del_int_list(KS_INT_LIST *listp, int value);
  void ks_free_int_list(KS_INT_LIST *listp);
  KS_INT_LIST2 *ks_cm_new_int_list2(int key, int val, KS_INDEX_PROTO);
  KS_INT_LIST2 *ks_add_int_list2(KS_INT_LIST2 *listp, KS_INT_LIST2 *newp);
  KS_INT_LIST2 *ks_cat_int_list2(KS_INT_LIST2 *listp, KS_INT_LIST2 *newp);
  KS_INT_LIST2 *ks_addend_int_list2(KS_INT_LIST2 *listp, KS_INT_LIST2 *newp);
  KS_INT_LIST2 *ks_lookup_int_list2(KS_INT_LIST2 *listp, int key);
  int ks_count_int_list2(KS_INT_LIST2 *listp);
  void ks_free_int_list2(KS_INT_LIST2 *listp);


  KS_INT_LIST_LIST *ks_cm_new_int_list_list(KS_INT_LIST *p, KS_INDEX_PROTO);
  KS_INT_LIST_LIST *ks_add_int_list_list(KS_INT_LIST_LIST *listp, KS_INT_LIST_LIST *newp);
  int ks_count_int_list_list(KS_INT_LIST_LIST *listp);
  void ks_free_int_list_list(KS_INT_LIST_LIST *listp);
  KS_WIDEUSE_LIST *ks_cm_new_wideuse_list(char *name, int ip_num, int *ip, int dp_num, double *dp,
					  KS_INDEX_PROTO);
  KS_WIDEUSE_LIST *ks_add_wideuse_list(KS_WIDEUSE_LIST *listp, KS_WIDEUSE_LIST *newp);
  KS_WIDEUSE_LIST *ks_addend_wideuse_list(KS_WIDEUSE_LIST *listp, KS_WIDEUSE_LIST *newp);
  KS_WIDEUSE_LIST *ks_lookup_wideuse_list(KS_WIDEUSE_LIST *listp, char *name);
  int ks_count_wideuse_list(KS_WIDEUSE_LIST *listp);
  void ks_free_wideuse_list(KS_WIDEUSE_LIST *listp);
  KS_RANGE_LIST *ks_cm_new_range_list(unsigned int flags, double range[2][3], KS_INDEX_PROTO);
  KS_RANGE_LIST *ks_add_range_list(KS_RANGE_LIST *listp, KS_RANGE_LIST *newp);
  KS_RANGE_LIST *ks_addend_range_list(KS_RANGE_LIST *listp, KS_RANGE_LIST *newp);
  int ks_count_range_list(KS_RANGE_LIST *listp);
  KS_RANGE_LIST *ks_divide_range_list_periodic_cell(KS_RANGE_LIST *listp,double cell[KS_RANGE][3]);
  void ks_free_range_list(KS_RANGE_LIST *listp);

  void ks_remove_return(char *s);
  void ks_remove_end_blank(char *s);
  char* ks_get_str(char *list, char *s, char *d, int size);
  void ks_exit(int i);
  void ks_init_average_buffer(KS_AVERAGE_BUFFER *av);
  void ks_add_average_buffer(KS_AVERAGE_BUFFER *av, double val);
  KS_CIRCLE_BUFFER *ks_cm_allocate_circle_buffer(int num, KS_INDEX_PROTO);
  void ks_free_circle_buffer(KS_CIRCLE_BUFFER *cir);
  size_t ks_io(KS_IO io, void *p, size_t size, size_t num);
  KS_SPHERE *ks_new_sphere(unsigned int label, char *name, float radius, float color[3]);
  KS_SPHERE *ks_add_sphere(KS_SPHERE *listp, KS_SPHERE *newp);
  KS_SPHERE *ks_addend_sphere(KS_SPHERE *listp, KS_SPHERE *newp);
  KS_SPHERE *ks_lookup_sphere(KS_SPHERE *listp, char *name);
  int ks_count_sphere(KS_SPHERE *listp);
  void ks_free_sphere(KS_SPHERE *listp);
  char* ks_upper_all(char *s);
  void ks_strncpy(char *d, const char *s, const size_t size);
  void ks_strncat(char *d, const char *s, const size_t size);
  BOOL ks_check_memory(void);
  KS_COMMINUCATION_INFO *ks_cm_allocate_comminucation_info(KS_INDEX_PROTO);
  void ks_free_comminucation_info(KS_COMMINUCATION_INFO *info);
  void ks_copy_range(double d_range[2][3], double s_range[2][3]);
  void ks_marge_range(double total_range[2][3], double range[2][3]);
  BOOL ks_is_overlap_range(double range0[2][3], double range1[2][3]);
  BOOL ks_is_overlap_range_list(KS_RANGE_LIST *range0, KS_RANGE_LIST *range1);
  char *ks_get_date(void);
  KS_LABEL_LIST *ks_new_label_list(unsigned int label, unsigned int value, unsigned int flags);
  KS_LABEL_LIST *ks_add_label_list(KS_LABEL_LIST *listp, KS_LABEL_LIST *newp);
  KS_LABEL_LIST *ks_lookup_label_list(KS_LABEL_LIST* listp, unsigned int label);
  int ks_count_label_list(KS_LABEL_LIST *listp);
  void ks_free_label_list(KS_LABEL_LIST *listp);
  KS_TEXT_LIST *ks_new_text_list(char *text, double lifetime);
  KS_TEXT_LIST *ks_add_text_list(KS_TEXT_LIST *listp, KS_TEXT_LIST *newp);
  KS_TEXT_LIST *ks_addend_text_list(KS_TEXT_LIST *listp, KS_TEXT_LIST *newp);
  KS_TEXT_LIST *ks_lookup_text_list(KS_TEXT_LIST *listp, char *text, int size);
  int ks_count_text_list(KS_TEXT_LIST *listp);
  KS_TEXT_LIST *ks_del_text_list(KS_TEXT_LIST *listp, char *text);
  KS_TEXT_LIST *ks_del_text_list_time(KS_TEXT_LIST *listp, double time);
  void ks_free_text_list(KS_TEXT_LIST *listp);
  BOOL ks_read_dir_first(char *path, KS_DIR_HANDLE *handle, KS_DIR_DATA *data);
  BOOL ks_read_dir_next(KS_DIR_HANDLE handle, KS_DIR_DATA *data);
  void ks_close_dir(KS_DIR_HANDLE handle);
  BOOL ks_set_exe_path(char *argv0);
  char *ks_get_exe_path(void);
#ifdef SOCK
  /* ks_sock */
  KS_SOCK_BASE *ks_sock_open_server(unsigned short int port, int mode);
  KS_SOCK_BASE *ks_sock_open_client(char *host, unsigned short int port, int mode);
  size_t ks_sock_recv(KS_SOCK_BASE *sb, char *buf, size_t count);
  size_t ks_sock_send(KS_SOCK_BASE *sb, const char *buf, size_t count);
  size_t ks_sock_send_char(KS_SOCK_BASE *sb, char* buf, size_t size);
  size_t ks_sock_send_int(KS_SOCK_BASE *sb, int* buf, size_t size);
  size_t ks_sock_send_double(KS_SOCK_BASE *sb, double* buf, size_t size);
  size_t ks_sock_recv_char(KS_SOCK_BASE *sb, char* buf, size_t size);
  size_t ks_sock_recv_int(KS_SOCK_BASE *sb, int* buf, size_t size);
  size_t ks_sock_recv_double(KS_SOCK_BASE *sb, double* buf, size_t size);
  void ks_sock_close(KS_SOCK_BASE *sb);
  /* ks_sock end */
#endif
  KS_INT_LIST *ks_expand_sequence(char *c);
  void ks_get_file_path(char *file_name, char *path, size_t path_size);
  int ks_fprintf2(FILE *fp1, FILE *fp2, const char *fmt,...);
  BOOL ks_combsort_int_buf(int *data, int num, int **sb);
  BOOL ks_combsort_double_buf(double *data, int num, int **sb);
  BOOL ks_inssort_int_buf(int *data, int num, int **sb);
  BOOL ks_inssort_inv_int_buf(int *data, int num, int **sb);
  void ks_enable_error_hit_enter_key();
  void ks_disable_error_hit_enter_key();
  void ks_enable_hit_enter_key();
  void ks_disable_hit_enter_key();
  void ks_hit_enter_key();
  BOOL ks_replace_string(char *input, int size, char *output,
			 const char *serach, const char *replace);
  int ks_printf_flush_stdout(const char* fmt, ...);
  KS_FILE_LINES* ks_allocate_file_lines();
  void ks_free_file_lines(KS_FILE_LINES *fi);
  BOOL ks_read_file_lines(const char* file_name, KS_FILE_LINES* fi);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

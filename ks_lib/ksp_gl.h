#ifndef KSP_GL_H
#define KSP_GL_H

#ifdef USE_GLM

typedef float gvec3[3];
typedef float gvec4[4];
typedef float gmat3[9];
typedef float gmat4[16];

#ifdef __cplusplus

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class GlmAgent {
public:
  GlmAgent() : projMatrix(), viewMatrix(), modelMatrix() {}
  void lookAt(const glm::vec3 eye, const glm::vec3 center, const glm::vec3 up);
private:
  glm::mat4 projMatrix;
  glm::mat4 viewMatrix;
  glm::mat4 modelMatrix;
};


extern "C" {
#endif /* __cplusplus */

  void *construct_Glm(void);
  void destruct_Glm(void *that);
  void glm_lookat(void *that, gvec3 eye, gvec3 center, gvec3 up);
  void glm_multi_3x3(gmat3 result, gmat3 lhs, gmat3 rhs);
  void glm_multi_4x4(gmat4 result, gmat4 lhs, gmat4 rhs);
  void glm_translate_matrix(gmat4 result, gmat4 mat, gvec3 translation);
  void glm_rotate_matrix(gmat4 result, gmat4 mat, float angle, gvec3 axis);
  void glm_rotate_matrix_xyz(gmat4 result, gmat4 mat, float angle, float ax, float ay, float az);
  void glm_rotate_matrix_x(gmat4 result, gmat4 mat, float angle);
  void glm_rotate_matrix_y(gmat4 result, gmat4 mat, float angle);
  void glm_rotate_matrix_z(gmat4 result, gmat4 mat, float angle);
  void glm_scale_matrix(gmat4 result, gmat4 mat, gvec3 factors);
  void glm_scale_matrix_xyz(gmat4 result, gmat4 mat, float x, float y, float z);
  void glm_scale_matrix_s(gmat4 result, gmat4 mat, float r);
  void print_gmat3(gmat3 m);
  void print_gmat4(gmat4 m);
  
#ifdef __cplusplus
};
#endif /*  __cplusplus  */

#endif // USE_GLM

#endif

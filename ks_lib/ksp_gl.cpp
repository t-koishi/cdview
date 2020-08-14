
#include <iostream>
#include <iomanip>
#include <ksp_gl.h>

using namespace std;

#ifdef USE_GLM
void GlmAgent::lookAt(const glm::vec3 eye, const glm::vec3 center, const glm::vec3 up)
{
  viewMatrix = glm::lookAt(eye,center,up);
}

// glm wrappers
void *construct_Glm()
{
  GlmAgent *that;
  that = new GlmAgent();
  return static_cast<void*>(that);
}
void destruct_Glm(void *that)
{
  static_cast<GlmAgent*>(that)->~GlmAgent();
}
void glm_lookat(void *that, gvec3 eye, gvec3 center, gvec3 up)
{
  static_cast<GlmAgent*>(that)->lookAt(*reinterpret_cast<glm::vec3*>(eye),
				       *reinterpret_cast<glm::vec3*>(center),
				       *reinterpret_cast<glm::vec3*>(up));
}

// friend functions
namespace {
glm::vec3 xAxis(1.0f,0.0f,0.0f);
glm::vec3 yAxis(0.0f,1.0f,0.0f);
glm::vec3 zAxis(0.0f,0.0f,1.0f);
void printMatBorder()
{
  cout << "------------" << endl;
}
}
void glm_multi_3x3(gmat3 result, gmat3 lhs, gmat3 rhs)
{
  *reinterpret_cast<glm::mat3*>(result) =
    *reinterpret_cast<glm::mat3*>(lhs) * *reinterpret_cast<glm::mat3*>(rhs);
}
void glm_multi_4x4(gmat4 result, gmat4 lhs, gmat4 rhs)
{
  *reinterpret_cast<glm::mat4*>(result) =
    *reinterpret_cast<glm::mat4*>(lhs) * *reinterpret_cast<glm::mat4*>(rhs);
}
void glm_translate_matrix(gmat4 result, gmat4 mat, gvec3 translation)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::translate(*reinterpret_cast<glm::mat4*>(mat),*reinterpret_cast<glm::vec3*>(translation));
}
void glm_rotate_matrix(gmat4 result, gmat4 mat, float angle, gvec3 axis)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::rotate(*reinterpret_cast<glm::mat4*>(mat),angle,
		*reinterpret_cast<glm::vec3*>(axis));
}
void glm_rotate_matrix_xyz(gmat4 result, gmat4 mat, float angle, float ax, float ay, float az)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::rotate(*reinterpret_cast<glm::mat4*>(mat),angle,glm::vec3(ax,ay,az));
}
void glm_rotate_matrix_x(gmat4 result, gmat4 mat, float angle)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::rotate(*reinterpret_cast<glm::mat4*>(mat),angle,xAxis);
}
void glm_rotate_matrix_y(gmat4 result, gmat4 mat, float angle)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::rotate(*reinterpret_cast<glm::mat4*>(mat),angle,yAxis);
}
void glm_rotate_matrix_z(gmat4 result, gmat4 mat, float angle)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::rotate(*reinterpret_cast<glm::mat4*>(mat),angle,zAxis);

}
void glm_scale_matrix(gmat4 result, gmat4 mat, gvec3 factors)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::scale(*reinterpret_cast<glm::mat4*>(mat),
		*reinterpret_cast<glm::vec3*>(factors));
}
void glm_scale_matrix_xyz(gmat4 result, gmat4 mat, float x, float y, float z)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::scale(*reinterpret_cast<glm::mat4*>(mat),glm::vec3(x,y,z));
}
void glm_scale_matrix_s(gmat4 result, gmat4 mat, float r)
{
  *reinterpret_cast<glm::mat4*>(result) =
    glm::scale(*reinterpret_cast<glm::mat4*>(mat),glm::vec3(r,r,r));
}
void print_gmat3(gmat3 m)
{
  int i,j;
  printMatBorder();
  for(i = 0; i < 3; i++){
    for(j = 0; j < 3; j++){
      cout << fixed << setprecision(4) << " " << ((float(*)[3])m)[i][j];
    }
    cout << endl;
  }
  printMatBorder();
}
void print_gmat4(gmat4 m)
{
  int i,j;
  printMatBorder();
  for(i = 0; i < 4; i++){
    for(j = 0; j < 4; j++){
      cout << fixed << setprecision(4) << " " << ((float(*)[4])m)[i][j];
    }
    cout << endl;
  }  
  printMatBorder();
}

#endif

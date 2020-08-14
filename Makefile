
ALT_PICK=false
GLSL=true

CC := x86_64-w64-mingw32-gcc # 64 bit
#CC := i686-w64-mingw32-gcc  # 32 bit
CXX := x86_64-w64-mingw32-g++

# Please set following variables to compile the target in Makefile.local
# GLUT_INCLUDE_DIR, GLUT_LIB_DIR,
# GLEW_INCLUDE_DIR, GLEW_LIB_DIR,
# GLEXT_INCLUDE_DIR, GLM_INCLUDE_DIR
-include Makefile.local

OPT := -Wreturn-type
ifeq ($(OP),true)
OPT += -O3 -DNDEBUG
endif # $(OP)
ifeq ($(DS),true)
OPT += -g
endif # $(DS)
ifeq ($(WA),true)
#OPT += -Wunused -Werror -u
OPT += -Wall -Werror -u
endif # $(WA)

GL_FLAGS := -I$(GLUT_INCLUDE_DIR)  -DGLUT -DFREEGLUT_STATIC
GL_LIBS := -L$(GLUT_LIB_DIR) -lfreeglut_static -lglu32 -lopengl32 -lgdi32 -lWinmm -static -lstdc++

ifeq ($(GLSL),true)
GL_FLAGS += -DUSE_GLSL -DGLEW_STATIC -I$(GLEW_INCLUDE_DIR) -I$(GLEXT_INCLUDE_DIR) \
            -DUSE_GLM -I$(GLM_INCLUDE_DIR)
GL_LIBS += -L$(GLEW_LIB_DIR)  -lglew32s
endif # $(GLSL)

ifndef KS_LIB_DIR
KS_LIB_DIR := ks_lib
endif # $(KS_LIB_DIR)

CFLAGS := -I$(KS_LIB_DIR) $(GL_FLAGS) $(OPT) \
           -DUSE_PRINTF_FFLUSH_STDOUT_ALWAYS

KS_SRCS := ks_atom.c  ks_gl.c  ks_math.c  ks_pdb.c  ks_std.c
KS_OBJS := $(subst .c,.o,$(KS_SRCS))
KS_DEPS := $(subst .c,.d,$(KS_SRCS))
KS_SRCS := $(addprefix $(KS_LIB_DIR)/,$(KS_SRCS))

KSP_SRCS := ksp_gl.cpp
KSP_OBJS := $(subst .cpp,.o,$(KSP_SRCS))
KSP_DEPS := $(subst .cpp,.d,$(KSP_SRCS))
KSP_SRCS := $(addprefix $(KS_LIB_DIR)/,$(KSP_SRCS))

ifeq ($(ALT_PICK),true)
CFLAGS += -DUSE_ALT_PICK
endif # $(ALT_PICK)

LDLIBS := $(GL_LIBS)  -lm

SRCS := cv304.c fv033.c cdview3_main.c
HEDS := cv304.h fv033.h
OBJS := $(subst .c,.o,$(SRCS)) $(KS_OBJS) $(KSP_OBJS)
DEPS := $(subst .c,.d,$(SRCS)) $(KS_DEPS) $(KSP_DEPS)

TARGET := cdview

ifeq ($(OS),Windows_NT)
TARGET := $(addsuffix .exe,$(TARGET))
endif # $(OS)

.PHONY: all
all : $(TARGET)

$(TARGET) : $(OBJS)
	${CXX} -o $@ ${OBJS} $(LDLIBS)

%.o: %.c
	@${CC} $< -MM -MP -MF $*.d -I$(KS_LIB_DIR)
	${CC} -c $< -o $@ $(CFLAGS)

%.o: $(KS_LIB_DIR)/%.c
	@${CC} $< -MM -MP -MF $*.d -I$(KS_LIB_DIR)
	${CC} -c $< -o $@ $(CFLAGS)

%.o: $(KS_LIB_DIR)/%.cpp
	@${CC} $< -MM -MP -MF $*.d -I$(KS_LIB_DIR)
	${CXX} -c $< -o $@ $(CFLAGS)

ifeq ($(findstring clean,${MAKECMDGOALS}),)
  -include ${DEPS}
endif

.PHONY: install
install: 
	cp $(TARGET) $(HOME)/bin/

CV_VER_MAJOR=$(shell grep CV_VER_MAJOR cv304.h | sed -e 's/\s*CV_VER_MAJOR = \([0-9]*\),/\1/')
CV_VER_MINOR=$(shell grep CV_VER_MINOR cv304.h | sed -e 's/\s*CV_VER_MINOR = \([0-9]*\),/\1/')
CV_VER_TEENY=$(shell grep CV_VER_TEENY cv304.h | sed -e 's/\s*CV_VER_TEENY = \([0-9]*\)/\1/')
VER=$(shell printf "%d_%d_%d" $(CV_VER_MAJOR) $(CV_VER_MINOR) $(CV_VER_TEENY))

.PHONY: zip
zip:
	zip cdview$(VER).zip $(TARGET)

.PHONY: clean
clean :
	rm *.o $(TARGET)


ALT_PICK=false
GLSL=true

CC := x86_64-w64-mingw32-gcc # 64 bit
#CC := i686-w64-mingw32-gcc  # 32 bit
CXX := x86_64-w64-mingw32-g++

-include Makefile.local

ifeq ($(OP),true)
OPT += -O3
endif # $(OP)
ifeq ($(DS),true)
OPT += -g
else
OPT += -DNDEBUG
endif # $(DS)
ifeq ($(WA),true)
#OPT += -Wunused -Werror -u
OPT += -Wno-unused-variable -Wall
# OPT += -Wall -Werror -u
endif # $(WA)

GL_FLAGS := -I$(GLUT_INCLUDE_DIR)  -DGLUT -DFREEGLUT_STATIC
GL_LIBS := -L$(GLUT_LIB_DIR) -lfreeglut_static -lglu32 -lopengl32 -lgdi32 -lWinmm -static -lstdc++

ifeq ($(GLSL),true)
GL_FLAGS += -DUSE_GLSL -DGLEW_STATIC -I$(GLEW_INCLUDE_DIR) -I$(GLEXT_INCLUDE_DIR) \
            -DUSE_GLM -I$(GLM_INCLUDE_DIR)
GL_LIBS += -L$(GLEW_LIB_DIR)  -lglew32s
endif

ifndef KS_LIB_DIR
KS_LIB_DIR := ks_lib
endif

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

install: 
	cp cdview.exe /home/koishi/bin/

VER_LINE=$(shell grep CV_VER cv304.h)
VER=$(wordlist 3,3,$(VER_LINE))

update:
	cd ..; zip -r cdview.zip cdview/cdview.exe; mv cdview.zip cdview/cdview$(VER)_win.zip
	cp cdview_main.c cdview_src/
	cp cv304.c cv304.h fv033.c fv033.h cdview_src/
	cp $(KS_HEDS) cdview_src/
	cp $(subst .h,.c,$(KS_HEDS)) cdview_src/
	cp Makefile.linux cdview_src/Makefile
	sed -i 's/..\/ks_lib\//.\//' cdview_src/Makefile
	chmod a-x cdview_src/*
	mv cdview_src cdview$(VER)_src
#	zip -r cdview$(VER)_src.zip cdview$(VER)_src/
	tar zcvf cdview$(VER)_src.tar.gz cdview$(VER)_src/
	mv cdview$(VER)_src cdview_src/

clean_all :
	rm *.o cdview.exe $(KS_LIB_DIR)/*.o

clean :
	rm *.o cdview.exe

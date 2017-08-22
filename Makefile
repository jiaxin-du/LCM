#compiler for C++
CC := g++

OPT_FLAGS := -O3 -flto

OMP_FLAGS := -fopenmp

CPP_FLAGS := $(CFLAGS) -Werror

#deal with ROOT options
ifneq "$(ROOTSYS)" ""
   ROOTFLAGS := $(shell root-config --cflags)
   ROOTLIBS := $(shell root-config --libs)
   ROOTGLIBS := $(shell root-config --glibs)
endif

FFTWFLAGS := -lfftw3

PARENT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

#all headers
HDR_LIST := defines.h array.h exsource.h layer.h
HDR_LIST += lcm.h misc.h neurgrp.h rand.h receptor.h
HDR_LIST += spikesrc.h stimulator.h synpconn.h simulation.h

HDR_FILES := $(addprefix $(PARENT_DIR)/src/,$(HDR_LIST))

#all class file
CPP_LIST := array.cpp layer.cpp misc.cpp rand.cpp lcm.cpp
CPP_LIST += spikesrc.cpp synpconn.cpp exsource.cpp neurgrp.cpp
CPP_LIST += receptor.cpp stimulator.cpp simulation.cpp

CPP_FILES := $(addprefix $(PARENT_DIR)/src/,$(CPP_LIST))

#object file
OBJ_LIST := $(subst .cpp,.o,$(CPP_LIST))

#
PRINT_LIST := README src/defines.h src/misc.h src/misc.cpp src/rand.h src/rand.cpp \
            src/array.h src/array.cpp src/layer.h src/layer.cpp src/receptor.h \
            src/receptor.cpp src/spikesrc.h src/spikesrc.cpp src/stimulator.h \
            src/stimulator.cpp src/exsource.h src/exsource.cpp src/neurgrp.h \
            src/neurgrp.cpp src/synpconn.h src/synpconn.cpp src/lcm.h src/lcm.cpp \
            src/simulation.h src/simulation.cpp runlcm.cpp para_templt.cfg mktree.cpp 

PRINT_FILES := $(addprefix $(PARENT_DIR)/,$(PRINT_LIST)) 

all: runlcm mktree analyse 
	rm -fr *~ 
runlcm_%: $(PARENT_DIR)/runlcm.cpp $(CPP_FILES) $(HDR_FILES)
	$(info >>> Compiling ${@} <<<)
	$(CC) -o $@ $(PARENT_DIR)/runlcm.cpp $(CPP_FILES) $(CPP_FLAGS) $(OMP_FLAGS) $(OPT_FLAGS)
runlcm: $(PARENT_DIR)/runlcm.cpp $(CPP_FILES) $(HDR_FILES)
	$(info >>> Compiling ${@} <<<)
	$(CC) -o $@ $(PARENT_DIR)/runlcm.cpp $(CPP_FILES) $(CPP_FLAGS) $(OMP_FLAGS) $(OPT_FLAGS) 
mktree: $(PARENT_DIR)/mktree.cpp $(CPP_FILES) $(HDR_FILES)
	$(info >>> Compiling ${@} <<<)
	$(CC) -o $@ $(PARENT_DIR)/mktree.cpp $(CPP_FILES) $(CPP_FLAGS) $(OMP_FLAGS) $(ROOTFLAGS) $(ROOTLIBS)
print: $(PRINT_FILES) 
	a2ps -q -E --header="Laminar Cortex Model by Jiaxin DU (jiaxin.du@uqconnect.edu.au)" -o print.ps $(PRINT_FILES) 
	ps2pdf print.ps 
	rm -fr print.ps
clean: 
	rm -fr runlcm.o $(OBJ_LIST)
distclean: clean
	rm -fr runlcm mktree analyse
%.o: $(PARENT_DIR)/src/%.cpp $(HERADER_LIST)
	$(CC) -o $@ $< -Ofast -c $(CPP_FLAGS) $(OPT_FLAGS) $(OMP_FLAGS) $(IPO_FLAGS)

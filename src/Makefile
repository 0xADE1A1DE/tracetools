INCL=-I../include -I../src -I../thirdparty/ -I/usr/local/opt/boost/include/
#FLAGS=-g -Wall -O0 -fsanitize=address -fno-inline-functions -fPIC -D_GLIBCXX_DEBUG -DDEBUG
FLAGS=-g -Wall -O2 -fPIC -mavx 
#FLAGS=-g -Wall -O0  -fno-inline-functions -fPIC -DDEBUG -D_GLIBCXX_DEBUG
ifdef PARALLEL
FLAGS+=-fopenmp -DPARALLEL
endif
CXXFLAGS=$(FLAGS) -std=c++11 
CFLAGS=$(FLAGS)
IIR_INC=-I../thirdparty/iir1/base/include
IIR_LIB=-L../thirdparty/iir1/base/lib -liir_static
LIBNAME_BASE=pwmodel
LIBS=-L. -l$(LIBNAME_BASE) -lpthread -ldl -lboost_filesystem -lfftw3 $(IIR_LIB)
INCL:=$(INCL) $(IIR_INC)
SRC_C_FILES=$(wildcard *.c) $(wildcard ../thirdparty/*/*.c)
SRC_CXX_FILES=$(wildcard *.cpp) $(wildcard ../thirdparty/*/*.cpp)
OBJ_C_FILES=$(patsubst %.c,%.o, $(SRC_C_FILES))
OBJ_CXX_FILES=$(patsubst %.cpp,%.o, $(SRC_CXX_FILES))

TPROC_OBJ_FILES=traceprocessor.o PWArgs.o  
TPROC_BINNAME=traceprocessor

BIVM_OBJ_FILES=bivoutputmerge.o
BIVM_BINNAME=bivoutputmerge

TRANSP_OBJ_FILES=transposer.o
TRANSP_BINNAME=transposer
# filter out bin files from library files
LIB_OBJ_FILES:=$(OBJ_C_FILES) $(OBJ_CXX_FILES)
LIB_OBJ_FILES:=$(filter-out traceprocessor.o, $(LIB_OBJ_FILES))
LIB_OBJ_FILES:=$(filter-out bivoutputmerge.o, $(LIB_OBJ_FILES))
LIB_OBJ_FILES:=$(filter-out transposer.o, $(LIB_OBJ_FILES))

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	SHAREDFLAG=-shared
	LIBNAME=lib$(LIBNAME_BASE).so
endif
ifeq ($(UNAME_S),Darwin)
	SHAREDFLAG=-dynamiclib
	LIBNAME=lib$(LIBNAME_BASE).dylib
	CXX=/usr/local/bin/g++-10
endif
all: $(OBJ_CXX_FILES) $(OBJ_C_FILES) library traceprocessor bivoutputmerge transposer

%.o: %.cpp
	$(CXX) -c ${INCL} ${CXXFLAGS} -o $@ $<
%.o: %.c
	$(CC) -c ${INCL} ${CFLAGS} -o $@ $<
library: $(LIB_OBJ_FILES)
	$(CXX) ${SHAREDFLAG} ${CXXFLAGS} ${LIB_OBJ_FILES} -o ${LIBNAME}
traceprocessor: $(TPROC_OBJ_FILES) library
	$(CXX) ${CXXFLAGS} ${TPROC_OBJ_FILES} -o ${TPROC_BINNAME} ${LIBS}
bivoutputmerge: $(BIVM_OBJ_FILES) library
	$(CXX) ${CXXFLAGS} ${BIVM_OBJ_FILES} -o ${BIVM_BINNAME} ${LIBS}
transposer: $(TRANSP_OBJ_FILES) library
	$(CXX) ${CXXFLAGS} ${TRANSP_OBJ_FILES} -o ${TRANSP_BINNAME} ${LIBS}


install:
	mkdir -p $(PREFIX)/bin $(PREFIX)/lib
	cp $(TRANSP_BINNAME) $(BIVM_BINNAME) $(TPROC_BINNAME) $(PREFIX)/bin
	cp $(LIBNAME) $(PREFIX)/lib
clean: 
	rm $(OBJ_CXX_FILES) $(OBJ_C_FILES) $(LIBNAME) $(TPROC_BINNAME) $(BIVM_BINNAME) $(TRANSP_BINNAME)

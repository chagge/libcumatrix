CC=gcc
CXX=g++
CFLAGS=
NVCC=nvcc -arch=sm_21 -w

CUDA_ROOT=/usr/local/cuda

EXECUTABLES=
EXAMPLE_PROGRAM=benchmark example1 example2
OBJ=obj/device_matrix.o obj/cuda_memory_manager.o
 
.PHONY: debug all o3 ctags clean
all: libs $(EXECUTABLES) $(EXAMPLE_PROGRAM) ctags

o3: CFLAGS+=-O3
o3: all
debug: CFLAGS+=-g -DDEBUG
debug: all

libs: $(OBJ) lib/libcumatrix.a

lib/libcumatrix.a: $(OBJ)
	rm -f $@
	ar rcs $@ $^
	ranlib $@

vpath %.h include/
vpath %.cpp src/
vpath %.cu src/

INCLUDE= -I include/\
	 -I ../math_ext/

LIBRARY= -lcuda -lcublas -lcudart
LIBRARY_PATH=-L$(CUDA_ROOT)/lib64/
CUDA_INCLUDE=$(INCLUDE) \
	     -I $(CUDA_ROOT)/samples/common/inc/ \
	     -I $(CUDA_ROOT)/include

CPPFLAGS= -std=c++0x $(CFLAGS) $(INCLUDE)

benchmark: $(OBJ) benchmark.cpp
	$(CXX) $(CPPFLAGS) $(CUDA_INCLUDE) -o $@ $^ $(LIBRARY_PATH) $(LIBRARY)
example1: $(OBJ) example1.cpp
	$(CXX) $(CPPFLAGS) $(CUDA_INCLUDE) -o $@ $^ $(LIBRARY_PATH) $(LIBRARY)
example2: $(OBJ) example2.cu
	$(NVCC) $(NVCCFLAGS) $(CFLAGS) $(CUDA_INCLUDE) -o $@ $^ $(LIBRARY_PATH) $(LIBRARY)
# +==============================+
# +===== Other Phony Target =====+
# +==============================+
obj/%.o: %.cpp include/%.h
	$(CXX) $(CPPFLAGS) $(CUDA_INCLUDE) -o $@ -c $<

obj/%.o: %.cu
	$(NVCC) $(NVCCFLAGS) $(CFLAGS) $(CUDA_INCLUDE) -o $@ -c $<

obj/%.d: %.cpp
	@$(CXX) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,obj/\1.o $@ : ,g' < $@.$$$$ > $@;\
	rm -f $@.$$$$

-include $(addprefix obj/,$(subst .cpp,.d,$(SOURCES)))

ctags:
	@if command -v ctags >/dev/null 2>&1; then ctags -R --langmap=C:+.cu *; fi
clean:
	rm -rf $(EXECUTABLES) $(EXAMPLE_PROGRAM) obj/* lib/*.a

JSONLITE2_SRC=$(SRC)/jsonlite2
TARGET=femto
OBJ=obj
OBJD=objd
SRC=src
TESTS=testing


export C_INCLUDE_PATH=$(JSONLITE2_SRC)
export LIBRARY_PATH=$(JSONLITE2_SRC)

CC=gcc
WARN=-Wall -Wextra -Wpedantic -Wconversion -Wunused-variable -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wdouble-promotion -Waggregate-return -Wunused-function -Wunused-result
CDEFFLAGS=-std=c99 $(WARN) -municode -D UNICODE -D _UNICODE -D PROFILING_ENABLE=1
CFLAGS=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -D NDEBUG
CFLAGSD=-g -Og
LIB=-static -ljsonlite2


default: debug

$(OBJ):
	mkdir $@
$(OBJD):
	mkdir $@
$(TESTS)/bin:
	mkdir $@


srcs = $(wildcard $(SRC)/*.c)
srcs := $(subst $(SRC)/,,$(srcs))

RSC_OBJ=$(wildcard $(SRC)/*.rc)
RSC_OBJ:=$(RSC_OBJ:%.rc=%.rc.o)

RSC_OBJ_R=$(RSC_OBJ:$(SRC)/%=$(OBJ)/%)
RSC_OBJ_D=$(RSC_OBJ:$(SRC)/%=$(OBJD)/%)

tests = $(wildcard $(TESTS)/*.c)
tests := $(subst $(TESTS)/,,$(tests))
TESTBINS = $(tests:%.c=$(TESTS)/bin/%)

jsonlite2_lib = $(JSONLITE2_SRC)/libjsonlite2.a


bulk_srcs = $(wildcard $(SRC)/bulk/*.c)
bulk_srcs += $(jsonlite2_obj)


objs_d = $(srcs:%=$(OBJD)/%.o)
objs_r = $(srcs:%=$(OBJ)/%.o)

objs_d += $(jsonlite2_obj)
objs_d += $(RSC_OBJ_D)

objs_r += $(jsonlite2_obj)
objs_r += $(RSC_OBJ_R)

objs_test = $(subst $(OBJD)/main.c.o,,$(objs_d))

$(jsonlite2_lib): $(JSONLITE2_SRC)
	$(MAKE) -C $(JSONLITE2_SRC)


$(OBJ)/%.c.o: $(SRC)/%.c $(OBJ)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CFLAGS)

$(OBJD)/%.c.o: $(SRC)/%.c $(OBJD)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CFLAGSD)

$(OBJ)/%.rc.o: $(SRC)/%.rc $(OBJ)
	windres -i $< -o $@ -D NDEBUG

$(OBJD)/%.rc.o: $(SRC)/%.rc $(OBJD)
	windres -i $< -o $@ -D _DEBUG



debug_impl: $(objs_d)
	$(CC) $^ -o deb$(TARGET).exe $(CDEFFLAGS) $(CFLAGSD) $(LIB)
debug: $(jsonlite2_lib) debug_impl


bulkd_impl: $(bulk_srcs) $(RSC_OBJ_D)
	$(CC) $^ -o deb$(TARGET).exe $(CDEFFLAGS) $(CFLAGSD) $(LIB)
bulkd: $(jsonlite2_lib) bulkd_impl

release_impl: $(objs_r)
	$(CC) $^ -o $(TARGET).exe $(CDEFFLAGS) $(CFLAGS) $(LIB)
release: $(jsonlite2_lib) release_impl


bulkr_impl: $(bulk_srcs) $(RSC_OBJ_R)
	$(CC) $^ -o $(TARGET).exe $(CDEFFLAGS) $(CFLAGS) $(LIB)
bulkr: $(jsonlite2_lib) bulkr_impl

$(TESTS)/bin/%: $(TESTS)/%.c $(objs_test)
	$(CC) $(CDEFFLAGS) $^ -o $@ $(LIB)

test: $(jsonlite2_lib) $(TESTS)/bin $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done

clean:
	rm -r -f $(OBJ)
	rm -r -f $(OBJD)
	rm -f $(TARGET).exe
	rm -f deb$(TARGET).exe
	rm -r -f $(TESTS)/bin
	$(MAKE) -C $(JSONLITE2_SRC) clean

.PHONY: bulkd_impl bulkd bulkr_impl bulkr debug_impl debug release_impl release test clean

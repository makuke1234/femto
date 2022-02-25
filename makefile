JSONLITE2_SRC=$(SRC)/jsonlite2
TARGET=femto
OBJ=obj
OBJD=objd
SRC=src
TESTS=testing


export C_INCLUDE_PATH=$(JSONLITE2_SRC)
export LIBRARY_PATH=$(JSONLITE2_SRC)

CC=gcc
WARN=-Wall -Wextra -Wpedantic -Wconversion -Wunused-variable -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wdouble-promotion -Waggregate-return -Winline -Wunused-function -Wunused-result
CDEFFLAGS=-std=c99 $(WARN) -municode -D UNICODE -D _UNICODE
CFLAGS=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -D NDEBUG
CFLAGSD=-g -O0 -D PROFILING_ENABLE=1
LIB=-ljsonlite2


default: debug

$(OBJ):
	mkdir $@
$(OBJD):
	mkdir $@
$(TESTS)/bin:
	mkdir $@


srcs = $(wildcard $(SRC)/*.c)
srcs := $(subst $(SRC)/,,$(srcs))

tests = $(wildcard $(TESTS)/*.c)
tests := $(subst $(TESTS)/,,$(tests))
TESTBINS = $(tests:%.c=$(TESTS)/bin/%)

jsonlite2_srcs = $(wildcard $(JSONLITE2_SRC)/*.c)
jsonlite2_srcs := $(subst $(JSONLITE2_SRC)/,,$(jsonlite2_srcs))

jsonlite2_objs = $(jsonlite2_srcs:%=$(OBJ)/%.j.o)

jsonlite2_lib = $(JSONLITE2_SRC)/libjsonlite2.a


bulk_srcs = $(wildcard $(SRC)/bulk/*.c)
bulk_srcs += $(jsonlite2_obj)


objs_d = $(srcs:%=$(OBJD)/%.o)
objs_r = $(srcs:%=$(OBJ)/%.o)

objs_d += $(jsonlite2_obj)
objs_r += $(jsonlite2_obj)

objs_test = $(subst $(OBJD)/main.c.o,,$(objs_d))


$(OBJ)/%.c.o: $(SRC)/%.c $(OBJ) $(jsonlite2_lib)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CFLAGS)

$(OBJD)/%.c.o: $(SRC)/%.c $(OBJD) $(jsonlite2_lib)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CFLAGSD)


$(OBJ)/%.c.j.o: $(JSONLITE2_SRC)/%.c $(OBJ)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CFLAGS)

$(jsonlite2_lib): $(jsonlite2_objs)
	ar rcs $@ $^

debug: $(objs_d)
	$(CC) $^ -o deb$(TARGET).exe $(CDEFFLAGS) $(CFLAGSD) $(LIB)

bulkd: $(jsonlite2_lib) bulkd_impl
bulkd_impl: $(bulk_srcs)
	$(CC) $^ -o deb$(TARGET).exe $(CDEFFLAGS) $(CFLAGSD) $(LIB)

release: $(objs_r)
	$(CC) $^ -o $(TARGET).exe $(CDEFFLAGS) $(CFLAGS) $(LIB)

bulkr: $(jsonlite2_lib) bulkr_impl
bulkr_impl: $(bulk_srcs)
	$(CC) $^ -o $(TARGET).exe $(CDEFFLAGS) $(CFLAGS) $(LIB)

$(TESTS)/bin/%: $(TESTS)/%.c $(objs_test)
	$(CC) $(CDEFFLAGS) $^ -o $@ $(LIB)

test: $(jsonlite2_lib) $(TESTS)/bin $(TESTBINS)
	for test in $(TESTBINS) ; do ./$$test ; done

clean:
	rm -r -f $(OBJ)
	rm -r -f $(OBJD)
	rm -f $(TARGET).exe
	rm -f deb$(TARGET).exe
	rm -f $(jsonlite2_lib)
	rm -r -f $(TESTS)/bin

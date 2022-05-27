OBJ=obj
TARGET=jsonlite2

CC=gcc
CDEFFLAGS=-std=c99 -Wall -Wextra -Wpedantic -Wconversion
CFLAGS=-O3 -Wl,--strip-all,--build-id=none,--gc-sections -fno-ident -D NDEBUG
CDEBFLAGS=-g -O0 -D _DEBUG

SRCFILES=$(wildcard *.c)

RELOBJFILES=$(SRCFILES:%.c=%.c.o)
RELOBJFILES:=$(RELOBJFILES:%=$(OBJ)/%)

DEBOBJFILES=$(SRCFILES:%.c=%.c.d.o)
DEBOBJFILES:=$(DEBOBJFILES:%=$(OBJ)/%)

default: all
all: clean release
.PHONY: all


rel: release
deb: debug

$(OBJ):
	mkdir $(OBJ)

$(OBJ)/%.c.o: %.c $(OBJ)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CFLAGS)
$(OBJ)/%.c.d.o: %.c $(OBJ)
	$(CC) -c $< -o $@ $(CDEFFLAGS) $(CDEFFLAGS)



release: $(RELOBJFILES)
	ar rcs lib$(TARGET).a $^
	$(CC) $^ -shared -o $(TARGET).dll $(CFLAGS) -Wl,--out-implib=lib$(TARGET).dll.a

debug: $(DEBOBJFILES)
	ar rcs libdeb$(TARGET).a $^
	$(CC) $^ -shared -o deb$(TARGET).dll -Wl,--out-implib=libdeb$(TARGET).dll.a


clean:
	rm -r -f $(OBJ)
	rm -f *$(TARGET).a
	rm -f *$(TARGET).dll
	rm -f *$(TARGET).dll.a

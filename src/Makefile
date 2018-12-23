CFLAGS  := -pipe -Wall -Wextra -D_FORTIFY_SOURCE=0
LDFLAGS := -L.
LDLIBS  := -lcoro
OS      := $(shell uname -s)
TARGET  := pipe fib

.PHONY: all clean

ifdef M32
  CFLAGS  += -m32
  LDFLAGS += -m32
endif

ifdef DEBUG
  CFLAGS += -O0 -g
else
  CFLAGS += -O2 -DNDEBUG
  ifneq "$(OS)" "Darwin"
    LDFLAGS += -s
  endif
endif

all: $(TARGET)

coroutine.o: coroutine.c coroutine.h
libcoro.a: libcoro.a(coroutine.o)

pipe.o: pipe.c libcoro.a
pipe:

fib.o: fib.c libcoro.a
fib:

clean:
	rm -f *.[oa] $(TARGET)


LDLIBS = -lm
OBJS = rt.o src/bmp.o src/image.o src/camera.o src/utils/pvect.o src/utils/alloc.o src/sphere.o src/phong.o src/utils/refcnt.o src/scene.o src/triangle.o
DEPS = $(OBJS:.o=.d)
BIN = rt

CPPFLAGS = -MMD -D_GNU_SOURCE -iquote includes/
CFLAGS ?= -Wall -Wextra -pedantic --std=c99

all: $(BIN)

$(BIN): $(OBJS)

debug: CFLAGS += -O0 -g3 -fsanitize=address
debug: LDLIBS += -fsanitize=address
debug: all

-include $(DEPS)

clean:
	$(RM) $(OBJS)

.PHONY: all clean

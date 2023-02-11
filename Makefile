
CC      := gcc
PKGCONF := pkg-config
CFLAGS  := -g -Wall -pthread -O3
LDFLAGS := -g -pthread
OBJS    := main.o cpu_65c02.o cpu_ccu3000.o ui.o
PKGS    := sdl2 SDL2_image

CFLAGS  += $(shell $(PKGCONF) --cflags $(PKGS))
LDFLAGS += $(shell $(PKGCONF) $(EXTRA_PKGFLAGS) --libs $(PKGS))

all: sim

sim: $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c Makefile
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) $(CFLAGS) -MM $< -o $(@:.o=.d)

clean:
	rm -f *.o *.d sim

-include $(OBJS:.o=.d)


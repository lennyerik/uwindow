CFLAGS=-Wall -Wextra -Werror --std=c17 -pedantic
LDFLAGS=-lwayland-client -lwayland-egl -lEGL -lGL

SRCS=$(wildcard src/*.c)
OBJS=$(SRCS:.c=.o)

XDG_SHELL_XML=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
XDG_SHELL_CSRC=src/xdg-shell-protocol.c
XDG_SHELL_OBJ=$(XDG_SHELL_CSRC:.c=.o)
XDG_SHELL_HEADER=src/xdg-shell-client-protocol.h


all: $(XDG_SHELL_HEADER) libuwindow.a


# Library
libuwindow.a: $(XDG_SHELL_OBJ) $(OBJS)
	$(AR) rcs $@ $^


# Compiling a .o file from a .c file
%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<


# Generating the Wayland XDG definitions from the xml
$(XDG_SHELL_CSRC): $(XDG_SHELL_XML)
	wayland-scanner private-code < $< > $@

$(XDG_SHELL_HEADER): $(XDG_SHELL_XML)
	wayland-scanner client-header < $< > $@


docs: docs/html

.PHONY: docs/html
docs/html:
	cd docs && doxygen


.PHONY: clean
clean:
	$(RM) $(XDG_SHELL_CSRC) $(XDG_SHELL_HEADER) $(XDG_SHELL_OBJ) $(OBJS) libuwindow.a
	$(RM) -r docs/html


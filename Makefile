CC=clang
CFLAGS=-Wall -Wextra -Werror --std=c17 -pedantic
LIBS=-lwayland-client -lwayland-egl -lEGL -lGL

SRCS=main.c window.c error.c xdg-shell-protocol.c

XDG_SHELL_XML=/usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml

all: uwindow_test

xdg-shell-protocol.c: $(XDG_SHELL_XML)
	wayland-scanner private-code < $^ > xdg-shell-protocol.c
	wayland-scanner client-header < $^ > xdg-shell-client-protocol.h

uwindow_test: $(SRCS)
	$(CC) $(CFLAGS) $(LIBS) -o uwindow_test $^

.PHONY: clean
clean:
	$(RM) xdg-shell-protocol.c xdg-shell-client-protocol.h uwindow_test


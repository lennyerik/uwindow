CFLAGS=-Wall -Wextra -Werror --std=c17 -pedantic
LIBS=-lwayland-client -lwayland-egl -lEGL -lGL -L../../ -luwindow

simple: main.c
	$(CC) $(CFLAGS) -o simple $^ $(LIBS)

.PHONY: clean
clean:
	$(RM) simple


CC = gcc
APP = stw
# git tag as source 
VERSION := $(shell git describe --tags --abbrev=0 2>/dev/null | sed 's/^v//' || echo "dev")
PREFIX ?= /usr/local

BINDIR = $(PREFIX)/bin
DATADIR = $(PREFIX)/share
APPDIR = $(DATADIR)/applications
ICONDIR = $(DATADIR)/icons/hicolor/scalable/apps

SRCS = main.c
OBJS = $(SRCS:.c=.o)

GTK_CFLAGS := $(shell pkg-config --cflags gtk4)
GTK_LIBS := $(shell pkg-config --libs gtk4)
CFLAGS_COMMON = -std=c2x -Wall -Wextra -DAPP_VERSION=\"$(VERSION)\" $(GTK_CFLAGS)
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -O0
CFLAGS_REL = $(CFLAGS_COMMON) -O2 -DNDEBUG
CFLAGS = $(CFLAGS_DEBUG) 
# pkg-config covers this from gtk libs
LDFLAGS =
LDLIBS = $(GTK_LIBS)

.PHONY: all debug release install uninstall clean

all: debug

debug: CFLAGS = $(CFLAGS_DEBUG)
debug: $(APP)

release: CFLAGS = $(CFLAGS_REL)
release: $(APP)

# .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(APP): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

install: release
	install -Dm755 $(APP) $(DESTDIR)$(BINDIR)/$(APP)
	strip $(DESTDIR)$(BINDIR)/$(APP)
	install -Dm644 data/$(APP).desktop $(DESTDIR)$(APPDIR)/$(APP).desktop
	install -Dm644 data/$(APP).svg $(DESTDIR)$(ICONDIR)/$(APP).svg
	@gtk-update-icon-cache -f -t $(DESTDIR)$(DATADIR)/icons/hicolor/ 2>/dev/null || true
	@echo "status: installed $(APP) $(VERSION) to $(DESTDIR)$(PREFIX)."

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(APP)
	rm -f $(DESTDIR)$(APPDIR)/$(APP).desktop
	rm -f $(DESTDIR)$(ICONDIR)/$(APP).svg
	@echo "status: uninstalled $(APP) successfully."

clean:
	rm -f $(OBJS) $(APP)

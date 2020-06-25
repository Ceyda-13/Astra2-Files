BINDIR = /usr/game/share/bin
BIN = $(BINDIR)/game
RAN_LIB = 0
INCDIR = -I../include -I/usr/include -I/usr/local/include
LIBDIR = -L../lib/$(PLATFORM)/ -L/usr/lib$(PLATFORM)/ -L/usr/local/lib$(PLATFORM)
LIBS = -lsql -lthecore -llua -lmd
LIBS += -lcryptopp

# DevIL
LIBS += -lIL
# LIBS += -ljasper -lpng -ltiff -ljbig -lmng -llzma -llcms -ljpeg

# MySQL
INCDIR += -I/usr/local/include/mysql
LIBDIR += -L/usr/local/lib$(PLATFORM)/mysql/
LIBS += -lmysqlclient

include ../MakeConfig

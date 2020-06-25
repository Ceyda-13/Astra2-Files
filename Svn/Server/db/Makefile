BINDIR = /usr/game/share/bin
BIN = $(BINDIR)/db
RAN_LIB = 0
INCDIR = -I../include -I/usr/include -I/usr/local/include
LIBDIR = -L../lib/$(PLATFORM)/ -L/usr/lib$(PLATFORM)/ -L/usr/local/lib$(PLATFORM)
LIBS = -lthecore -lsql -lm
# LIBS += -ll -lstdc++

# MySQL
INCDIR += -I/usr/local/include/mysql
LIBDIR += -L/usr/local/lib$(PLATFORM)/mysql/
LIBS += -lmysqlclient

include ../MakeConfig

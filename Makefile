TARGET=ptrac2root

SOURCES=$(wildcard *.cpp)
HEADERS=$(wildcard *.h)

CFLAGS+=$(shell root-config --cflags)
CFLAGS+=-Og -g -ggdb -I. -Wall -Wextra
LIBS+=$(shell root-config --libs)

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS) Makefile
	g++ $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES) $(LIBS)

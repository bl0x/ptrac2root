TARGET=ptrac2root

SOURCES=$(wildcard *.cpp)
OBJECTS:=$(SOURCES:.cpp=.o)
HEADERS=$(wildcard *.h)

# ROOT dictionary files
DICTHEADERS:=PtracStep.h
DICTIONARIES:=$(DICTHEADERS:%.h=%_dict.cc)
RDICT_FILES:=$(DICTHEADERS:%.h=%_dict_rdict.pcm)
DICTIONARY_OBJECTS:=$(DICTIONARIES:.cc=.o)

CFLAGS+=$(shell root-config --cflags)
CFLAGS+=-Og -g -ggdb -I. -Wall -Wextra
LIBS+=$(shell root-config --libs)

all: $(TARGET)

$(TARGET): $(OBJECTS) $(DICTIONARY_OBJECTS) Makefile
	@echo " LINK $@" && \
	$(CXX) $(LDFLAGS) -o $@ $(filter-out Makefile, $^) $(LIBS)

%.d: %.cpp
	@echo " DEPS $@" && \
	set -e; rm -f $@; \
		$(CXX) -MM $(CFLAGS) $< > $@.$$$$; \
		sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
		rm -f $@.$$$$

%.o: %.cpp Makefile
	@echo "  CC  $@" && \
	$(CXX) $(CFLAGS) -c -o $@ $<

# ROOT dictionary creation
%_dict.cc: %.h LinkDef.h
	@echo " ROOT $@" && \
	rootcling -f $@ -c $^

%_dict.o: %_dict.cc
	@echo " DICT $@" && \
	$(CXX) $(CFLAGS) -c -o $@ $<

.SECONDARY: $(RDICT_FILES)


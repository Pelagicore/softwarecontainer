SOURCES = ../../src/generators.c test_generators.cpp
CXXFLAGS += `pkg-config --cflags --libs glib-2.0`
CXXFLAGS += -I ../../src

all: test_generators

test_generators: $(SOURCES)
	$(CXX) $(SOURCES) -o test_generators $(CXXFLAGS)

clean:
	rm test_generators

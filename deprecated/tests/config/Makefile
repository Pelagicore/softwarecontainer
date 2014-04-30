SOURCES = ../../src/config.cpp test_config.cpp
CXXFLAGS += `pkg-config --cflags --libs glib-2.0 jansson`
CXXFLAGS += -I ../../src

all: test_config

test_config: $(SOURCES)
	$(CXX) $(SOURCES) -o test_config $(CXXFLAGS)

clean:
	rm test_config

all: test_config

test_config:
	$(CC) ../../src/config.c test_config.c -o test_config -I ../../src `pkg-config --cflags --libs glib-2.0` `pkg-config --cflags --libs jansson`

clean:
	rm test_config

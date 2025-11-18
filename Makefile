CC = cc
CFLAGS = -I./src
TARGET = muffing
DIST = ./build

ifeq ($(DEBUG),true)
	CFLAGS += -pg
	TARGET = muffingd
endif

build: distdir handler.o main.o server.o sig.o handlers_http.o utils_path.o utils_time.o utils_url.o
	$(CC) -o $(DIST)/$(TARGET) $(DIST)/handler.o $(DIST)/main.o \
		$(DIST)/server.o $(DIST)/sig.o $(DIST)/handlers_http.o $(DIST)/utils_path.o $(DIST)/utils_time.o $(DIST)/utils_url.o

clean:
	rm -r $(DIST)

distdir:
	mkdir -p $(DIST)

handler.o: src/handler.c
	$(CC) -c $(CFLAGS) src/handler.c -o $(DIST)/handler.o

main.o: src/main.c
	$(CC) -c $(CFLAGS) src/main.c -o $(DIST)/main.o
server.o: src/server.c
	$(CC) -c $(CFLAGS) src/server.c -o $(DIST)/server.o

sig.o: src/sig.c
	$(CC) -c $(CFLAGS) src/sig.c -o $(DIST)/sig.o

handlers_http.o: src/handlers/http.c
	$(CC) -c $(CFLAGS) src/handlers/http.c -o $(DIST)/handlers_http.o

utils_path.o:
	$(CC) -c $(CFLAGS) src/utils/path.c -o $(DIST)/utils_path.o

utils_time.o:
	$(CC) -c $(CFLAGS) src/utils/time.c -o $(DIST)/utils_time.o

utils_url.o:
	$(CC) -c $(CFLAGS) src/utils/url.c -o $(DIST)/utils_url.o

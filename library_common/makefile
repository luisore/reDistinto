RM=rm -rf
CC=gcc

C_SRCS=$(shell find . -iname "*.c" | tr '\n' ' ')
H_SRCS=$(shell find . -iname "*.h" | tr '\n' ' ')
OBJS=$(C_SRCS:./%.c=%.o)

LIBS = -lcommons -lreadline -lcrypto -lpthread 

# Clean and compile .so
all: build/liblibrary_commons.so

create-dirs:
	mkdir -p build

build/library/%.o: library/%.c
	$(CC) -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"

build/liblibrary_commons.so: create-dirs $(OBJS)
	$(CC) -shared -o "build/liblibrary_commons.so" $(OBJS) $(LIBS)

# Add debug parameters and compile
debug: CC += -DDEBUG -g
debug: all

# Clean build files
clean:
	$(RM) build

install: all
	cp -u build/liblibrary_commons.so /usr/lib
	cp --parents -u $(H_SRCS) /usr/include

uninstall:
	rm -f /usr/lib/liblibrary_commons.so

.PHONY: all create-dirs clean install uninstall

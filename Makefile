LIBS  = -lslog -lpthread -lm -lavcodec -lavfilter
INCLUDES = -I /usr/include/x86_64-linux-gnu
CFLAGS = -Wall -Wextra
CRELEASEFLAGS = -O2
CDEBUGFLAGS = -g

BUILD_DIR = build
BIN_DIR = bin

# Should be equivalent to your list of C files, if you don't build selectively
SRCS=$(wildcard *.c)
HEADERS=$(wildcard *.h)
OBJS=$(addprefix ${BUILD_DIR}/,$(SRCS:.c=.o))


all: release

.PHONY: release
release:
	@echo Building release
	@$(MAKE) $(MAKEFILE) link

.PHONY: debug
debug:
	@echo Building debug
	@$(MAKE) $(MAKEFILE) DEBUG="1" link

link: ${OBJS}
	@mkdir -p ${BIN_DIR}
	@echo Linking
ifndef DEBUG
	$(CC) $^ -o ${BIN_DIR}/mpeg7MatchRelease.elf ${LIBS}
else
	$(CC) $^ -o ${BIN_DIR}/mpeg7MatchDebug.elf ${LIBS}
endif

compile: ${OBJS}
	@mkdir -p $(BUILD_DIR)
	@echo Compiling

${BUILD_DIR}/%o:%c
	# DO NOT change the options order
ifndef DEBUG
	$(CC) -c ${CFLAGS} ${CRELEASEFLAGS} $< -o $@ ${INCLUDES}
else
	$(CC) -c ${CFLAGS} ${CDEBUGFLAGS} $< -o $@ ${INCLUDES}
endif

.PHONY: clean
clean:
	@echo "Cleaning files"
	@$(RM) -r $(DEST_DIR)
	@echo "Cleaning finished"

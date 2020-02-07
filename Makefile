LIBS  = -lslog -lpthread -lm -lavcodec -lavfilter
INCLUDES = -I src/includes -I /usr/include/x86_64-linux-gnu
CFLAGS = -Wall -Wextra -std=c11
CRELEASEFLAGS = -O1
CDEBUGFLAGS = -g3

BUILD_DIR = build
BIN_DIR = bin

# Should be equivalent to your list of C files, if you don't build selectively
SRCS=$(wildcard src/*.c)
HEADERS=$(wildcard src/includes/*.h)
OBJS=$(addprefix ${BUILD_DIR}/,$(SRCS:src/%.c=%.o))


all: release

.PHONY: buildDirs
buildDirs:
	@mkdir -p ${BIN_DIR}
	@mkdir -p $(BUILD_DIR)

.PHONY: release
release: buildDirs
	@echo Building release
	@$(MAKE) $(MAKEFILE) link

.PHONY: debug
debug: buildDirs
	@echo Building debug
	@$(MAKE) $(MAKEFILE) DEBUG="1" link

.PHONY: optiDebug
optiDebug: buildDirs
	@echo Building debug
	@$(MAKE) $(MAKEFILE) DEBUG="1" OPTIDEBUG="1" link

link: ${OBJS}
	@echo Linking
ifndef DEBUG
	$(CC) $^ -o ${BIN_DIR}/mpeg7DupesRelease.elf ${LIBS}
else

ifndef OPTIDEBUG
	$(CC) $^ -o ${BIN_DIR}/mpeg7DupesOptiDebug.elf ${LIBS}
else
	$(CC) $^ -o ${BIN_DIR}/mpeg7DupesOptiDebug.elf ${LIBS}
endif

endif

compile: ${OBJS}
	@echo Compiling

${BUILD_DIR}/%o: src/%c
	# DO NOT change the options order
ifndef DEBUG
	$(CC) -c  ${CFLAGS} ${CRELEASEFLAGS} $< -o $@ ${INCLUDES}
else

ifndef OPTIDEBUG
	$(CC) -c -D DEBUG ${CFLAGS} ${CDEBUGFLAGS} -Og $< -o $@ ${INCLUDES}
else
	$(CC) -c -D DEBUG ${CFLAGS} ${CDEBUGFLAGS} $< -o $@ ${INCLUDES}
endif

endif

.PHONY: clean
clean:
	@echo "Cleaning files"
	@$(RM) -r $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaning finished"

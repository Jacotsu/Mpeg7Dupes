LIBS  = -lslog -lpthread -lm -lavcodec -lavfilter
DEBUG_LIBS = -lasan
INCLUDES = -I src/includes -I /usr/include/x86_64-linux-gnu
CFLAGS = -Wall -Wextra -std=c11 -fopenmp
CRELEASEFLAGS = -O2 -march=native
CDEBUGFLAGS = -g3 -fsanitize=address
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src

# Should be equivalent to your list of C files, if you don't build selectively
SRCS=$(shell find src/ -type f -name '*.c')
HEADERS=$(shell find src/includes -type f -name '*.h')
OBJS=$(addprefix ${BUILD_DIR}/,$(SRCS:src/%.c=%.o))

EXE_PATH ?= "${BIN_DIR}/mpeg7Dupes.elf"

all: release

.PHONY: buildDirs
buildDirs:
	@mkdir -p ${BIN_DIR}
	@mkdir -p $(BUILD_DIR)

.PHONY: release
release:
	@echo Building release
	@$(MAKE) $(MAKEFILE) \
		EXE_PATH="${BIN_DIR}/mpeg7Dupes.elf" link

.PHONY: debug
debug:
	@echo Building debug
	@$(MAKE) $(MAKEFILE) DEBUG="1"\
		EXE_PATH="${BIN_DIR}/mpeg7DupesDebug.elf" link

.PHONY: optiDebug
optiDebug:
	@echo Building optimized debug
	@$(MAKE) $(MAKEFILE) DEBUG="1" OPTIDEBUG="1" \
		EXE_PATH="${BIN_DIR}/mpeg7DupesOptiDebug.elf" link

.PHONY: nonVerboseDebug
nonVerboseDebug:
	@echo Building non verbose debug
	@$(MAKE) $(MAKEFILE) DEBUG="1" NVDEBUG="1" \
		EXE_PATH="${BIN_DIR}/mpeg7DupesNVDebug.elf" link

.PHONY: compile
compile: buildDirs ${HEADERS} ${OBJS}

.PHONY: clean
clean:
	@echo "Cleaning files"
	@$(RM) -r $(BUILD_DIR) $(BIN_DIR)
	@echo "Cleaning finished"

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@# DO NOT change the options order
	@echo Compiling
ifdef DEBUG
ifdef OPTIDEBUG
	$(CC) -c -D DEBUG ${CFLAGS} ${CDEBUGFLAGS} -Og $< -o $@ ${INCLUDES}
else ifdef NVDEBUG
	$(CC) -c -g3 ${CFLAGS} ${CDEBUGFLAGS} $< -o $@ ${INCLUDES}
else
	$(CC) -c -g3 -D DEBUG ${CFLAGS} ${CDEBUGFLAGS}  $< -o $@ ${INCLUDES}
endif
else
	$(CC) -c  ${CFLAGS} ${CRELEASEFLAGS} $< -O2 -o $@ ${INCLUDES}
endif

link: compile
	@echo Linking
ifdef DEBUG
	$(CC) -g3 -o ${EXE_PATH} ${OBJS} ${DEBUG_LIBS} ${CFLAGS} ${LIBS}
else
	$(CC) -o ${EXE_PATH} ${OBJS} ${CFLAGS} ${LIBS}
endif

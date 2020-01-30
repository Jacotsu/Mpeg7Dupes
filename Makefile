LIBS  = -lslog -lpthread -lm -lavcodec -lavfilter
INCLUDES = -I /usr/include/x86_64-linux-gnu
CFLAGS = -Wall -Wextra
CRELEASEFLAGS = -O2
CDEBUGFLAGS = -g

# Should be equivalent to your list of C files, if you don't build selectively
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

DEST_DIR=build

.PHONY: clean release debug

all: release

release:
	@echo Building release
	$(MAKE) $(MAKEFILE) link

debug:
	@echo Building debug
	@$(MAKE) $(MAKEFILE) DEBUG="" link

link:${DEST_DIR}/${OBJS}
	@echo Linking
	$(CC) $< -o ${DEST_DIR}/mpeg7Match.elf ${LIBS}

${DEST_DIR}/${OBJS}: ${SRCS}
	# DO NOT change the options order
	@mkdir -p $(DEST_DIR)
ifndef DEBUG
	$(CC) -c ${CFLAGS} ${CRELEASEFLAGS} $< -o $@ ${INCLUDES}
else
	$(CC) -c ${CFLAGS} ${CDEBUGFLAGS} $< -o $@ ${INCLUDES}
endif

clean:
	@echo "Cleaning files"
	@$(RM) -r $(DEST_DIR)
	@echo "Cleaning finished"

LIBS  = -lslog -lpthread -lm -lavcodec -lavfilter
INCLUDES = -I /usr/include/x86_64-linux-gnu
#CFLAGS = -Wall -Wextra
CFLAGS =
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

link: compile
	@echo Linking
	#$(CC) ${OBJS} -o ${DEST_DIR}/mpeg7Match.elf ${LIBS}

compile: ${SRCS}
	# DO NOT change the options order
	@mkdir -p $(DEST_DIR)
	@echo $<
	@echo ${DEST_DIR}/$(<:.c=.o)
ifndef DEBUG
	$(CC) -c ${CFLAGS} ${CRELEASEFLAGS} $< -o ${DEST_DIR}/$(<:.c=.o) ${INCLUDES}
else
	$(CC) -c ${CFLAGS} ${CDEBUGFLAGS} $< -o ${DEST_DIR}/$(<:.c=.o) ${INCLUDES}
endif

clean:
	@echo "Cleaning files"
	@$(RM) -r $(DEST_DIR)
	@echo "Cleaning finished"

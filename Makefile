LIBS  = -lslog -lpthread
CFLAGS = -O2 -Wall -g

# Should be equivalent to your list of C files, if you don't build selectively
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

DEST_DIR=build


.PHONY: clean

all: release

release: ${DEST_DIR}/${OBJS}
ifndef DEBUG
	@echo Building release
endif

debug: ${DEST_DIR}/${OBJS}
	@echo Building debug
	@$(MAKE) $(MAKEFILE) DEBUG="${CFLAGS} -g"

${DEST_DIR}/%.o: %.c
	@mkdir -p $(DEST_DIR)
	$(CC) ${CFLAGS} ${DEBUG}  $< -o $@ ${LIBS}

clean:
	@echo "Cleaning files"
	@$(RM) -r $(DEST_DIR)
	@echo "Cleaning finished"

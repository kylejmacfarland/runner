.PHONY: all clean

CFLAGS = -C src/nes.cfg -t nes -Oisr
CC = cl65
SRCDIR = src
LIBDIR = src/lib
OBJDIR = build/obj
OUTPUTDIR = build/bin
OUTPUT = $(OUTPUTDIR)/runner.nes

SRC = $(wildcard $(SRCDIR)/*.c)
SRC += $(wildcard $(SRCDIR)/*.s)

OBJ = $(SRC:.c=.o)
OBJ := $(OBJ:.s=.o)
OBJ := $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -o $@ -c $(CFLAGS) $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	$(CC) -o $@ -c $(CFLAGS) $<

all: $(OUTPUT)

$(OUTPUT): $(OBJ) $(OUTPUTDIR)
	$(CC) -o $(OUTPUT) $(CFLAGS) $(OBJ)

$(OBJ): $(wildcard $(LIBDIR)/*.h $(LIBDIR)/*.sinc) $(OBJDIR)

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OUTPUTDIR):
	@mkdir -p $(OUTPUTDIR)

clean:
	rm -f $(OUTPUT) $(OBJDIR)/*.o

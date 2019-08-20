CC=gcc

ODIR=obj
SDIR=src
IDIR=include
LICHESS=/home/j/Desktop/lichess/lichess-bot-master/engines/

CFLAGS=-I.$(IDIR) -std=c11 -finline-functions -lm

DEPS=$(IDIR)/*

SRC:=$(wildcard $(SDIR)/*)

_OBJ:=$(foreach wrd, $(SRC), $(subst .c,.o,$(wrd)))
OBJ:=$(foreach wrd, $(_OBJ), $(subst $(SDIR),$(ODIR),$(wrd)))
OBJO:=$(foreach wrd, $(OBJ), $(subst .o,O.o,$(wrd)))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%O.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -Ofast

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean optimized lichess all

optimized: $(OBJO)
	$(CC) -o $@ $^ $(CFLAGS) -Ofast $(LIBS)

lichess:
	make optimized
	mv optimized $(LICHESS)

all:
	(mkdir $(ODIR) && make optimized) || make optimized 

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f main
	rm -f optimized
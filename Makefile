CC=gcc

ODIR=obj
SDIR=src
IDIR=include
LICHESS=~/Desktop/lichess/lichess-bot-master/engines/

CFLAGS=-I.$(IDIR) -std=c11 -finline-functions -lm

DEPS=$(IDIR)/*

SRC:=$(wildcard $(SDIR)/*)

_OBJ:=$(foreach wrd, $(SRC), $(subst .c,.o,$(wrd)))
OBJ:=$(foreach wrd, $(_OBJ), $(subst $(SDIR),$(ODIR),$(wrd)))
OBJO:=$(foreach wrd, $(OBJ), $(subst .o,O.o,$(wrd)))
OBJR:=$(foreach wrd, $(OBJ), $(subst .o,R.o,$(wrd)))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%O.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -O3

$(ODIR)/%R.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -Ofast -flto

.PHONY: clean optimized lichess all release

release: $(OBJR)
	$(CC) -o $@ $^ $(CFLAGS) -Ofast -flto $(LIBS)

optimized: $(OBJO)
	$(CC) -o $@ $^ $(CFLAGS) -O3 $(LIBS)

lichess:
	make release
	mv release $(LICHESS)

all:
	(mkdir $(ODIR) && make release) || make release 

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f main
	rm -f optimized
	rm -f release
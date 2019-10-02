CC=gcc

NAME=NoC
ODIR=obj
SDIR=src
IDIR=include
GDIR=gav
LICHESS=~/Desktop/lichess/lichess-bot-master/engines/

CFLAGS= -finline-functions -std=c11 -O3 -flto

DEPS=$(IDIR)/*

SRC:=$(wildcard $(SDIR)/*)

_OBJ:=$(foreach wrd, $(SRC), $(subst .c,.o,$(wrd)))
OBJ:=$(foreach wrd, $(_OBJ), $(subst $(SDIR),$(ODIR),$(wrd)))
OBJO:=$(foreach wrd, $(OBJ), $(subst .o,O.o,$(wrd)))
OBJR:=$(foreach wrd, $(OBJ), $(subst .o,R.o,$(wrd)))

GAVLIB=-L$(GDIR) -lgtb -lpthread -lm


$(ODIR)/%O.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DEBUG

$(ODIR)/%R.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DNDEBUG $(GAVLIB)

.PHONY: clean debug lichess all release

release: $(OBJR)
	$(CC) -o $@ $^ $(CFLAGS) -DNDEBUG $(LIBS) $(GAVLIB)

debug: $(OBJO)
	$(CC) -o $@ $^ $(CFLAGS) -DEBUG $(LIBS) $(GAVLIB)

lichess:
	make release
	mv release $(LICHESS)

all:
	mkdir -p $(ODIR)
	make release
	mv release $(NAME)


clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f debug
	rm -f release
CC=gcc
CPP=g++

NAME=NoC
ODIR=obj
SDIR=src
IDIR=include
GDIR=gav
TRAINER=trainer.cpp
LICHESS=~/Desktop/Chess/lichess-bot-master/engines/

#If the cpu doesn't have popcnt instructions remove -mpopcnt
CFLAGS=-O3 -flto -lm -lpthread -mpopcnt

DEPS=$(IDIR)/*

SRC:=$(wildcard $(SDIR)/*)

_OBJ:=$(foreach wrd, $(SRC), $(subst .c,.o,$(wrd)))
OBJ:=$(foreach wrd, $(_OBJ), $(subst $(SDIR),$(ODIR),$(wrd)))
OBJD:=$(filter-out $(ODIR)/gaviotaO.o, $(foreach wrd, $(OBJ), $(subst .o,O.o,$(wrd))))
OBJA:=$(filter-out $(ODIR)/gaviotaA.o, $(foreach wrd, $(OBJ), $(subst .o,A.o,$(wrd))))
OBJT:=$(filter-out $(ODIR)/gaviotaT.o, $(foreach wrd, $(OBJ), $(subst .o,T.o,$(wrd))))
OBJR:=$(foreach wrd, $(OBJ), $(subst .o,R.o,$(wrd)))

GAVLIB=-L$(GDIR) -lgtb -lpthread -lm


$(ODIR)/%A.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -Wall

$(ODIR)/%T.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DTRAIN -DNDEBUG -DNUSE_TB -lpthread

$(ODIR)/%O.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DDEBUG

$(ODIR)/%R.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DNDEBUG -DUSE_TB $(GAVLIB)

.PHONY: clean debug lichess all release assert train trainer

#GTB will only be used in release

release: $(OBJR)
	$(CC) -o $@ $^ $(CFLAGS) -DNDEBUG -DUSE_TB $(GAVLIB)

debug: $(OBJD)
	$(CC) -o $@ $^ $(CFLAGS) -DDEBUG -DNUSE_TB

assert: $(OBJA)
	$(CC) -o $@ $^ $(CFLAGS) -DNUSE_TB -Wall

train: $(OBJT)
	$(CC) -o $@ $^ $(CFLAGS) -DTRAIN -DNUSE_TB -DNDEBUG -lpthread

lichess:
	make all
	mv $(NAME) $(LICHESS)

trainer:
	$(CPP) $(TRAINER) -O3 -o trainer

all:
	mkdir -p $(ODIR)
	make release
	mv release $(NAME)


clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f debug release train trainer assert


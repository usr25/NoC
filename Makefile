CC=gcc
CPP=g++

NAME=NoC
ODIR=obj
SDIR=src
IDIR=include
GDIR=gav
TRAINER=trainer.cpp
LICHESS=~/Desktop/Chess/lichess-bot-master/engines/

nnue = yes
nnuedebug = no
sparse = yes
gaviota = no
popcnt = yes
profile = no
native = yes
vectorize = no

ifneq ($(NNUE),)
	nnue = yes
endif
ifeq ($(NNUE),no)
	nnue = no
endif

ifneq ($(SPARSE),)
	sparse = yes
endif
ifeq ($(SPARSE),no)
	sparse = no
endif

CFLAGS=-O3 -flto -lm -lpthread

ifeq ($(profile),yes)
	CFLAGS += -pg
endif

ifeq ($(vectorize), yes)
	CFLAGS += -fopt-info-vec -ftree-vectorizer-verbose=5
endif

ifeq ($(native),yes)
	CFLAGS += -march=native
endif

ifeq ($(nnue),yes)
	CFLAGS += -DUSE_NNUE
	ifneq ($(NNUE_PATH),)
		CFLAGS += -DNNUE_PATH=\"$(NNUE_PATH)\"
	endif
	ifeq ($(sparse),yes)
		CFLAGS += -DNNUE_SPARSE
	endif
	ifeq ($(nnuedebug),yes)
		CFLAGS += -DNNUE_DEBUG
	endif
endif

ifeq ($(popcnt),yes)
	CFLAGS += -mpopcnt
endif

ifeq ($(gaviota),yes)
	CFLAGS += $(GAVLIB) -DUSE_TB
endif

DEPS=$(IDIR)/*

SRC:=$(wildcard $(SDIR)/*)

_OBJ:=$(foreach wrd, $(SRC), $(subst .c,.o,$(wrd)))
OBJ:=$(foreach wrd, $(_OBJ), $(subst $(SDIR),$(ODIR),$(wrd)))
OBJD:=$(filter-out $(ODIR)/gaviotaO.o, $(foreach wrd, $(OBJ), $(subst .o,O.o,$(wrd))))
OBJA:=$(filter-out $(ODIR)/gaviotaA.o, $(foreach wrd, $(OBJ), $(subst .o,A.o,$(wrd))))
OBJT:=$(filter-out $(ODIR)/gaviotaT.o, $(foreach wrd, $(OBJ), $(subst .o,T.o,$(wrd))))
OBJR:=$(foreach wrd, $(OBJ), $(subst .o,R.o,$(wrd)))

GAVLIB=-L$(GDIR) -lgtb -lpthread -lm

.PHONY: help clean debug lichess all release assert train trainer

$(ODIR)/%A.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -Wall

$(ODIR)/%T.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DTRAIN -DNDEBUG -DNUSE_TB -lpthread

$(ODIR)/%O.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DDEBUG

$(ODIR)/%R.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -DNDEBUG -DUSE_TB $(GAVLIB)


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

help:
	@echo ""
	@echo "To compile NoC, type: "
	@echo ""
	@echo "make target [NNUE=yes|no] [NNUE_PATH=path] [SPARSE=yes|no]"
	@echo ""
	@echo "Targets:"
	@echo "  debug: Enables asserts and debugging options"
	@echo "  train: To compile the training model, it DOESN'T play chess"
	@echo "  assert: Enables asserts"
	@echo "  release: Without asserts, use this build when playing"
	@echo "  clean: Removes the binaries"

all:
	mkdir -p $(ODIR)
	make release
	mv release $(NAME)


clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
	rm -f debug release train trainer assert

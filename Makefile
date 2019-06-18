CC=gcc

ODIR=obj
SDIR=src
IDIR=include

CFLAGS=-I.$(IDIR) -std=c11

DEPS=$(IDIR)/*

SRC:=$(wildcard $(SDIR)/*)

_OBJ:=$(foreach wrd, $(SRC), $(subst .c,.o,$(wrd)))
OBJ:=$(foreach wrd, $(_OBJ), $(subst $(SDIR),$(ODIR),$(wrd)))

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean optimized

optimized: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -O3 $(LIBS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
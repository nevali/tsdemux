OUT = tsdump

OBJ = tsdump.o stream.o packet.o pid.o table.o pat.o pmt.o cat.o

CFLAGS = -W -Wall -O0 -g

$(OUT): $(OBJ)
	$(CC) -o $(LDFLAGS) $(OUT) $(OBJ) $(LIBS)

clean:
	rm -f $(OUT) $(OBJ)

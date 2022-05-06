DIROBJ := obj/
DIREXE := exec/
DIRHEA := include/
DIRSRC := src/

CFLAGS := -I$(DIRHEA) -c -Wall
LDLIBS := -lpthread -lrt -lm -lX11
CC := mpicc

all : dirs compile

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

compile: $(DIROBJ)pract2.o
	$(CC) -o $(DIREXE)pract2 $^ $(LDLIBS)

$(DIROBJ)%.o: $(DIRSRC)%.c
	$(CC) $(CFLAGS) $^ -o $@

test:
	mpirun -n 1 --hostfile mpi_config ./$(DIREXE)pract2

clean : 
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRHEA)*~ $(DIRSRC)*~
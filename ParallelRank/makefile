EXECS=random_rank
MPICC?=mpicc

all: ${EXECS}

# -c compiles into an object file
tmpi_rank.o: tmpi_rank.c
	${MPICC} -c tmpi_rank.c

# links the files together
random_rank: tmpi_rank.o random_rank.c
	${MPICC} -g -o random_rank random_rank.c tmpi_rank.o

clean:
	rm -f ${EXECS} *.o
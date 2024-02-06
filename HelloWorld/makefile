
# EXECS is the executable files that will be created
EXECS=mpi_hello_world
# variable MPICC if it is not already defined then it will be given value mpicc
MPICC?=mpicc

# all represents the files that the makefile aims to create
all: ${EXECS}

# actually compiling the code using mpicc
mpi_hello_world: mpi_hello_world.c
	${MPICC} -o mpi_hello_world mpi_hello_world.c

# cleanup
clean:
	rm -f ${EXECS}
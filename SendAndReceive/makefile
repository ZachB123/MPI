EXECS=send_recv ping_pong ring
MPICC?=mpicc

all: ${EXECS}

# comiles the send_recv
send_recv: send_recv.c
	${MPICC} -o send_recv send_recv.c

# compiles ping pong
ping_pong: ping_pong.c
	${MPICC} -o ping_pong ping_pong.c

# compiles ring
ring: ring.c
	${MPICC} -o ring ring.c

clean:
	rm -f ${EXECS}
EIGEN=/Users/hans/eigen

main: main.cpp Master.cpp Slave.cpp
	mpicxx -I $(EIGEN) -o eigenMpi main.cpp Master.cpp Slave.cpp


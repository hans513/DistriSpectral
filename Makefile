

UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
	EIGEN=/home/hans/eigen
endif
ifeq ($(UNAME_S),Darwin)
	EIGEN=/Users/hans/eigen
endif


main: main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp
	mpicxx -I $(EIGEN) -std=c++11 -o eigenMpi main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp


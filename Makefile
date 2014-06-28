
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
	EIGEN=/home/hans/eigen
endif
ifeq ($(UNAME_S),Darwin)
	EIGEN=/Users/hans/eigen
    CHCXX=/Users/hans/mpichInstall/bin/mpicxx
endif


main: main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp
	mpicxx -I $(EIGEN) -I $(EIGEN)/unsupported -std=c++11 -o eigenMpi main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp

ch: main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp
	$(CHCXX) -I $(EIGEN) -I $(EIGEN)/unsupported -std=c++11 -o eigenMpich main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp

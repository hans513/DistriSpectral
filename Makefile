
UNAME_S:=$(shell uname -s)
ifeq ($(UNAME_S),Linux)
	EIGEN=/home/hans/eigen
	CHCXX=/usr/bin/mpicxx.mpich
	CHEXE=/usr/bin/mpiexec.mpich
endif
ifeq ($(UNAME_S),Darwin)
	EIGEN=/Users/hans/eigen
	CHCXX=/Users/hans/mpichInstall/bin/mpicxx
	CHEXE=/Users/hans/mpichInstall/bin/mpiexec
endif

main: main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp
	mpicxx -I $(EIGEN) -I $(EIGEN)/unsupported -std=c++11 -o eigenMpi main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp

ch: main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp
	$(CHCXX) -I $(EIGEN) -I $(EIGEN)/unsupported -std=c++11 -o eigenMpich main.cpp Master.cpp Slave.cpp DataGenerator.cpp Task.cpp Logic.cpp Fastfood.cpp

run: eigenMpi
	mpiexec -np $(N) ./eigenMpi

chrun: eigenMpich
	$(CHEXE) -np $(N) ./eigenMpich

remoteRun:
	cp eigenMpi ~/mpi_shared/
	mpiexec --host ubuntu1,ubuntu2 -n 3 mpi_shared/eigenMpi

remoteChrun:
	cp eigenMpich ~/mpi_shared/
	$(CHEXE) --host ubuntu1,ubuntu2 -n 3 mpi_shared/eigenMpi

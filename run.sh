mpicxx -I /home/hans/eigen/ eigenMpi.cpp -o eigenmpi
sudo mv eigenmpi /home/hans/mpi_shared/
time mpiexec --host ubuntu1,ubuntu2,ubuntu3 -n 4 /home/hans/mpi_shared/eigenmpi

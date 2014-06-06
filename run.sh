#mpicxx -I /home/hans/eigen/ eigenMpi.cpp -o eigenmpi
git pull
make
sudo mv eigenMpi /home/hans/mpi_shared/
time mpiexec --host ubuntu1,ubuntu2 -n 3 /home/hans/mpi_shared/eigenMpi

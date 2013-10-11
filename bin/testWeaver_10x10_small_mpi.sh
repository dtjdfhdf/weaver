# create a new project/image workspace
echo "Creating new workspace ..."
mpirun -np 10 ./weaver_mpi -o /home/r6singh/data/images/weaverTest_10x10_small.tif -m /home/r6singh/temp -n 10x10 -p pieceFile_10x10_small_linux.txt -w -mr 256x256


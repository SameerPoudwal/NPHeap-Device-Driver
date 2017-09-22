sudo rmmod npheap
cd kernel_module
sudo make clean
make
sudo make install
cd ..
cd library
sudo make clean
make
sudo make install
cd ..
cd benchmark
sudo make clean
sudo make benchmark
sudo make validate
cd ..
sudo insmod kernel_module/npheap.ko
sudo chmod 777 /dev/npheap
./benchmark/benchmark 256 8192 10
cat *.log > trace
sort -n -k 3 trace > sorted_trace
./benchmark/validate 256 8192 < sorted_trace
rm -f *.log
sudo rmmod npheap

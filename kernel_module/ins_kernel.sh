sudo rmmod npheap
make
sudo make install
sudo insmod npheap.ko
sudo dmesg -c
sudo chmod 777 /dev/npheap

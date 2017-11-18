all: benchmark validate

benchmark: benchmark.c 
	$(CC) -g -O0 test1_1.c -o test11 -I/usr/local/include -lnpheap
	
validate: validate.c 
	$(CC) -g -O0 validate.c -o validate -lnpheap
	
clean:
	rm -f benchmark validate 

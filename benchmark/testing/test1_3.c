#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <npheap.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    int i=0,number_of_processes = 1, number_of_objects=1024, max_size_of_objects = 8192 ,j; 
    int a;
    int pid;
    int size;
    char data[8192];
    char filename[256];
    char *mapped_data;
    int devfd;
    unsigned long long msec_time;
    FILE *fp;
    struct timeval current_time;
    if(argc < 3)
    {
        fprintf(stderr, "Usage: %s number_of_objects max_size_of_objects number_of_processes\n",argv[0]);
        exit(1);
    }
    number_of_objects = atoi(argv[1]);
    max_size_of_objects = atoi(argv[2]);
    number_of_processes = atoi(argv[3]);
    devfd = open("/dev/npheap",O_RDWR);
    if(devfd < 0)
    {
        fprintf(stderr, "Device open failed");
        exit(1);
    }
    srand((int)time(NULL)+(int)getpid());
    // Writing to objects
    for(i=0;i<(number_of_processes-1) && pid != 0;i++)
    {
        pid=fork();
    }
    sprintf(filename,"npheap.%d.log",(int)getpid());
    fp = fopen(filename,"w");
    for(i = 0; i < number_of_objects; i++)
    {
        npheap_lock(devfd,i);
	if (i == 1)
	{
		size = 4080;
		mapped_data = (char *)npheap_alloc(devfd,i,size);
	}
	else if(i==2)
	{
		size = 4107;
		mapped_data1 = (char *)npheap_alloc(devfd,i,size);
	}
	else 
	{
		size = rand() % max_size_of_objects;
		mapped_datan = (char *)npheap_alloc(devfd,i,size)
	}
	if(!mapped_datan)
        {
            fprintf(stderr,"Failed in npheap_alloc()\n");
            exit(1);
        }
   		if (pid==0)
		{
			sleep(1);
		}
	if (i == 1)
		printf("mapped data: %lx\n", (unsigned long) mapped_data);
        else if (i==2)
	{
        	printf("mapped data1: %lx\n", (unsigned long) mapped_data1);
	}
	else
	{
		printf("mapped datan: %lx\n", (unsigned long) mapped_datan);
	}
        if (i==1)
        if(!mapped_data)
	{
		sprintf(mapped_data, "%s%d", mapped_data, 5);
	}
        else if(i==2)
	{
		sprintf(mapped_data1, "%s%d", mapped_data1, 10);
	}
	else
	{
		sprintf(mapped_datan, "%s%d", mapped_datan, 4080);
	}
     	npheap_unlock(devfd,i);
    }
    close(devfd);
    if(pid != 0)
        wait(NULL);
    return 0;
 }

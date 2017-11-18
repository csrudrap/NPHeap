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
#include <errno.h>

int main(int argc, char *argv[])
{
    int i=0,number_of_processes = 1, number_of_objects=1024, max_size_of_objects = 8192 ,j; 
    int a;
    int pid = -1;
    int size;
    char data[8192];
    char filename[256];
    char *mapped_data;
    char *mapped_data2;
    char *mapped_data3;
    char *mapped_data4;
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
        printf("PID: %d\n", getpid());
    }
    sprintf(filename,"npheap.%d.log",(int)getpid());
    fp = fopen(filename,"w");
    for(i = 1; i <= number_of_objects; i++)
    {
        npheap_lock(devfd,i);
        /*do 
        {
            size = rand() % max_size_of_objects;
        }
        while(size == 0 || size < 10);*/
		if (i == 1)
		{
			size = 600;
		} 
		else if (i == 2)
		{
			size = 5600;
		}
        else if (i == 3)
        {
            size = 7222;
        }
		else
		{
			size = 600;
		}
        mapped_data = (char *)npheap_alloc(devfd,i,size);
        if(!mapped_data)
        {
            fprintf(stderr,"Failed in npheap_alloc()\n");
            exit(1);
        }
		/*if (pid == 0)
		{
			sleep(1);
		}*/
        /*if (i == 1)
		printf("mapped data: %lx\n", (unsigned long) mapped_data);
        else if(i == 2)
        printf("mapped data2: %lx\n", (unsigned long) mapped_data2);
        else
        {
            printf("mapped data3: %lx\n", (unsigned long) mapped_data2);
        }*/
        
        //sprintf(mapped_data, "%s%d", mapped_data, 5);
//        memset(mapped_data, 0, 4096);
        //a = rand()+1;
        gettimeofday(&current_time, NULL);
        for(j = strlen(mapped_data); j < size-10; j=strlen(mapped_data))
        {
            sprintf(mapped_data,"%s%d",mapped_data,8);
        }
        //printf("This part.");
	    printf("MAPPED DATA IS %s\n", mapped_data);
        fprintf(fp,"S\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
        npheap_unlock(devfd,i);
    }

    npheap_lock(devfd, 4);
    npheap_delete(devfd, 4);
    npheap_unlock(devfd, 4);
    npheap_lock(devfd, 3);
    npheap_delete(devfd, 3);
    npheap_unlock(devfd, 3);
    npheap_lock(devfd, 15);
    npheap_delete(devfd,15);
    npheap_unlock(devfd, 15);
    npheap_lock(devfd, 25);
    npheap_delete(devfd,25);
    npheap_unlock(devfd,25);
    npheap_lock(devfd, 3);
    int size2 = 7600;
    mapped_data2 = (char *)npheap_alloc(devfd, 3, size2);
    for(j = strlen(mapped_data2); j < size2-10; j=strlen(mapped_data2))
    {
        sprintf(mapped_data2,"%s%d",mapped_data2,9);
    }
    npheap_unlock(devfd, 3);
    //npheap_lock(devfd, 4);
    //npheap_alloc(devfd, 4, 6000);
    //npheap_unlock(devfd, 4);
    printf("Delete done. Strlen for mapped data2 is %d\n", strlen(mapped_data2));
/*    
    // try delete something
    i = rand()%256;
    npheap_lock(devfd,i);
    npheap_delete(devfd,i);
    fprintf(fp,"D\t%d\t%ld\t%d\t%lu\t%s\n",pid,current_time.tv_sec * 1000000 + current_time.tv_usec,i,strlen(mapped_data),mapped_data);
    */
    close(devfd);
	printf("MAPPED DATA2 IS %s\n", mapped_data2);
	//printf("MAPPED DATA3 IS %s\n", mapped_data3);
    if(pid != 0)
    {
        while (pid = waitpid(-1, NULL, 0))
        {
            if (errno == ECHILD)
            {
                break;
            }
        }
   }
    printf("PID %d finished.\n", getpid());
    return 0;
}


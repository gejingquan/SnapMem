#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <sched.h>
#include <time.h>


#define SNAPMEM_ALLOC 0
#define SNAPMEM_LIGHT_OPEN 1
#define SNAPMEM_LIGHT_CLOSE 3
#define SNAPMEM_FREE 4


uint64_t arm_v8_get_timing(void)
{
	struct timespec time = {0, 0};
	clock_gettime(CLOCK_REALTIME, &time);
	return time.tv_nsec;
	
	// asm volatile ("DSB ISH");
	// asm volatile ("ISB");	
	// uint64_t res;	
	// asm volatile("mrs %0, cntvct_el0" : "=r" (res) :: "memory");
	// return res;			
}


int SnapMem_alloc(int fd)
{
	int ret;
	ret = ioctl(fd,SNAPMEM_ALLOC,0);
	return ret;
}

int SnapMem_light_open(int fd)
{
	int ret;
	ret = ioctl(fd,SNAPMEM_LIGHT_OPEN,0);
	return ret;
}


int SnapMem_light_close(int fd)
{
	int ret;
	ret = ioctl(fd,SNAPMEM_LIGHT_CLOSE,0);
	return ret;
}


int SnapMem_read(int fd, unsigned long* read_buffer,unsigned long offset_size)
{
	int ret;
	ret = read(fd,read_buffer,offset_size);
	return ret;
}

int SnapMem_write(int fd, unsigned long* write_buffer,unsigned long offset_size)
{
	int ret;
	ret = write(fd,write_buffer,offset_size);
	return ret;
}

int SnapMem_free(int fd)
{
	int ret;
	ret = ioctl(fd,SNAPMEM_FREE,0);
	return ret;
}




int main(int argc, const char **argv) {
	int  fd,i,j,ret;
	FILE *fd2;
	int  snapmem_step, snapmem_size;
	unsigned long read_buffer[1024],write_buffer[1024],offset_size;
	uint64_t start_time, end_time;
	char * malloc_addr, snapmem_user_addr;
	uint64_t time_delay, mean_time, sum_time;
		
	fd=open("/dev/SnapMem_device", O_RDWR);
	fd2=fopen("/home/ubuntu/snapmem-test.txt","w");
	
	
	offset_size=0x000800;
	fprintf(fd2,"offset_size=%lx\n",offset_size);
	snapmem_size   =  (offset_size & 0xFFF)*4;	
	snapmem_step  = (snapmem_size*64/0x2000); 
	fprintf(fd2,"snapmem_step=%d\n",snapmem_step);



//-----------------------------test SnapMem's function-----------------------------------		

	ret = SnapMem_alloc(fd);
	fprintf(fd2,"SnapMem_alloc = %d\n",ret);	
	ret=SnapMem_light_open(fd);
	fprintf(fd2,"SnapMem_light_open = %d\n",ret);
	
	for(i=0;i<1024;i++)
		write_buffer[i]=(i)*(i+1)*(i+2);
	fprintf(fd2,"SnapMem_write input:\n");
	for(i=0;i<16;i++)
		fprintf(fd2,"write_buffer[%d]=%016lx\n",snapmem_step*i,write_buffer[snapmem_step*i]);			
	ret = SnapMem_write(fd, write_buffer, offset_size);
	fprintf(fd2,"SnapMem_write = %d\n",ret);	

	for(i=0;i<1024;i++)
		read_buffer[i]=(i+3)*(i+4)*(i+5);
	fprintf(fd2,"Before SnapMem_read:\n");
	for(i=0;i<16;i++)
		fprintf(fd2,"read_buffer[%d]=%016lx\n",snapmem_step*i,read_buffer[snapmem_step*i]);
	ret = SnapMem_read(fd, read_buffer, offset_size);
	fprintf(fd2,"SnapMem_read = %d\n",ret);	
	fprintf(fd2,"After SnapMem_read:\n");
	for(i=0;i<16;i++){
		fprintf(fd2,"read_buffer[%d]=%016lx\n",snapmem_step*i,read_buffer[snapmem_step*i]);
	}
	
	for(i=0;i<1024;i++)
		write_buffer[i]=(i+6)*(i+7)*(i+8);
	fprintf(fd2,"SnapMem_write input:\n");
	for(i=0;i<16;i++)
		fprintf(fd2,"write_buffer[%d]=%016lx\n",snapmem_step*i,write_buffer[snapmem_step*i]);		
	ret = SnapMem_write(fd, write_buffer, offset_size);
	fprintf(fd2,"SnapMem_write = %d\n",ret);	
	
	for(i=0;i<1024;i++)
		read_buffer[i]=(i+9)*(i+10)*(i+11);	
	fprintf(fd2,"Before SnapMem_read:\n");
	for(i=0;i<16;i++)
		fprintf(fd2,"read_buffer[%d]=%016lx\n",snapmem_step*i,read_buffer[snapmem_step*i]);	
	ret = SnapMem_read(fd, read_buffer, offset_size);
	fprintf(fd2,"SnapMem_read = %d\n",ret);		
	fprintf(fd2,"After SnapMem_read:\n");
	for(i=0;i<16;i++){
		fprintf(fd2,"read_buffer[%d]=%016lx\n",snapmem_step*i,read_buffer[snapmem_step*i]);
	}

	ret=SnapMem_light_close(fd);
	fprintf(fd2,"SnapMem_light_close = %d\n",ret);	
	ret = SnapMem_free(fd);
	fprintf(fd2,"SnapMem_free = %d\n",ret);






//--------------------------test SnapMem_alloc and malloc--------------------------------	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_alloc(fd);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		ret = SnapMem_free(fd);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_alloc mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(128);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(128) mean_time=%ld\n",mean_time);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(256);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(256) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(512);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(512) mean_time=%ld\n",mean_time);
		
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(1024);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(1024) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(2048);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(4096);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(4096) mean_time=%ld\n",mean_time);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		malloc_addr = (char *)malloc(8192);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
		free(malloc_addr);
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"malloc(8192) mean_time=%ld\n",mean_time);	






//----------------------------------test memcpy------------------------------------------

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 128);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(128) mean_time=%ld\n",mean_time);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 256);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(256) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 512);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(512) mean_time=%ld\n",mean_time);
		
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 1024);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(1024) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 2048);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 4096);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(4096) mean_time=%ld\n",mean_time);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		memcpy(write_buffer, read_buffer, 8192);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"memcpy(8192) mean_time=%ld\n",mean_time);




//---------------------------------test SnapMem_light write------------------------------------


	ret = SnapMem_alloc(fd);
	fprintf(fd2,"SnapMem_alloc = %d\n",ret);
	ret=SnapMem_light_open(fd);
	fprintf(fd2,"SnapMem_light_open = %d\n",ret);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000020);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(128) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000040);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(256) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000080);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(512) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000100);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(1024) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000200);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000400);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(4096) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000800);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light write(8192) mean_time=%ld\n",mean_time);

	ret=SnapMem_light_close(fd);
	fprintf(fd2,"SnapMem_light_close = %d\n",ret);
	ret = SnapMem_free(fd);
	fprintf(fd2,"SnapMem_free = %d\n",ret);




//---------------------------------test SnapMem_light read------------------------------------

	ret = SnapMem_alloc(fd);
	fprintf(fd2,"SnapMem_alloc = %d\n",ret);
	ret=SnapMem_light_open(fd);
	fprintf(fd2,"SnapMem_light_open = %d\n",ret);


	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000020);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(128) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000040);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(256) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000080);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(512) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000100);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(1024) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000200);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000400);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(4096) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000800);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_light read(8192) mean_time=%ld\n",mean_time);

	ret=SnapMem_light_close(fd);
	fprintf(fd2,"SnapMem_light_close = %d\n",ret);
	
	ret = SnapMem_free(fd);
	fprintf(fd2,"SnapMem_free = %d\n",ret);
	




//-------------------------------test SnapMem_write--------------------------------------

	ret = SnapMem_alloc(fd);
	fprintf(fd2,"SnapMem_alloc = %d\n",ret);

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000020);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(128) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000040);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(256) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000080);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(512) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000100);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(1024) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000200);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000400);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(4096) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_write(fd, write_buffer, 0x000800);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_write(8192) mean_time=%ld\n",mean_time);

	ret = SnapMem_free(fd);
	fprintf(fd2,"SnapMem_free = %d\n",ret);





//-------------------------------test SnapMem_read---------------------------------------

	ret = SnapMem_alloc(fd);
	fprintf(fd2,"SnapMem_alloc = %d\n",ret);

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000020);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(128) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000040);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(256) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000080);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(512) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000100);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(1024) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000200);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000400);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(4096) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_read(fd, write_buffer, 0x000800);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10;
	fprintf(fd2,"SnapMem_read(8192) mean_time=%ld\n",mean_time);

	ret = SnapMem_free(fd);
	fprintf(fd2,"SnapMem_free = %d\n",ret);











//-------------------------------test SnapMem_free and free------------------------------

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		ret = SnapMem_alloc(fd);		
		start_time  = arm_v8_get_timing();	
		ret = SnapMem_free(fd);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"SnapMem_free mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(128);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(128) mean_time=%ld\n",mean_time);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(256);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(256) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(512);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(512) mean_time=%ld\n",mean_time);
		
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(1024);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(1024) mean_time=%ld\n",mean_time);

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(2048);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(2048) mean_time=%ld\n",mean_time);	

	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(4096);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(4096) mean_time=%ld\n",mean_time);
	
	sum_time = 0;
	for(i=0;i<10000;i++){
		start_time  = arm_v8_get_timing();
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time;
		malloc_addr = (char *)malloc(8192);		
		start_time  = arm_v8_get_timing();	
		free(malloc_addr);		
		end_time    = arm_v8_get_timing();
		time_delay  = end_time - start_time - time_delay;
		sum_time    = sum_time + time_delay; 
	}
	mean_time = sum_time/10000;
	fprintf(fd2,"free(8192) mean_time=%ld\n",mean_time);	




	close(fd);	
	return (0);
	
}
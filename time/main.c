#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

void show_time(void)
{
	struct timeval time_val;

	gettimeofday(&time_val, NULL);

	printf("%s:%d time %ld:%ld\n", __FILE__, __LINE__, time_val.tv_sec, time_val.tv_usec);
}

int main(int argc, char *argv[])
{
	show_time();

	return 0;	
}

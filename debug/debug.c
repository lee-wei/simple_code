#include <stdio.h>

#define		DEBUG		1

#if		DEBUG
#define		MY_PRINTF(args...)	printf("%s==%d:\t", __FILE__, __LINE__);	\
					printf(args);
#else
#define		MY_PRINTF(args...)
#endif


int main(int argc, char *argv[])
{
	int x = 10;

	MY_PRINTF("%d\n", x);

	return 0;
}

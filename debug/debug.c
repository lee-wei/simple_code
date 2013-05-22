#include <stdio.h>
#include "debug.h"				

#undef		DEBUG	
#define		DEBUG		1

int main(int argc, char *argv[])
{
	int x = 10;

	MY_PRINTF("%d\n", x);

	return 0;
}

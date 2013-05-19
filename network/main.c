#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
	struct hostent * host;
	char str[32] = {0};

	host = gethostbyname("www.sina.com");
	if (host)
		printf("ip:%s\n", inet_ntop(host->h_addrtype, host->h_addr_list[0], str, sizeof(str)));
	else
		printf("gethostbyname fail\n");

	host = gethostbyname("192.168.1.128");
        if (host)
                printf("ip:%s\n", inet_ntop(host->h_addrtype, host->h_addr_list[0], str, sizeof(str)));
	else
		printf("gethostbyname fail\n");

	return 0;

}

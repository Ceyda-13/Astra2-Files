#include <stdio.h>
#include <stdlib.h>

void WriteVersion()
{
#ifndef _WIN32
	FILE* fp(fopen("ver.txt", "w"));

	if (nullptr != fp)
	{
		fprintf(fp, "%s@%s:%s\n", __USER__, __HOSTNAME__, __PWD__);
		fclose(fp);
	}
	else
	{
		fprintf(stderr, "cannot open ver.txt\n");
		exit(0);
	}
#endif
}


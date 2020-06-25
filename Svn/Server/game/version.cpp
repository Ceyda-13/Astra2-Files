#include <stdio.h>

void WriteVersion()
{
#ifndef _WIN32
	FILE* fp = fopen("ver.txt", "w");

	if (fp)
	{
		fprintf(fp, "%s@%s:%s\n", __USER__, __HOSTNAME__, __PWD__);
		fclose(fp);
	}
#endif
}


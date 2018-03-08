#include <stdio.h>
#include <unistd.h>

#include "zing.h"

void zing(void)
{
	printf("Hello, %s!\n", getlogin());
}

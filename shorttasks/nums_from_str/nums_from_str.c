#include <string.h>

void strdelnum(char* s)
{
	size_t i, j;
	for (i = 0; s[i] != '\0'; i++)
	{
		if (isdigit(s[i])) 
		{
			for (j = i+1; isdigit(s[j]); j++);

		}
	}
}

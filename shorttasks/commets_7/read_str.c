#define BUFFADD 100
#define ERROR_MEM -1
ssize_t read_str(FILE* f, char** strp) 
{
	char *buf = NULL;
	size_t size = 0;
	size_t mem = 0;
	char *tmp = NULL;
	int sym = 0;
	while(1)
	{
		sym = fgetc(f);
		if (sym == EOF) break;
		if (size == mem)
		{
			tmp = (char*)realloc(buf, size + BUFFADD);
			if (tmp == NULL) return ERROR_MEM;
			buf = tmp;
			mem += BUFFADD;
		}
		buf[size] = (char) sym;
		size++;
	}
	tmp = (char*)realloc(buf, size + 1);
	if (tmp == NULL) return ERROR_MEM;
	buf = tmp;
	buf[size] = '\0';
	*strp = buf;
	return size;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>
#include <time.h>

#define SNADD 3
#define BUFADD 200
#define NEWBUF {NULL, 0, 0}

struct config 
{
	struct termios orig_termios;
 	struct termios raw;
 	int started;
};



typedef struct point
{
	int x;
	int y;
} point;

struct field
{
	char **grid;
	int width;
	int height;
	point food;
	int score;
	int highscore;
};

struct snake
{
	point *body;
	int len;
	int mem;
	point dir;
	int alive;
};

struct field F;
struct snake S;
struct config E;

/*TERMINAL*/
void disable_raw_mode();

int init_modes()
{
  if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    return -1;

  atexit(disable_raw_mode);

  E.raw = E.orig_termios;
  E.raw.c_iflag &= ~(BRKINT | IXON | ICRNL | INPCK | ISTRIP);
  E.raw.c_oflag &= ~(OPOST);
  E.raw.c_cflag |= (CS8);
  E.raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
  E.raw.c_cc[VMIN] = 0;
  E.raw.c_cc[VTIME] = 1;

  return 0;
}

int enable_raw_mode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.raw) == -1) 
    return -1;
  return 0;
}

void disable_raw_mode()
{
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
    printf("failed to disable raw mode\n");
}

int get_window_size()
{
  struct winsize ws;

  ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
  
  F.width = ws.ws_col / 2;
  F.height = ws.ws_row - 1;

  return 0;
}

/*KEY HANDLING*/
char read_key() 
{
	char c;
	read(STDIN_FILENO, &c, 1);

	if (c == '\x1b') 
	{
		char seq[3];
		if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
		if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';
		if (seq[0] == '[') 
		{
			switch (seq[1]) 
			{
				case 'A': return 'w';
				case 'B': return 's';
				case 'C': return 'd';
				case 'D': return 'a';
			}
		}
		return '\x1b';
	} 
	else 
	{
		return c;
	}
}

void turn(char key) 
{
	switch (key) 
	{
		case 'a':
			if (S.dir.x != 1)
			{
				S.dir.x = -1;
				S.dir.y = 0;
			}
			break;
		case 'd':
			if (S.dir.x != -1)
			{
				S.dir.x = 1;
				S.dir.y = 0;
			}
			break;
		case 'w':
			if (S.dir.y != 1)
			{
				S.dir.x = 0;
				S.dir.y = -1;
			}
			break;
		case 's':
			if (S.dir.y != -1)
			{
				S.dir.x = 0;
				S.dir.y = 1;
			}
			break;
	}
}

int process_key() 
{
	char c = read_key();
	switch (c) 
	{
		case 'q':
			return -1;
			break;
		case 'w':
		case 's':
		case 'a':
		case 'd':
			E.started = 1;
			turn(c);
			break;
	}

	return 0;
}



/*FIELD*/
int read_highscore()
{
	int i;

	FILE *fp = fopen("highscore.bin", "r");
	if (fp == NULL) 
	{
		fp = fopen("highscore.bin", "w");
		if (fp == NULL)
		{
			printf("failed to open file\n");
			return -1;
		}
		F.highscore = 0;
	}
	else
	{
		if (fread(&i, sizeof(int), 1, fp) != 1)
			return -1;
		F.highscore = i;
	}

	fclose(fp);

	return 0;
}

int write_highscore()
{
	FILE *fp = fopen("highscore.bin", "w");
	if (fp == NULL) return -1;
	
	if (fwrite(&F.score, sizeof(int), 1, fp) != 1) 
		return -1;

	fclose(fp);
	return 0;
}

int make_food()
{
	int x, y;
	do
	{
		x = (int)(random() % (F.width - 2)) + 1;
		y = (int)(random() % (F.height - 2)) + 1;
	} while (F.grid[y][x] != ' ');

	F.food.x = x;
	F.food.y = y;
	F.grid[y][x] = '@';

	return 0;
}

int init_field()
{
	get_window_size();
	char **tmp = (char**)malloc(sizeof(char*)*F.height);
	int j;
	int i;
	for (i = 0; i < F.height; i++)
	{
		tmp[i] = (char*)malloc(F.width);
		for (j = 0; j < F.width; j++)
		{
			if (i == 0 || i == F.height - 1) tmp[i][j] = '-';
			else if (j == 0 || j == F.width - 1) tmp[i][j] = '|';
			else tmp[i][j] = ' ';
		}
	}
	F.grid = tmp;
	F.score = 0;
	if (read_highscore() == -1) return -1;
	make_food();
	return 0;
}

void freefield()
{
	int i;
	for (i = 0; i < F.height; i++)
	{
		free(F.grid[i]);
	}
	free(F.grid);
}

int draw_field()
{
	int i, j;

	write(STDOUT_FILENO, "\x1b[H", 3);
	if (S.alive && E.started)
		printf("\x1b[KSCORE : %5d\t\t\t\t\t\tHIGHSCORE : %5d\r\n", F.score, F.highscore);
	else if (!E.started)
	{
		printf("w,a,s,d or arrows -- move\t\t q -- quit\r\n");
	}
	else
	{
		printf("SCORE : %5d\t\t\tYOU DIED", F.score);
		if (F.score > F.highscore)
			printf("\t\tNEW HIGHSCORE!\x1b[K");
		printf("\r\n");
	}


	for (i = 0; i < F.height; i++)
	{
		for (j = 0; j < F.width; j++)
		{	
			write(STDOUT_FILENO, &F.grid[i][j], 1);
			write(STDOUT_FILENO, " ", 1);
		}
		if (i != F.height - 1) write(STDOUT_FILENO, "\r\n", 2);
	}

	return 0;
}


/*SNAKE*/
int init_snake()
{
	S.body = (point*)malloc(SNADD*sizeof(point));
	S.body[0].x = F.width /2;
	S.body[0].y = F.height /2;
	S.len = 1;
	S.mem = SNADD;
	S.dir.x = 0;
	S.dir.y = 0;
	S.alive = 1;

	return 0;
}

int update_snake()
{
	int j;
	int x, y;
	int ihead = S.len - 1;

	x = S.body[ihead].x + S.dir.x;
	y = S.body[ihead].y + S.dir.y;

	for (j = 0; j < S.len - 1; j++)
	{
		if (F.grid[y][x] == 'o')
		{
			F.grid[S.body[ihead].y][S.body[ihead].x] = 'o';
			F.grid[S.body[0].y][S.body[0].x] = ' ';
			F.grid[y][x] = 'X';
			S.alive = 0;
			return 0;
		}
	}

	if (x == F.food.x && y == F.food.y)
	{
		S.len++;
		if (S.len >= S.mem)
		{
			S.mem += SNADD;
			S.body = (point*)realloc(S.body, S.mem);
			if (S.body == NULL) return -1;
		}

		ihead = S.len - 1;
		S.body[ihead].x = x;
		S.body[ihead].y = y;
		F.grid[y][x] = 'O';
		F.grid[S.body[ihead - 1].y][S.body[ihead - 1].x] = 'o';
		F.score += 100;
		make_food();
	}
	else if (x == 0 || x == F.width - 1 || y == 0 || y == F.height - 1)
	{
		F.grid[y][x] = 'X';
		F.grid[S.body[ihead].y][S.body[ihead].x] = 'o';
		F.grid[S.body[0].y][S.body[0].x] = ' ';
		S.alive = 0;
	}
	else
	{
		F.grid[S.body[ihead].y][S.body[ihead].x] = 'o';
		F.grid[S.body[0].y][S.body[0].x] = ' ';
		for (j = 0; j < ihead; j++)
		{		
			S.body[j] = S.body[j + 1];
		}

		S.body[ihead].x = x;
		S.body[ihead].y = y;
		F.grid[y][x] = 'O';
	}

	return 0;
}

int msleep(long msec)
{
    struct timespec ts;
    int res;

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res);

    return res;
}



int main()
{
	int j;

	init_modes();
	if (init_field() == -1)
	{
		printf("failure\n");
		exit(1);
	}

	write(STDOUT_FILENO, "\x1b[?25l", 6);
	enable_raw_mode();

	for (j = 0; j < F.height; j++)
		printf("\r\n");

	init_snake();
	
	while (1)
	{	
		if (process_key() == -1) break;
		if (S.alive)
		{
			if (update_snake() == -1) printf("\r\nmem error");
			draw_field();
		}
		msleep(50);
	}

	write(STDOUT_FILENO, "\x1b[H", 3);
	for (j = 0; j <= F.height; j++)
	{
		write(STDOUT_FILENO, "\x1b[K",3);
		if (j != F.height) write(STDOUT_FILENO, "\n", 1);
	}
	write(STDOUT_FILENO, "\x1b[H", 3);
	write(STDOUT_FILENO, "\x1b[?25h", 6);
	disable_raw_mode();

	if (F.score > F.highscore)
		write_highscore();

	freefield();
	free(S.body);

	return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <math.h>

#define BUFFADD 250
#define MEM_ERROR -1

struct buffer
{
  char *chars;
  int len;
  int mem;
};

int append(struct buffer *buf, const char* s, int len)
{
  char *tmp = NULL;
  int j;
  int memadd;

  if (buf->len + len > buf->mem)
  {
    memadd = BUFFADD > len ? BUFFADD : len;

    tmp = (char*)realloc(buf->chars, buf->mem + memadd);
    if (tmp == NULL) return MEM_ERROR;

    buf->chars = tmp;
    buf->mem += memadd;
  }

  for (j = 0; j < len; j++)
  {
    buf->chars[j + buf->len] = s[j];
  }

  buf->len += len;

  return 0;
}

typedef struct str
{
	char *chars;
	int length;
} str;

struct arraystr
{
	str *lines;
	int num;
};

void freear(struct arraystr *ar)
{
  if (ar->lines == NULL) 
  {
    ar->num = 0;
    return;
  }

  int j;
  for (j = 0; j < ar->num; j++)
  {
    free(ar->lines[j].chars);
  }
  free(ar->lines);
  
  ar->lines = NULL;
  ar->num = 0;
}

struct arraystr ahelp;



struct config
{
	int width;
	int height;
	char *filename;
	int wrap;
	int numbers;
	int tabwidth;
  int blank;
  int printing;
  int saved;
  struct termios orig_termios;
  struct termios raw;
};

struct config E;
struct arraystr T;
struct pagesInfo I;

/* functions */
int init();

int init_modes();
int enable_raw_mode();
void disable_raw_mode();
int get_window_size();
void sighandler(int sig);

void set_name(char *filename);
int e_read(char *filename);
int e_open(char *filename);
int e_write(char *filename);

void set_wrap(int k);
void set_numbers(int k);
void set_tabwidth(int k);

int print(int start, int end, struct arraystr *ar);

int e_insert_after(str toin, int pos, struct arraystr *ar);
int e_replace_substr(int start, int end, str tofind, str toreplace);
int e_insert_symbol(str *line, char c, int pos);
int e_edit(str *line, char c, int pos);
int e_delr(int start, int end);
int e_delcom(int mode);
void e_help();
void e_exit();

int split(struct arraystr *ar, str s);
int read_command(struct arraystr *ar);

void err_com()
{
  printf("invalid command\n");
}


int main(int argc, char **argv)
{
  init();
	if (argc >= 2)
  {
    e_open(argv[1]);
  }

  struct arraystr ar;
  ar.lines = NULL;
  ar.num = 0;

  while (1)
  {
    freear(&ar);
    printf("editor: ");
    read_command(&ar); 

    if (ar.num < 1)
      err_com();

    /* Setters */
    else if (!strcmp(ar.lines[0].chars, "set"))
    {
      if (ar.num != 3)
      {
        err_com();
        continue;
      }
      if (!strcmp(ar.lines[1].chars, "wrap"))
      {
        if (!strcmp(ar.lines[2].chars, "yes"))
          E.wrap = 1;
        else if (!strcmp(ar.lines[2].chars, "no"))
          E.wrap = 0;
        else 
          err_com();
      }
      else if (!strcmp(ar.lines[1].chars, "numbers"))
      {
        if (!strcmp(ar.lines[2].chars, "yes"))
          E.numbers = 1;
        else if (!strcmp(ar.lines[2].chars, "no"))
          E.numbers = 0;
        else 
          err_com();
      }
      else if (!strcmp(ar.lines[1].chars, "tabwidth"))
      {
        if (atoi(ar.lines[2].chars) == 0)
          err_com();
        else
          E.tabwidth = atoi(ar.lines[2].chars);
      }
      else if (!strcmp(ar.lines[1].chars, "name"))
      {
        if (ar.num > 3)
          err_com();
        else if (ar.lines[2].length == 0)
        {
          free(E.filename);
          E.filename = NULL;
        }
        else
          set_name(ar.lines[2].chars);
      }
      else
        err_com();
    }

    /* Print */
    else if (!strcmp(ar.lines[0].chars, "print"))
    {
      if (ar.num == 1)
        err_com();
      else if (!strcmp(ar.lines[1].chars, "pages"))
      {
        if (ar.num > 2)
          err_com();
        else
          print(1, T.num, &T);
      }
      else if (!strcmp(ar.lines[1].chars, "range"))
      {
        if (ar.num == 2)
          print(1, T.num, &T);
        else if (ar.num == 3)
        {
          if (!atoi(ar.lines[2].chars)) 
            err_com();
          else 
            print(atoi(ar.lines[2].chars), T.num, &T);
        }
        else if (ar.num == 4)
        {
          if (!atoi(ar.lines[2].chars) || !atoi(ar.lines[3].chars))
            err_com();
          else
            print(atoi(ar.lines[2].chars), atoi(ar.lines[3].chars), &T);
        }
        else
          err_com();
      }
      else
      {
        err_com();
      }
    }

    /* File interactions */
    else if (!strcmp(ar.lines[0].chars, "read"))
    {
      if (ar.num != 2)
        err_com();
      else
        e_read(ar.lines[1].chars);
    }
    else if (!strcmp(ar.lines[0].chars, "open"))
    {
      if (ar.num != 2)
        err_com();
      else
        e_open(ar.lines[1].chars);
    }
    else if (!strcmp(ar.lines[0].chars, "write"))
    {
      if (ar.num == 1)
        e_write(NULL);
      else if (ar.num == 2)
        e_write(ar.lines[1].chars);
      else
        err_com();
    }

    /*  */
    else if (!strcmp(ar.lines[0].chars, "edit"))
    {
      if (ar.num != 5 || strcmp(ar.lines[1].chars, "string") || 
              !atoi(ar.lines[2].chars) || !atoi(ar.lines[3].chars) || ar.lines[4].length != 1)
        err_com();
      else e_edit(&T.lines[atoi(ar.lines[2].chars) - 1], *ar.lines[4].chars, atoi(ar.lines[3].chars));
    }
    else if (!strcmp(ar.lines[0].chars, "insert"))
    {
      if (ar.num == 5 && !strcmp(ar.lines[1].chars, "symbol"))
      {
        if (!atoi(ar.lines[2].chars) || !atoi(ar.lines[3].chars) || ar.lines[4].length != 1)
          err_com();
        else 
          e_insert_symbol(&T.lines[atoi(ar.lines[2].chars) - 1], *ar.lines[4].chars, atoi(ar.lines[3].chars));
      }
      else if (!strcmp(ar.lines[1].chars, "after"))
      {
        if (ar.num == 3)
          e_insert_after(ar.lines[2], T.num, &T);
        else if (ar.num == 4)
        {
          if (atoi(ar.lines[2].chars))
          {
            printf("%d\n", atoi(ar.lines[2].chars));
            if (atoi(ar.lines[2].chars) < 0 || atoi(ar.lines[2].chars) > T.num)
              printf("out of bounds\n");
            else
              e_insert_after(ar.lines[3], atoi(ar.lines[2].chars), &T);
          }
          else if (ar.lines[2].length == 1 && ar.lines[2].chars[0] == '0')
            e_insert_after(ar.lines[3], 0, &T);
          else err_com();
        }
        else err_com();
      }
      else
        err_com();
    }
    else if (!strcmp(ar.lines[0].chars, "delete"))
    {
      if (ar.num > 2 && !strcmp(ar.lines[1].chars, "range") && atoi(ar.lines[2].chars))
        if (ar.num == 3)
          e_delr(atoi(ar.lines[2].chars), T.num);
        else if (ar.num == 4)
          if(!atoi(ar.lines[3].chars))
            err_com();
          else
            e_delr(atoi(ar.lines[2].chars), atoi(ar.lines[3].chars));
        else
          err_com();
      else if (ar.num == 3 && !strcmp(ar.lines[1].chars, "comments"))
      {
        if (!strcmp(ar.lines[2].chars, "pascal"))
          e_delcom(1);
        else if (!strcmp(ar.lines[2].chars, "shell"))
          e_delcom(2);
        else if (!strcmp(ar.lines[2].chars, "c"))
          e_delcom(3);
        else if (!strcmp(ar.lines[2].chars, "c++"))
          e_delcom(4);
      }
      else err_com();
        
    }
    else if (!strcmp(ar.lines[0].chars, "replace"))
    {
      if (ar.num < 4 || strcmp(ar.lines[1].chars, "substring"))
        err_com();
      else if (ar.num > 4 && atoi(ar.lines[2].chars))
      {
        if (ar.num == 6 && atoi(ar.lines[3].chars))
          e_replace_substr(atoi(ar.lines[2].chars), atoi(ar.lines[3].chars), ar.lines[4], ar.lines[5]);
        else if (ar.num == 5)
          e_replace_substr(atoi(ar.lines[2].chars), T.num, ar.lines[3], ar.lines[4]);
        else
          err_com();
      }
      else if (ar.num == 4)
        e_replace_substr(1, T.num, ar.lines[2], ar.lines[3]);
      else
        err_com();
    }
    else if (!strcmp(ar.lines[0].chars, "help"))
    {
      if (ar.num > 1)
        err_com();
      else e_help();
    }

    else if (!strcmp(ar.lines[0].chars, "exit"))
    {
      if (ar.num == 2 && !strcmp(ar.lines[1].chars, "force"))
      {
        freear(&ar);
        e_exit();
      }
      else if (ar.num == 1)
        if (E.saved)
        {
          freear(&ar);
          e_exit();
        }
        else 
          printf("progress wasn`t saved, unable to exit\n");
      else
        err_com();
    }

    else
      err_com();

    freear(&ar);
  
  } //while
  
} //main




void init_help()
{
  struct buffer buf;
  buf.chars = NULL;
  buf.len = 0;
  buf.mem = 0;

  append(&buf, "________________________________TETX-ED_____________________________", 69);
  append(&buf, "\n\n(.) - required parameter\n[.] - optional parameter\n\".\" - parameter has to be in quotes", 87);
  append(&buf, "\n\n\nCOMMAND LIST:", 16);
  append(&buf, "\n\n\tTEXT VIEW", 12);
  append(&buf, "\n\n\t\tset wrap (yes/no) -- enables/disables wrapping", 50);
  append(&buf, "\n\n\t\tset numbers (yes/no) -- enables/disables line counter", 57);
  append(&buf, "\n\n\t\tset tabwidth (X) -- sets tabwidth to X", 42);
  append(&buf, "\n\n\t\tprint pages -- show the whole text; while active:\n\t\t\t-press space to show next page",87);
  append(&buf, "\n\t\t\t-press \'<\'/\''>\'' to scroll left/right (only if wrap is off)", 61);
  append(&buf, "\n\n\t\tprint range [X] [Y] -- shows lines in selected boundaries (from X to Y)", 75);
  append(&buf, "\n\t\t\t-if used without Y, prints lines from X to END", 51);
  append(&buf, "\n\t\t\t-if used without X and Y, prints all the lines", 50);
  append(&buf, "\n\n\tLINE INSERT", 14);
  append(&buf, "\n\n\t\tinsert after [X] (\"S\") -- puts string S after line X in text", 65);
  append(&buf, "\n\t\t\t-if used without X, puts S at the end of text", 49);
  append(&buf, "\n\t\t\t-S can be input in several lines like \"\"\"S\"\"\"", 49);
  append(&buf, "\n\n\tLINE EDIT", 12);
  append(&buf, "\n\n\t\tedit string (X) (Y) (C) -- changes symbol in line X in position Y to C", 74);
  append(&buf, "\n\n\t\tinsert symbol (X) (Y) (C) -- inserts symbol C in line X in position Y", 73);
  append(&buf, "\n\n\t\treplace substring [X] [Y] (\"R\") (\"S\") -- replaces sequence R to S in lines", 78);
  append(&buf, "\n\t\t\t-use with X to change lines from X to END", 45);
  append(&buf, "\n\t\t\t-use with X and Y to change lines from X to Y", 49);
  append(&buf, "\n\t\t\t-R can be \'^\'/\'$\' to insert S to beginning/end of lines", 60);
  append(&buf, "\n\n\t\tdelete range (X) [Y] -- removes lines from X to Y (or END if Y is not specified)", 85);
  append(&buf, "\n\n\t\tdelete comments (T) -- removes comments of type T (pascal/c/c++/shell)", 74);
  append(&buf, "\n\n\tTECH COMMANDS", 16);
  append(&buf, "\n\n\t\texit -- closes editor if saved (use \"exit force\" to close even if not saved)", 80);
  append(&buf, "\n\n\t\tread (\"F\") -- reads lines from file F to memory", 51);
  append(&buf, "\n\n\t\topen (\"F\") -- read + remembers F as filename", 48);
  append(&buf, "\n\n\t\twrite [\"F\"] -- writes lines to file F (or to filename if F is not specified)", 80);
  append(&buf, "\n\n\t\tset name (\"S\") -- sets filename to S", 40);

  ahelp.lines = NULL;
  ahelp.num = 0;
  str tmp;
  tmp.chars = buf.chars;
  tmp.length = buf.len;
  e_insert_after(tmp, 0, &ahelp);
  E.saved = 1;
}

int init()
{
  E.tabwidth = 4;
  E.wrap = 0;
  E.numbers = 1;
  E.blank = 5;
  E.filename = NULL;
  E.printing = 0;
  E.saved = 1;
  get_window_size();
  init_modes();
  signal(SIGWINCH, sighandler);
  init_help();

  return 0;
}
/* LOW LEVEL STUFF 
  ________________
*/
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
  
  E.width = ws.ws_col;
  E.height = ws.ws_row - 1;

  return 0;
  
}


/* FILE I/O
  _______________________
*/
ssize_t get_line(FILE *f, char **line, int *eofflag)
{
    char* buff = NULL;
    char *tmp = NULL;
    size_t size = 0;
    size_t mem = 0;
    int symbol = 0;

    *eofflag = 0;

    while(1)
    {
        symbol = fgetc(f);

        if (symbol == '\n' || (symbol == EOF && (*eofflag = 1))) break;

        if (size == mem)
        {
            tmp = (char*) realloc(buff, (size + BUFFADD) * sizeof(char));
            if (tmp == NULL) return MEM_ERROR;

            buff = tmp;
            mem += BUFFADD;
        }

        buff[size++] = (char)symbol;
    }

    tmp = (char*) realloc(buff, (size + 1) * sizeof(char));

    if (tmp == NULL) return MEM_ERROR;

    buff = tmp;
    buff[size] = '\0';

    *line = buff;

    return size;
}

ssize_t read_file(FILE *f, str **lines)
{
    str *buff = NULL;
    str *tmp = NULL;
    str line;
    size_t size = 0;
    size_t mem = 0;
    int eofflag;

    while (1)
    {
        line.length = get_line(f, &line.chars, &eofflag);
        if (line.length < 0) break;

        if (size == mem)
        {
            tmp = (str*) realloc(buff, (size + BUFFADD) * sizeof(str));
            if(tmp == NULL) return MEM_ERROR;

            buff = tmp;
            mem += BUFFADD;
        }

        buff[size++] = line;

        if(eofflag) break;

    }

    tmp = (str*) realloc(buff, size * sizeof(str));
    if(tmp == NULL) return MEM_ERROR;

    *lines = tmp;

    return size;
}

void set_name(char *filename)
{
  free(E.filename);
  E.filename = strdup(filename);
}

int e_read(char *filename)
{
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) 
  {
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
  	 printf("failed to open file\n");
  	 return -1;
    }
  }

  T.num = read_file(fp, &T.lines);

  fclose(fp);

  return 0;
}

int e_open(char *filename)
{
  if (!e_read(filename))
    set_name(filename);
  else return -1;

  return 0;
}

int e_write(char *filename)
{
  if (filename == NULL)
  {
    if (E.filename == NULL)
    {
      printf("file name is not associated\n");
      return 1;
    }
    else
      filename = E.filename;
  }
  else if (E.filename == NULL)
    set_name(filename);

  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    printf("failed to open file\n");
    return 1;
  }

  struct buffer buf;
  int j;
  buf.chars = 0;
  buf.len = 0;
  buf.mem = 0;

  for (j = 0; j < T.num; j++)
  {
    append(&buf, T.lines[j].chars, T.lines[j].length);
    if (j != T.num - 1) append(&buf, "\n", 1);
  }

  if ((int)fwrite(buf.chars, sizeof(char), buf.len, f) != buf.len)
  {
    printf("failed to write\n");
    return 1;
  }

  free(buf.chars);

  E.saved = 1;
  return 0;
}



/* SETTINGS 
  _________
*/
void set_wrap(int k)
{
  E.wrap = k;
}

void set_numbers(int k)
{
  E.numbers = k;
}

void set_tabwidth(int k)
{
  E.tabwidth = k;
}


/* EDITOR OPERATIONS
  __________________
*/

int idxsubstr(str line, str tofind)
{
  int i;
  int j;
  int found;

  if (tofind.length < 1)
  {
    printf("invalid parameter\n");
    return -1;
  }

  for (i = 0; i <= line.length - tofind.length; i++)
  {
    if (line.chars[i] == tofind.chars[0])
    {
      found = 1;
      for (j = 1; j < tofind.length; j++)
      {
        if (tofind.chars[j] != line.chars[i + j])
        {
          found = 0;
          break;
        }
      }
      if (found)
        return i;
    }
  }

  return -1;
}

int e_insert_after(str toin, int pos, struct arraystr *ar)
{
  int j;
  int i;
  int k = 0;
  int idx = 0;
  int strtoadd = 1;
  str* newlines = NULL;
  char *tmp = NULL;

  for (j = 0; j < toin.length; j++)
    if (toin.chars[j] == '\n')
      strtoadd++;

  newlines = malloc((ar->num + strtoadd)*sizeof(str));
  if (newlines == NULL) return MEM_ERROR;

  for (j = 0; j < pos; j++)
  {
    newlines[idx++] = ar->lines[j];
  }

  tmp = malloc(toin.length);
  if (tmp == NULL) return MEM_ERROR;

  for (i = 0; i <= toin.length; i++)
  {
    if (toin.chars[i] == '\n' || i == toin.length)
    {
      tmp[k] = '\0';
      tmp = realloc(tmp, k + 1);
      if (tmp == NULL) return MEM_ERROR;

      newlines[idx].chars = tmp;
      newlines[idx++].length = k;

      k = 0;
      tmp = malloc(toin.length);
      if (tmp == NULL) return MEM_ERROR;
    }
    else
      tmp[k++] = toin.chars[i];
  }

  for(; j < ar->num; j++)
  {
    newlines[idx++] = ar->lines[j];
  }

  if (ar->lines != NULL)
    free(ar->lines);
  ar->lines = newlines;
  ar->num += strtoadd;

  E.saved = 0;
  return strtoadd;
}

int e_replace_substr(int start, int end, str tofind, str toreplace)
{
  int j;
  int i;
  int idx;
  int index;
  int added;
  char *tmp = NULL;
  str toin;


  if (start < 1 || start > T.num || end < 1 || end > T.num)
  {
    printf("out of bounds\n");
    return -1;
  }

  for (j = start - 1; j < end;)
  {
    if (tofind.length <=1 && tofind.chars[0] == '^')
    {
      index = 0;
      tofind.length = 0;
    }
    else if (tofind.length <= 1 && tofind.chars[0] == '$')
    {
      index = T.lines[j].length;
      tofind.length = 0;
    }
    else
      index = idxsubstr(T.lines[j], tofind);

    if (index != -1)
    {
      idx = 0;
      tmp = (char*)malloc(T.lines[j].length - tofind.length + toreplace.length + 1);
      if (tmp == NULL) return MEM_ERROR;

      for (i = 0; i < index; i++)
      {
        tmp[idx++] = T.lines[j].chars[i];
      }
      for (i = 0; i < toreplace.length; i++)
      {
        tmp[idx++] = toreplace.chars[i];
      }
      for (i = index + tofind.length; i < T.lines[j].length; i++)
      {
        tmp[idx++] = T.lines[j].chars[i];
      }
      tmp[idx] = '\0';

      toin.chars = tmp;
      toin.length = T.lines[j].length - tofind.length + toreplace.length;

      e_delr(j+1, j+1);
      added = e_insert_after(toin, j, &T);
      j += added;
      end += (added - 1);
    }
    else j++;
  }

  return 0;
}

int e_insert_symbol(str *line, char c, int pos)
{
  char *tmp = NULL;
  int j;

  if (pos < 0) pos = 0;
  if (pos > line->length) pos = line->length;

  tmp = malloc(line->length + 2);
  if (tmp == NULL) return MEM_ERROR;

  for (j = line->length; j >= pos; j--)
  {
    tmp[j] = line->chars[j - 1];
  }

  tmp[j--] = c;

  while (j >= 0)
  {
    tmp[j] = line->chars[j];
    j--;
  }

  tmp[line->length + 1] = '\0';
  free(line->chars);
  line->chars = tmp;
  line->length += 1;

  E.saved = 0;

  return 0;
}

int e_edit(str *line, char c, int pos)
{

  if (line->length < pos - 1 || pos < 1)
  {
    printf("out of bounds\n");
    return -1;
  }

  line->chars[pos - 1] = c;

  E.saved = 0;

  return 0;
}

int e_delr(int start, int end)
{ 
  int j;
  int idx = 0;
  int len;
  str *tmp = NULL;

  start = start < 1 ? 0 : start - 1;
  end = end > T.num ? T.num : end;

  len = (T.num - (end - start));

  tmp = (str*)malloc(len*sizeof(str));
  if (tmp == NULL) return MEM_ERROR;

  for (j = 0; j < T.num; j++)
  {
    while (j >= start && j < end)
    {
      free(T.lines[j++].chars);
    }
    tmp[idx++] = T.lines[j];
  }

  free(T.lines);
  T.lines = tmp;
  T.num = len;

  E.saved = 0;
  return 0;
}

int e_delcom(int mode)
{
  //1 pascal 2 shell 3 c 4 c++
  int j;
  int i;
  int quotes = 0;
  char *tmp = NULL;
  struct buffer buf;
  buf.chars = NULL;
  buf.len = 0;
  buf.mem = 0;

  for (j = 0; j < T.num; j++)
  {
    for (i = 0; i < T.lines[j].length - 1; i++)
    {
      if ((mode <= 2 && T.lines[j].chars[i] == '\'') || (mode >= 2 && T.lines[j].chars[i] == '\"'))
        quotes = quotes ? 0 : 1;
      if (!quotes)
      {
        if ((mode == 4 && T.lines[j].chars[i] == '/' && T.lines[j].chars[i + 1] == '/')
           || (mode == 2 && T.lines[j].chars[i] == '#'))
        {
          tmp = (char*)realloc(T.lines[j].chars, i + 1);
          if (tmp == NULL) return MEM_ERROR;

          tmp[i] = '\0';
          T.lines[j].chars = tmp;
          T.lines[j].length = i;

          break;
        }
        else if (((mode == 1 && T.lines[j].chars[i] == '(') || (mode == 3 && T.lines[j].chars[i] == '/')) 
                  && T.lines[j].chars[i + 1] == '*')
        {
          int rows;
          int broke = 0;
          int start = i;
          for (rows = 0; j + rows < T.num; rows++)
          {
            while (i < T.lines[j + rows].length - 1)
            {
              if (T.lines[j + rows].chars[i] == '*' && ((mode == 1 && T.lines[j + rows].chars[i + 1] == ')')
                  || (mode == 3 && T.lines[j + rows].chars[i + 1] == '/')))
              {
                broke = 1;
                break;
              }
              i++;
            }
            if (broke) break;
            i = 0;
          }

          if (rows == 0)
          {
            append(&buf, T.lines[j].chars, start);
            append(&buf, &T.lines[j].chars[i + 2], T.lines[j].length - i - 2);
            append(&buf, "\0", 1);

            tmp = (char*)realloc(buf.chars, buf.len);
            if (tmp == NULL) return MEM_ERROR;

            free(T.lines[j].chars);
            T.lines[j].chars = tmp;
            T.lines[j].length = buf.len - 1;

            buf.chars = NULL;
            buf.len = 0;
            buf.mem = 0;
          }
          else
          {
            tmp = (char*)realloc(T.lines[j].chars, start + 1);
            if (tmp == NULL) return MEM_ERROR;

            tmp[start] = '\0';
            T.lines[j].chars = tmp;
            T.lines[j].length = start;
            
            if (T.lines[j + rows].length > i + 2)
              append(&buf, &T.lines[j + rows].chars[i + 2], T.lines[j + rows].length - i - 2);
            append(&buf, "\0", 1);
            tmp = (char*)realloc(buf.chars, buf.len);
            if (tmp == NULL) return MEM_ERROR;

            T.lines[j + rows].chars = tmp;
            T.lines[j + rows].length = buf.len - 1;
            i = 0;


            buf.chars = NULL;
            buf.len = 0;
            buf.mem = 0;
            
            if (rows > 1)
              e_delr(j + 2, j + 1 + rows);
          }
        }

      }
    }
  }
  return 0;
}

void e_help()
{
  int w = E.wrap;
  int n = E.numbers;
  int t = E.tabwidth;
  E.wrap = 0;
  E.numbers = 0;
  E.tabwidth = 4;

  print(1, ahelp.num, &ahelp);

  E.wrap = w;
  E.numbers = n;
  E.tabwidth = t;
}

void e_exit()
{
  freear(&T);
  freear(&ahelp);
  exit(0);
}

/* PRINT 
  ____________
*/
int app_num(struct buffer *buf, int n) 
{
  char *m;
  int k;
  int t = (int)(ceil(log10(n)));
  if (n == 1) t = 1;
  else if (n == 10) t = 2;
  else if (n == 100) t = 3;
  else if (n == 1000) t = 4;

  for (k = t; k < E.blank - 1; k++)
    append(buf, " ", 1);
  m = malloc(t);
  sprintf(m, "%d", n);
  append(buf, m, t);
  free(m);
  append(buf, " ", 1);

  return 0;
}

ssize_t line_insert_tabs(char** line, str current)
{
  int tabs = 0;
  size_t idx = 0;
  int j;
  char *l;

  for (j = 0; j < current.length; j++)
    if (current.chars[j] == '\t') tabs++;

  l = malloc(current.length + tabs*(E.tabwidth - 1) + 1);
  if (l == NULL) return MEM_ERROR;

  for (j = 0; j < current.length; j++)
  {
    if (current.chars[j] == '\t')
    {
      l[idx++] = ' ';
      while (idx % E.tabwidth != 0) l[idx++] = ' ';
    }
    else
    {
      l[idx++] = current.chars[j];
    }
  }

  *line = l;
  return idx;
}

struct pagesInfo
{
  int index;
  int offset;
  int x;
  int pindex;
  int of;
  int bound;
  int max;
};

int page(struct pagesInfo *I, struct arraystr *ar)
{
  int width;
  int rlen;
  char *rend;

  int j;
  int k;
  int idx;
  int numrows;

  struct buffer buf;
  int rows = 0;
  
  buf.chars = NULL;
  buf.len = 0;
  buf.mem = 0;

  width = (E.numbers || E.wrap) ? E.width - E.blank : E.width;
  I->max = 0;

  if (I->of)
  {
    I->index = I->pindex;
  }
  

  if (I->index != 0 || I->of == 1) append(&buf, "\x1b[H", 3); //move to 1,1
  I->pindex = I->index;
  I->of = 0;

  while (rows < E.height)
  {
    if (I->index == I->bound) 
    {
      if (rows == 0) return 0;
      while (rows++ < E.height)
      {
        append(&buf, "\x1b[K\n", 4);
      }
      break;
    }

    if (I->x != 0)
    {
      for (k = 0; k < E.blank - 3; k++) append(&buf, " ", 1);
      append(&buf, "-> ", 3);
    }
    else if (E.numbers) app_num(&buf, I->index + 1);
    else if (E.wrap) for (k = 0; k < E.blank; k++) append(&buf, " ", 1);


    if (!E.wrap)
    {
      rlen = line_insert_tabs(&rend, ar->lines[I->index]);
      if (rlen == MEM_ERROR) return MEM_ERROR;
      
      rlen -= I->offset;
      if (rlen < 0) rlen = 0;
      if (rlen > width) rlen = width;

      append(&buf, &rend[I->offset], rlen);
      if (I->max < ar->lines[I->index].length) I->max = ar->lines[I->index].length;

      free(rend);
    }
    else
    {
      idx = 5;
      numrows = 1;
      j = 0;

      if (I->x > 0)
      {
        j = I->x;
        I->x = 0;
      }

      while (j < ar->lines[I->index].length)
      {
        if (idx == numrows*(width + 6) - 1)
        {
          rows++;
          if (rows == E.height)
          {
            I->x = j - 1;
            I->index--;
            break;
          }

          append(&buf, "\n", 1);
          for (k = 0; k < E.blank - 3; k++)
            append(&buf, " ", 1);
          append(&buf, "-> ", 3);
          idx += E.blank + 1;
          numrows++;
          
        }
        else if (ar->lines[I->index].chars[j] == '\t')
        {
          do 
          {
            append(&buf, " ", 1);
            idx++;
            if (idx - numrows*(width + 6) > width)
            {
              j++;
              break;
            }
          } while (idx % E.tabwidth != 0);
          j++;
        }
        else
        {
          append(&buf, &ar->lines[I->index].chars[j++], 1);
          idx++;
        } 
      }
    }

    append(&buf, "\x1b[K\n", 4);
    I->index++;
    rows++;
  }

  write(STDIN_FILENO, buf.chars, buf.len);
  free(buf.chars);
  return 1;
}


int print(int start, int end, struct arraystr *ar) 
{
  char c;
  int printed;
  int j;

  I.index = start < 1 ? 0 : start - 1;
  I.offset = 0;
  I.x = 0;
  I.pindex = 0;
  I.of = 0;
  I.bound = end > ar->num ? ar->num : end;
  I.max = 0;

  page(&I, ar);

  E.printing = 1;

  while (1) 
  {
    enable_raw_mode();
    read(STDIN_FILENO, &c, 1);
    disable_raw_mode();

    if (c == ' ')
    {
      printed = page(&I, ar);
      if (printed == MEM_ERROR) return MEM_ERROR;
      if (printed == 0) 
      {
        write(STDOUT_FILENO, "\x1b[H", 3);
        for (j = 0; j < E.height; j++)
          write(STDOUT_FILENO, "\x1b[K\n", 4);
        write(STDOUT_FILENO, "\x1b[H", 3);
        break;
      }
      if (I.max - E.width + E.blank + 1 < I.offset)
      {
        I.offset = I.max - E.width + E.blank + 1;
        I.of = 1;
        page(&I, ar);
      }

    }
    else if (c == 'q')
    {
      write(STDOUT_FILENO, "\x1b[H", 3);
      for (j = 0; j < E.height; j++)
        write(STDOUT_FILENO, "\x1b[K\n", 4);
      write(STDOUT_FILENO, "\x1b[H", 3);
      break;
    }
    else if (c == '>' && E.wrap == 0 && I.max - E.width + E.blank >= I.offset)
    {
      I.offset++;
      I.of = 1;
      page(&I, ar);
    }
    else if (c == '<' && E.wrap == 0 && I.offset > 0)
    {
      I.offset--;
      I.of = 1;
      page(&I, ar);
    }
    c = 0;
  }

  E.printing = 0;
  return 0;
}

void sighandler(int sig)
{
  get_window_size();
  // if (E.printing)
  // {
  //   I.of = 1;
  //   page(&I);
  // }
}


/*  COMMANDS
   _________
 */
int add_token(struct arraystr *ar, struct buffer *buf)
{
  str* tmp = NULL;

  append(buf, "\0", 1);

  ar->num++;
  tmp = (str*)realloc(ar->lines, (ar->num)* sizeof(str));
  if (tmp == NULL) return MEM_ERROR;

  tmp[ar->num - 1].chars = buf->chars;
  tmp[ar->num - 1].length = buf->len - 1;

  ar->lines = tmp;

  buf->chars = NULL;
  buf->len = 0;
  buf->mem = 0;

  return 0;
}

int read_command(struct arraystr *ar)
{
  struct buffer buf;
  buf.chars = NULL;
  buf.len = 0;
  buf.mem = 0;

  char c;
  int par = 0;
  int parn = 0;
  int trpar = 0;

  while (1)
  {
    c = fgetc(stdin);

    if (c == '\n')
    {
      if (!par)
      {
        if (buf.len > 0 || parn == 2) add_token(ar, &buf);
        break;
      }
      else if (par && !trpar)
      {
        freear(ar);
        printf("wrong input: parenthases\n");
        return 1;
      }
      else if (trpar)
      {
        append(&buf, "\n", 1);
      }
    }
    else if (c == '\\')
    {
      c = fgetc(stdin);
      if (c == 'n')
      {
        append(&buf, "\n" , 1);
      }
      else if (c == 't')
      {
        append(&buf, "\t", 1);
      }
      else if (c == 'r')
      {
        append(&buf, "\r", 1);
      }
      else if (c == '\\') 
      {
        append(&buf, "\\" , 1);
      }
      else
      {
        append(&buf, &c, 1);
      }

      parn = 0;
    }
    else if (!par && (c == ' ' || c == '\t'))
    { 
      if (buf.len > 0 || parn == 2) add_token(ar, &buf);
    }
    else if (c == '\"') 
    {
      par = par == 0 ? 1 : 0;
      parn++;
      if (parn == 3)
      {
        trpar = trpar == 0 ? 1 : 0;
        parn = 0;
      }
    }
    else if (!par && c == '#')
    {
      add_token(ar, &buf);
      break;
    }
    else
    {
      append(&buf, &c, 1);
      parn = 0;
    }
  }

  return 0;
}


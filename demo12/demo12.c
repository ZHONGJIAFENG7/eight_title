#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* NOTE: https://docs.python.org/zh-cn/3/library/curses.html */
#include <ncurses.h>
#define SIZE 3
#define N 1000000

struct board
{
  int chess[SIZE * SIZE];
  int prev;
  int pos;
};

struct eighttile
{
  struct board *q;
  char dist[N][SIZE];
  struct board start;
  int first;
  int end;
  int min_step;
  int finish_pos;
};

struct board get_snapshot(FILE *fp);
void add(struct eighttile *e, struct board b);
void search_path(struct eighttile *e);
void pop(struct eighttile *e, struct board *b);
int is_solved(struct board b);
void find_neighbor(struct board b, struct eighttile *e);
void swap(int *a, int *b);
int can_insert(struct eighttile *e, struct board b);
void exchange(struct board b, struct eighttile *e, int i, int j);
void copy(struct board *dest, struct board *source);
int get_step(struct eighttile *e, int b);
void show_path(struct eighttile *e, struct board *b, int step);
void show_board(struct eighttile *e, struct board b, int step);
int is_valid(struct board b);
void animation(struct eighttile e);
void moving_and_sleeping(char *a, int width, int height);

int main(int argc, char *argv[])
{
  FILE *fp = NULL;
  struct eighttile e;
  struct board start;

  if (argc != 2)
  {
    fprintf(stderr, "argument error!\n");
    exit(EXIT_FAILURE);
  }
  if ((fp = fopen(argv[1], "r")) == NULL)
  {
    fprintf(stderr, "file is null!");
    exit(EXIT_FAILURE);
  }
  /* 分配无限大的空间 */
  e.q = (struct board *)malloc(sizeof(struct board) * N);
  memset(e.q, 0, sizeof(struct board) * N);
  e.first = 0;
  e.end = 0;
  e.min_step = N;
  e.finish_pos = 0;

  start = get_snapshot(fp);
  if (!is_valid(start))
  {
    printf("invalid input!\n");
  }
  else
  {
    start.prev = -1;
    add(&e, start);
    search_path(&e);
    printf("step in total: %d\n", e.min_step);
    show_path(&e, &(e.q[e.finish_pos]), e.min_step);
    animation(e);
  }

  fclose(fp);

  if (e.q)
  {
    free(e.q);
  }

  return 0;
}

struct board get_snapshot(FILE *fp)
{
  char ch;
  int i = 0;
  struct board start;

  /* 获取1-8的数字 */
  while ((ch = fgetc(fp)) != EOF)
  {
    if (ch == '\n' || ch == '\r')
    {
      continue;
    }

    /* 如果遇到为空则用0进行标识 */
    if (ch == ' ')
    {
      start.chess[i] = 0;
      start.pos = i;
    }
    else
    {
      start.chess[i] = ch - '0';
    }

    i++;
  }
  return start;
}

void add(struct eighttile *e, struct board b)
{
  int i;
  for (i = 0; i < SIZE * SIZE; i++)
  {
    e->q[e->end].chess[i] = b.chess[i];
  }
  e->q[e->end].prev = b.prev;
  e->q[e->end].pos = b.pos;
  e->end++;
}

void pop(struct eighttile *e, struct board *b)
{
  int i;
  for (i = 0; i < SIZE * SIZE; i++)
  {
    b->chess[i] = e->q[e->first].chess[i];
  }
  b->pos = e->q[e->first].pos;
  b->prev = e->q[e->first].prev;
  e->first++;
}

void search_path(struct eighttile *e)
{
  struct board b;
  int step;
  while (e->end != e->first)
  {
    pop(e, &b);
    if (is_solved(b))
    {
      step = get_step(e, e->first - 1);
      if (e->min_step > step)
      {
        e->min_step = step;
        e->finish_pos = e->first - 1;
      }
      return;
    }
    find_neighbor(b, e);
  }
}

int is_solved(struct board b)
{
  int i;
  if (b.chess[SIZE * SIZE - 1] != 0)
  {
    return 0;
  }

  for (i = 0; i < SIZE * SIZE - 1; i++)
  {
    if (b.chess[i] != i + 1)
    {
      return 0;
    }
  }

  return 1;
}

void find_neighbor(struct board b, struct eighttile *e)
{
  /* 获取空格所在行 */
  int i = b.pos / SIZE;
  /* 获取空格所在列 */
  int j = b.pos % SIZE;

  if (i - 1 >= 0)
  {
    exchange(b, e, i - 1, j);
  }

  if (i + 1 < SIZE)
  {
    exchange(b, e, i + 1, j);
  }

  if (j - 1 >= 0)
  {
    exchange(b, e, i, j - 1);
  }

  if (j + 1 < SIZE)
  {
    exchange(b, e, i, j + 1);
  }
}

void swap(int *a, int *b)
{
  int tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}

int can_insert(struct eighttile *e, struct board b)
{
  int i, j;

  for (i = e->first; i != e->end;)
  {
    for (j = 0; j < SIZE * SIZE; j++)
    {
      if (b.chess[j] != e->q[i].chess[j])
      {
        break;
      }
    }
    if (j == SIZE * SIZE)
    {
      break;
    }
    i++;
  }

  if (i == e->end)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void exchange(struct board b, struct eighttile *e, int i, int j)
{
  /* 1. 保存原先镜像 */
  struct board nb;
  copy(&nb, &b);

  /* swap */
  swap(&nb.chess[nb.pos], &nb.chess[i * SIZE + j]);

  /* 更新pos */
  nb.pos = i * SIZE + j;

  /* 更新prev */
  nb.prev = e->first - 1;

  /* 判断当前镜像是否有存在于队列中 */
  if (can_insert(e, nb))
  {
    add(e, nb);
  }
}

void copy(struct board *dest, struct board *source)
{
  int i;
  for (i = 0; i < SIZE * SIZE; i++)
  {
    dest->chess[i] = source->chess[i];
  }
  dest->pos = source->pos;
  dest->prev = source->prev;
}

int get_step(struct eighttile *e, int b)
{
  if (b > 0)
  {
    return get_step(e, e->q[b].prev) + 1;
  }
  else
  {
    return 0;
  }
}

void show_path(struct eighttile *e, struct board *b, int step)
{
  if (b->prev >= 0)
  {
    show_path(e, &(e->q[b->prev]), step - 1);
  }
  show_board(e, *b, step);
}

/* 扩容存储所有有效的board */
void show_board(struct eighttile *e, struct board b, int step)
{
  int i, j;
  int ds;

  ds = step * SIZE;
  for (i = ds; i < ds + SIZE; i++)
  {
    for (j = 0; j < SIZE; j++)
    {
      e->dist[i][j] = ' ';
    }
  }

  for (i = ds; i < ds + SIZE; i++)
  {
    for (j = 0; j < SIZE; j++)
    {
      if (b.chess[(i - ds) * SIZE + j] == 0)
      {
        e->dist[i][j] = ' ';
      }
      else
      {
        e->dist[i][j] = b.chess[(i - ds) * SIZE + j] + '0';
      }
    }
  }
}

int is_valid(struct board b)
{
  int i;
  for (i = 0; i < SIZE * SIZE; i++)
  {
    if (b.chess[i] < 0 || b.chess[i] >= SIZE * SIZE)
    {
      return 0;
    }
  }

  return 1;
}

void animation(struct eighttile e)
{
  int i = 0;
  initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_YELLOW);
  attrset(COLOR_PAIR(1));
  clear();

  do
  {
    moving_and_sleeping(&(e.dist[i][0]), SIZE, SIZE);
    i += SIZE;
  } while (e.min_step--);

  endwin();
}

void moving_and_sleeping(char *a, int width, int height)
{
  int i, j;

  clear();
  for (j = 0; j < height; j++)
  {
    move(j, 0);
    for (i = 0; i < width; i++)
    {
      int c = a[j * SIZE + i];
      addch(c);
      napms(100);
    }
  }
  refresh();
}

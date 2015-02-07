#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

/* csv entry */
typedef struct csv_entry
{
  int row;
  int col;
  char data[MAX_FIELD_LEN];
}csv_entry;

static char fieldValue[MAX_FIELD_LEN];
static long FCurPos;

static int col;  /* column number */
static int row;  /* row number, not real line number */
static int line; /* real line number */
static FieldCB fieldCB;
static PARSE_TYPE parse_type;

static csv_entry *entry;
static int nAllocEntry; /* total allocated csv_entry */
static int nCurrEntry;  /* current csv_entry */

/* Rules for describing the state changes and associated actions for the FSM. */
typedef struct fsmRule {
  int state;
  int c;
  int new_state;
  int (*action)(int state, int symbol);
}fsmRule;

static int increate_entry()
{
  csv_entry *newPtr = NULL;

  if ( entry == NULL)
    {
      entry = calloc(MEM_INCREASE_SZ, sizeof(csv_entry));
      if (entry == NULL) 
        {
          fprintf(stderr, "Malloc failed\n");
          return 1;
        }
      nAllocEntry = MEM_INCREASE_SZ;
      nCurrEntry = 0;
      return 0;
    }

  if (nCurrEntry >= nAllocEntry)
    {
      newPtr = realloc(entry, (nAllocEntry + MEM_INCREASE_SZ) * sizeof(csv_entry));
      if (newPtr == NULL)
        {
          fprintf(stderr, "Realloc failed\n");
          free(entry);
          return 1;
        }
      nAllocEntry += MEM_INCREASE_SZ;
      entry = newPtr;
      memset(entry+nCurrEntry, 0, MEM_INCREASE_SZ);
    }

  return 0;
}

static int newField(int state, int symbol)
{
  int result = 0;
  if (symbol == '\n' && strlen(fieldValue) == 0)
      return 0;

  /* 调试用 */
  //printf("row=[%d], line=[%d], col=[%d], data=[%s]\n", row, line, col, fieldValue);
  if (parse_type == PT_CALLBACK)
    {
      if (fieldCB) fieldCB(row, line, col, fieldValue);
    }
  else
    {
      result = increate_entry();
      if (result == 1) return 1;

      entry[nCurrEntry].row = row;
      entry[nCurrEntry].col = col;
      strcpy(entry[nCurrEntry].data, fieldValue);
      nCurrEntry++;
    }

  if (symbol == FS)
     col++;
  else if (symbol == '\n')
    {
      col = 1; line++; row++; 
    }

  memset(fieldValue, 0x00, sizeof(fieldValue));
  FCurPos = 0;
  return 0;
}

static int newLine(int state, int symbol)
{
  line++;
  col = 1;

  memset(fieldValue, 0x00, sizeof(fieldValue));
  FCurPos = 0;
  return 0;
}

static int collectData(int state, int symbol)
{
  if (FCurPos >= MAX_FIELD_LEN)
    {
      fprintf(stderr, "row:[%d] col:[%d] field length > %d\n", 
          row, col, MAX_FIELD_LEN);
      return 1;
    }

  if (symbol == '\n' && state == 1) line++;
  fieldValue[FCurPos++] = symbol;
  return 0;
}

static int getRows()
{
  if (parse_type != PT_ROWCOL)
      return -1;

  return row - 1;
}

static int getCol(int row)
{
  int result = 0;
  int i;
  if (parse_type != PT_ROWCOL)
      return -1;

  for (i = 0; i < nCurrEntry; i++)
      if (entry[i].row == row) result++;

  return result;
}

static char *getField(int row, int col)
{
  int i;
  if (parse_type != PT_ROWCOL)
      return NULL;

  for (i = 0; i < nCurrEntry; i++)
    {
      if (entry[i].row == row && entry[i].col == col)
          return entry[i].data;
    }
  return NULL;
}

static int csvParse(const char *filename, PARSE_TYPE type, FieldCB cb)
{
  FILE *fp = NULL;
  const fsmRule *p;
  int in, i, state = 0;
  int result = 0;

  /* 状态机, 这个状态机也可以更改成3D数组类型.
     下面的状态使用一个enum类型会更加易读
   */
  static const fsmRule fsm[] = {
        /* state = [START] - 0 :初始状态 */
        {0, '"',  1, 0          },
        {0, ' ',  0, 0          },
        {0, '\t', 0, 0          },
        {0, '#',  5, 0          },
        {0, '\n', 0, newField   },
        {0, FS,   0, newField   },
        {0,  0,   0, collectData},

        /* state = [InStr] - 1 :字符串中 */
        {1, '\\', 2, 0          },
        {1, '"',  3, 0          },
        {1, 0,    1, collectData},

        /* state = [StrQuote] - 2 :字符串中遇到\"时候的状态 */
        {2, 0 , 1, collectData},

        /* state = [MayBeDoubleQuote] - 3 :可能为DoubleQuote */
        {3, '"',  4, collectData},
        {3, '\n', 0, newField   },
        {3, FS,   0, newField   },
        {3, 0,    1, 0          },

        /* state = [DoubleQuote] - 4 :DoubleQuote */
        {4, '"',   0, 0          },
        {4, 0,     1, collectData},

        /* state = [Comment] - 5 :注释状态 */
        {5, '\n', 0, newLine},
        {5, 0,    5, 0},

    };

  fp = fopen(filename, "r");
  if (fp == NULL)
    {
      fprintf(stderr, "open file %s error\n", filename);
      return 1;
    }

  fieldCB = cb;
  parse_type = type;
  col = row = line = 1;
  while((in = getc(fp)) != EOF) {
      for (i = 0; i< sizeof(fsm) /sizeof(fsm[0]); ++i)
        {
          p = &fsm[i];
          if (p->state == state && (p->c == in || p->c == 0))
            {
              //printf("state=[%d] in=[%c] ", state, in);
              if (p->action != NULL)
                {
                  result = (p->action)(state, in);
                  if (result != 0) 
                    {
                      fclose(fp);
                      return 1;
                    }
                }
              state = p->new_state;
              //printf("next state=[%d]\n", state);
              break;
            }
        } /* end for */
    } /* end while */


  if (state != 0) {
      printf("input is malformed, ends inside comment or literal \n");
      printf("state=[%d]\n", state);
      fclose(fp);
      return 1;
    }

  fclose(fp);
  return 0;
}

static void csvFree()
{
  if (parse_type != PT_ROWCOL) return;
  if (entry)
    {
      free(entry);
      entry = NULL;
    }
}

CsvParserApi csvParserApi = {
  csvParse,
  getRows,
  getCol,
  getField,
  csvFree,
};

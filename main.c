#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv.h"

void printField(int row, int line, int col, char *fieldValue)
{
  printf("row=[%d], line=[%d], col=[%d], data=[%s]\n", row, line, col, fieldValue);
}

int main(int argc, char **argv) {
  int result;

#if 1
  /* 使用行列模式 */
  result = csvParserApi.csvParse("test.csv", PT_ROWCOL, NULL);
  if (result != 0) 
    {
      csvParserApi.csvFree();
      return 1;
    }

  int rows = csvParserApi.getRows();
  int i, j;
  for (i = 1; i <= rows; i++)
    {
      printf("\n\n=======================ROW[%i]============================\n", i);
      int cols = csvParserApi.getCol(i);
      for (j = 1; j <= cols; j++)
        {
          printf("data[%d][%d]=[%s]\n", i, j, csvParserApi.getField(i,j));
        }

    }

  csvParserApi.csvFree();

#endif

#if 0
    /* 使用回调模式 */
  result = csvParserApi.csvParse("test.csv", PT_CALLBACK, printField);
  if (result != 0) 
    {
      csvParserApi.csvFree();
      return 1;
    }
 csvParserApi.csvFree();

#endif
  return 0;
}


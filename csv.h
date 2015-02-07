#ifndef __CSV_PARSER_H__
#define __CSV_PARSER_H__

#ifndef FS
  #define FS ','
#endif

#define MEM_INCREASE_SZ 5   /* memory increate size */
#define MAX_FIELD_LEN 2048  /* max field length */

typedef enum PARSE_TYPE{
    PT_CALLBACK, //使用回调函数
    PT_ROWCOL, //行列模式
}PARSE_TYPE;

/* 回调函数 */
typedef void (*FieldCB)(int row, int line, int col, char *fieldValue);

typedef struct tagCsvParser
{
  int   (*csvParse)(const char *filename, PARSE_TYPE type, FieldCB cb);
  int   (*getRows)(void); //得到行数, PT_ROWCOL时有效
  int   (*getCol)(int row); //得到指定行的列数, PT_ROWCOL时有效
  char* (*getField)(int row, int col); //得到指定行列的数据, PT_ROWCOL时有效
  void  (*csvFree)(void);
}CsvParserApi;

extern CsvParserApi csvParserApi;

#endif


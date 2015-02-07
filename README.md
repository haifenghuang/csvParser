# csvParser 
使用状态机实现的csv文件解析器。

## 特性
1. 简单轻量
2. 使用状态机实现
3. 提供两种模式取csv数据：回调函数和行列模式
4. 使用接口方式实现API
5. 支持Excel方式的csv格式
6. 单元格支持多行
7. 单元格内容可以使用引号,也可以不用引号(“def”或者def) 
8. 支持单行注释(#开头)

## 使用
### 使用回调函数
```c
//回调函数
void printField(int row, int line, int col, char *fieldValue)
{
   printf("row=[%d], line=[%d], col=[%d], data=[%s]\n", row, line, col, fieldValue);
}

int main()
{
    int result;
    result = csvParserApi.csvParse("test.csv", PT_CALLBACK, printField);
    if (result != 0) 
      {
        csvParserApi.csvFree();
        return 1;
      }
    csvParserApi.csvFree();
    return 0;
}
```
### 使用行列模式
```c
int main(int argc, char **argv) {
  int result;

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
  return 0;
}
```

关于详细信息：  
csv文件请参照`test.csv`  
测试文件请参照`main.c`

## Bug汇报
如果你发现程序中有任何错误，请发送邮件给我：`fenghai_hhf@163.com`。

## 许可证
MIT许可证

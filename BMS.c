#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "/usr/include/mysql/mysql.h"
enum status {manager, student, unknown};
typedef enum status now_sta;
int main(int argc, char *argv[])
{
    MYSQL mysql;
    MYSQL_RES *res = NULL;
    MYSQL_ROW row;
    char query_str[1024] = {0};
    char command[1024] = {0};
    now_sta sta = unknown;
    char now_manager_id[21] = {0};
    int rc, i;
    unsigned int fields;
    if(NULL==mysql_init(&mysql))
    {
       printf("MYSQL初始化出错: %s, 请尝试重启此系统.\n", mysql_error(&mysql));
       exit(-1);
    }
    if(NULL==mysql_real_connect(&mysql,
    "192.168.56.129", "cosmathly",
    "liaoyu520233", "test",
    0, NULL, 0)) 
    {
       printf("MYSQL连接出错: %s, 请尝试重启此系统.\n", mysql_error(&mysql));
       exit(-1);
    }
    printf("欢迎来到图书管理系统!\n");
    printf("初始化工作已经完成.\n");
    printf("请输入相应指令来操作此系统.\n");
    while(1)
    {  
          fgets(command, 1000, stdin);
          int len = strlen(command);
          command[len-1] = '\0';
          if(strcmp(command, "sign in")==0)
          {
             printf("请输入管理员ID和密码:\n");
             int id, pswd;
             scanf("%d %d", &id, &pswd);
             sprintf(query_str, "select pswd from manager where manager.id = %d", id);
             rc = mysql_real_query(&mysql, query_str, strlen(query_str));
             if(rc!=0)
             {
                printf("数据库操作失败, 请尝试重新启动此系统.\n");
                exit(-1);
             }
             res = mysql_store_result(&mysql);  
             if(res==NULL) 
             {
                printf("账户不存在!\n");
                continue;
             }
             else 
             {
                row = mysql_fetch_row(res);
                int real_pswd = atoi(row[0]);
                if(real_pswd!=pswd)
                {
                    printf("密码错误!\n");
                    continue;
                }
                else 
                {
                    printf("登陆成功!\n");
                    sta = manager;
                    sprintf(now_manager_id, "%d", id);
                    continue;
                }
             }
          }
          else if(strcmp(command, "insert book")==0)
          {
               if(sta!=manager) 
               {
                  printf("这是管理员才能进行的操作, 请先进行管理员登陆!\n");
                  continue;
               }
               printf("请输入图书入库模式(单本或多本):\n");
               char mode[10] = {0};
               fgets(mode, 9, stdin);
               int len = strlen(mode);
               mode[len-1] = '\0';
               if(strcmp(mode, "one")==0) 
               {
                  printf("请输入书籍信息:\n");
                  char one_book[112] = {0};
                  fgets(one_book, 100, stdin);
                  len = strlen(one_book);
                  one_book[len-1] = '\0';
                  printf("one_book%s\n", one_book);
                  sprintf(query_str, "insert into book values%s", one_book);
                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                  if(rc!=0)
                  {
                     printf("图书入库失败, 请尝试重新入库!\n");
                     continue;
                  }
                  else 
                  {
                     printf("图书入库成功!\n");
                     continue;
                  }
               }
               else if(strcmp(mode, "many")==0)
               {
                    printf("请输入图书信息所在文件路径:\n");
                    char path[112] = {0};
                    fgets(path, 100, stdin);
                    int len = strlen(path);
                    path[len-1] = '\0';
                    int fd = open(path, O_RDONLY);
                    char new_book[112] = {0};
                    int flag = 1;
                    int cnt;
                    int success_insert_book_cnt = 0;
                    int fail_insert_book_cnt = 0;
                    while(flag)
                    { 
                         cnt = 0;
                         memset(new_book, 0, sizeof(new_book));
                         char ch[2] = {0};
                         while(1)
                         {
                               int len = read(fd, ch, 1);
                               if(len==0)
                               {
                                  if(cnt==0) {flag = 0; break;}
                                  sprintf(query_str, "insert into book values%s", new_book);
                                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                                  if(rc!=0) fail_insert_book_cnt++;
                                  else success_insert_book_cnt++;
                                  flag = 0;
                                  break;
                               }
                               else 
                               {
                                  if(ch[0]=='\n') 
                                  {
                                     sprintf(query_str, "insert into book values%s", new_book);
                                     rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                                     if(rc!=0) fail_insert_book_cnt++;
                                     else success_insert_book_cnt++;
                                     break;
                                  }
                                  else new_book[cnt++] = ch[0];
                               }
                         }
                    }
                    printf("成功入库的图书数量: %d， 入库失败的图书数量: %d.\n", success_insert_book_cnt, fail_insert_book_cnt);
               } 
          }
          else if(strcmp(command, "book query")==0)
          {
               printf("请输入查询条件, 包括类别、书名、出版社、年份(区间如: 2014~2015)、作者、价格(区间如: 90.00~98.00)\n");
               printf("如对某一属性无要求则设置为 NULL\n");
               char type[21] = {0};
               char book_name[21] = {0};
               char publish_house[21] = {0};
               char year[21] = {0};
               char author[21] = {0};
               char price[21] = {0};
               char predicate[127] = {0};
               scanf("%s %s %s %s %s %s", type, book_name, publish_house, year, author, price);
               if(strcmp(type, "NULL")!=0) sprintf(predicate, "book.type = \'%s\' and ", type);
               printf("predicate:%s\n", predicate);
               if(strcmp(book_name, "NULL")!=0) 
               {
                  char tmp[41] = {0};
                  sprintf(tmp, "book.book_name = \'%s\' and ", book_name);
                  strcat(predicate, tmp);
               }
               if(strcmp(publish_house, "NULL")!=0)
               {
                  char tmp[41] = {0};
                  sprintf(tmp, "book.publish_house = \'%s\' and ", publish_house);
                  strcat(predicate, tmp);
               }
               if(strcmp(year, "NULL")!=0)
               {
                  char begin[5] = {0};
                  char end[5] = {0};
                  sprintf(begin, "%c%c%c%c", year[0], year[1], year[2], year[3]);
                  sprintf(end, "%c%c%c%c", year[5], year[6], year[7], year[8]);
                  char tmp[41] = {0};
                  sprintf(tmp, "book.year >= %s and book.year <= %s and ", begin, end);
                  strcat(predicate, tmp);
               }
               if(strcmp(author, "NULL")!=0)
               {
                  char tmp[41] = {0};
                  sprintf(tmp, "book.author = \'%s\' and ", author);
                  strcat(predicate, tmp);
               }
               if(strcmp(price, "NULL")!=0)
               {
                  int cur = 0;
                  int cur1 = 0;
                  int cur2 = 0;
                  char low[21] = {0};
                  char high[21] = {0};
                  while(1)
                  {
                      if(price[cur]=='~') {cur++; break;}
                      if(price[cur]=='.')
                      {
                         low[cur1++] = price[++cur];
                         low[cur1++] = price[++cur];
                         cur++;
                      }
                      else low[cur1++] = price[cur++];
                  }
                  while(1)
                  {
                      if(price[cur]=='.')
                      {
                         high[cur2++] = price[++cur];
                         high[cur2++] = price[++cur];
                         break;
                      }
                      else high[cur2++] = price[cur++];
                  }
                  char tmp[100] = {0};
                  sprintf(tmp, "book.price >= %s and book.price <= %s and ", low, high);
                  strcat(predicate, tmp);
               }
               int len = strlen(predicate);
               if(len!=0) predicate[len-5] = '\0';
               if(len==0)
               sprintf(query_str, "select * from book");
               else if(len!=0)
               sprintf(query_str, "select * from book where %s", predicate);
               printf("query_str:%s\n", query_str);
               rc = mysql_real_query(&mysql, query_str, strlen(query_str));
               if(rc!=0) 
               {
                  printf("查询失败!\n");
                  continue;
               } 
               else 
               {
                  res = mysql_store_result(&mysql);
                  fields = mysql_num_fields(res);
                  if(res==NULL) 
                  {
                     printf("结果集打开失败!\n");
                     continue;
                  }
                  else 
                  {
                     while((row = mysql_fetch_row(res))!=NULL)
                     {
                          for(i = 0; i < fields; i++)
                          {
                              int len = strlen(row[i]);
                              if(len==0) printf("NULL ");
                              else printf("%s ", row[i]);
                          }
                          printf("\n");
                     }
                  }
               }
          }
          else if(strcmp(command, "borrow book")==0) 
          {     
                  int len = strlen(now_manager_id);
                  if(len==0) 
                  {
                     printf("当前无任何管理员登陆, 请先进行管理员登陆!\n");
                     continue;
                  }
                  printf("这是教师或学生进行的操作, 请输入借书证卡号进行身份验证:\n");
                  char cno[21] = {0};
                  scanf("%s", cno);
                  sprintf(query_str, "select name from card where card.cno = %s", cno);
                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                  if(rc!=0) 
                  {
                     printf("查询身份失败!\n");
                     continue;
                  }
                  else 
                  {
                     res = mysql_store_result(&mysql);
                     if(res==NULL) 
                     {
                        printf("查无此人!\n");
                        continue;
                     }
                     else 
                     {
                        printf("身份验证成功!\n");
                        sta = student;
                        printf("以下是您目前已借的书籍:\n");
                        char tmp[100] = {0};
                        sprintf(tmp, "select bno from record where record.cno = %s", cno);
                        sprintf(query_str, "select * from book where book.bno in(%s)", tmp);
                        rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                        if(rc!=0) 
                        {
                           printf("查询失败!\n");
                           continue;
                        }
                        else 
                        {
                           res = mysql_store_result(&mysql);
                           if(res==NULL) printf("暂无已借书籍!\n");
                           else 
                           {
                              unsigned int fields = mysql_num_fields(res);
                              while((row = mysql_fetch_row(res))!=NULL)
                              {
                                   for(int i = 0; i < fields; i++)
                                   {
                                       int len = strlen(row[i]);
                                       if(len==0) printf("NULL ");
                                       else printf("%s ", row[i]);
                                   }
                                   printf("\n");
                              }
                           }
                        }
                        printf("请输入想借的书的书号:\n");
                        char bno[21] = {0};
                        scanf("%s", bno);
                        sprintf(query_str, "select * from book where book.bno = %s", bno);
                        rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                        if(rc!=0) 
                        {
                           printf("查询失败!\n");
                           continue;
                        }
                        else 
                        {
                            res = mysql_store_result(&mysql);
                            if(res==NULL) 
                            {
                               printf("查无此书!\n");
                               continue;
                            }
                            else 
                            {
                               unsigned int fields = mysql_num_fields(res);
                               row = mysql_fetch_row(res);
                               int stock = atoi(row[fields-1]);
                               if(stock>0) 
                               {
                                  printf("借书成功!\n");
                                  printf("请输入借期和还期:\n");
                                  char begin_time[21] = {0};
                                  char end_time[21] = {0};
                                  scanf("%s", begin_time);
                                  scanf("%s", end_time);
                                  sprintf(query_str, "insert into record values(%s, %s, \'%s\', \'%s\', %s)", bno, cno, begin_time, end_time, now_manager_id);
                                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                                  if(rc!=0) printf("借书记录插入失败!\n");
                                  else printf("借书记录插入成功!\n");
                                  sprintf(query_str, "update book set stock = stock - 1 where book.bno = %s", bno);
                                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                                  if(rc!=0) 
                                  {
                                     printf("库存更新失败!\n");
                                     continue;
                                  }
                                  else 
                                  {
                                     printf("库存更新成功!\n");
                                     continue;
                                  }
                               }
                               else 
                               {
                                   sprintf(query_str, "select min(end_time) from record where record.bno = %s", bno);
                                   rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                                   if(rc!=0) 
                                   {
                                      printf("查询失败!\n");
                                      continue;
                                   }
                                   else 
                                   {
                                      res = mysql_store_result(&mysql);
                                      row = mysql_fetch_row(res);
                                      printf("该书预计最快返还时间为: %s\n", row[0]);
                                      continue;
                                   }
                               }
                            }
                        }
                     }
                  }
          }
          else if(strcmp(command, "return book")==0)
          {
                  printf("这是教师或学生进行的操作, 请输入借书证卡号进行身份验证:\n");
                  char cno[21] = {0};
                  scanf("%s", cno);
                  sprintf(query_str, "select name from card where card.cno = %s", cno);
                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                  if(rc!=0) 
                  {
                     printf("查询身份失败!\n");
                     continue;
                  }
                  else 
                  {
                     res = mysql_store_result(&mysql);
                     if(res==NULL) 
                     {
                        printf("查无此人!\n");
                        continue;
                     }
                     else 
                     {
                        printf("身份验证成功!\n");
                        sta = student;
                        printf("以下是您目前已借的书籍:\n");
                        char tmp[100] = {0};
                        sprintf(tmp, "select bno from record where record.cno = %s", cno);
                        sprintf(query_str, "select * from book where book.bno in(%s)", tmp);
                        rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                        if(rc!=0) 
                        {
                           printf("查询失败!\n");
                           continue;
                        }
                        else 
                        {
                           res = mysql_store_result(&mysql);
                           if(res==NULL) printf("暂无已借书籍!\n");
                           else 
                           {
                              unsigned int fields = mysql_num_fields(res);
                              while((row = mysql_fetch_row(res))!=NULL)
                              {
                                   for(int i = 0; i < fields; i++)
                                   {
                                       int len = strlen(row[i]);
                                       if(len==0) printf("NULL ");
                                       else printf("%s ", row[i]);
                                   }
                                   printf("\n");
                              }
                           }
                        }
                        printf("请输入所还书号:\n");
                        char bno[21] = {0};
                        scanf("%s", bno);
                        sprintf(query_str, "select id from record where record.bno = %s and record.cno = %s", bno, cno);
                        rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                        if(rc!=0) 
                        {
                           printf("查询失败!\n");
                           continue;
                        }
                        else 
                        {
                           res = mysql_store_result(&mysql);
                           if(res==NULL) 
                           {
                              printf("出错: 该书不在借书列表!\n");
                              continue;
                           }
                           else 
                           {
                               printf("还书成功!\n");
                               sprintf(query_str, "delete from record where record.bno = %s and record.cno = %s", bno, cno);
                               rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                               if(rc!=0) printf("借书记录删除失败!\n");
                               else printf("借书记录删除成功!\n");
                               sprintf(query_str, "update book set stock = stock + 1 where book.bno = %s", bno);
                               rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                               if(rc!=0) 
                               {
                                  printf("库存增加失败!\n");
                                  continue;
                               }
                               else 
                               {
                                   printf("库存增加成功!\n");
                                   continue; 
                               }
                           }
                        }
                     }
                  }

          }
          else if(strcmp(command, "card management")==0)
          {
               printf("请输入是增加还是删除一个借书证:\n");
               char mode[11] = {0};
               scanf("%s", mode);
               if(strcmp(mode, "add")==0)
               {
                  printf("请输入借书证信息, 包括(卡号, 姓名, 单位, 类别(教师或学生))\n");
                  printf("如有某属性未知则置为NULL:\n");
                  char cno[11] = {0};
                  char name[21] = {0};
                  char dept[21] = {0};
                  char type[2] = {0};
                  scanf("%s %s %s %s", cno, name, dept, type);
                  char tuple[40] = {0};
                  strcat(tuple, cno);
                  if(strcmp(name, "NULL")==0) 
                  {
                     char tmp[41] = {0};
                     sprintf(tmp, ", NULL");
                     strcat(tuple, tmp);
                  }
                  else 
                  {
                     char tmp[41] = {0};
                     sprintf(tmp, ", \'%s\'", name);
                     strcat(tuple, tmp);
                  }
                  if(strcmp(dept, "NULL")==0)
                  {
                     char tmp[41] = {0};
                     sprintf(tmp, ", NULL");
                     strcat(tuple, tmp);
                  }
                  else 
                  {
                      char tmp[41] = {0};
                      sprintf(tmp, ", \'%s\'", dept);
                      strcat(tuple, tmp);
                  }
                  if(strcmp(type, "NULL")==0)
                  {
                      char tmp[41] = {0};
                      sprintf(tmp, ", NULL");
                      strcat(tuple, tmp);
                  }
                  else 
                  {
                      char tmp[41] = {0};
                      sprintf(tmp, ", \'%s\'", type);
                      strcat(tuple, tmp);
                  }
                  sprintf(query_str, "insert into card values(%s)", tuple);
                  rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                  if(rc!=0) 
                  {
                     printf("增加借书证失败!\n");
                     continue;
                  }
                  else 
                  {
                     printf("增加借书证成功!\n");
                  }
               }
               else if(strcmp(mode, "delete")==0)
               {
                    printf("请输入要删除的借书证卡号:\n");
                    char cno[11] = {0};
                    scanf("%s", cno);
                    sprintf(query_str, "delete from card where card.cno = %s", cno);
                    rc = mysql_real_query(&mysql, query_str, strlen(query_str));
                    if(rc!=0)
                    {
                       printf("删除失败!\n");
                       continue;
                    }
                    else 
                    {
                       printf("删除成功!\n");
                    }
               }
          }
          else if(strcmp(command, "exit")==0) 
          {
               printf("已退出图书管理系统!\n");
               break;
          }
    }
    return 0;
}
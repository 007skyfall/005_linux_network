#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sqlite3.h>
#define DATA_PATH "./mystu.db"

void do_insert(sqlite3 *db);//插入数据
void do_query(sqlite3 *db); //查询数据
void do_delete(sqlite3 *db);//删除数据
void do_update(sqlite3 *db);//更新数据
int main(int argc, const char *argv[])
{
	
	int cmd;
	char clean[128]={0};
	sqlite3 *db;

	if(sqlite3_open(DATA_PATH,&db)!=SQLITE_OK)//打开数据库获得数据库的句柄
	{
		 printf("%s\n",sqlite3_errmsg(db));
		 return -1;
	}


	while(1)
	{
		puts("***********************************************");
		puts("***************student system******************");
		puts("1.insert  2.query   3.delete   4.update  5.quit");
		puts("***********************************************");
		puts("input cmd");
		if(scanf("%d",&cmd)!=1)
		{
			puts("cmd error");
			fgets(clean,sizeof(clean),stdin);
			continue;
		}
		switch(cmd)
		{
			case 1:
				do_insert(db);
				break;
			case 2:
				do_query(db);
				break;
			case 3:
				do_delete(db);
				break;
			case 4:
				do_update(db);
				break;
			case 5:
				exit(0);
			default:
				puts("error cmd:1 or 2 or 3 or 4 or 5");
				break;
		}
	}
	sqlite3_close(db);//关闭数据库


	return 0;
}
int callback(void *arg,int num,char **f_val,char **f_name)//回调函数
{

	int ret=(*(int*)arg)++;
	int i;
	if(ret==0)//只打印一次字段名
	{
		for(i=0;i<num;i++)//打印字段名
		{
			printf("%-15s",*f_name++);
		}
	}
	putchar(10);
	for(i=0;i<num;i++)//打印字段值
	{
		printf("%-15s",*f_val++);
	}
	putchar(10);

	
	return 0;
}
void do_insert(sqlite3 *db)//插入数据
{
       char sql[128]={0};
	   char *errmsg;
	   int id;
	   char name[32]={0};
	   char sex[4] = {0};
	   float score;
	   puts("input id>>>");
	   scanf("%d",&id);
	   puts("input name>>");
	   scanf("%s",name);
	   puts("input score>>>");
	   scanf("%f",&score);
	   puts("input sex>>>");
	   scanf("%s",sex);
	   sprintf(sql,"insert into student values(%d,'%s',%f,'%s')",id,name,score,sex);//拼接SQL语句
	   if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	   {
			printf("%s\n",errmsg);
			return;
	   }
	   puts("insert ok!");
	  return;

}
/*
void do_query(sqlite3 *db)//查询数据  根据ID找成绩
{
	 char sql[128]={0};
	 char *errmsg;
	 int id;
	 int arg=0;
	 puts("input id>>>");
	 scanf("%d",&id);
	 sprintf(sql,"select * from student where id=%d",id);
	 if(sqlite3_exec(db,sql,callback,(void*)&arg,&errmsg)!=SQLITE_OK)
     {
	 		printf("%s\n",errmsg);
			return;
	 }
	 puts("query ok!");
	 return;
}*/
void do_query(sqlite3 *db)//查询数据  根据ID找成绩
{
		char sql[128]={0};
		char *errmsg;
		char **rep;
		int n_row;
		int n_column;
		int id;
		int i,j;
		puts("input id>>>");
		scanf("%d",&id);
		sprintf(sql,"select * from student where id=%d",id);
		if(sqlite3_get_table(db,sql,&rep,&n_row,&n_column,&errmsg)!=SQLITE_OK)
		{
			printf("%s\n",errmsg);
			return;
		}
		else
		{
			if(n_row==0)
			{
				puts("query fail!");
				return;
			}
			else
			{
				for(i=0;i<n_row+1;i++)
				{
					for(j=0;j<n_column;j++)
					{
						printf("%-15s",*rep++);
					}
					putchar(10);
				}
				putchar(10);
				puts("query ok!");
				return;
			}
		}

}
void do_delete(sqlite3 *db)//删除数据 根据ID删除一项
{
	char sql[128]={0};
	char *errmsg;
	int id;
	puts("input id>>");
	scanf("%d",&id);
	sprintf(sql,"delete from student where id=%d",id);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{
		printf("%s\n",errmsg);
		return;
	}
	puts("delete ok!");
	return;
}
void do_update(sqlite3 *db)//更新数据 根据ID 修改分数
{
	char sql[128]={0};
	char *errmsg;
	int id;
	float score;
	puts("input id>>");
	scanf("%d",&id);
	puts("input score>>>");
	scanf("%f",&score);
	sprintf(sql,"update student set score=%f where id=%d",score,id);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!=SQLITE_OK)
	{	
		printf("%s\n",errmsg);
		return;
	}
	puts("update ok!");
	return;

}

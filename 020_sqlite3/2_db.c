#include <stdio.h>
#include <sqlite3.h>

//由于系统本身没有相关函数，所以需要手动链接库文件 gcc *.c -lsqlite3

#define N 128
#define DATABASE "student.db"

int flags = 0;

void do_insert(sqlite3 *db);
int callback(void *arg, int ncolumn, char **f_text, char **f_name);
void do_select(sqlite3 *db);
void do_select_get_table(sqlite3 *db);
int main(int argc, const char *argv[])
{
	sqlite3 *db;
	char *errmsg;

	//打开或者创建一个数据库
	if(sqlite3_open(DATABASE, &db) != SQLITE_OK)
	{
		printf("%s\n", sqlite3_errmsg(db));
	}
	else 
	{
		printf("The database is created or opened successfully\n");
	}

	//创建一张表
	if(sqlite3_exec(db, "create table stu(id int, name char, score int)", NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("%s\n", errmsg);
	}
	else 
	{
		printf("The table is created successfully\n");
	}
	
	int n;

	while(1)
	{
		printf("***************************************************************\n");
		printf("*** 1：插入数据 2：查询数据 3：修改数据 4：删除数据 5：退出 ***\n");
		printf("***************************************************************\n");
		printf(">>> ");

		scanf("%d", &n);

		switch(n)
		{
		case 1:
			do_insert(db);
			break;
		case 2:
			//flags = 0;
			//do_select(db);
			do_select_get_table(db);
			break;
		case 3:
			//do_update();
			break;
		case 4:
			//do_delete();
			break;
		case 5:
			sqlite3_close(db);
			return -1;
		default:
			printf("您输入的有误，请重新输入\n");
		}
	}

	return 0;
}

void do_insert(sqlite3 *db)
{
	int id;
	char name[N] = {};
	int score;
	char sql[N] = {};
	char *errmsg;

	printf("请输入学号：");
	scanf("%d", &id);

	printf("请输入姓名：");
	scanf("%s", name);

	printf("请输入分数：");
	scanf("%d", &score);
	
	sprintf(sql, "insert into stu values(%d, '%s', %d)", id, name, score);

	if(sqlite3_exec(db, sql, NULL, NULL, &errmsg) != SQLITE_OK)
	{
		printf("%s\n", errmsg);
	}
	else 
	{
		printf("The data is inserted successfully\n");
	}

	return ;
}

int callback(void *arg, int ncolumn, char **f_text, char **f_name)
{
	int i;

	if(flags == 0)
	{
		for(i = 0; i < ncolumn; i++)
		{
			printf("%-11s", f_name[i]);
		}
		putchar(10);

		printf("---------------------------------\n");

		flags = 1;
	}

	for(i = 0; i < ncolumn; i++)
	{
		printf("%-11s", f_text[i]);
	}
	putchar(10);

	return 0;
}

void do_select(sqlite3 *db)
{
	char *errmsg;

	if(sqlite3_exec(db, "select * from stu", callback, NULL, &errmsg) != SQLITE_OK)
	{
		printf("%s\n", errmsg);
	}
	else 
	{
		printf("The data is selected successfully\n");
	}

	return ;
}

void do_select_get_table(sqlite3 *db)
{
	char **ret;
	int nrow, ncolumn;
	char *errmsg;

	if(sqlite3_get_table(db, "select * from stu", &ret, &nrow, &ncolumn, &errmsg) != SQLITE_OK)
	{
		printf("%s\n", errmsg);
	}
	
	//printf("nrow = %d\n", nrow);

	int i, j, n = 0;
	for(i = 0; i < nrow + 1; i++)
	{
		for(j = 0; j < ncolumn; j++)
		{
			printf("%-11s", ret[n++]);
		}

		putchar(10);
	}

	return ;
}

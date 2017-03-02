#ifndef MINISQL_H
#define MINISQL_H

#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

typedef short int attr_type;	//属性类型
typedef short int cond_type;	//条件类型
typedef short int op_type;		//操作类型
typedef short int error_type;	//错误类型
typedef short int buffer_type;	//缓冲区类型
typedef short int node_type;	//B+树节点类型

//块属性
#define MAX_BLOCK 1024
#define BLOCK_SIZE 8192

//属性类型
#define INT 	0
#define FLOAT	1
#define CHAR	2

//类型长度
#define INT_LEN		11
#define FLOAT_LEN	20

//条件类型
#define LT		0	//less than
#define LE		1	//less or equal
#define GT		2	//great than
#define GE		3	//great or equal
#define EQ		4	//equal
#define NE		5   //not equal

//操作类型
#define CRETAB	0
#define DRPTAB	1
#define CREIDX	2
#define DRPIDX	3
#define SELECT	4
#define INSERT	5
#define DELETE	6
#define UNKNOWN	99

//错误类型
#define NOERR	0	//无错误
#define SYNERR	1	//语法错误
#define PRIERR	2	//主键错误
#define FLERR	3	//文件读取错误
#define OPERR	4	//操作类型错误
#define TABEXT	5	//表已经存在
#define NOTAB	6	//表不存在
#define VALERR	7	//插入值与表不匹配
#define IDXEXT	8	//索引已存在
#define UNIERR	9	//违反唯一性
#define NOIDX	10	//不存在该索引
#define CONERR	11	//条件与表不匹配

//缓冲区类型
#define UNDEFINE	0	//未定义
#define TABLE		1	//表块
#define INDEX		2	//索引块
#define MAP			3	//图块

//B+树节点类型
#define ROOT	0	//根节点
#define INNER	1	//内节点
#define LEAF	2	//叶子节点
#define RANDL	3	//既是根节点，又是叶子节点




//表中的列，即一个属性
struct column
{
	string col_name;			//属性名字
	attr_type type;				//属性类型
	unsigned int col_offset;	//属性在记录中的offset，以字节为单位
	unsigned int col_length;	//属性的长度，以字节为单位
	bool isPrimary;				//是否为primary key
	bool isUnique;				//是否为unique
};

//where语句中的条件
struct condition
{
	string attr_name;			//属性名字
	cond_type cond;				//比较条件，LT,GT,QU,NE,LE,GE中的一种
	string value;				//参加比较的常数
	attr_type type;				//属性类型
	unsigned int attr_offset;	//属性在记录中的offset
	unsigned int attr_length;	//属性的长度
};

//insert into语句中的一个插入项
struct insertvalue
{
	string value;				//插入的值
	attr_type type;				//插入值类型
	unsigned int length;		//值长度
};

//表的信息
struct table
{
	string table_name;			//表名
	unsigned int col_num;		//属性数量
	vector<column> cols;		//属性信息
	string primary;				//主键信息
};

//索引的信息
struct index
{
	string table_name;			//表名
	string index_name;			//索引名
	string attr_name;			//索引使用的属性名
	int attr_type;			//索引使用的属性类型
	int attr_len;				//索引使用的属性的长度
};

string read_error(error_type error);

#endif
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "MiniSQL.h"
using namespace std;

class BufferManager
{
public:
	string name;				//块对应的表或者索引的名字
	buffer_type type;			//块对应的类型(TABLE或INDEX)
	char* block;				//块的内容首地址
	unsigned int block_page;	//块在对应的文件中的页序数
	bool isChanged;				//块是否被改变
	bool isUsing;				//块是否正在使用
	unsigned int LRU_count;		//用于LRU算法的计数

	static vector<BufferManager> BlockSet;

	BufferManager() {};
	~BufferManager() {};

	static void initiate_BlockSet();
	static unsigned int get_block(buffer_type type, string name, unsigned int block_page);
	static void using_block(unsigned int block_num);
	static unsigned int get_blank_block();
	static unsigned int LRU_number();
	static void flush_all();
	static void used_block(unsigned int block_num) { BlockSet[block_num].isUsing = false; };
	void flush_block();

};

bool ValueCompare(string value, attr_type type, cond_type cond, string comvalue);
#endif
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "MiniSQL.h"
using namespace std;

class BufferManager
{
public:
	string name;				//���Ӧ�ı��������������
	buffer_type type;			//���Ӧ������(TABLE��INDEX)
	char* block;				//��������׵�ַ
	unsigned int block_page;	//���ڶ�Ӧ���ļ��е�ҳ����
	bool isChanged;				//���Ƿ񱻸ı�
	bool isUsing;				//���Ƿ�����ʹ��
	unsigned int LRU_count;		//����LRU�㷨�ļ���

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
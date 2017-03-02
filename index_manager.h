#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include "MiniSQL.h"
#include "buffer_manager.h"

using namespace std;

//������ʾB+���е�ֵ-ָ���
struct value_pointer
{
	string value;
	int pointer;
};

//������ʾB+���е�һ���ڵ�
struct node
{
	node_type type;				//�ڵ�����
	int SelfBlock_num;			//������
	int SelfPointer;			//ָ�������ָ��(ҳ��)
	int len;					//�ڵ��е�ֵ����
	int value_num;				//�ڵ��е�ֵ����
	int LastPointer;			//�ڵ�����һ��ָ��
	int ParentBlockPointer;		//�ڵ�ĸ��ڵ�ָ��
	vector<value_pointer> V_P;	//�ڵ��е�ֵ-ָ���

};

class IndexManager : public BufferManager
{
public:
	IndexManager() {};
	~IndexManager() {};

	static node GetNode(int block_num, int len);
	static void WriteNode(node NewNode);
	static void InsertIndex(string table_name, string attr_name, string value,int offset);
	static void DeleteIndex(string table_name, string attr_name, string value);
	static void delete_entry(string block_name, node Node, string value,attr_type type);
	static int FindLeaf(string block_name, int Page, int attr_len, string value, attr_type type);
	static void InsertInLeaf(int block_num, string value, int len, int offset, attr_type a_type);
	static void InsertInParent(node Node_1, string min_value, node Node_2, string block_name);
	static int GetNewBlockSite(string block_name);
	static void ChangeParentPointer(string block_name,int len,int SelfPage, int ParentPage);
	static void AddBlockNum(string block_name);
	static void MinuBlockNum(string block_name);
	static vector<int> SelectIndex(string table_name, condition cond);
	static vector<int> GetSection(string block_name,node use_node, string value, string mode,attr_type a_type);
	static int FindBeginBlock(string block_name, int len);
	static void FreeMapSite(string block_name, int Page);
	static error_type UniqueCheck(string table_name, condition cond);
	static int GetNum(string block_name);
};

int FindSite(char *map);
#endif
#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include "MiniSQL.h"
#include "buffer_manager.h"

using namespace std;

//用来表示B+树中的值-指针对
struct value_pointer
{
	string value;
	int pointer;
};

//用来表示B+树中的一个节点
struct node
{
	node_type type;				//节点类型
	int SelfBlock_num;			//自身块号
	int SelfPointer;			//指向自身的指针(页数)
	int len;					//节点中的值长度
	int value_num;				//节点中的值数量
	int LastPointer;			//节点的最后一个指针
	int ParentBlockPointer;		//节点的父节点指针
	vector<value_pointer> V_P;	//节点中的值-指针对

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
#include "buffer_manager.h"

using namespace std;

vector<BufferManager> BufferManager::BlockSet;

//初始化所有的块
void BufferManager::initiate_BlockSet()
{
	for (int i = 0; i < MAX_BLOCK; ++i)
	{
		BufferManager tmp;
		tmp.name = "";
		tmp.type = UNDEFINE;
		tmp.block = nullptr;
		tmp.block_page = 0;
		tmp.isChanged = false;
		tmp.isUsing = false;
		tmp.LRU_count = 0;
		BlockSet.push_back(tmp);
	}
}

//获得所需要的块
unsigned int BufferManager::get_block(buffer_type type, string name, unsigned int block_page)
{
	unsigned int block_num;
	string file_name;
	fstream fs;

	//查找块集中是否已经存在需要使用的块，如果存在直接返回块号
	for (int i = 0; i < MAX_BLOCK; ++i)
	{
		if (BlockSet[i].name == name && BlockSet[i].type == type && BlockSet[i].block_page == block_page)
		{
			using_block(i);
			return i;
		}
	}

	//如果需要使用的块还未曾读取过，获取一个新的块并向其中写入内容
	block_num = get_blank_block();
	BlockSet[block_num].type = type;
	BlockSet[block_num].name = name;
	BlockSet[block_num].block_page = block_page;
	BlockSet[block_num].LRU_count = 0;

	if (type == TABLE)
		file_name = "tab/" + name + ".tab";
	else if (type == INDEX)
		file_name = "idx/" + name + ".idx";
	else if (type == MAP)
		file_name = "map/" + name + ".map";

	fs.open(file_name.c_str(), ios::in | ios::binary);
	fs.seekp(block_page * BLOCK_SIZE);
	fs.read(BlockSet[block_num].block, BLOCK_SIZE);
	fs.close();

	return block_num;
}

//获取一个新的块
unsigned int BufferManager::get_blank_block()
{
	unsigned int block_num = -1;

	for (int i = 0; i < MAX_BLOCK; ++i)
	{
		//如果找到没有使用过的块则直接使用
		if (BlockSet[i].type == UNDEFINE)
		{
			BlockSet[i].block = new char[BLOCK_SIZE];
			block_num = i;
			break;
		}
	}

	//如果所有的块都已经写入过内容则通过LRU算法找出应该覆盖的块使用
	if (block_num == -1)
	{
		block_num = LRU_number();
		BlockSet[block_num].flush_block();
	}

	using_block(block_num);
	return block_num;
}

//找到在LRU算法下应该使用的块号
unsigned int BufferManager::LRU_number()
{
	unsigned int max_LRU = 0;
	unsigned int LRU_num = 0;

	//找出LRU标记最大的块覆盖使用
	for (unsigned int i = 0; i < MAX_BLOCK; ++i)
	{
		if (BlockSet[i].LRU_count > max_LRU)
		{
			LRU_num = i;
			max_LRU = BlockSet[i].LRU_count;
		}
	}
	return LRU_num;
}

//当使用一个块时，对其进行标记并更新其他块的LRU标记
void BufferManager::using_block(unsigned int block_num)
{
	BlockSet[block_num].isUsing = true;
	BlockSet[block_num].LRU_count = 0;

	for (unsigned int i = 0; i < MAX_BLOCK; i++)
		if (BlockSet[i].isUsing = false)
			BlockSet[i].LRU_count++;
}

//将一个块写回文件
void BufferManager::flush_block()
{
	fstream fs;
	string file_name;

	if (isChanged && type != UNDEFINE)
	{
		if (type == TABLE)
			file_name = "tab/" + name + ".tab";
		else if (type == INDEX)
			file_name = "idx/" + name + ".idx";
		else if (type == MAP)
			file_name = "map/" + name + ".map";

		fs.open(file_name, ios::out | ios::in);
		fs.seekp(block_page * BLOCK_SIZE);
		fs.write(block, BLOCK_SIZE);
		fs.close();
		this->type = UNDEFINE;
	}
}

//将块集中的所有块写回文件
void BufferManager::flush_all()
{
	for (int i =0; i <  BlockSet.size(); ++i)
	{
		BlockSet[i].flush_block();
	}
}

//比较两个值
bool ValueCompare(string value, attr_type type, cond_type cond, string comvalue)
{
	int int_value, int_comvalue;
	double float_value, float_comvalue;
	value.erase(value.find_last_not_of(" ") + 1);
	switch (type)
	{
	case INT:
		int_value = atoi(value.c_str());
		int_comvalue = atoi(comvalue.c_str());
		if (cond == LT)
			return int_value < int_comvalue;
		else if (cond == GT)
			return int_value > int_comvalue;
		else if (cond == GE)
			return int_value >= int_comvalue;
		else if (cond == LE)
			return int_value <= int_comvalue;
		else if (cond == EQ)
			return int_value == int_comvalue;
		else if (cond == NE)
			return int_value != int_comvalue;
		break;
	case FLOAT:
		float_value = atof(value.c_str());
		float_comvalue = atof(comvalue.c_str());
		if (cond == LT)
			return float_value < float_comvalue;
		else if (cond == GT)
			return float_value > float_comvalue;
		else if (cond == GE)
			return float_value >= float_comvalue;
		else if (cond == LE)
			return float_value <= float_comvalue;
		else if (cond == EQ)
			return float_value == float_comvalue;
		else if (cond == NE)
			return float_value != float_comvalue;
		break;
	case CHAR:
		if (cond == LT)
			return value < comvalue;
		else if (cond == GT)
			return value > comvalue;
		else if (cond == GE)
			return value >= comvalue;
		else if (cond == LE)
			return value <= comvalue;
		else if (cond == EQ)
			return value == comvalue;
		else if (cond == NE)
			return value != comvalue;
		break;
	default:
		return 0;
	}
	return 0;
}
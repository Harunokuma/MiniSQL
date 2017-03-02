#include "record_manager.h"

using namespace std;

//将值插入表的数据中,返回插入值的偏移量
unsigned int RecordManager::InsertValue(string name, vector<insertvalue> value)
{
	int mapblock_num, tabblock_num;					//储存map块和table块的块号
	int map_page = 0, tab_page;						//储存map文件和tab文件的页序数
	int map_offset = 0,tab_offset;					//储存map块和tab块中的偏移量
	int total_len;									//储存一条记录的总长度
	int record_per_page;							//储存每页的记录条数
	char mark;										//储存map文件中的标记
	char *write_site;								//块内容的写入点
	stringstream tmp;								//记录的临时存储
	string buffer;

	//从map文件的第一个块中获取记录长度和每页记录数
	mapblock_num = get_block(MAP, name, map_page);
	for (int i = 0; BlockSet[mapblock_num].block[i] != '#'; ++i)
		buffer += BlockSet[mapblock_num].block[i];
	total_len = atoi(buffer.c_str());
	record_per_page = BLOCK_SIZE / total_len;

	//从map文件的第二个块开始读取tab文件的位置信息
	++map_page;
	used_block(mapblock_num);
	mapblock_num = get_block(MAP, name, map_page);

	//寻找map文件中所对应的tab文件空位
	while (BlockSet[mapblock_num].block[map_offset] != '#' && BlockSet[mapblock_num].block[map_offset] != '0')
	{
		map_offset++;

		//如果找完了一个map块还没有找到，继续在下个块中找
		if (map_offset >= BLOCK_SIZE)
		{
			map_offset = 0;
			++map_page;
			used_block(mapblock_num);
			mapblock_num = get_block(MAP, name, map_page);
		}
	}

	//找到map文件里面有0（空位）或者#（结束位）
	tab_page = ((map_page - 1)*BLOCK_SIZE + map_offset) / record_per_page;
	tab_offset = ((map_page - 1)*BLOCK_SIZE + map_offset) % record_per_page;
	mark = BlockSet[mapblock_num].block[map_offset];

	if (map_offset == BLOCK_SIZE - 1)	//结束符位于块末尾
	{
		BlockSet[mapblock_num].block[map_offset] = '1';
		map_offset = 0;
		++map_page;
		used_block(mapblock_num);
		mapblock_num = get_block(MAP, name, map_page);
		BlockSet[mapblock_num].isChanged = true;
		if(mark == '#')
			BlockSet[mapblock_num].block[map_offset] = '#';
	}
	else								//结束符不位于块末尾
	{
		BlockSet[mapblock_num].block[map_offset] = '1';
		++map_offset;
		BlockSet[mapblock_num].isChanged = true;
		if (mark == '#')
			BlockSet[mapblock_num].block[map_offset] = '#';
	}

	//将值写入表块的空位中
	tabblock_num = get_block(TABLE, name, tab_page);
	write_site = BlockSet[tabblock_num].block + tab_offset*total_len;
	for (int j = 0; j < value.size(); ++j)
		tmp << setw(value[j].length) << setfill(' ') << left << value[j].value;
	strcpy(write_site, tmp.str().c_str());
	BlockSet[tabblock_num].isChanged = true;
	used_block(tabblock_num);

	return tab_page*record_per_page + tab_offset;
}

//根据表名和对于记录条数的偏移量来删除一条记录
error_type RecordManager::DeleteValue(string name, int record_offset)
{
	int mapblock_num;								//储存map块的块号
	int map_page = 0;								//储存map文件的页序数
	int map_offset = 0;								//储存map块偏移量

	map_page = record_offset / BLOCK_SIZE +1;
	map_offset = record_offset % BLOCK_SIZE;
	
	mapblock_num = get_block(MAP, name, map_page);
	BlockSet[mapblock_num].block[map_offset] = '0';
	BlockSet[mapblock_num].isChanged = true;
	used_block(mapblock_num);
	return NOERR;
}

//在数据库中搜索符合条件的数据，返回它们相对于记录条数的偏移量
vector<int> RecordManager::SelectValue(string name, vector<condition> conds)
{
	int mapblock_num, tabblock_num;					//储存map块和table块的块号
	int map_page = 0, tab_page;						//储存map文件和tab文件的页序数
	int map_offset = 0, tab_offset;					//储存map块和tab块中的偏移量
	int total_len;									//储存一条记录的总长度
	int record_per_page;							//储存每页的记录条数
	vector<int> result_offset;						//储存查询结果的偏移
	string buffer;

	//从map文件的第一个块中获取记录长度和每页记录数
	mapblock_num = get_block(MAP, name, map_page);
	for (int i = 0; BlockSet[mapblock_num].block[i] != '#'; ++i)
		buffer += BlockSet[mapblock_num].block[i];
	total_len = atoi(buffer.c_str());
	record_per_page = BLOCK_SIZE / total_len;

	//从map文件的第二个块开始读取tab文件的位置信息
	++map_page;
	used_block(mapblock_num);
	mapblock_num = get_block(MAP, name, map_page);

	while (BlockSet[mapblock_num].block[map_offset] != '#')		//通过map找到tab文件中有效的记录位置
	{
		bool flag = true;
		stringstream sst;

		if (BlockSet[mapblock_num].block[map_offset] == '1')
		{				
			tab_page = ((map_page - 1)*BLOCK_SIZE + map_offset) / record_per_page;			//计算记录的页
			tab_offset = ((map_page - 1)*BLOCK_SIZE + map_offset) % record_per_page;		//计算记录的相对偏移
			
			tabblock_num = get_block(TABLE, name, tab_page);								//读取该页的记录
			sst.write(BlockSet[tabblock_num].block + tab_offset*total_len, total_len);

			//判断该记录是否符合条件，如果是，则将其绝对偏移加入结果中
			for (int i = 0; i < conds.size(); ++i)
				if (!ValueCompare(sst.str().substr(conds[i].attr_offset, conds[i].attr_length), conds[i].type, conds[i].cond, conds[i].value))
					flag = false;
			if (flag == true)
				result_offset.push_back(tab_offset + tab_page*record_per_page);
			used_block(tabblock_num);
		}

		//如果map读到了当前页的最后一个，则进入下一页
		if (map_offset == BLOCK_SIZE - 1)
		{
			map_offset = 0;
			map_offset = 0;
			++map_page;
			used_block(mapblock_num);
			mapblock_num = get_block(MAP, name, map_page);
		}

		//将map中的位置向前移一位
		else
			++map_offset;
	}
	used_block(mapblock_num);
	return result_offset;
}

//为结果集寻找交集
vector<int> FindIntersection(vector<int> result_1, vector<int> result_2)
{
	int i = 0, j = 0;
	vector<int> result;			//储存交集

	//如果有一个结果集遍历完则结束
	while (j < result_2.size() && i < result_1.size())
	{
		if (result_1[i] == result_2[j])
		{
			result.push_back(result_1[i]);
			++i;
			++j;
		}
		else if (result_1[i] > result_2[j])
			++j;
		else if (result_1[i] < result_2[j])
			++i;
	}
	return result;
}

//根据偏移集和表名来打印查询结果
void RecordManager::PrintResult(vector<int> result, string table_name)
{
	int mapblock_num, tabblock_num;					//储存map块和table块的块号
	int map_page = 0, tab_page;						//储存map文件和tab文件的页序数
	int map_offset = 0, tab_offset;					//储存map块和tab块中的偏移量
	int total_len;									//储存一条记录的总长度
	int record_per_page;							//储存每页的记录条数
	vector<int> result_offset;						//储存查询结果的偏移
	vector<int> attr_len;							//储存属性的长度
	int len_sum = 0;
	string buffer;
	int count;

	//从map文件的第一个块中获取记录长度和每页记录数
	mapblock_num = get_block(MAP, table_name, map_page);
	for (count = 0; BlockSet[mapblock_num].block[count] != '#'; ++count)
		buffer += BlockSet[mapblock_num].block[count];
	total_len = atoi(buffer.c_str());
	buffer.clear();
	++count;
	while (total_len != len_sum)
	{
		int len;
		while (BlockSet[mapblock_num].block[count] != '#')
		{
			buffer += BlockSet[mapblock_num].block[count];
			++count;
		}
		len = atoi(buffer.c_str());
		attr_len.push_back(len);
		len_sum += len;
		++count;
		buffer.clear();
	}

	record_per_page = BLOCK_SIZE / total_len;
	used_block(mapblock_num);

	//打印查询结果
	for (int i = 0; i < result.size(); ++i)
	{
		stringstream sst, tmp;
		int used_len = 0;

		tab_page = result[i] / record_per_page;
		tab_offset = result[i] % record_per_page;
		tabblock_num = get_block(TABLE, table_name, tab_page);
		sst << BlockSet[tabblock_num].block;
		for (int j = 0; j < attr_len.size(); ++j)
		{ 
			tmp << setw(attr_len[j] + 4) << setfill(' ') << left << sst.str().substr(tab_offset*total_len + used_len, attr_len[j]);
			tmp << "|";
			used_len += attr_len[j];
		}
		cout << tmp.str() << endl;
	}
}

//根据记录文件来查询是否违反了唯一性
error_type RecordManager::UniqueCheck(string table_name, condition cond)
{
	vector<int> result;
	vector<condition> tmp;

	tmp.push_back(cond);
	result = SelectValue(table_name, tmp);

	if (result.size() > 0)
		return UNIERR;
	else
		return NOERR;
}

//根据记录的偏移量来获取记录中的某个特定属性的值
string RecordManager::GetValue(string table_name, column col, int offset)
{
	int mapblock_num, tabblock_num;					//储存map块和table块的块号
	int map_page = 0, tab_page;						//储存map文件和tab文件的页序数
	int tab_offset;									//储存tab块中的偏移量
	int total_len;									//储存一条记录的总长度
	int record_per_page;							//储存每页的记录条数
	string buffer;
	stringstream sst;

	//从map文件的第一个块中获取记录长度和每页记录数
	mapblock_num = get_block(MAP, table_name, map_page);
	for (int i = 0; BlockSet[mapblock_num].block[i] != '#'; ++i)
		buffer += BlockSet[mapblock_num].block[i];
	total_len = atoi(buffer.c_str());
	record_per_page = BLOCK_SIZE / total_len;
	used_block(mapblock_num);

	//根据偏移量和表结构来获取值
	tab_page = offset / record_per_page;
	tab_offset = offset % record_per_page;
	tabblock_num = get_block(TABLE, table_name, tab_page);
	sst << BlockSet[tabblock_num].block;
	return sst.str().substr(tab_offset*total_len + col.col_offset, col.col_length);
}
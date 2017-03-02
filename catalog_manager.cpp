#include "catalog_manager.h"

//从Catalog文件中读取表和索引的结构信息
void CatalogManager::ReadCatalog()
{
	clear();
	fstream fs;
	char buffer[100];
	string bufstr;

	/*tables.catalog
	 *0	table name
	 *1	primary key
	 *2	num of attr
	 *3	attr name
	 *4	attr type and length
	 *5	attr isUnique
	 *6	attr name
	 *	...
	 *	table name
	 *	primary key
	 *	...
	 *	-1
	 */

	//打开储存表信息的文件，如果文件不存在，则新建一个内容为-1的文件
	fs.open("catalog/tables.catalog", ios::in);
	if (fs.is_open() ==false)
	{
		fs.open("catalog/tables.catalog", ios::out);
		fs << "-1\n";
	}
	fs.close();

	fs.open("catalog/tables.catalog", ios::in);
	fs.getline(buffer, 100);
	bufstr = buffer;
	while (bufstr != "-1")				//判断是否已经读取完毕
	{
		table tmp_tab;					//储存表信息的结构
		unsigned int offset = 0;		//属性的偏移量

		tmp_tab.table_name = buffer;	//从文件中获取表名
		fs.getline(buffer, 100);
		tmp_tab.primary = buffer;		//从文件中获取主键
		fs.getline(buffer, 100);
		tmp_tab.col_num = atoi(buffer);	//从文件中获取属性数量

		for (int i = 0; i < tmp_tab.col_num; ++i)
		{
			column tmp_col;
			int type;
			fs.getline(buffer, 100);
			tmp_col.col_name = buffer;	//从文件中获取属性名
			fs.getline(buffer, 100);
			type = atoi(buffer);		//从文件中获取属性类型和长度

			if (type == 0)
			{
				tmp_col.type = INT;
				tmp_col.col_offset = offset;
				tmp_col.col_length = INT_LEN;
			}
			else if (type == 1)
			{
				tmp_col.type = FLOAT;
				tmp_col.col_offset = offset;
				tmp_col.col_length = FLOAT_LEN;
			}
			else
			{
				tmp_col.type = CHAR;
				tmp_col.col_offset = offset;
				tmp_col.col_length = type - 1;
			}
			offset += tmp_col.col_length;	//更新偏移量

			//从文件中获取该属性是否为主键
			if (tmp_col.col_name == tmp_tab.primary)
				tmp_col.isPrimary = true;
			else
				tmp_col.isPrimary = false;

			//从文件中获取该属性是否唯一
			fs.getline(buffer, 100);
			if (atoi(buffer) == 1)
				tmp_col.isUnique = true;
			else
				tmp_col.isUnique = false;

			//将属性存入表的信息结构中
			tmp_tab.cols.push_back(tmp_col);
		}
		fs.getline(buffer, 100);
		bufstr = buffer;

		//将表的信息存入CatalogManager中
		table_set.push_back(tmp_tab);
	}
	fs.close();

	/*index.catalog
	*0	index name
	*1	table name
	*2	attr name
	*3	index name
	*4	table name
	*5	attr name
	*6	...
	*	-1
	*/

	//打开储存索引信息的文件，如果文件不存在，则新建一个内容为-1的文件
	fs.open("catalog/index.catalog", ios::in);
	if (fs.is_open() == false)
	{
		fs.open("catalog/index.catalog", ios::out);
		fs << "-1\n";
	}
	fs.close();

	fs.open("catalog/index.catalog", ios::in);
	fs.getline(buffer, 100);
	bufstr = buffer;
	while (bufstr != "-1")				//判断是否已经读取完毕
	{
		index tmp_idx;
		tmp_idx.index_name = buffer;
		fs.getline(buffer, 100);
		tmp_idx.table_name = buffer;
		fs.getline(buffer, 100);
		tmp_idx.attr_name = buffer;
		index_set.push_back(tmp_idx);
		fs.getline(buffer, 100);
		bufstr = buffer;
	}
	fs.close();
}

//将更改过的表和索引信息写入Catalog文件中
void CatalogManager::UpdateCatalog()
{
	fstream fs;

	//更新表记录文件
	fs.open("catalog/tables.catalog", ios::out);

	for (int i = 0; i < table_set.size(); ++i)
	{
		char buffer[100];
		int char_len;
		fs << table_set[i].table_name << endl;			//更新表名
		fs << table_set[i].primary << endl;				//更新主键
		_itoa_s(table_set[i].col_num, buffer, 10);		//更新属性数量
		fs << buffer << endl;

		//更新属性信息
		for (int j = 0; j < table_set[i].col_num; ++j)
		{
			fs << table_set[i].cols[j].col_name << endl;

			if (table_set[i].cols[j].type == INT)
				fs << "0" << endl;
			else if (table_set[i].cols[j].type == FLOAT)
				fs << "1" << endl;
			else
			{
				char_len = table_set[i].cols[j].col_length + 1;
				_itoa_s(char_len, buffer, 10);
				fs << buffer << endl;
			}
			if (table_set[i].cols[j].isUnique == true)
				fs << "1" << endl;
			else
				fs << "0" << endl;
		}
	}
	fs << "-1" << endl;
	fs.close();

	//更新索引信息
	fs.open("catalog/index.catalog", ios::out);

	for (int i = 0; i < index_set.size(); ++i)
	{
		fs << index_set[i].index_name << endl;
		fs << index_set[i].table_name << endl;
		fs << index_set[i].attr_name << endl;
	}
	fs << "-1" << endl;
	fs.close();
}

//新建一个表的信息文件
error_type CatalogManager::CreateTable(table NewTable)
{
	//如果表已经存在，返回错误信息
	if (FindTable(NewTable.table_name) == true)
		return TABEXT;

	//将新表的信息存入表信息集合中
	table_set.push_back(NewTable);

	fstream fs;									//文件流
	string file_name;							//文件名
	int total_len = 0;							//一条记录的总长度
	char len[100];

	/*table_name.map
	 *Block0:
	 *|total_len|#|len1|#|len2|#...|len...|#
	 *
	 *Block1:
	 *1111010101111.....#
	 * ↑    ↑           ↑
	 *used free        end
	 */

	/*table_name.tab
	 *|record1|record2|record3|.....
	 */

	file_name = "tab/" + NewTable.table_name + ".tab";
	fs.open(file_name, ios::out);
	fs.close();

	file_name = "map/" + NewTable.table_name + ".map";
	fs.open(file_name, ios::out);
	for (int i = 0; i < NewTable.col_num; ++i)
		total_len += NewTable.cols[i].col_length;
	_itoa_s(total_len, len, 10);
	fs << string(len) << "#";					//map文件的第一个数字代表表中一条记录的总长度，之后用#分隔

	//map文件中记录总长度之后是每个属性的长度
	for (int i = 0; i < NewTable.col_num; ++i)
		fs << NewTable.cols[i].col_length << "#";

	//map文件在第二个块里开始储存tab文件的使用信息
	fs.seekp(BLOCK_SIZE, ios::beg);
	fs << "#";									//map文件中'1'代表已使用，'0'代表未使用，'#'代表文件结束
	fs.close();

	//对表中的主键建立索引
	for (auto it = NewTable.cols.begin(); it != NewTable.cols.end(); ++it)
	{
		if (it->isPrimary == true)
		{
			index tmp;
			tmp.attr_len = it->col_length;
			tmp.attr_name = it->col_name;
			tmp.attr_type = it->type;
			tmp.index_name = NewTable.table_name + "_" + it->col_name;
			tmp.table_name = NewTable.table_name;
			CreateIndex(tmp);
			break;
		}
	}
	return NOERR;
}

//新建一个索引的信息文件
error_type CatalogManager::CreateIndex(index NewIndex)
{
	if (FindIndex(NewIndex.table_name, NewIndex.attr_name) == true)
		return IDXEXT;

	//将新索引的信息加入索引集中
	index_set.push_back(NewIndex);

	fstream fs;									//文件流
	string file_name;							//文件名
	stringstream sst;

	/*tabname_attrname.idx
	 *Block0:
	 *|attr_len(4 byte)|block_num(4 byte)|type(1 byte)|map(1:used 0:free #:end)|
	 *
	 *Block1:
	 *|node_type(1 byte)|parent_block(4 byte)|self_pointer(4 byte)|record_num(4 byte)|
	 *|record...|
	 *
	 */

	//新建一个.idx文件，并对其进行初始化
	file_name = "idx/" + NewIndex.table_name + "_" + NewIndex.attr_name + ".idx";
	fs.open(file_name, ios::out);
	sst << setw(4) << setfill(' ') << left << NewIndex.attr_len;
	sst << setw(4) << setfill(' ') << left << "0";
	sst << NewIndex.attr_type;
	sst << "#";
	fs << sst.str();
	fs.close();
}

//删除一个表的所有信息
error_type CatalogManager::DropTable(string table_name)
{
	//将表的信息从表集删除，并删除文件
	string tabfile_name = "tab/" + table_name + ".tab";
	string mapfile_name = "map/" + table_name + ".map";
	bool flag = false;									//判断是否有此表

	//删除表的catalog信息和tab文件和map文件
	for (auto it = table_set.begin(); it != table_set.end(); ++it)
	{
		if (it->table_name == table_name)
		{
			flag = true;
			table_set.erase(it);
			remove(tabfile_name.c_str());
			remove(mapfile_name.c_str());
			break;
		}
	}

	//删除与该表有关的索引的catalog信息和idx文件
	for (auto it = index_set.begin(); it != index_set.end();)
	{
		string idxfile_name;
		if (it->table_name == table_name)
		{
			idxfile_name = "idx/" + it->table_name + "_" + it->attr_name + ".idx";
			index_set.erase(it);
			remove(idxfile_name.c_str());
			it = index_set.begin();
		}
		else
			it++;
	}

	//如果不存在表，则返回错误信息
	if (flag == true)
		return NOERR;
	else
		return NOTAB;
}

//销毁一个索引
error_type CatalogManager::DropIndex(string index_name)
{
	//删除与该索引的catalog信息和idx文件
	for (auto it = index_set.begin(); it != index_set.end(); ++it)
		if (it->index_name == index_name)
		{
			string file_name;

			file_name = "idx/" + it->table_name + "_" + it->attr_name + ".idx";
			index_set.erase(it);
			remove(file_name.c_str());
			return NOERR;
		}

	//如果不存在该索引，返回错误信息
	return NOIDX;
}

//查找表是否存在，是返回true，否返回false
bool CatalogManager::FindTable(string table_name)
{
	for (int i = 0; i < table_set.size(); ++i)
	{
		if (table_set[i].table_name == table_name)
			return true;
	}
	return false;
}

//查找索引是否存在，是返回true，否返回false
bool CatalogManager::FindIndex(string table_name, string attr_name)
{
	for (int i = 0; i < index_set.size(); ++i)
	{
		if (index_set[i].table_name == table_name && index_set[i].attr_name == attr_name)
			return true;
	}
	return false;
}

//查找插入的值是否与表的结构相匹配，并将插入值的长度和表结构同步
error_type CatalogManager::CheckValue(string name, vector<insertvalue> &values)
{
	int table_num = -1;
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == name)
		{
			table_num = i;
			break;
		}

	if (table_num == -1)
		return NOTAB;

	for (int i = 0; i < table_set[table_num].col_num; ++i)
	{
		if (values[i].type != table_set[table_num].cols[i].type)
			return VALERR;
		else if (values[i].length > table_set[table_num].cols[i].col_length)
			return VALERR;
		values[i].length = table_set[table_num].cols[i].col_length;
	}
	return NOERR;
}

//清空表集和索引集
void CatalogManager::clear()
{
	table_set.clear();
	index_set.clear();
}

//利用Catalog对Cond进行补完
void CatalogManager::FillCond(vector<condition> &cond, string table_name)
{
	table tmp;

	//获取table信息
	for (int i = 0; i < table_set.size(); ++i)
	{
		if (table_set[i].table_name == table_name)
		{
			tmp = table_set[i];
			break;
		}
	}

	//添加cond中的信息（类型、长度、偏移等）
	for (int i = 0; i < cond.size(); ++i)
	{
		for (int j = 0; j < tmp.cols.size(); ++j)
		{
			if (tmp.cols[j].col_name == cond[i].attr_name)
			{
				cond[i].type = tmp.cols[j].type;
				cond[i].attr_length = tmp.cols[j].col_length;
				cond[i].attr_offset = tmp.cols[j].col_offset;
			}
		}
	}
}

//根据catalog设置验证唯一性的condition
vector<condition> CatalogManager::UniqueSet(string table_name, vector<insertvalue> values)
{
	int table_num;
	vector<condition> conds;

	//找到当前需要的table信息
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == table_name)
		{
			table_num = i;
			break;
		}

	//如果属性是unique或者primary key，则将其读入conds中
	for (int i = 0; i < table_set[table_num].cols.size(); ++i)
	{
		if (table_set[table_num].cols[i].isPrimary == true || table_set[table_num].cols[i].isUnique == true)
		{
			condition tmp;
			tmp.attr_name = table_set[table_num].cols[i].col_name;
			tmp.value = values[i].value;
			tmp.attr_length = table_set[table_num].cols[i].col_length;
			tmp.attr_offset = table_set[table_num].cols[i].col_offset;
			tmp.type = table_set[table_num].cols[i].type;
			tmp.cond = EQ;
			conds.push_back(tmp);
		}
	}
	return conds;
}

//获取一个table下所有index的信息
vector<index> CatalogManager::GetIndexInfo(string table_name)
{
	vector<index> indexes;
	for (auto it = index_set.begin(); it != index_set.end(); ++it)
		if (it->table_name == table_name)
			indexes.push_back(*it);
	return indexes;
}

//获取一个属性在table中是第几个属性
int CatalogManager::FindPosition(string table_name, string attr_name)
{
	for (auto it = table_set.begin(); it != table_set.end(); ++it)
		if (it->table_name == table_name)
			for (int i = 0; i < it->cols.size(); ++i)
				if (it->cols[i].col_name == attr_name)
					return i;
}

//返回属性是否唯一
bool CatalogManager::isUnique(string table_name, string attr_name)
{
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == table_name)
			for (int j = 0; j < table_set[i].cols.size(); ++j)
				if (table_set[i].cols[j].col_name == attr_name)
				{
					if (table_set[i].cols[j].isUnique == true)
						return true;
					else
						return false;
				}
	return false;
}

//获取属性信息
column CatalogManager::GetColInfo(string table_name, string attr_name)
{
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == table_name)
			for (int j = 0; j < table_set[i].cols.size(); ++j)
				if (table_set[i].cols[j].col_name == attr_name)
					return table_set[i].cols[j];
}

//打印一个表的结构
void CatalogManager::PrintRecordForm(string table_name)
{
	table tmp;
	
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == table_name)
		{
			tmp = table_set[i];
			break;
		}

	cout << tmp.table_name << " :" << endl;
	for (int i = 0; i < tmp.cols.size(); ++i)
	{
		cout << setw(tmp.cols[i].col_length + 4) << setfill(' ') << left << tmp.cols[i].col_name;
		cout << "|";
	}
	cout << endl;
}

//检查条件是否符合表的结构
error_type CatalogManager::CheckCond(string table_name, vector<condition> conds)
{
	vector<column> cols;

	for (auto it = table_set.begin(); it != table_set.end(); ++it)
	{
		if (it->table_name == table_name)
		{
			cols = it->cols;
			break;
		}
	}
	if (cols.size() == 0)
		return NOTAB;

	for (auto it = conds.begin(); it != conds.end(); ++it)
	{
		bool flag = false;
		for (auto it_col = cols.begin(); it_col != cols.end(); ++it_col)
		{
			if (it_col->col_name == it->attr_name && it_col->type == it->type)
			{
				flag = true;
				break;
			}
		}
		if (flag == false)
			return CONERR;
	}
	return NOERR;
}
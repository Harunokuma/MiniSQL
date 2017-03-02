#include "API.h"
#include "catalog_manager.h"
#include "record_manager.h"
#include "index_manager.h"

static CatalogManager CM;

void Command::initiate()
{
	BufferManager::initiate_BlockSet();
	CM.ReadCatalog();
}

void Command::end()
{
	BufferManager::flush_all();
	CM.UpdateCatalog();
}

void Command::setOperation(op_type type)
{
	operation = type;
}

void Command::setTableName(string name)
{
	table_name = name;
}

void Command::setIndexName(string name)
{
	index_name = name;
}

void Command::setIndexAttr(string attr_name)
{
	index_attr = attr_name;
}

void Command::test()
{
	condition tmp;
	vector<condition> conds;
	vector<int> result;

	tmp.attr_name = "a";
	tmp.cond = GT;
	tmp.value = "5";
	tmp.type = INT;
	tmp.attr_length = INT_LEN;
	tmp.attr_offset = 0;
	conds.push_back(tmp);
	BufferManager::initiate_BlockSet();
	result = IndexManager::SelectIndex("tab", tmp);
	for (int i = 0; i < result.size(); ++i)
		cout << result[i] << endl;
	BufferManager::flush_all();
}

void Command::clear()
{
	operation = UNKNOWN;
	table_name = "";
	index_name = "";
	file_name = "";
	index_attr = "";
	cols.clear();
	conds.clear();
	values.clear();
}

void Command::setCols(vector<string> attr_name, vector<attr_type> attr_type, vector<int> char_length, vector<bool> isUnique, string primary)
{
	unsigned int offset = 0;
	for (unsigned int i = 0; i < attr_name.size(); ++i)
	{
		column temp;
		temp.col_name = attr_name[i];
		temp.type = attr_type[i];
		//存储属性长度
		if (temp.type == CHAR)
			temp.col_length = char_length[i];
		else if (temp.type == INT)
			temp.col_length = INT_LEN;
		else if (temp.type == FLOAT)
			temp.col_length = FLOAT_LEN;
		//存储属性偏移
		temp.col_offset = offset;
		offset += temp.col_length;

		if (temp.col_name == primary)
			temp.isPrimary = true;
		else
			temp.isPrimary = false;
		temp.isUnique = isUnique[i];
		cols.push_back(temp);
	}
}
void Command::setConds(vector<string> attr_name, vector<cond_type> cond, vector<string> const_value, vector<attr_type> attr_type)
{
	for (unsigned int i = 0; i < attr_name.size(); ++i)
	{
		condition temp;
		temp.attr_name = attr_name[i];
		temp.type = attr_type[i];
		temp.cond = cond[i];
		temp.value = const_value[i];
		conds.push_back(temp);
	}
}
void Command::setValues(vector<string> insert_value, vector<attr_type> attr_type)
{
	for (unsigned int i = 0; i < insert_value.size(); ++i)
	{
		insertvalue temp;
		temp.value = insert_value[i];
		temp.type = attr_type[i];
		if (temp.type == CHAR)
			temp.length = insert_value[i].size();
		else if (temp.type == INT)
			temp.length = INT_LEN;
		else if (temp.type == FLOAT)
			temp.length = FLOAT_LEN;
		values.push_back(temp);
	}
}

string Command::FindPrimary()
{
	for (unsigned int i = 0; i < cols.size(); ++i)
		if (cols[i].isPrimary == 1)
			return cols[i].col_name;
	return "NO PRIMARY!";
}

//执行函数，根据不同的operation来执行不同的操作
error_type Command::exec()
{
	error_type err_info;
	
	switch (operation)
	{
	case CRETAB:
		err_info = exec_cretab();
		break;
	case DRPTAB:
		err_info = exec_drptab();
		break;
	case CREIDX:
		err_info = exec_creidx();
		break;
	case DRPIDX:
		err_info = exec_drpidx();
		break;
	case SELECT:
		err_info = exec_select();
		break;
	case INSERT:
		err_info = exec_insert();
		break;
	case DELETE:
		err_info = exec_delete();
		break;
	default:
		return OPERR;
	}
	return err_info;
}

//新建表的操作
error_type Command::exec_cretab()
{
	table tmp;						//储存表结构信息
	error_type err;					//储存错误信息

	tmp.table_name = table_name;
	tmp.col_num = cols.size();
	tmp.cols = cols;
	tmp.primary = FindPrimary();

	err = CM.CreateTable(tmp);		//新增表信息
	if (err != NOERR)
		return err;
	return NOERR;
}

//销毁一个表的操作
error_type Command::exec_drptab()
{
	error_type err;					//储存错误信息

	BufferManager::flush_all();
	err = CM.DropTable(table_name);	//删除表信息
	if (err != NOERR)
		return err;
	else 
		return NOERR;
}

//创建一个索引的操作
error_type Command::exec_creidx()
{
	vector<int> result;
	column col;
	index NewIndex;

	//是否已经存在该索引
	if (CM.FindIndex(table_name, index_attr) == true)
		return IDXEXT;

	//是否违反属性唯一性
	if (CM.isUnique(table_name, index_attr) == false)
		return UNIERR;
	
	col = CM.GetColInfo(table_name, index_attr);		//获取索引的属性的信息

	//构建新索引的信息
	NewIndex.table_name = table_name;
	NewIndex.index_name = index_name;
	NewIndex.attr_name = index_attr;
	NewIndex.attr_type = col.type;
	NewIndex.attr_len = col.col_length;
	CM.CreateIndex(NewIndex);
	
	result = RecordManager::SelectValue(table_name, conds);	//获取所有的记录的位置

	//对所有的记录进行索引的插入操作
	for (int i = 0; i < result.size(); ++i)
	{
		string value;
		value = RecordManager::GetValue(table_name, col, result[i]);	//获取记录中的特定属性的值
		
		IndexManager::InsertIndex(table_name, index_attr, value,result[i]);
	}

	return NOERR;
}

//销毁一个索引的操作
error_type Command::exec_drpidx()
{
	//通过CatalogManager销毁一个索引
	return CM.DropIndex(index_name);
}

//执行查询操作
error_type Command::exec_select()
{
	vector<int> result;					//储存查询结果的偏移量
	vector<int> tmp_result;				//临时储存查询结果
	vector<condition> RecordConds;		//储存需要直接在记录文件中搜索的条件
	error_type err;
	bool flag = false;

	err = CM.CheckCond(table_name, conds);	//检查条件和表的结构是否相符
	if (err != NOERR)
		return err;
	CM.FillCond(conds, table_name);			//对条件进行补完（长度、偏移）

	if (conds.size() == 0)
		result = RecordManager::SelectValue(table_name, conds);
	else
	{
		for (int i = 0; i < conds.size(); ++i)
		{
			//如果某条条件中的属性有索引，执行索引查询
			if (CM.FindIndex(table_name, conds[i].attr_name) == true)
			{
				tmp_result = IndexManager::SelectIndex(table_name, conds[i]);
				if (flag == false)
					result = FindIntersection(tmp_result, tmp_result);
				else if(flag == true)
					result = FindIntersection(result, tmp_result);
				flag = true;
			}
			//否则将该条件加入RecordConds
			else
				RecordConds.push_back(conds[i]);
	}

		//如果存在需要使用记录文件搜索的条件则调用RecordManager的搜索
		if (RecordConds.size() > 0)
		{
			tmp_result = RecordManager::SelectValue(table_name, RecordConds);
			if (flag == false)
				result = FindIntersection(tmp_result, tmp_result);
			else if (flag == true)
				result = FindIntersection(result, tmp_result);
		}
	}

	//如果结果中没有数据，则表示不存在该条件下的记录
	if (result.size() == 0)
		cout << "No data" << endl;

	//否则通过结果中的偏移量读取记录文件进行输出
	else
	{
		CM.PrintRecordForm(table_name);
		RecordManager::PrintResult(result, table_name);
	}

	return NOERR;
}

//执行记录的插入操作
error_type Command::exec_insert()
{
	error_type err_info;				//错误信息
	vector<condition> UniqueCond;		//唯一性检查的条件
	int offset;							//新插入记录的偏移量
	vector<index> indexes;

	//检查输入的插入值是否符合表的结构
	err_info = CM.CheckValue(table_name, values);
	if (err_info != NOERR)
		return err_info;

	//通过CatalogManager来建立唯一性检查的条件
	UniqueCond = CM.UniqueSet(table_name, values);

	//对记录中所有的数据与新插入的数据执行唯一性检查
	for (int i = 0; i < UniqueCond.size(); ++i)
	{
		if (CM.FindIndex(table_name, UniqueCond[i].attr_name) == true)
			err_info = IndexManager::UniqueCheck(table_name, UniqueCond[i]);
		else
			err_info = RecordManager::UniqueCheck(table_name, UniqueCond[i]);

		if (err_info != NOERR)
			return err_info;
	}

	//将新数据插入记录文件
	offset = RecordManager::InsertValue(table_name, values);

	//获取所有当前表所有的索引的信息
	indexes = CM.GetIndexInfo(table_name);

	//对所有的索引更新新插入的记录
	for (auto it = indexes.begin(); it != indexes.end(); ++it)
	{
		int position;
		position = CM.FindPosition(table_name, it->attr_name);		//获取属性在记录中的位置
		IndexManager::InsertIndex(table_name, it->attr_name, values[position].value, offset);
	}
	return NOERR;
}

//执行删除记录的操作
error_type Command::exec_delete()
{
	vector<int> result;
	vector<index> indexes;

	CM.FillCond(conds, table_name);			//对条件进行补完（长度、偏移）

	//获取所有当前表所有的索引的信息
	indexes = CM.GetIndexInfo(table_name);

	//获取需要删除的记录的偏移量
	result = RecordManager::SelectValue(table_name, conds);

	//根据偏移量逐个删除记录
	for (int i = 0; i < result.size(); ++i)
	{
		RecordManager::DeleteValue(table_name, result[i]);
		for (int j = 0; j < indexes.size(); j++)
		{
			column col;
			string value;

			col = CM.GetColInfo(table_name, indexes[j].attr_name);					//获取索引的属性的信息
			value = RecordManager::GetValue(table_name, col, result[i]);			//获取删除的值
			IndexManager::DeleteIndex(table_name, indexes[j].attr_name, value);		//根据值对索引进行删除
		}
	}
	return NOERR;
}
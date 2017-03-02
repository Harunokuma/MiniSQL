#include "Iterpreter.h"
#include "MiniSQL.h"
#include <iostream>
#include <regex>
#include <vector>
#include <fstream>
#include <iomanip>

Command API;

using namespace std;
void iterpreter()
{
	cout << "welcome to MiniSQL!" << endl;
	API.initiate();
	while(1)
	{
		string SQL_string, result;
		error_type iterpreter_error;
		cout << "MiniSQL-->";
		getline(cin, SQL_string);
		if (SQL_string == "quit;")
		{
			API.end();
			break;
		}
		else if (SQL_string == "test")
		{
			API.test();
			continue;
		}
		else if (SQL_string.find("exec") != string::npos)
		{
			iterpreter_error = exec_file(SQL_string);
			if(iterpreter_error != NOERR)
			{
				result = read_error(iterpreter_error);
				cout << result << endl;
			}
			continue;
		}
		iterpreter_error = SQL_to_Command(SQL_string);

		if (iterpreter_error == NOERR)	//成功转换
			result = read_error(API.exec());
		else
			result = read_error(iterpreter_error);
		cout << result << endl;
		API.clear();
	}
}

error_type SQL_to_Command(string SQL_string)
{
	string create_table_regex(CREATE_TABLE_REGEX);
	string drop_table_regex(DROP_TABLE_REGEX);
	string create_index_regex(CREATE_INDEX_REGEX);
	string drop_index_regex(DROP_INDEX_REGEX);
	string select_regex(SELECT_REGEX);
	string insert_regex(INSERT_REGEX);
	string delete_regex(DELETE_REGEX);

	regex r_create_table(create_table_regex, regex::icase);
	regex r_drop_table(drop_table_regex, regex::icase);
	regex r_create_index(create_index_regex, regex::icase);
	regex r_drop_index(drop_index_regex, regex::icase);
	regex r_select(select_regex, regex::icase);
	regex r_insert(insert_regex, regex::icase);
	regex r_delete(delete_regex, regex::icase);

	if(regex_match(SQL_string, r_create_table))
		return create_table_tras(SQL_string);
	else if(regex_match(SQL_string, r_drop_table))
		return drop_table_tras(SQL_string);
	else if(regex_match(SQL_string, r_create_index))
		return create_index_tras(SQL_string);
	else if(regex_match(SQL_string, r_drop_index))
		return drop_index_tras(SQL_string);
	else if(regex_match(SQL_string, r_select))
		return select_tras(SQL_string);
	else if(regex_match(SQL_string, r_insert))
		return insert_tras(SQL_string);
	else if(regex_match(SQL_string, r_delete))
		return delete_tras(SQL_string);
	else
		return SYNERR;
	return NOERR;
}

error_type create_table_tras(string SQL_string)
{
	//cout << "create table tras" << endl;
	vector<string> attr_name;		//保存属性名字
	vector<attr_type> attr_type;	//保存属性类型
	vector<bool> isUnique;			//保存属性唯一性
	vector<int> char_length;		//保存char长度
	string table_name;				//保存表名
	string primary;					//保存主键

	//定义正则表达式
	string table_name_regex;
	string attr_regex;
	string primary_regex;
	table_name_regex = "create table ([a-z0-9_]+)";
	attr_regex = "([a-z0-9_]+) ((int)|(float)|(char\\(\\d+\\)))( unique)?";
	primary_regex = "primary key\\(([a-z0-9_]+)\\)";
	
	//读取属性
	regex r_attr(attr_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_attr), end_it; it != end_it; ++it)
	{
		string type = it->str(2);
		attr_name.push_back(it->str(1));
		if (type == "int")
		{
			attr_type.push_back(INT);
			char_length.push_back(0);
		}
		else if (type == "float")
		{
			attr_type.push_back(FLOAT);
			char_length.push_back(0);
		}
		else
		{
			attr_type.push_back(CHAR);
			char_length.push_back(atoi(type.substr(type.find("(") + 1, type.find(")") - 1).c_str()));
		}
			
		if (it->str(0).find("unique") != string::npos)
			isUnique.push_back(true);
		else
			isUnique.push_back(false);
	}
	//读取表名
	regex r_table_name(table_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_table_name), end_it; it != end_it; ++it)
		table_name = it->str(1);
	//读取主键
	regex r_primary(primary_regex, regex::icase);
	for(sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_primary), end_it; it != end_it; ++it)
		primary = it->str(1);

	//测试代码
	//cout << left;
	//cout << "table name is :" << table_name << endl;
	//cout << setw(15) << "attr_name" << setw(15) << "attr_type" << setw(15) << "char_length" << setw(15) << "isUnique" << endl;
	//for (int i = 0; i < attr_name.size(); i++)
	//	cout << setw(15) << attr_name.at(i) << setw(15) << attr_type.at(i) << setw(15) << char_length.at(i) << setw(15) << isUnique.at(i) << endl;
	//cout << "primary key is:" << primary << endl;

	//传入API层
	if (find(attr_name.begin(), attr_name.end(), primary) == attr_name.end())
		return PRIERR;
	API.setCols(attr_name, attr_type, char_length, isUnique, primary);
	API.setTableName(table_name);
	API.setOperation(CRETAB);
	return NOERR;
}

error_type drop_table_tras(string SQL_string)
{
	// cout << "drop table tras" << endl;
	string table_name;				//保存表名

	//定义正则表达式
	string table_name_regex;
	table_name_regex = "drop table ([a-z0-9_]+)";

	//读取表名
	regex r_table_name(table_name_regex, regex::icase);
	for(sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_table_name), end_it; it != end_it; ++it)
		table_name = it->str(1);

	//测试代码
	//cout << "table name is:" << table_name << endl;

	//传入API层
	API.setOperation(DRPTAB);
	API.setTableName(table_name);
	return NOERR;
}
error_type create_index_tras(string SQL_string)
{
	//cout << "create index tras" << endl;
	string index_name;				//保存索引名
	string table_name;				//保存表名
	string attr_name;				//保存属性名

	//定义正则表达式
	string index_name_regex;
	string table_name_regex;
	string attr_name_regex;
	index_name_regex = "create index ([a-z0-9_]+)";
	table_name_regex = "on ([a-z0-9_]+)";
	attr_name_regex = "\\(([a-z0-9_]+)\\)";

	//读取索引名
	regex r_index_name(index_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_index_name), end_it; it != end_it; ++it)
		index_name = it->str(1);

	//读取表名
	regex r_table_name(table_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_table_name), end_it; it != end_it; ++it)
		table_name = it->str(1);

	//读取属性名
	regex r_attr_name(attr_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_attr_name), end_it; it != end_it; ++it)
		attr_name = it->str(1);

	//测试代码
	//cout << "table name is :" << table_name << endl;
	//cout << "index name is :" << index_name << endl;
	//cout << "attr name is :" << attr_name << endl;

	//传入API层
	API.setOperation(CREIDX);
	API.setTableName(table_name);
	API.setIndexName(index_name);
	API.setIndexAttr(attr_name);
	return NOERR;
}
error_type drop_index_tras(string SQL_string)
{
	//cout << "drop index tras" << endl;
	string index_name;				//保存表名

	//定义正则表达式
	string index_name_regex;
	index_name_regex = "drop index ([a-z0-9_]+)";

	//读取索引名
	regex r_index_name(index_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_index_name), end_it; it != end_it; ++it)
		index_name = it->str(1);

	//测试代码
	//cout << "index name is:" << index_name << endl;

	//传入API层
	API.setOperation(DRPIDX);
	API.setIndexName(index_name);
	return NOERR;
}
error_type select_tras(string SQL_string)
{
	//cout << "select tras" << endl;
	string table_name;				//保存表名
	vector<string> attr_name;		//保存比较的属性名
	vector<cond_type> cond;			//保存比较的操作
	vector<string> const_value;		//保存比较的常数
	vector<attr_type> attr_type;	//保存比较的属性类型

	//定义正则表达式
	string table_name_regex;
	string attr_regex;
	table_name_regex = "from ([a-z0-9_]+)( where)?";
	attr_regex = "([a-z0-9_]+)([<>=]+)('[a-z]+'|[0-9\\.]+)";

	//读取表名
	regex r_table_name(table_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_table_name), end_it; it != end_it; ++it)
		table_name = it->str(1);

	//读取条件
	regex r_attr(attr_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_attr), end_it; it != end_it; ++it)
	{
		string op(it->str(2));
		string value(it->str(3));
		//读取属性
		attr_name.push_back(it->str(1));
		//读取操作
		if (op == "<") cond.push_back(LT);
		else if (op == "<=") cond.push_back(LE);
		else if (op == ">") cond.push_back(GT);
		else if (op == ">=") cond.push_back(GE);
		else if (op == "=") cond.push_back(EQ);
		else if (op == "<>") cond.push_back(NE);
		//读取常量和类型
		if (value.find(string("'")) != string::npos)
		{
			attr_type.push_back(CHAR);
			const_value.push_back(value.substr(1, value.size() - 2));
		}
		else if (value.find(string(".")) != string::npos)
		{
			attr_type.push_back(FLOAT);
			const_value.push_back(value);
		}
		else
		{
			attr_type.push_back(INT);
			const_value.push_back(value);
		}
	}

	//测试代码
	//cout << "table name is :" << table_name << endl;
	//cout << setw(15) << "attr_name" << setw(15) << "cond" << setw(15) << "value" << setw(15) << "attr_type" << endl;
	//for (int i = 0; i < attr_name.size(); i++)
	//	cout << setw(15) << attr_name[i] << setw(15) << cond[i] << setw(15) << const_value[i] << setw(15) << attr_type[i] << endl;

	//传入API层
	API.setOperation(SELECT);
	API.setTableName(table_name);
	API.setConds(attr_name, cond, const_value, attr_type);
	return NOERR;
}
error_type insert_tras(string SQL_string)
{
	//cout << "insert tras" << endl;
	string table_name;				//保存表名
	vector<string> insert_value;		//保存插入的常数
	vector<attr_type> attr_type;	//保存插入的类型

	//定义正则表达式
	string table_name_regex;
	string insert_value_regex;
	table_name_regex = "insert into ([a-z0-9_]+)";
	insert_value_regex = "([0-9//.]+|'[a-z]+')[,|\\)]";

	//读取表名
	regex r_table_name(table_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_table_name), end_it; it != end_it; ++it)
		table_name = it->str(1);

	//读取插入值
	regex r_insert_value(insert_value_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_insert_value), end_it; it != end_it; ++it)
	{
		string value(it->str(1));
		if (value.find(string("'")) != string::npos)
		{
			attr_type.push_back(CHAR);
			insert_value.push_back(value.substr(1, value.size() - 2));
		}
		else if (value.find(string(".")) != string::npos)
		{
			attr_type.push_back(FLOAT);
			insert_value.push_back(value);
		}
		else
		{
			attr_type.push_back(INT);
			insert_value.push_back(value);
		}
	}

	//测试代码
	//cout << "table name is :" << table_name << endl;
	//cout << setw(15) << "insert_value" << setw(15) << "attr_type" << endl;
	//for (int i = 0; i < insert_value.size(); i++)
	//	cout << setw(15) << insert_value[i] << setw(15) << attr_type[i] << endl;

	//传入API层
	API.setOperation(INSERT);
	API.setTableName(table_name);
	API.setValues(insert_value, attr_type);
	return NOERR;
}
error_type delete_tras(string SQL_string)
{
	//cout << "delete tras" << endl;
	string table_name;				//保存表名
	vector<string> attr_name;		//保存比较的属性名
	vector<cond_type> cond;			//保存比较的操作
	vector<string> const_value;		//保存比较的常数
	vector<attr_type> attr_type;	//保存比较的属性类型

	//定义正则表达式
	string table_name_regex;
	string attr_regex;
	table_name_regex = "from ([a-z0-9_]+)( where)?";
	attr_regex = "([a-z0-9_]+)([<>=]+)('[a-z]+'|[0-9\\.]+)";

	//读取表名
	regex r_table_name(table_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_table_name), end_it; it != end_it; ++it)
		table_name = it->str(1);

	//读取条件
	regex r_attr(attr_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_attr), end_it; it != end_it; ++it)
	{
		string op(it->str(2));
		string value(it->str(3));
		//读取属性
		attr_name.push_back(it->str(1));
		//读取操作
		if (op == "<") cond.push_back(LT);
		else if (op == "<=") cond.push_back(LE);
		else if (op == ">") cond.push_back(GT);
		else if (op == ">=") cond.push_back(GE);
		else if (op == "=") cond.push_back(EQ);
		else if (op == "<>") cond.push_back(NE);
		//读取常量和类型
		if (value.find(string("'")) != string::npos)
		{
			attr_type.push_back(CHAR);
			const_value.push_back(value.substr(1, value.size() - 2));
		}
		else if (value.find(string(".")) != string::npos)
		{
			attr_type.push_back(FLOAT);
			const_value.push_back(value);
		}
		else
		{
			attr_type.push_back(INT);
			const_value.push_back(value);
		}
	}

	//测试代码
	//cout << "table name is :" << table_name << endl;
	//cout << setw(15) << "attr_name" << setw(15) << "cond" << setw(15) << "value" << setw(15) << "attr_type" << endl;
	//for (int i = 0; i < attr_name.size(); i++)
	//	cout << setw(15) << attr_name[i] << setw(15) << cond[i] << setw(15) << const_value[i] << setw(15) << attr_type[i] << endl;

	//传入API层
	API.setOperation(DELETE);
	API.setTableName(table_name);
	API.setConds(attr_name, cond, const_value, attr_type);
	return NOERR;
}

error_type exec_file(string SQL_string)
{
	string file_name_regex;
	string file_name;
	vector<string> SQL;

	file_name_regex = "exec ([a-z0-9\\._/]+)";
	regex r_file_name(file_name_regex, regex::icase);
	for (sregex_iterator it(SQL_string.begin(), SQL_string.end(), r_file_name), end_it; it != end_it; ++it)
		file_name = it->str(1);

	ifstream in(file_name);
	if(!in.is_open())
		return FLERR;
	while(!in.eof())
	{
		string subSQL;
		getline(in,subSQL);
		if(subSQL != "")
			SQL.push_back(subSQL);
	}
	in.close();

	error_type iterpreter_error;
	string result;
	cout << "This file contain " << SQL.size() << " SQL strings" << endl;
	for(auto it = SQL.begin(); it != SQL.end(); ++it)
	{
		cout << *it << endl;
		iterpreter_error = SQL_to_Command(*it);

		if (iterpreter_error == NOERR)	//成功转换
			result = read_error(API.exec());
		else
			result = read_error(iterpreter_error);
		cout << result << endl;
		API.clear();
	}
	return NOERR;
}
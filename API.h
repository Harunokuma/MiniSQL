#ifndef API_H
#define API_H

#include "MiniSQL.h"
#include <vector>
#include <string>
#include <iostream>

class Command
{
private:
	op_type operation;
	string table_name;
	string index_name;
	string file_name;
	string index_attr;
	vector<column> cols;
	vector<condition> conds;
	vector<insertvalue> values;
public:
	Command()
	{
		operation = UNKNOWN;
		table_name = "";
		index_name = "";
		file_name = "";
		index_attr = "";
	}
	void initiate();
	void end();
	void setOperation(op_type type);
	void setTableName(string name);
	void setIndexName(string name);
	void setCols(vector<string> attr_name, vector<attr_type> attr_type, vector<int> char_length, vector<bool> isUnique, string primary);
	void setConds(vector<string> attr_name, vector<cond_type> cond, vector<string> const_value, vector<attr_type> attr_type);
	void setValues(vector<string> insert_value, vector<attr_type> attr_type);
	void setIndexAttr(string attr_name);
	string FindPrimary();
	void clear();
	void test();
	error_type exec();
	error_type exec_cretab();
	error_type exec_drptab();
	error_type exec_creidx();
	error_type exec_drpidx();
	error_type exec_select();
	error_type exec_insert();
	error_type exec_delete();
};

#endif
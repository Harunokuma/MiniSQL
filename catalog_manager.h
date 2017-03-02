#ifndef _CATALOG_MANAGER_H
#define _CATALOG_MANAGER_H

#include "MiniSQL.h"
#include <stdio.h>

using namespace std;

class CatalogManager
{
private:
	vector<table> table_set;		//储存所有表的结构信息
	vector<index> index_set;		//储存所有索引的结构信息
public:
	CatalogManager() {};
	~CatalogManager() {};
	void clear();
	void ReadCatalog();
	void UpdateCatalog();

	error_type CreateTable(table NewTable);
	error_type CreateIndex(index NewIndex);
	bool FindIndex(string table_name, string attr_name);
	error_type DropTable(string table_name);
	bool FindTable(string table_name);
	error_type CheckValue(string name, vector<insertvalue> &values);
	void FillCond(vector<condition> &cond,string table_name);
	vector<condition> UniqueSet(string table_name, vector<insertvalue> values);
	vector<index> GetIndexInfo(string table_name);
	int FindPosition(string table_name, string attr_name);
	bool isUnique(string table_name, string attr_name);
	column GetColInfo(string table_name, string attr_name);
	error_type DropIndex(string index_name);
	error_type CheckCond(string table_name, vector<condition> conds);
	void PrintRecordForm(string table_name);
};

#endif
#ifndef RECORD_MANAGER_H
#define RECORD_MANAGER_H

#include "MiniSQL.h"
#include "buffer_manager.h"

using namespace std;

class RecordManager : public BufferManager
{
public:
	RecordManager() {};
	~RecordManager() {};

	static unsigned int InsertValue(string name, vector<insertvalue> value);
	static error_type DeleteValue(string name, int record_offset);
	static vector<int> SelectValue(string name, vector<condition> conds);
	static void PrintResult(vector<int> result, string table_name);
	static error_type UniqueCheck(string table_name, condition conds);
	static string GetValue(string table_name, column col, int offset);
};

vector<int> FindIntersection(vector<int> result_1, vector<int> result_2);

#endif
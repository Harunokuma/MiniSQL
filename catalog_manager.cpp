#include "catalog_manager.h"

//��Catalog�ļ��ж�ȡ��������Ľṹ��Ϣ
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

	//�򿪴������Ϣ���ļ�������ļ������ڣ����½�һ������Ϊ-1���ļ�
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
	while (bufstr != "-1")				//�ж��Ƿ��Ѿ���ȡ���
	{
		table tmp_tab;					//�������Ϣ�Ľṹ
		unsigned int offset = 0;		//���Ե�ƫ����

		tmp_tab.table_name = buffer;	//���ļ��л�ȡ����
		fs.getline(buffer, 100);
		tmp_tab.primary = buffer;		//���ļ��л�ȡ����
		fs.getline(buffer, 100);
		tmp_tab.col_num = atoi(buffer);	//���ļ��л�ȡ��������

		for (int i = 0; i < tmp_tab.col_num; ++i)
		{
			column tmp_col;
			int type;
			fs.getline(buffer, 100);
			tmp_col.col_name = buffer;	//���ļ��л�ȡ������
			fs.getline(buffer, 100);
			type = atoi(buffer);		//���ļ��л�ȡ�������ͺͳ���

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
			offset += tmp_col.col_length;	//����ƫ����

			//���ļ��л�ȡ�������Ƿ�Ϊ����
			if (tmp_col.col_name == tmp_tab.primary)
				tmp_col.isPrimary = true;
			else
				tmp_col.isPrimary = false;

			//���ļ��л�ȡ�������Ƿ�Ψһ
			fs.getline(buffer, 100);
			if (atoi(buffer) == 1)
				tmp_col.isUnique = true;
			else
				tmp_col.isUnique = false;

			//�����Դ�������Ϣ�ṹ��
			tmp_tab.cols.push_back(tmp_col);
		}
		fs.getline(buffer, 100);
		bufstr = buffer;

		//�������Ϣ����CatalogManager��
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

	//�򿪴���������Ϣ���ļ�������ļ������ڣ����½�һ������Ϊ-1���ļ�
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
	while (bufstr != "-1")				//�ж��Ƿ��Ѿ���ȡ���
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

//�����Ĺ��ı��������Ϣд��Catalog�ļ���
void CatalogManager::UpdateCatalog()
{
	fstream fs;

	//���±��¼�ļ�
	fs.open("catalog/tables.catalog", ios::out);

	for (int i = 0; i < table_set.size(); ++i)
	{
		char buffer[100];
		int char_len;
		fs << table_set[i].table_name << endl;			//���±���
		fs << table_set[i].primary << endl;				//��������
		_itoa_s(table_set[i].col_num, buffer, 10);		//������������
		fs << buffer << endl;

		//����������Ϣ
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

	//����������Ϣ
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

//�½�һ�������Ϣ�ļ�
error_type CatalogManager::CreateTable(table NewTable)
{
	//������Ѿ����ڣ����ش�����Ϣ
	if (FindTable(NewTable.table_name) == true)
		return TABEXT;

	//���±����Ϣ�������Ϣ������
	table_set.push_back(NewTable);

	fstream fs;									//�ļ���
	string file_name;							//�ļ���
	int total_len = 0;							//һ����¼���ܳ���
	char len[100];

	/*table_name.map
	 *Block0:
	 *|total_len|#|len1|#|len2|#...|len...|#
	 *
	 *Block1:
	 *1111010101111.....#
	 * ��    ��           ��
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
	fs << string(len) << "#";					//map�ļ��ĵ�һ�����ִ������һ����¼���ܳ��ȣ�֮����#�ָ�

	//map�ļ��м�¼�ܳ���֮����ÿ�����Եĳ���
	for (int i = 0; i < NewTable.col_num; ++i)
		fs << NewTable.cols[i].col_length << "#";

	//map�ļ��ڵڶ������￪ʼ����tab�ļ���ʹ����Ϣ
	fs.seekp(BLOCK_SIZE, ios::beg);
	fs << "#";									//map�ļ���'1'������ʹ�ã�'0'����δʹ�ã�'#'�����ļ�����
	fs.close();

	//�Ա��е�������������
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

//�½�һ����������Ϣ�ļ�
error_type CatalogManager::CreateIndex(index NewIndex)
{
	if (FindIndex(NewIndex.table_name, NewIndex.attr_name) == true)
		return IDXEXT;

	//������������Ϣ������������
	index_set.push_back(NewIndex);

	fstream fs;									//�ļ���
	string file_name;							//�ļ���
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

	//�½�һ��.idx�ļ�����������г�ʼ��
	file_name = "idx/" + NewIndex.table_name + "_" + NewIndex.attr_name + ".idx";
	fs.open(file_name, ios::out);
	sst << setw(4) << setfill(' ') << left << NewIndex.attr_len;
	sst << setw(4) << setfill(' ') << left << "0";
	sst << NewIndex.attr_type;
	sst << "#";
	fs << sst.str();
	fs.close();
}

//ɾ��һ�����������Ϣ
error_type CatalogManager::DropTable(string table_name)
{
	//�������Ϣ�ӱ�ɾ������ɾ���ļ�
	string tabfile_name = "tab/" + table_name + ".tab";
	string mapfile_name = "map/" + table_name + ".map";
	bool flag = false;									//�ж��Ƿ��д˱�

	//ɾ�����catalog��Ϣ��tab�ļ���map�ļ�
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

	//ɾ����ñ��йص�������catalog��Ϣ��idx�ļ�
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

	//��������ڱ��򷵻ش�����Ϣ
	if (flag == true)
		return NOERR;
	else
		return NOTAB;
}

//����һ������
error_type CatalogManager::DropIndex(string index_name)
{
	//ɾ�����������catalog��Ϣ��idx�ļ�
	for (auto it = index_set.begin(); it != index_set.end(); ++it)
		if (it->index_name == index_name)
		{
			string file_name;

			file_name = "idx/" + it->table_name + "_" + it->attr_name + ".idx";
			index_set.erase(it);
			remove(file_name.c_str());
			return NOERR;
		}

	//��������ڸ����������ش�����Ϣ
	return NOIDX;
}

//���ұ��Ƿ���ڣ��Ƿ���true���񷵻�false
bool CatalogManager::FindTable(string table_name)
{
	for (int i = 0; i < table_set.size(); ++i)
	{
		if (table_set[i].table_name == table_name)
			return true;
	}
	return false;
}

//���������Ƿ���ڣ��Ƿ���true���񷵻�false
bool CatalogManager::FindIndex(string table_name, string attr_name)
{
	for (int i = 0; i < index_set.size(); ++i)
	{
		if (index_set[i].table_name == table_name && index_set[i].attr_name == attr_name)
			return true;
	}
	return false;
}

//���Ҳ����ֵ�Ƿ����Ľṹ��ƥ�䣬��������ֵ�ĳ��Ⱥͱ�ṹͬ��
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

//��ձ���������
void CatalogManager::clear()
{
	table_set.clear();
	index_set.clear();
}

//����Catalog��Cond���в���
void CatalogManager::FillCond(vector<condition> &cond, string table_name)
{
	table tmp;

	//��ȡtable��Ϣ
	for (int i = 0; i < table_set.size(); ++i)
	{
		if (table_set[i].table_name == table_name)
		{
			tmp = table_set[i];
			break;
		}
	}

	//���cond�е���Ϣ�����͡����ȡ�ƫ�Ƶȣ�
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

//����catalog������֤Ψһ�Ե�condition
vector<condition> CatalogManager::UniqueSet(string table_name, vector<insertvalue> values)
{
	int table_num;
	vector<condition> conds;

	//�ҵ���ǰ��Ҫ��table��Ϣ
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == table_name)
		{
			table_num = i;
			break;
		}

	//���������unique����primary key���������conds��
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

//��ȡһ��table������index����Ϣ
vector<index> CatalogManager::GetIndexInfo(string table_name)
{
	vector<index> indexes;
	for (auto it = index_set.begin(); it != index_set.end(); ++it)
		if (it->table_name == table_name)
			indexes.push_back(*it);
	return indexes;
}

//��ȡһ��������table���ǵڼ�������
int CatalogManager::FindPosition(string table_name, string attr_name)
{
	for (auto it = table_set.begin(); it != table_set.end(); ++it)
		if (it->table_name == table_name)
			for (int i = 0; i < it->cols.size(); ++i)
				if (it->cols[i].col_name == attr_name)
					return i;
}

//���������Ƿ�Ψһ
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

//��ȡ������Ϣ
column CatalogManager::GetColInfo(string table_name, string attr_name)
{
	for (int i = 0; i < table_set.size(); ++i)
		if (table_set[i].table_name == table_name)
			for (int j = 0; j < table_set[i].cols.size(); ++j)
				if (table_set[i].cols[j].col_name == attr_name)
					return table_set[i].cols[j];
}

//��ӡһ����Ľṹ
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

//��������Ƿ���ϱ�Ľṹ
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
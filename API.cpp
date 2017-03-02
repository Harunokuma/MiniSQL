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
		//�洢���Գ���
		if (temp.type == CHAR)
			temp.col_length = char_length[i];
		else if (temp.type == INT)
			temp.col_length = INT_LEN;
		else if (temp.type == FLOAT)
			temp.col_length = FLOAT_LEN;
		//�洢����ƫ��
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

//ִ�к��������ݲ�ͬ��operation��ִ�в�ͬ�Ĳ���
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

//�½���Ĳ���
error_type Command::exec_cretab()
{
	table tmp;						//�����ṹ��Ϣ
	error_type err;					//���������Ϣ

	tmp.table_name = table_name;
	tmp.col_num = cols.size();
	tmp.cols = cols;
	tmp.primary = FindPrimary();

	err = CM.CreateTable(tmp);		//��������Ϣ
	if (err != NOERR)
		return err;
	return NOERR;
}

//����һ����Ĳ���
error_type Command::exec_drptab()
{
	error_type err;					//���������Ϣ

	BufferManager::flush_all();
	err = CM.DropTable(table_name);	//ɾ������Ϣ
	if (err != NOERR)
		return err;
	else 
		return NOERR;
}

//����һ�������Ĳ���
error_type Command::exec_creidx()
{
	vector<int> result;
	column col;
	index NewIndex;

	//�Ƿ��Ѿ����ڸ�����
	if (CM.FindIndex(table_name, index_attr) == true)
		return IDXEXT;

	//�Ƿ�Υ������Ψһ��
	if (CM.isUnique(table_name, index_attr) == false)
		return UNIERR;
	
	col = CM.GetColInfo(table_name, index_attr);		//��ȡ���������Ե���Ϣ

	//��������������Ϣ
	NewIndex.table_name = table_name;
	NewIndex.index_name = index_name;
	NewIndex.attr_name = index_attr;
	NewIndex.attr_type = col.type;
	NewIndex.attr_len = col.col_length;
	CM.CreateIndex(NewIndex);
	
	result = RecordManager::SelectValue(table_name, conds);	//��ȡ���еļ�¼��λ��

	//�����еļ�¼���������Ĳ������
	for (int i = 0; i < result.size(); ++i)
	{
		string value;
		value = RecordManager::GetValue(table_name, col, result[i]);	//��ȡ��¼�е��ض����Ե�ֵ
		
		IndexManager::InsertIndex(table_name, index_attr, value,result[i]);
	}

	return NOERR;
}

//����һ�������Ĳ���
error_type Command::exec_drpidx()
{
	//ͨ��CatalogManager����һ������
	return CM.DropIndex(index_name);
}

//ִ�в�ѯ����
error_type Command::exec_select()
{
	vector<int> result;					//�����ѯ�����ƫ����
	vector<int> tmp_result;				//��ʱ�����ѯ���
	vector<condition> RecordConds;		//������Ҫֱ���ڼ�¼�ļ�������������
	error_type err;
	bool flag = false;

	err = CM.CheckCond(table_name, conds);	//��������ͱ�Ľṹ�Ƿ����
	if (err != NOERR)
		return err;
	CM.FillCond(conds, table_name);			//���������в��꣨���ȡ�ƫ�ƣ�

	if (conds.size() == 0)
		result = RecordManager::SelectValue(table_name, conds);
	else
	{
		for (int i = 0; i < conds.size(); ++i)
		{
			//���ĳ�������е�������������ִ��������ѯ
			if (CM.FindIndex(table_name, conds[i].attr_name) == true)
			{
				tmp_result = IndexManager::SelectIndex(table_name, conds[i]);
				if (flag == false)
					result = FindIntersection(tmp_result, tmp_result);
				else if(flag == true)
					result = FindIntersection(result, tmp_result);
				flag = true;
			}
			//���򽫸���������RecordConds
			else
				RecordConds.push_back(conds[i]);
	}

		//���������Ҫʹ�ü�¼�ļ����������������RecordManager������
		if (RecordConds.size() > 0)
		{
			tmp_result = RecordManager::SelectValue(table_name, RecordConds);
			if (flag == false)
				result = FindIntersection(tmp_result, tmp_result);
			else if (flag == true)
				result = FindIntersection(result, tmp_result);
		}
	}

	//��������û�����ݣ����ʾ�����ڸ������µļ�¼
	if (result.size() == 0)
		cout << "No data" << endl;

	//����ͨ������е�ƫ������ȡ��¼�ļ��������
	else
	{
		CM.PrintRecordForm(table_name);
		RecordManager::PrintResult(result, table_name);
	}

	return NOERR;
}

//ִ�м�¼�Ĳ������
error_type Command::exec_insert()
{
	error_type err_info;				//������Ϣ
	vector<condition> UniqueCond;		//Ψһ�Լ�������
	int offset;							//�²����¼��ƫ����
	vector<index> indexes;

	//�������Ĳ���ֵ�Ƿ���ϱ�Ľṹ
	err_info = CM.CheckValue(table_name, values);
	if (err_info != NOERR)
		return err_info;

	//ͨ��CatalogManager������Ψһ�Լ�������
	UniqueCond = CM.UniqueSet(table_name, values);

	//�Լ�¼�����е��������²��������ִ��Ψһ�Լ��
	for (int i = 0; i < UniqueCond.size(); ++i)
	{
		if (CM.FindIndex(table_name, UniqueCond[i].attr_name) == true)
			err_info = IndexManager::UniqueCheck(table_name, UniqueCond[i]);
		else
			err_info = RecordManager::UniqueCheck(table_name, UniqueCond[i]);

		if (err_info != NOERR)
			return err_info;
	}

	//�������ݲ����¼�ļ�
	offset = RecordManager::InsertValue(table_name, values);

	//��ȡ���е�ǰ�����е���������Ϣ
	indexes = CM.GetIndexInfo(table_name);

	//�����е����������²���ļ�¼
	for (auto it = indexes.begin(); it != indexes.end(); ++it)
	{
		int position;
		position = CM.FindPosition(table_name, it->attr_name);		//��ȡ�����ڼ�¼�е�λ��
		IndexManager::InsertIndex(table_name, it->attr_name, values[position].value, offset);
	}
	return NOERR;
}

//ִ��ɾ����¼�Ĳ���
error_type Command::exec_delete()
{
	vector<int> result;
	vector<index> indexes;

	CM.FillCond(conds, table_name);			//���������в��꣨���ȡ�ƫ�ƣ�

	//��ȡ���е�ǰ�����е���������Ϣ
	indexes = CM.GetIndexInfo(table_name);

	//��ȡ��Ҫɾ���ļ�¼��ƫ����
	result = RecordManager::SelectValue(table_name, conds);

	//����ƫ�������ɾ����¼
	for (int i = 0; i < result.size(); ++i)
	{
		RecordManager::DeleteValue(table_name, result[i]);
		for (int j = 0; j < indexes.size(); j++)
		{
			column col;
			string value;

			col = CM.GetColInfo(table_name, indexes[j].attr_name);					//��ȡ���������Ե���Ϣ
			value = RecordManager::GetValue(table_name, col, result[i]);			//��ȡɾ����ֵ
			IndexManager::DeleteIndex(table_name, indexes[j].attr_name, value);		//����ֵ����������ɾ��
		}
	}
	return NOERR;
}
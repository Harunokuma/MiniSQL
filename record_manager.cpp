#include "record_manager.h"

using namespace std;

//��ֵ������������,���ز���ֵ��ƫ����
unsigned int RecordManager::InsertValue(string name, vector<insertvalue> value)
{
	int mapblock_num, tabblock_num;					//����map���table��Ŀ��
	int map_page = 0, tab_page;						//����map�ļ���tab�ļ���ҳ����
	int map_offset = 0,tab_offset;					//����map���tab���е�ƫ����
	int total_len;									//����һ����¼���ܳ���
	int record_per_page;							//����ÿҳ�ļ�¼����
	char mark;										//����map�ļ��еı��
	char *write_site;								//�����ݵ�д���
	stringstream tmp;								//��¼����ʱ�洢
	string buffer;

	//��map�ļ��ĵ�һ�����л�ȡ��¼���Ⱥ�ÿҳ��¼��
	mapblock_num = get_block(MAP, name, map_page);
	for (int i = 0; BlockSet[mapblock_num].block[i] != '#'; ++i)
		buffer += BlockSet[mapblock_num].block[i];
	total_len = atoi(buffer.c_str());
	record_per_page = BLOCK_SIZE / total_len;

	//��map�ļ��ĵڶ����鿪ʼ��ȡtab�ļ���λ����Ϣ
	++map_page;
	used_block(mapblock_num);
	mapblock_num = get_block(MAP, name, map_page);

	//Ѱ��map�ļ�������Ӧ��tab�ļ���λ
	while (BlockSet[mapblock_num].block[map_offset] != '#' && BlockSet[mapblock_num].block[map_offset] != '0')
	{
		map_offset++;

		//���������һ��map�黹û���ҵ����������¸�������
		if (map_offset >= BLOCK_SIZE)
		{
			map_offset = 0;
			++map_page;
			used_block(mapblock_num);
			mapblock_num = get_block(MAP, name, map_page);
		}
	}

	//�ҵ�map�ļ�������0����λ������#������λ��
	tab_page = ((map_page - 1)*BLOCK_SIZE + map_offset) / record_per_page;
	tab_offset = ((map_page - 1)*BLOCK_SIZE + map_offset) % record_per_page;
	mark = BlockSet[mapblock_num].block[map_offset];

	if (map_offset == BLOCK_SIZE - 1)	//������λ�ڿ�ĩβ
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
	else								//��������λ�ڿ�ĩβ
	{
		BlockSet[mapblock_num].block[map_offset] = '1';
		++map_offset;
		BlockSet[mapblock_num].isChanged = true;
		if (mark == '#')
			BlockSet[mapblock_num].block[map_offset] = '#';
	}

	//��ֵд����Ŀ�λ��
	tabblock_num = get_block(TABLE, name, tab_page);
	write_site = BlockSet[tabblock_num].block + tab_offset*total_len;
	for (int j = 0; j < value.size(); ++j)
		tmp << setw(value[j].length) << setfill(' ') << left << value[j].value;
	strcpy(write_site, tmp.str().c_str());
	BlockSet[tabblock_num].isChanged = true;
	used_block(tabblock_num);

	return tab_page*record_per_page + tab_offset;
}

//���ݱ����Ͷ��ڼ�¼������ƫ������ɾ��һ����¼
error_type RecordManager::DeleteValue(string name, int record_offset)
{
	int mapblock_num;								//����map��Ŀ��
	int map_page = 0;								//����map�ļ���ҳ����
	int map_offset = 0;								//����map��ƫ����

	map_page = record_offset / BLOCK_SIZE +1;
	map_offset = record_offset % BLOCK_SIZE;
	
	mapblock_num = get_block(MAP, name, map_page);
	BlockSet[mapblock_num].block[map_offset] = '0';
	BlockSet[mapblock_num].isChanged = true;
	used_block(mapblock_num);
	return NOERR;
}

//�����ݿ��������������������ݣ�������������ڼ�¼������ƫ����
vector<int> RecordManager::SelectValue(string name, vector<condition> conds)
{
	int mapblock_num, tabblock_num;					//����map���table��Ŀ��
	int map_page = 0, tab_page;						//����map�ļ���tab�ļ���ҳ����
	int map_offset = 0, tab_offset;					//����map���tab���е�ƫ����
	int total_len;									//����һ����¼���ܳ���
	int record_per_page;							//����ÿҳ�ļ�¼����
	vector<int> result_offset;						//�����ѯ�����ƫ��
	string buffer;

	//��map�ļ��ĵ�һ�����л�ȡ��¼���Ⱥ�ÿҳ��¼��
	mapblock_num = get_block(MAP, name, map_page);
	for (int i = 0; BlockSet[mapblock_num].block[i] != '#'; ++i)
		buffer += BlockSet[mapblock_num].block[i];
	total_len = atoi(buffer.c_str());
	record_per_page = BLOCK_SIZE / total_len;

	//��map�ļ��ĵڶ����鿪ʼ��ȡtab�ļ���λ����Ϣ
	++map_page;
	used_block(mapblock_num);
	mapblock_num = get_block(MAP, name, map_page);

	while (BlockSet[mapblock_num].block[map_offset] != '#')		//ͨ��map�ҵ�tab�ļ�����Ч�ļ�¼λ��
	{
		bool flag = true;
		stringstream sst;

		if (BlockSet[mapblock_num].block[map_offset] == '1')
		{				
			tab_page = ((map_page - 1)*BLOCK_SIZE + map_offset) / record_per_page;			//�����¼��ҳ
			tab_offset = ((map_page - 1)*BLOCK_SIZE + map_offset) % record_per_page;		//�����¼�����ƫ��
			
			tabblock_num = get_block(TABLE, name, tab_page);								//��ȡ��ҳ�ļ�¼
			sst.write(BlockSet[tabblock_num].block + tab_offset*total_len, total_len);

			//�жϸü�¼�Ƿ��������������ǣ��������ƫ�Ƽ�������
			for (int i = 0; i < conds.size(); ++i)
				if (!ValueCompare(sst.str().substr(conds[i].attr_offset, conds[i].attr_length), conds[i].type, conds[i].cond, conds[i].value))
					flag = false;
			if (flag == true)
				result_offset.push_back(tab_offset + tab_page*record_per_page);
			used_block(tabblock_num);
		}

		//���map�����˵�ǰҳ�����һ�����������һҳ
		if (map_offset == BLOCK_SIZE - 1)
		{
			map_offset = 0;
			map_offset = 0;
			++map_page;
			used_block(mapblock_num);
			mapblock_num = get_block(MAP, name, map_page);
		}

		//��map�е�λ����ǰ��һλ
		else
			++map_offset;
	}
	used_block(mapblock_num);
	return result_offset;
}

//Ϊ�����Ѱ�ҽ���
vector<int> FindIntersection(vector<int> result_1, vector<int> result_2)
{
	int i = 0, j = 0;
	vector<int> result;			//���潻��

	//�����һ������������������
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

//����ƫ�Ƽ��ͱ�������ӡ��ѯ���
void RecordManager::PrintResult(vector<int> result, string table_name)
{
	int mapblock_num, tabblock_num;					//����map���table��Ŀ��
	int map_page = 0, tab_page;						//����map�ļ���tab�ļ���ҳ����
	int map_offset = 0, tab_offset;					//����map���tab���е�ƫ����
	int total_len;									//����һ����¼���ܳ���
	int record_per_page;							//����ÿҳ�ļ�¼����
	vector<int> result_offset;						//�����ѯ�����ƫ��
	vector<int> attr_len;							//�������Եĳ���
	int len_sum = 0;
	string buffer;
	int count;

	//��map�ļ��ĵ�һ�����л�ȡ��¼���Ⱥ�ÿҳ��¼��
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

	//��ӡ��ѯ���
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

//���ݼ�¼�ļ�����ѯ�Ƿ�Υ����Ψһ��
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

//���ݼ�¼��ƫ��������ȡ��¼�е�ĳ���ض����Ե�ֵ
string RecordManager::GetValue(string table_name, column col, int offset)
{
	int mapblock_num, tabblock_num;					//����map���table��Ŀ��
	int map_page = 0, tab_page;						//����map�ļ���tab�ļ���ҳ����
	int tab_offset;									//����tab���е�ƫ����
	int total_len;									//����һ����¼���ܳ���
	int record_per_page;							//����ÿҳ�ļ�¼����
	string buffer;
	stringstream sst;

	//��map�ļ��ĵ�һ�����л�ȡ��¼���Ⱥ�ÿҳ��¼��
	mapblock_num = get_block(MAP, table_name, map_page);
	for (int i = 0; BlockSet[mapblock_num].block[i] != '#'; ++i)
		buffer += BlockSet[mapblock_num].block[i];
	total_len = atoi(buffer.c_str());
	record_per_page = BLOCK_SIZE / total_len;
	used_block(mapblock_num);

	//����ƫ�����ͱ�ṹ����ȡֵ
	tab_page = offset / record_per_page;
	tab_offset = offset % record_per_page;
	tabblock_num = get_block(TABLE, table_name, tab_page);
	sst << BlockSet[tabblock_num].block;
	return sst.str().substr(tab_offset*total_len + col.col_offset, col.col_length);
}
#include "index_manager.h"

//���ݿ���������ǰ��Ľṹ����ʾ
node IndexManager::GetNode(int block_num, int len)
{
	node tmp;				//��������node
	stringstream buffer;	//�ṩ�ַ�������

	//����������
	buffer.write(BlockSet[block_num].block, BLOCK_SIZE);

	tmp.SelfBlock_num = block_num;										//���õ�ǰ��Ŀ��
	tmp.len = len;														//���ø�index�����Գ���
	tmp.type = atoi(buffer.str().substr(0, 1).c_str());					//���ø�index������ͣ�����Ҷ�ڵ�ȣ�
	tmp.ParentBlockPointer = atoi(buffer.str().substr(1, 4).c_str());	//���ø��ڵ��ҳ��
	tmp.SelfPointer = atoi(buffer.str().substr(5, 4).c_str());			//���������ҳ��
	tmp.value_num = atoi(buffer.str().substr(9, 4).c_str());			//���������������ݵ�����

	//���ÿ��еĸ���ֵ��ָ��
	for (int i = 0; i < tmp.value_num; ++i)
	{
		value_pointer tmp_vp;
		tmp_vp.pointer = atoi(buffer.str().substr(13+(len+4)*i, 4).c_str());
		tmp_vp.value = buffer.str().substr(17 + (len + 4)*i, len);
		tmp.V_P.push_back(tmp_vp);
	}

	//���ÿ��е����һ��ָ��
	tmp.LastPointer = atoi(buffer.str().substr(13 + (len + 4)*tmp.value_num,4).c_str());

	return tmp;
}

//���ڵ��е���Ϣд�ؿ���
void IndexManager::WriteNode(node NewNode)
{
	stringstream buffer;

	buffer << NewNode.type;
	buffer << setw(4) << setfill(' ') << left << NewNode.ParentBlockPointer;
	buffer << setw(4) << setfill(' ') << left << NewNode.SelfPointer;
	buffer << setw(4) << setfill(' ') << left << NewNode.value_num;
	 
	for (int i = 0; i < NewNode.value_num; ++i)
	{
		buffer << setw(4) << setfill(' ') << left << NewNode.V_P[i].pointer;
		buffer << setw(NewNode.len) << setfill(' ') << left << NewNode.V_P[i].value;
	}
	buffer << setw(4) << setfill(' ') << left << NewNode.LastPointer;
	buffer.read(BlockSet[NewNode.SelfBlock_num].block, BLOCK_SIZE);
	BlockSet[NewNode.SelfBlock_num].isChanged = true;
	used_block(NewNode.SelfBlock_num);
}

//��һ��ֵ����������
void IndexManager::InsertIndex(string table_name, string attr_name, string value,int offset)
{
	string block_name = table_name + "_" + attr_name;		//����
	int attr_len;											//���Գ���
	int block_num;											//�������
	int value_per_node;										//һ���ڵ�����������ֵ������
	attr_type a_type;										//��������
	char* map;												//map���ݵ�ָ��
	int infoBlock_num, useBlock_num;						//�ṹ��Ϣ�Ŀ�ź���Ҫʹ�õĿ��
	stringstream buffer;									//�����ַ���
	node node_tmp;											//��ʱ����node�ڵ�

	//��ȡ�����ļ��Ľṹ��Ϣ����ȡ�����Գ��ȡ��������������͡�mapָ��ͽڵ���������ֵ������
	infoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[infoBlock_num].block, 9);
	attr_len = atoi(buffer.str().substr(0, 4).c_str());
	block_num = atoi(buffer.str().substr(4, 4).c_str());
	a_type = atoi(buffer.str().substr(8, 1).c_str());
	map = BlockSet[infoBlock_num].block + 9;
	value_per_node = (BLOCK_SIZE - 13) / (attr_len + 4) - 2;

	//�����л�û�нڵ�
	if (block_num == 0)
	{
		node tmp;						//��ʱ����node�ڵ�
		int WritePage;					//����д���ҳ��
		int WriteBlockNum;				//����д��Ŀ��
		value_pointer tmp_vp;			//��ʱ����ֵ-ָ���

		//��������ֵ��ָ�������ݽṹ��
		tmp_vp.value = value;
		tmp_vp.pointer = offset;

		//�����µĽڵ���Ϣ
		tmp.V_P.push_back(tmp_vp);
		tmp.type = RANDL;
		tmp.len = attr_len;
		tmp.value_num = 1;
		tmp.LastPointer = 0;
		tmp.ParentBlockPointer = 0;
		WritePage = FindSite(map);									//�ҵ������ҳ
		WriteBlockNum = get_block(INDEX, block_name, WritePage);	
		tmp.SelfPointer = WritePage;
		tmp.SelfBlock_num = WriteBlockNum;
		WriteNode(tmp);
		used_block(WriteBlockNum);
		AddBlockNum(block_name);									//�Կ��������һ
		return;
	}

	//�ҵ���Ҫ�����Ҷ�ڵ�
	useBlock_num = FindLeaf(block_name, 1, attr_len, value, a_type);
	node_tmp = GetNode(useBlock_num,attr_len);

	//Ҷ�ڵ��л��п�λ
	if (node_tmp.value_num < value_per_node)
		InsertInLeaf(useBlock_num, value, attr_len, offset,a_type);
	//Ҷ�ڵ���û�п�λ
	else
	{
		node leaf_1, leaf_2;
		int newPage;

		newPage = GetNewBlockSite(block_name);
		InsertInLeaf(useBlock_num, value, attr_len, offset,a_type);								//����ֵ�����Ҷ�ڵ�L��
		leaf_1 = GetNode(useBlock_num, attr_len);
		leaf_2 = GetNode(get_block(INDEX, block_name, newPage),attr_len);						//�½�һ���ڵ�L'
		leaf_2.SelfPointer = newPage;
		leaf_2.LastPointer = leaf_1.LastPointer;												//L'β��ָ��̳�L
		leaf_1.LastPointer = leaf_2.SelfPointer;												//Lβ��ָ��ָ��L'
		leaf_2.ParentBlockPointer = leaf_1.ParentBlockPointer;									//L'��ָ��̳�L
		leaf_2.type = leaf_1.type;																//L'�ڵ����ͼ̳�L

		//�����ڵ����ȡһ���ֵ
		auto it = leaf_1.V_P.begin();
		it += value_per_node / 2;
		leaf_2.V_P.assign(it, leaf_1.V_P.end());
		leaf_1.V_P.erase(it, leaf_1.V_P.end());
		leaf_2.value_num = leaf_2.V_P.size();
		leaf_1.value_num = leaf_1.V_P.size();
		WriteNode(leaf_1);
		WriteNode(leaf_2);

		//ִ���򸸽ڵ����Ĳ���
		InsertInParent(leaf_1, leaf_2.V_P[0].value, leaf_2,block_name);
	}
}

//�ҵ���ֵӦ�ڵ�Ҷ�ڵ㣬����Ҷ�ڵ�Ŀ��
int IndexManager::FindLeaf(string block_name, int Page, int attr_len, string value,attr_type type)
{
	node tmp;
	int block_num;

	block_num = get_block(INDEX, block_name, Page);
	tmp = GetNode(block_num, attr_len);
	if (tmp.type == LEAF || tmp.type == RANDL)
		return block_num;
	
	for (int i = 0; i < tmp.V_P.size(); i++)
		if (ValueCompare(value, type, LT, tmp.V_P[i].value))
			return FindLeaf(block_name, tmp.V_P[i].pointer, attr_len, value, type);
	return FindLeaf(block_name, tmp.LastPointer, attr_len, value, type);
}

//����ֵ����Ҷ�ڵ���
void IndexManager::InsertInLeaf(int block_num, string value, int len, int offset,attr_type a_type)
{
	node tmp;
	value_pointer tmp_vp;

	tmp = GetNode(block_num, len);
	tmp.value_num++;
	tmp_vp.value = value;
	tmp_vp.pointer = offset;

	//�ҵ���ֵӦ���ڵ�λ�ã��������
	for (auto it = tmp.V_P.begin(); it != tmp.V_P.end(); ++it)
		if (ValueCompare(value, a_type, LT, it->value) == true)
		{
			tmp.V_P.insert(it, tmp_vp);
			WriteNode(tmp);
			return;
		}
	tmp.V_P.push_back(tmp_vp);
	WriteNode(tmp);
}

//�򸸽ڵ��в���ֵ
void IndexManager::InsertInParent(node Node_1, string min_value, node Node_2, string block_name)
{
	int attr_len;				//���Գ���
	int infoBlock_num;			//�ṹ��Ϣ���
	int value_per_node;			//һ���ڵ�����������ֵ������
	stringstream buffer;		//�����ַ���
	node parent;				//���ڵ�

	//��ȡ�����ļ��Ľṹ��Ϣ����ȡ�����Գ��Ⱥͽڵ���������ֵ������
	infoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[infoBlock_num].block, 9);
	attr_len = atoi(buffer.str().substr(0, 4).c_str());
	value_per_node = (BLOCK_SIZE - 13) / (attr_len + 4) - 2;

	//��ȡ���ڵ�Ľڵ�
	if(Node_1.type != ROOT && Node_1.type != RANDL)
		parent = GetNode(get_block(INDEX, block_name, Node_1.ParentBlockPointer), attr_len);

	//������ѵĽڵ��Ǹ��ڵ�
	if (Node_1.type == ROOT || Node_1.type == RANDL)
	{
		node tmp;
		value_pointer tmp_vp;
		int newPage;

		//�½�һ���ڵ㣬���Ƹ��ڵ����Ϣ
		newPage = GetNewBlockSite(block_name);
		tmp = GetNode(get_block(INDEX, block_name, newPage), attr_len);
		tmp.SelfPointer = newPage;
		tmp.LastPointer = Node_1.LastPointer;
		tmp.len = Node_1.len;
		tmp.ParentBlockPointer = Node_1.SelfPointer;
		if (Node_1.type == ROOT)
			tmp.type = INNER;
		else if (Node_1.type == RANDL)
			tmp.type = LEAF;
		tmp.value_num = Node_1.value_num;
		tmp.V_P = Node_1.V_P;

		//ʹ���ѳ����Ľڵ�ĸ��ڵ�Ϊ���ڵ㣬�ı�������
		Node_2.ParentBlockPointer = Node_1.SelfPointer;
		if (Node_1.type == ROOT)
			Node_2.type = INNER;
		else if (Node_1.type == RANDL)
			Node_2.type = LEAF;
		
		//�ı���ڵ�����ͣ�ʹ��β��ָ��ָ����ѳ����Ľڵ�
		Node_1.type = ROOT;
		Node_1.LastPointer = Node_2.SelfPointer;
		
		//���½��Ľڵ��ֵ��Ϣ��ָ�������ڵ�
		tmp_vp.pointer = tmp.SelfPointer;
		tmp_vp.value = min_value;
		Node_1.V_P.clear();
		Node_1.V_P.push_back(tmp_vp);
		Node_1.value_num = 1;
		
		//�������ڵ�д�ؿ���
		WriteNode(tmp);
		WriteNode(Node_1);
		WriteNode(Node_2);
	}

	//���ڵ��л��п��ֱ࣬���ҵ�λ�ý�����ֵ����
	else if(parent.value_num < value_per_node)
	{ 
		bool flag = false;
		value_pointer tmp_vp;
		tmp_vp.value = min_value;
		tmp_vp.pointer = Node_1.SelfPointer;
		for (auto it = parent.V_P.begin(); it != parent.V_P.end(); ++it)
			if (it->pointer == Node_1.SelfPointer)
			{
				++it;
				parent.V_P.insert(it, tmp_vp);
				flag = true;
			}
		if (flag == false)
		{
			parent.LastPointer = Node_2.SelfPointer;
			parent.V_P.push_back(tmp_vp);
		}
		++parent.value_num;
		WriteNode(parent);
	}

	//���ڵ���û�п��ִ࣬�к�֮ǰ���Ѳ������ƵĲ���
	else if(parent.value_num >= value_per_node)
	{ 
		node NewNode;
		value_pointer tmp_vp;
		int newPage;

		tmp_vp.value = min_value;
		tmp_vp.pointer = Node_2.SelfPointer;

		for (auto it = parent.V_P.begin(); it != parent.V_P.end(); ++it)
			if (it->pointer == Node_1.SelfPointer)
			{
				++it;
				parent.V_P.insert(it, tmp_vp);
			}
		
		newPage = GetNewBlockSite(block_name);
		NewNode = GetNode(get_block(INDEX, block_name, newPage), attr_len);
		NewNode.SelfPointer = newPage;
		NewNode.LastPointer = parent.LastPointer;
		NewNode.ParentBlockPointer = parent.ParentBlockPointer;
		NewNode.type = parent.type;

		auto it = parent.V_P.begin();
		it += value_per_node/2;
		NewNode.V_P.assign(it, parent.V_P.end());
		copy(it, parent.V_P.end(), NewNode.V_P.begin());
		NewNode.value_num = NewNode.V_P.size();
		for (; it != parent.V_P.end(); ++it)
		{
			ChangeParentPointer(block_name, attr_len, it->pointer, NewNode.SelfPointer);
		}
		it = parent.V_P.begin();
		it += value_per_node / 2;
		parent.V_P.erase(it, parent.V_P.end());

		parent.LastPointer = NewNode.SelfPointer;
		parent.value_num = parent.V_P.size();

		WriteNode(parent);
		WriteNode(NewNode);
		InsertInParent(parent, NewNode.V_P[0].value, NewNode, block_name);
	}
}

//��ȡһ���µ�ҳ��
int IndexManager::GetNewBlockSite(string block_name)
{
	char *map;
	int infoBlock_num;
	int Blank_site;
	stringstream buffer;

	infoBlock_num = get_block(INDEX, block_name, 0);
	map = BlockSet[infoBlock_num].block + 9;
	Blank_site = FindSite(map);
	BlockSet[infoBlock_num].isChanged = true;
	used_block(infoBlock_num);
	AddBlockNum(block_name);
	return Blank_site;
}

//����ֵɾ��һ������
void IndexManager::DeleteIndex(string table_name, string attr_name, string value)
{
	string block_name;			//����
	int block_num;				//���
	int infoBlock_num;			//�ṹ��Ϣ���
	node tmp;					//��ʱ����ڵ�
	int attr_len;				//���Գ���
	attr_type type;				//��������
	int value_per_node;			//һ���ڵ�����������ֵ������
	stringstream buffer;		//�����ַ���

	block_name = table_name + "_" + attr_name;

	//��ȡ�����ļ��Ľṹ��Ϣ����ȡ�����Գ��Ⱥͽڵ���������ֵ������
	infoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[infoBlock_num].block, 9);
	attr_len = atoi(buffer.str().substr(0, 4).c_str());
	type = atoi(buffer.str().substr(8, 1).c_str());
	value_per_node = (BLOCK_SIZE - 13) / (attr_len + 4) - 2;
	
	block_num = FindLeaf(block_name, 1, attr_len, value, type);
	tmp = GetNode(block_num, attr_len);
	delete_entry(block_name, tmp, value, type);
}

//�ڽڵ���ִ��ɾ������
void IndexManager::delete_entry(string block_name, node Node, string value,attr_type type)
{
	int value_per_node = (BLOCK_SIZE - 13) / (Node.len + 4) - 2;

	for (auto it = Node.V_P.begin(); it != Node.V_P.end();++it)
		if (ValueCompare(it->value, type, EQ, value) == true)
		{
			Node.V_P.erase(it);
			Node.value_num--;
			WriteNode(Node);
			break;
		}

	//����ýڵ�Ϊ���ڵ����ֻʣ��һ��ֵ-ָ���
	if (Node.type == ROOT && Node.value_num == 1)
	{
		node tmp;

		tmp = GetNode(get_block(INDEX, block_name, Node.V_P[0].pointer), Node.len);
		Node.LastPointer = tmp.LastPointer;
		Node.value_num = tmp.value_num;
		Node.V_P = tmp.V_P;
		if (tmp.type == LEAF)
			Node.type = RANDL;
		FreeMapSite(block_name, tmp.SelfPointer);
		WriteNode(Node);
	}

	//����ýڵ���ֵ����
	else if (Node.type != RANDL && Node.type != ROOT && Node.value_num < value_per_node)
	{

	}
}

//��ȡ����Ŀ�λ��
int FindSite(char *map)
{
	for (int i = 0; i<BLOCK_SIZE-9; ++i)
	{
		if (map[i] == '0')
		{
			map[i] = '1';
			return i+1;
		}
		else if (map[i] == '#')
		{
			map[i] = '1';
			map[i + 1] = '#';
			return i+1;
		}
	}
	return -1;
}

//�ı�һ���ڵ�ĸ��ڵ�ָ��
void IndexManager::ChangeParentPointer(string block_name, int len, int SelfPage, int ParentPage)
{
	node tmp;
	int block_num;

	block_num = get_block(INDEX, block_name, SelfPage);
	tmp = GetNode(block_num, len);
	tmp.ParentBlockPointer = ParentPage;
	WriteNode(tmp);
}

//���ܿ�����һ
void IndexManager::AddBlockNum(string block_name)
{
	int InfoBlock_num;
	int block_num;
	stringstream buffer;

	InfoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[InfoBlock_num].block, 9);
	block_num = atoi(buffer.str().substr(4, 4).c_str());
	++block_num;
	buffer.clear();
	buffer.str("");
	buffer << setw(4) << setfill(' ') << left << block_num;
	buffer.read(BlockSet[InfoBlock_num].block + 4, 4);
	BlockSet[InfoBlock_num].isChanged = true;
	used_block(InfoBlock_num);
}

//���ܿ�����һ
void IndexManager::MinuBlockNum(string block_name)
{
	int InfoBlock_num;
	int block_num;
	stringstream buffer;

	InfoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[InfoBlock_num].block, 9);
	block_num = atoi(buffer.str().substr(4, 4).c_str());
	--block_num;
	buffer.clear();
	buffer.str("");
	buffer << setw(4) << setfill(' ') << left << block_num;
	buffer.read(BlockSet[InfoBlock_num].block + 4, 4);
	BlockSet[InfoBlock_num].isChanged = true;
	used_block(InfoBlock_num);
}

//������������������������ֵ��ƫ��
vector<int> IndexManager::SelectIndex(string table_name, condition cond)
{
	int block_num;				//���
	int num;					//����
	string block_name;			//����
	node use_node;				//ʹ�õĽڵ�
	vector<int> result;			//�����
	vector<int> tmp;			//��ʱ����Ľ����

	//�ҵ�ֵӦ���ڵĽڵ�
	block_name = table_name + "_" + cond.attr_name;
	num = GetNum(block_name);
	if (num < 1)
		return result;
	block_num = FindLeaf(block_name, 1, cond.attr_length, cond.value, cond.type);
	use_node = GetNode(block_num, cond.attr_length);

	switch (cond.cond)
	{
	case EQ:
		//�ڸýڵ��������Ƿ���ڸ�ֵ��������ڣ����������
		for (int i = 0; i < use_node.V_P.size(); ++i)
			if (ValueCompare(use_node.V_P[i].value, cond.type, EQ, cond.value))
			{
				result.push_back(use_node.V_P[i].pointer);
				break;
			}
		break;
	case NE:
		//��ȡ���ʼ��ֵ������ֵ���������ֵ�����һ��ֵ�����ƫ�ƵĲ���
		result = GetSection(block_name, use_node, cond.value, "pre", cond.type);
		tmp = GetSection(block_name, use_node, cond.value, "post", cond.type);
		result.insert(result.end(), tmp.begin(), tmp.end());
		break;
	case GT:
		//��ȡ������ֵ�����һ��ֵ�����ƫ�ƽ����
		result = GetSection(block_name, use_node, cond.value, "post", cond.type);
		break;
	case GE:
		//��ȡ������ֵ�����һ��ֵ�����ƫ�ƽ�������������������ֵ��ͬ�����ݣ���������
		result = GetSection(block_name, use_node, cond.value, "post", cond.type);
		for (int i = 0; i < use_node.V_P.size(); ++i)
			if (ValueCompare(use_node.V_P[i].value, cond.type, EQ, cond.value))
			{
				result.push_back(use_node.V_P[i].pointer);
				break;
			}
		break;
	case LT:
		//��ȡ���ʼ��ֵ������ֵ�����ƫ�ƽ����
		result = GetSection(block_name, use_node, cond.value, "pre", cond.type);
		break;
	case LE:
		//��ȡ���ʼ��ֵ������ֵ�����ƫ�ƽ�������������������ֵ��ͬ�����ݣ���������
		result = GetSection(block_name, use_node, cond.value, "pre", cond.type);
		for (int i = 0; i < use_node.V_P.size(); ++i)
			if (ValueCompare(use_node.V_P[i].value, cond.type, EQ, cond.value))
			{
				result.push_back(use_node.V_P[i].pointer);
				break;
			}
		break;
	default:
		break;
	}
	
	sort(result.begin(), result.end());				//�Խ�����������򣬷���ִ�н�������
	return result;
}

//��ȡ���ʼ��ֵ������ֵ���ߴ�����ֵ������ֵ��ƫ������Ľ����
vector<int> IndexManager::GetSection(string block_name, node use_node, string value, string mode,attr_type a_type)
{
	vector<int> result;
	if (mode == "pre")
	{
		node read_node;
		read_node = GetNode(FindBeginBlock(block_name,use_node.len), use_node.len);
		while (read_node.SelfPointer != use_node.SelfPointer)
		{
			for (int i = 0; i < read_node.value_num; ++i)
				result.push_back(read_node.V_P[i].pointer);
			int next_block;
			next_block = get_block(INDEX, block_name, read_node.LastPointer);
			read_node = GetNode(next_block, use_node.len);
		}
		for (int i = 0; i < read_node.value_num; ++i)
		{
			if (ValueCompare(read_node.V_P[i].value, a_type, LT, value) == true)
				result.push_back(read_node.V_P[i].pointer);
			else
				break;
		}
	}
	else if(mode == "post")
	{
		node read_node = use_node;

		for (int i = 0; i < read_node.value_num; ++i)
			if (ValueCompare(read_node.V_P[i].value, a_type, GT, value) == true)
				result.push_back(read_node.V_P[i].pointer);
		while (read_node.LastPointer != 0)
		{
			int next_block;
			next_block = get_block(INDEX, block_name, read_node.LastPointer);
			read_node = GetNode(next_block, use_node.len);
			for (int i = 0; i < read_node.value_num; ++i)
				result.push_back(read_node.V_P[i].pointer);
		}
	}
	return result;
}

//�ҵ���һ��Ҷ�ڵ�Ŀ��
int IndexManager::FindBeginBlock(string block_name,int len)
{
	int block_num;
	node use_node;

	block_num = get_block(INDEX, block_name, 1);
	use_node = GetNode(block_num, len);
	while (use_node.type != LEAF && use_node.type != RANDL)
	{
		block_num = get_block(INDEX, block_name, use_node.V_P[0].pointer);
		use_node = GetNode(block_num, len);
	}
	return block_num;
}

//��map�е�һ��λ������Ϊ��
void IndexManager::FreeMapSite(string block_name, int Page)
{
	int infoBlock_num;
	char* write_site;

	infoBlock_num = get_block(INDEX, block_name, 0);
	write_site = BlockSet[infoBlock_num].block + 9;
	write_site[Page - 1] = '0';
	BlockSet[infoBlock_num].isChanged = true;
	used_block(infoBlock_num);
}

//����index����Ψһ�Լ��
error_type IndexManager::UniqueCheck(string table_name, condition cond)
{
	vector<int> result;

	result = SelectIndex(table_name, cond);

	if (result.size() != 0)
		return UNIERR;
	else
		return NOERR;
}


//��ȡ�ܿ���
int IndexManager::GetNum(string block_name)
{
	int InfoBlock_num;
	int block_num;
	stringstream buffer;

	InfoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[InfoBlock_num].block, 9);
	block_num = atoi(buffer.str().substr(4, 4).c_str());
	
	return block_num;
}
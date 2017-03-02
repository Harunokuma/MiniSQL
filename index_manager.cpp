#include "index_manager.h"

//根据块来建立当前块的结构化表示
node IndexManager::GetNode(int block_num, int len)
{
	node tmp;				//储存结果的node
	stringstream buffer;	//提供字符串处理

	//读入块的内容
	buffer.write(BlockSet[block_num].block, BLOCK_SIZE);

	tmp.SelfBlock_num = block_num;										//设置当前块的块号
	tmp.len = len;														//设置该index的属性长度
	tmp.type = atoi(buffer.str().substr(0, 1).c_str());					//设置该index块的类型（根、叶节点等）
	tmp.ParentBlockPointer = atoi(buffer.str().substr(1, 4).c_str());	//设置父节点的页数
	tmp.SelfPointer = atoi(buffer.str().substr(5, 4).c_str());			//设置自身的页数
	tmp.value_num = atoi(buffer.str().substr(9, 4).c_str());			//设置自身数据数据的数量

	//设置块中的各个值和指针
	for (int i = 0; i < tmp.value_num; ++i)
	{
		value_pointer tmp_vp;
		tmp_vp.pointer = atoi(buffer.str().substr(13+(len+4)*i, 4).c_str());
		tmp_vp.value = buffer.str().substr(17 + (len + 4)*i, len);
		tmp.V_P.push_back(tmp_vp);
	}

	//设置块中的最后一个指针
	tmp.LastPointer = atoi(buffer.str().substr(13 + (len + 4)*tmp.value_num,4).c_str());

	return tmp;
}

//将节点中的信息写回块中
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

//将一个值插入索引中
void IndexManager::InsertIndex(string table_name, string attr_name, string value,int offset)
{
	string block_name = table_name + "_" + attr_name;		//块名
	int attr_len;											//属性长度
	int block_num;											//块的数量
	int value_per_node;										//一个节点中最多包含的值的数量
	attr_type a_type;										//属性类型
	char* map;												//map数据的指针
	int infoBlock_num, useBlock_num;						//结构信息的块号和需要使用的块号
	stringstream buffer;									//处理字符串
	node node_tmp;											//临时储存node节点

	//读取索引文件的结构信息，获取其属性长度、块数、属性类型、map指针和节点中最多包含值的数量
	infoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[infoBlock_num].block, 9);
	attr_len = atoi(buffer.str().substr(0, 4).c_str());
	block_num = atoi(buffer.str().substr(4, 4).c_str());
	a_type = atoi(buffer.str().substr(8, 1).c_str());
	map = BlockSet[infoBlock_num].block + 9;
	value_per_node = (BLOCK_SIZE - 13) / (attr_len + 4) - 2;

	//索引中还没有节点
	if (block_num == 0)
	{
		node tmp;						//临时储存node节点
		int WritePage;					//可以写入的页数
		int WriteBlockNum;				//可以写入的块号
		value_pointer tmp_vp;			//临时储存值-指针对

		//将新增的值和指针存进数据结构中
		tmp_vp.value = value;
		tmp_vp.pointer = offset;

		//设置新的节点信息
		tmp.V_P.push_back(tmp_vp);
		tmp.type = RANDL;
		tmp.len = attr_len;
		tmp.value_num = 1;
		tmp.LastPointer = 0;
		tmp.ParentBlockPointer = 0;
		WritePage = FindSite(map);									//找到空余的页
		WriteBlockNum = get_block(INDEX, block_name, WritePage);	
		tmp.SelfPointer = WritePage;
		tmp.SelfBlock_num = WriteBlockNum;
		WriteNode(tmp);
		used_block(WriteBlockNum);
		AddBlockNum(block_name);									//对块的总数加一
		return;
	}

	//找到需要插入的叶节点
	useBlock_num = FindLeaf(block_name, 1, attr_len, value, a_type);
	node_tmp = GetNode(useBlock_num,attr_len);

	//叶节点中还有空位
	if (node_tmp.value_num < value_per_node)
		InsertInLeaf(useBlock_num, value, attr_len, offset,a_type);
	//叶节点中没有空位
	else
	{
		node leaf_1, leaf_2;
		int newPage;

		newPage = GetNewBlockSite(block_name);
		InsertInLeaf(useBlock_num, value, attr_len, offset,a_type);								//将新值插入该叶节点L中
		leaf_1 = GetNode(useBlock_num, attr_len);
		leaf_2 = GetNode(get_block(INDEX, block_name, newPage),attr_len);						//新建一个节点L'
		leaf_2.SelfPointer = newPage;
		leaf_2.LastPointer = leaf_1.LastPointer;												//L'尾部指针继承L
		leaf_1.LastPointer = leaf_2.SelfPointer;												//L尾部指针指向L'
		leaf_2.ParentBlockPointer = leaf_1.ParentBlockPointer;									//L'父指针继承L
		leaf_2.type = leaf_1.type;																//L'节点类型继承L

		//两个节点各获取一半的值
		auto it = leaf_1.V_P.begin();
		it += value_per_node / 2;
		leaf_2.V_P.assign(it, leaf_1.V_P.end());
		leaf_1.V_P.erase(it, leaf_1.V_P.end());
		leaf_2.value_num = leaf_2.V_P.size();
		leaf_1.value_num = leaf_1.V_P.size();
		WriteNode(leaf_1);
		WriteNode(leaf_2);

		//执行向父节点插入的操作
		InsertInParent(leaf_1, leaf_2.V_P[0].value, leaf_2,block_name);
	}
}

//找到该值应在的叶节点，返回叶节点的块号
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

//将新值插入叶节点中
void IndexManager::InsertInLeaf(int block_num, string value, int len, int offset,attr_type a_type)
{
	node tmp;
	value_pointer tmp_vp;

	tmp = GetNode(block_num, len);
	tmp.value_num++;
	tmp_vp.value = value;
	tmp_vp.pointer = offset;

	//找到新值应该在的位置，将其插入
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

//向父节点中插入值
void IndexManager::InsertInParent(node Node_1, string min_value, node Node_2, string block_name)
{
	int attr_len;				//属性长度
	int infoBlock_num;			//结构信息块号
	int value_per_node;			//一个节点中最多包含的值的数量
	stringstream buffer;		//处理字符串
	node parent;				//父节点

	//读取索引文件的结构信息，获取其属性长度和节点中最多包含值的数量
	infoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[infoBlock_num].block, 9);
	attr_len = atoi(buffer.str().substr(0, 4).c_str());
	value_per_node = (BLOCK_SIZE - 13) / (attr_len + 4) - 2;

	//获取父节点的节点
	if(Node_1.type != ROOT && Node_1.type != RANDL)
		parent = GetNode(get_block(INDEX, block_name, Node_1.ParentBlockPointer), attr_len);

	//如果分裂的节点是根节点
	if (Node_1.type == ROOT || Node_1.type == RANDL)
	{
		node tmp;
		value_pointer tmp_vp;
		int newPage;

		//新建一个节点，复制根节点的信息
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

		//使分裂出来的节点的父节点为根节点，改变其类型
		Node_2.ParentBlockPointer = Node_1.SelfPointer;
		if (Node_1.type == ROOT)
			Node_2.type = INNER;
		else if (Node_1.type == RANDL)
			Node_2.type = LEAF;
		
		//改变根节点的类型，使其尾部指针指向分裂出来的节点
		Node_1.type = ROOT;
		Node_1.LastPointer = Node_2.SelfPointer;
		
		//将新建的节点的值信息和指针存入根节点
		tmp_vp.pointer = tmp.SelfPointer;
		tmp_vp.value = min_value;
		Node_1.V_P.clear();
		Node_1.V_P.push_back(tmp_vp);
		Node_1.value_num = 1;
		
		//将各个节点写回块中
		WriteNode(tmp);
		WriteNode(Node_1);
		WriteNode(Node_2);
	}

	//父节点中还有空余，直接找到位置将新增值插入
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

	//父节点中没有空余，执行和之前分裂操作相似的操作
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

//获取一个新的页号
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

//根据值删除一个索引
void IndexManager::DeleteIndex(string table_name, string attr_name, string value)
{
	string block_name;			//块名
	int block_num;				//块号
	int infoBlock_num;			//结构信息块号
	node tmp;					//临时储存节点
	int attr_len;				//属性长度
	attr_type type;				//属性类型
	int value_per_node;			//一个节点中最多包含的值的数量
	stringstream buffer;		//处理字符串

	block_name = table_name + "_" + attr_name;

	//读取索引文件的结构信息，获取其属性长度和节点中最多包含值的数量
	infoBlock_num = get_block(INDEX, block_name, 0);
	buffer.write(BlockSet[infoBlock_num].block, 9);
	attr_len = atoi(buffer.str().substr(0, 4).c_str());
	type = atoi(buffer.str().substr(8, 1).c_str());
	value_per_node = (BLOCK_SIZE - 13) / (attr_len + 4) - 2;
	
	block_num = FindLeaf(block_name, 1, attr_len, value, type);
	tmp = GetNode(block_num, attr_len);
	delete_entry(block_name, tmp, value, type);
}

//在节点上执行删除操作
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

	//如果该节点为根节点而且只剩下一个值-指针对
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

	//如果该节点中值过少
	else if (Node.type != RANDL && Node.type != ROOT && Node.value_num < value_per_node)
	{

	}
}

//获取空余的块位置
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

//改变一个节点的父节点指针
void IndexManager::ChangeParentPointer(string block_name, int len, int SelfPage, int ParentPage)
{
	node tmp;
	int block_num;

	block_num = get_block(INDEX, block_name, SelfPage);
	tmp = GetNode(block_num, len);
	tmp.ParentBlockPointer = ParentPage;
	WriteNode(tmp);
}

//对总块数加一
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

//对总块数减一
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

//从索引中搜索到符合条件的值的偏移
vector<int> IndexManager::SelectIndex(string table_name, condition cond)
{
	int block_num;				//块号
	int num;					//块数
	string block_name;			//块名
	node use_node;				//使用的节点
	vector<int> result;			//结果集
	vector<int> tmp;			//临时储存的结果集

	//找到值应该在的节点
	block_name = table_name + "_" + cond.attr_name;
	num = GetNum(block_name);
	if (num < 1)
		return result;
	block_num = FindLeaf(block_name, 1, cond.attr_length, cond.value, cond.type);
	use_node = GetNode(block_num, cond.attr_length);

	switch (cond.cond)
	{
	case EQ:
		//在该节点中搜索是否存在该值，如果存在，则加入结果集
		for (int i = 0; i < use_node.V_P.size(); ++i)
			if (ValueCompare(use_node.V_P[i].value, cond.type, EQ, cond.value))
			{
				result.push_back(use_node.V_P[i].pointer);
				break;
			}
		break;
	case NE:
		//获取从最开始的值到输入值区间和输入值到最后一个值区间的偏移的并集
		result = GetSection(block_name, use_node, cond.value, "pre", cond.type);
		tmp = GetSection(block_name, use_node, cond.value, "post", cond.type);
		result.insert(result.end(), tmp.begin(), tmp.end());
		break;
	case GT:
		//获取从输入值到最后一个值区间的偏移结果集
		result = GetSection(block_name, use_node, cond.value, "post", cond.type);
		break;
	case GE:
		//获取从输入值到最后一个值区间的偏移结果集，如果存在与输入值相同的数据，加入结果集
		result = GetSection(block_name, use_node, cond.value, "post", cond.type);
		for (int i = 0; i < use_node.V_P.size(); ++i)
			if (ValueCompare(use_node.V_P[i].value, cond.type, EQ, cond.value))
			{
				result.push_back(use_node.V_P[i].pointer);
				break;
			}
		break;
	case LT:
		//获取从最开始的值到输入值区间的偏移结果集
		result = GetSection(block_name, use_node, cond.value, "pre", cond.type);
		break;
	case LE:
		//获取从最开始的值到输入值区间的偏移结果集，如果存在与输入值相同的数据，加入结果集
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
	
	sort(result.begin(), result.end());				//对结果集进行排序，方便执行交集操作
	return result;
}

//获取从最开始的值到输入值或者从输入值到最后的值的偏移区间的结果集
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

//找到第一个叶节点的块号
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

//将map中的一个位置设置为空
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

//利用index进行唯一性检查
error_type IndexManager::UniqueCheck(string table_name, condition cond)
{
	vector<int> result;

	result = SelectIndex(table_name, cond);

	if (result.size() != 0)
		return UNIERR;
	else
		return NOERR;
}


//获取总块数
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
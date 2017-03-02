#include "MiniSQL.h"
#include "iterpreter.h"
#include <iostream>
using namespace std;
int main()
{
	iterpreter();
	return 0;
}

string read_error(error_type error)
{
	string s("-------------------------------------------------------------------------\n");
	switch (error)
	{
	case NOERR:
		return "Operation succeeded!\n"+s;
	case SYNERR:
		return "syntax error!\n"+s;
	case PRIERR:
		return "not exist this primary key!\n"+s;
	case FLERR:
		return "read file fail!\n"+s;
	case OPERR:
		return "operation type error!\n"+s;
	case TABEXT:
		return "this table already exist!\n"+s;
	case NOTAB:
		return "this table not exist!\n"+s;
	case VALERR:
		return "values are not match table!\n"+s;
	case IDXEXT:
		return "this index already exist!\n"+s;
	case UNIERR:
		return "unique error!\n"+s;
	case NOIDX:
		return "this index not exist!\n"+s;
	case CONERR:
		return "conditions are not match table!\n"+s;
	default:
		return "wtf?\n"+s;
	}
}
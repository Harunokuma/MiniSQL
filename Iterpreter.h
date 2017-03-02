#ifndef ITERPRETER_H
#define ITERPRETER_H

#include "API.h"
#include <string>
#include <iostream>
#define CREATE_TABLE_REGEX "^create table [a-z0-9_]+\\(([a-z0-9_]+ ((int)|(float)|(char\\(\\d+\\)))( unique)?)(,[a-z0-9_]+ ((int)|(float)|(char\\(\\d+\\)))( unique)?)*((,[a-z0-9_]+ ((int)|(float)|(char\\(\\d+\\)))( unique)?)|(,primary key\\([a-z0-9_]+\\)))?\\);(\\n)?$"
#define DROP_TABLE_REGEX "^drop table [a-z0-9_]+;(\\n)?$"
#define CREATE_INDEX_REGEX "^create index [a-z0-9_]+ on [a-z0-9_]+\\([a-z0-9_]+\\);(\\n)?$"
#define DROP_INDEX_REGEX "^drop index [a-z0-9_]+;(\\n)?$"
#define SELECT_REGEX "^select \\* from [a-z0-9_]+( where [a-z0-9_]+((=)|(<)|(>)|(<>)|(<=)|(>=))(('[a-z]+')|([0-9//.]+))( and [a-z0-9_]+((=)|(<)|(>)|(<>)|(<=)|(>=))(('[a-z]+')|([0-9//.]+)))*)?;(\\n)?$"
#define INSERT_REGEX "^insert into [a-z0-9_]+ values\\((([0-9//.]+)|('[a-z]+'))(,( )?(([0-9//.]+)|('[a-z]+')))*\\);(\\n)?$"
#define DELETE_REGEX "^delete from [a-z0-9_]+( where [a-z0-9_]+((=)|(<)|(>)|(<>)|(<=)|(>=))(('[a-z]+')|([0-9//.]+))( and [a-z0-9_]+((=)|(<)|(>)|(<>)|(<=)|(>=))(('[a-z]+')|([0-9//.]+)))*)?;(\\n)?$"

void iterpreter();
error_type SQL_to_Command(string SQL_string);
error_type create_table_tras(string SQL_string);
error_type drop_table_tras(string SQL_string);
error_type create_index_tras(string SQL_string);
error_type drop_index_tras(string SQL_string);
error_type select_tras(string SQL_string);
error_type insert_tras(string SQL_string);
error_type delete_tras(string SQL_string);
error_type exec_file(string SQL_string);

#endif

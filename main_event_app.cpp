#include <string.h>
#include <functional>
#include <iostream>
#include <iterator>
#include <utility>

#include "EventApp.h"

using namespace std;

int main( int argc, char ** argv )
{
    if( argc < 2 ){
	cerr << "Please give query parameters: \"field1:arg1|field2:arg2|... \"" << endl;
    	return -1;
    }

    //extract arguments
    map<string,string> arg_map;
    string args = argv[1];
    string delimiters = "|";
    size_t last = 0;
    size_t next = args.find_first_of( delimiters, last );
    vector<string> args_list;
    while( string::npos != next ){
	string extracted = args.substr( last, next-last );
	args_list.push_back( extracted );
	++next;
	last = next;
	next = args.find_first_of( delimiters, last );
    }
    if( string::npos != last ){
	string extracted = args.substr( last, next-last );
	args_list.push_back( extracted );
    }
    for( auto i : args_list ){
	size_t find_field = i.find_first_of(":");
	if( string::npos == find_field || i.length() == 1 ){
	    cerr << "invalid field:arg pair" << endl;
	    return -1;
	}
	string fieldname = i.substr(0, find_field);
	string fieldval = i.substr( find_field + 1 );
	arg_map.insert(std::pair<string,string>(fieldname, fieldval));
    }
#ifdef DEBUG
    for( auto i : arg_map ){
	cout <<"field: " << i.first << ", val: " << i.second << endl;
    }
#endif
    
    EventApp event_app;
    bool bRet;
    bRet = event_app.RegisterContentExtraction( { "title", "venue_name", "start_time", "venue_address" } );
    if( !bRet ){
	cerr << "Registering extraction information failed." << endl;
	return -1;
    }
    // bRet = event_app.Query( { { "location", "San Diego" }, { "within", "200" }, { "category", "business" }, { "date", "2016020100-2016120100" },{ "page_size", "20" } } );
    bRet = event_app.Query( arg_map );
    if( !bRet ){
	cerr << "Query failed." << endl;
	return -1;
    }
    return 0;
}

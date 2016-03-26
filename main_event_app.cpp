#include <string.h>
#include <functional>
#include <iostream>
#include <iterator>

#include "EventApp.h"

using namespace std;

int main( int argc, char ** argv )
{
    // if( argc < 1 )
    // 	return 0;
    
    // string query_args = "&keywords=books&location=San+Diego&category=technology&within=20";
    string query_args = "&location=San%20Diego&category=technology&within=200";
    EventApp event_app;
    bool bRet;
    EventApp::t_extraction_func func_cout = []( vector<pair<string,string> > extracted ) -> bool {
	auto it_start = extracted.begin();
	auto it_end = extracted.end();
	for( auto it = it_start; it != it_end; ++it ){
	    string label = it->first;
	    string detail = it->second;
	    if( it == it_start ){
		cout << "[ ";
	    }
	    cout << label << ": " << detail;
	    auto it_next = std::next(it);
	    if( it_next != it_end ){
		cout << " $ ";
	    }else{
		cout << " ]" << endl;
	    }
	}
	return true;
    };
    bRet = event_app.RegisterContentExtraction( { "title", "venue_name", "city_name", "venue_address" } );
    bRet = event_app.Query( { { "location", "San%20Diego" }, { "within", "200" }, { "page_size", "10" } } );
    return 0;
}

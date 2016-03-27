#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <utility>

#include "StringSearch.h"

using namespace std;

TEST_CASE( "String Search", "[A]" ) {

    StringSearch str_search;

    set<pair<int,int> > found;
    
    auto func_something = [&]( int index_found, int length )->bool{
	//cout << "something found at: " << index_found << ", length: " << length << endl;
	found.insert( std::pair<int,int>( index_found, length ) );
	return true;
    };
    
    SECTION( "Register search string" ) {
	str_search.RegisterStringToSearch( "something", func_something );
    }
    SECTION( "Search input" ) {
	str_search.RegisterStringToSearch( "something", func_something );
	str_search.RegisterStringToSearch( "here", func_something );
	str_search.Search( "some random sentence having the word something here and also here: something" );
	CHECK( found.end() != found.find( std::pair<int,int>( 37, 9 ) ) );
	CHECK( found.end() != found.find( std::pair<int,int>( 67, 9 ) ) );
	CHECK( found.end() != found.find( std::pair<int,int>( 47, 4 ) ) );
	CHECK( found.end() != found.find( std::pair<int,int>( 61, 4 ) ) );
    }
}

#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <map>
#include <string>

#include "EventApp.h"
#include "Test.h"

using namespace std;

TEST_CASE( "Internal Functions", "[Internal]" ) {

    EventApp event_app;
    EventApp * p_event_app = &event_app;
    Tester tester;
    bool bRet;
    
    SECTION( "ProcessQueryArgs Zero Item" ) {
	map<string, string> args {};
	string query;
	bRet = tester.TestProcessQueryArgs( p_event_app, args, query );
	CHECK( true == bRet );
	CHECK( (0 == query.length()) );
    }
    SECTION( "ProcessQueryArgs One Item" ) {
	map<string, string> args { {"ABC", "abc" } };
	string query;
	bRet = tester.TestProcessQueryArgs( p_event_app, args, query );
	CHECK( true == bRet );
	CHECK( (string::npos != query.find("&ABC=abc") ) );
    }
    SECTION( "ProcessQueryArgs Three Items" ) {
	map<string, string> args { {"ABC", "abc" }, {"xyz", "GHJ" }, {"Laptops", "mac" } };
	string query;
	bRet = tester.TestProcessQueryArgs( p_event_app, args, query );
	CHECK( true == bRet );
	CHECK( (string::npos != query.find("&ABC=abc") ) );
	CHECK( (string::npos != query.find("&xyz=GHJ") ) );
	CHECK( (string::npos != query.find("&Laptops=mac") ) );
    }
    SECTION( "ProcessExtract Valid" ) {
	string extracted;
	bRet = tester.TestProcessExtract( p_event_app, "<some label here>7778</some label here>", "some label here", extracted );
	CHECK( true == bRet );
	CHECK( ( "7778" == extracted ) );
    }
    SECTION( "ProcessExtract Invalid" ) {
	string extracted;
	bRet = tester.TestProcessExtract( p_event_app, "<some label here>7778</some label>", "some label here", extracted );
	CHECK( false == bRet );
    }
    SECTION( "ProcessExtract Number Valid" ) {
	int extracted;
	bRet = tester.TestProcessExtract( p_event_app, "<some label here>778</some label here>", "some label here", extracted );
	CHECK( true == bRet );
	CHECK( 778 == extracted );
    }
    SECTION( "ProcessExtract Number Invalid" ) {
	int extracted;
	bRet = tester.TestProcessExtract( p_event_app, "<some label here>77x8</some label>", "some label here", extracted );
	CHECK( false == bRet );
    }
    SECTION( "ProcessEscapeCharacters" ) {
	map<string,string> escape_chars { {" ","%20"}, {"$", "%24"}, {"&","%26"} };
	string escaped;
	string input = "Hello, here is $25 dollars & you can leave";
	bRet = tester.TestProcessEscapeCharacters( p_event_app, input, escape_chars, escaped );
	CHECK( true == bRet );
	cout << escaped << endl;
	CHECK( "Hello,%20here%20is%20%2425%20dollars%20%26%20you%20can%20leave"== escaped );
    }
        
}

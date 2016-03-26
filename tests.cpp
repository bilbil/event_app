#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <Sstream>

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
	CHECK( "Hello,%20here%20is%20%2425%20dollars%20%26%20you%20can%20leave"== escaped );
    }
    SECTION( "ProcessEscapeCharacters XML" ) {
	map<string,string> escape_chars { {"&amp;","&"} };
	string escaped;
	string input = "1 &amp; 2 3 4 &amp; 5";
	bRet = tester.TestProcessEscapeCharacters( p_event_app, input, escape_chars, escaped );
	CHECK( true == bRet );
	CHECK( "1 & 2 3 4 & 5"== escaped );
    }
    SECTION( "Local Input Source XML Processing: local.xml" ) {
	ifstream in("local.xml");
	streambuf *cinbuf = cin.rdbuf(); //save old buf
	cin.rdbuf(in.rdbuf()); //redirect cin to file
	string line;
	stringstream ss;
	ss.str("");
	ss.clear();
	while(std::getline(cin, line))  //input from the file in.txt
	{
	    ss << line;   //output to the file out.txt
	}
	cin.rdbuf(cinbuf);   //reset to standard input again

	string data = ss.str();
#ifdef DEBUG
	cout << data << endl;
#endif
	//expect 100 items in sample XML file
	int count_expected = 100;
	int count_retrieved = 0;
	//create a lambda to count number of events extracted
	EventApp::t_extraction_func func_extraction = [&count_retrieved]( vector<pair<string,string> > extracted )->bool {
	    ++count_retrieved;
	    return true;
	};
	bRet = tester.TestRegisterContentExtraction( p_event_app, { "title", "venue_name", "start_time", "venue_address" }, func_extraction );
	CHECK( true == bRet );
	bRet = tester.TestProcessQuerySimulated( p_event_app, "arbitrary query arguments", data.c_str() );
	CHECK( true == bRet );
	CHECK( count_expected == count_retrieved );
    }
}

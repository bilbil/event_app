#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <utility>
#include <mutex>

#include <curl/curl.h>

#include "EventApp.h"

using namespace std;

std::mutex EventApp::_mutex_print;

bool EventApp::ProcessQuery( string query_arguments )
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if( !curl ){
	return false;
    }
    //sample query
    //string query = "http://api.eventful.com/rest/events/search?app_key=Bv3nNxLh5nv8rx4b&keywords=books&location=San+Diego&category=technology";
    
    /* Perform the request, res will get the return code */
    do {
	stringstream ss;
	ss.str("");
	ss << _chunk._current_page_num;
	string current_page;
	ss >> current_page;
	string query = "http://api.eventful.com/rest/events/search?app_key=Bv3nNxLh5nv8rx4b" + query_arguments;
	query += "&page_number=";
	query += current_page;
	curl_easy_setopt( curl, CURLOPT_URL, query.c_str() );
	/* send all data to this function  */ 
	curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, EventApp::EventExtractCallback );
	/* we pass our 'chunk' struct to the callback function */ 
	curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *) this );
	res = curl_easy_perform( curl );
	cout << "-----"<< endl;
	cout << "total pages: " <<  _chunk._total_page_num << endl;
	cout << "current page: " <<  _chunk._current_page_num << endl;
	cout << "-----"<< endl;
	++_chunk._current_page_num;
    }while( res == CURLE_OK && _chunk._current_page_num <= _chunk._total_page_num );

    /* Check for errors */ 
    if(res != CURLE_OK){
#ifdef DEBUG
	fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
#endif
    } else {
	cout << "-----"<< endl;
	fprintf( stdout, "%lu bytes retrieved\n", (long)_chunk._total_bytes_retrieved );
	cout << "Number of events found: " << _chunk._count_event_found << endl;
    }
    /* always cleanup */ 
    curl_easy_cleanup(curl);

    return true;
}
size_t EventApp::EventExtractCallback( void * contents, size_t size, size_t nmemb, void * data )
{
    EventApp * event_app = ( EventApp *) data;
    std::lock_guard<std::mutex> lock( event_app->_mutex );

    size_t realsize = size * nmemb;
    
    struct EventApp::MemoryStruct * mem = &event_app->_chunk;
    mem->_total_bytes_retrieved += realsize;

    //create temporary buffer with null termination
    char * content_buffer = (char*)malloc( realsize + 1 );
    if( ! content_buffer ){
	return 0;
    }
    //copy streaming data into buffer
    memcpy( content_buffer, contents, realsize );
    content_buffer[ realsize ] = '\0';

    string content_str = mem->_data_buffer + string( content_buffer );

    //search for certain keywords, assume xml format
    int total_page_number;
    if( event_app->ProcessExtract( content_str, "page_count", total_page_number ) ) //save when found
    {
    	mem->_total_page_num = total_page_number;
    }

    int current_page_number;
    if( event_app->ProcessExtract( content_str, "page_number", current_page_number ) ) //save when found
    {
    	mem->_current_page_num = current_page_number;
    }
    
    size_t found_event_start = content_str.find( "<event id=" );
    if( string::npos != found_event_start ){
	found_event_start = content_str.find( ">", found_event_start );
	if( string::npos != found_event_start )
	    ++found_event_start;
    }

    size_t last_found = string::npos;
    bool bFound = false;
    while( string::npos != found_event_start && found_event_start < nmemb ){
#ifdef DEBUG
	cout << "Found <event>" << endl;
#endif
	string end_tag = "</event>";
	size_t found_event_end = content_str.find( end_tag, found_event_start );
        if( string::npos != found_event_end ){
#ifdef DEBUG
	    cout << "Found </event>" << endl;
#endif
	}else{
	    break;
	}
	//found event
	++mem->_count_event_found;

	size_t str_size = found_event_end - found_event_start;
	string event_content( content_str, found_event_start, str_size );
	found_event_end += end_tag.length();
	last_found = found_event_end;
	bFound = true;

	//process found event
	event_app->ProcessEventContent( event_content );
	
	size_t next = found_event_end + 1;

	found_event_start = content_str.find( "<event id=", next );
	if( string::npos != found_event_start ){
	    found_event_start = content_str.find( ">", found_event_start );
	    if( string::npos != found_event_start )
		++found_event_start;
	}else{
	    break;
	}
    }

    //remove found event contents if applicable
    if( ( string::npos == last_found || content_str.length() == 0 ) && !bFound ){
	size_t size_remain = 0;
	mem->_size = size_remain;
	mem->_data_buffer = content_str;
    }else{
	if( string::npos == last_found ){
	    content_str = "";
	    size_t size_remain = 0;
	    mem->_size = size_remain;
	}else{
	    content_str = content_str.substr( last_found );
	    size_t size_remain = content_str.length() - last_found;
	    mem->_size = size_remain;
	}
	mem->_data_buffer = content_str;
    }

    free( content_buffer );
    
    return realsize;
}
bool EventApp::RegisterContentExtraction( std::vector< std::string > labels, t_extraction_func func )
{
    _content_extraction_labels = labels;
    _content_extraction_func = func;
    return true;
}
bool EventApp::ProcessExtract( std::string input, std::string label, std::string & extracted ){
    //assume xml format
    string label_start = "<" + label;
    label_start += ">";
    string label_end = "</" + label;
    label_end += ">";
    //extract content detail
    size_t index_start = input.find( label_start );
    if( string::npos == index_start )
	return false;
    size_t index_end = input.find( label_end, index_start + label_start.length() );
    if( string::npos == index_end )
	return false;

    extracted = input.substr( index_start + label_start.length(), index_end - index_start - label_start.length() );
    return true;
}
bool EventApp::ProcessExtract( std::string input, std::string label, int & extracted_int ){
    std::string extracted;
    if( !ProcessExtract( input, label, extracted ) )
	return false;

    stringstream ss;
    ss.str("");
    ss << extracted;
    ss >> extracted_int;
    if( ss.fail() )
	return false;
	    
    return true;
}
bool EventApp::ProcessEventContent( std::string event_content ){
#ifdef DEBUG
    cout << "Event Content: " << event_content << endl;
#endif

    //do a simple search for each registered content
    vector<pair<string,string> > extracted;
    auto it = _content_extraction_labels.begin();
    auto it_end = _content_extraction_labels.end();
    for( ; it != it_end; ++it ){
    	//assume xml format
    	string label = *it;
    	string label_start = "<" + label;
    	label_start += ">";
    	string label_end = "</" + label;
    	label_end += ">";
    	//extract content detail
    	size_t index_start = event_content.find( label_start );
    	if( string::npos == index_start ){
    	    continue;
    	}
    	size_t index_end = event_content.find( label_end, index_start + label_start.length() );
    	string content_detail = event_content.substr( index_start + label_start.length(), index_end - index_start - label_start.length() );

    	//escape characters in XML
    	map<string,string> escape_chars { {"&quot;","\""}, {"&amp;", "&"}, {"&apos;","'"}, {"&lt;","<"}, {"&gt;",">"} };
    	string escaped_content;
    	ProcessEscapeCharacters( content_detail, escape_chars, escaped_content );

    	extracted.push_back( pair<string,string>( label, escaped_content ) );
    }
    //call function/functor
    auto func = _content_extraction_func;
    bool bRet = func( extracted );
    return bRet;
}
bool EventApp::DefaultPrintExtracted( vector<pair<string,string> > extracted )
{
    std::lock_guard<std::mutex> lock( _mutex_print );

    auto it_start = extracted.begin();
    auto it_end = extracted.end();
    for( auto it = it_start; it != it_end; ++it ){
    	string label = it->first;
    	string detail = it->second;
    	if( it == it_start ){
    	    cout << "{ ";
    	}
    	cout << label << ": " << detail;
    	auto it_next = std::next(it);
    	if( it_next != it_end ){
    	    cout << " | ";
    	}else{
    	    cout << " }" << endl;
    	}
    }
    return true;
}
bool EventApp::Query( map<string,string> args )
{
    string query_args;
    ProcessQueryArgs( args, query_args );
    bool bRet = ProcessQuery( query_args );
    return bRet;
}
bool EventApp::ProcessQueryArgs( map<string,string> args, string & query_args )
{
    query_args = "";
    map<string,string> escape_chars { {" ","%20"}, {"$", "%24"}, {"&","%26"}, {"`","%60"}, {":","%3A"}, {"<","%3C"},
    				      {">","%3E"}, {"[","%5B"}, {"]","%5D"}, {"{","%7B"}, {"}","%7D"}, {"\"","%22"},
    				      {"+","%2B"}, {"#","%23"}, {"%","%25"}, {"@","%40"}, {"/","%2F"}, {";","%3B"},
           			      {"=","%3D"}, {"?","%3F"}, {"\\","%5C"}, {"^","%5E"}, {"|","%7C"}, {"~","%7E"},
    				      {"'","%27"}, {",","%2C"}, {".","%2E"}
                                    };
    for( auto i : args ){
	string escaped_arg;
	string escaped_value;
	ProcessEscapeCharacters( i.first, escape_chars, escaped_arg );
	ProcessEscapeCharacters( i.second, escape_chars, escaped_value );

	query_args += "&";
	query_args += escaped_arg;
	query_args += "=";
	query_args += escaped_value;
    }
    cout << "query: " << query_args << endl;
    return true;
}

bool EventApp::ProcessEscapeCharacters( string input, map<string,string> escape_chars, string & escaped )
{
    size_t index_searched = 0;
    bool bFound;
    do {
	string str_first;
	string str_replace;
        bFound = false;
	//find the first that matches
	size_t index_first = string::npos;
	for( auto & i : escape_chars ){	    
	    size_t index_find = input.find( i.first, index_searched );
	    if( string::npos != index_find ){
		if( string::npos == index_first || index_find < index_first ){
		    index_first = index_find;
		    str_first = i.first;
		    str_replace = i.second;
		    bFound = true;
		}
	    }
	}
	//reform the string
	if( bFound ){
	    string before = input.substr( 0, index_first );
	    string after = input.substr( index_first + str_first.length(), string::npos );
	    input = before + str_replace;
	    input += after;
	    index_searched = index_first + str_replace.length();
	}
    }while( bFound );
    escaped = input;
    return true;
}

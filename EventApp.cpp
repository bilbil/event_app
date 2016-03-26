#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>
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
    //use xml format for simple parsing
    // string query = "http://api.eventful.com/rest/events/search?app_key=Bv3nNxLh5nv8rx4b&keywords=books&location=San+Diego&category=technology";
    string query = "http://api.eventful.com/rest/events/search?app_key=Bv3nNxLh5nv8rx4b" + query_arguments;
    curl_easy_setopt( curl, CURLOPT_URL, query.c_str() );

    // /* example.com is redirected, so we tell libcurl to follow redirection */ 
    // curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    /* send all data to this function  */ 
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, EventApp::EventExtractCallback );

    /* we pass our 'chunk' struct to the callback function */ 
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *) this );
    
    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform( curl );

    /* Check for errors */ 
    if(res != CURLE_OK){
#ifdef DEBUG
	fprintf( stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res) );
#endif
    } else {
// #ifdef DEBUG	
	printf( "%lu bytes retrieved\n", (long)_chunk._total_bytes_retrieved );
// #endif
    }
    /* always cleanup */ 
    curl_easy_cleanup(curl);

// #ifdef DEBUG
    cout << "Number of <event> found: " << _chunk._count_event_found << endl;
// #endif

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
    
#ifdef DEBUG
    cout << "Invoked WriteCallback: " << realsize << " bytes retrieved. " << endl;
    cout << "-------" << endl;
#endif

    //search for certain keywords, assume xml format
    size_t found_event_start = content_str.find( "<event id=" );
    if( string::npos != found_event_start ){
	found_event_start = content_str.find( ">", found_event_start );
	++found_event_start;
    }

    size_t last_found = string::npos;
    while( string::npos != found_event_start && found_event_start < nmemb ){
#ifdef DEBUG
	cout << "Found <event>" << endl;
#endif
	size_t found_event_end = content_str.find( "</event>", found_event_start );
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
	last_found = found_event_end;

	//process found event
	event_app->ProcessEventContent( event_content );

	size_t next = found_event_end + 1;
	if( string::npos == next ){
	    break;
	}
	found_event_start = content_str.find( "<event id=", next );
	if( string::npos != found_event_start ){
	    found_event_start = content_str.find( ">", found_event_start );
	    ++found_event_start;
	}
    }

    //remove found event contents if applicable
    if( string::npos == last_found || content_str.length() == 0 ){
	size_t size_remain = 0;
	mem->_size = size_remain;
	mem->_data_buffer = content_str;
    }else{
	content_str = content_str.substr( last_found );
	size_t size_remain = content_str.length() - last_found;
	mem->_size = size_remain;
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
	extracted.push_back( pair<string,string>( label, content_detail ) );
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

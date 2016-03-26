#include <cstdio>
#include <cstdlib>
#include <string.h>
#include <iostream>

#include <thread>
#include <chrono>

#include <curl/curl.h>
using namespace std;

struct MemoryStruct {
    char *memory;
    size_t size;
};

int iCountEventStart = 0;
int iCountEventEnd = 0;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
    mem->memory = (char*)realloc(mem->memory, mem->size + realsize + 1);
    if(mem->memory == NULL) {
	/* out of memory! */ 
	printf("not enough memory (realloc returned NULL)\n");
	return 0;
    }
 
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    char * content_buffer = (char*)malloc( realsize + 1 );
    memcpy( content_buffer, contents, realsize );
    content_buffer[ realsize + 1 ] = 0;
    
  
    cout << "Invoked WriteCallback: " << realsize << " bytes retrieved. " << endl;
    cout << content_buffer << endl;
    cout << "-------" << endl;

    //search for certain keywords
    char * found_event_start = strstr( content_buffer, "<event id=" );
    while( found_event_start ){
	cout << "Found <event>" << endl;
	++iCountEventStart;
        char * next = found_event_start + 1;
	if( *next == 0 ){
	    break;
	}
	found_event_start = strstr( next, "<event id=" );
    }

    char * found_event_end = strstr( content_buffer, "</event>" );
    while( found_event_end ){
	cout << "Found </event>" << endl;
	++iCountEventEnd;
	char * next = found_event_end + 1;
	if( *next == 0 ){
	    break;
	}
        found_event_end = strstr( next, "</event>" );
    }
    
    free( content_buffer );

    // cout << "Sleep" << endl;
    // std::this_thread::sleep_for (std::chrono::seconds(1));

    return realsize;
}

int main(void)
{
    CURL *curl;
    CURLcode res;

    struct MemoryStruct chunk;
 
    chunk.memory = (char*)malloc(1);  /* will be grown as needed by the realloc above */ 
    chunk.size = 0;    /* no data at this point */
  
    curl = curl_easy_init();
    if(curl) {
	//curl_easy_setopt(curl, CURLOPT_URL, "http://api.eventful.com/yaml/events/search?app_key=Bv3nNxLh5nv8rx4b&keywords=books&location=San+Diego&category=technology");
	curl_easy_setopt(curl, CURLOPT_URL, "http://api.eventful.com/rest/events/search?app_key=Bv3nNxLh5nv8rx4b&keywords=books&location=San+Diego&category=technology");

	// /* example.com is redirected, so we tell libcurl to follow redirection */ 
	// curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	/* send all data to this function  */ 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

	/* we pass our 'chunk' struct to the callback function */ 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    
	/* Perform the request, res will get the return code */ 
	res = curl_easy_perform(curl);

	/* Check for errors */ 
	if(res != CURLE_OK)
	    fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	else
	    printf("%lu bytes retrieved\n", (long)chunk.size);


	free(chunk.memory);

	/* always cleanup */ 
	curl_easy_cleanup(curl);

	cout << "Number of <event> found: " << iCountEventStart << endl;
	cout << "Number of </event> found: " << iCountEventEnd << endl;
    }
    return 0;
}

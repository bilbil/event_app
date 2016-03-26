#ifndef EVENT_APP_H
#define EVENT_APP_H

#include <string>
#include <functional>
#include <map>
#include <vector>
#include <utility>
#include <mutex>

class EventApp {
public:
    using t_extraction_func = std::function<bool(std::vector<std::pair<std::string,std::string> >)>;
    struct MemoryStruct {
	std::string _data_buffer;
	size_t _size;
	int _count_event_found;
	size_t _total_bytes_retrieved;
	MemoryStruct(){
	    _data_buffer = "";
	    _size = 0;
	    _count_event_found = 0;
	    _total_bytes_retrieved = 0;
	}
    };
    bool ProcessQuery( std::string query_arguments );
    bool RegisterContentExtraction( std::vector<std::string> labels, t_extraction_func func = EventApp::DefaultPrintExtracted );
private:
    static bool DefaultPrintExtracted( std::vector<std::pair<std::string,std::string> > extracted );
    bool ProcessEventContent( std::string event_detail );
    //callback function provided to libcurl for processing received data
    static size_t EventExtractCallback( void * contents, size_t size, size_t nmemb, void * data );
    struct MemoryStruct _chunk;
    std::vector< std::string > _content_extraction_labels;
    t_extraction_func _content_extraction_func;
    std::mutex _mutex;
    static std::mutex _mutex_print;
};

#endif

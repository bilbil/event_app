#ifndef TESTER_H
#define TESTER_H

#include "EventApp.h"

#include <string>
#include <map>

class Tester {
public:
    bool TestProcessQueryArgs( EventApp * p_event_app, std::map<std::string, std::string> args, std::string & query ){
	return p_event_app->ProcessQueryArgs( args, query );
    }
    bool TestProcessExtract( EventApp * p_event_app, std::string input, std::string label, std::string & extracted ){
	return p_event_app->ProcessExtract( input, label, extracted );
    }
    bool TestProcessExtract( EventApp * p_event_app, std::string input, std::string label, int & extracted ){
	return p_event_app->ProcessExtract( input, label, extracted );
    }
    bool TestProcessEscapeCharacters( EventApp * p_event_app, std::string input, std::map<std::string,std::string> escape_chars, std::string & escaped ){
	return p_event_app->ProcessEscapeCharacters( input, escape_chars, escaped );
    }
};

#endif

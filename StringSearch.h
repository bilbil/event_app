#ifndef STRING_SEARCH_H
#define STRING_SEARCH_H

#include <functional>
#include <unordered_map>
#include <string>
#include <set>

//string encoding of a certain length
class CharacterEncode {
public:
    CharacterEncode(){
	_num_chars = 0;
    }
    CharacterEncode( int num_chars ){
	_num_chars = num_chars;
    }
    unsigned int _num_chars;
    std::unordered_map<unsigned int,std::set<std::string> > _encoding; //encode search string of certain length into a single number and further hash string using itself
};

class StringSearch {
public:
    bool Search( std::string input );
    bool RegisterStringToSearch( std::string search, std::function<bool(int index_found, int length)> func );
private:
    std::unordered_map< unsigned int, CharacterEncode  > _string_encode;
    std::unordered_map< std::string, std::function<bool(int index_found, int length)> > _string_func;
};

#endif

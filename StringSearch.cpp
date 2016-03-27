#include <string>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <iostream>

#include "StringSearch.h"

using namespace std;

bool StringSearch::Search( string input ){
    bool bFound = false;
    //partial summation of input string
    vector<unsigned int> partial_sum(input.length());
    vector<unsigned int> input_encoding(input.length());
    for( int h = 0; h < input.length(); ++h ){
	partial_sum[h] = input[h];
	if( h != 0 ){
	    partial_sum[h] += partial_sum[h-1];
	}
    }
    //compute encoding for string of same length as search string using partial sum
    for( auto & i : _string_encode ){
	unsigned int search_string_length = i.first;
	for( int k = 0; k < input.length() - search_string_length + 1; ++k ){
	    input_encoding[k] = partial_sum[k+search_string_length-1];
	    if( k > 0 ){
		input_encoding[k] -= partial_sum[k-1];
	    }
	    unsigned int encode = input_encoding[k];
	    //lookup for match
	    string strFound;
	    //if encoding match, try matching the exact string
	    auto find_encode = i.second._encoding.find( encode );
	    if( i.second._encoding.end() == find_encode ){
		continue; //not found, continue search
	    }
	    string string_to_check = input.substr(k, search_string_length );
	    auto find_search_string = find_encode->second.find( string_to_check );
	    if( find_encode->second.end() == find_search_string ){
		continue; //not found, continue search
	    }
	    strFound = *find_search_string;
	    bFound = true;
	    _string_func[ strFound ]( k, strFound.length() ); //found and activate function
	}
    }
    return true;
}

bool StringSearch::RegisterStringToSearch( string search, function<bool(int index_found, int length )> func ){
    _string_func[ search ] = func;
    int length_str = search.length();
    if( _string_encode.end() == _string_encode.find( length_str ) ){
	_string_encode[length_str] = CharacterEncode( length_str );
    }
    //compute encoding of the string
    unsigned int encode = 0;
    for( auto & i : search ){
	encode += i;
    }
    auto it = _string_encode.find(length_str);
    auto find_encoded = it->second._encoding.find( encode );
    if( it->second._encoding.end() == find_encoded ){
        it->second._encoding[ encode ] = std::set<string>();
    }
    it->second._encoding[ encode ].insert( search );
    return true;
}

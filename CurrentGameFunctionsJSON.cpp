#include <string>
#include <iostream>
#include <json/json.h>


using namespace std;

string data;

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up){
    for (size_t c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size*nmemb;
}


string standardise(string name){

    string temp = "";
    for(auto c : name){
        if(!isspace(c)) temp.push_back(tolower(c));
    }
    return temp;

}

string capitalised(string name){

    string temp = "";
    for(auto c : name){
        temp.push_back(toupper(c));
    }
    return temp;

}

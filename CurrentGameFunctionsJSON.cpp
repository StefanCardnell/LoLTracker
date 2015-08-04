#include <string>
#include "Functions.h"

using namespace std;

extern string data;

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up){
    for (size_t c = 0; c<size*nmemb; c++)
        data.push_back(buf[c]);

    return size*nmemb;
}


string& makeStandardised(string& name){

    for(auto it = name.begin(); it != name.end();)
        if(!isspace(*it)){
            *it = tolower(*it);
            ++it;
        }
        else it = name.erase(it);

    return name;

}

string returnCapitalised(const string& name){

    string temp;
    for(const auto& c : name){
        temp.push_back(toupper(c));
    }
    return temp;

}

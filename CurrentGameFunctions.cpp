#include <string>

using namespace std;

string data;

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up){
    for (size_t c = 0; c<size*nmemb; c++)
    {
        data.push_back(buf[c]);
    }
    return size*nmemb;
}

bool patternandrecord(string pattern, unsigned& position, char exitchar, string& recorder){
    auto length = data.size();
    bool findpattern = true;
    bool record = false;
    unsigned i = 0;

    while(position < length){

        if(!record && !findpattern) return true;

        if(record){ //record free week champ following "name"
            if(data[position] != exitchar)
                recorder.push_back(data[position]);
            else{
                record = false;
            }
        }

        if(findpattern){
            if(data[position] == pattern[i]) ++i;
            else i = 0;
            if(i == pattern.size()){
                record = true;
                findpattern = false;
                i = 0;
            }
        }
        ++position;
    }

    return false;
}

bool pattern(string pattern, unsigned& position){
    auto length = data.size();
    bool findpattern = true;
    unsigned i = 0;

    while(position < length){

        if(findpattern){
            if(data[position] == pattern[i]) ++i;
            else i = 0;
            if(i == pattern.size()){
                ++position;
                return true;
            }
        }
        ++position;
    }

    return false;

}

string removewhitespace(string name){

    string temp = "";
    for(auto c : name)
        if(!isspace(c)) temp.push_back(c);
    return temp;

}

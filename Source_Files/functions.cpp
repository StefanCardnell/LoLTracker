#include <string>
#include <thread>
#include <chrono>
#include <stdexcept>
#include "functions.h"

extern std::string curl_data;

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up){

    for (size_t c = 0; c<size*nmemb; c++)
        curl_data.push_back(buf[c]);

    return size*nmemb;

}

#ifdef __linux__
    #include <unistd.h>
    #include <limits.h> //PATH_MAX
    std::string ExePath() {

        char buffer[PATH_MAX];
        ssize_t len = ::readlink("/proc/self/exe", buffer, sizeof(buffer)-1);
        if(len == (sizeof(buffer)-1)) throw std::runtime_error("Path of executable is too long, try moving the executable to a shorter path location.");
        if(len == -1) throw std::runtime_error("readlink return of -1: Error occurred - unable to acquire module path.");
        buffer[len] = '\0';
        std::string::size_type pos = std::string(buffer).find_last_of( "\\/" );
        return std::string(buffer).substr(0, pos);

    }
#endif

#ifdef _WIN32
    #include <windows.h>
    std::string ExePath() {

        char buffer[MAX_PATH];
        ssize_t len = GetModuleFileName(NULL, buffer, MAX_PATH);
        if(len == MAX_PATH) throw std::runtime_error("Path of executable is too long, try moving the executable to a shorter path location.");
        if(len == 0) throw std::runtime_error("GetModuleFileName return of 0: Error occurred - unable to acquire module path.");
        std::string::size_type pos = std::string(buffer).find_last_of( "\\/" );
        return std::string(buffer).substr(0, pos);

    }
#endif

std::string& standardise(std::string& name){ //remove spaces and lower all letters

    for(auto it = name.begin(); it != name.end();)
        if(!isspace(*it)){
            *it = tolower(*it);
            ++it;
        }
        else it = name.erase(it);

    return name;

}

std::string& capitalise(std::string& word){

    for(auto& c : word)
        c = toupper(c);
    return word;

}

std::string capitalise_copy(const std::string& word){

    std::string ret = word; //RVO conditions met, copy elision may occur.
    capitalise(ret); //passes to capitalise taking lvalue reference. NOTE: We do not return this directly otherwise we would remove RVO and copy elision.
    return ret;

}

void sleepMilli(unsigned t){

    std::this_thread::sleep_for(std::chrono::milliseconds(t));

}

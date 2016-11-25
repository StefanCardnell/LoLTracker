#ifndef FUNCTIONS
#define FUNCTIONS

#include <string>

size_t writeCallback(char*, size_t, size_t, void*);
std::string ExePath();
std::string& standardise(std::string&);
std::string& capitalise(std::string&);
std::string capitalise_copy(const std::string&);
void sleepMilli(unsigned);

#endif // FUNCTIONS

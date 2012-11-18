#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <sys/timeb.h>

using namespace std;

namespace http {
namespace server3 {

class logger {
private:
    static ofstream out;
public:
    logger();
    ~logger();
    static void log(const char *str);
    static void open(const char *str);
    static void close();
};

}
}

#endif

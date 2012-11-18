#include "logger.hpp"

namespace http {
namespace server3 {

ofstream logger::out;

logger::logger() 
{
}

logger::~logger() 
{
}

void logger::log(const char *str) 
{
    struct timeb now;
    ftime(&now);
    struct tm *ntm = localtime(&now.time);
    out << ntm->tm_year + 1900 << "-" << ntm->tm_mon + 1 << "-" << ntm->tm_mday << " " 
	<< ntm->tm_hour << ":" << ntm->tm_min << ":" << ntm->tm_sec 
	<< "." << now.millitm << " " << str << endl;

}

void logger::open(const char *str)
{
    out.open(str);
    if(!out) {
	cerr << "Failed to open file: '" << str << "'\n";
	exit(1);
    }
}

void logger::close()
{
    out.close();
}

}
}



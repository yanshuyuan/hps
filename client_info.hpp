#ifndef CLIENT_INFO_HPP
#define CLIENT_INFO_HPP

#include <string>
#include <sys/timeb.h>

using namespace std;

namespace http {
namespace server3 {
struct client_info {
    string address;
    unsigned short port;
    string filename;
    size_t filesize;
    int ecode;
    struct timeb stime, etime;
};

}
}

#endif

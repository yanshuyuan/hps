#include "request.hpp"

namespace http {
namespace server3 {

request::request()
{
}

request::~request()
{
}

void request::reset()
{
    method.clear();
    uri.clear();
    headers.clear();
}

}
}

#ifndef FILE_CACHE_HPP
#define FILE_CACHE_HPP

#include <map>
#include <string>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

using namespace std;

namespace http {
namespace server3 {

typedef boost::shared_mutex rwmutex; 
typedef boost::shared_lock<rwmutex> read_lock; 
typedef boost::unique_lock<rwmutex> write_lock; 


class file_item {
public:
    boost::mutex c_mutex;
    long long count;
    rwmutex rw_mutex; 
    string filename;
    string content;
public:
    file_item()
	: count(0) {};
    ~file_item(){};
    file_item(const file_item &item) {
	this->count = item.count;
    }
    file_item& operator = (const file_item &item) {
	this->count = item.count;
	return *this;
    };
};

class file_cache {
private:
    vector<file_item *> index; 
    int limit_size;
public:
    file_cache(size_t size);
    ~file_cache();
    vector<file_item *>::iterator find(string& filename);
    string read(string& filename);
    void replace(string& filename, string& content);
};

}
}

#endif

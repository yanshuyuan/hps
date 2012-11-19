#include "file_cache.hpp"
#include <iostream>

using namespace std;

namespace http {
namespace server3 {

file_cache::file_cache(size_t size)
{
    limit_size = 0;
    for(size_t ix = 0; ix < size; ix++) {
        index.push_back(new file_item());
    }
}


file_cache::~file_cache()
{
    for(vector<file_item *>::iterator iter = index.begin();
	iter != index.end(); iter++) {
	delete *iter;
    }
    index.clear();
}

void file_cache::replace(string &filename, string &content)
{
    /*  
	find the min count iterator
	but the min count is not exactly, 
	because in multiple thread, 
	maybe the thread read the count,
	another thread change the count
    */
    long long min = (*index.begin())->count;
    vector<file_item *>::iterator min_iter = index.begin();

    cout << "==============replace cache===========================" << endl;
    for(vector<file_item *>::iterator iter = index.begin();
    	iter != index.end(); iter++) {
	if((*iter)->filename.size() > 0) {cout << (*iter)->filename << " :" << (*iter)->count << endl; }
        if((*iter)->count < min) {
    	    min = (*iter)->count;
    	    min_iter = iter; 
        }
    }
    cout << endl;
    /* 
	try the iterator is writable?
	if other thread read it, we not change the filename and content, 
	we just decrease the count
    */
    write_lock wlock((*min_iter)->rw_mutex, boost::defer_lock); 
    if(wlock.try_lock()) {
        (*min_iter)->filename = filename;
        (*min_iter)->content = content;
	(*min_iter)->count = 1;
	wlock.unlock();
    } else {
	(*min_iter)->c_mutex.lock();
	(*min_iter)->count--;
	(*min_iter)->c_mutex.unlock();
    }
}

vector<file_item *>::iterator file_cache::find(string& filename)
{
    cout << "==============find cache===========================" << endl;
    for(vector<file_item *>::iterator iter = index.begin();
        iter != index.end(); iter++) {
	if((*iter)->filename.size() > 0) {cout << (*iter)->filename << " :" << (*iter)->count << endl; }
	if((*iter)->filename == filename) {
	    return iter;
	}
    }
    cout << endl;
    return index.end();
}

string file_cache::read(string& filename)
{
    vector<file_item *>::iterator iter = find(filename); 
    if(iter != index.end()) {
        read_lock rdlock((*iter)->rw_mutex); 
        (*iter)->c_mutex.lock();
        (*iter)->count++;
        (*iter)->c_mutex.unlock();
        return (*iter)->content;
    } else {
	return string();
    }
} 

}
}


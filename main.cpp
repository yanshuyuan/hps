//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "logger.hpp"
#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "server.hpp"
#include "file_cache.hpp"

http::server3::file_cache *cache;

int main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 6)
    {
      std::cerr << "Usage: http_server <address> <port> <threads> <doc_root> <log_fullname>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 80 1 .\n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 80 1 .\n";
      return 1;
    }

    http::server3::logger::open(argv[5]);
    // Initialise the server.
    std::size_t num_threads = boost::lexical_cast<std::size_t>(argv[3]);
    http::server3::file_cache memcache(num_threads * 2);
    cache = &memcache;

    http::server3::server s(argv[1], argv[2], argv[4], num_threads);

    // Run the server until stopped.
    s.run();
    http::server3::logger::close();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}

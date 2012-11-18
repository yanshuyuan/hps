//
// connection.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2011 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include "request_handler.hpp"

namespace http {
namespace server3 {

connection::connection(boost::asio::io_service& io_service,
    request_handler& handler)
  : strand_(io_service),
    socket_(io_service),
    request_handler_(handler)
{
}

boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::start()
{
  cinfo.address = socket_.remote_endpoint().address().to_string();
  cinfo.port = socket_.remote_endpoint().port();
  socket_.async_read_some(boost::asio::buffer(buffer_),
      strand_.wrap(
        boost::bind(&connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));
}

void connection::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
  if (!e)
  {
    boost::tribool result;
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);

    if (result)
    {
      ftime(&cinfo.stime);
      cinfo.ecode = request_handler_.handle_request(request_, reply_);
      cinfo.filename = request_.uri;
      cinfo.filesize = reply_.content.size();
      boost::asio::async_write(socket_, reply_.to_buffers(),
          strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
              boost::asio::placeholders::error)));
      std::stringstream sstream;
      sstream << "Client: [" << cinfo.address << ":" << cinfo.port 
	      << " request file " << cinfo.filename << ".";
      logger::log(sstream.str().c_str());
    }
    else if (!result)
    {
      reply_ = reply::stock_reply(reply::bad_request);
      boost::asio::async_write(socket_, reply_.to_buffers(),
          strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
              boost::asio::placeholders::error)));
    }
    else
    {
      socket_.async_read_some(boost::asio::buffer(buffer_),
          strand_.wrap(
            boost::bind(&connection::handle_read, shared_from_this(),
              boost::asio::placeholders::error,
              boost::asio::placeholders::bytes_transferred)));
    }
  }

  // If an error occurs then no new asynchronous operations are started. This
  // means that all shared_ptr references to the connection object will
  // disappear and the object will be destroyed automatically after this
  // handler returns. The connection class's destructor closes the socket.
}

void connection::handle_write(const boost::system::error_code& e)
{
  std::stringstream sstream;
  if (!e && cinfo.ecode == reply::ok)
  {
    sstream << "Successful send file " << cinfo.filename << " " << cinfo.filesize
	    << " bytes to client ["  << cinfo.address << ":" << cinfo.port 
	    << "] after " << (cinfo.etime.time - cinfo.stime.time) * 1000 + 
	    (cinfo.etime.millitm - cinfo.stime.millitm) << " ms from receiving its request.";
    logger::log(sstream.str().c_str());

    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  } else {
    std::string error = "'send failed'";
    if(!e) {
        switch(cinfo.ecode) {
        case reply::not_found:
            error = "'not found'";
            break;
        case reply::bad_request:
            error = "'bad request'";
            break;
        default:
            error = "'unknown'";
            break;
        }
    }
    sstream << "Fail to send file " << cinfo.filename << " to client [" << cinfo.address << ":"
	    << cinfo.port << "] due to error " << error << ".";
    logger::log(sstream.str().c_str());
  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

} // namespace server3
} // namespace http

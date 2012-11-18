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
    request_handler_(handler),
    timer_(io_service, boost::posix_time::seconds(60))
{
}

boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::start()
{
  
  cinfo_.address = socket_.remote_endpoint().address().to_string();
  cinfo_.port = socket_.remote_endpoint().port();

  std::stringstream sstream;
  sstream << "Client: [" << cinfo_.address << ":" << cinfo_.port 
          << "] open connection.";
  logger::log(sstream.str().c_str());

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
	/*
    logger::log((string() + "receive data: " + buffer_.data()).c_str());
	*/
    request_.reset();
    boost::tie(result, boost::tuples::ignore) = request_parser_.parse(
        request_, buffer_.data(), buffer_.data() + bytes_transferred);

    if (result)
    {
      ftime(&cinfo_.stime);
      cinfo_.ecode = request_handler_.handle_request(request_, reply_);
      cinfo_.filename = request_.uri;
      cinfo_.filesize = reply_.content.size();
      boost::asio::async_write(socket_, reply_.to_buffers(),
          strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
              boost::asio::placeholders::error)));
      std::stringstream sstream;
      sstream << "Client: [" << cinfo_.address << ":" << cinfo_.port 
	      << "] request file '" << cinfo_.filename << "'.";
      logger::log(sstream.str().c_str());
    }
    else if (!result)
    {
      reply_ = reply::stock_reply(reply::bad_request);
      cinfo_.ecode = reply::bad_request;
      boost::asio::async_write(socket_, reply_.to_buffers(),
          strand_.wrap(
            boost::bind(&connection::handle_write, shared_from_this(),
              boost::asio::placeholders::error)));
      std::stringstream sstream;
      sstream << "Client: [" << cinfo_.address << ":" << cinfo_.port 
	      << "] bad request.";
      logger::log(sstream.str().c_str());

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

void connection::handle_timeout(const boost::system::error_code& e)
{
  if (!e)
  {
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    stringstream sstream;
    sstream << "Client [" << cinfo_.address << ":" << cinfo_.port << "] connection closed.";
    logger::log(sstream.str().c_str());
  } 
}

void connection::handle_write(const boost::system::error_code& e)
{
  std::stringstream sstream;
  if (!e && cinfo_.ecode == reply::ok)
  {
    ftime(&cinfo_.etime);
    sstream << "Successful send file '" << cinfo_.filename << "' " << cinfo_.filesize
	    << " bytes to client ["  << cinfo_.address << ":" << cinfo_.port 
	    << "] after " << (cinfo_.etime.time - cinfo_.stime.time) * 1000 + 
	    (cinfo_.etime.millitm - cinfo_.stime.millitm) << " ms from receiving its request.";
    logger::log(sstream.str().c_str());

    socket_.async_read_some(boost::asio::buffer(buffer_),
      strand_.wrap(
        boost::bind(&connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));

    timer_.expires_from_now(boost::posix_time::seconds(60));
    timer_.async_wait(boost::bind(&connection::handle_timeout, shared_from_this(),
              boost::asio::placeholders::error));
    /*

    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
    */
  } else if(!e) {
    std::string error;
    switch(cinfo_.ecode) {
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
    sstream << "Fail to send file '" << cinfo_.filename << "' to client [" << cinfo_.address << ":"
	    << cinfo_.port << "] due to error " << error << ".";
    logger::log(sstream.str().c_str());

    socket_.async_read_some(boost::asio::buffer(buffer_),
      strand_.wrap(
        boost::bind(&connection::handle_read, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred)));

    timer_.expires_from_now(boost::posix_time::seconds(60));
    timer_.async_wait(boost::bind(&connection::handle_timeout, shared_from_this(),
              boost::asio::placeholders::error));
  } else {
    std::string error = "'send failed'";
    sstream << "Fail to send file '" << cinfo_.filename << "' to client [" << cinfo_.address << ":"
	    << cinfo_.port << "] due to error " << error << ".";
    logger::log(sstream.str().c_str());

  }

  // No new asynchronous operations are started. This means that all shared_ptr
  // references to the connection object will disappear and the object will be
  // destroyed automatically after this handler returns. The connection class's
  // destructor closes the socket.
}

} // namespace server3
} // namespace http

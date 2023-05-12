#ifndef CNR_IPC_UTILITIES_INCLUDE_ASIO_UTILITIES
#define CNR_IPC_UTILITIES_INCLUDE_ASIO_UTILITIES

#include <boost/asio.hpp>
#include <iostream>

namespace cnr
{
namespace ipc
{

constexpr int max_length = 1024;

using Executor = std::function<void(const std::string& req, std::string& res)>;

class AsioSession : public std::enable_shared_from_this<AsioSession>
{
 public:
  AsioSession(boost::asio::ip::tcp::socket socket, Executor executor);
  void start();

  Executor get_executor();

 private:
  Executor executor_;
  void do_read();
  void do_write(std::size_t lenght);

  boost::asio::ip::tcp::socket socket_;

  char income_data_[max_length];
  char outcome_data_[max_length];
};

class AsioServer
{
 public:
  using Ptr = std::shared_ptr<AsioServer>;
  using ConstPtr = std::shared_ptr<const AsioServer>;

  AsioServer(boost::asio::io_context& io_context, boost::asio::ip::tcp::endpoint ep, Executor executor);

 private:
  void do_accept();

  Executor executor_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

using AsioServerPtr = AsioServer::Ptr;
using AsioServerConstPtr = AsioServer::ConstPtr;

AsioServerPtr make_shared(boost::asio::io_context& io_context, const std::string& host, short port, Executor executor);

class AsioClient
{
 public:
  void connect(const std::string& host, const std::string& service, std::chrono::steady_clock::duration timeout);

  std::string read_response(std::chrono::steady_clock::duration timeout);

  std::error_code write_request(const std::string& line, std::chrono::steady_clock::duration timeout);

 private:
  void run(std::chrono::steady_clock::duration timeout);

  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket socket_{io_context_};
  std::string input_buffer_;
};

}  // namespace ipc
}  // namespace cnr

#endif  // CNR_IPC_UTILITIES_INCLUDE_ASIO_UTILITIES

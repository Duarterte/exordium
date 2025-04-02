#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>
#include "../includes/exordium.hpp"
#include <string>

boost::asio::awaitable<void> server_handler(boost::asio::ip::udp::socket& socket, boost::asio::ip::udp::endpoint endpoint)
{
  char data[128];
  co_await socket.async_receive_from(boost::asio::buffer(data), endpoint,  boost::asio::use_awaitable);
  std::string server_data;

  int CMD = std::atoi(data);
  switch (static_cast<size_t>(CMD))
  {
    case KEYS(PING):
      server_data = exordium::command[KEYS(PING)]();
      break;
    case KEYS(SIZE):
      server_data = exordium::command[KEYS(SIZE)]();
      break;
    default:
      server_data = "Invalid Command\n";
      break;
  }
  co_await socket.async_send_to(boost::asio::buffer(server_data), endpoint,  boost::asio::use_awaitable);
}

boost::asio::awaitable<void> echo(boost::asio::ip::udp::socket socket)
{
  try
  {
    for (;;)
    {
      boost::asio::ip::udp::endpoint endpoint;
      // Pass the endpoint to server_handler so it knows where to send the response
      co_await server_handler(socket, endpoint);
    }
  }
  catch (std::exception& e)
  {
    std::printf("echo Exception: %s\n", e.what());
  }
}

boost::asio::awaitable<void> listener()
{
  auto executor = co_await boost::asio::this_coro::executor;
  boost::asio::ip::udp::socket socket(executor, {boost::asio::ip::udp::v4(), 55555});
  co_await echo(std::move(socket)); // Pass the socket to the echo coroutine
}

int main()
{
  try
  {
    boost::asio::io_context io_context(1);

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&](auto, auto){ io_context.stop(); });

    boost::asio::co_spawn(io_context, listener(), boost::asio::detached);

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::printf("Exception: %s\n", e.what());
  }
}
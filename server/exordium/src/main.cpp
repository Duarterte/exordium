#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <cstdio>
#include "../includes/exordium.hpp"
#include <string>

boost::asio::awaitable<void> server_handler(boost::asio::ip::udp::socket& socket)
{
    char data[128];
    while (true) {
        boost::asio::ip::udp::endpoint sender_endpoint;
        try {
            co_await socket.async_receive_from(boost::asio::buffer(data), sender_endpoint, boost::asio::use_awaitable);
            int CMD = std::atoi(data);
            boost::asio::co_spawn(socket.get_executor(), [&, sender_endpoint, CMD]() mutable -> boost::asio::awaitable<void> {
                std::string server_data;
                std::string error_message;
                bool error_occurred = false;
                try {
                    switch (static_cast<size_t>(CMD))
                    {
                        case KEYS(PING):
                            server_data = co_await exordium::command[KEYS(PING)]();
                            break;
                        case KEYS(SIZE):
                            server_data = co_await exordium::command[KEYS(SIZE)]();
                            break;
                        default:
                            server_data = "Invalid Command\n";
                            break;
                    }
                    std::printf("About to send: %s to %s:%d\n", server_data.c_str(), sender_endpoint.address().to_string().c_str(), sender_endpoint.port());
                    co_await socket.async_send_to(boost::asio::buffer(server_data), sender_endpoint, boost::asio::use_awaitable);
                    std::printf("Response sent.\n");
                } catch (const std::exception& e) {
                    std::fprintf(stderr, "Error processing command: %s\n", e.what());
                    error_message = "Error processing command\n";
                    error_occurred = true;
                }

                if (error_occurred) {
                    try {
                        co_await socket.async_send_to(boost::asio::buffer(error_message), sender_endpoint, boost::asio::use_awaitable);
                    } catch (const std::exception& send_error) {
                        std::fprintf(stderr, "Error sending error response: %s\n", send_error.what());
                    }
                }
            }, boost::asio::detached);
        } catch (const boost::system::system_error& error) {
            std::fprintf(stderr, "Error receiving data: %s\n", error.what());
        }
    }
}

boost::asio::awaitable<void> echo(boost::asio::ip::udp::socket socket)
{
  try
  {
    for (;;)
    {

      // Pass the endpoint to server_handler so it knows where to send the response
      co_await server_handler(socket);
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
  boost::asio::ip::udp::socket socket(executor, {boost::asio::ip::udp::v4(), 49382});
  std::printf("UDP server listening on port 49382\n");
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
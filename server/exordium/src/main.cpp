#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <thread>
#include <string>
#include <cstdio>
#include "../includes/exordium.hpp"



boost::asio::awaitable<void> server_handler(boost::asio::ip::udp::socket& socket)
{ // Assuming BUFFER_SIZE is defined elsewhere
    constexpr int HEADER_SIZE = 32;
    constexpr int BUFFER_SIZE = 1024;
    char data[BUFFER_SIZE];
    bool running = true;
    bool overflow_flag = false;
    int CMD = 0;
    std::string big_data = "";
    std::string header = "";
    while (running) {
        boost::asio::ip::udp::endpoint sender_endpoint;
        try {
            size_t msg_length = co_await socket.async_receive_from(boost::asio::buffer(data), sender_endpoint, boost::asio::use_awaitable);
            if (data[msg_length - 1] != '\n') {
              big_data += data;
              if (overflow_flag) {
                header = big_data.substr(HEADER_SIZE, msg_length - HEADER_SIZE);
                big_data = big_data.substr(msg_length - HEADER_SIZE);
              }
              overflow_flag = true;
              continue;
            }
            if (overflow_flag) {
              big_data += data;
              overflow_flag = false;
              CMD = std::atoi(header.c_str());
              header.clear();
            } else {
              CMD = std::atoi(data);
            }

            boost::asio::co_spawn(socket.get_executor(), [&, sender_endpoint, CMD]() mutable -> boost::asio::awaitable<void> {
                std::string server_data;
                std::string error_message;
                bool error_occurred = false;
                try {
                    switch (static_cast<size_t>(CMD))
                    {
                        case KEYS(PING):big_data.clear();
                            server_data = co_await exordium::command[KEYS(PING)]();
                            break;
                        case KEYS(SIZE):
                            server_data = co_await exordium::command[KEYS(SIZE)]();
                            break;
                        case KEYS(CHECK_CONNECTION):
                            server_data = co_await exordium::command[KEYS(CHECK_CONNECTION)]();
                            break;
                        default:
                            server_data = std::format("Unknown command of size: {}\n", msg_length);
                            break;
                    }
                    big_data.clear();
                    std::printf("About to send response: %s to %s:%d\n", server_data.c_str(), sender_endpoint.address().to_string().c_str(), sender_endpoint.port());
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
      size_t thread_count = std::thread::hardware_concurrency();
      std::printf("Thread count: %ld\n", thread_count);

      boost::asio::io_context io_context;
        
      boost::asio::io_context::executor_type main_executor = io_context.get_executor();
      
      exordium::db_store = std::make_unique<Store>(main_executor, thread_count);
      
      

      boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
      signals.async_wait([&](auto, auto){ io_context.stop(); });
      
      boost::asio::co_spawn(io_context, listener(), boost::asio::detached);
      
      

      boost::asio::thread_pool pool(thread_count);

      // Post work to the thread pool
      for (size_t i = 0; i < thread_count; ++i)
      {
          boost::asio::post(pool, [&io_context]{ io_context.run(); });
      }

      // Wait for all threads in the pool to complete
      pool.join();
  }
  catch (std::exception& e)
  {
      std::printf("Exception: %s\n", e.what());
  }

  return 0;
}

#ifndef STORE_HPP
#define STORE_HPP


#include <boost/mysql/handshake_params.hpp> // Needed for connection parameters
#include <boost/asio/io_context.hpp>      // The core Asio context
#include <boost/asio/ip/tcp.hpp>          // Needed for resolver and endpoint types
#include <boost/asio/awaitable.hpp>       // For awaitable<void>
#include <boost/mysql/connection.hpp>  
#include <boost/asio/executor.hpp>
#include <boost/asio/execution_context.hpp>  
#include <boost/asio/strand.hpp> // Needed for connection
#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/chrono.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/detached.hpp>
#include <boost/array.hpp>
#include <boost/container/deque.hpp>

#include <string> // For using std::string




struct Connection {
    private:
    bool connected = false;
    const char* host = "192.168.0.5";
    const char* port = "3306";
    const char* user = "root";
    const char* password = "joda";
    const char* database = "exordium";

    boost::mysql::handshake_params handshake_params;
    std::unique_ptr<boost::mysql::connection<boost::asio::ip::tcp::socket>> connection;
    

    public:
    Connection(boost::asio::any_io_executor ex) :
        handshake_params(user, password, database),
        connection(std::make_unique<boost::mysql::connection<boost::asio::ip::tcp::socket>>(ex))
        {}
    
    
    friend class Store;      
};


struct Store
{
    // Static connection details - using const char* is safer with constexpr
    // than std::string in older C++ standards typically used with these Boost versions
    private:
        size_t max_connections;
        size_t current;
        size_t count_con;
        boost::container::deque<std::unique_ptr<Connection>> db_pool;  
    public:

    boost::asio::any_io_executor ex;
    boost::asio::strand<boost::asio::any_io_executor> strand;
    
    Store(boost::asio::any_io_executor ex, size_t max_connections) :
        max_connections(max_connections),
        current(0),
        count_con(0),
        //handshake_params(user, password, database),
        ex(ex),
        strand(boost::asio::make_strand(ex))
    {
        db_pool.resize(max_connections);
        for (size_t i = 0; i < max_connections; ++i) {
            db_pool[i] = std::make_unique<Connection>(ex);
        }
        boost::asio::co_spawn(strand, test_query(), boost::asio::detached);
    }
    ~Store() {}



    boost::asio::awaitable<void> get_connection() {
        boost::asio::post(strand, [this] {
            current = (current + 1) % max_connections;       
        });
        co_await boost::asio::post(boost::asio::bind_executor(strand, boost::asio::use_awaitable));
    }

    boost::asio::awaitable<std::string> test_query();
    
};



#endif // STORE_HPP
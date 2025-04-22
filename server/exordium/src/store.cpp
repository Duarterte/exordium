#include "../includes/store.hpp"

boost::asio::awaitable<std::string> Store::test_query()
{
    co_await get_connection();
    std::string query_response = "";
    printf("Current connection: %ld\n", current);
    if (!db_pool[current]->connected)
    {
        boost::asio::ip::tcp::resolver resolver(db_pool[current]->connection->get_executor());
        auto endpoints = co_await resolver.async_resolve(db_pool[current]->host, db_pool[current]->port, boost::asio::use_awaitable);
        co_await db_pool[current]->connection->async_connect(*endpoints.begin(), db_pool[current]->handshake_params, boost::asio::use_awaitable);
        db_pool[current]->connected = true;
    }
    boost::mysql::results query_results;
    co_await db_pool[current]->connection->async_execute("SELECT 'Connected SUCCESSFULLY!\n'", query_results, boost::asio::use_awaitable);
    if (!query_results.empty())
    {
        boost::mysql::resultset_view res_set = query_results.at(0);
        if (!res_set.rows().empty())
        {
            boost::mysql::row_view r_view = res_set.rows().at(0);
            boost::mysql::string_view sv = r_view.at(0).as_string();
            query_response = std::string(sv);
        }
    }
    co_return query_response;
}

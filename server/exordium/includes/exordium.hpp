#pragma once
#ifndef EXORDIUM_H
#define EXORDIUM_H
#define KEYS(member) to_index(exordium::Key::member)


#include <array>
#include <functional>
#include <string>
#include <boost/asio/awaitable.hpp>
#include "store.hpp"



namespace exordium {

  std::unique_ptr<Store> db_store;

  enum class Key
    {
      NONE,
      PING,
      SIZE,
      CHECK_CONNECTION,
      CUSTOM_MSG,
      COUNT // Add a COUNT to represent the number of commands
    };

  constexpr std::size_t to_index(Key key) {
    return static_cast<std::size_t>(key);
  }

  static std::array<std::function<boost::asio::awaitable<std::string>(std::string_view)>, KEYS(COUNT)> command;

  static boost::asio::awaitable<std::string> ping() {
    co_return "pong\n";
  }

  static boost::asio::awaitable<std::string> size_func() {
    co_return "some size info\n"; // Replace with actual size logic
  }

  static boost::asio::awaitable<std::string> custom_msg(std::string_view msg) {
    std::string message = "";
    if(msg.size() > 0) {
      message = std::string(msg)+"\n";
    }
    else {
      message = "No custom message provided\n";
    }
    co_return message; // Replace with actual custom message logic
  }

  static boost::asio::awaitable<std::string> check_connection() {
    std:: string response = co_await db_store->test_query();
    co_return response;
  }

  // Example usage of custom_msg
  
  // Initialize the commands array after the functions are defined
  struct CommandInitializer {
    CommandInitializer() {
      command[KEYS(PING)] = [](std::string_view) { return ping(); };
      command[KEYS(SIZE)] = [](std::string_view) { return size_func(); };
      command[KEYS(CHECK_CONNECTION)] = [](std::string_view) { return check_connection(); };
      command[KEYS(CUSTOM_MSG)] = [](std::string_view msg) { return custom_msg(msg);
      };
    }
  };

  static CommandInitializer initializer;
}

#endif
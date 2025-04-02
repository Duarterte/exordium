#pragma once
#ifndef EXORDIUM_H
#define EXORDIUM_H
#define KEYS(member) to_index(exordium::Key::member)

#include <array>
#include <functional>
#include <string>

namespace exordium {
  enum class Key
    {
      NONE,
      PING,
      SIZE,
      COUNT // Add a COUNT to represent the number of commands
    };
  
  constexpr std::size_t to_index(Key key) {
    return static_cast<std::size_t>(key);
  }

  static std::array<std::function<std::string(void)>, KEYS(COUNT)> command;

  static inline std::string ping(){
    return "pong\n";
  }

  static inline std::string size_func(){
    return "some size info\n"; // Replace with actual size logic
  }


  // Initialize the commands array after the functions are defined
  struct CommandInitializer {
    CommandInitializer() {
      command[KEYS(PING)] = ping;
      command[KEYS(SIZE)] = size_func;
    }
  };

  // Create a static instance of the initializer to ensure it runs once
  static CommandInitializer initializer;

 
}

#endif
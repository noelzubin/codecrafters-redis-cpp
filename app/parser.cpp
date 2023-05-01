#include "parser.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

// Simple parser for paring redis requests.
// Sample request: *1\r\n$4\r\nECHO\r\n$5\r\nhello\r\n
struct Parser {
  char *input;  // input string
  char *cursor; // current parsed position in input string

  Parser(char *inp) : input(inp), cursor(inp) {}

  // Parse redis request and return a vector of strings.
  std::vector<std::string> parse() {
    auto results = std::vector<std::string>();
    cursor += 1; // move past *
    auto wordCount = stoi(until_char('\r'));

    while (wordCount--) {
      cursor += 3; // move past \r\n$
      auto wordLength = stoi(until_char('\r'));
      cursor += 2; // move past \r\n
      auto word = take(wordLength);
      results.push_back(word);
    }

    return results;
  }

  // take n elements from the input and return the string
  std::string take(int n) {
    auto result = std::string();
    while (n--) {
      result += *cursor;
      cursor += 1;
    }
    return result;
  }

  // take until you find the given char.
  std::string until_char(char c) {
    auto result = std::string();
    while (*cursor != c) {
      result += *cursor;
      cursor += 1;
    }
    return result;
  }
};

Command parseCommand(char *command) {
  auto words = Parser(command).parse();

  std::string cmd = words[0];
  std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
  if (cmd == "ECHO") {
    return Command{CommandType::ECHO, EchoString{words[1]}};
  } else if (cmd == "PING") {
    return Command{.type = CommandType::PING};
  } else if (cmd == "SET") {
    auto data = SetParams{.key = words[1], .value = words[2]};

    std::transform(words[3].begin(), words[3].end(), words[3].begin(),
                   ::toupper);
    // check if expiry is set in milliseconds
    if (words[3] == "PX")
      data.expire_ms = std::stoi(words[4].c_str());

    return Command{CommandType::SET, data};
  } else if (cmd == "GET") {
    return Command{CommandType::GET, GetKey{words[1]}};
  } else {
    return Command{CommandType::UNKNOWN};
  }
}

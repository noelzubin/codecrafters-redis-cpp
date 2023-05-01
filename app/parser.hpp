#pragma once 

#include <string>
#include <tuple>
#include <variant>

// List of supported commands.
// If unsupported command is received, UNKNOWN is returned.
enum class CommandType {
    PING,
    ECHO,
    SET,
    GET,
    UNKNOWN
};

struct EchoString  { std::string message; };

// Params for SET command. 0 expire_ms means no expiry.
struct SetParams { std::string key; std::string value; int expire_ms; }; 

struct GetKey { std::string key; };


struct Command {
    CommandType type;
    std::variant<EchoString , SetParams, GetKey> data; 
};

// parse the redis request and return the response.
Command parseCommand(char* command);
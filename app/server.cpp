#include "parser.hpp"
#include "unordered_map"
#include <errno.h>
#include <future>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <optional>

struct Entry
{
  // The actual string value for the key 
  std::string value;
  // Optionally set expiry time.
  std::optional<std::chrono::system_clock::time_point> expire_at;
};

std::unordered_map<std::string, Entry> store;

void write_message(int client_fd, const char *message)
{
  ssize_t bytes_written = write(client_fd, message, strlen(message));
  if (bytes_written == -1)
  {
    std::cerr << "error failed to write to socket";
    return;
  }
  else
  {
    std::cout << "wrote to socket " << bytes_written
              << "bytes written: " << message << std::endl;
  }
}

void handle_set(int client_fd, Command *cmd)
{
  auto params = std::get<SetParams>(cmd->data);
  std::optional<std::chrono::system_clock::time_point> expire_at = std::nullopt;

  //  Check if there is expiry to be set.
  if (params.expire_ms)
  {
    expire_at = std::chrono::system_clock::now() + std::chrono::milliseconds(params.expire_ms);
  }

  auto entry = Entry{.value = params.value, .expire_at = expire_at};
  store[params.key] = entry;
  write_message(client_fd, "+OK\r\n");
}

void handle_get(int client_fd, Command *cmd)
{
  auto keyReq = std::get<GetKey>(cmd->data);
  auto entry = store.find(keyReq.key);

  bool found = true;

  // if key is not found or is expired then skip it 
  if (entry == store.end())
    found = false;
  if (found && (entry->second.expire_at.has_value()) && entry->second.expire_at < std::chrono::system_clock::now()) {
    // passively delete expired keys
    store.erase(keyReq.key);
    found = false;
  }

  if (!found)
  {
    // return null string 
    write_message(client_fd, "$-1\r\n");
    return;
  }

  std::string resp = store[keyReq.key].value;
  resp = "$" + std::to_string(resp.length()) + "\r\n" + resp + "\r\n";
  write_message(client_fd, resp.c_str());
}

void handle_ping(int client_fd, Command *cmd)
{
  const char *message = "+PONG\r\n";
  write_message(client_fd, message);
}

void handle_unknown(int client_fd, Command *cmd)
{
  const char *message = "+UKNOWN COMMAND\r\n";
  write_message(client_fd, message);
}

void handle_echo(int client_fd, Command *cmd)
{
  std::string s = "+" + std::get<EchoString>(cmd->data).message + "\r\n";
  const char *message = s.c_str();
  write_message(client_fd, message);
}

// Handle each connection
void handle_connection(int client_fd)
{
  ssize_t bytes_read;
  char buffer[1024];

  // listen to multiple commands from the same connection
  while ((bytes_read = read(client_fd, buffer, 1024)))
  {

    buffer[bytes_read] = '\0'; // NULL terminate the string
    printf("Received: %s\n", buffer);

    std::unordered_map<CommandType, std::function<void(int, Command *)>>
        handlers{
            {CommandType::PING, handle_ping},
            {CommandType::ECHO, handle_echo},
            {CommandType::SET, handle_set},
            {CommandType::GET, handle_get},
            {CommandType::UNKNOWN, handle_unknown},
        };

    Command cmd = parseCommand(buffer);
    handlers[cmd.type](client_fd, &cmd);
  }

  close(client_fd);
}

int main()
{
  // Disable output buffering
  setbuf(stdout, NULL);

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  printf("Logs from your program will appear here!\n");

  int server_fd, client_addr_len;

  // socket creates a channel for communication.
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1)
  {
    printf("Socket creation failed: %s...\n", strerror(errno));
    return 1;
  }

  // Since the tester restarts your program quite often, setting REUSE_PORT
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
      0)
  {
    printf("SO_REUSEPORT0x00007ff7bfefe7f0 failed: %s \n", strerror(errno));
    return 1;
  }

  // Bind the socket to local address and port
  struct sockaddr_in serv_addr = {
      .sin_family = AF_INET,
      .sin_port = htons(6379),
      .sin_addr = {htonl(INADDR_ANY)},
  };

  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
  {
    std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    std::cerr << "Listen failed: " << strerror(errno) << std::endl;
    return 1;
  }

  // using futures to handle connection so that it works like a thread pool
  const int thread_count = std::thread::hardware_concurrency();
  std::vector<std::future<void>> futures;

  while (true)
  {
    std::cout << "Waiting for a client to connect..." << std::endl;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    // accepts gets the first pending request from the queue and returns a file
    // descriptor for the same.
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                           (socklen_t *)&client_addr_len);

    if (client_fd == -1)
    {
      std::cerr << "error accepting connection";
      continue;
    }

    futures.emplace_back(std::async(handle_connection, client_fd));
  }

  for (auto &f : futures)
  {
    f.get();
  }

  read(server_fd, NULL, 0);

  close(server_fd);

  return 0;
}

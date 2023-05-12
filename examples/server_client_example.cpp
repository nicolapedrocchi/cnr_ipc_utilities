
#include <unistd.h>
#include "my_server_client.h"


int main(int, char*[])
{
  pid_t c_pid = fork();
  std::shared_ptr<MyClient> get_client;
  std::shared_ptr<MyClient> set_client;

  constexpr unsigned short set_port = 1024;
  constexpr unsigned short get_port = 1025;

  if (c_pid == -1)
  {
    perror("fork");
    exit(EXIT_FAILURE);
  }
  else if (c_pid > 0)
  {
    std::map<int, std::string> config{{1, "elmo1"}, {2, "elmo2"}, {3, "elmo3"}};
    MyServer server(config, "localhost", set_port, get_port);

    usleep(20 * 1e6);
  }
  else
  {
     MyClient ex("127.0.0.1", std::to_string(get_port));

     ex.write_correct_request<get_t::Request>();
     ex.read_response<get_t::Response>();
  }

  return 0;
}

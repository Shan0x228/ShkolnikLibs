/*********************************************************************************************************************
* AtomHTTP - single-header(small) http library, coded for self perfomance. Only POST and GET request implemented yet.*
* Coded by:  Shkolnik Prahramist(Shan0x228)                                                                          *
* Contacts:  Discord Shan0x228#5690                                                                                  *
* YouTube :  https://www.youtube.com/channel/UCmJT3IfHtpFJyln2UdABBKg                                                *
* Supported platforms : UNIX(macOS, Linux...), Windows                                                               *
* Version :  2.0                                                                                                     *
*********************************************************************************************************************/

#ifdef _WIN32
  // Windows
  #include <WinSock2.h>
  #include <ws2tcpip.h>
#else
  // UNIX
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <fcntl.h>
  #include <unistd.h>
  #include <sys/types.h>	
#endif

#include <string>
#include <vector>
#include <regex>
#include <exception>
#include <algorithm>
#include <chrono>
#include <thread>

namespace Shkolnik
{
  namespace net
  {
    // IP-address regular expression
    static const std::regex url_ip  (R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)");
    // URL        regular expression
    static const std::regex url     (R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)");

    // AtomHTTP - class definition
    class AtomHTTP
    {
    // Private section with server variables
    private:
      std::string               ip_server;       // server ip
      int                       port_server;     // server port
      std::vector<std::string>  http_headers;    // http headers request
      std::string               content;         // full response content(without headers)
      std::string               data;            // response body(with headers)

    // Private section with native sockets variables
    public:
      int         socket_handle;                // socket handle
      int         connection_handle;            // connection handle
      sockaddr_in peer;                         // connection struct
    
    // Public section with methods
    public:
      // Enumiration with request types
      enum class HTTP
      {
        GET,
        POST
      };

      // Default CTOR
      AtomHTTP(const std::string &URL, const int &port);

      // Method return server ip
      std::string get_server_ip();

      // Method return server port
      int         get_server_port();

      // Method perform request
      void        request(std::string PAGE, AtomHTTP::HTTP req_type);

      // Method add custom header to request
      void        add_header(std::string param);

      // Method send request to server and return full response(with headers)
      std::string send_request();

      // Method return response body(without headers)
      std::string get_content();

      // Default DTOR
      ~AtomHTTP();
    };
  }
}

// Default CTOR
Shkolnik::net::AtomHTTP::AtomHTTP(const std::string &URL, const int &port)
{
  #ifdef _WIN32
	 WSADATA wsaData = { 0 };
	 if (int iRes = WSAStartup(MAKEWORD(2, 2), &wsaData); iRes != 0)
	    throw std::string("AtomHTTP: can't init WSA!");
  #endif

  // setup port
  this->port_server = port;
  // check regex - IP
  if (std::regex_match(URL, url_ip))
  {
    // setup IP
    this->ip_server = URL;
  }
  // check regex - URL
  else if (std::regex_match(URL, url))
  {
    // setup URL && convert to ip(ASCII)
    this->ip_server = inet_ntoa(*(in_addr*)(gethostbyname(URL.c_str())->h_addr));
  }
  else
    // Exception
    throw std::string("AtomHTTP: can't resolve this URL!");
}

// Method return server ip
std::string Shkolnik::net::AtomHTTP::get_server_ip()
{
  return this->ip_server;
}

// Method return server port
int Shkolnik::net::AtomHTTP::get_server_port()
{
  return this->port_server;
}

// Method perform request
void Shkolnik::net::AtomHTTP::request(std::string PAGE, AtomHTTP::HTTP req_type)
{
  // checking request PAGE-URL string
  PAGE.empty() ? PAGE="/" : PAGE;

  // creating socket, and obtaining handle
  #ifdef _WIN32
    // Windows
    this->socket_handle = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_handle == INVALID_SOCKET)
      throw std::string("AtomHTTP: can't create socket!");
  #else
    // UNIX
    this->socket_handle = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socket_handle <= 0)
      throw std::string("AtomHTTP: can't create socket!");
  #endif

  // setup connection struct
  this->peer.sin_family      = AF_INET;
  this->peer.sin_port        = htons(this->port_server);
  this->peer.sin_addr.s_addr = inet_addr(this->ip_server.c_str());

  // trying to connect...
  this->connection_handle = connect(this->socket_handle, reinterpret_cast<sockaddr*>(&peer), sizeof(this->peer));
  if (this->connection_handle)
    throw std::string("AtomHTTP: can't connect to server!");

  // prepair default http-headers...
  req_type == HTTP::GET ? this->http_headers.push_back("GET "+PAGE+" HTTP/1.1\r\n") : this->http_headers.push_back("POST "+PAGE+" HTTP/1.1\r\n");
}

// Method add custom header to request
void Shkolnik::net::AtomHTTP::add_header(std::string param)
{
  // check if request structure not setup'ed
  if (this->http_headers.at(0).empty())
    throw std::string("AtomHTTP: first, you need to call request()!");

  // check if header is not empty
  if (!param.empty())
  {
    // stuff check...
    if (param.find_last_of("\r\n") == std::string::npos)
      param.append("\r\n");

    // add header to request-struct
    this->http_headers.push_back(param);
  } 
}

// Method send request to server and return full response(with headers)
std::string Shkolnik::net::AtomHTTP::send_request()
{
  // contains full request body
  std::string req{};

  // adding stuff for http protocol...
  this->http_headers.push_back("\r\n");

  // fill with all headers
  for (const auto x : this->http_headers)
    req += x;

  // send by socket for server
  int bytes = send(this->socket_handle, req.c_str(), req.length(), 0);
  if (bytes <= 0)
    throw std::string("AtomHTTP: some error in send(...)!");

  // make socket non blocking
  #ifdef _WIN32
    // Windows
    u_long nonblocking_enabled = TRUE;
    ioctlsocket(this->socket_handle, FIONBIO, &nonblocking_enabled);
  #else
    // UNIX
    fcntl(this->socket_handle, F_SETFL, O_NONBLOCK);
  #endif
  
  
  /*RESPONSE SIZE CALCULATION*/
  int total_size{},           // total size of responce 
      recv_size{};            // data chunk size

  // contains 512bytes of recived data
  std::string chunk{};
  // re-size
  chunk.resize(512);
  while (true)
  {
    // some delay...
    #ifdef _WIN32
      // Windows
      std::this_thread::sleep_for(std::chrono::microseconds(1000));
    #else
      // UNIX
      usleep(1000);
    #endif

    // reciving data from server by socket
    int n = recv(this->socket_handle, &chunk[0], chunk.size(), 0);

    // check if all data has been sended
    if (total_size > 0 && n <= 0)
      break;

    // increment total size by recived bytes value
    total_size += n;
    // adding chunk-string to data
    this->data += chunk;
  }

  // content(without headers and other http stuff) formatting
  std::size_t content_pos = this->data.find("\r\n\r\n");
  if (content_pos != std::string::npos)
  {
    /*HTTP-STUFF CUT*/
    this->content = this->data.substr(content_pos+4);
    this->content.erase(std::remove(std::begin(this->content), std::end(this->content), 0), std::end(this->content));
  }

  // return all data with http stuff
  return this->data;
}

// Method return response body(without headers)
std::string Shkolnik::net::AtomHTTP::get_content()
{
  return this->content;
}

// Default DTOR
Shkolnik::net::AtomHTTP::~AtomHTTP()
{
    // clear instance
    WSACleanup();
}

#include <cstring>          // Memset
#include <sys/socket.h>     // Socket functions
#include <netdb.h>          // Socket funtions
#include <unistd.h>         // Close() socket
#include <errno.h>          // Error messages

class socketHandler
{
    public:

        struct addrinfo _host_info;
        struct addrinfo *_host_info_list;
        int _socketfd;
        std::string _addr;
        std::string _port;

        bool debug = false;

        socketHandler(std::string addr, std::string port)
        {
            _addr = addr;
            _port = port;
         
            memset(&_host_info, 0, sizeof _host_info);    // Because allocated memory is not necessary empty
        }

        ~socketHandler()
        {
            freeaddrinfo(_host_info_list);
            close(_socketfd);
        }


        int init(bool passive = false)
        {
            if (debug)
            {
                std::cout << "Setting the structs..." << std::endl;
            }

            _host_info.ai_family = AF_UNSPEC;        // Let getaddrinfo() decide if IPv4 or IPv6
            _host_info.ai_socktype = SOCK_STREAM;    // We want a TCP connection

            if (passive)
            {
                _host_info.ai_flags = AI_PASSIVE;         // Accept any connection on localhost
            }

            int status;
            // Fill the struct with packet values
            status = getaddrinfo(_addr.c_str(), _port.c_str(), &_host_info, &_host_info_list);
         
            if (status != 0)
            {
                std::cout << "getaddrinfo() Error" << std::endl << gai_strerror(status) << std::endl;
                std::cout << _addr << std::endl;
                std::cout << _port << std::endl;                
                return -1;
            }

            if (debug)
            {
                std::cout << "Creating socket..." << std::endl;
            }
            
            _socketfd = socket(_host_info_list->ai_family, _host_info_list->ai_socktype, _host_info_list->ai_protocol);
         
            if (_socketfd == -1)
            {
                std::cout << "Socket Error" << std::endl << strerror(errno);
                return -1;
            }

            return 1;
        }

        int startConnection()
        {

            if (debug)
            {
                std::cout << "Connecting..." << std::endl;
            }

            int status;
            status = connect(_socketfd, _host_info_list->ai_addr, _host_info_list->ai_addrlen);
 
            if (status == -1)
            {
                std::cout << "Connect error" << std::endl << strerror(errno) << std::endl << errno << std::endl;
                return -1;
            }

            return 1;
        }

        int sendMessage(std::string message, int socketfd = 0)
        {
            if (socketfd == 0)
            {
                socketfd = _socketfd;
            }

            if (debug)
            {
                std::cout << "Sending message..." << std::endl;
            }

            return send(socketfd, message.c_str(), message.size(), 0);
        }

        std::string receive(int socketfd = 0, int bufferSize = 1500)
        {
            if (socketfd == 0)
            {
                socketfd = _socketfd;
            }

            if (debug)
            {
                std::cout << "Waiting to recieve data..." << std::endl;
            }

            char incoming_data_buffer[bufferSize];
            std::string answer;
            
            struct timeval tv;
            tv.tv_sec = 1;          //Timeout sec
            tv.tv_usec = 0*1000;   //Timeout ms

            setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

            int bytes_received;
            while ((bytes_received = recv(socketfd, incoming_data_buffer, bufferSize, 0)) > 0)
            {
                for (int x = 0; x < bytes_received; ++x)
                {
                    answer.push_back(incoming_data_buffer[x]);
                }

                std::fill(&incoming_data_buffer[0], &incoming_data_buffer[sizeof(incoming_data_buffer)], 0);
            }
            return answer;
        }

        int bindPort()
        {
            if (debug)
            {
                std::cout << "Binding socket..." << std::endl;
            }

            //Make sure port is not in use by previous code
            int yes = 1;
            int status = setsockopt(_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
            status = bind(_socketfd, _host_info_list->ai_addr, _host_info_list->ai_addrlen);

            if (status == -1)
            {
                std::cout << "Bind error" << std::endl << strerror(errno) << std::endl;
                return -1;
            }

            return 1;
        }

        int startListen()
        {
            if (debug)
            {
                std::cout << "Listening for connections..." << std::endl;
            }
            
            int status = listen(_socketfd, 5);

            if (status == -1)
            {
                std::cout << "Listen error" << std::endl << strerror(errno) << std::endl;
                return -1;
            }

            return 1;
        }

        int acceptConnection()
        {
            if (debug)
            {
                std::cout << "Waiting for client" << std::endl;
            }

            struct sockaddr_storage their_addr;
            socklen_t addr_size;

            return accept(_socketfd, (struct sockaddr *)&their_addr, &addr_size);
        }
};


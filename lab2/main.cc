#include <iostream>
#include <signal.h>

#include "socketHandler.cc"
#include "requestHandler.cc"

// Catching unexpected exits like ctrl+c
void exit_handler(int s)
{
    exit(0);
}

int main(int argc, char *argv[])
{
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = exit_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    
    std::string port = "8080";
    if(argc>1){
        port = argv[1];
    }

    std::cout << "Using port: " << port << std::endl;

    // Start proxy listen socket
    socketHandler serverSocket("127.0.0.1", port);
    serverSocket.init(true);
    serverSocket.bindPort();
    serverSocket.startListen();

    while(1)
    {
        int new_fd = serverSocket.acceptConnection();
        if (!fork())
        {
            std::string requested = serverSocket.receive(new_fd);
            if (requested.length() > 0)
            {
                std::string answer;

                if (requestHandler::passedFilter(requested))
                {
                    std::string host = requestHandler::getHost(requested);
                    std::string URI = requestHandler::getURI(requested);
                    std::string modifiedReq = requestHandler::assembleGetRequest(host, URI);

                    // Start webserver connection
                    socketHandler webSocket(host, "80");
                    webSocket.init();
                    webSocket.startConnection();
                    webSocket.sendMessage(modifiedReq);

                    std::cout << "Requested: " << host + URI << std::endl;
                    answer = webSocket.receive();

                    if (requestHandler::isText(answer) && !requestHandler::passedFilter(answer))
                    {
                        std::cout << "Redirected: Bad content" << std::endl;
                        answer = requestHandler::assembleAnswer("302", "http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error2.html");
                    }
                }
                else
                {
                    std::cout << "Redirected: Bad URI" << std::endl;
                    answer = requestHandler::assembleAnswer("302", "http://www.ida.liu.se/~TDTS04/labs/2011/ass2/error1.html");
                }

                // Answer client request
                serverSocket.sendMessage(answer, new_fd);
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }

    std::cout << "Thank you for using our software.\nBye!" << std::endl;

    return 0;
}


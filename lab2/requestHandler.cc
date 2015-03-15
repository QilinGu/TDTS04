#include <algorithm>

class requestHandler
{
    public:
        static std::string getHost(std::string txt)
        {
            std::string host;
            std::string startIdentifier = "Host:";
            int startLength = startIdentifier.length();

            std::string endIdentifier = "\r\n";

            int beginning = txt.find(startIdentifier);
            if (beginning != std::string::npos)
            {
                std::size_t end = txt.find(endIdentifier, beginning);
                host = txt.substr(beginning+startLength, end-beginning-startLength);
            }

            removeSpaces(host);
            return host;
        }

        static std::string getURI(std::string txt)
        {
            std::string URI;

            std::string startIdentifier = "GET";
            int startLength = startIdentifier.length();

            std::string endIdentifier = "HTTP/1";

            int beginning = txt.find(startIdentifier);

            if (beginning != std::string::npos)
            {
                std::size_t end = txt.find(endIdentifier, beginning);
                URI = txt.substr(beginning+startLength, end-beginning-startLength);
            }
            else
            {
                std::cout << "Possible POST request detected (not supported), request discontinued." << std::endl;
                exit(1);
            }

            removeSpaces(URI);

            int offset = 7; // http:// length
            URI = URI.substr(getHost(txt).length() + offset);

            return URI;
        }

        static std::string assembleGetRequest(std::string host, std::string URI)
        {
            return "GET " + URI + " HTTP/1.1\r\nHost: " + host + "\r\nAccept: image/png,image/*;q=0.8,*/*;q=0.5\nConnection: close\r\n\r\n";
        }

        static std::string assembleAnswer(std::string httpCode, std::string URI)
        {
            return "HTTP/1.1 " + httpCode + "Found\r\nLocation: " + URI + "\r\n\r\n";
        }

        static bool passedFilter(std::string txt)
        {
            std::string badWords[] = {"SpongeBob","Britney Spears","Paris Hilton","Norrköping"};
            for (std::string word : badWords)
            {
                if (txt.find(word) != std::string::npos)
                    return false;
            }
            return true;
        }

        static bool isText(std::string txt)
        {
            return (txt.find("text/") != std::string::npos);
        }

    private:
        static void removeSpaces(std::string &txt)
        {
            txt.erase(remove_if(txt.begin(), txt.end(), isspace), txt.end());
        }
};


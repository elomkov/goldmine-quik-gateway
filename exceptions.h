
#ifndef GQG_EXCEPTIONS
#define GQG_EXCEPTIONS 

#include <exception>
#include <string>
#include <boost/exception/all.hpp>

struct GoldmineQuikGatewayException : virtual public boost::exception, virtual public std::exception
{
    const char* what() const throw()
    {
        return boost::diagnostic_information_what(*this);
    }
};

struct ProtocolError : public GoldmineQuikGatewayException
{
};

struct ZmqError : public GoldmineQuikGatewayException
{
};

typedef boost::error_info<struct errinfo_str_, std::string> errinfo_str;


#endif /* ifndef GQG_EXCEPTIONS */

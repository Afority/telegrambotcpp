#include "../boost_1_86_0/boost/asio.hpp"
#include "../boost_1_86_0/boost/asio/ssl.hpp"
#include "../boost_1_86_0/boost/beast.hpp"

using namespace boost::beast;
using namespace boost::asio;

http::response<http::string_body> send_request(std::string url, std::string path){
    io_service svc;

    ssl::context ctx(ssl::context::sslv23_client);
    ssl::stream<ip::tcp::socket> ssocket = { svc, ctx };
    ip::tcp::resolver resolver(svc);
    auto it = resolver.resolve(url, "443");
    connect(ssocket.lowest_layer(), it);
    ssocket.handshake(ssl::stream_base::handshake_type::client);
    http::request<http::string_body> req{ http::verb::get, path, 11 };
    req.set(http::field::host, url);
    http::write(ssocket, req);
    http::response<http::string_body> res;
    flat_buffer buffer;
    http::read(ssocket, buffer, res);
    return res;
}

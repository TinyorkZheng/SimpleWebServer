//
// SERVER_BASE_HPP
//

#ifndef SERVER_BASE_HPP
#define SERVER_BASE_HPP

#include <boost/asio.hpp>

#include <regex>
#include <unordered_map>
#include <thread>

namespace WebSever
{
	struct Request
	{
		std::string method, path, http_version;
		std::shared_ptr <std::istream> content;
		std::unordered_map <std::string, std::string> header;
		std::smatch path_match;
	};

	typedef std::map <std::string, std::unordered_map <std::string, 
		std::function<void(std::ostream&, Request&)>>> resource_type;


	// socket_type = {HTTP,HTTPS}
	template <typename socket_type>
	class ServerBase
	{
	  public:
	  	resource_type resource;
	  	resource_type default_resource;

	  protected:
	  	std::vector <resource_type::iterator> all_resource;
	  	// asio 库中的 io_service 是调度器，
	  	// 所有的异步IO事件通过它来分发处理
	  	boost::asio::io_service m_io_service;
	  	// IP 地址、端口号、协议版本构成一个 endpoint，并通过这个 endpoint 在服务端生成
        // tcp::acceptor 对象，并在指定端口上等待连接
	  	boost::asio::ip::tcp::endpoint endpoint;
        // 所以构造 acceptor 对象需要 io_service 和 endpoint
	  	boost::asio::ip::tcp::acceptor acceptor;

	  	// server threads
	  	size_t num_threads;
	  	std::vector<std::thread> threads;

	  	// 所有的资源及默认资源都会在 vector 尾部添加, 并在 start() 中创建
	  	std::vector<resource_type::iterator> all_resource;

	  public:
	  	// constructor
	  	ServerBase(unsigned short port, size_t num_threads=1):
	  		endpoint(boost::asio::ip::tcp::v4(),port),
	  		acceptor(m_io_service,endpoint),
	  		num_threads(num_threads){} 
		void start();  // TODO

	  protected:
		virtual void accept(){}   
	  	void process_request_and_response(std::shared_ptr<socket_type>) const; // TODO

	};

	template <typename socket_type>
	class Server: public ServerBase {};
}

#endif /* SEVER_BASE_HPP */

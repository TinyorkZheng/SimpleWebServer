//
// SEVER_BASE_HPP
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
		void start()
		{
			// 默认资源放在 vector 的末尾, 用作默认应答
    		// 默认的请求会在找不到匹配请求路径时，进行访问，故在最后添加
			for (auto it=resource.begin(); it!=resource.end(); it++){
				all_resource.push_back(it);
			}
			for (auto it=default_resource.begin(); it!=default_resource.end(); it++){
				all_resource.push_back(it);
			}

			// 调用 socket 的连接方式，还需要子类来实现 accept() 逻辑
			accept();

			// 如果 num_threads>1, 那么 m_io_service.run()
			// 将运行 (num_threads-1) 线程成为线程池
			for (size_t c=1; c<num_threads; c++){
				threads.emplace_back( [this]() { 
					m_io_service.run();
				});
			}

			// main thread
			m_io_service.run();

			// 等待其他线程，如果有的话, 就等待这些线程的结束
			for (auto& t: threads)
				t.join();
		}

	  protected:
		virtual void accept(){}   

	  	void process_request_and_response(std::shared_ptr<socket_type>) const
	  	{
	  		// 为 async_read_untile() 创建新的读缓存
    		// shared_ptr 用于传递临时对象给匿名函数
    		// 会被推导为 std::shared_ptr<boost::asio::streambuf>
	  		auto read_buffer = std::make_shared<boost::asio::streambuf>();

	  		boost::asio::async_read_untile(*socket, &read_buffer, "\r\n\r\n",
	  		[this, socket, read_buffer](const boost::system::error_code& ec, size_t bytes_transferred){
	  			if(!ec){
	  				// 注意：read_buffer->size() 的大小并一定和 bytes_transferred 相等， Boost 的文档中指出：
            		// 在 async_read_until 操作成功后,  streambuf 在界定符之外可能包含一些额外的的数据
            		// 所以较好的做法是直接从流中提取并解析当前 read_buffer 左边的报头, 再拼接 async_read 后面的内容
	  				size_t total = read_buffer->size();
	  				std::istream stream(read_buffer.get());
	  				auto request = std::make_shared<Request>();

	  				// 解析 stream 中的请求信息，保存到 request 对象中
	  				auto request - std::make_shared<Request>();
	  				*request = parse_request(stream);

	  				size_t num_additional_bytes = total-bytes_transferred;

	  				if(request->head.count("Content-Legth") > 0 ){
	  					boost::asio::async_read(*socket, *read_buffer,
	  					boost::asio::transfer_exactly(stoull(request->header["Content-Legth"])-num_additional_bytes),
	  					[this,socket, read_buffer, request](const boost::system::error_code& ec, 
	  					size_t bytes_transferred){
	  						if(!ec){
	  							request->content = std::shared_ptr<std::istream>(
	  								new std::istream(read_buffer.get() ));
	  						}
	  						else{
	  							respond(socket, request);
	  						}
	  					});
	  				}
	  			}
	  		});
	  	}
	  	
	  	//
	  	Request parse_request(std::istream& stream) const{
	  		Request request;
	  		std::regex e("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");

	  		std::smatch sub_match;

	  		// method path version
	  		std::string line;
	  		getline(straeam, line);
	  		line.pop_back();
	  		if(std::regex_match(line, sub_match, e)){
	  			request.method       = sub_match[1];
	  			request.path         = sub_match[2];
	  			request.http_version = sub_match[3];

	  			bool matched;
	  			e = "^([^:]*): ?(.*)$";
	  			do{
	  				getline(stream, line);
	  				line.pop_back();
	  				matched = std::regex_match(line, sub_match, e);
	  				if(matched){
	  					request.header[sub_match[1]] = sub_match[2];
	  				}
	  			}while(matched == true);
	  		}
	  		return request;
	  	}

	  	// 
	  	void respond(std::shared_ptr<socket_type> socket, std::shared_ptr<Request> request) const{
	  		// match path and method and 
	  		for (auto res_it: all_resource){
	  			std::regex e(res_it->first);
	  			std::smatch sm_res;
	  			if(std::regex_match(request->path, sm_res, e)){
	  				if(res_it->second.count(request->method) > 0 ){
	  					request0>path_match = move(sm_res);

	  					auto write_buffer = std::make_shared<boost::asio::streambuf>();
	  					std::ostream response(write_buffer.get());
	  					res_it->second[request->method](response, *request);
	  					// 
	  					boost::asio::async_write(*socket, *write_buffer,
	  					[this, socket, write_buffer](const boost::system::error_code& ec, size_t bytes_transferred){
	  						if(!ec && stof(request->http_version) > 1.05)
	  							process_request_and_response(socket);
	  					});
	  					return;
	  				}
	  			}
	  		}
	  	} 

	};

	template <typename socket_type>
	class Server: public ServerBase<socket_type> {};

}

#endif /* SEVER_BASE_HPP */
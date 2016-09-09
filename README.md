# SimpleWebServer
 使用 C++11 及 Boost 中的 Asio 实现 HTTP 和 HTTPS 的服务器框架

# 编译说明
g++ >=4.9   --- 保证C++11正常编译
boost 库

# 浏览器访问web服务器的过程了解
1-服务器启动后通过socket监听端口上的请求          
  start()接口
2-客户端发送相应的HTTP/HTTPS请求，来要访问服务器资源，          
  服务器收到请求，处理并返回客户端需要的资源       
  accept()、process_request_and_respond()    

# 类的设计
确定基类为ServerBase模板类，Server继承ServerBase
对ServerBase基类，start可以设为public

HTTP 和 HTTPS 两种方式的服务器之间在处理、返回请求的区别在于如何处理与客户端建立连接的方式上，也就是 accept() 方法，设计子类时可以设计成虚函数，让不同类型的服务器来实现


# 懒




// 
// main_http.cpp
// web_server
//

#include "server_http.hpp"
#include "handler.hpp"

using namespace WebServer;

int main(){
	// http server run on port 12345 and run 4 thread

	Server<HTTP> server(12345,4);
	start_server<Server<HTTP>>(server);

	return 0;
	
}


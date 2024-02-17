#include <simple_http.h>

#include <cstdlib>

int main() {
    simple_http::HttpServer server;
    server.sayHello();
    return EXIT_SUCCESS;
}
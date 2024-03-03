const http = require("http");

const server = http.createServer((_request, response) => {
    response.writeHead(200, "OK");
    response.end();
});

server.listen(3000);

console.log("Server is running at http://localhost:3000...");

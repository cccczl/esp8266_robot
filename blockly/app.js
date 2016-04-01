var app = require("express")();
var server = require("http").createServer(app);
var io = require("socket.io").listen(server);
var bone = require("bonescript");
var robot = require("./robot");

io.set('log level', 1); 
server.listen(9000);

app.get('/', function handler(request, response) {
  console.log("request: ", request.url); 
  response.sendfile(__dirname + "/index.html");
});

app.get(/^(.+)$/, function handler(request, response) {
  console.log("request: ", request.url);
  response.sendfile(__dirname + "/" + request.params[0]);
});

io.sockets.on("connection", function(socket) {
  
  socket.on("disconnect", function() {
    console.log("Bye bye programmer client");
  });

  socket.on("program", function(program) {
    console.log(program);
    eval(program);
  });

  console.log("Hello programmer client");
}); 

console.log("Server started on port 9000"); 


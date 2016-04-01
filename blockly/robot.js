var MAX_COMMANDS = 50;

var PROGRAM = "0";
var FORWARD = "1";
var BACKWARD = "2";
var LEFT = "3";
var RIGHT = "4";
var START = "5";

var cmds;
var url;

function robot_url(robot) {
	url = robot;
}

function robot_start() {
	cmds = "";
}

function robot_forward() {
	cmds = cmds + FORWARD;
}

function robot_backward() {
	cmds = cmds + BACKWARD;
}

function robot_left() {
	cmds = cmds + LEFT;
}

function robot_right() {
	cmds = cmds + RIGHT;
}

function robot_end() {
  if (cmds.length >= MAX_COMMANDS) {
	  console.log("Error: cmd limit exceeded (max: " + MAX_COMMANDS + ")");
	  send_commands(PROGRAM);
  } else if (cmds.length <= 0) {
	  send_commands(PROGRAM);
  } else {
   	  cmds = cmds + PROGRAM + START;
	  send_commands(cmds);
  }
}

function robot_send(request, callback)
{
    var xmlHttp = new XMLHttpRequest();
	console.log("Request: " + request);

    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
    }

    xmlHttp.open("GET", request, true);
    xmlHttp.send(null);
}

function send_commands(cmds) {
	uri = url + "?robot=" + cmds;
	robot_send(uri, console.log);
}
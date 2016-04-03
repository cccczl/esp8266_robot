var MAX_COMMANDS = 50;

var PROGRAM = "0";
var FORWARD = "1";
var BACKWARD = "2";
var LEFT = "3";
var RIGHT = "4";
var START = "5";

var cmd;
var url;

function robot_url(robot) {
	url = robot;
}

function robot_start() {
	cmd = "";
}

function robot_forward() {
	cmd = cmd + FORWARD;
}

function robot_backward() {
	cmd = cmd + BACKWARD;
}

function robot_left() {
	cmd = cmd + LEFT;
}

function robot_right() {
	cmd = cmd + RIGHT;
}

function robot_end() {
  var msg;
  if (cmd.length > MAX_COMMANDS) {
	  msg = "Error: cmd limit exceeded (max: " + MAX_COMMANDS + ")";
	  console.log(msg);
	  robot_command(PROGRAM);
  } else if (cmd.length <= 0) {
	  msg = "Info: no commands";
	  console.log(msg);
	  robot_command(PROGRAM);
  } else {
	  msg = "Info: program sent";
   	  cmd = cmd + PROGRAM + START;
	  robot_command(cmd);
  }
  
  return msg;
}

function robot_send(request)
{
    if (url) {
	  request = "http://" + url + request;
    }
	
	var xmlHttp = new XMLHttpRequest();
	console.log("Request: " + request);

    xmlHttp.open( "GET", request, false );
    xmlHttp.send( null );
    return xmlHttp.responseText;
}

function robot_AP(url, ssid, passphrase) {
  var uri = "/robot?ssid=" + ssid + "&" + "passphrase=" + passphrase;
  robot_url(url);
  
  var response = robot_send(uri);
  console.log(response);
  return response;
}

function robot_command(cmd) {
	var uri = "/robot?cmd=" + cmd;
	var response = robot_send(uri);
	console.log(response);
	return response;
}

function robot_test(gpio, value) {
	var uri = "/test?gpio=";
	uri	+= gpio;
	uri += "&value=";
	uri += value;
	var response = robot_send(uri);
	console.log(response);
	return response;	
}
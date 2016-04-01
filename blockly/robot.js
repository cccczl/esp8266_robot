var MAX_COMMANDS = 50;

var PROGRAM = "0";
var FORWARD = "1";
var BACKWARD = "2";
var LEFT = "3";
var RIGHT = "4";
var START = "5";

var cmds;

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

function send_commands(cmds) {
	console.log("Send commands: " + cmds);
}
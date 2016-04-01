'use strict';

goog.provide('Blockly.JavaScript.robot');

goog.require('Blockly.JavaScript');

Blockly.JavaScript['robot_start'] = function(block) {
  return "robot_start();\n";
};

Blockly.JavaScript['robot_end'] = function(block) {			  
  return "robot_end();\n";
};

Blockly.JavaScript['robot_move'] = function(block) {
  var dropdown_move = block.getFieldValue('move');
  var code;
  
  if (dropdown_move == 'forward') {
	  code = "robot_forward();\n";
  } else {
	  code = "robot_backward();\n";
  }
  
  return code;
};

Blockly.JavaScript['robot_turn'] = function(block) {
  var dropdown_turn = block.getFieldValue('turn');
  var code; 
  
  if (dropdown_turn == 'left') {
	  code = "robot_left();\n";
  } else {
	  code = "robot_right();\n";
  }
  
  return code;
};
	
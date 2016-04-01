'use strict';

goog.provide('Blockly.Blocks.robot');

goog.require('Blockly.Blocks');


/**
 * Common HSV hue for all blocks in this category.
 */
Blockly.Blocks.robot.HUE = 230;


Blockly.Blocks['robot_start'] = {
  init: function() {
    this.appendDummyInput()
		.appendField("start");
    this.setInputsInline(true);
    this.setNextStatement(true);
    this.setColour(Blockly.Blocks.robot.HUE);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['robot_end'] = {
  init: function() {
    this.appendDummyInput()
		.appendField("end");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setNextStatement(false);
    this.setColour(Blockly.Blocks.robot.HUE);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['robot_move'] = {
  init: function() {
    this.appendDummyInput()
		.appendField("move")
        .appendField(new Blockly.FieldDropdown([["forward", "FORWARD"], ["backward", "BACKWARD"]]), "move");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(Blockly.Blocks.robot.HUE);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

Blockly.Blocks['robot_turn'] = {
  init: function() {
    this.appendDummyInput()
		.appendField("turn")
        .appendField(new Blockly.FieldDropdown([["left", "LEFT"], ["right", "RIGHT"]]), "turn");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setColour(Blockly.Blocks.robot.HUE);
    this.setTooltip('');
    this.setHelpUrl('http://www.example.com/');
  }
};

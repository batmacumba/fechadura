/*******************************************************************************
 Esse programa deve simular a entrada e saída de usuários de um ambiente prote-
 gido por uma fechadura inteligente. Cada entrada ou saída será enviada à um
 servidor MQTT seguindo um padrão JSON
 ******************************************************************************/

var crypto = require('crypto');
var mqtt = require('mqtt');
var client  = mqtt.connect('mqtt://127.0.0.1')
var current = [];

function someoneComesIn() {
	var random = Math.random().toString();
	var id = crypto.createHash('sha256').update(random).digest('hex');
	var message = '{ "id": "' + id + '", "timestamp": "' + new Date().toISOString() + '" }';
	current.push(id);
	client.publish('entrada', message);
}

function someoneComesOut() {
	if (current.length === 0) return;
	var random = Math.floor(Math.random() * current.length);
	var id = current.splice(random, 1)[0];
	var message = '{ "id": "' + id + '", "timestamp": "' + new Date().toISOString() + '" }';
	client.publish('saida', message);
	
}

function coinToss() {
	if (Math.random() < 0.5) someoneComesIn();
	else someoneComesOut();
	var message = '{ "ocupantes": "' + current + '", "timestamp": "' + new Date().toISOString() + '" }';
	client.publish('ocupacao', message);
}

setInterval(coinToss, 1000);
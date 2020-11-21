/*******************************************************************************
 Esse programa deve simular a entrada e saída de usuários de um ambiente prote-
 gido por uma fechadura inteligente. Cada entrada ou saída será enviada à um
 servidor MQTT seguindo um padrão JSON
 ******************************************************************************/

var mqtt = require('mqtt')
var client  = mqtt.connect('mqtt://test.mosquitto.org')
 
client.on('connect', function () {
	client.publish('presence', 'Hello mqtt')
})
 
client.on('message', function (topic, message) {
  // message is Buffer
  console.log(message.toString())
  client.end()
})

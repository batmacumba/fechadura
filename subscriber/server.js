/*******************************************************************************
 O subscriber deve assinar os tópicos 'entrada', 'saida' e 'ocupacao' do servidor
 MQTT, salvando os eventos no banco de dados MongoDB.
 ******************************************************************************/

// Configuração do MongoDB e dos modelos

const mongoose = require('mongoose');
mongoose.connect('mongodb://root:password@localhost:27017', {useNewUrlParser: true});

const db = mongoose.connection;
db.on('error', console.error.bind(console, 'Could not connect to database!'));
db.once('open', function() {
 	console.log('Connected to database!');
});

const eventoSchema = new mongoose.Schema({
	id: String,
	timestamp: Date
});

const ocupacaoSchema = new mongoose.Schema({
	ocupantes: Array,
	timestamp: Date
});

const Entrada = mongoose.model('Entrada', eventoSchema);
const Saida = mongoose.model('Saida', eventoSchema);
const Ocupacao = mongoose.model('Ocupacao', ocupacaoSchema);

/******************************************************************************/

// Assinatura dos tópicos MQTT e handling das novas mensagens

var mqtt = require('mqtt');
var client  = mqtt.connect('mqtt://127.0.0.1');
var instantanea = []


function registrar_entrada(message) {
	const entrada = new Entrada(JSON.parse(message));
	entrada.save(function (err, entrada) {
    	if (err) return console.error(err);
  	});
}

function registrar_saida(message) {
	const saida = new Saida(JSON.parse(message));
	saida.save(function (err, saida) {
    	if (err) return console.error(err);
  	});
}

function registrar_ocupacao(message) {
	const ocupacao = new Ocupacao(JSON.parse(message));
	ocupacao.ocupantes = ocupacao.ocupantes[0].split(',');
	instantanea = ocupacao.ocupantes
	ocupacao.save(function (err, ocupacao) {
    	if (err) return console.error(err);
  	});
}

client.on('connect', function () {
	client.subscribe('entrada');
	client.subscribe('saida');
	client.subscribe('ocupacao');
});

client.on('message', function (topic, message) {
	if (topic === 'entrada') registrar_entrada(message.toString());
	else if (topic === 'saida') registrar_saida(message.toString());
	else if (topic === 'ocupacao') registrar_ocupacao(message.toString());
});


/******************************************************************************/

// Criação das rotas e parse das informações no DB

const express = require('express')
const app = express()
const port = 3001

app.get('/instantanea', (req, res) => {
	res.send(instantanea.length.toString());
});

app.get('/diaria', (req, res) => {
	var weekAgo = new Date();
	weekAgo.setDate(weekAgo.getDate() - 7);
	var pipeline = [ 
		{$match: {timestamp: {$gte: weekAgo}}},
	    {$project: {hour: {$hour: '$timestamp'}, 
	    			ocupacao: {$size: '$ocupantes'}}}
	];
	db.collection('ocupacaos').aggregate(pipeline, async function(err, query) {
	    if (err) throw err;
	    ocupacao_total = new Array(24).fill(0);
	    quantidade_de_valores = new Array(24).fill(0);
	    await query.forEach(function(doc) { 
	    	ocupacao_total[doc.hour] += doc.ocupacao;
	    	quantidade_de_valores[doc.hour] += 1;
	    });
	    ocupacao_media = new Array(24).fill(0);
	    for (let i = 0; i <= 24; i++) {
	    	if (quantidade_de_valores[i] > 0)
	    		ocupacao_media[i] = ocupacao_total[i] / quantidade_de_valores[i];
	    }
	    res.send(ocupacao_media.toString())
	});
});

/* Devolve a ocupação média por dia de semana nos últimos sete dias */
app.get('/semanal', (req, res) => {
	var weekAgo = new Date();
	weekAgo.setDate(weekAgo.getDate() - 7);
	var pipeline = [ 
		{$match: {timestamp: {$gte: weekAgo}}},
	    {$project: {dayOfWeek: {$dayOfWeek: '$timestamp'}, 
	    			ocupacao: {$size: '$ocupantes'}}}
	];
	db.collection('ocupacaos').aggregate(pipeline, async function(err, query) {
	    if (err) throw err;
	    ocupacao_total = new Array(7).fill(0);
	    quantidade_de_valores = new Array(7).fill(0);
	    await query.forEach(function(doc) { 
	    	ocupacao_total[doc.dayOfWeek] += doc.ocupacao;
	    	quantidade_de_valores[doc.dayOfWeek] += 1;
	    });
	    ocupacao_media = new Array(7).fill(0);
	    for (let i = 0; i <= 7; i++) {
	    	if (quantidade_de_valores[i] > 0)
	    		ocupacao_media[i] = ocupacao_total[i] / quantidade_de_valores[i];
	    }
	    res.send(ocupacao_media.toString())
	});
});

app.listen(port, () => {
  console.log(`Subscriber listening at http://localhost:${port}`)
});

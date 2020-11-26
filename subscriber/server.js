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
var instantanea = [];
var uid_desconhecido = "";
var autorizados = [];


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

function registrar_desconhecido(message) {
	uid_desconhecido = message;
}

function registrar_autorizados(message) {
	autorizados.push(message);
}

client.on('connect', function () {
	client.subscribe('entrada');
	client.subscribe('saida');
	client.subscribe('ocupacao');
	client.subscribe('desconhecidos');
	client.subscribe('autorizados');
});

client.on('message', function (topic, message) {
	if (topic === 'entrada') registrar_entrada(message.toString());
	else if (topic === 'saida') registrar_saida(message.toString());
	else if (topic === 'ocupacao') registrar_ocupacao(message.toString());
	else if (topic === 'desconhecidos') registrar_desconhecido(message.toString());
	else if (topic === 'autorizados') registrar_autorizados(message.toString());
});


/******************************************************************************/

// Criação das rotas e parse das informações no DB

var express = require('express');
var cors = require('cors');
var app = express();
app.use(cors());
const port = 3001;

/* Autoriza o último UID desconhecido */
app.get('/autorizados', (req, res) => {
	res.send('\"' + autorizados.toString() + '\"');
});

/* Autoriza o último UID desconhecido */
app.get('/autoriza', (req, res) => {
	res.sendStatus(200);
	// TODO: possível bug: possibilidade de uid_desconhecido != autorizado
	autorizados.push(' ' + uid_desconhecido);
	client.publish('autorizados', autorizados.toString(), { retain: true });
});

/* Não autoriza o último UID desconhecido */
app.get('/nao_autoriza', (req, res) => {
	res.sendStatus(200);
	uid_desconhecido = "";
});

/* Devolve o último UID desconhecido escaneado */
app.get('/desconhecido', (req, res) => {
	res.send('\"' + uid_desconhecido + '\"');
});

/* Devolve o número de ocupantes no momento */
app.get('/instantanea', (req, res) => {
	if (instantanea.length == 1 && instantanea[0] === "") res.send("0")
	else res.send(instantanea.length.toString());
});

/* Devolve a ocupação média por hora nos últimos sete dias */
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
	    		ocupacao_media[i] = Math.floor(ocupacao_total[i] / quantidade_de_valores[i]);
	    }
	    // res.send(JSON.stringify({ diaria: ocupacao_media}));
	    res.send(ocupacao_media);
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
	    		ocupacao_media[i] = Math.floor(ocupacao_total[i] / quantidade_de_valores[i]);
	    }
	    // res.send(JSON.stringify({ semanal: ocupacao_media}));
	    res.send(ocupacao_media);
	});
});

app.listen(port, () => {
  console.log(`Subscriber listening at http://localhost:${port}`)
});

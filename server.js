const express = require('express');
const bodyParser = require('body-parser');
const Aedes = require('aedes');
const cors = require('cors');
 
const app = express();
const aedes = Aedes();
const mqttServer = require('net').createServer(aedes.handle);
const mqttPort = 1883;

// Configuração dos middlewares
app.use(cors());
app.use(bodyParser.json());

mqttServer.listen(mqttPort, () => {
  console.log(`MQTT server is running on port ${mqttPort}`);
});

app.post('/send', (req, res) => {
    try{
    const mensagem = req.body.message;
    aedes.publish({topic:'esp32/data', payload: mensagem});
    res.status(200).send({message : "Mensagem publicada com sucesso!"});
}
catch (error) {
    console.error('Erro ao publicar mensagem:', error);
    res.status(500).send({ message: 'Erro ao publicar mensagem' });
  }
  
});

app.post('/echo', (req, res) => {
    const {email,senha} = req.body;
    console.log(`Email: ${email}, Senha: ${senha}`);
    res.send(req.body);
}
);

aedes.on('client', (client) => {
  console.log(`Client connected: ${client.id}`);
});

aedes.on('clientDisconnect', (client) => {
  console.log(`Client disconnected: ${client.id}`);
});

aedes.on('publish', (packet, client) => {
  if (client) {
    console.log(`Message from ${client.id}: ${packet.payload.toString()}`);
  } else {
    console.log(`Message from broker: ${packet.payload.toString()}`);
  }
});

//aedes.on('publish', async function (packet, client) {
//    console.log('Client \x1b[31m' + (client ? client.id : 'BROKER_') + '\x1b[0m published', packet.payload.toString(), 'on' , packet.topic, 'to broker', aedes.id);
    
//})



app.get('/', (req, res) => {
  res.send('Welcome to the MQTT server');
});

const port = 3000;
app.listen(port, () => {
  console.log(`HTTP server is running on port ${port}`);
});
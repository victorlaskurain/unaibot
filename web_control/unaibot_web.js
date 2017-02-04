'use strict';

var restify    = require('restify'),
    morgan     = require('morgan'),
    WsServer   = require('ws').Server,
    Promise    = require('promise'),
    UnaiBot    = require('./lib/unaibot'),
    defaults   = {
        'serial-port':  '/dev/ttyACM0',
        'http-basedir': '/opt/unaibot/share/',
        'http-port':    8080
    },
    options    = require('minimist')(process.argv.slice(2),
                                     {default: defaults}),
    serialPort = options['serial-port'];

function main() {
    var server      = restify.createServer({}),
        wsServer    = new WsServer({server: server.server}),
        unaiBot     = new UnaiBot(serialPort),
        handlerByType = {
            'PING!': handlePing,
            'SPEED2': handleSpeed2
        };
    function handlePing(msg) {
        return new Promise(function(resolve, reject) {
            resolve();
        });
    }
    function handleSpeed2(msg) {
        var values = msg.data;
        return unaiBot.setSpeed2(values[0], values[1], values[2], values[3]);
    }
    wsServer.on('connection', function connection(conn) {
        conn.on('message', function incoming(msgJson) {
            var msg     = JSON.parse(msgJson),
                msgType = msg.type,
                msgId   = msg.id,
                handler = handlerByType[msgType];
            if (handler) {
                handler(msg).then(function(data) {
                    conn.send(JSON.stringify({id:msgId, data: data}));
                });
            }
        });
    });
    server.use(morgan('dev', {format: 'dev'}));
    server.use(restify.queryParser());
    server.use(restify.bodyParser());
    server.get(/.*/, restify.serveStatic({
        directory: defaults['http-basedir'] + 'html',
        default: 'index.html',
        maxAge: 1
    }));
    server.listen(options['http-port']);
}

main();

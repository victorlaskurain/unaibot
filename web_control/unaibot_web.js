#!/usr/bin/nodejs
'use strict';
var restify    = require('restify'),
    morgan     = require('morgan'),
    WsServer   = require('ws').Server,
    Promise    = require('promise'),
    SerialPort = require('serialport');

function main() {
    var server      = restify.createServer({}),
        wsServer    = new WsServer({server: server.server}),
        handlerByType = {
            'PING!': handlePing,
            'SPEED2': handleSpeed2
        },
        // simulate serial port with:
        // socat PTY,link=tmp/vmodem0,rawer,wait-slave,crnl -
        serial      = new SerialPort('/home/victor/tmp/vmodem0', {
            baudRate: 9600,
            parser:   SerialPort.parsers.readline('\r\n')
        }),
        serialCbs = [];
    serial.on('open', function() {
        console.log('SERIAL PORT OPENED');
    });
    serial.on('data', function(serialData) {
        console.log(serialData);
        var cb = serialCbs.shift();
        if (cb) {
            cb(serialData);
        }
    });
    function handlePing(msg) {
        return new Promise(function(resolve, reject) {
            resolve();
        });
    }
    function handleSpeed2(msg) {
        console.log(['handleSpeed2', msg]);
        if (msg[0] < 0 || msg[0] > 3   ||
            msg[1] < 0 || msg[1] > 255 ||
            msg[3] < 0 || msg[3] > 3   ||
            msg[4] < 0 || msg[4] > 255) {
            return new Promise(function(resolve, reject) {
                resolve('ERROR');
            });
        }
        var serialMessage = 'SPEED2 ' + msg.data.map(Math.round)
                                                .map(toHex).join(' ');
        function toHex(n) {
            return '0x' + n.toString(16);
        }
        serial.write(serialMessage);
        return new Promise(function(resolve, reject) {
            serialCbs.push(resolve);
        });
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
        directory: 'html',
        default: 'index.html',
        maxAge: 1
    }));
    server.listen(8080);
}

main();

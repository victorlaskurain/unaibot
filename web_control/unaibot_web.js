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
        serial      = new SerialPort('vmodem0', {
            baudRate: 9600,
            parser:   SerialPort.parsers.readline('\r\n')
        }),
        serialCbs = [];
    serial.on('open', function() {
        handleSpeed2({data:[0, 0, 0, 0]});
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
        var values = msg.data;
        if (!values || values.length != 4 ||
            values[0] < 0 || values[0] > 3   ||
            values[1] < 0 || values[1] > 255 ||
            values[2] < 0 || values[2] > 3   ||
            values[3] < 0 || values[3] > 255) {
            return new Promise(function(resolve, reject) {
                resolve('ERROR');
            });
        }
        var serialMessage = 'SPEEDS2 ' + msg.data.map(Math.round)
                                                .map(toHex).join(' ') + '\r';
        function toHex(n) {
            var str = n.toString(16);
            if (str.length == 1) {
                return '0x0' + n.toString(16);
            } else {
                return '0x'  + n.toString(16);
            }
        }
        console.log(serialMessage);
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

'use strict';

var exec       = require('child_process').exec,
    http       = require('http'),
    restify    = require('restify'),
    morgan     = require('morgan'),
    WebSocket  = require('ws'),
    WsServer   = WebSocket.Server,
    Promise    = require('promise'),
    UnaiBot    = require('./lib/unaibot'),
    BotCam     = require('./lib/botcam'),
    url        = require('url'),
    defaults   = {
        'serial-port':       '/dev/ttyACM0',
        'http-basedir':      '/opt/unaibot/share/',
        'http-port':         8080,
        'cam-stream-port':   8081,
        'cam-stream-secret': 'supersecret'
    },
    options    = require('minimist')(process.argv.slice(2),
                                     {default: defaults}),
    serialPort = options['serial-port'];

function main() {
    var server      = restify.createServer({}),
        // the way to have different ws server in the same http server
        // is to creat them with noServer and then handle the upgrade
        // explicity for each required path.
        wsCmdServer = new WsServer({noServer: true}),
        wsCamServer = new WsServer({noServer: true}),
        streamServer,
        unaiBot     = new UnaiBot(serialPort),
        botCam      = new BotCam(broadcastVideo.bind(undefined, wsCamServer)),
        handlerByType = {
            'PING!':  handlePing,
            'SPEED2': handleSpeed2,
            'CAMON':  handleCamOn,
            'CAMOFF': handleCamOff
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
    function handleCamOn(msg) {
        console.log(['handleCamOn']);
        return new Promise(function(resolve, reject) {
            botCam.capture(true);
            resolve();
        });
    }
    function handleCamOff(msg) {
        console.log(['handleCamOff']);
        return new Promise(function(resolve, reject) {
            botCam.capture(false);
            resolve();
        });
    }
    wsCmdServer.on('connection', function initCmdConnection(conn) {
        console.log(['new cmd connection']);
        conn.on('message', function incoming(msgJson) {
            console.log(['new message', msgJson]);
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
    function broadcastVideo(ws, data) {
        ws.clients.forEach(function(client) {
		    if (client.readyState === WebSocket.OPEN) {
			    client.send(data);
		    }
        });
    }
    wsCamServer.on('connection', function initCamConnection(conn) {
        console.log(['new cam connection']);
    });
    server.on('upgrade', function httpServerUpgrade(request, socket, head) {
        const pathname = url.parse(request.url).pathname;
        console.log(['upgrade', pathname]);
        if (pathname === '/unaibot/cmd') {
            wsCmdServer.handleUpgrade(request, socket, head, function(ws) {
                wsCmdServer.emit('connection', ws);
            });
        } else if (pathname === '/unaibot/cam') {
            wsCamServer.handleUpgrade(request, socket, head, function (ws) {
                wsCamServer.emit('connection', ws);
            });
        } else {
            socket.destroy();
        }
    });
    server.use(morgan('dev', {format: 'dev'}));
    server.use(restify.queryParser());
    server.use(restify.bodyParser());
    server.get(/.*/, restify.serveStatic({
        directory: options['http-basedir'] + 'html',
        default: 'index.html',
        maxAge: 1
    }));
    server.listen(options['http-port']);
}

main();

'use strict';

var SerialPort = require('serialport');

function _toHex(n) {
    var str = n.toString(16);
    if (str.length == 1) {
        return '0x0' + n.toString(16);
    } else {
        return '0x'  + n.toString(16);
    }
}

function UnaiBot(serialPort, serialCfg) {
    if (!(this instanceof UnaiBot)) {
        return new UnaiBot(serialPort, serialCfg);
    }
    // simulate serial port with:
    // socat PTY,link=tmp/vmodem0,rawer,wait-slave,crnl -
    serialPort = serialPort || 'vmodem0';
    serialCfg  = serialCfg || {
        baudRate: 9600,
        parser:   SerialPort.parsers.readline('\r\n')
    };
    var serial      = new SerialPort(serialPort, serialCfg),
        serialQueue = [];
    serial.on('open', function() {
        // serial.once('data', _waitForReady);
    });
    serial.on('data', _serialDequeue);

    function _waitForReady(data) {
        serial.on('data', _serialDequeue);
        if (data.startsWith('READY')) {
            setSpeed2(0, 0, 0, 0);
            return;
        }
    }


    function _serialDequeue(serialData) {
        console.log('Arduino answer: ' + serialData);
        if (serialData.startsWith('READY')) {
            serialQueue.forEach(function(msg) {
                msg.cb('DROPPED');
            });
            serialQueue = [];
            setSpeed2(0, 0, 0, 0);
            return;
        }
        if (serialQueue.length == 0) {
            console.warn('no callback wainting for reply');
            return;
        }
        serialQueue[0].cb(serialData);
        serialQueue.shift();
        if (serialQueue.length) {
            console.log('Arduino command (a): ' + serialQueue[0].data);
            serial.write(serialQueue[0].data);
        }
    }

    function _serialEnqueue(msg) {
        return new Promise(function(resolve, reject) {
            serialQueue.push({
                data: msg,
                cb: resolve,
                err: reject
            });
            if (serialQueue.length == 1) {
                console.log('Arduino command (b): ' + msg);
                serial.write(msg);
            };
        });
    }

    function setSpeed2(d0, s0, d1, s1) {
        var values = [d0, s0, d1, s1],
            serialMessage;
        if (!values || values.length != 4 ||
            values[0] < 0 || values[0] > 3   ||
            values[1] < 0 || values[1] > 255 ||
            values[2] < 0 || values[2] > 3   ||
            values[3] < 0 || values[3] > 255) {
            return new Promise(function(resolve, reject) {
                resolve('INVALID PARAMETERS');
            });
        }
        serialMessage = 'SPEEDS2 ' +
            values.map(Math.round)
                  .map(_toHex).join(' ') + '\r';
        return _serialEnqueue(serialMessage);
    }

    this.setSpeed2 = setSpeed2;

    return this;
}

module.exports = UnaiBot;

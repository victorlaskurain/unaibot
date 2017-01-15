define(['jquery'],
function($) {
    console.log('BEGIN api');
    const MOTOR_IDLE     = 0x00,
          MOTOR_FORWARD  = 0x01,
          MOTOR_BACKWARD = 0x02,
          MOTOR_STOP     = 0x11,
          MOTOR_FIRST    = MOTOR_IDLE,
          MOTOR_LAST     = MOTOR_STOP;
    const MESSAGE_PING   = 'PING!',
          MESSAGE_SPEED2 = 'SPEED2';
    var server     = null,
        msgCounter = 0,
        promises   = {},
        m0Speed    = 0.0,
        m1Speed    = 0.0;

    function _discretize(f) {
        if (f < 0) {
            return -_discretizePositive(-f);
        } else {
            return  _discretizePositive(f);
        }
    }

    function _discretizePositive(f) {
        var steps = [
            [0.1, 0],
            [0.4, 127], //  50%
            [0.7, 189], //  75%
            [1.0, 255]  // 100%
        ], i;
        for (i = 0; i < steps.length; ++i) {
            if (f < steps[i][0]) {
                return steps[i][1];
            }
        }
        return 0;
    }

    function _makeMessagePromise(msgType, msgData) {
        var deferred = $.Deferred(),
            msg     = JSON.stringify({
                id:   msgCounter,
                type: msgType,
                data: msgData || null
            });
        console.log(['readyState', server.readyState]);
        if (server.readyState == WebSocket.OPEN) {
            promises[msgCounter++] = deferred;
            server.send(msg);
        } else {
            deferred.reject();
        }
        return deferred.promise();
    }

    /**
     * speed0 and speed1 must be floats [-1, 1].
     * +1 => full forward
     * -1 => full backward
     *  0 => stop
     */
    function setSpeed2(speed0, speed1) {
        var d0 = 0, s0 = 0, d1 = 0, s1 = 0;
        speed0 = _discretize(speed0);
        speed1 = _discretize(speed1);
        if (speed0 == m0Speed && speed1 == m1Speed) {
            return $.when('OK');
        }
        if (speed0 > 0) {
            d0 = MOTOR_FORWARD;
            s0 = speed0;
        } else if (speed0 < 0) {
            d0 = MOTOR_BACKWARD;
            s0 = -speed0;
        }
        if (speed1 > 0) {
            d1 = MOTOR_FORWARD;
            s1 = speed1;
        } else if (speed1 < 0) {
            d1 = MOTOR_BACKWARD;
            s1 = -speed1;
        }
        m0Speed = speed0;
        m1Speed = speed1;
        return setSpeed4(d0, s0, d1, s1);
    }

    function setSpeed4(dir0, speed0, dir1, speed1) {
        var msg     = [dir0, speed0, dir1, speed1],
            promise = $.Deferred();
        _makeMessagePromise(MESSAGE_SPEED2, msg).then(function(msg) {
            if (msg === 'OK') {
                promise.resolve(msg);
            } else {
                promise.reject(msg);
            }
        }, promise.reject.bind(promise));
        return promise;
    }

    function ping() {
        return _makeMessagePromise(MESSAGE_PING);
    }

    function _init() {
        server = new window.WebSocket('ws://' + window.location.host + '/unaibot');
        server.onerror = function(evt) {
            console.log(['WS ERROR', evt]);
        };
        server.onopen = function(evt) {
            console.log(['WS OPENED', evt]);
        };
        // reject all pending messages and try to reconnect
        server.onclose = function(evt) {
            var promiseId;
            for (promiseId in promises) {
                promises[promiseId].reject();
                delete promises[promiseId];
            }
            _init();
        };
        server.onmessage = function(evt) {
            var msg     = JSON.parse(evt.data),
                promise = promises[msg.id];
            if (!promise) {
                console.log(['UNEXPECTED DATA', msg]);
                return;
            }
            delete promises[msg.id];
            promise.resolve(msg.data);
        };
    }

    _init();

    console.log('END api');
    return {
        ping: ping,
        setSpeed2: setSpeed2,
        MOTOR_IDLE: MOTOR_IDLE,
        MOTOR_FORWARD: MOTOR_FORWARD,
        MOTOR_BACKWARD: MOTOR_BACKWARD,
        MOTOR_STOP: MOTOR_STOP
    };
});

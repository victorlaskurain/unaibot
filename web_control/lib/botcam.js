'use strict';

/**
 * Captures from a v4l device using ffmpeg as required by jsmpeg.
 *
 * :TODO: use Broadway https://github.com/mbebenita/Broadway to
 * require less bandwidth allowing for better quality on worse
 * connections. Would require changes to the client too.
 */

const spawn        = require('child_process').spawn;
const EventEmitter = require('events').EventEmitter;
const ffmpegCmdStr = 'ffmpeg -f v4l2 -framerate 25 -video_size 640x480 -i %VIDEODEV% -f mpegts -codec:v mpeg1video -s 640x480 -b:v 1000k -bf 0 -loglevel panic -';

function BotCam(videoDev) {
    if (!(this instanceof BotCam)) {
        return new BotCam(videoDev);
    }
    videoDev = videoDev || '/dev/video0';
    var self         = this,
        ffmpegCmd    = ffmpegCmdStr.split(' ')[0],
        ffmpegParams = ffmpegCmdStr
            .replace('%VIDEODEV%', videoDev)
            .split(' ')
            .slice(1),
        ffmpeg       = null;
    function _onFfmpegExit(code, signal) {
        self.emit('videoOff', 'videoOff');
    }
    function _signalVideoOn() {
        self.emit('videoOn', 'videOn');
    }
    function _signalVideoData(data) {
        self.emit('data', data);
    }
    function _nop(){}
    function capture(doCapture) {
        if (doCapture) {
            if (ffmpeg) {
                return;
            }
            ffmpeg = spawn(ffmpegCmd, ffmpegParams);
            ffmpeg.stderr.on('data', _nop);
            ffmpeg.stdout.on('data', _signalVideoData);
            ffmpeg.stdout.once('data', _signalVideoOn);
            ffmpeg.on('exit', _onFfmpegExit);
        } else {
            ffmpeg.kill();
            ffmpeg = null;
        }
    }
    function isVideoOn() {
        return ffmpeg != null;;
    }
    this.capture   = capture;
    this.isVideoOn = isVideoOn;
    return this;
}

BotCam.prototype = Object.create(EventEmitter.prototype);

module.exports = BotCam;

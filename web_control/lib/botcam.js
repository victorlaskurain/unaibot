'use strict';

/**
 * Captures from a v4l device using ffmpeg as required by jsmpeg.
 *
 * :TODO: use Broadway https://github.com/mbebenita/Broadway to
 * require less bandwidth allowing for better quality on worse
 * connections. Would require changes to the client too.
 */

const spawn = require('child_process').spawn;
const exec = require('child_process').exec;
const ffmpegCmdStr = 'ffmpeg -f v4l2 -framerate 25 -video_size 640x480 -i %VIDEODEV% -f mpegts -codec:v mpeg1video -s 640x480 -b:v 1000k -bf 0 -loglevel panic -';

function BotCam(onVideoData, videoDev) {
    if (!(this instanceof BotCam)) {
        return new BotCam(onVideoData);
    }
    videoDev = videoDev || '/dev/video0';
    var ffmpegCmd    = ffmpegCmdStr.split(' ')[0],
        ffmpegParams = ffmpegCmdStr
            .replace('%VIDEODEV%', videoDev)
            .split(' ')
            .slice(1),
        ffmpeg       = null;
    function _nop(){}
    function capture(doCapture) {
        if (doCapture) {
            if (ffmpeg) {
                return;
            }
            ffmpeg = spawn(ffmpegCmd, ffmpegParams);
            ffmpeg.stdout.on('data', onVideoData);
            ffmpeg.stderr.on('data', _nop);
        } else {
            var killall = exec('killall ffmpeg');
            ffmpeg = null;
        }
    }
    this.capture = capture;
    return this;
}

module.exports = BotCam;

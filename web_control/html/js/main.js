define(['api'],
function(api) {
window.botApi = api;
    var doc               = window.document,
        docEl             = doc.documentElement,
        requestFullScreen = docEl.requestFullscreen ||
                            docEl.mozRequestFullScreen ||
                            docEl.webkitRequestFullScreen ||
                            docEl.msRequestFullscreen,
        cancelFullScreen  = doc.exitFullscreen ||
                            doc.mozCancelFullScreen ||
                            doc.webkitExitFullscreen ||
                            doc.msExitFullscreen,
        left              = document.getElementById('left'),
        right             = document.getElementById('right'),
        lSpeed            = null,
        rSpeed            = null;

    window.document.documentElement.addEventListener('touchstart', function(evt) {
        switch (evt.target) {
        case left:
            lSpeed = 0;
            break;
        case right:
            rSpeed = 0;
            break;
        default:
            // do nothing
        }
    });

    window.document.documentElement.addEventListener('touchend', function(evt) {
        switch (evt.target) {
        case left:
            lSpeed = null;
            break;
        case right:
            rSpeed = null;
            break;
        default:
            // do nothing
        }
        api.setSpeed2(0, 0);
    });

    window.document.documentElement.addEventListener('touchmove', function(evt) {
        var i, height = left.clientHeight, speed;
        if (lSpeed !== null && rSpeed !== null) {
            for (i = 0; i < evt.touches.length; ++i) {
                speed = 1 - evt.touches[i].pageY / (height / 2);
                switch (evt.touches[i].target) {
                case left:
                    lSpeed = speed;
                    break;
                case right:
                    rSpeed = speed;
                    break;
                default:
                    // do nothing
                }
            }
            if (lSpeed !== null && rSpeed !== null) {
                api.setSpeed2(lSpeed, rSpeed);
            }
        }

    });
});

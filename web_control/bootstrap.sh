#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"
. setenv.sh
npm install restify morgan bower ws promise serialport minimist
bower install requirejs bootstrap
wget https://github.com/phoboslab/jsmpeg/raw/master/jsmpeg.min.js -O html/lib/jsmpeg.min.js
mkdir -p html/lib
test -h html/lib/jquery.js  || ln -s ../../bower_components/jquery/dist/jquery.min.js html/lib/jquery.js
test -h html/lib/require.js || ln -s ../../bower_components/requirejs/require.js      html/lib/require.js
test -h html/lib/bootstrap  || ln -s ../../bower_components/bootstrap/dist/           html/lib/bootstrap

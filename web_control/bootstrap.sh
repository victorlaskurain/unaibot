#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR"
. setenv.sh
npm install restify morgan bower ws promise serialport minimist
bower install requirejs bootstrap
mkdir -p html/lib
test -h html/lib/jquery.js  || ln -s ../../bower_components/jquery/dist/jquery.min.js html/lib/jquery.js
test -h html/lib/require.js || ln -s ../../bower_components/requirejs/require.js      html/lib/require.js
test -h html/lib/bootstrap  || ln -s ../../bower_components/bootstrap/dist/           html/lib/bootstrap

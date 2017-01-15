#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export PATH="$DIR":"$DIR"/node_modules/bower/bin:$PATH
alias node=nodejs

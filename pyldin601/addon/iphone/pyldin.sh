#!/bin/bash
DIR=`dirname $0`
cd ~
exec $DIR/pyldin -s 1 -t -d $DIR/share/pyldin

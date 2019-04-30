#!/bin/bash
# Qing.

ps -ef | grep vmp
echo "killall -9 tvmpssd"
killall -9 tvmpssd


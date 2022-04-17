#!/bin/bash

set -ex -o pipefail

[ $# = 0 ]

exec &> >(tee run.log)

make

trap 'ps -fH' EXIT  # при любом выходе из скрипта

(
  echo create 1 -1
  echo create 2 -1
  sleep 0.2

  echo create 11 1
  echo create 111 11
  echo create 12 1
  sleep 0.2

  ps -fH >&2

  echo exec 11 2 100 200
  sleep 0.2

  echo exec 21 2 100 200  # должно напечататься сообщение об ошибке
  sleep 0.2

  echo pingall
  sleep 0.2

  pkill -9 computer
  ps -fH >&2

  echo pingall
  sleep 0.2

  echo exec 11 2 100 200
  sleep 0.2
) | ./controller

echo "Controller exited with code $?"

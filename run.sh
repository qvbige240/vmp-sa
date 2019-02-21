#!/bin/bash


function current_dir {
  echo "$(cd "$(dirname $0)"; pwd)"
}

./bin/tvmpssd


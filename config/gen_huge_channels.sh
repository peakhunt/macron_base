#!/bin/bash

NUM_CHANNELS=5000
CNT=1

while [ $CNT -le $NUM_CHANNELS ]
do
  echo "{"
  echo " \"name\": \"test channel #$CNT\","
  echo " \"chnl_num\": $CNT,"
  echo '  "chnl_type": "digital",'

  MOD=$(expr $CNT % 2)
  if [ $MOD -eq 0 ]
  then
    echo '  "chnl_dir": "input"'
  else
    echo '  "chnl_dir": "output"'
  fi

  if [ $CNT -lt $NUM_CHANNELS ]
  then
    echo "},"
  else
    echo "}"
  fi
  CNT=$(expr $CNT + 1)
done

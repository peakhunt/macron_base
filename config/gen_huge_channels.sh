#!/bin/bash

NUM_CHANNELS=5000
CNT=1

while [ $CNT -le $NUM_CHANNELS ]
do
  echo "{"
  echo " \"name\": \"test channel #$CNT\","
  echo " \"chnl_num\": $CNT,"

  MOD=$(expr $CNT % 2)
  if [ $MOD -eq 0 ]
  then
    echo '  "chnl_dir": "input",'
  else
    echo '  "chnl_dir": "output",'
  fi

  MOD=$(expr $CNT % 4)
  if [ $MOD -eq 0 ]
  then
    echo '  "chnl_type": "analog",'
    echo '  "init_val": 0.0,'
    echo '  "failsafe_val": 0.0,'
    echo '  "min_val": 0.0,'
    echo '  "max_val": 100000.0,'

    echo '  "lookup_table": ['
    echo '    { "raw": 0,  "eng": 0     },'
    echo '    { "raw": 20, "eng": 20    }'
    echo '  ]'
  else
    echo '  "chnl_type": "digital",'
    echo '  "init_val": false,'
    echo '  "failsafe_val": false'
  fi

  if [ $CNT -lt $NUM_CHANNELS ]
  then
    echo "},"
  else
    echo "}"
  fi
  CNT=$(expr $CNT + 1)
done

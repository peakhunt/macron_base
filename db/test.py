#!/usr/bin/python

import random
import datetime
import sqlite3
import time
import random

conn = sqlite3.connect("logging_db.sq3")
cur = conn.cursor()

for i in range(0, 10000000):
  for j in range(1,4):
    print i,j
    row = [(j, i, 0)]
    cur.executemany("insert into channel_log (ch_num, time_stamp, eng_val) values (?, ?, ?)", row)


conn.commit()
conn.close()

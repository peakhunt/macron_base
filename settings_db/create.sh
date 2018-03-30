#!/bin/bash

SQLITE3="../src/sqlite-autoconf-3220000/sql3_install/bin/sqlite3"

${SQLITE3} -init ./settings_db.sql settings.db


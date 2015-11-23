#!/bin/bash


#grant all privileges on db.table to user_name@localhost identified by “your_pwd”;



PROJECT_ROOT_DIR=$(pwd)/..

SQL_DIR=$PROJECT_ROOT_DIR/sql

HOST=127.0.0.1

PORT=3306

USERNAME=water

PASSWORD=111111

DBNAME=waterDB

MYSQL_PWD=$PASSWORD mysql -u$USERNAME -h$HOST -P$PORT $DBNAME < $SQL_DIR/test.sql || exit 1;
MYSQL_PWD=$PASSWORD mysql -u$USERNAME -h$HOST -P$PORT $DBNAME < $SQL_DIR/alter_table.sql || exit 1;

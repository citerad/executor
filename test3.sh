#!/bin/bash

./monitor&
sleep 2

sudo pkill -SIGINT executor

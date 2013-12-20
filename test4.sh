#!/bin/bash

./monitor&
sleep 2

sudo pkill -SIGILL executor

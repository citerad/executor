#!/bin/bash

./monitor&
sleep 2

./requester fifo<com2.txt


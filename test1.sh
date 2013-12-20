#!/bin/bash

./monitor&
sleep 2

./requester fifo<com1.txt


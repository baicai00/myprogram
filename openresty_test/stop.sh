#!/bin/bash

sudo /usr/local/openresty/nginx/sbin/nginx -p `pwd`/ -c conf/nginx.conf -s stop

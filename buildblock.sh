#!/bin/bash
rm -r /home/bitnami/files
mkdir /home/bitnami/files
COUNTER=0
#while(($int<$1))
while [  $COUNTER -lt 500 ]; do
    dd if=/dev/zero of=/home/bitnami/files/$COUNTER bs=4K count=1
    let COUNTER=COUNTER+1
done

#!/bin/bash

if [ $# -gt 0 ]
then
	grep -Rn "$1"|grep "$1"
else
	echo 'Veuillez indiquer une regex'
fi

#!/bin/bash

bytes=$(wc -c < out.txt)
file=$(<out.txt)

for ((i=0; i<bytes; i++)); do
	echo ${file:$i:1}

	if [ ${file:$i:1} == '1' ] 
	then
 		#setleds -D +caps < /dev/console
 		echo 1 > /sys/class/leds/ath9k-phy0/brightness
		sleep 1
	else
		#setleds -D -caps < /dev/console
		echo 0 > /sys/class/leds/ath9k-phy0/brightness
		sleep 1
	fi
done
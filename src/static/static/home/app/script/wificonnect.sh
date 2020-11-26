ifconfig wlan0 up
sleep 1
#/home/base/tools/wpa_supplicant -c/tmp/wpa_supplicant.conf -g/tmp/wpa_supplicant-global -iwlan0 -B;

if [ -f /tmp/sd/wpa_supplicant.conf ] || [ -f /tmp/wpa_supplicant2.conf ]; then
	/bin/awk -f /home/app/script/wpa_combine.awk /tmp/sd/wpa_supplicant.conf /tmp/wpa_supplicant.conf > /tmp/wpa_supplicant2.conf;
	/home/base/tools/wpa_supplicant -c/tmp/wpa_supplicant2.conf -g/var/run/wpa_supplicant-global -Dnl80211 -iwlan0 -B;
else
	/home/base/tools/wpa_supplicant -c/tmp/wpa_supplicant.conf -g/var/run/wpa_supplicant-global -Dnl80211 -iwlan0 -B;
fi

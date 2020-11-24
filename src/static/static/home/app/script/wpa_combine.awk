#/bin/awk

BEGIN {
	begin_network = "false";
	networks = "";
	interface = "";
	ap_scan = "";
}

/^network=/ {
	begin_network = "true";
}

/^ctrl_interface=/ {
	interface = $0;
}

/^ap_scan=/ {
	ap_scan = $0;
}

/\}/ {
	networks = networks $0 "\n";
	begin_network = "false";
}

{
	if ( begin_network == "true" ) networks = networks $0 "\n";
}

END {
	if (interface == "") {
		print "ctrl_interface=/var/run/wpa_supplicant\n";
	} else {
		print interface;
	}

	if (ap_scan == "") {
		print "ap_scan=1\n"; 
	} else {
		print ap_scan;
	}
	
	print networks; # print network last
}


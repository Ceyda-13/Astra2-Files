import app
import localeInfo
app.ServerName = None

SRVS = {
	1 : {
		"name":"Astra2",
		"HOST":"34.91.215.127",
		"AUTH":30001,
		"CH1":30003,
		"CH2":30005,
		"CH3":30007,
		"CH4":30009,
		"CH5":30011,
		"CH6":30013,
	},
	2 : {
		"name":"Astra2 - Beta",
		"HOST":"192.168.1.1",
		"AUTH":30001,
		"CH1":30003,
		"CH2":30005,
		"CH3":30007,
		"CH4":30009,
		"CH5":30011,
		"CH6":30013,
	}
}

STATE_NONE = "OFF"

STATE_DICT = {
	0 : "OFF",
	1 : "NORM",
	2 : "BUSY",
	3 : "FULL"
}

SERVER1_CHANNEL_DICT = {
	1:{"key":11,"name":"Kanal 1","ip":SRVS[1]["HOST"],"tcp_port":SRVS[1]["CH1"],"udp_port":SRVS[1]["CH1"],"state":STATE_NONE,},
	2:{"key":12,"name":"Kanal 2","ip":SRVS[1]["HOST"],"tcp_port":SRVS[1]["CH2"],"udp_port":SRVS[1]["CH2"],"state":STATE_NONE,},
	3:{"key":13,"name":"Kanal 3","ip":SRVS[1]["HOST"],"tcp_port":SRVS[1]["CH3"],"udp_port":SRVS[1]["CH3"],"state":STATE_NONE,},
	4:{"key":14,"name":"Kanal 4","ip":SRVS[1]["HOST"],"tcp_port":SRVS[1]["CH4"],"udp_port":SRVS[1]["CH4"],"state":STATE_NONE,},
	5:{"key":15,"name":"Kanal 5","ip":SRVS[1]["HOST"],"tcp_port":SRVS[1]["CH5"],"udp_port":SRVS[1]["CH5"],"state":STATE_NONE,},
	6:{"key":16,"name":"Kanal 6","ip":SRVS[1]["HOST"],"tcp_port":SRVS[1]["CH6"],"udp_port":SRVS[1]["CH6"],"state":STATE_NONE,},
}

SERVER2_CHANNEL_DICT = {
	1:{"key":21,"name":"Kanal 1","ip":SRVS[2]["HOST"],"tcp_port":SRVS[2]["CH1"],"udp_port":SRVS[2]["CH1"],"state":STATE_NONE,},
	2:{"key":22,"name":"Kanal 2","ip":SRVS[2]["HOST"],"tcp_port":SRVS[2]["CH2"],"udp_port":SRVS[2]["CH2"],"state":STATE_NONE,},
	3:{"key":23,"name":"Kanal 3","ip":SRVS[2]["HOST"],"tcp_port":SRVS[2]["CH3"],"udp_port":SRVS[2]["CH3"],"state":STATE_NONE,},
	4:{"key":24,"name":"Kanal 4","ip":SRVS[2]["HOST"],"tcp_port":SRVS[2]["CH4"],"udp_port":SRVS[2]["CH4"],"state":STATE_NONE,},
	5:{"key":25,"name":"Kanal 5","ip":SRVS[2]["HOST"],"tcp_port":SRVS[2]["CH5"],"udp_port":SRVS[2]["CH5"],"state":STATE_NONE,},
	6:{"key":26,"name":"Kanal 6","ip":SRVS[2]["HOST"],"tcp_port":SRVS[2]["CH6"],"udp_port":SRVS[2]["CH6"],"state":STATE_NONE,},
}

REGION_NAME_DICT = {
	0 : SRVS[1]["name"],
	1 : SRVS[2]["name"],
}

REGION_AUTH_SERVER_DICT = {
	0 : {
		1 : { "ip":SRVS[1]["HOST"], "port":SRVS[1]["AUTH"], },
		2 : { "ip":SRVS[2]["HOST"], "port":SRVS[2]["AUTH"], },
	}
}

REGION_DICT = {
	0 : {
		1 : { "name" : SRVS[1]["name"], "channel" : SERVER1_CHANNEL_DICT, },
		2 : { "name" : SRVS[2]["name"], "channel" : SERVER2_CHANNEL_DICT, },
	},
}

MARKADDR_DICT = {
	10 : { "ip" : SRVS[1]["HOST"], "tcp_port" : SRVS[1]["CH1"], "mark" : "10.tga", "symbol_path" : "10", },
	20 : { "ip" : SRVS[2]["HOST"], "tcp_port" : SRVS[2]["CH1"], "mark" : "10.tga", "symbol_path" : "10", },
}


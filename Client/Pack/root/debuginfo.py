g_isDebugMode=0

def SetDebugMode(isDebugMode):
	global g_isDebugMode
	g_isDebugMode=isDebugMode

def IsDebugMode():
	import player
	global g_isDebugMode
	return g_isDebugMode and player.IsGameMaster()


#include "stdafx.h"
#include "char.h"

TJobInitialPoints JobInitialPoints[JOB_MAX_NUM] =
{
	{   6,  4,  3,  3,  600,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },
	{   4,  3,  6,  3,  650,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },
	{   5,  3,  3,  5,  650,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },
	{   3,  4,  3,  6,  700,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	{   2,  6,  6,  2,  600,   200,     40,    20,    44, 44,     22, 22,     800,      5,      1, 3  },
#endif
};

const TMobRankStat MobRankStats[MOB_RANK_MAX_NUM] =
{
	{  20,  },
	{  20,  },
	{  25,  },
	{  30,  },
	{  50,  },
	{ 100,  }
};

TBattleTypeStat BattleTypeStats[BATTLE_TYPE_MAX_NUM] =
{
	{	  0,	  0,	  0,	-10	},
	{	 10,	-20,	-10,	-15	},
	{	 -5,	 -5,	 10,	 10	},
	{	  0,	  0,	  0,	  0	},
	{	 10,	-10,	  0,	-15	},
	{	-10,	 10,	-10,	  0	},
	{	 20,	-20,	  0,	-10	},
	{	-20,	 20,	-10,	  0	},
};

const uint32_t * exp_table = nullptr;

const uint32_t exp_table_common[PLAYER_MAX_LEVEL_CONST + 1] = // @correction001 PLAYER_EXP_TABLE_MAX -> PLAYER_MAX_LEVEL_CONST
{
	0,
	300,
	800,
	1500,
	2500,
	4300,
	7200,
	11000,
	17000,
	24000,
	33000,
	43000,
	58000,
	76000,
	100000,
	130000,
	169000,
	219000,
	283000,
	365000,
	472000,
	610000,
	705000,
	813000,
	937000,
	1077000,
	1237000,
	1418000,
	1624000,
	1857000,
	2122000,
	2421000,
	2761000,
	3145000,
	3580000,
	4073000,
	4632000,
	5194000,
	5717000,
	6264000,
	6837000,
	7600000,
	8274000,
	8990000,
	9753000,
	10560000,
	11410000,
	12320000,
	13270000,
	14280000,
	15340000,
	16870000,
	18960000,
	19980000,
	21420000,
	22930000,
	24530000,
	26200000,
	27960000,
	29800000,
	32780000,
	36060000,
	39670000,
	43640000,
	48000000,
	52800000,
	58080000,
	63890000,
	70280000,
	77310000,
	85040000,
	93540000,
	102900000,
	113200000,
	124500000,
	137000000,
	150700000,
	165700000,
	236990000,
	260650000,
	286780000,
	315380000,
	346970000,
	381680000,
	419770000,
	461760000,
	508040000,
	558740000,
	614640000,
	676130000,
	743730000,
	1041222000,
	1145344200,
	1259878620,
	1385866482,
	1524453130,
	1676898443,
	1844588288,
	1929047116,		
	2000000000,
	2020000000,
	2040000000,
	2060000000,
	2080000000,
	2090000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2100000000,
	2251500000,
	2253000000,
	2254500000,
	2256000000,
	2257500000,
	2259000000,
	2260500000,
	2262000000,
	2263500000,
	2265000000,
	2266500000,
	2268000000,
	2269500000,
	2271000000,
	2272500000,
	2274000000,
	2275500000,
	2277000000,
	2278500000,
	2280000000,
	2281800000,
	2283600000,
	2285400000,
	2287200000,
	2289000000,
	2290800000,
	2292600000,
	2294400000,
	2296200000,
	2298000000,
	2299800000,
	2301600000,
	2303400000,
	2305200000,
	2307000000,
	2308800000,
	2310600000,
	2312400000,
	2314200000,
	2316000000,
	2317800000,
	2319600000,
	2321400000,
	2323200000,
	2325000000,
	2326800000,
	2328600000,
	2330400000,
	2332200000,
	2334000000,
	2335800000,
	2337600000,
	2339400000,
	2341200000,
	2343000000,
	2344800000,
	2346600000,
	2348400000,
	2350200000,
	2352000000,
	2353800000,
	2355600000,
	2357400000,
	2359200000,
	2361000000,
	2362800000,
	2364600000,
	2366400000,
	2368200000,
	2370000000,
	2371800000,
	2373600000,
	2375400000,
	2377200000,
	2379000000,
	2380800000,
	2382600000,
	2384400000,
	2386200000,
	2388000000,
	2389800000,
	2391600000,
	2393400000,
	2395200000,
	2397000000,
	2398800000,
	2400600000,
	2402400000,
	2404200000,
	2406000000,
	2407800000,
	2409600000,
	2411400000,
	2413200000,
	2415000000,
	2416800000,
	2418600000,
	2420400000,
	2422200000,
	2424000000,
	2425800000,
	2427600000,
	2429400000,
	2431200000,
	2433000000,
	2434800000,
	2436600000,
	2438400000,
	2440200000,
	2442000000,
	2443800000,
	2445600000,
	2447400000,
	2449200000,
	2451000000,
	2452800000,
	2454600000,
	2456400000,
	2458200000,
	2460000000,
	2461800000,
	2463600000,
	2465400000,
	2467200000,
	2469000000,
	2470800000,
	2472600000,
	2474400000,
	2478000000,
	2500000000,
};

const int32_t * aiPercentByDeltaLev = nullptr;
const int32_t * aiPercentByDeltaLevForBoss = nullptr;

const int32_t aiPercentByDeltaLevForBoss_euckr[MAX_EXP_DELTA_OF_LEV] =
{
	1,
	3,
	5,
	7,
	15,
	30,
	60,
	90,
	91,
	92,
	93,
	94,
	95,
	97,
	99,
	100,
	105,
	110,
	115,
	120,
	125,
	130,
	135,
	140,
	145,
	150,
	155,
	160,
	165,
	170,
	180
};



const int32_t aiPercentByDeltaLev_euckr[MAX_EXP_DELTA_OF_LEV] =
{
	1,
	5,
	10,
	20,
	30,
	50,
	70,
	80,
	85,
	90,
	92,
	94,
	96,
	98,
	100,
	100,
	105,
	110,
	115,
	120,
	125,
	130,
	135,
	140,
	145,
	150,
	155,
	160,
	165,
	170,
	180,
};

const uint32_t party_exp_distribute_table[PLAYER_EXP_TABLE_MAX + 1] = // @correction001 PLAYER_MAX_LEVEL_CONST -> PLAYER_EXP_TABLE_MAX
{
	0,
	10,		10,		10,		10,		15,		15,		20,		25,		30,		40,
	50,		60,		80,		100,	120,	140,	160,	184,	210,	240,
	270,	300,	330,	360,	390,	420,	450,	480,	510,	550,
	600,	640,	700,	760,	820,	880,	940,	1000,	1100,	1180,
	1260,	1320,	1380,	1440,	1500,	1560,	1620,	1680,	1740,	1800,
	1860,	1920,	2000,	2100,	2200,	2300,	2450,	2600,	2750,	2900,
	3050,	3200,	3350,	3500,	3650,	3800,	3950,	4100,	4250,	4400,
	4600,	4800,	5000,	5200,	5400,	5600,	5800,	6000,	6200,	6400,
	6600,	6900,	7100,	7300,	7600,	7800,	8000,	8300,	8500,	8800,
	9000,	9000,	9000,	9000,	9000,	9000,	9000,	9000,	9000,	9000,
	10000,	10000,	10000,	10000,	10000,	10000,	10000,	10000,	10000,	10000,
	12000,	12000,	12000,	12000,	12000,	12000,	12000,	12000,	12000,	12000,
};

Coord aArroundCoords[ARROUND_COORD_MAX_NUM] =
{
	{	     0,	      0	    },
	{        0,      50     },
	{       35,     35      },
	{       50,     -0      },
	{       35,     -35     },
	{       0,      -50     },
	{       -35,    -35     },
	{       -50,    0       },
	{       -35,    35      },
	{       0,      100     },
	{       71,     71      },
	{       100,    -0      },
	{       71,     -71     },
	{       0,      -100    },
	{       -71,    -71     },
	{       -100,   0       },
	{       -71,    71      },
	{       0,      150     },
	{       106,    106     },
	{       150,    -0      },
	{       106,    -106    },
	{       0,      -150    },
	{       -106,   -106    },
	{       -150,   0       },
	{       -106,   106     },
	{       0,      200     },
	{       141,    141     },
	{       200,    -0      },
	{       141,    -141    },
	{       0,      -200    },
	{       -141,   -141    },
	{       -200,   0       },
	{       -141,   141     },
	{       0,      250     },
	{       177,    177     },
	{       250,    -0      },
	{       177,    -177    },
	{       0,      -250    },
	{       -177,   -177    },
	{       -250,   0       },
	{       -177,   177     },
	{       0,      300     },
	{       212,    212     },
	{       300,    -0      },
	{       212,    -212    },
	{       0,      -300    },
	{       -212,   -212    },
	{       -300,   0       },
	{       -212,   212     },
	{       0,      350     },
	{       247,    247     },
	{       350,    -0      },
	{       247,    -247    },
	{       0,      -350    },
	{       -247,   -247    },
	{       -350,   0       },
	{       -247,   247     },
	{       0,      400     },
	{       283,    283     },
	{       400,    -0      },
	{       283,    -283    },
	{       0,      -400    },
	{       -283,   -283    },
	{       -400,   0       },
	{       -283,   283     },
	{       0,      450     },
	{       318,    318     },
	{       450,    -0      },
	{       318,    -318    },
	{       0,      -450    },
	{       -318,   -318    },
	{       -450,   0       },
	{       -318,   318     },
	{       0,      500     },
	{       354,    354     },
	{       500,    -0      },
	{       354,    -354    },
	{       0,      -500    },
	{       -354,   -354    },
	{       -500,   0       },
	{       -354,   354     },
	{       0,      550     },
	{       389,    389     },
	{       550,    -0      },
	{       389,    -389    },
	{       0,      -550    },
	{       -389,   -389    },
	{       -550,   0       },
	{       -389,   389     },
	{       0,      600     },
	{       424,    424     },
	{       600,    -0      },
	{       424,    -424    },
	{       0,      -600    },
	{       -424,   -424    },
	{       -600,   0       },
	{       -424,   424     },
	{       0,      650     },
	{       460,    460     },
	{       650,    -0      },
	{       460,    -460    },
	{       0,      -650    },
	{       -460,   -460    },
	{       -650,   0       },
	{       -460,   460     },
	{       0,      700     },
	{       495,    495     },
	{       700,    -0      },
	{       495,    -495    },
	{       0,      -700    },
	{       -495,   -495    },
	{       -700,   0       },
	{       -495,   495     },
	{       0,      750     },
	{       530,    530     },
	{       750,    -0      },
	{       530,    -530    },
	{       0,      -750    },
	{       -530,   -530    },
	{       -750,   0       },
	{       -530,   530     },
	{       0,      800     },
	{       566,    566     },
	{       800,    -0      },
	{       566,    -566    },
	{       0,      -800    },
	{       -566,   -566    },
	{       -800,   0       },
	{       -566,   566     },
	{       0,      850     },
	{       601,    601     },
	{       850,    -0      },
	{       601,    -601    },
	{       0,      -850    },
	{       -601,   -601    },
	{       -850,   0       },
	{       -601,   601     },
	{       0,      900     },
	{       636,    636     },
	{       900,    -0      },
	{       636,    -636    },
	{       0,      -900    },
	{       -636,   -636    },
	{       -900,   0       },
	{       -636,   636     },
	{       0,      950     },
	{       672,    672     },
	{       950,    -0      },
	{       672,    -672    },
	{       0,      -950    },
	{       -672,   -672    },
	{       -950,   0       },
	{       -672,   672     },
	{       0,      1000    },
	{       707,    707     },
	{       1000,   -0      },
	{       707,    -707    },
	{       0,      -1000   },
	{       -707,   -707    },
	{       -1000,  0       },
	{       -707,   707     },
};

const uint32_t guild_exp_table[GUILD_MAX_LEVEL+1] =
{
	0,
	15000UL,
	45000UL,
	90000UL,
	160000UL,
	235000UL,
	325000UL,
	430000UL,
	550000UL,
	685000UL,
	835000UL,
	1000000UL,
	1500000UL,
	2100000UL,
	2800000UL,
	3600000UL,
	4500000UL,
	6500000UL,
	8000000UL,
	10000000UL,
	42000000UL
};

const uint32_t guild_exp_table2[GUILD_MAX_LEVEL+1] =
{
	0,
	6000UL,
	18000UL,
	36000UL,
	64000UL,
	94000UL,
	130000UL,
	172000UL,
	220000UL,
	274000UL,
	334000UL,
	400000UL,
	600000UL,
	840000UL,
	1120000UL,
	1440000UL,
	1800000UL,
	2600000UL,
	3200000UL,
	4000000UL,
	16800000UL
};

const int32_t aiMobEnchantApplyIdx[MOB_ENCHANTS_MAX_NUM] =
{
	APPLY_CURSE_PCT,
	APPLY_SLOW_PCT,
	APPLY_POISON_PCT,
	APPLY_STUN_PCT,
	APPLY_CRITICAL_PCT,
	APPLY_PENETRATE_PCT,
// #if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON) // @correction190
	// APPLY_BLEEDING_PCT,
// #endif
};

const int32_t aiMobResistsApplyIdx[MOB_RESISTS_MAX_NUM] =
{
	APPLY_RESIST_SWORD,
	APPLY_RESIST_TWOHAND,
	APPLY_RESIST_DAGGER,
	APPLY_RESIST_BELL,
	APPLY_RESIST_FAN,
	APPLY_RESIST_BOW,
	APPLY_RESIST_FIRE,
	APPLY_RESIST_ELEC,
	APPLY_RESIST_MAGIC,
	APPLY_RESIST_WIND,
	APPLY_POISON_REDUCE,
// #if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_CLAW_AS_DAGGER) // @correction190
	APPLY_RESIST_CLAW,
// #endif
// #if defined(ENABLE_WOLFMAN_CHARACTER) && !defined(USE_MOB_BLEEDING_AS_POISON) // @correction190
	// APPLY_BLEEDING_REDUCE,
// #endif
};

const int32_t aiSocketPercentByQty[5][4] =
{
	{  0,  0,  0,  0 },
	{  3,  0,  0,  0 },
	{ 10,  1,  0,  0 },
	{ 15, 10,  1,  0 },
	{ 20, 15, 10,  1 }
};

const int32_t aiWeaponSocketQty[WEAPON_NUM_TYPES] =
{
	3,
	3,
	3,
	3,
	3,
	3,
	0,
	0,
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	3, // WEAPON_CLAW
#endif
#ifdef __QUIVER__ // @correction203
	0, // WEAPON_QUIVER
#endif
};

const int32_t aiArmorSocketQty[ARMOR_NUM_TYPES] =
{
	3,
	1,
	1,
	0,
	0,
	0
};

TItemAttrMap g_map_itemAttr;
TItemAttrMap g_map_itemRare;

const TApplyInfo aApplyInfo[MAX_APPLY_NUM] =
{
	{ POINT_NONE,			},
	{ POINT_MAX_HP,		        },
	{ POINT_MAX_SP,		        },
	{ POINT_HT,			        },
	{ POINT_IQ,			        },
	{ POINT_ST,			        },
	{ POINT_DX,			        },
	{ POINT_ATT_SPEED,		        },
	{ POINT_MOV_SPEED,		        },
	{ POINT_CASTING_SPEED,	        },
	{ POINT_HP_REGEN,			},
	{ POINT_SP_REGEN,			},
	{ POINT_POISON_PCT,		        },
	{ POINT_STUN_PCT,		        },
	{ POINT_SLOW_PCT,		        },
	{ POINT_CRITICAL_PCT,		},
	{ POINT_PENETRATE_PCT,	        },
	{ POINT_ATTBONUS_HUMAN,	        },
	{ POINT_ATTBONUS_ANIMAL,	        },
	{ POINT_ATTBONUS_ORC,		},
	{ POINT_ATTBONUS_MILGYO,	        },
	{ POINT_ATTBONUS_UNDEAD,	        },
	{ POINT_ATTBONUS_DEVIL,	        },
	{ POINT_STEAL_HP,		        },
	{ POINT_STEAL_SP,		        },
	{ POINT_MANA_BURN_PCT,	        },
	{ POINT_DAMAGE_SP_RECOVER,	        },
	{ POINT_BLOCK,		        },
	{ POINT_DODGE,		        },
	{ POINT_RESIST_SWORD,		},
	{ POINT_RESIST_TWOHAND,	        },
	{ POINT_RESIST_DAGGER,	        },
	{ POINT_RESIST_BELL,		},
	{ POINT_RESIST_FAN,		        },
	{ POINT_RESIST_BOW,		        },
	{ POINT_RESIST_FIRE,		},
	{ POINT_RESIST_ELEC,		},
	{ POINT_RESIST_MAGIC,		},
	{ POINT_RESIST_WIND,		},
	{ POINT_REFLECT_MELEE,	        },
	{ POINT_REFLECT_CURSE,	        },
	{ POINT_POISON_REDUCE,	        },
	{ POINT_KILL_SP_RECOVER,	        },
	{ POINT_EXP_DOUBLE_BONUS,	        },
	{ POINT_GOLD_DOUBLE_BONUS,	        },
	{ POINT_ITEM_DROP_BONUS,	        },
	{ POINT_POTION_BONUS,		},
	{ POINT_KILL_HP_RECOVERY,	        },
	{ POINT_IMMUNE_STUN,		},
	{ POINT_IMMUNE_SLOW,		},
	{ POINT_IMMUNE_FALL,		},
	{ POINT_NONE,			},
	{ POINT_BOW_DISTANCE,		},
	{ POINT_ATT_GRADE_BONUS,	        },
	{ POINT_DEF_GRADE_BONUS,	        },
	{ POINT_MAGIC_ATT_GRADE_BONUS,      },
	{ POINT_MAGIC_DEF_GRADE_BONUS,      },
	{ POINT_CURSE_PCT,			},
	{ POINT_MAX_STAMINA			},
	{ POINT_ATTBONUS_WARRIOR		},
	{ POINT_ATTBONUS_ASSASSIN		},
	{ POINT_ATTBONUS_SURA		},
	{ POINT_ATTBONUS_SHAMAN		},
	{ POINT_ATTBONUS_MONSTER		},
	{ POINT_ATT_BONUS		},
	{ POINT_MALL_DEFBONUS		},
	{ POINT_MALL_EXPBONUS		},
	{ POINT_MALL_ITEMBONUS		},
	{ POINT_MALL_GOLDBONUS		},
	{ POINT_MAX_HP_PCT			},
	{ POINT_MAX_SP_PCT			},
	{ POINT_SKILL_DAMAGE_BONUS		},
	{ POINT_NORMAL_HIT_DAMAGE_BONUS	},

	{ POINT_SKILL_DEFEND_BONUS		},
	{ POINT_NORMAL_HIT_DEFEND_BONUS	},

	{ POINT_PC_BANG_EXP_BONUS	},
	{ POINT_PC_BANG_DROP_BONUS	},

	{ POINT_NONE,		},

	{ POINT_RESIST_WARRIOR,		},
	{ POINT_RESIST_ASSASSIN,	},
	{ POINT_RESIST_SURA,		},
	{ POINT_RESIST_SHAMAN,		},
	{ POINT_ENERGY				},
	{ POINT_DEF_GRADE			},
	{ POINT_COSTUME_ATTR_BONUS	},
	{ POINT_MAGIC_ATT_BONUS_PER },
	{ POINT_MELEE_MAGIC_ATT_BONUS_PER		},
	{ POINT_RESIST_ICE,			},
	{ POINT_RESIST_EARTH,		},
	{ POINT_RESIST_DARK,		},
	{ POINT_RESIST_CRITICAL,		},
	{ POINT_RESIST_PENETRATE,		},
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	{ POINT_BLEEDING_REDUCE,		},
	{ POINT_BLEEDING_PCT,			},
	{ POINT_ATTBONUS_WOLFMAN,		},
	{ POINT_RESIST_WOLFMAN,			},
	{ POINT_RESIST_CLAW,			},
#endif
#ifdef __ACCE_SYSTEM__ // @correction191
	{ POINT_ACCEDRAIN_RATE,		},
#endif
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
	{ POINT_RESIST_MAGIC_REDUCTION, }, // APPLY_RESIST_MAGIC_REDUCTION 98
#endif
#ifdef __ELEMENT_ADD__
	{ POINT_ENCHANT_ELECT, },	// APPLY_ENCHANT_ELECT	99
	{ POINT_ENCHANT_FIRE, },	// APPLY_ENCHANT_FIRE 100
	{ POINT_ENCHANT_ICE, },	// APPLY_ENCHANT_ICE 101
	{ POINT_ENCHANT_WIND, },	// APPLY_ENCHANT_WIND 102
	{ POINT_ENCHANT_EARTH, },	// APPLY_ENCHANT_EARTH 103
	{ POINT_ENCHANT_DARK, },	// APPLY_ENCHANT_DARK 104
	{ POINT_ATTBONUS_CZ, },	// APPLY_ATTBONUS_CZ 105
	{ POINT_ATTBONUS_INSECT, },	// APPLY_ATTBONUS_INSECT 106
	{ POINT_ATTBONUS_DESERT, },	// APPLY_ATTBONUS_DESERT 107
#endif
#ifdef __PENDANT__
	{ POINT_ATTBONUS_SWORD, },	// APPLY_ATTBONUS_SWORD 108
	{ POINT_ATTBONUS_TWOHAND, },	// APPLY_ATTBONUS_TWOHAND 109
	{ POINT_ATTBONUS_DAGGER, },	// APPLY_ATTBONUS_DAGGER 110
	{ POINT_ATTBONUS_BELL, },	// APPLY_ATTBONUS_BELL 111
	{ POINT_ATTBONUS_FAN, },	// APPLY_ATTBONUS_FAN 112
	{ POINT_ATTBONUS_BOW, },	// APPLY_ATTBONUS_BOW 113
#ifdef ENABLE_WOLFMAN_CHARACTER
	{ POINT_ATTBONUS_CLAW, },	// APPLY_ATTBONUS_CLAW 114
#endif
	{ POINT_RESIST_HUMAN, },	// APPLY_RESIST_HUMAN 115
	{ POINT_RESIST_MOUNT_FALL, },	// APPLY_RESIST_MOUNT_FALL 116
#endif
#ifdef __NEW_BONUSES__
	{ POINT_ATTBONUS_STONE },
	{ POINT_ATTBONUS_BOSS },
#endif
#ifdef __ELEMENT_ADD__
	{ POINT_ENCHANT_AVATAR },
#endif
};

const int32_t aiItemMagicAttributePercentHigh[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
	30, 40, 20, 8, 2
};

const int32_t aiItemMagicAttributePercentLow[ITEM_ATTRIBUTE_MAX_LEVEL] =
{
	50, 40, 10, 0, 0
};

const int32_t aiItemAttributeAddPercent[ITEM_ATTRIBUTE_MAX_NUM] =
{
	100, 80, 60, 50, 30, 0, 0,
};

const int32_t aiExpLossPercents[PLAYER_EXP_TABLE_MAX + 1] =
{
	0,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 4,
	4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	4, 4, 4, 4, 4, 4, 4, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	3, 3, 3, 3, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

const int32_t aiSkillBookCountForLevelUp[10] =
{
	3, 3, 3, 3, 3, 4, 4, 5, 5, 6
};

const int32_t aiGrandMasterSkillBookCountForLevelUp[10] =
{
	3, 3, 5, 5, 7, 7, 10, 10, 10, 20,
};

const int32_t aiGrandMasterSkillBookMinCount[10] =
{
	1, 1, 1, 2,  2,  3,  3,  4,  5,  6
};

const int32_t aiGrandMasterSkillBookMaxCount[10] =
{
	5,  7,  9, 11, 13, 15, 20, 25, 30, 35
};

const int32_t CHN_aiPartyBonusExpPercentByMemberCount[9] =
{
	0, 0, 12, 18, 26, 40, 53, 70, 100
};


const int32_t KOR_aiPartyBonusExpPercentByMemberCount[9] =
{
	0,
	0,
	30,
	60,
	75,
	90,
	105,
	110,
	140,
};

const int32_t KOR_aiUniqueItemPartyBonusExpPercentByMemberCount[9] =
{
	0,
	0,
	15*2,
	14*3,
	13*4,
	12*5,
	11*6,
	10*7,
	10*8,
};

const int32_t * aiChainLightningCountBySkillLevel = nullptr;

const int32_t aiChainLightningCountBySkillLevel_euckr[SKILL_MAX_LEVEL+1] =
{
	0,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	2,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	3,
	4,
	4,
	4,
	4,
	4,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
	5,
};

const SStoneDropInfo aStoneDrop[STONE_INFO_MAX_NUM] =
{
	{8001,	60,	{30, 30, 30, 9,	1}	},
	{8002,	60,	{30, 30, 30, 9,	1}	},
	{8003,	60,	{30, 30, 30, 9,	1}	},
	{8004,	60,	{30, 30, 30, 9,	1}	},
	{8005,	60,	{30, 30, 30, 9,	1}	},
	{8006,	60,	{28, 29, 31, 11, 1}	},
	{8007,	60,	{24, 29, 32, 13, 2}	},
	{8008,	60,	{22, 28, 33, 15, 2}	},
	{8009,	60,	{21, 27, 33, 17, 2}	},
	{8010,	60,	{18, 26, 34, 20, 2}	},
	{8011,	60,	{14, 26, 35, 22, 3}	},
	{8012,	60,	{10, 26, 37, 24, 3}	},
	{8013,	60,	{2,	26,	40,	29,	3}	},
	{8014,	60,	{0,	26,	41,	30,	3}	},
	{8024,	60,	{0,	26,	41,	30,	3}	},
	{8025,	60,	{0,	26,	41,	30,	3}	},
	{8026,	60,	{0,	26,	41,	30,	3}	},
	{8027,	60,	{0,	26,	41,	30,	3}	},
	{8051,	60,	{0,	26,	41,	30,	3}	},
	{8052,	60,	{0,	26,	41,	30,	3}	},
	{8053,	60,	{0,	26,	41,	30,	3}	},
	{8054,	60,	{0,	26,	41,	30,	3}	},
	{8055,	60,	{0,	26,	41,	30,	3}	},
	{8056,	60,	{0,	26,	41,	30,	3}	},
	{8059,	60,	{0,	26,	41,	30,	3}	},
	{8061,	60,	{0,	26,	41,	30,	3}	},
};

const char * c_apszEmpireNames[EMPIRE_MAX_NUM] =
{
	"전제국",
	"신수국",
	"천조국",
	"진노국"
};

const char * c_apszPrivNames[MAX_PRIV_NUM] =
{
	"",
	"아이템이 나올 확률",
	"돈이 나올 확률",
	"돈 대박이 나올 확률",
	"경험치 배율",
};

const int32_t aiPolymorphPowerByLevel[SKILL_MAX_LEVEL + 1] =
{
	10,
	11,
	11,
	12,
	13,
	13,
	14,
	15,
	16,
	17,
	18,
	19,
	20,
	22,
	23,
	24,
	26,
	27,
	29,
	31,
	33,
	35,
	37,
	39,
	41,
	44,
	46,
	49,
	52,
	55,
	59,
	62,
	66,
	70,
	74,
	79,
	84,
	89,
	94,
	100,
};

TGuildWarInfo KOR_aGuildWarInfo[GUILD_WAR_TYPE_MAX_NUM] =
{
	{ 0,        0,      0,      0,      0,      0       },
	{ 110,      0,      100,    50,     0,      100     },
	{ 111,      0,      100,    50,     0,      10      },
};


const int32_t aiAccessorySocketAddPct[ITEM_ACCESSORY_SOCKET_MAX_NUM] =
{
	50, 50, 50, 50 // @correction140
};

const int32_t aiAccessorySocketEffectivePct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
	0, 10, 20, 40, 80 // @correction140
};

const int32_t aiAccessorySocketDegradeTime[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
	0, 3600 * 24, 3600 * 12, 3600 * 6, 3600 * 3 // @correction140
};

const int32_t aiAccessorySocketPutPct[ITEM_ACCESSORY_SOCKET_MAX_NUM + 1] =
{
	90, 80, 70, 60, 0 // @correction140
};

#include "../common/length.h"
typedef struct SValueName
{
    const char *	c_pszName;
    int32_t		lValue;
} TValueName;

TValueName c_aApplyTypeNames[] =
{
    { "STR",		APPLY_STR		},
    { "DEX",		APPLY_DEX		},
    { "CON",		APPLY_CON		},
    { "INT",		APPLY_INT		},
    { "MAX_HP",		APPLY_MAX_HP		},
    { "MAX_SP",		APPLY_MAX_SP		},
    { "MAX_STAMINA",	APPLY_MAX_STAMINA	},
    { "POISON_REDUCE",	APPLY_POISON_REDUCE	},
    { "EXP_DOUBLE_BONUS", APPLY_EXP_DOUBLE_BONUS },
    { "GOLD_DOUBLE_BONUS", APPLY_GOLD_DOUBLE_BONUS },
    { "ITEM_DROP_BONUS", APPLY_ITEM_DROP_BONUS	},
    { "HP_REGEN",	APPLY_HP_REGEN		},
    { "SP_REGEN",	APPLY_SP_REGEN		},
    { "ATTACK_SPEED",	APPLY_ATT_SPEED		},
    { "MOVE_SPEED",	APPLY_MOV_SPEED		},
    { "CAST_SPEED",	APPLY_CAST_SPEED	},
    { "ATT_BONUS",	APPLY_ATT_GRADE_BONUS	},
    { "DEF_BONUS",	APPLY_DEF_GRADE_BONUS	},
    { "MAGIC_ATT_GRADE",APPLY_MAGIC_ATT_GRADE	},
    { "MAGIC_DEF_GRADE",APPLY_MAGIC_DEF_GRADE	},
    { "SKILL",		APPLY_SKILL		},
    { "ATTBONUS_ANIMAL",APPLY_ATTBONUS_ANIMAL	},
    { "ATTBONUS_UNDEAD",APPLY_ATTBONUS_UNDEAD	},
    { "ATTBONUS_DEVIL", APPLY_ATTBONUS_DEVIL	},
    { "ATTBONUS_HUMAN", APPLY_ATTBONUS_HUMAN	},
    { "ADD_BOW_DISTANCE",APPLY_BOW_DISTANCE	},
    { "DODGE",		APPLY_DODGE		},
    { "BLOCK",		APPLY_BLOCK		},
    { "RESIST_SWORD",	APPLY_RESIST_SWORD	},
    { "RESIST_TWOHAND",	APPLY_RESIST_TWOHAND	},
    { "RESIST_DAGGER",	APPLY_RESIST_DAGGER    },
    { "RESIST_BELL",	APPLY_RESIST_BELL	},
    { "RESIST_FAN",	APPLY_RESIST_FAN	},
    { "RESIST_BOW",	APPLY_RESIST_BOW	},
    { "RESIST_FIRE",	APPLY_RESIST_FIRE	},
    { "RESIST_ELEC",	APPLY_RESIST_ELEC	},
    { "RESIST_MAGIC",	APPLY_RESIST_MAGIC	},
    { "RESIST_WIND",	APPLY_RESIST_WIND	},
	{ "REFLECT_MELEE",	APPLY_REFLECT_MELEE },
	{ "REFLECT_CURSE",	APPLY_REFLECT_CURSE },
    { "RESIST_ICE",		APPLY_RESIST_ICE	},
    { "RESIST_EARTH",	APPLY_RESIST_EARTH	},
    { "RESIST_DARK",	APPLY_RESIST_DARK	},
    { "RESIST_CRITICAL",	APPLY_ANTI_CRITICAL_PCT	},
    { "RESIST_PENETRATE",	APPLY_ANTI_PENETRATE_PCT	},
    { "POISON",		APPLY_POISON_PCT	},
    { "SLOW",		APPLY_SLOW_PCT		},
    { "STUN",		APPLY_STUN_PCT		},
    { "STEAL_HP",	APPLY_STEAL_HP		},
    { "STEAL_SP",	APPLY_STEAL_SP		},
    { "MANA_BURN_PCT",	APPLY_MANA_BURN_PCT	},
    { "CRITICAL",	APPLY_CRITICAL_PCT	},
    { "PENETRATE",	APPLY_PENETRATE_PCT	},
    { "KILL_SP_RECOVER",APPLY_KILL_SP_RECOVER	},
    { "KILL_HP_RECOVER",APPLY_KILL_HP_RECOVER	},
    { "PENETRATE_PCT",	APPLY_PENETRATE_PCT	},
    { "CRITICAL_PCT",	APPLY_CRITICAL_PCT	},
    { "POISON_PCT",	APPLY_POISON_PCT	},
    { "STUN_PCT",	APPLY_STUN_PCT		},
    { "ATT_BONUS_TO_WARRIOR",	APPLY_ATTBONUS_WARRIOR  },
    { "ATT_BONUS_TO_ASSASSIN",	APPLY_ATTBONUS_ASSASSIN },
    { "ATT_BONUS_TO_SURA",	APPLY_ATTBONUS_SURA	    },
    { "ATT_BONUS_TO_SHAMAN",	APPLY_ATTBONUS_SHAMAN   },
    { "ATT_BONUS_TO_MONSTER",	APPLY_ATTBONUS_MONSTER  },
    { "ATT_BONUS_TO_MOB",	APPLY_ATTBONUS_MONSTER  },
    { "MALL_ATTBONUS",	APPLY_MALL_ATTBONUS	},
    { "MALL_EXPBONUS",	APPLY_MALL_EXPBONUS	},
    { "MALL_DEFBONUS",	APPLY_MALL_DEFBONUS	},
    { "MALL_ITEMBONUS",	APPLY_MALL_ITEMBONUS	},
    { "MALL_GOLDBONUS", APPLY_MALL_GOLDBONUS	},
    { "MAX_HP_PCT",	APPLY_MAX_HP_PCT	},
    { "MAX_SP_PCT",	APPLY_MAX_SP_PCT	},
    { "SKILL_DAMAGE_BONUS",	APPLY_SKILL_DAMAGE_BONUS	},
    { "NORMAL_HIT_DAMAGE_BONUS",APPLY_NORMAL_HIT_DAMAGE_BONUS	},
    { "SKILL_DEFEND_BONUS",	APPLY_SKILL_DEFEND_BONUS	},
    { "NORMAL_HIT_DEFEND_BONUS",APPLY_NORMAL_HIT_DEFEND_BONUS	},
    { "PCBANG_EXP_BONUS", APPLY_PC_BANG_EXP_BONUS	},
    { "PCBANG_DROP_BONUS", APPLY_PC_BANG_DROP_BONUS	},

    { "RESIST_WARRIOR",	APPLY_RESIST_WARRIOR},
    { "RESIST_ASSASSIN",	APPLY_RESIST_ASSASSIN},
    { "RESIST_SURA",		APPLY_RESIST_SURA},
    { "RESIST_SHAMAN",	APPLY_RESIST_SHAMAN},
    { "INFINITE_AFFECT_DURATION", 0x1FFFFFFF	},
	{ "ENERGY", APPLY_ENERGY },
	{ "COSTUME_ATTR_BONUS", APPLY_COSTUME_ATTR_BONUS },
	{ "MAGIC_ATTBONUS_PER",	APPLY_MAGIC_ATTBONUS_PER	},
    { "MELEE_MAGIC_ATTBONUS_PER",	APPLY_MELEE_MAGIC_ATTBONUS_PER	},
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	{ "BLEEDING_REDUCE",APPLY_BLEEDING_REDUCE },
	{ "BLEEDING_PCT",APPLY_BLEEDING_PCT },
	{ "ATT_BONUS_TO_WOLFMAN",APPLY_ATTBONUS_WOLFMAN },
	{ "RESIST_WOLFMAN",APPLY_RESIST_WOLFMAN },
	{ "RESIST_CLAW",APPLY_RESIST_CLAW },
#endif
#ifdef __ACCE_SYSTEM__ // @correction191
	{ "ACCEDRAIN_RATE", APPLY_ACCEDRAIN_RATE },	// 97
#endif
#ifdef __ANTI_RESIST_MAGIC_REDUCTION__
	{ "RESIST_MAGIC_REDUCTION", APPLY_RESIST_MAGIC_REDUCTION }, // 98
#endif
#ifdef __ELEMENT_ADD__
	{ "ENCHANT_ELECT", APPLY_ENCHANT_ELECT },	// 99
	{ "ENCHANT_FIRE", APPLY_ENCHANT_FIRE },	// 100
	{ "ENCHANT_ICE", APPLY_ENCHANT_ICE },	// 101
	{ "ENCHANT_WIND", APPLY_ENCHANT_WIND },	// 102
	{ "ENCHANT_EARTH", APPLY_ENCHANT_EARTH },	// 103
	{ "ENCHANT_DARK", APPLY_ENCHANT_DARK },	// 104
	{ "ATTBONUS_CZ", APPLY_ATTBONUS_CZ },	// 105
	{ "ATTBONUS_INSECT", APPLY_ATTBONUS_INSECT },	// 106
	{ "ATTBONUS_DESERT", APPLY_ATTBONUS_DESERT },	// 107
#endif
#ifdef __PENDANT__
	{ "ATTBONUS_SWORD", APPLY_ATTBONUS_SWORD },	// 108
	{ "ATTBONUS_TWOHAND", APPLY_ATTBONUS_TWOHAND },	// 109
	{ "ATTBONUS_DAGGER", APPLY_ATTBONUS_DAGGER },	// 110
	{ "ATTBONUS_BELL", APPLY_ATTBONUS_BELL },	// 111
	{ "ATTBONUS_FAN", APPLY_ATTBONUS_FAN },	// 112
	{ "ATTBONUS_BOW", APPLY_ATTBONUS_BOW },	// 113
#ifdef ENABLE_WOLFMAN_CHARACTER // @correction190
	{ "ATTBONUS_CLAW", APPLY_ATTBONUS_CLAW },	// 114
#endif
	{ "RESIST_HUMAN", APPLY_RESIST_HUMAN },	// 115
	{ "RESIST_MOUNT_FALL", APPLY_RESIST_MOUNT_FALL },	// 116
#endif
#ifdef __NEW_BONUSES__
	{ "ATTBONUS_STONE", APPLY_ATTBONUS_STONE },
	{ "ATTBONUS_BOSS", APPLY_ATTBONUS_BOSS },
#endif
#ifdef __ELEMENT_ADD__
	{ "ENCHANT_AVATAR", APPLY_ENCHANT_AVATAR },
#endif
    { nullptr,		0			}
};

int32_t FN_get_apply_type(const char *apply_type_string)
{
	TValueName	*value_name;
	for (value_name = c_aApplyTypeNames; value_name->c_pszName; ++value_name)
	{
		if (0==strcasecmp(value_name->c_pszName, apply_type_string))
			return value_name->lValue;
	}
	return 0;
}

#ifdef __SUPPORT__
const uint32_t exp_support_table[121] =
{
	0,
	300,
	800,
	1500,
	2500,
	4300,
	7200,
	11000,
	17000,
	24000,
	33001,
	43000,
	58000,
	76000,
	100000,
	130000,
	169000,
	219000,
	283000,
	365000,
	472000,
	610000,
	705000,
	813000,
	937000,
	1077000,
	1237000,
	1418000,
	1624000,
	1857000,
	2122000,
	2421000,
	2761000,
	3145000,
	3580000,
	4073000,
	4632000,
	5194000,
	5717000,
	6264000,
	6837000,
	7600000,
	8274000,
	8990000,
	9753000,
	10560000,
	11410000,
	12320000,
	13270000,
	14280000,
	15340000,
	16870000,
	18960000,
	19980000,
	21420000,
	22930000,
	24530000,
	26200000,
	27960000,
	29800000,
	32780000,
	36060000,
	39670000,
	43640000,
	48000000,
	52800000,
	58080000,
	63890000,
	70280000,
	77310000,
	85040000,
	93540000,
	102900000,
	113200000,
	124500000,
	137000000,
	150700000,
	165700000,
	236990000,
	260650000,
	286780000,
	315380000,
	346970000,
	381680000,
	419770000,
	461760000,
	508040000,
	558740000,
	614640000,
	676130000,
	743730000,
	1041222000,
	1145344200,
	1259878620,
	1385866482,
	1524453130,
	1676898443,
	1844588288,
	2029047116,
	2147483646,
};
#endif


#ifdef __GROWTH_PET__
uint32_t* exppet_table = nullptr;

const uint32_t Pet_Skill_Table[18][23] =
{
	//Level Like Index ->> Bonus Value --> typeSkill --> CoolDown -->> Percentage
	{ 78, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 },
	{ 80, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 },
	{ 79, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 },
	{ 81, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 },
	{ 95, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 }, //5
	{ 64, 0, 0, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 10 },
	{ 97, 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 7, 7, 8, 8, 9, 10, 10, 11, 11, 12 },
	{ 63, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 },
	{ 16, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9 },
	{ 4, 1, 480, 17, 68, 153, 272, 425, 612, 833, 1088, 1377, 1700, 2057, 2448, 2873, 3332, 3825, 4352, 4913, 5508, 6137, 6800 }, //10
	{ 23, 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5 },
	{ 24, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 6, 6, 6, 6, 6 },
	{ 27, 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 10, 10, 11, 11 },
	{ 117, 0, 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 11, 12 },
	{ 44, 0, 0, 5, 10, 15, 20, 25, 31, 36, 41, 46, 51, 56, 61, 67, 72, 77, 82, 87, 92, 97, 103 }, //15
	{ 52, 0, 0, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6 },
	{ 11, 1, 600, 2, 4, 6, 7, 8, 9, 11, 12, 13, 15, 16, 17, 18, 20, 21, 22, 23, 25, 26, 27 },
	{ 6, 1, 480, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }


};

const uint32_t Pet_SKill_TablePerc[3][20] =
{
	{ 2, 5, 8, 11, 13, 16, 19, 22, 24, 27, 30, 33, 35, 38, 41, 44, 46, 49, 52, 55 },
	{ 2, 5, 7, 10, 12, 15, 17, 20, 23, 25, 27, 30, 32, 35, 37, 40, 42, 45, 47, 50 },
	{ 2, 5, 8, 11, 13, 16, 19, 22, 25, 27, 30, 33, 36, 38, 41, 44, 47, 50, 52, 55 }
};

const uint32_t Pet_Types[9][3] =
{
	{0, 0, 0},
	{6, 1, 5},
	{5, 2, 3},
	{6, 1, 7},
	{5, 3, 4},
	{6, 1, 9},
	{5, 5, 6},
	{6, 1, 10},
	{5, 7, 10}
};

uint32_t exppet_table_common[121];
#endif


#ifdef __MINI_GAME__
const uint32_t dwMiniGameNpcVnums[MINIGAME_TYPE_MAX] =
{
#ifdef __MINI_GAME_RUMI__
	20417,
#endif
#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
	30131,
	30129,
#endif
#ifdef __MINI_GAME_FISH__
	0,
#endif
	0,
#ifdef __MINI_GAME_CATCH_KING__
	20506,
#endif
};

const uint32_t dwMiniGameNpcPos[MINIGAME_TYPE_MAX][6] =
{
#ifdef __MINI_GAME_RUMI__
	{ 599, 637, 595, 618, 354, 739 },	// 20417
#endif
#if defined(__MINI_GAME_ATTENDANCE__) || defined(__MINI_GAME_MONSTERBACK__)
	{ 608, 617, 596, 610, 357, 743 },	// 30131
	{ 590, 471, 517, 742, 306, 828 },	// 30129
#endif
#ifdef __MINI_GAME_FISH__
	{ 0, 0, 0, 0, 0, 0 },				// 0
#endif
	{ 0, 0, 0, 0, 0, 0 },				// 0
#ifdef __MINI_GAME_CATCH_KING__
	{ 608, 623, 596, 614, 350, 738 }, // 20506
#endif
};
#endif

#ifdef __12ZI__
const int32_t iCzTable[2][30][2] =
{
	//Yellow
	{
		//0
		{
			33001,
			33013,
		},

		//1
		{
			33003,
			33013,
		},

		//2
		{
			33005,
			33013,
		},

		//3
		{
			33007,
			33013,
		},

		//4
		{
			33009,
			33013,
		},

		//5
		{
			33011,
			33013,
		},

		//6
		{
			33001,
			33015,
		},

		//7
		{
			33003,
			33015,
		},

		//8
		{
			33005,
			33015,
		},

		//9
		{
			33007,
			33015,
		},

		//10
		{
			33009,
			33015,
		},

		//11
		{
			33011,
			33015,
		},

		//12
		{
			33001,
			33017,
		},

		//13
		{
			33003,
			33017,
		},

		//14
		{
			33005,
			33017,
		},

		//15
		{
			33007,
			33017,
		},

		//16
		{
			33009,
			33017,
		},

		//17
		{
			33011,
			33017,
		},

		//18
		{
			33001,
			33019,
		},

		//19
		{
			33003,
			33019,
		},

		//20
		{
			33005,
			33019,
		},

		//21
		{
			33007,
			33019,
		},

		//22
		{
			33009,
			33019,
		},

		//23
		{
			33011,
			33019,
		},

		//24
		{
			33001,
			33021,
		},

		//25
		{
			33003,
			33021,
		},

		//26
		{
			33005,
			33021,
		},

		//27
		{
			33007,
			33021,
		},

		//28
		{
			33009,
			33021,
		},

		//29
		{
			33011,
			33021,
		},
	},
	//Green
	{
		//0
		{
			33002,
			33014,
		},

		//1
		{
			33004,
			33014,
		},

		//2
		{
			33006,
			33014,
		},

		//3
		{
			33008,
			33014,
		},

		//4
		{
			33010,
			33014,
		},

		//5
		{
			33012,
			33014,
		},

		//6
		{
			33002,
			33016,
		},

		//7
		{
			33004,
			33016,
		},

		//8
		{
			33006,
			33016,
		},

		//9
		{
			33008,
			33016,
		},

		//10
		{
			33010,
			33016,
		},

		//11
		{
			33012,
			33016,
		},

		//12
		{
			33002,
			33018,
		},

		//13
		{
			33004,
			33018,
		},

		//14
		{
			33006,
			33018,
		},

		//15
		{
			33008,
			33018,
		},

		//16
		{
			33010,
			33018,
		},

		//17
		{
			33012,
			33018,
		},

		//18
		{
			33002,
			33020,
		},

		//19
		{
			33004,
			33020,
		},

		//20
		{
			33006,
			33020,
		},

		//21
		{
			33008,
			33020,
		},

		//22
		{
			33010,
			33020,
		},

		//23
		{
			33012,
			33020,
		},

		//24
		{
			33002,
			33022,
		},

		//25
		{
			33004,
			33022,
		},

		//26
		{
			33006,
			33022,
		},

		//27
		{
			33008,
			33022,
		},

		//28
		{
			33010,
			33022,
		},

		//29
		{
			33012,
			33022,
		},
	},
};

int32_t GetRandomLevel(uint8_t bFloor, bool bIsBoss)
{
	if (bFloor >= 1 && bFloor <= 3)
	{
		return number(15, 30);
	}
	if (bFloor >= 4 && bFloor <= 6)
	{
		return number(30, 45);
	}
	else if (bFloor >= 7 && bFloor <= 9)
	{
		return number(45, 60);
	}
	else if (bFloor >= 10 && bFloor <= 12)
	{
		return number(60, 75);
	}
	else if (bFloor >= 13 && bFloor <= 15)
	{
		return number(75, 80);
	}
	else if (bFloor >= 16 && bFloor <= 21)
	{
		return number(80, 85);
	}
	else if (bFloor >= 22 && bFloor <= 28)
	{
		return number(85, 100);
	}
	else if (bFloor >= 29 && bFloor <= 34)
	{
		return number(100, 135);
	}
	else
		return 1;
}
#endif

#ifdef __AFAR_BIOLOG__
const uint32_t BiyologSistemi[11][14] =
{
	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{ 30006, 10, 0, 100, 30220, 50109, APPLY_MOV_SPEED, 10, 0,0,0,0,0,0},
	{ 30047, 15, 0, 100, 30221, 50110, APPLY_ATT_SPEED, 5, 0,0,0,0,0,0},
	{ 30015, 15, 0, 100, 30222, 50111, APPLY_DEF_GRADE_BONUS, 60, 0,0,0,0,0,0},
	{ 30050, 20, 0, 100, 30223, 50112, APPLY_ATT_GRADE_BONUS, 50, 0,0,0,0,0,0},
	{ 30165, 25, 0, 100, 30224, 50113, APPLY_MOV_SPEED, 11, 0,0,0,0,0,0},
	{ 30166, 30, 0, 100, 30225, 50114, APPLY_ATT_SPEED, 6, APPLY_ATT_GRADE_BONUS,10,0,0,0,0},
	{ 30167, 40, 0, 100, 30226, 50115, APPLY_ATTBONUS_HUMAN, 10, 0,0,0,0,0,0},
	{ 30168, 50, 30, 100, 30227, 50114, APPLY_ATTBONUS_HUMAN, 8,0,0,0,0,0,0},
	{ 30251, 10, 60, 100, 0, 0, APPLY_MAX_HP, 1000, APPLY_DEF_GRADE_BONUS,120,APPLY_ATT_GRADE_BONUS,50,0,0},
	{ 30252, 20, 60, 100, 30228, 0, APPLY_MAX_HP, 1100, APPLY_DEF_GRADE_BONUS,140,APPLY_ATT_GRADE_BONUS,60,0,0},
};
#endif
#pragma once

#define MAX_CASTLE_GUARD_REGION		4
#define MAX_CASTLE_GUARD_PER_REGION	10
#define MAX_CASTLE_FROG				20
#define MAX_CASTLE_TOWER			10
#define MIN_CASTLE_TOWER			5


#define CASTLE_FROG_PRICE		100000000
#define CASTLE_FROG_VNUM		11505
#define IS_CASTLE_MAP(map)		(181==(map)||182==(map)||(183)==(map))


enum CASTLE_STATE
{
	CASTLE_SIEGE_NONE,
	CASTLE_SIEGE_STRUGGLE,
	CASTLE_SIEGE_END
};


struct CASTLE_DATA
{
	LPCHARACTER	frog[MAX_CASTLE_FROG];

	LPCHARACTER	guard[MAX_CASTLE_GUARD_REGION][MAX_CASTLE_GUARD_PER_REGION];
	uint32_t		guard_group[MAX_CASTLE_GUARD_REGION][MAX_CASTLE_GUARD_PER_REGION];

	LPCHARACTER	tower[MAX_CASTLE_TOWER];

	LPEVENT		siege_event;
	LPEVENT		stone_event;
};

bool		castle_boot();
void		castle_save();
int32_t			castle_siege(int32_t empire, int32_t tower_count);
void		castle_start_siege(int32_t empire, int32_t tower_count);
void		castle_end_siege();

LPCHARACTER castle_spawn_frog(int32_t empire);
LPCHARACTER	castle_spawn_guard(int32_t empire, uint32_t group_vnum, int32_t region_index);
bool		castle_spawn_tower(int32_t empire, int32_t tower_count);

void		castle_guard_die(LPCHARACTER ch, LPCHARACTER killer);
void		castle_frog_die(LPCHARACTER ch, LPCHARACTER killer);
void		castle_tower_die(LPCHARACTER ch, LPCHARACTER killer);

int32_t			castle_guard_count(int32_t empire, int32_t region_index);
int32_t			castle_frog_count(int32_t empire);

bool		castle_is_guard_vnum(uint32_t vnum);
int32_t			castle_cost_of_hiring_guard(uint32_t vnum);
bool		castle_can_attack(LPCHARACTER ch, LPCHARACTER victim);

bool		castle_frog_to_empire_money(LPCHARACTER ch);
bool		castle_is_my_castle(int32_t empire, int32_t map_index);
bool		castle_is_tower_vnum(uint32_t vnum);




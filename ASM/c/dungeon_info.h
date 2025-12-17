#ifndef DUNGEON_INFO_H
#define DUNGEON_INFO_H

#include "util.h"
#include "z64.h"

typedef enum {
    /* 0x00 */ OPT_HINT_AREA_UNKNOWN,
    /* 0x01 */ OPT_HINT_AREA_ROOT,
    /* 0x02 */ OPT_HINT_AREA_HYRULE_FIELD,
    /* 0x03 */ OPT_HINT_AREA_LON_LON_RANCH,
    /* 0x04 */ OPT_HINT_AREA_MARKET,
    /* 0x05 */ OPT_HINT_AREA_TEMPLE_OF_TIME,
    /* 0x06 */ OPT_HINT_AREA_HYRULE_CASTLE,
    /* 0x07 */ OPT_HINT_AREA_OUTSIDE_GANONS_CASTLE,
    /* 0x08 */ OPT_HINT_AREA_INSIDE_GANONS_CASTLE,
    /* 0x09 */ OPT_HINT_AREA_KOKIRI_FOREST,
    /* 0x0A */ OPT_HINT_AREA_DEKU_TREE,
    /* 0x0B */ OPT_HINT_AREA_LOST_WOODS,
    /* 0x0C */ OPT_HINT_AREA_SACRED_FOREST_MEADOW,
    /* 0x0D */ OPT_HINT_AREA_FOREST_TEMPLE,
    /* 0x0E */ OPT_HINT_AREA_DEATH_MOUNTAIN_TRAIL,
    /* 0x0F */ OPT_HINT_AREA_DODONGOS_CAVERN,
    /* 0x10 */ OPT_HINT_AREA_GORON_CITY,
    /* 0x11 */ OPT_HINT_AREA_DEATH_MOUNTAIN_CRATER,
    /* 0x12 */ OPT_HINT_AREA_FIRE_TEMPLE,
    /* 0x13 */ OPT_HINT_AREA_ZORA_RIVER,
    /* 0x14 */ OPT_HINT_AREA_ZORAS_DOMAIN,
    /* 0x15 */ OPT_HINT_AREA_ZORAS_FOUNTAIN,
    /* 0x16 */ OPT_HINT_AREA_JABU_JABUS_BELLY,
    /* 0x17 */ OPT_HINT_AREA_ICE_CAVERN,
    /* 0x18 */ OPT_HINT_AREA_LAKE_HYLIA,
    /* 0x19 */ OPT_HINT_AREA_WATER_TEMPLE,
    /* 0x1A */ OPT_HINT_AREA_KAKARIKO_VILLAGE,
    /* 0x1B */ OPT_HINT_AREA_BOTTOM_OF_THE_WELL,
    /* 0x1C */ OPT_HINT_AREA_GRAVEYARD,
    /* 0x1D */ OPT_HINT_AREA_SHADOW_TEMPLE,
    /* 0x1E */ OPT_HINT_AREA_GERUDO_VALLEY,
    /* 0x1F */ OPT_HINT_AREA_GERUDO_FORTRESS,
    /* 0x20 */ OPT_HINT_AREA_THIEVES_HIDEOUT,
    /* 0x21 */ OPT_HINT_AREA_GERUDO_TRAINING_GROUND,
    /* 0x22 */ OPT_HINT_AREA_HAUNTED_WASTELAND,
    /* 0x23 */ OPT_HINT_AREA_DESERT_COLOSSUS,
    /* 0x24 */ OPT_HINT_AREA_SPIRIT_TEMPLE,

    /* 0x25 */ OPT_HINT_AREA_MAX,
} opt_hint_area_t;

typedef struct {
    uint8_t index;
    struct {
        uint8_t has_keys : 1;
        uint8_t has_boss_key : 1;
        uint8_t has_card : 1;
        uint8_t has_map : 1;
    };
    uint8_t skulltulas;
    char name[11];
    uint8_t silver_rupee_puzzles_vanilla[4];
    uint8_t silver_rupee_puzzles_mq[4];
    opt_hint_area_t hint_area;
} dungeon_entry_t;

typedef struct {
    uint8_t index;
    uint8_t has_map;
    char name[10];
} boss_entry_t;

extern int dungeon_count;
extern dungeon_entry_t dungeons[14];

void draw_dungeon_info(z64_disp_buf_t* db);
void draw_world_info(z64_disp_buf_t* db);
void draw_silver_rupee_count(z64_game_t* globalCtx, z64_disp_buf_t* db);
void draw_boss_key(z64_game_t* globalCtx, z64_disp_buf_t* db);
extern uint8_t bk_display;
int dungeon_info_is_drawn();

extern unsigned char CFG_DUNGEON_IS_MQ[14];
extern uint32_t CFG_DUNGEON_INFO_ENABLE;
extern uint8_t CFG_DPAD_DUNGEON_INFO_ENABLE;
extern char CFG_DUNGEON_BOSS_INFO[14];
extern char CFG_DUNGEON_ENTRANCES[12][0x9];
extern char CFG_BOSSES[21][0x9];

static int show_dungeon_info = 0;
static uint8_t everdrive_last_dungeon_info[0x13] = { 0x05, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

#define CAN_DRAW_DUNGEON_INFO (CFG_DUNGEON_INFO_ENABLE != 0 && \
        z64_game.pause_ctxt.state == PAUSE_STATE_MAIN && \
        z64_game.pause_ctxt.screen_idx == 0 && \
        (!z64_game.pause_ctxt.changing || \
        z64_game.pause_ctxt.changing == 3))

#define CAN_DRAW_WORLD_INFO (CFG_DUNGEON_INFO_ENABLE != 0 && \
        z64_game.pause_ctxt.state == PAUSE_STATE_MAIN && \
        z64_game.pause_ctxt.screen_idx == 1 && \
        (!z64_game.pause_ctxt.changing || \
        z64_game.pause_ctxt.changing == 3))

#endif

#ifndef moomaster_h
#define moomaster_h

#define REFRESH_TIMER	5
#define INVENTORY_TIMER	20

#define GOAL_MINETOWN	1
#define GOAL_MINESEND	2
#define GOAL_QUEST	3
#define GOAL_CASTLE	4
#define GOAL_VLAD	5
#define GOAL_RODNEY	6
#define GOAL_INVOCATION	7
#define GOAL_AMULET	8
#define GOAL_EARTH	9
#define GOAL_AIR	10
#define GOAL_FIRE	11
#define GOAL_WATER	12
#define GOAL_ASTRAL	13
#define GOAL_ASCEND	14

#define STATE_DESCENDING	1
#define STATE_ASCENDING		2

#define NOTE_DOOR		0x01
#define NOTE_UPSTAIR		0x02
#define NOTE_DOWNSTAIR		0x04
#define NOTE_ALTAR		0x08
#define NOTE_OPENDOOR		0x10
#define NOTE_TRAP		0x80
#define NOTE_BADITEM		0x100
#define NOTE_SEENFLOOR		0x200

#define PEACEFUL_MONSTER	','
#define NOTOUCH_MONSTER		0x7F

void Write(int fd, const char *message);
unsigned char ismonster(char c);
unsigned char isitem(char c);
unsigned char isgooditem(char c);
unsigned char corridorchar(char c);
unsigned char canbepeaceful(char c);
unsigned char shouldpickup(const char *message);
unsigned char shouldeat(const char *message);
void investigate(int x, int y, char *message);

#endif

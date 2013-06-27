#ifndef inventory_h
#define inventory_h

#define ITEM_DAGGER		1
#define ITEM_ARMOR		2
#define ITEM_HELM		3
#define ITEM_FOOTWEAR		4
#define ITEM_WAND		5
#define ITEM_POTION		6
#define ITEM_UNKNOWN		7
#define ITEM_TOOL		8
#define ITEM_SCROLL		9
#define ITEM_SHIELD		10
#define ITEM_CORPSE		11

// Begin item IDs

#define DAGGER_NONE		0
#define DAGGER_UNKNOWN		1
#define DAGGER_STANDARD		2

#define ARMOR_NONE		0
#define ARMOR_NOWEAR		0
#define ARMOR_LEATHER		1
#define ARMOR_RING_MAIL		2
#define ARMOR_STUDDED_LEATHER	3
#define ARMOR_SCALE_MAIL	4
#define ARMOR_CHAIN_MAIL	5
#define ARMOR_MITHRIL_ELVEN	6
#define ARMOR_BRONZE_PLATE_MAIL	7
#define ARMOR_BANDED_MAIL	8
#define ARMOR_SPLINT_MAIL	9
#define ARMOR_MITHRIL_DWARVEN	10
#define ARMOR_PLATE_MAIL	11
#define ARMOR_DRAGON_SCALE_MAIL	12
#define ARMOR_CRAPPY		13

#define HELM_NONE		0
#define HELM_UNKNOWN		1
#define HELM_ORCISH		2
#define HELM_DWARVISH		3
#define HELM_OPPOSITEALIGNMENT	4
#define HELM_BRILLIANCE		5
#define HELM_ELVEN		6
#define HELM_TELEPATHY		7

#define FOOTWEAR_NONE		0
#define FOOTWEAR_UNKNOWN	1
#define FOOTWEAR_IRONSHOES	2

#define WAND_NONE		0
#define WAND_UNKNOWN		1

#define POTION_NONE		0
#define POTION_UNKNOWN		1
#define POTION_BOOZE		2
#define POTION_FRUITJUICE	3
#define POTION_SEEINVISIBLE	4
#define POTION_SICKNESS		5
#define POTION_CONFUSION	6
#define POTION_EXTRAHEALING	7
#define POTION_HALLUCINATION	8
#define POTION_HEALING		9
#define POTION_RESTOREABILITY	10
#define POTION_SLEEPING		11
#define POTION_WATER		12
#define POTION_BLINDNESS	13
#define POTION_GAINENERGY	14
#define POTION_INVISIBILITY	15
#define POTION_MONSTERDETECTION	16
#define POTION_OBJECTDETECTION	17
#define POTION_ENLIGHTENMENT	18
#define POTION_FULLHEALING	19
#define POTION_LEVITATION	20
#define POTION_POLYMORPH	21
#define POTION_SPEED		22
#define POTION_ACID		23
#define POTION_OIL		24
#define POTION_GAINABILITY	25
#define POTION_GAINLEVEL	26
#define POTION_PARALYSIS	27

#define TOOL_NONE		0
#define TOOL_UNKNOWN		1
#define TOOL_UNIHORN		2
#define TOOL_LAMP		3

#define SCROLL_NONE		0
#define SCROLL_UNKNOWN		1

#define SHIELD_NONE		0
#define SHIELD_UNKNOWN		1
#define SHIELD_SMALL		2
#define SHIELD_LARGE		3
#define SHIELD_REFLECTION	4
#define SHIELD_URUK		5
#define SHIELD_ORCISH		6
#define SHIELD_ELVEN		7
#define SHIELD_DWARVISH		8

#define CORPSE_NONE		0
#define CORPSE_UNKNOWN		1
#define CORPSE_LICHEN		2
#define CORPSE_LIZARD		3

// End item IDs

#define FLAG_WORN		0x01
#define FLAG_LIT		0x02
#define FLAG_WIELDED		0x04
#define FLAG_UNPAID		0x08

typedef struct
{
	int type;
	int subtype;
	int count;
	char letter;
	int buc;
	int enchantment;
	int flags;
	char container;	// inventory letter of the container. Not used atm.
} item_t;

int armorac(int armor);
int getarmortype(const char *message);
unsigned char shouldweararmor(const char *message);
unsigned char shouldweararmor2(item_t *item);
unsigned char shouldwearhelm(const char *message);
unsigned char shouldwearhelm2(item_t *item);
unsigned char shouldwearboots(const char *message);
unsigned char shouldwearboots2(item_t *item);
//void setarmorpref(const char *message);
unsigned char shouldwear(const char *message);
int getdaggertype(const char *message);
unsigned char shouldgrabweapon(const char *message);
int getpotiontype(const char *message);
unsigned char IDpotion(const char *message);
unsigned char getshieldtype(const char *message);
int gettooltype(const char *message);
//void stripnaked();
void str2item(const char *line, item_t *item);
void refreshinventory();
int countitems(int type, int subtype);
item_t *getitem(int type, int subtype);

#endif

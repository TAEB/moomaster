#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inventory.h"

extern FILE *logfile;
extern int fd1[2];
extern int fd2[2];
extern int mydlvl;
extern item_t inventory[128];
extern int inventorycount;

int bodyarmor = ARMOR_NONE;
int bodyarmorpreference = ARMOR_NONE;
int helm = HELM_NONE;
int helmpreference = HELM_NONE;
int boots = FOOTWEAR_NONE;
int bootpreference = FOOTWEAR_NONE;

int armorac(int armor)
{
	switch (armor)
	{
		case ARMOR_NONE:
		case ARMOR_CRAPPY:
			return ARMOR_NOWEAR;

		case ARMOR_LEATHER:
			return 2;

		case ARMOR_RING_MAIL:
		case ARMOR_STUDDED_LEATHER:
			return 3;

		case ARMOR_SCALE_MAIL:
			return 4;

		case ARMOR_CHAIN_MAIL:
		case ARMOR_MITHRIL_ELVEN:
			return 5;

		case ARMOR_BRONZE_PLATE_MAIL:
		case ARMOR_BANDED_MAIL:
		case ARMOR_SPLINT_MAIL:
		case ARMOR_MITHRIL_DWARVEN:
			return 6;

		case ARMOR_PLATE_MAIL:
			return 7;

		case ARMOR_DRAGON_SCALE_MAIL:
			return 9;

		default:
			return ARMOR_NOWEAR;
	}

	return ARMOR_NOWEAR;
}

int armorquality(int armor)
{
	int preference = armorac(armor);

	if (armor == ARMOR_MITHRIL_DWARVEN || armor == ARMOR_MITHRIL_ELVEN)
	{
		preference += 2;
	}
	else if (armor == ARMOR_DRAGON_SCALE_MAIL)
	{
		preference += 5;
	}

	return preference;
}

int getarmortype(const char *message)
{
	int armortype = ARMOR_NONE;

	if (strcasestr(message, "crystal plate mail"))
	{
		armortype = ARMOR_CRAPPY;
	}
	else if (strcasestr(message, "bronze plate mail"))
	{
		armortype = ARMOR_BRONZE_PLATE_MAIL;
	}
	else if (strcasestr(message, "orcish") && strcasestr(message, "mail"))
	{
		armortype = ARMOR_CRAPPY;
	}
	else if (strcasestr(message, "plate mail"))
	{
		armortype = ARMOR_PLATE_MAIL;
	}
	else if (strcasestr(message, "ring mail"))
	{
		armortype = ARMOR_RING_MAIL;
	}
	else if (strcasestr(message, "chain mail"))
	{
		armortype = ARMOR_CHAIN_MAIL;
	}
	else if (strcasestr(message, "banded mail"))
	{
		armortype = ARMOR_BANDED_MAIL;
	}
	else if (strcasestr(message, "splint mail"))
	{
		armortype = ARMOR_SPLINT_MAIL;
	}
	else if (strcasestr(message, "dragon scale mail"))
	{
		armortype = ARMOR_DRAGON_SCALE_MAIL;
	}
	else if (strcasestr(message, "scale mail"))
	{
		armortype = ARMOR_SCALE_MAIL;
	}
	else if (strcasestr(message, "dwarvish") && strcasestr(message, "mithril"))
	{
		armortype = ARMOR_MITHRIL_DWARVEN;
	}
	else if (strcasestr(message, "elven") && strcasestr(message, "mithril"))
	{
		armortype = ARMOR_MITHRIL_ELVEN;
	}
	else if (strcasestr(message, "studded leather"))
	{
		armortype = ARMOR_STUDDED_LEATHER;
	}
	else if (strcasestr(message, "leather armor"))
	{
		armortype = ARMOR_LEATHER;
	}
	else if (strcasestr(message, "leather jacket"))
	{
		armortype = ARMOR_CRAPPY;
	}

	return armortype;
}

unsigned char shouldweararmor(const char *message)
{
	int armortype = ARMOR_NONE;
	int preference;

	if (strcasestr(message, "cursed"))
	{
		return 0;
	}

	armortype = getarmortype(message);

	preference = armorquality(armortype);
	if (preference > bodyarmorpreference)
	{
		bodyarmorpreference = preference;
		return 1;
	}

	return 0;
}

unsigned char shouldweararmor2(item_t *item)
{
	int armortype = ARMOR_NONE;
	int preference;

	if (item->buc == -1)
	{
		return 0;
	}

	armortype = item->subtype;

	preference = armorquality(armortype);
	if (preference > bodyarmorpreference)
	{
		bodyarmorpreference = preference;
		return 1;
	}

	return 0;
}

int helmac(int helm)
{
	switch (helm)
	{
		case HELM_ORCISH:
			return 1;
		case HELM_DWARVISH:
			return 2;
		// the rest are worthless right now
	}

	return 0;
}

int gethelmtype(const char *message)
{
	int type = HELM_UNKNOWN;

	if (!strcasestr(message, " helm"))
	{
		return HELM_NONE;
	}
	if (strcasestr(message, "orcish"))
	{
		fprintf(logfile, "Identified as orcish helm\n");
		type = HELM_ORCISH;
	}
	else if (strcasestr(message, "dwarvish"))
	{
		type = HELM_DWARVISH;
	}

	return type;
}

unsigned char shouldwearhelm(const char *message)
{
	int helmtype = HELM_NONE;
	int preference;

	if (strcasestr(message, "cursed"))
	{
		return 0;
	}

	helmtype = gethelmtype(message);

	preference = helmac(helmtype);
	if (preference > helmpreference)
	{
		helmpreference = preference;
		return 1;
	}

	return 0;
}

unsigned char shouldwearhelm2(item_t *item)
{
	int helmtype = ARMOR_NONE;
	int preference;

	if (item->buc == -1)
	{
		return 0;
	}

	helmtype = item->subtype;

	preference = helmac(helmtype);
	if (preference > helmpreference)
	{
		helmpreference = preference;
		return 1;
	}

	return 0;
}

unsigned char shouldwear(const char *message)
{
	return shouldweararmor(message) || shouldwearhelm(message);// || shouldwearboots(message);
}

void setarmorpref(const char *message)
{
	int armortype = ARMOR_NONE;

	armortype = getarmortype(message);

	if (armortype != ARMOR_NONE)
	{
		bodyarmor = armortype;
		bodyarmorpreference = armorquality(armortype);
		fprintf(logfile, "Set body armor preference to %i\n", bodyarmorpreference);
	}
}

void sethelmpref(const char *message)
{
	int helmtype = HELM_NONE;

	helmtype = gethelmtype(message);

	helm = helmtype;
	helmpreference = helmac(helmtype);
}

int getdaggertype(const char *message)
{
	int daggertype = DAGGER_UNKNOWN;

	if (!strcasestr(message, "dagger"))
	{
		return DAGGER_NONE;
	}
	else
	{
		// we don't really care what type of dagger atm
		daggertype = DAGGER_STANDARD;
	}

	return daggertype;
}

unsigned char shouldgrabweapon(const char *message)
{
	return (getdaggertype(message) == DAGGER_STANDARD);
}

int getpotiontype(const char *message)
{
	int potiontype = POTION_UNKNOWN;

	if (!strcasestr(message, "potion"))
	{
		return POTION_NONE;
	}
	if (strcasestr(message, "booze"))
	{
		potiontype = POTION_BOOZE;
	}
	else if (strcasestr(message, "fruit juice"))
	{
		potiontype = POTION_FRUITJUICE;
	}
	else if (strcasestr(message, "see invisible"))
	{
		potiontype = POTION_SEEINVISIBLE;
	}
	else if (strcasestr(message, "sickness"))
	{
		potiontype = POTION_SICKNESS;
	}
	else if (strcasestr(message, "confusion"))
	{
		potiontype = POTION_CONFUSION;
	}
	else if (strcasestr(message, "extra healing"))
	{
		potiontype = POTION_EXTRAHEALING;
	}
	else if (strcasestr(message, "hallucination"))
	{
		potiontype = POTION_HALLUCINATION;
	}
	else if (strcasestr(message, "healing"))
	{
		potiontype = POTION_HEALING;
	}
	else if (strcasestr(message, "restore ability"))
	{
		potiontype = POTION_RESTOREABILITY;
	}
	else if (strcasestr(message, "sleeping"))
	{
		potiontype = POTION_SLEEPING;
	}
	else if (strcasestr(message, "water"))
	{
		potiontype = POTION_WATER;
	}
	else if (strcasestr(message, "blindness"))
	{
		potiontype = POTION_BLINDNESS;
	}
	else if (strcasestr(message, "gain energy"))
	{
		potiontype = POTION_GAINENERGY;
	}
	else if (strcasestr(message, "invisibility"))
	{
		potiontype = POTION_INVISIBILITY;
	}
	else if (strcasestr(message, "monster detection"))
	{
		potiontype = POTION_MONSTERDETECTION;
	}
	else if (strcasestr(message, "object detection"))
	{
		potiontype = POTION_OBJECTDETECTION;
	}
	else if (strcasestr(message, "enlightenment"))
	{
		potiontype = POTION_ENLIGHTENMENT;
	}
	else if (strcasestr(message, "full healing"))
	{
		potiontype = POTION_FULLHEALING;
	}
	else if (strcasestr(message, "levitation"))
	{
		potiontype = POTION_LEVITATION;
	}
	else if (strcasestr(message, "polymorph"))
	{
		potiontype = POTION_POLYMORPH;
	}
	else if (strcasestr(message, "speed"))
	{
		potiontype = POTION_SPEED;
	}
	else if (strcasestr(message, "acid"))
	{
		potiontype = POTION_ACID;
	}
	else if (strcasestr(message, "oil"))
	{
		potiontype = POTION_OIL;
	}
	else if (strcasestr(message, "gain ability"))
	{
		potiontype = POTION_GAINABILITY;
	}
	else if (strcasestr(message, "gain level"))
	{
		potiontype = POTION_GAINLEVEL;
	}
	else if (strcasestr(message, "paralysis"))
	{
		potiontype = POTION_PARALYSIS;
	}

	return potiontype;
}

unsigned char IDpotion(const char *message)
{
	if (strcasestr(message, "this burns"))
	{
		if (!strcasestr(message, "like acid"))
		{
			fprintf(logfile, "IDed holy water\n");
			Write(fd1[1], " water\r");
		}
		else
		{
			fprintf(logfile, "IDed acid\n");
			Write(fd1[1], " acid\r");
		}
	}
	else if (strcasestr(message, "cloud of darkness falls") || strcasestr(message, "everything is dark!"))
	{
		fprintf(logfile, "IDed blindness\n");
		Write(fd1[1], " blindness\r");
	}
	else if (strcasestr(message, "liquid fire"))
	{
		fprintf(logfile, "IDed booze\n");
		Write(fd1[1], " booze\r");
	}
	else if (strcasestr(message, "where am I"))
	{
		fprintf(logfile, "IDed confusion\n");
		Write(fd1[1], " confusion\r");
	}
	else if (strcasestr(message, "uneasy feeling"))
	{
		if (mydlvl == 1)
		{
			fprintf(logfile, "IDed enlightenment/gain level\n");
			Write(fd1[1], " enlightenment/gain level\r");
		}
		else
		{
			fprintf(logfile, "IDed enlightenment\n");
			Write(fd1[1], " enlightenment\r");
		}
	}
	else if (strcasestr(message, "rise up through"))
	{
		fprintf(logfile, "IDed gain level\n");
		Write(fd1[1], " gain level\r");
	}
	else if (strcasestr(message, "fruit juice") || strcasestr(message, "tastes rotten"))
	{
		fprintf(logfile, "IDed fruit juice/see invisible\n");
		Write(fd1[1], " fruit juice/see invisible\r");
	}
	else if (strcasestr(message, "tasted foul"))
	{
		fprintf(logfile, "IDed gain ability\n");
		Write(fd1[1], " gain ability\r");
	}
	else if (strcasestr(message, "cosmic"))
	{
		fprintf(logfile, "IDed hallucination\n");
		Write(fd1[1], " hallucination\r");
	}
	else if (strcasestr(message, "this makes you feel"))
	{
		fprintf(logfile, "IDed restore ability\n");
		Write(fd1[1], " restore ability\r");
	}
	else if (strcasestr(message, "poison"))
	{
		fprintf(logfile, "IDed poison\n");
		Write(fd1[1], " sickness\r");
	}
	else if (strcasestr(message, "yawn"))
	{
		fprintf(logfile, "IDed sleeping\n");
		Write(fd1[1], " sleeping\n");
	}
	else
	{
		return 0;
	}
	return 1;
}

unsigned char getshieldtype(const char *message)
{
	int shieldtype = SHIELD_UNKNOWN;

	if (!strcasestr(message, "shield"))
	{
		return SHIELD_NONE;
	}
	if (strcasestr(message, "small"))
	{
		shieldtype = SHIELD_SMALL;
	}
	else if (strcasestr(message, "orcish") || strcasestr(message, "red"))
	{
		shieldtype = SHIELD_ORCISH;
	}
	else if (strcasestr(message, "Uruk") || strcasestr(message, "white"))
	{
		shieldtype = SHIELD_URUK;
	}
	else if (strcasestr(message, "elven") || strcasestr(message, "blue"))
	{
		shieldtype = SHIELD_ELVEN;
	}
	else if (strcasestr(message, "dwarv") || strcasestr(message, "round"))
	{
		shieldtype = SHIELD_DWARVISH;
	}
	else if (strcasestr(message, "large"))
	{
		shieldtype = SHIELD_LARGE;
	}
	else if (strcasestr(message, "reflection") || strcasestr(message, "polished"))
	{
		shieldtype = SHIELD_REFLECTION;
	}

	return shieldtype;
}

int gettooltype(const char *message)
{
	int tooltype = TOOL_NONE;

	if (strcasestr(message, "unicorn horn"))
	{
		tooltype = TOOL_UNIHORN;
	}
	else if (strcasestr(message, "lamp"))
	{
		tooltype = TOOL_LAMP;
	}

	return tooltype;
}

int getcorpsetype(const char *message)
{
	int corpsetype = CORPSE_NONE;

	if (strcasestr(message, "corpse"))
	{
		if (strcasestr(message, "lichen"))
		{
			corpsetype = CORPSE_LICHEN;
		}
		else if (strcasestr(message, "lizard"))
		{
			corpsetype = CORPSE_LIZARD;
		}
		else
		{
			corpsetype = CORPSE_UNKNOWN;
		}
	}

	return corpsetype;
}

void str2item(const char *line, item_t *item)
{
	int tmp;

	item->flags = 0;
	item->type = ITEM_UNKNOWN;
	item->buc = 2;
	item->enchantment = 0;
	item->container = '-';
	item->count = 1;

	if (strcasestr(line, "helm"))
	{
		fprintf(logfile, "line = %s\n", line);
	}

	item->letter = line[0];
	if (isdigit(line[4]))	// we have a count of items
	{
		sscanf(&line[4], "%i", &item->count);
	}
	tmp = getarmortype(line);
	if (tmp != ARMOR_NONE)
	{
		item->type = ITEM_ARMOR;
		item->subtype = tmp;
	}
	tmp = getpotiontype(line);
	if (tmp != POTION_NONE)
	{
		item->type = ITEM_POTION;
		item->subtype = tmp;
	}
	tmp = getshieldtype(line);
	if (tmp != SHIELD_NONE)
	{
		item->type = ITEM_SHIELD;
		item->subtype = tmp;
	}
	tmp = gettooltype(line);
	if (tmp != TOOL_NONE)
	{
		item->type = ITEM_TOOL;
		item->subtype = tmp;
	}
	tmp = gethelmtype(line);
	if (tmp != HELM_NONE)
	{
		item->type = ITEM_HELM;
		item->subtype = tmp;
	}
	/*tmp = getboottype(line);
	if (tmp != FOOTWEAR_NONE)
	{
		item->type = ITEM_FOOTWEAR;
		item->subtype = tmp;
	}*/
	tmp = getcorpsetype(line);
	if (tmp != CORPSE_NONE)
	{
		item->type = ITEM_CORPSE;
		item->subtype = tmp;
	}
	tmp = getdaggertype(line);
	if (tmp != DAGGER_NONE)
	{
		item->type = ITEM_DAGGER;
		item->subtype = tmp;
	}

	if (strcasestr(line, "worn"))
	{
		item->flags |= FLAG_WORN;
	}
	if (strcasestr(line, "lit"))
	{
		item->flags |= FLAG_LIT;
	}
	if (strcasestr(line, "weapon in hand"))
	{
		item->flags |= FLAG_WIELDED;
	}
	if (strcasestr(line, "unpaid"))
	{
		item->flags |= FLAG_UNPAID;
	}
}

void refreshinventory()
{
	int result;
	char buffer[4096];
	int timeout = 0;
	char *ptr;
	char *ptr2;
	char inventorystring[50][50];
	int count = 0;
	int idx;
	int idx2;

	Write(fd1[1], "i");

	sleep(1);

	while (1)
	{
		usleep(5000);
		result = read(fd2[0], buffer, 4096);
		if (result > 0)
		{
			buffer[result] = '\0';
			for (ptr = buffer; *ptr; ptr++)
			{
				if (ptr[1] == ' ' && ptr[2] == '-' && ptr[3] == ' ')
				{
					ptr2 = strstr(ptr, "\x1b");
					*ptr2 = '\0';
					strcpy(inventorystring[count], ptr);
					count++;
					ptr = ptr2;
				}
			}
		}
		else
		{
			timeout++;
			if (timeout >= 100)
			{
				break;
			}
		}
	}

	Write(fd1[1], "  ");

	inventorycount = count;
	bodyarmorpreference = 0;
	helmpreference = 0;
	for (idx = 0; idx < count; idx++)
	{
		if (strcasestr(inventorystring[idx], "worn"))
		{
			if (getarmortype(inventorystring[idx]) != ARMOR_NONE)
			{
				setarmorpref(inventorystring[idx]);
			}
			else if (gethelmtype(inventorystring[idx]) != HELM_NONE)
			{
				sethelmpref(inventorystring[idx]);
			}
		}
		str2item(inventorystring[idx], &inventory[idx]);
		if (strcasestr(inventorystring[idx], "stamped scroll") || (strcasestr(inventorystring[idx], "scroll") && strcasestr(inventorystring[idx], "of mail")))
		{
			sprintf(buffer, "r%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
			Write(fd1[1], "# Why do you people keep sending me mail? I grok vt100, not english!\r");
			sleep(2);
		}
	}
	for (idx = 0; idx < count; idx++)
	{
		if (inventory[idx].flags & FLAG_UNPAID)
		{
			continue;
		}
		if (inventory[idx].type == ITEM_ARMOR && armorquality(inventory[idx].subtype) < bodyarmorpreference && !(inventory[idx].flags & FLAG_WORN))
		{
			fprintf(logfile, "Dropping crappy armor %s (%c)\n", inventorystring[idx], inventory[idx].letter);
			sprintf(buffer, "d%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
		if (inventory[idx].type == ITEM_ARMOR && shouldweararmor2(&inventory[idx]))
		{
			for (idx2 = 0; idx2 < count; idx2++)
			{
				if (inventory[idx2].flags & FLAG_WORN && inventory[idx2].type == ITEM_ARMOR)
				{
					sprintf(buffer, "T%c  ", inventory[idx2].letter);
					Write(fd1[1], buffer);
				}
			}
			sprintf(buffer, "W%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
		else if (inventory[idx].type == ITEM_HELM && shouldwearhelm2(&inventory[idx]))
		{
			for (idx2 = 0; idx2 < count; idx2++)
			{
				if (inventory[idx2].flags & FLAG_WORN && inventory[idx2].type == ITEM_HELM)
				{
					sprintf(buffer, "T%c  ", inventory[idx2].letter);
					Write(fd1[1], buffer);
				}
			}
			sprintf(buffer, "W%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
		else if (inventory[idx].type == ITEM_POTION && inventory[idx].subtype == POTION_UNKNOWN)
		{
			sprintf(buffer, "q%c", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
		else if (inventory[idx].type == ITEM_TOOL && inventory[idx].subtype == TOOL_UNIHORN)
		{
			sprintf(buffer, "a%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
		else if (inventory[idx].type == ITEM_TOOL && inventory[idx].subtype == TOOL_LAMP && !(inventory[idx].flags & FLAG_LIT))
		{
			sprintf(buffer, "a%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
		else if (inventory[idx].type == ITEM_CORPSE && inventory[idx].subtype != CORPSE_LICHEN && inventory[idx].subtype != CORPSE_LIZARD)
		{
			sprintf(buffer, "d%c  ", inventory[idx].letter);
			Write(fd1[1], buffer);
		}
	}

	fprintf(logfile, "Refreshed inventory\n");
}

int countitems(int type, int subtype)
{
	int count = 0;
	int idx;

	for (idx = 0; idx < inventorycount; idx++)
	{
		if (inventory[idx].type != type)
		{
			continue;
		}
		if (subtype != -1 && inventory[idx].subtype != subtype)
		{
			continue;
		}
		count += inventory[idx].count;
	}

	return count;
}

item_t *getitem(int type, int subtype)
{
	int idx;

	for (idx = 0; idx < inventorycount; idx++)
	{
		if (inventory[idx].type != type)
		{
			continue;
		}
		if (subtype != -1 && inventory[idx].subtype != subtype)
		{
			continue;
		}
		return &inventory[idx];
	}

	return NULL;
}

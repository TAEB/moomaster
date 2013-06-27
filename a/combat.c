#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "moomaster.h"
#include "movement.h"
#include "combat.h"
#include "inventory.h"

extern char framebuffer[80 * 25];
extern int myx;
extern int myy;
extern char lastdirection;
extern FILE *logfile;
extern int monsterx;
extern int monstery;
extern int lastkilltimer;

unsigned char vulnerable(int dx, int dy)
{
	if (ismonster(framebuffer[dy * 80 + dx]))
	{
		return 3;
	}
	else if (isitem(framebuffer[dy * 80 + dx]))
	{
		return 1;
	}
	else if (corridorchar(framebuffer[dy * 80 + dx]))
	{
		return 1;
	}
	return 0;
}

int assessvulnerability(int dx, int dy)
{
	int vulnerability = 0;

	vulnerability += vulnerable(dx - 1, dy - 1);
	vulnerability += vulnerable(dx + 1, dy - 1);
	vulnerability += vulnerable(dx + 1, dy + 1);
	vulnerability += vulnerable(dx - 1, dy + 1);
	vulnerability += vulnerable(dx, dy - 1);
	vulnerability += vulnerable(dx + 1, dy);
	vulnerability += vulnerable(dx, dy + 1);
	vulnerability += vulnerable(dx - 1, dy);

	return vulnerability;
}

int scorecombatlocation(int dx, int dy)
{
	int score = 0;
	int distx;
	int disty;
	float dist;

	if (!corridorchar(framebuffer[dy * 80 + dx]))
	{
		return 9999999;
	}

	distx = dx - myx;
	disty = dy - myy;
	dist = sqrtf(dx * dx + dy * dy);

	score += assessvulnerability(dx, dy);
	score += dist * dist * 5.0;

	return score;
}

unsigned char straightline(int tx, int ty, float *dist, unsigned char *direction)
{
	int dx;
	int dy;

	dx = tx - myx;
	dy = ty - myy;
	*dist = sqrtf(dx * dx + dy * dy);
	if (dx == 0 && dy != 0)	// vertical straight line
	{
		if (dy > 0)	// below us
		{
			*direction = '2';
		}
		else
		{
			*direction = '8';
		}
		return 1;
	}
	else if (dx != 0 && dy == 0)	// horizontal straight line
	{
		if (dx > 0)
		{
			*direction = '6';
		}
		else
		{
			*direction = '4';
		}
		return 1;
	}
	else if (abs(dx) == abs(dy))	// diagonal
	{
		if (dx > 0 && dy > 0)
		{
			*direction = '3';
		}
		else if (dx > 0 && dy < 0)
		{
			*direction = '9';
		}
		else if (dx < 0 && dy < 0)
		{
			*direction = '7';
		}
		else if (dx < 0 && dy > 0)
		{
			*direction = '1';
		}
		return 1;
	}
	return 0;
}

const char *handlecombat(int tx, int ty, char target)
{
	static char action[16];
	int idx;
	int monstercount = 0;
	int lowestscore = 9999999;
	int lowestx;
	int lowesty;
	int score;
	float dist;
	unsigned char direction;
	item_t *daggeritem;

	// this doesn't work but hell if I know why not
	if (target == 'Z' || target == 'M' || target == 'V' || target == '@' || target == 'c' || target == 'l' || target == 'n')
	{
		lastkilltimer = 999;
		monsterx = 0;
		monstery = 0;
	}
	else
	{
		lastkilltimer = 0;
		monsterx = tx;
		monstery = ty;
	}

	for (idx = 80; idx < 80 * 22; idx++)
	{
		if (ismonster(framebuffer[idx]) && ((idx % 80) != myx || (idx / 80) != myy) && !(strcasestr(framebuffer, "see here") && tx >= 58 && ty <= 3))
		{
			monstercount++;
		}
	}

	// see if we should fling daggers
	if (straightline(tx, ty, &dist, &direction) && (dist > 1.5 || target == NOTOUCH_MONSTER) && dist < 5.0 && countitems(ITEM_DAGGER, -1))
	{
		daggeritem = getitem(ITEM_DAGGER, -1);
		fprintf(logfile, "flinging daggers @ %c (%f distance)\n", direction, dist);
		sprintf(action, "t%c%c   ", daggeritem->letter, direction);
		daggeritem->count--;
		return action;
	}

	if (1 || monstercount <= 2)
	{
		// don't worry about advanced tactics when not facing large
		// numbers of opponents
		lastdirection = pathto(tx, ty, target, 0);
		fprintf(logfile, "naively attacking %i.%i by moving %c\n", tx, ty, lastdirection);
		sprintf(action, "%c", lastdirection);
	}
	else
	{
		if (abs(tx - myx) <= 1 && abs(ty - myy) <= 1)
		{
			lastdirection = pathto(tx, ty, target, 0);
			fprintf(logfile, "monster is next to us. Moving %c\n", lastdirection);
			sprintf(action, "%c", lastdirection);
			return action;
		}
		if (assessvulnerability(myx, myy) >= VULNERABILITY_CUTOFF)
		{
			fprintf(logfile, "Current location (%i.%i) is a little hot. Moving to somewhere more secure.\n", myx, myy);
			for (idx = 80; idx < 80 * 22; idx++)
			{
				if (!(idx % 80) || (idx % 80) == 79)
				{
					continue;
				}
				score = scorecombatlocation(idx % 80, idx / 80);
				if (score < lowestscore)
				{
					lowestscore = score;
					lowestx = idx % 80;
					lowesty = idx / 80;
				}
			}
			if (lowestscore < 9999999)
			{
				fprintf(logfile, "%i.%i seems to be a comfy spot to fight (score = %i)\n", lowestx, lowesty, lowestscore);
				lastdirection = pathto(lowestx, lowesty, framebuffer[lowesty * 80 + lowestx], 0);
				sprintf(action, "%c", lastdirection);
			}
		}
		else
		{
			lastdirection = pathto(tx, ty, target, 0);
			fprintf(logfile, "monster is NOT next to us. Moving %c\n", lastdirection);
			sprintf(action, "%c", lastdirection);
		}
	}
	return action;
}

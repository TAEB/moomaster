#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "moomaster.h"
#include "movement.h"
#include "combat.h"
#include "inventory.h"

extern char framebuffer[80 * 25];
extern int notebuffer[80 * 25];
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

	if (notebuffer[dy * 80 + dx] & NOTE_FOUNTAIN)
	{
		vulnerability += 10;
	}

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
	if (dist >= 4.0)
	{
		return 9999999;
	}

	return score;
}

unsigned char straightline(int tx, int ty, float *dist, unsigned char *direction)
{
	int fx = myx;
	int fy = myy;
	int dx;
	int dy;

	dx = tx - myx;
	dy = ty - myy;

	while (fx != tx || fy != ty)
	{
		if (framebuffer[fy * 80 + fx] == PEACEFUL_MONSTER)
		{
			return 0;
		}
		if (tx < fx)
		{
			fx--;
		}
		else if (tx > fx)
		{
			fx++;
		}
		if (ty < fy)
		{
			fy--;
		}
		else if (ty > fy)
		{
			fy++;
		}
		if (framebuffer[fy * 80 + fx] == PEACEFUL_MONSTER)
		{
			return 0;
		}
	}

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

unsigned char emptysquare(int x, int y, unsigned char allowseenfloor)
{
	if (corridorchar(framebuffer[y * 80 + x]))
	{
		return 1;
	}
	if (allowseenfloor && notebuffer[y * 80 + x] & NOTE_SEENFLOOR)
	{
		return 1;
	}
	return 0;
}

char findemptyadjacent()
{
	int x;
	int y;
	int pass;

	for (pass = 0; pass < 2; pass++)
	{
		x = myx + 1;
		y = myy - 1;
		if (emptysquare(x, y, pass))
		{
			return '9';
		}
		x = myx;
		y = myy - 1;
		if (emptysquare(x, y, pass))
		{
			return '8';
		}
		x = myx - 1;
		y = myy - 1;
		if (emptysquare(x, y, pass))
		{
			return '7';
		}
		x = myx + 1;
		y = myy;
		if (emptysquare(x, y, pass))
		{
			return '6';
		}
		x = myx - 1;
		y = myy;
		if (emptysquare(x, y, pass))
		{
			return '4';
		}
		x = myx + 1;
		y = myy + 1;
		if (emptysquare(x, y, pass))
		{
			return '3';
		}
		x = myx;
		y = myy + 1;
		if (emptysquare(x, y, pass))
		{
			return '2';
		}
		x = myx - 1;
		y = myy + 1;
		if (emptysquare(x, y, pass))
		{
			return '1';
		}
	}

	return '.';
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
	int dx;
	int dy;

	// this doesn't work but hell if I know why not
	if (target == 'Z' || target == 'M' || target == 'V' || target == '@' || target == 'c' || target == 'l' || target == 'n' || target == 'X')
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
	if (straightline(tx, ty, &dist, &direction) && countitems(ITEM_WEAPON, WEAPON_DAGGER))
	{
		if ((dist > 1.5 || target == NOTOUCH_MONSTER) && dist < 5.0)
		{
			daggeritem = getitem(ITEM_WEAPON, WEAPON_DAGGER);
			fprintf(logfile, "flinging daggers @ %c (%f distance)\n", direction, dist);
			sprintf(action, "t%c%c   ", daggeritem->letter, direction);
			daggeritem->count--;
			return action;
		}
	}

	if (notebuffer[myy * 80 + myx] & NOTE_FOUNTAIN)
	{
		lastdirection = findemptyadjacent();
		if (lastdirection != '.')
		{
			sprintf(action, "%c", lastdirection);
			return action;
		}
	}
	// assume that if we're fighting an a, there are others nearby. We'd
	// best find somewhere nice to fight.
	if (monstercount <= 2 && target != 'a')
	{
		// don't worry about advanced tactics when not facing large
		// numbers of opponents
		if (countitems(ITEM_WEAPON, WEAPON_DAGGER) && (target == RANGED_MONSTER || target == NYMPH_MONSTER))
		{
			dx = tx - myx;
			dy = ty - myy;
			fprintf(logfile, "We are combating a nymph, trying to move to get in line with her\n");
			if (abs(dx) < abs(dy) && (abs(dx) > 1 || abs(dy) > 1))
			{
				if (dx > 0 && !impassable(framebuffer[dy * 80 + dx + 1], dx + 1, dy))
				{
					return "6";
				}
				else if (dx < 0 && !impassable(framebuffer[dy * 80 + dx - 1], dx - 1, dy))
				{
					return "4";
				}
			}
			else if (abs(dx) > 1 || abs(dy) > 1)
			{
				if (dy > 0 && !impassable(framebuffer[(dy + 1) * 80 + dx], dx, dy + 1))
				{
					return "2";
				}
				else if (dy < 0 && !impassable(framebuffer[(dy - 1) * 80 + dx], dx, dy - 1))
				{
					return "8";
				}
			}
			fprintf(logfile, "Unable to get in line with nymph. Proceeding with normal combat\n");
		}
		lastdirection = pathto(tx, ty, target, 0);
		fprintf(logfile, "naively attacking %i.%i by moving %c\n", tx, ty, lastdirection);
		sprintf(action, "%c", lastdirection);
	}
	else
	{
		if (assessvulnerability(myx, myy) >= VULNERABILITY_CUTOFF)
		{
			fprintf(logfile, "Current location (%i.%i) is a little hot. Moving to somewhere more secure.\n", myx, myy);
			lowestscore = assessvulnerability(myx, myy);
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
			if (lowestscore < assessvulnerability(myx, myy))
			{
				fprintf(logfile, "%i.%i seems to be a comfy spot to fight (score = %i)\n", lowestx, lowesty, lowestscore);
				lastdirection = pathto(lowestx, lowesty, framebuffer[lowesty * 80 + lowestx], 0);
				sprintf(action, "%c", lastdirection);
			}
			else
			{
				fprintf(logfile, "There's no place like home. Staying put\n");
				goto combat;
			}
		}
		else
		{
			combat:
			if ((abs(tx - myx) <= 1 && abs(ty - myy) <= 1) || immobilemonster(target))
			{
				docombat:
				lastdirection = pathto(tx, ty, target, 0);
				fprintf(logfile, "monster is next to us. Attacking %c\n", lastdirection);
				sprintf(action, "%c", lastdirection);
				return action;
			}
			else
			{
				// leps tend to come in groups and are often
				// asleep
				if (target == 'l')
				{
					goto docombat;
				}
				//lastdirection = pathto(tx, ty, target, 0);
				//fprintf(logfile, "monster is NOT next to us. Moving %c\n", lastdirection);
				//sprintf(action, "%c", lastdirection);
				fprintf(logfile, "monster is NOT next to us. Waiting.\n");
				sprintf(action, "s");
				return action;
			}
		}
	}
	return action;
}

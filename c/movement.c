#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "movement.h"
#include "moomaster.h"

extern char framebuffer[80 * 25];
extern int trafficbuffer[80 * 25];
extern int notebuffer[80 * 25];

extern int myx;
extern int myy;
extern int turncount;
extern node_t node[1024];
extern int nodecount;
extern FILE *logfile;

const char traffic_char[] = {
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'A',
	'B',
	'C',
	'D',
	'E',
	'F',
	'G',
	'H',
	'I',
	'J',
	'K',
	'L',
	'M',
	'N',
	'O',
	'P',
	'Q',
	'R',
	'S',
	'T',
	'U',
	'V',
	'W',
	'X',
	'Y',
	'Z',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
	'c',
	'h',
	'i',
	'j',
	'k',
	'l',
	'm',
	'n',
	'o',
	'p',
	'q',
	'r',
	's',
	't',
	'u',
	'v',
	'w',
	'x',
	'y',
	'z',
};

unsigned char impassable(char c, int x, int y)
{
	if (c == ' ')
	{
		if (trafficbuffer[y * 80 + x] < traffictimeout() && (framebuffer[y * 80 + x - 1] == '.' || framebuffer[y * 80 + x + 1] == '.' || framebuffer[(y - 1) * 80 + x] == '.' || framebuffer[(y + 1) * 80 + x] == '.'))
		{
			//trafficbuffer[y * 80 + x]++;
			return 0;
		}
		return 1;
	}
	return (c == '|' || c == '-' || c == '0' || c == '+' || c == PEACEFUL_MONSTER || c == NOTOUCH_MONSTER);
}

unsigned char isdoorway(int x, int y, unsigned char isdoorchar)
{
	int idx = y * 80 + x;
	int count = 0;
	int count2 = 0;
	int count3 = 0;

	if (framebuffer[idx + 1] == '-' || framebuffer[idx + 1] == '|')
	{
		count++;
	}
	if (framebuffer[idx - 1] == '-' || framebuffer[idx - 1] == '|')
	{
		count++;
	}
	if (framebuffer[idx + 80] == '-' || framebuffer[idx + 80] == '|')
	{
		count2++;
	}
	if (framebuffer[idx - 80] == '-' || framebuffer[idx - 80] == '|')
	{
		count2++;
	}
	if (framebuffer[idx + 1] == ' ')
	{
		count3++;
	}
	if (framebuffer[idx - 1] == ' ')
	{
		count3++;
	}
	if (framebuffer[idx + 80] == ' ')
	{
		count3++;
	}
	if (framebuffer[idx - 80] == ' ')
	{
		count3++;
	}

	return ((count == 2) || (count2 == 2) || (isdoorchar == 0xFF && count3 == 2) || (isdoorchar == 0xFE && count3 == 3));
}

void whereidbe(char direction, int *x, int *y)
{
	*x = myx;
	*y = myy;

	if (direction == '7' || direction == '4' || direction == '1')
	{
		(*x)--;
	}
	else if (direction == '9' || direction == '6' || direction == '3')
	{
		(*x)++;
	}
	if (direction == '7' || direction == '8' || direction == '9')
	{
		(*y)--;
	}
	else if (direction == '1' || direction == '2' || direction == '3')
	{
		(*y)++;
	}
}

char pickrandomdirection()
{
	char direction;
	int x;
	int y;
	int timeout = 100;

	do
	{
		direction = '1' + (rand() % 9);
		whereidbe(direction, &x, &y);
		if (!impassable(framebuffer[y * 80 + x], x, y))
		{
			break;
		}
		timeout--;
	} while (timeout);

	if (!timeout)
	{
		return 's';
	}

	return direction;
}

int traffictimeout()
{
	return 6 + (turncount / 250) * 2;
}

int findunexploredarea(int *x, int *y, char *target)
{
	int idx;
	int leasttraffic = 999;
	int leastx;
	int leasty;

	for (idx = 0; idx < 80 * 25; idx++)
	{
		if (framebuffer[idx] == '.' && (framebuffer[idx + 1] == ' ' || framebuffer[idx - 1] == ' ' || framebuffer[idx + 80] == ' ' || framebuffer[idx - 80] == ' ') && pathto(idx % 80, idx / 80, framebuffer[idx], 1) != '!')
		{
			if (trafficbuffer[idx] < leasttraffic)
			{
				leastx = idx % 80;
				leasty = idx / 80;
				leasttraffic = trafficbuffer[idx];
			}
		}
	}

	if (leasttraffic >= traffictimeout() / 2)
	{
		//fprintf(logfile, "Unable to find interesting area\n");
		return 0;
	}

	*x = leastx;
	*y = leasty;
	*target = framebuffer[leasty * 80 + leastx];
	if (abs(leastx - myx) <= 2 && abs(leasty - myy) <= 2)
	{
		trafficbuffer[leasty * 80 + leastx]++;
	}
	fprintf(logfile, "Found interesting area @ %i.%i (traffic'd %i, timeout %i)\n", *x, *y, trafficbuffer[leasty * 80 + leastx], traffictimeout());

	return 1;
}

void searchfromleft(int *leasttraffic, int *leastx, int *leasty)
{
	int tx;
	int ty;
	int idx;

	for (tx = 0; tx < 80; tx++)
	{
		for (ty = 1; ty < 22; ty++)
		{
			idx = ty * 80 + tx;
			if ((framebuffer[idx] == '-' || framebuffer[idx] == '|'))
			{
				if (trafficbuffer[idx] < *leasttraffic)
				{
					*leasttraffic = trafficbuffer[idx];
					*leastx = idx % 80;
					*leasty = idx / 80;
				}
			}
		}
	}
}

void searchfromright(int *leasttraffic, int *leastx, int *leasty)
{
	int tx;
	int ty;
	int idx;

	for (tx = 80; tx > 0; tx--)
	{
		for (ty = 1; ty < 22; ty++)
		{
			idx = ty * 80 + tx;
			if ((framebuffer[idx] == '-' || framebuffer[idx] == '|'))
			{
				if (trafficbuffer[idx] < *leasttraffic)
				{
					*leasttraffic = trafficbuffer[idx];
					*leastx = idx % 80;
					*leasty = idx / 80;
				}
			}
		}
	}
}

int findinterestingwall(int *x, int *y, char *target)
{
	int tx;
	int ty;
	int idx;
	int leasttraffic = 999;
	int leastx;
	int leasty;
	int timeout = 0;
	int exploreweight[2] = { 0, 0 };

	for (tx = 0; tx < 40; tx++)
	{
		for (ty = 1; ty < 22; ty++)
		{
			if (framebuffer[ty * 80 + tx] != ' ' && framebuffer[ty * 80 + tx] != 0x00)
			{
				exploreweight[0]++;
			}
		}
	}
	for (tx = 40; tx < 80; tx++)
	{
		for (ty = 1; ty < 22; ty++)
		{
			if (framebuffer[ty * 80 + tx] != ' ' && framebuffer[ty * 80 + tx] != 0x00)
			{
				exploreweight[1]++;
			}
		}
	}

	again:
	if (exploreweight[0] < exploreweight[1])
	{
		searchfromleft(&leasttraffic, &leastx, &leasty);
	}
	else
	{
		searchfromright(&leasttraffic, &leastx, &leasty);
	}

	if (leasttraffic >= traffictimeout() * 2)
	{
		//fprintf(logfile, "Unable to find interesting wall\n");
		return 0;
	}

	if (pathto(leastx, leasty, framebuffer[leasty * 80 + leastx], 1) == '!')
	{
		trafficbuffer[leasty * 80 + leastx]++;
		timeout++;
		if (timeout >= 100)
		{
			*x = 0;
			*y = 0;
			//fprintf(logfile, "Unable to find interesting wall (timeout)\n");
			return 0;
		}
		goto again;
	}
	if (abs(leastx - myx) <= 1 && abs(leasty - myy) <= 1)
	{
		trafficbuffer[leasty * 80 + leastx]++;
	}
	*x = leastx;
	*y = leasty;
	*target = framebuffer[leasty * 80 + leastx];
	fprintf(logfile, "Found interesting wall (%c) @ %i.%i (traffic'd %i, timeout %i)\n", *target, *x, *y, trafficbuffer[leasty * 80 + leastx], traffictimeout());

	return 1;
}

void findinterestingcorridor(int *x, int *y)
{
	int idx;
	int count;
	int count2;
	int count3;
	int count4;
	int count5;
	int closestx = 0;
	int closesty = 0;
	int closestdist = 0;
	int xdist;
	int ydist;
	int dist;
	int pass = 1;

	*x = 0;
	*y = 0;

	startover:
	for (idx = 80; idx < 80 * 22; idx++)
	{
		if (trafficbuffer[idx] >= traffictimeout())
		{
			continue;
		}
		if (pass == 1 && (framebuffer[idx] == '#' || framebuffer[idx] == '.' || isitem(framebuffer[idx]) || framebuffer[idx] == 'K' || framebuffer[idx] == '@'))
		{
			count = 0;
			if (corridorchar(framebuffer[idx - 80]))
			{
				count++;
			}
			if (corridorchar(framebuffer[idx + 80]))
			{
				count++;
			}
			if (corridorchar(framebuffer[idx - 1]))
			{
				count++;
			}
			if (corridorchar(framebuffer[idx + 1]))
			{
				count++;
			}
			if (count == 1)
			{
				*x = idx % 80;
				*y = idx / 80;
				xdist = *x - myx;
				ydist = *y - myy;
				dist = (int)(sqrtf(xdist * xdist + ydist * ydist) * 100);
				if ((dist < closestdist || closestdist == 0) && pathto(*x, *y, framebuffer[idx], 1) != '!')
				{
					closestx = *x;
					closesty = *y;
					closestdist = dist;
				}
			}
		}
		if (pass == 2 && (framebuffer[idx] == '#' || framebuffer[idx] == '@') && trafficbuffer[idx] < 5)
		{
			count = 0;
			count2 = 0;
			count3 = 0;
			count4 = 0;
			count5 = 0;
			if (framebuffer[idx - 81] == '#' || framebuffer[idx - 81] == '@')
			{
				count2++;
				count4++;
			}
			if (framebuffer[idx - 80] == '#' || framebuffer[idx - 80] == '@')
			{
				count++;
				count4++;
			}
			if (framebuffer[idx - 79] == '#' || framebuffer[idx - 79] == '@')
			{
				count2++;
				count4++;
			}
			if (framebuffer[idx - 1] == '#' || framebuffer[idx - 1] == '@')
			{
				count3++;
			}
			if (framebuffer[idx + 1] == '#' || framebuffer[idx + 1] == '@')
			{
				count3++;
			}
			if (framebuffer[idx + 79] == '#' || framebuffer[idx + 79] == '2')
			{
				count2++;
				count5++;
			}
			if (framebuffer[idx + 80] == '#' || framebuffer[idx + 80] == '@')
			{
				count++;
				count5++;
			}
			if (framebuffer[idx + 81] == '#' || framebuffer[idx + 81] == '@')
			{
				count2++;
				count5++;
			}
			if (count == 1 && count2 == 1 && count3 == 1 && (count4 == 2 || count5 == 2))
			{
				//fprintf(logfile, "Got match @ %i.%i with traffic %i\n", idx % 80, idx / 80, trafficbuffer[idx]);
				*x = idx % 80;
				*y = idx / 80;
				xdist = *x - myx;
				ydist = *y - myy;
				dist = (int)(sqrtf(xdist * xdist + ydist * ydist) * 100);
				if ((dist < closestdist || closestdist == 0) && pathto(*x, *y, framebuffer[idx], 1) != '!')
				{
					closestx = *x;
					closesty = *y;
					closestdist = dist;
				}
			}
		}
	}
	if (closestx && closesty)
	{
		*x = closestx;
		*y = closesty;
		fprintf(logfile, "Found pass %i interesting corridor @ %i.%i (%i away, traffic'd %i, timeout %i)\n", pass, *x, *y, closestdist, trafficbuffer[closesty * 80 + closestx], traffictimeout());
		if (pass == 2 && (abs(closestx - myx) <= 2 && abs(closesty - myy) <= 2))
		{
			trafficbuffer[closesty * 80 + closestx]++;
		}
	}
	else
	{
		pass++;
		if (pass >= 3)
		{
			//fprintf(logfile, "Unable to find interesting corridor.\n");
			*x = 0;
			*y = 0;
			return;
		}
		goto startover;
	}
}

int makescore(int dx, int dy, int x, int y, char target)
{
	float fx;
	float fy;
	float dist;
	int score;

	fx = x - dx;
	fy = y - dy;
	dist = sqrtf(fx * fx + fy * fy);
	score = dist * 1000;
	if (framebuffer[y * 80 + x] == '-' || framebuffer[y * 80 + x] == '|' || framebuffer[y * 80 + x] == ' ' || (framebuffer[y * 80 + x] == '+' && (target != '+' || x != dx || y != dy)))
	{
		score += 10000;
	}

	return score;
}

char trafficchar(int index)
{
	return traffic_char[index % 62];
}

void dump_traffic()
{
	int idx;
	FILE *trafficout;

	trafficout = fopen("traffic.txt", "wb");
	if (!trafficout)
	{
		fprintf(logfile, "Can't open traffic.txt\n");
		return;
	}

	for (idx = 0; idx < 80 * 25; idx++)
	{
		if (framebuffer[idx] == ' ' && trafficbuffer[idx] == 0)
		{
			fprintf(trafficout, " ");
		}
		else if (traffictimeout() > 0)
		{
			fprintf(trafficout, "%c", trafficchar(trafficbuffer[idx] / (traffictimeout() / 5)));
		}
		else
		{
			fprintf(trafficout, "%c", trafficchar(trafficbuffer[idx] / 10));
		}
		if (idx % 80 == 79)
		{
			fprintf(trafficout, "\n");
		}
	}

	fclose(trafficout);
}

void dump_nodes()
{
	int idx;
	int idx2;
	FILE *nodeout;

	nodeout = fopen("nodes.txt", "wb");
	if (!nodeout)
	{
		fprintf(logfile, "Can't open nodes.txt\n");
		return;
	}

	for (idx = 0; idx < 80 * 25; idx++)
	{
		for (idx2 = 0; idx2 < nodecount; idx2++)
		{
			if (node[idx2].x == (idx % 80) && node[idx2].y == (idx / 80))
			{
				if (node[idx2].closed)
				{
					fputc('c', nodeout);
				}
				else
				{
					fputc('o', nodeout);
				}
				break;
			}
		}
		if (idx2 == nodecount)
		{
			fputc(' ', nodeout);
		}
		if (idx % 80 == 79)
		{
			fputc('\n', nodeout);
		}
	}

	fclose(nodeout);
}

void spawnnode(int x, int y, char direction, node_t *parent, char target, int targetx, int targety)
{
	int idx;
	int tx;
	int ty;

	if (nodecount >= 2048)
	{
		dump_nodes();
		fprintf(logfile, "Ran out of nodes to %c @ %i.%i (I'm at %i.%i)\n", target, targetx, targety, myx, myy);
		return;
	}

	if (y < 2 || y > 22)
	{
		return;
	}

	whereidbe(direction, &tx, &ty);
	if (notebuffer[myy * 80 + myx] & NOTE_OPENDOOR || notebuffer[ty * 80 + tx] & NOTE_OPENDOOR)
	{
		if (notebuffer[ty * 80 + tx] & NOTE_OPENDOOR && ismonster(target) && targetx == tx && targety == ty)
		{
			// just trap against the next if
		}
		else if (direction == '1' || direction == '3' || direction == '7' || direction == '9')
		{
			return;
		}
	}
	/*if (notebuffer[ty * 80 + tx] & NOTE_TRAP)
	{
		return;
	}*/

	for (idx = 0; idx < nodecount; idx++)
	{
		if (node[idx].x == x && node[idx].y == y)
		{
			return;
		}
	}

	if (impassable(framebuffer[y * 80 + x], x, y) && (framebuffer[y * 80 + x] != target || x != targetx || y != targety))
	{
		return;
	}
	if (framebuffer[y * 80 + x] == ' ' && (target != ' ' || x != targetx || y != targety))
	{
		return;
	}

	node[nodecount].x = x;
	node[nodecount].y = y;
	node[nodecount].closed = 0;
	node[nodecount].direction = direction;
	node[nodecount].parent = parent;
	nodecount++;
}

void spawnnodesaround(int x, int y, node_t *parent, char target, int targetx, int targety)
{
	spawnnode(x - 1, y + 1, '1', parent, target, targetx, targety);
	spawnnode(x, y + 1, '2', parent, target, targetx, targety);
	spawnnode(x + 1, y + 1, '3', parent, target, targetx, targety);
	spawnnode(x - 1, y, '4', parent, target, targetx, targety);
	spawnnode(x + 1, y, '6', parent, target, targetx, targety);
	spawnnode(x - 1, y - 1, '7', parent, target, targetx, targety);
	spawnnode(x, y - 1, '8', parent, target, targetx, targety);
	spawnnode(x + 1, y - 1, '9', parent, target, targetx, targety);
}

char pathto(int x, int y, char target, unsigned char test)
{
	int idx;
	int least = 9999999;
	int leastidx = -1;
	int timeout = 0;
	node_t *nodeptr;
	int oldnodecount;
	char tmpact;

	nodecount = 0;
	spawnnodesaround(myx, myy, NULL, target, x, y);
	for (idx = 0; idx < nodecount; idx++)
	{
		if (node[idx].x == x && node[idx].y == y)
		{
			if (node[idx].direction == '5')
			{
				node[idx].direction = '.';
			}
			if (!test)
			{
				dump_nodes();
				fprintf(logfile, "Resolved %i.%i as %c (I'm at %i.%i)\n", x, y, node[idx].direction, myx, myy);
			}
			return node[idx].direction;
		}
	}

	do
	{
		oldnodecount = nodecount;
		for (idx = 0; idx < oldnodecount; idx++)
		{
			if (!node[idx].closed)
			{
				spawnnodesaround(node[idx].x, node[idx].y, &node[idx], target, x, y);
				node[idx].closed = 1;
			}
		}
		for (idx = 0; idx < nodecount; idx++)
		{
			if (node[idx].closed)
			{
				continue;
			}
			if (node[idx].x == x && node[idx].y == y)
			{
				nodeptr = &node[idx];
				while (nodeptr->parent)
				{
					nodeptr = nodeptr->parent;
				}
				if (!test)
				{
					dump_nodes();
					fprintf(logfile, "Resolved %i.%i as %c (I'm at %i.%i)\n", x, y, nodeptr->direction, myx, myy);
				}
				if (nodeptr->direction == '5')
				{
					nodeptr->direction = '.';
				}
				return nodeptr->direction;
			}
		}
		timeout++;
	} while (timeout < 200);

	if (test)
	{
		return '!';
	}

	getact:
	tmpact = (rand() % 2 ? ('s') : pickrandomdirection());
	if (tmpact == '5')
	{
		goto getact;
	}

	if (test)
	{
		dump_nodes();
		fprintf(logfile, "Unable to resolve %i.%i (%c), I am at %i.%i. Searching\n", x, y, target, myx, myy);
	}

	return tmpact;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include "movement.h"
#include "moomaster.h"

extern char framebuffer[80 * 25];
extern int trafficbuffer[80 * 25];
extern int notebuffer[80 * 25];

extern int myx;
extern int myy;
extern int turncount;
extern FILE *logfile;

typedef struct
{
	int x;
	int y;
	float dist;
} unexploredarea_t;

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
	return 6 + (turncount / 450) * 2;
}

int findunexploredarea(int *x, int *y, char *target)
{
	int idx;
	float leastdist = 99999999.9;
	int leasttraffic = 999;
	int leastx;
	int leasty;
	unexploredarea_t unexploredarea[1024];
	int unexploredareacount = 0;
	int dx;
	int dy;

	for (idx = 0; idx < 80 * 25; idx++)
	{
		if ((framebuffer[idx] == '.') && (framebuffer[idx + 1] == ' ' || framebuffer[idx - 1] == ' ' || framebuffer[idx + 80] == ' ' || framebuffer[idx - 80] == ' ') && pathto(idx % 80, idx / 80, framebuffer[idx], 1) != '!')
		{
			if (trafficbuffer[idx] < leasttraffic)
			{
				leastx = idx % 80;
				leasty = idx / 80;
				leasttraffic = trafficbuffer[idx];
				unexploredareacount = 0;
			}
			if (trafficbuffer[idx] == leasttraffic && unexploredareacount >= 0 && unexploredareacount < 1024)
			{
				unexploredarea[unexploredareacount].x = idx % 80;
				unexploredarea[unexploredareacount].y = idx / 80;
				dx = (idx % 80) - myx;
				dy = (idx / 80) - myy;
				unexploredarea[unexploredareacount].dist = sqrtf(dx * dx + dy * dy);
				unexploredareacount++;
			}
		}
	}

	if (leasttraffic >= traffictimeout() / 2 || !unexploredareacount)
	{
		//fprintf(logfile, "Unable to find interesting area\n");
		return 0;
	}

	leastdist = 9999999.9;
	for (idx = 0; idx < unexploredareacount; idx++)
	{
		if (unexploredarea[idx].dist < leastdist)
		{
			leastdist = unexploredarea[idx].dist;
			leastx = unexploredarea[idx].x;
			leasty = unexploredarea[idx].y;
		}
	}
	if (leastdist >= 9999999.0)
	{
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

static node_t nodes[80*24];
static int to_x, to_y;
static int from_x, from_y;
static char to_object;

static int dxes[8] = { -1,  0,  1, -1,  1, -1,  0,  1 };
static int dyes[8] = {  1,  1,  1,  0,  0, -1, -1, -1 };
static char dch[9] = {'1','2','3','4','6','7','8','9','s' };

void dump_nodes()
{
	int idx;
	FILE *nodeout;

	nodeout = fopen("nodes.txt", "wb");
	if (!nodeout)
	{
		fprintf(logfile, "Can't open nodes.txt\n");
		return;
	}

	int x = from_x, y = from_y;
	
	if (nodes[y*80 + x].score != SCORE_UNTOUCHED)
	{
		while (x != to_x || y != to_y)
		{
			int dir = nodes[y*80 + x].dir;
			nodes[y*80 + x].dir |= 128;
			x += dxes[dir];
			y += dyes[dir];
		}
	}


	for (idx = 0; idx < 80 * 24; idx++)
	{
		if (nodes[idx].score == SCORE_UNTOUCHED)
		{
			fputc(' ', nodeout);
		}
		else if (nodes[idx].dir & 128)
		{
			fputc('*', nodeout);
		}
		else if (nodes[idx].score == SCORE_CLOSED)
		{
			fputc('.', nodeout);
		}
		else
		{
			fputc('?', nodeout);
		}

		nodes[idx].dir &= 127;

		if (idx % 80 == 79)
		{
			fputc('\n', nodeout);
		}
	}

	fclose(nodeout);
}

/* Consider moving TO (x,y) BY dir; to reach the destination from
 * (x,y) took pathcost.
 */
void evaluate(int x, int y, int dir, int pathcost)
{
	int dx = dxes[dir], dy = dyes[dir];
	int fx = x - dx, fy = y - dy;
	int ix = 80*y + x, fix = 80*fy + fx;

	int newscore = pathcost + 1 + heuristic(fx,fy);

	node_t *from_node = &nodes[fix];

	/* Let's not be pathing off the map */
	if (fx < 0 || fx >= 80 || fy < 1 || fy >= 22)
		return;

	if (x == to_x && y == to_y && ismonster(to_object))
	{
		/* Special rule for attacking monsters - normal terrain
		 * effects do not apply in this case
		 */
	}
	else
	{
		/* Don't path into impassible terrain */
		if (impassable(framebuffer[ix], x, y))
			return;

		if (dx && dy)
		{
			/* Moving diagonally!  Check if there are doorways. */
			if (notebuffer[fix] & NOTE_OPENDOOR)
				return;

			if (notebuffer[ix] & NOTE_OPENDOOR)
				return;
		}

		/* Don't path into traps unless forced */
		if (notebuffer[ix] & NOTE_TRAP)
			newscore += 1000; /* prefer a trap over 1001 squares of walking */
	}

	if (from_node->score != SCORE_CLOSED && newscore < from_node->score)
	{
		/* Ooh.  A better way to solve (fx,fy). */
		from_node->score = newscore;
		from_node->dir = dir;
	}
}

/* Guess how hard it will be to get from from_x,from_y to here.  Must
 * never overestimate.
 */
int
heuristic(int x, int y)
{
	int d1 = from_x - x;
	int d2 = from_y - y;

	if (d1 < 0) d1 = -d1;
	if (d2 < 0) d2 = -d2;

	return (d1 > d2) ? d1 : d2;
}

/* Attempt to find a path FROM (fx,fy) TO (tx,ty) (which is a tc).
 * Return '!' if no path is found.
 */
char
findpath(int fx, int fy, int tx, int ty, char tc)
{
	int ix;

	from_x = fx; from_y = fy;
	to_x = tx; to_y = ty; to_object = tc;

	/* Mark all squares as unsolved */
	for (ix = 0; ix < 80*24; ix++)
		nodes[ix].score = SCORE_UNTOUCHED;

	/* We can solve the destination trivially */
	nodes[ty*80 + tx].score = heuristic(tx,ty);
	nodes[ty*80 + tx].dir   = 8;

	while(1)
	{
		int bestix = 0, dir, bx, by;
		unsigned int oldpathcost;
		/* Find a node that has been optimally solved */
		for (ix = 0; ix < 80*24; ix++)
		{
			if (nodes[ix].score < nodes[bestix].score)
			{
				bestix = ix;
			}
		}

		if (nodes[bestix].score >= SCORE_UNTOUCHED)
		{
			/* Urk.  No path. */
			return '!';
		}

		bx = bestix % 80; by = bestix / 80;

		/* Have we found the source? */
		if (bestix == (80*fy + fx))
			return dch[nodes[bestix].dir];

		/* Try to expand the solution */
		oldpathcost = nodes[bestix].score - heuristic(bx, by);
		nodes[bestix].score = SCORE_CLOSED;

		for (dir = 0; dir < 8; dir++)
		{
			evaluate(bx, by, dir, oldpathcost);
		}
	}
}

char pathto(int x, int y, char target, unsigned char test)
{
	char tmpact;
	char v = findpath(myx, myy, x, y, target);

	if (!test)
		dump_nodes();

	if (test || v != '!')
		return v;

	getact:
	tmpact = ((rand() % 2) ? ('s') : pickrandomdirection());
	if (tmpact == '5')
	{
		goto getact;
	}

	return tmpact;
}

/*char pathto(int x, int y, char target, unsigned char test)
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
				//fprintf(logfile, "Resolved %i.%i as %c (I'm at %i.%i)\n", x, y, node[idx].direction, myx, myy);
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
					//fprintf(logfile, "Resolved %i.%i as %c (I'm at %i.%i)\n", x, y, nodeptr->direction, myx, myy);
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
}*/


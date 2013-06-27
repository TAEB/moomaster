#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>

#define LOCAL_NETHACK		"/home/dragoniz3r/games/nethack"

#define ELBERETH_THRESHOLD	(maxhp / 2)
#define GOODHP			(maxhp / 4 * 3)

#include "inventory.h"
#include "movement.h"
#include "moomaster.h"
#include "combat.h"
#include "templates.h"

unsigned char done = 0;
int goal = GOAL_MINETOWN;
int state = STATE_DESCENDING;
int fd1[2];
int fd2[2];
pid_t pid;
char lastdirection = '\0';
FILE *logfile;
FILE *framelogfile;
char framebuffer[80 * 25];
int trafficbuffer[80 * 25];
int notebuffer[80 * 25];
char notechar[80 * 25];
int cx = 0;
int cy = 0;
int myx = 0;
int myy = 0;
int targetlocationx = 0;
int targetlocationy = 0;
int lastmyx;
int lastmyy;
int monsterx;
int monstery;
int monstercount;
int prayertimer = 300;
int eattimer = 0;
int refreshtimer = 0;
int inventorytimer = 0;
int hp = 100;
int maxhp = 100;
unsigned char lycanthropy = 0;
int turncount = 0;
int elberethtimer = 0;
int elberethx = 0;
int elberethy = 0;
int elberethcount = 0;
int myxlvl = 1;
int lastkilltimer = 100;
unsigned char havexcal = 0;
int mydlvl = 1;
int lasttimer = 0;
unsigned char satiated = 0;
unsigned char hallucinating = 0;
unsigned char blind = 0;
unsigned char weak = 0;
int humancount = 0;
unsigned char needlongpause;
char olditemchar = '.';
int branchlevel = -1;	// the level of the branch between mines and dungeon
unsigned char inminetown = 0;
unsigned char playlocal = 0;

int nodecount = 0;
node_t node[2048];

item_t inventory[128];
int inventorycount = 0;

void investigate(int x, int y, char *buffer);

void Write(int fd, const char *message)
{
	write(fd, message, strlen(message));
}

void dsleep(int seconds)
{
	if (!playlocal)
	{
		sleep(seconds);
	}
}

void dusleep(int microseconds)
{
	if (!playlocal)
	{
		usleep(microseconds);
	}
}

void sigint(int discard)
{
	fclose(logfile);
	fclose(framelogfile);
	sleep(1);
	Write(fd1[1], "S");
	sleep(1);
	Write(fd1[1], "y");
	sleep(1);
	Write(fd1[1], "\r");
	sleep(1);
	printf("Shut down clean\n");
	if (!playlocal)
	{
		kill(pid, SIGTERM);
		wait();
	}
	exit(0);
	done = 1;
}

void sigusr1(int discard)
{
	memset(notebuffer, 0x00, 80 * 25 * 4);
	memset(framebuffer, 0x20, 80 * 25);
}

unsigned char ismonster(char c)
{
	// only bother with NOTOUCH monsters if we have daggers
	return (((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == ':'  || c == '@' || c == '~' || c == '&' || c == '\'' || c == RANGED_MONSTER || c == NYMPH_MONSTER || (c == NOTOUCH_MONSTER && countitems(ITEM_WEAPON, WEAPON_DAGGER))) && !(c == 'K'));
}

unsigned char iseasymonster(char c)
{
	return (c == 'G' || c == 'd');
}

unsigned char isprioritymonster(char c)
{
	return (c == '@' || c == NYMPH_MONSTER || c == 'L');
}

unsigned char immobilemonster(char c)
{
	// these monsters move too slowly for us to wait for them when we're in
	// multi-monster combat mode
	return (c == 'F' || c == 'P' || c == 'p');
}

unsigned char isitem(char c)
{
	return (c == '%' || c == '!' || c == '%' || c == '$' || c == ')' || c == '?' || c == '[' || c == '*' || c == '=' || c == '`' || c == '(' || c == ']');
}

unsigned char isgooditem(char c)
{
	return (c == '$' || c == '?' || c == '!' || c == '[' || c == '=' || c == '%' || c == ')' || c == '/');
}

unsigned char corridorchar(char c)
{
	return (c == '#' || c == '.' || c == '>' || c == '<' || c == '@' || c == '0' || c == '{' || c == '^' || c == '_' || c == 'K' || ismonster(c) || isitem(c));
}

unsigned char canbepeaceful(char c)
{
	return (c == '@' || c == 'h' || c == 'u' || c == 'e' || c == 'i' || c == '&');
}

unsigned char shouldpickup(const char *message)
{
	int tx;
	int ty;

	whereidbe(lastdirection, &tx, &ty);

	if (notebuffer[myy * 80 + myx] & NOTE_BADITEM)
	{
		return 0;
	}
	if (strcasestr(message, "gold pieces"))
	{
		return 1;
	}
	else if (strcasestr(message, "slime mold"))
	{
		return 1;
	}
	else if (strcasestr(message, "ration"))
	{
		return 1;
	}
	else if (strcasestr(message, " tin."))
	{
		return 1;
	}
	else if (strcasestr(message, " tin "))
	{
		return 1;
	}
	else if (strcasestr(message, "potion"))
	{
		return 1;
	}
	else if (strcasestr(message, "scroll"))
	{
		return 1;
	}
	else if (strcasestr(message, "lichen") && !strcasestr(message, "statue"))
	{
		return 1;
	}
	else if (strcasestr(message, "wand"))
	{
		return 1;
	}
	else if (strcasestr(message, "lizard") && !strcasestr(message, "statue"))
	{
		return 1;
	}
	else if (strcasestr(message, "lembas"))
	{
		return 1;
	}
	else if (strcasestr(message, "unicorn horn"))
	{
		return 1;
	}
	else if (strcasestr(message, "wolfsbane"))
	{
		return 1;
	}
	else if (strcasestr(message, "ring"))
	{
		return 1;
	}
	else if (shouldwear(message))
	{
		fprintf(logfile, "we should grab this shit while we still can!\n");
		return 1;
	}
	else if (shouldgrabweapon(message))
	{
		fprintf(logfile, "whee for guns and buttah!!!\n");
		return 1;
	}
	else
	{
		notebuffer[ty * 80 + tx] |= NOTE_BADITEM;
		notechar[ty * 80 + tx] = olditemchar;
	}
	return 0;
}

unsigned char shouldeat(const char *message)
{
	int tx;
	int ty;

	// marking the square as BADITEM here has bad consequences on further
	// looting of the square

	if (monstercount)
	{
		return 0;
	}

	whereidbe(lastdirection, &tx, &ty);

	//fprintf(logfile, "Found %s\n", message);
	if (strcasestr(message, "corpse"))
	{
		fprintf(logfile, "Found a corpse\n");
		fprintf(logfile, "lastdirection = %c (%i.%i, I am at %i.%i)\n", lastdirection, tx, ty, myx, myy);
	 	if (satiated)
		{
			//notebuffer[ty * 80 + tx] |= NOTE_BADITEM;
			//notechar[ty * 80 + tx] = olditemchar;
			fprintf(logfile, "I'm satiated\n");
			return 0;
		}
		else if (lastkilltimer > 40)
		{
			//notebuffer[ty * 80 + tx] |= NOTE_BADITEM;
			//notechar[ty * 80 + tx] = olditemchar;
			fprintf(logfile, "Haven't killed anything recently (timer is %i)\n", lastkilltimer);
			return 0;
		}
		// as soon as I figure out why he seems to not recognize himself
		// as being on top of the food, this can be exact; until then
		// it has to be a little flexible
		else if (abs(myx - monsterx) >= 1 && abs(myy - monstery) >= 1)
		{
			//notebuffer[ty * 80 + tx] |= NOTE_BADITEM;
			//notechar[ty * 80 + tx] = olditemchar;
			fprintf(logfile, "I killed something at %i.%i, but I'm at %i.%i\n", monsterx, monstery, myx, myy);
			return 0;
		}
		//fprintf(logfile, "I see %s\n", message);
		if (
			!strcasestr(message, "human") &&
			!strcasestr(message, "were") &&
			!strcasestr(message, "kobold") &&
			!strcasestr(message, "bee") &&
			!strcasestr(message, "bat") &&
			!strcasestr(message, "trice") &&
			!strcasestr(message, "nymph") &&
			!strcasestr(message, "leprechaun") &&
			!strcasestr(message, "watch") &&
			!strcasestr(message, "soldier") &&
			!strcasestr(message, "lieutenant") &&
			!strcasestr(message, "captain") &&
			!strcasestr(message, "giant spider") &&
			!strcasestr(message, "scorpion") &&
			!strcasestr(message, "centipede") &&
			!strcasestr(message, "yellow mold") &&
			!strcasestr(message, "violet fungus") &&
			!strcasestr(message, "dog") &&
			!strcasestr(message, "cat") &&
			!strcasestr(message, "kitten") &&
			!strcasestr(message, "homunculus")
			)
		{
			fprintf(logfile, "eating corpse\n");
			lastkilltimer = 999;
			return 1;
		}
	}

	return 0;
}

void parseframe(const char *inframe)
{
	int idx;
	int idx2;
	const char *ptr;
	char token[128];
	static char leftover[4096] = "";
	char frame[8192];

	if (strcmp(leftover, ""))
	{
		sprintf(frame, "%s%s", leftover, inframe);
		strcpy(leftover, "");
	}
	else
	{
		strcpy(frame, inframe);
	}

	//fprintf(framelogfile, "%s\n", frame);
	for (idx = strlen(frame) - 1; idx > 0; idx--)
	{
		if (isalpha(frame[idx]))
		{
			break;
		}
		if (frame[idx] == 0x1B)
		{
			// fugger, they only gave us part of the frame :-/
			memcpy(leftover, &frame[idx], strlen(frame) - idx + 1);
		}
	}
	
	for (idx = 0; idx < strlen(frame); idx++)
	{
		if (frame[idx] == 0x1B)
		{
			idx2 = idx;
			ptr = &frame[idx + 2];
			idx++;
			for (idx = idx; idx < strlen(frame); idx++)
			{
				if (isalpha(frame[idx]))
				{
					if (frame[idx] == 'H')
					{
						if (frame[idx - 1] == '[')
						{
							cx = 0;
							cy = 0;
						}
						else
						{
							sscanf(ptr, "%i;%i", &cy, &cx);
							if (cx || cy)
							{
								cy--;
								cx--;
							}
						}
					}
					else if (frame[idx] == 'J')
					{
						if (frame[idx - 1] == '[')
						{
							memset(&framebuffer[cy * 80], 0x20, (80 * 25) - (cy * 80));
						}
						else if (frame[idx - 1] == '1')
						{
							memset(framebuffer, 0x20, (cy * 80) + 80);
						}
						else if (frame[idx - 1] == '2')
						{
							memset(framebuffer, 0x20, 80 * 25);
							cx = 0;
							cy = 0;
						}
					}
					else if (frame[idx] == 'K')
					{
						if (frame[idx - 1] == '[')
						{
							memset(&framebuffer[cy * 80 + cx], 0x20, 80 - cx);
						}
						else if (frame[idx - 1] == '1')
						{
							memset(&framebuffer[cy * 80], 0x20, cx);
						}
						else if (frame[idx - 1] == '2')
						{
							memset(&framebuffer[cy * 80], 0x20, 80);
						}
					}
					else if (frame[idx] == 'm' && frame[idx - 1] == '[')
					{
					}
					else if (frame[idx] == 'A')
					{
						if (cy > 0)
						{
							cy--;
						}
					}
					else if (frame[idx] == 'B')
					{
						if (cy < 25)
						{
							cy++;
						}
					}
					else if (frame[idx] == 'C')
					{
						if (cx < 80)
						{
							cx++;
						}
					}
					else if (frame[idx] == 'D')
					{
						if (cx > 0)
						{
							cx--;
						}
					}
					memcpy(token, &frame[idx2], idx - idx2 + 1);
					token[idx - idx2 + 1] = '\0';
					fprintf(framelogfile, "%s\n", token);
					break;
				}
			}
		}
		else if (frame[idx] == 0x08)
		{
			cx--;
		}
		else if (frame[idx] != 0x0A && frame[idx] != 0x0D && frame[idx] != 0x08 && frame[idx] != 0x0F)
		{
			if (cy >= 0 && cy < 25 && cx >= 0 && cx < 80)
			{
				framebuffer[cy * 80 + cx] = frame[idx];
				cx++;
			}
		}
	}
}

void handlemessage(const char *message, const char *when)
{
	char buffer[4096];
	int result;
	char *ptr;
	char item[2];
	int idx;

	fprintf(framelogfile, "%s\n", message);

	parseframe(message);
	if (strcasestr(message, "more"))
	{
		Write(fd1[1], " ");
	}
	if (strcasestr(message, "--More--"))
	{
		Write(fd1[1], " ");
	}
	if (strcasestr(message, "carrying too much"))
	{
		sprintf(buffer, "%c", '1' + (rand() % 9));
		Write(fd1[1], buffer);
	}
	if (strcasestr(message, "Really attack"))
	{
		Write(fd1[1], "n ");
	}
	if ((strcasestr(message, "hungry") || weak) && eattimer <= 0 )
	{
		Write(fd1[1], "e");
		dusleep(500000);
		result = read(fd2[0], buffer, 4096);
		if (result > 0)
		{
			buffer[result] = '\0';
			if (strstr(buffer, "eat? ["))
			{
				ptr = strstr(buffer, "eat? [");
				item[0] = ptr[6];
				item[1] = '\0';
				Write(fd1[1], item);
			}
		}
		eattimer = 100;
	}
	if ((strcasestr(message, "needs food") || weak || strcasestr(message, "FoodPois")) && prayertimer <= 200)
	{
		fprintf(logfile, "praying for food/healing\n");
		Write(fd1[1], " ");
		Write(fd1[1], " ");
		Write(fd1[1], " ");
		Write(fd1[1], "#");
		Write(fd1[1], "p");
		Write(fd1[1], "r");
		Write(fd1[1], "a");
		Write(fd1[1], "y");
		Write(fd1[1], "\r");
		Write(fd1[1], "y");
		Write(fd1[1], " ");
		Write(fd1[1], " ");
		Write(fd1[1], " ");
		prayertimer += 500;
	}
	if (strcasestr(message, "see here") || strcasestr(message, "things that are here") || strcasestr(message, "several items here"))
	{
		//fprintf(logfile, "%s: I see %s\n", when, message);
		if (shouldeat(message))
		{
			fprintf(logfile, "Found something fun to eat\n");
			dsleep(1);
			Write(fd1[1], " ey  ");
			needlongpause = 1;
		}
		if (shouldpickup(message))
		{
			fprintf(logfile, "Found something fun to pick up. I am at %i.%i\n", myx, myy);
			Write(fd1[1], " ");
			Write(fd1[1], ",");
			Write(fd1[1], "a");
			Write(fd1[1], " ");
			needlongpause = 1;
		}
		//sprintf(buffer, "\x12");
		//Write(fd1[1], buffer);
	}
	if (IDpotion(message))
	{
		return;
	}
	if (strcasestr(message, "possessions identified"))
	{
		dsleep(1);
		Write(fd1[1], "y ");
		dsleep(1);
		Write(fd1[1], "y ");
		dsleep(1);
		Write(fd1[1], "y ");
		dsleep(1);
		Write(fd1[1], "y ");
		dsleep(1);
		Write(fd1[1], "q");
		dsleep(1);
		Write(fd1[1], "\r");
		Write(fd1[1], "\r");
		Write(fd1[1], "\r");
		Write(fd1[1], "\r");
		Write(fd1[1], "\r");
		Write(fd1[1], "\r");
		Write(fd1[1], "\r");
		dsleep(5);
		kill(pid, SIGTERM);
		wait();
		exit(0);
	}
	if (strcasestr(message, "no shape"))
	{
		fprintf(logfile, "leg sore\n");
		Write(fd1[1], " ");
		Write(fd1[1], " ");
		Write(fd1[1], "n");
		Write(fd1[1], "5");
		Write(fd1[1], "0");
		Write(fd1[1], "s");
	}
	if (strcasestr(message, "locked") || strcasestr(message, "whamm"))
	{
		if (inminetown)
		{
			fprintf(logfile, "Encountered locked door, but %i @ are in sight. Waiting\n", humancount);
			Write(fd1[1], "s");
		}
		else
		{
			fprintf(logfile, "Encountered locked door, %i @ in sight. Kicking it down\n", humancount);
			Write(fd1[1], "k");
			sprintf(buffer, "%c", lastdirection);
			Write(fd1[1], buffer);
		}
	}
	if (strstr(message, "Xp:"))
	{
		ptr = strstr(message, "Xp:") + 3;
		if (ptr)
		{
			sscanf(ptr, "%i", &myxlvl);
		}
	}
	if (strstr(message, "Dlvl:"))
	{
		ptr = strstr(message, "Dlvl:") + 5;
		sscanf(ptr, "%i", &idx);
		if (idx != mydlvl)
		{
			fprintf(logfile, "Clearing memory\n");
			memset(trafficbuffer, 0x00, 80 * 25 * 4);
			memset(notebuffer, 0x00, 80 * 25 * 4);
			turncount = 0;
			inminetown = 0;
			if (state == STATE_ASCENDING && mydlvl == branchlevel)
			{
				notebuffer[myy * 80 + myx] |= NOTE_BADDOWNSTAIR;
			}
			if (state == STATE_ASCENDING && mydlvl == branchlevel && turncount > 100)
			{
				fprintf(logfile, "--- Time to continue the main dungeon ---\n");
				state = STATE_DESCENDING;
			}
		}
		mydlvl = idx;
	}
	if (strcasestr(message, "AC:") && !playlocal)
	{
		ptr = strstr(message, "AC");
		while (*ptr != 'H')
		{
			ptr--;
		}
		ptr++;
		sscanf(ptr, "%i(%i)", &hp, &maxhp);
	}
	else if (strstr(message, "HP:") && playlocal)
	{
		ptr = strstr(message, "HP:") + 3;
		sscanf(ptr, "%i(%i)", &hp, &maxhp);
	}
	if (strcasestr(message, "confident"))
	{
		Write(fd1[1], " ");
		Write(fd1[1], "#");
		Write(fd1[1], "e");
		Write(fd1[1], "\r");
		dsleep(1);
		Write(fd1[1], "a");
		Write(fd1[1], " ");
		sprintf(buffer, "\x12");
		Write(fd1[1], buffer);
	}
	if (strcasestr(message, "You turn into"))
	{
		if (prayertimer < 100)
		{
			Write(fd1[1], "  #pray\ry  ");
			prayertimer += 500;
		}
		lycanthropy = 1;
	}
	if (strcasestr(message, "human form"))
	{
		lycanthropy = 0;
	}
	if (strcasestr(message, "purified"))
	{
		lycanthropy = 0;
	}
	if (strcasestr(message, "hello stranger"))
	{
		Write(fd1[1], "Your mom\r     d$ ");
		trafficbuffer[myy * 80 + myx] = 999;
		notebuffer[myy * 80 + myx] |= NOTE_BADITEM;
	}
	if (strcasestr(message, "unpaid"))
	{
		fprintf(logfile, "Dropping unpaid stuff\n");
		Write(fd1[1], "Du\r,\r   ");
		notebuffer[myy * 80 + myx] |= NOTE_BADITEM;
		//Write(fd1[1], "pn  ");
	}
	if (strcasestr(message, "have enough money"))
	{
		Write(fd1[1], "Du\r,\r   ");
	}
	if (strcasestr(message, "reaches up from"))
	{
		havexcal = 1;
	}
	if (strcasestr(message, "satiated"))
	{
		satiated = 1;
	}
	if (strcasestr(message, "weak"))
	{
		weak = 1;
	}
	if (strcasestr(message, "hallu"))
	{
		hallucinating = 1;
	}
	if (strcasestr(message, "blind"))
	{
		blind = 1;
	}
	if (strcasestr(message, "wish?"))
	{
		Write(fd1[1], "blessed greased fixed +2 gray dragon scale mail\r  ");
	}
	if (strcasestr(message, "bought it"))
	{
		Write(fd1[1], "pyy  ");
	}
	if (strcasestr(message, "pay whom?"))
	{
		Write(fd1[1], ".s");
	}
}

unsigned char engulfed()
{
	if (framebuffer[myy * 80 + myx - 1] == '|' && framebuffer[myy * 80 + myx + 1] == '|' && framebuffer[(myy - 1) * 80 + myx] == '-' && framebuffer[(myy + 1) * 80 + myx] == '-')
	{
		return 1;
	}
	return 0;
}

unsigned char inmines()
{
	int idx;

	for (idx = 80; idx < 80 * 22; idx++)
	{
		if (framebuffer[idx] == '-' && framebuffer[idx + 1] == '-' && framebuffer[idx + 2] == '.' && framebuffer[idx - 80] == '|' && framebuffer[idx + 80] == '.')
		{
			return 1;
		}
		if (framebuffer[idx] == '-' && framebuffer[idx + 1] == '-' && framebuffer[idx + 2] == '.' && framebuffer[idx - 80] == '.' && framebuffer[idx + 80] == '|')
		{
			return 1;
		}
		if (framebuffer[idx] == '.' && framebuffer[idx + 1] == '-' && framebuffer[idx + 2] == '-' && framebuffer[idx - 79] == '|' && framebuffer[idx + 81] == '.')
		{
			return 1;
		}
		if (framebuffer[idx] == '.' && framebuffer[idx + 1] == '-' && framebuffer[idx + 2] == '-' && framebuffer[idx - 79] == '.' && framebuffer[idx + 81] == '|')
		{
			return 1;
		}
	}

	return 0;
}

unsigned char inminesend()
{
	int matches = 0;
	int comparisons = 0;
	int idx;
	int idx2;
	int idx3;

	// first check to see if we can see at least 1/4 of the map.
	// if not, it's not worth it.
	for (idx = 80; idx < 80 * 22; idx++)
	{
		comparisons++;
		if (framebuffer[idx] != ' ')
		{
			matches++;
		}
	}
	if ((float)matches / (float)comparisons < 0.25)
	{
		return 0;
	}

	matches = 0;
	comparisons = 0;
	for (idx2 = 0; idx2 < 3; idx2++)
	{
		idx3 = 80;
		for (idx = 0; idx < strlen(minesend_template[idx2]); idx++)
		{
			if (framebuffer[idx3] == ' ')
			{
				continue;
			}
			if (minesend_template[idx2][idx] == '&')
			{
				// align with the beginning of the next row
				idx3 += 80 - (idx % 80);
				continue;
			}
			if (minesend_template[idx2][idx] == '|' || minesend_template[idx2][idx] == '-')
			{
				comparisons++;
			}
			else
			{
				continue;
			}
			if (framebuffer[idx3] == '#' || framebuffer[idx3] == minesend_template[idx2][idx])
			{
				matches++;
			}
		}
		if (((float)matches / (float)comparisons) >= 0.75)
		{
			return 1;
		}
	}
	return 0;
}

const char *chooseaction()
{
	static char action[16];
	int idx;
	int x = 0;
	int y = 0;
	int tx;
	int ty;
	char target;
	int pass = 0;
	static int lastx;
	static int lasty;
	unsigned char longact = 0;
	unsigned char longacttime = 3;
	int nearestx;
	int nearesty;
	float nearestdist = 0.0;
	int xoff;
	int yoff;
	float dist;
	char buffer[256];
	unsigned char easymonsters = 1;
	int elbereththreshold = ELBERETH_THRESHOLD;
	int goodhp = GOODHP;

	if (engulfed())
	{
		return "4";
	}
	if (!(turncount % 50) && inmines() && (branchlevel == -1 || branchlevel > mydlvl))
	{
		branchlevel = mydlvl - 1;
		fprintf(logfile, "Identified level %i as being mines (branch level is %i)\n", mydlvl, branchlevel);
	}
	if (!(turncount % 50) && inminesend())
	{
		fprintf(logfile, "Identified level %i as being mines end\n", mydlvl);
		state = STATE_ASCENDING;
		goal = GOAL_ORACLE;
	}

	monstercount = 0;
	if (myxlvl >= 5)
	{
		for (idx = 80; idx < 80 * 22; idx++)
		{
			if (ismonster(framebuffer[idx]) && ((idx % 80) != myx || (idx / 80) != myy))
			{
				monstercount++;
				if (!iseasymonster(framebuffer[idx]))
				{
					easymonsters = 0;
				}
			}
		}
		if (easymonsters)
		{
			// experimental. Maybe a bad idea.
			goodhp -= (maxhp / 4);
		}
	}

	for (pass = 0; pass < 7; pass++)
	{
		nearestdist = 0.0;
		if (pass == 0 && hp <= elbereththreshold)
		{
			strcpy(action, "");
			if (countitems(ITEM_POTION, POTION_FULLHEALING))
			{
				sprintf(action, "q%c  ", getitem(ITEM_POTION, POTION_FULLHEALING)->letter);
			}
			else if (countitems(ITEM_POTION, POTION_EXTRAHEALING))
			{
				sprintf(action, "q%c  ", getitem(ITEM_POTION, POTION_EXTRAHEALING)->letter);
			}
			else if (countitems(ITEM_POTION, POTION_HEALING))
			{
				sprintf(action, "q%c  ", getitem(ITEM_POTION, POTION_HEALING)->letter);
			}
			else if (countitems(ITEM_SCROLL, SCROLL_TELEPORTATION))
			{
				sprintf(action, "r%c  ", getitem(ITEM_SCROLL, SCROLL_TELEPORTATION)->letter);
			}
			if (strcmp(action, ""))
			{
				return action;
			}
		}
		if (pass == 0 && (hp < maxhp / 7 || hp < 6) && prayertimer <= 0)
		{
			fprintf(logfile, "praying for hp\n");
			prayertimer += 500;
			strcpy(action, "#pray\ry  ");
			return action;
		}
		if (pass == 0 && hp <= elbereththreshold && elberethtimer <= 0 && humancount == 1 && !blind && !hallucinating)
		{
			fprintf(logfile, "oshi about to die, elberething\n");
			strcpy(action, "E-  Elbereth\r");
			elberethtimer = 10;
			elberethx = myx;
			elberethy = myy;
			elberethcount++;
			return action;
		}
		else if (pass == 0 && hp <= goodhp && myx == elberethx && myy == elberethy && elberethtimer >= 8 && humancount == 1 && !blind && !hallucinating)
		{
			fprintf(logfile, "reiterating elbereth\n");
			if (elberethcount >= 50)
			{
				strcpy(action, "E-n Elbereth\r");
				elberethcount = 0;
			}
			else
			{
				strcpy(action, "E-y Elbereth\r");
			}
			elberethcount++;
			return action;
		}
		else if (pass == 0 && hp <= goodhp && myx == elberethx && myy == elberethy && humancount == 1 && !blind && !hallucinating)
		{
			fprintf(logfile, "resting on elbereth\n");
			strcpy(action, "s");
			return action;
		}
		else if (pass == 0 && hp <= goodhp)
		{
		}
		// this is deliberately GOODHP and not goodhp because we want
		// to rest up when not facing ANY enemies. The point of goodhp
		// is to allow us to fight longer against easy enemies.
		else if (pass > 0 && hp <= GOODHP)
		{
			fprintf(logfile, "oshi about to die. resting.\n");
			strcpy(action, "s");
			return action;
		}
		for (idx = 80; idx < 80 * 22; idx++)
		{
			target = framebuffer[idx];
			tx = idx % 80;
			ty = idx / 80;
			if (ismonster(framebuffer[idx]) && ((idx % 80) != myx || (idx / 80) != myy) && pass == 0 && pathto(tx, ty, framebuffer[idx], 1) != '!' && !(strcasestr(framebuffer, "see here") && tx >= 58 && ty <= 3))
			{
				x = idx % 80;
				y = idx / 80;
				xoff = x - myx;
				yoff = y - myy;
				dist = sqrtf(xoff * xoff + yoff * yoff);
				/*if (isprioritymonster(framebuffer[idx]))
				{
					dist -= 2.0;
				}*/
				if (dist < nearestdist || nearestdist == 0.0)
				{
					nearestx = x;
					nearesty = y;
					nearestdist = dist;
				}
			}
			else if (framebuffer[idx] == '%' && pass == 1 && pathto(tx, ty, '%', 1) != '!' && trafficbuffer[idx] < traffictimeout() && !(notebuffer[idx] & NOTE_BADITEM))
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found food @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				pass = 99;
				if (abs(tx - myx) <= 1 && abs(ty - myy) <= 1)
				{
					trafficbuffer[idx]++;
				}
				break;
			}
			else if (notebuffer[idx] & NOTE_DOOR && pass == 3 && pathto(tx, ty, '+', 1) != '!' && !lycanthropy && trafficbuffer[idx] < traffictimeout())
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found door @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				pass = 99;
				break;
			}
			else if (framebuffer[idx] == '0' && pass == 2 && pathto(tx, ty, '0', 1) != '!' && trafficbuffer[idx] < traffictimeout() / 2)
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found boulder @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				pass = 99;
				if (abs(x - myx) <= 1 && abs(y - myy) <= 1)
				{
					trafficbuffer[idx]++;
				}
				break;
			}
			else if (state == STATE_DESCENDING && notebuffer[idx] & NOTE_DOWNSTAIR && !(notebuffer[idx] & NOTE_BADDOWNSTAIR) && pass == 6 && turncount >= 200 && pathto(tx, ty, '>', 1) != '!')
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found staircase @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				pass = 99;
				longact = 1;
				longacttime = 4;
				break;
			}
			else if (state == STATE_ASCENDING && notebuffer[idx] & NOTE_UPSTAIR && pass == 6 && turncount >= 200 && pathto(tx, ty, '<', 1) != '!')
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found staircase @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				pass = 99;
				longact = 1;
				longacttime = 4;
				break;
			}
			else if (isgooditem(framebuffer[idx]) && pass == 4 && pathto(tx, ty, framebuffer[idx], 1) != '!' && trafficbuffer[idx] < traffictimeout() / 2 && !(notebuffer[idx] & NOTE_BADITEM))
			{
				x = idx % 80;
				y = idx / 80;
				xoff = x - myx;
				yoff = y - myy;
				dist = sqrtf(xoff * xoff + yoff * yoff);
				if (dist < nearestdist || nearestdist == 0.0)
				{
					nearestx = x;
					nearesty = y;
					nearestdist = dist;
				}
			}
			else if (myxlvl >= 5 && framebuffer[idx] == '{' && pass == 5 && pathto(tx, ty, '{', 1) != '!' && !havexcal && getitem(ITEM_WEAPON, WEAPON_LONGSWORD) && !inminetown)
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found fountain @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				pass = 99;
				break;
			}
		}
		if (pass == 0 && nearestdist > 0.0)
		{
			//x = nearestx;
			//y = nearesty;
			fprintf(logfile, "Found monster (%c) @ %i.%i. I am at %i.%i\n", framebuffer[y * 80 + x], x, y, myx, myy);
			target = framebuffer[nearesty * 80 + nearestx];
			return handlecombat(nearestx, nearesty, target);
			//break;
		}
		else if (pass == 4 && nearestdist > 0.0)
		{
			x = nearestx;
			y = nearesty;
			fprintf(logfile, "Found good item @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
			longact = 1;
			longacttime = 4;
			break;
		}
	}
	if (lasttimer > 0 && !x && !y)
	{
		x = lastx;
		y = lasty;
		lasttimer--;
		target = framebuffer[y * 80 + x];
		//fprintf(logfile, "Restoring longact to %i.%i (I am at %i.%i, %i turns left)\n", x, y, myx, myy, lasttimer);
		longact = 0;
		goto path;
	}
	if (!x && !y)
	{
		findunexploredarea(&x, &y, &target);
		if (!x && !y)
		{
			findinterestingcorridor(&x, &y);
			if (x || y)
			{
				longact = 1;
			}
		}
		else
		{
			longact = 1;
		}
	}
	/*if (!x && !y)
	{
		for (idx = 80; idx < 80 * 22; idx++)
		{
			if (notebuffer[idx] & NOTE_DOWNSTAIR && pathto(tx, ty, '>', 1) != '!')
			{
				x = idx % 80;
				y = idx / 80;
				fprintf(logfile, "Found staircase @ %i.%i. I am at %i.%i\n", x, y, myx, myy);
				break;
			}
		}
	}*/
	if (!x && !y)
	{
		if (findinterestingwall(&x, &y, &target))
		{
			if (abs(x - myx) <= 1 && abs(y - myy) <= 1)
			{
				fprintf(logfile, "Searching wall\n");
				sprintf(action, "n2s");
				return action;
			}
			else
			{
				longact = 1;
				longacttime = 8;
			}
		}
	}
	path:
	targetlocationx = x;
	targetlocationy = y;
	lastdirection = pathto(x, y, target, 0);
	if (lastdirection == '5')
	{
		lastdirection = '.';
	}
	elberethcount = 0;	// we've moved
	if (longact && (abs(x - myx) > 1 || abs(y - myy) > 1))
	{
		lastx = x;
		lasty = y;
		lasttimer = (rand() % longacttime) + 1;
	}
	whereidbe(lastdirection, &tx, &ty);
	olditemchar = framebuffer[ty * 80 + tx];
	//fprintf(logfile, "I am at %i.%i, I would be at %i.%i, target is at %i.%i\n", myx, myy, tx, ty, x, y);
	if (tx == x && ty == y)
	{
		longact = 0;
		lasttimer = 0;
		if (target == '+')
		{
			sprintf(action, "o%c", lastdirection);
			//fprintf(logfile, "Found the door at %c (I am at %i.%i)\n", lastdirection, myx, myy);
			trafficbuffer[y * 80 + x]++;
		}
		else if (target == '>')
		{
			sprintf(action, "%c>:", lastdirection);
			//fprintf(logfile, "Found the stairs at %c\n", lastdirection);
		}
		else if (target == '<')
		{
			sprintf(action, "%c<:", lastdirection);
			//fprintf(logfile, "Found the stairs at %c\n", lastdirection);
		}
		else if (target == '{' && getitem(ITEM_WEAPON, WEAPON_LONGSWORD) && !inminetown)
		{
			sprintf(action, "%c#dip\r%cy  ", lastdirection, getitem(ITEM_WEAPON, WEAPON_LONGSWORD)->letter);
			//fprintf(logfile, "Found the fountain at %c\n", lastdirection);
		}
		else if (ismonster(target))
		{
			sprintf(action, "%c", lastdirection);
			if (target == 'Z' || target == 'M' || target == 'V' || target == '@' || target == 'c' || target == 'l' || target == 'n')
			{
				lastkilltimer = 999;
			}
			else
			{
				lastkilltimer = 0;
				monsterx = myx;
				monstery = myy;
			}
			needlongpause = 1;
			//fprintf(logfile, "Found the monster at %c\n", lastdirection);
		}
		else if (target == '%')
		{
			sprintf(action, "%c", lastdirection);
			fprintf(logfile, "Found the food at %c\n", lastdirection);
			needlongpause = 1;
		}
		else if (isgooditem(target))
		{
			sprintf(action, "%c  ", lastdirection);
			fprintf(logfile, "Found the good item at %c\n", lastdirection);
			needlongpause = 1;
		}
		else
		{
			sprintf(action, "%cs", lastdirection);
			//fprintf(logfile, "Moving in direction %c\n", lastdirection);
		}
	}
	else
	{
		sprintf(action, "%c", lastdirection);
		//fprintf(logfile, "Moving in direction %c\n", lastdirection);
	}
	return action;
}

void investigate(int x, int y, char *message)
{
	int tx = myx;
	int ty = myy;
	char buffer[4096];
	int result;
	int idx;
	char *ptr;
	char *ptr2;
	char *ptr3;

	strcpy(message, "");

	if (x < 3 || x > 77 || y < 3 || y > 20 || (x > 60 && y < 5))	// nothing interesting is out this far
	{
		return;
	}
	if (x == myx && y == myy)
	{
		return;
	}

	dusleep(50000);

	fprintf(logfile, "investigating %i.%i\n", x, y);

	Write(fd1[1], ";");

	do
	{
		dusleep(50000);
		if (x > tx)
		{
			Write(fd1[1], "6");
			tx++;
		}
		else if (x < tx)
		{
			Write(fd1[1], "4");
			tx--;
		}
		else if (y > ty)
		{
			Write(fd1[1], "2");
			ty++;
		}
		else if (y < ty)
		{
			Write(fd1[1], "8");
			ty--;
		}
	} while (tx != x || ty != y);

	Write(fd1[1], ".");

	dusleep(50000);
	for (idx = 0; idx < 30; idx++)
	{
		dusleep(15000);
		result = read(fd2[0], buffer, 4096);
		if (result > 0)
		{
			buffer[result] = '\0';
			printf("%s\n", buffer);
			ptr = buffer;
			while (1)
			{
				ptr2 = strstr(ptr, "\033");
				if (!ptr2)
				{
					break;
				}
				if (ptr2[0] == '\033' && ptr2[1] == '[' && ptr2[2] == 'H' && ptr2[4] == ' ' && ptr2[5] == ' ' && ptr2[6] == ' ')
				{
					ptr3 = strstr(&ptr2[3], "\033");
					*ptr3 = '\0';
					memcpy(message, &ptr2[3], 80);
					return;
				}
				ptr = ptr2 + 1;
			}
		}
	}
	Write(fd1[1], "  ");
}

void dumpnotes()
{
	FILE *filePtr;
	int idx;

	filePtr = fopen("notes.txt", "wb");
	if (!filePtr)
	{
		return;
	}

	for (idx = 0; idx < 80 * 25; idx++)
	{
		if (idx % 80 == targetlocationx && idx / 80 == targetlocationy)
		{
			fputc('*', filePtr);
			continue;
		}
		else if (idx % 80 == myx && idx / 80 == myy)
		{
			fputc('@', filePtr);
			continue;
		}
		if (notebuffer[idx] & NOTE_DOOR)
		{
			fputc('+', filePtr);
		}
		else if (notebuffer[idx] & NOTE_UPSTAIR)
		{
			fputc('<', filePtr);
		}
		else if (notebuffer[idx] & NOTE_DOWNSTAIR)
		{
			fputc('>', filePtr);
		}
		else if (notebuffer[idx] & NOTE_ALTAR)
		{
			fputc('_', filePtr);
		}
		else if (notebuffer[idx] & NOTE_OPENDOOR)
		{
			fputc('K', filePtr);
		}
		else if (notebuffer[idx] & NOTE_TRAP)
		{
			fputc('^', filePtr);
		}
		else if (notebuffer[idx] & NOTE_BADITEM)
		{
			fputc('B', filePtr);
		}
		else if (notebuffer[idx] & NOTE_BADFLOOR)
		{
			fputc('b', filePtr);
		}
		else if (notebuffer[idx] & NOTE_SEENFLOOR)
		{
			fputc('.', filePtr);
		}
		else if (notebuffer[idx] & NOTE_BADDOWNSTAIR)
		{
			fputc('X', filePtr);
		}
		else if (notebuffer[idx] & NOTE_FOUNTAIN)
		{
			fputc('{', filePtr);
		}
		else
		{
			fputc(' ', filePtr);
		}
		if ((idx % 80) == 79)
		{
			fputc('\n', filePtr);
		}
	}

	fclose(filePtr);
}

void preprocess()
{
	int idx;
	char buffer[256];
	int timer = 0;
	int dx;
	int dy;

	for (idx = 80; idx < 80 * 22; idx++)
	{
		if (framebuffer[idx] == '.' || framebuffer[idx] == '#')
		{
			notebuffer[idx] |= NOTE_SEENFLOOR;
			if (framebuffer[idx] == '.')
			{
				notebuffer[idx] &= ~NOTE_BADFLOOR;
			}
		}
	}
	for (idx = 80; idx < 80 * 22; idx++)
	{
		if (notebuffer[idx] & NOTE_SEENFLOOR && framebuffer[idx] == ' ')
		{
			framebuffer[idx] = '.';
		}
	}
	startover:
	timer++;
	if (timer > 5)
	{
		fprintf(logfile, "Loop detected in preprocess\n");
		return;
	}
	for (idx = 80; idx < 80 * 21; idx++)
	{
		if (idx % 80 == myx && idx / 80 == myy)
		{
			continue;
		}
		if (framebuffer[idx] == 'j')
		{
			framebuffer[idx] = NOTOUCH_MONSTER;
			continue;
		}
		else if (framebuffer[idx] == 'n')
		{
			framebuffer[idx] = NYMPH_MONSTER;
			continue;
		}
		dx = (idx % 80) - myx;
		dy = (idx / 80) - myy;
		if (canbepeaceful(framebuffer[idx]))
		{
			investigate(idx % 80, idx / 80, buffer);
			fprintf(logfile, "Got message %s\n", buffer);
			if (strcasestr(buffer, "peaceful"))
			{
				framebuffer[idx] = PEACEFUL_MONSTER;
				if (strcasestr(buffer, "human ") && humancount <= 2)
				{
					Write(fd1[1], "pyy  \r");
				}
				if (strcasestr(buffer, "watch"))
				{
					inminetown = 1;
				}
			}
			else if (strcasestr(buffer, "floating eye"))
			{
				framebuffer[idx] = NOTOUCH_MONSTER;
				notebuffer[idx] |= NOTE_BADFLOOR;
			}
			else if (strcasestr(buffer, "sorearmoo") || strcasestr(buffer, "dragoniz3r"))
			{
				myx = idx % 80;
				myy = idx / 80;
				fprintf(logfile, "Found myself. Starting preprocess step over\n");
				goto startover;
			}
			else if (strcasestr(buffer, "dark part"))
			{
				framebuffer[idx] = ' ';
				fprintf(logfile, "Refreshing world\n");
				sprintf(buffer, "\x12");
				Write(fd1[1], buffer);
				memset(framebuffer, 0x20, 80 * 25);
				return;
			}
			else
			{
				framebuffer[idx] = buffer[0];
			}
		}
	}
	FILE *filePtr = fopen("framebuffer.txt", "wb");
	for (idx = 0; idx < 25; idx++)
	{
		fwrite(&framebuffer[idx * 80], 1, 80, filePtr);
		fputc('\n', filePtr);
	}
	fclose(filePtr);
}

void game()
{
	int idx;
	int result;
	char buffer3[4096];
	char buffer2[2];
	char buffer[4096];
	int moveidx = 0;
	char *ptr;
	int distx;
	int disty;
	int dist;
	int dist2;
	int count;

	logfile = fopen("log.txt", "wb");
	framelogfile = fopen("framelog.txt", "wb");
	setvbuf(logfile, NULL, _IOLBF, 4096);
	fprintf(logfile, "Coming up\n");
	//close(fd1[0]);
	//close(fd2[1]);

	result = fcntl(fd2[0], F_GETFL);
	result |= O_NONBLOCK;
	fcntl(fd2[0], F_SETFL, result);

	if (!playlocal)
	{
		dsleep(3);
		Write(fd1[1], "l");
		dsleep(1);
		Write(fd1[1], "sorearmoo\r");
		dsleep(1);
		Write(fd1[1], "sorearmoo\r");
		dsleep(1);
		for (idx = 0; idx < 20; idx++)
		{
			dusleep(5000);
			result = read(fd2[0], buffer, 4096);
		}
		Write(fd1[1], "p");
	}
	// deliberately not dsleep
	sleep(3);
	Write(fd1[1], " ");
	Write(fd1[1], " ");
	sleep(1);
	Write(fd1[1], " ");
	Write(fd1[1], " ");
	for (idx = 0; idx < 20; idx++)
	{
		dusleep(5000);
		result = read(fd2[0], buffer, 4096);
	}
	sprintf(buffer, "\x12");
	Write(fd1[1], buffer);

	memset(framebuffer, 0x20, 80 * 25);
	memset(trafficbuffer, 0x00, 80 * 25 * 4);
	memset(notebuffer, 0x00, 80 * 25 * 4);
	cx = 0;
	cy = 0;

	fprintf(logfile, "Beginning play\n");
	while (1)
	{
		needlongpause = 0;
		dump_traffic();
		for (idx = 0; idx < 10; idx++)
		{
			dusleep(5000);
			result = read(fd2[0], buffer, 4096);
			if (result > 0)
			{
				buffer[result] = '\0';
				printf("%s\n", buffer);
				handlemessage(buffer, "pre");
			}
		}
		Write(fd1[1], " ");
		move:
		lastkilltimer++;
		if (prayertimer > 0)
		{
			prayertimer--;
		}
		if (eattimer > 0)
		{
			eattimer--;
		}
		if (refreshtimer <= time(NULL))
		{
			satiated = 0;
			hallucinating = 0;
			blind = 0;
			fprintf(logfile, "Sent refresh request (Traffic timeout is %i, turncount is %i, prayertimer = %i)\n", traffictimeout(), turncount, prayertimer);
			sprintf(buffer, "\x12");
			Write(fd1[1], buffer);
			// this breaks combat, may need to be worked in
			// a delayed way
			//memset(framebuffer, 0x20, 80 * 25);
			for (idx = 0; idx < 80 * 25; idx++)
			{
				// this breaks dark rooms and such
				/*if (notebuffer[idx] & NOTE_SEENFLOOR)
				{
					notebuffer[idx] &= ~NOTE_SEENFLOOR;
				}*/
				if (notebuffer[idx] & NOTE_BADFLOOR)
				{
					notebuffer[idx] &= ~NOTE_BADFLOOR;
				}
			}
			refreshtimer = time(NULL) + REFRESH_TIMER;
		}
		if (inventorytimer <= time(NULL))
		{
			refreshinventory();
			sprintf(buffer, "\x12");
			Write(fd1[1], buffer);
			inventorytimer = time(NULL) + INVENTORY_TIMER;
		}
		if (elberethtimer > 0)
		{
			elberethtimer--;
		}
		count = 0;
		for (idx = 80; idx < 80 * 22; idx++)
		{
			if (framebuffer[idx] == '@')
			{
				count++;
			}
		}
		humancount = count;
		for (idx = 0; idx < 80 * 22; idx++)
		{
			if (framebuffer[idx] == 'A' || framebuffer[idx] == 'H')
			{
				humancount++;
			}
		}
		for (idx = 80; idx < 80 * 22; idx++)
		{
			if (framebuffer[idx] == '@')
			{
				break;
			}
		}
		myx = idx % 80;
		myy = idx / 80;
		if (myx != cx || myy != cy)
		{
			fprintf(logfile, "Cursor is at %i.%i, last known pos is %i.%i, @ is at %i.%i, %i @ in view, ", cx, cy, lastmyx, lastmyy, myx, myy, count);
			if (cy > 0 && cy < 22 && count != 1)
			{
				myx = cx;
				myy = cy;
			}
			fprintf(logfile, "I am at %i.%i\n", myx, myy);
		}
		for (idx = 0; idx < 80 * 25; idx++)
		{
			if (notebuffer[idx] & NOTE_DOOR)
			{
				notebuffer[idx] &= ~NOTE_DOOR;
			}
			if (notebuffer[idx] & NOTE_OPENDOOR)
			{
				notebuffer[idx] &= ~NOTE_OPENDOOR;
			}
			if (notebuffer[idx] & NOTE_BADITEM)
			{
				if (framebuffer[idx] != notechar[idx])
				{
					//notebuffer[idx] &= ~NOTE_BADITEM;
				}
			}
		}
		for (idx = 80; idx < 80 * 22; idx++)
		{
			if (framebuffer[idx] == '+' && isdoorway(idx % 80, idx / 80, 0xFE))
			{
				notebuffer[idx] |= NOTE_DOOR;
			}
			else if (framebuffer[idx] == '<')
			{
				notebuffer[idx] |= NOTE_UPSTAIR;
			}
			else if (framebuffer[idx] == '>')
			{
				notebuffer[idx] |= NOTE_DOWNSTAIR;
			}
			else if (framebuffer[idx] == '_')
			{
				notebuffer[idx] |= NOTE_ALTAR;
			}
			else if (framebuffer[idx] == '^')
			{
				notebuffer[idx] |= NOTE_TRAP;
			}
			else if (framebuffer[idx] == 'K')
			{
				notebuffer[idx] |= NOTE_OPENDOOR;
			}
			else if ((isitem(framebuffer[idx]) && isdoorway(idx % 80, idx / 80, 0xFF)) || (framebuffer[idx] == '@' && !(notebuffer[idx] & NOTE_SEENFLOOR) && isdoorway(idx % 80, idx / 80, 0xFE)))
			{
				notebuffer[idx] |= NOTE_OPENDOOR;
			}
			else if (framebuffer[idx] == '{')
			{
				notebuffer[idx] |= NOTE_FOUNTAIN;
			}
		}
		trafficbuffer[myy * 80 + myx]++;
		preprocess();
		dumpnotes();
		sprintf(buffer, "%s", chooseaction());
		dusleep(10000);
		sendtheaction:
		for (idx = 0; idx < strlen(buffer); idx++)
		{
			sprintf(buffer2, "%c", buffer[idx]);
			Write(fd1[1], buffer2);
			if (buffer[idx] == 'k')
			{
				dusleep(10000);
				getmessage:
				result = read(fd2[0], buffer3, 4096);
				if (result > 0)
				{
					buffer3[result] = '\0';
					if (strstr(buffer3, "no shape"))
					{
						fprintf(logfile, "Sore leg\n");
						sprintf(buffer, " n50s");
						goto sendtheaction;
					}
					goto getmessage;
				}
			}
		}
		memset(buffer, 0x00, 4097);
		//dsleep(1);
		if (needlongpause)
		{
			dusleep(2000000);
		}
		else
		{
			dusleep(100000);
		}
		for (idx = 0; idx < 10; idx++)
		{
			dusleep(10000);
			result = read(fd2[0], buffer, 4096);
			if (result > 0)
			{
				buffer[result] = '\0';
				printf("%s\n", buffer);
				handlemessage(buffer, "post");
			}
		}
		Write(fd1[1], " ");
		// deliberately not dusleep
		if (!playlocal)
		{
			usleep(250000);
		}
		else
		{
			usleep(100000);
		}
		lastmyx = myx;
		lastmyy = myy;
		turncount++;
	}
}

int main(int argc, char *argv[])
{
	int result;
	char slavedev[256];
	int masterfd;
	int stdoutfd;
	int stdinfd;
	int stderrfd;

	signal(SIGUSR2, sigint);
	signal(SIGUSR1, sigusr1);

	if (argc >= 2 && !strcmp(argv[1], "local"))
	{
		playlocal = 1;
	}

	if (pipe(fd1) < 0 || pipe(fd2) < 0)
	{
		printf("Error creating pipes\n");
		return 1;
	}

	srand(time(0));

	refreshtimer = time(NULL) + REFRESH_TIMER;
	inventorytimer = time(NULL) + INVENTORY_TIMER;

	if (!playlocal)
	{
		pid = fork();
		if (pid)
		{
			game();
			kill(0, SIGINT);
		}
		else
		{
			if (fd1[0] != STDIN_FILENO)
			{
				dup2(fd1[0], STDIN_FILENO);
			}
			if (fd2[1] != STDOUT_FILENO)
			{
				dup2(fd2[1], STDOUT_FILENO);
			}
			result = execl("/usr/bin/telnet", "/usr/bin/telnet", "nethack.alt.org", NULL);
			if (result < 0)
			{
				perror("Bugger on the exec\n");
			}
		}
	}
	else
	{
		strcpy(slavedev, "");
		pid = forkpty(&masterfd, slavedev, NULL, NULL);
		if (pid == -1)
		{
			fprintf(stderr, "Failed to forkpty()\n");
		}
		else if (pid)
		{
			fd2[0] = masterfd;
			fd1[1] = masterfd;
			game();
			// don't need masterfd anymore at this point
			waitpid(-1, &masterfd, 0);
		}
		else
		{
			//printf("Testing stdout\n");
			result = execl(LOCAL_NETHACK, LOCAL_NETHACK, NULL);
			if (result < 0)
			{
				perror("Bugger on the exec\n");
			}
		}
	}

	return 0;
}

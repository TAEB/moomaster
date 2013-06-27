#ifndef movement_h
#define movement_h

typedef struct node_s
{
	int x;
	int y;
	unsigned char closed;
	unsigned char direction;
	int score;
	unsigned char used;
	struct node_s *parent;
} node_t;

unsigned char impassable(char c, int x, int y);
unsigned char isdoorway(int x, int y, unsigned char isdoorchar);
void whereidbe(char direction, int *x, int *y);
char pickrandomdirection();
int traffictimeout();
int findunexploredarea(int *x, int *y, char *target);
int findinterestingwall(int *x, int *y, char *target);
void findinterestingcorridor(int *x, int *y);
int makescore(int dx, int dy, int x, int y, char target);
char trafficchar(int index);
void dump_traffic();
void dump_nodes();
void spawnnode(int x, int y, char direction, node_t *parent, char target, int targetx, int targety, int cx, int cy, unsigned char test);
void spawnnodesaround(int x, int y, node_t *parent, char target, int targetx, int targety, unsigned char test);
char pathto(int x, int y, char target, unsigned char test);

#endif

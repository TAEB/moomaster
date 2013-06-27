#ifndef combat_h
#define combat_h

#define VULNERABILITY_CUTOFF	3

int assessvulnerability(int dx, int dy);
int scorecombatlocation(int dx, int dy);
const char *handlecombat(int tx, int ty, char target);

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int scorethreshold = 2500;
	char path[128] = "score.txt";
	FILE *filePtr;
	char line[1024];
	int score[4096];
	int idx;
	int totalgames = 0;
	int gamesabove = 0;
	int gamesabovelast10 = 0;

	printf("Usage: %s <score file> <threshold value>\n", argv[0]);
	if (argc >= 2)
	{
		strncpy(path, argv[1], 127);
	}
	if (argc >= 3)
	{
		scorethreshold = atoi(argv[2]);
	}

	filePtr = fopen(path, "rb");
	if (!filePtr)
	{
		perror("Unable to open input file");
		exit(0);
	}

	while (1)
	{
		if (!fgets(line, 1023, filePtr))
		{
			break;
		}
		sscanf(line, "3.4.3 %i", &score[totalgames]);
		if (score[totalgames] >= scorethreshold)
		{
			gamesabove++;
		}
		totalgames++;
	}

	for (idx = totalgames - 1; idx >= totalgames - 10; idx--)
	{
		if (score[idx] >= scorethreshold)
		{
			gamesabovelast10++;
		}
	}

	printf("%i of %i games are >= %i points (%.2f%%)\n", gamesabove, totalgames, scorethreshold, (float)gamesabove / (float)totalgames * 100.0);
	printf("%i of last 10 games are >= %i points (%.2f%%)\n", gamesabovelast10, scorethreshold, (float)gamesabovelast10 / (float)10 * 100.0);

	fclose(filePtr);

	return 0;
}

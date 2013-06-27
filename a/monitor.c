#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <unistd.h>

#define WINDOW_FRAMEBUFFER	0
#define WINDOW_NOTEBUFFER	1
#define WINDOW_TRAFFICBUFFER	2
#define WINDOW_NODEBUFFER	3
#define WINDOW_MAX		4

unsigned char done = 0;
int window = WINDOW_FRAMEBUFFER;
FILE *filePtr[WINDOW_MAX];

int main(int argc, char *argv[])
{
	int c;
	char c2 = 'A';
	int idx;
	WINDOW *w;

	window = WINDOW_FRAMEBUFFER;
	w = initscr();
	noecho();
	cbreak();
	nodelay(w, true);

	while (!done)
	{
		c = getch();
		if (c == 'q')
		{
			done = 1;
		}
		else if (c == 'f')
		{
			window = WINDOW_FRAMEBUFFER;
		}
		else if (c == 'n')
		{
			window = WINDOW_NOTEBUFFER;
		}
		else if (c == 't')
		{
			window = WINDOW_TRAFFICBUFFER;
		}
		else if (c == 'p')
		{
			window = WINDOW_NODEBUFFER;
		}
		else if (c == 'k')
		{
			system("killall -HUP moomaster");
		}
		else if (c == 'u')
		{
			system("killall -USR1 moomaster");
		}
		/*if (!filePtr[window])
		{
			continue;
		}*/
		erase();
		filePtr[WINDOW_FRAMEBUFFER] = fopen("framebuffer.txt", "rb");
		filePtr[WINDOW_NOTEBUFFER] = fopen("notes.txt", "rb");
		filePtr[WINDOW_TRAFFICBUFFER] = fopen("traffic.txt", "rb");
		filePtr[WINDOW_NODEBUFFER] = fopen("nodes.txt", "rb");
		for (idx = 0; idx < 80 * 25; idx++)
		{
			mvaddch(idx / 80, idx % 80, fgetc(filePtr[window]));
			if ((idx % 80) == 79)
			{
				fgetc(filePtr[window]);
			}
		}
		fclose(filePtr[WINDOW_FRAMEBUFFER]);
		fclose(filePtr[WINDOW_NOTEBUFFER]);
		fclose(filePtr[WINDOW_TRAFFICBUFFER]);
		fclose(filePtr[WINDOW_NODEBUFFER]);
		refresh();
		usleep(100000);
	}

	nodelay(w, false);
	nocbreak();
	echo();
	endwin();

	return 0;
}

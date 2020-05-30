#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/stat.h>

/* Init variables */
int y, x, starty, startx, maxY, maxX;

/* Holds window border characters */
typedef struct windowBorder {
		chtype ls, rs, ts, bs, tl, tr, bl, br;
} WIN_BORDER;

/* Holds properties of a window */
typedef struct windowStruct {
		int startx, starty;
		int height, width;
		WIN_BORDER border;
} WIN;

/* Function used to store properties of a window */
void initWin(WIN *p_win, int rows, int cols);

/* Function used to check if path is valid */
int validPath(const char *path);

int main(int argc, char *argv[]) {

		char path[100];
		char query[100];
		char status[100];
		char prompt[100];
		
		/* Exit if incorrect execution is used */
		if(argc != 2) {
				printf("Usage: ./BatchDelete [Path to search]\n");
				exit(1);
		}
		
		/* Copy input supplied as the path */
		strcpy(path, argv[1]);
		
		/* Verify path is valid before proceeding */
		if(validPath(path)) 
		{
				printf("Path at '%s' valid, proceeding to screen...\n", path);
		} 
		else 
		{
				printf("Path '%s' invalid, exiting...\n", path);
				exit(1);
		}
		
		/* Initialize windows */
		WINDOW *status_win;
		initWin(status_win, 4, COLS);
		
		/* Load initial prompt */
		strcpy(prompt, "Query");
		
		/* Start ncurses session */
		initscr();
		clear();
		start_color();
		getyx(stdscr, y, x);
		getmaxyx(stdscr, maxY, maxX);
		
		status_win = newwin(5, 40, y, x);
		box(status_win, 0, 0);
		wrefresh(status_win);
		attron(A_BOLD);
		mvprintw(0, 0, "BatchDelete, a program by Joshua Harmon"); attroff(A_BOLD);
		mvprintw(1, 0, "Path		-> '%s'", path);
		mvprintw(2, 0, "Query		-> '%s'", query); 
		mvprintw(3, 0, "Status	-> '%s'", status);
		mvwprintw(stdscr, 20, 0, "%s ->", prompt);
		refresh();
		
		getch();
		endwin();
		return 0;
}

void initWin(WIN *p_win, int rows, int cols, int y, int x) {
		p_win->height = rows;
		p_win->width = cols;

		p_win->starty = y;
		p_win->startx = x;
		
		p_win->border.ls = '|';
		p_win->border.rs = '|';
		p_win->border.ts = '-';
		p_win->border.bs = '-';
		p_win->border.tl = '+';
		p_win->border.tr = '+';
		p_win->border.bl = '+';
		p_win->border.br = '+';
}

/* Function to draw windows. Places box around it as well. */
WINDOW *drawWin(int height, int width, int starty, int startx) {
	WINDOW *temp;

	temp = newwin(height, width, starty, startx);
	box(temp, 0, 0);
	wrefresh(temp);
	return temp;
}

int validPath(const char *path) {
		struct stat stats;
		
		stat(path, &stats);
		
		if(S_ISDIR(stats.st_mode))
				return 1;
		
		return 0;
}

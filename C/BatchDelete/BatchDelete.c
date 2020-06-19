#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <sys/stat.h>
#include <panel.h>
#include <fcntl.h> 
#include <form.h>
#include "WinManage.h"
#include "BatchLogic.h"


/* Init variables */
int y, x, starty, startx, maxY, maxX;

/* Variables used for field input and other diagnostic displays */
char path[100], query[100], status[100], prompt[100], input[75], filename[30];

/* Declare windows */
WINDOW *status_win, *query_win, *res_win, *hist_win;

/* Declare file */
FILE *hist_file;

/* Fields for form */
FORM *form;
FIELD *my_fields[4];

/* Function used to check if path is valid */
int validPath(const char *path);

/* History functions */
int updateHist(char *historyUpdate); 

int main(int argc, char *argv[]) {
  
  /* Exit if incorrect execution is used */
  if(argc != 2) {
	printf("Usage: ./BatchDelete [Path to search]\n");
	exit(1);
  }
  
  /* Copy input supplied as the path */
  strcpy(path, argv[1]);
  
  /* Verify path is valid before proceeding */
  if(!validPath(path)) 
  {
	printf("Path '%s' invalid, exiting...\n", path);
	exit(1);
  }
  
  /* Load history file */
  snprintf(filename, sizeof filename, "history.txt");
  int fdout = open(filename, O_RDWR | O_CREAT | O_APPEND, 0666);
  
  /* If file creation succeeds, dup2, else exit */
  if(fdout < 0) {
    printf("History file creation failed, extiting...\n");
    exit(-1);
  }
  
  /* Load initial prompt and status */
  strcpy(prompt, "Type in a search query");
  strcpy(status, "Awaiting query...");
  
  /* Field assignments */
  my_fields[0] = new_field(1, maxX - 17, 2, 1, 0, 0); /* Path */
  my_fields[1] = new_field(1, maxX - 17, 3, 1, 0, 0); /* Query */
  my_fields[2] = new_field(1, maxX - 17, 4, 1, 0, 0); /* Status */
  my_fields[3] = new_field(1, maxX - 1, maxY - 2, 3, 0, 0); /* Input */
  
  /* Start ncurses session */
  initscr();
  getmaxyx(stdscr, maxY, maxX);
  move(maxY - 2, 2);
  refresh();
  
  /* Check if terminal supports colors */
  if(has_colors() == FALSE)
  {
	endwin();
	printf("Terminal does not support ncurses colors. Exiting...\n");
	exit(1);
  }
  else if(maxX < 80 || maxY < 40) {
    endwin();
	printf("Terminal too small, resize to run. Exiting...\n");
	exit(1);
  }
  
  start_color();
  
  /* Initialize windows */
  WINDOW *status_win = initWin(6, maxX - 17, 0, 0);
  WINDOW *query_win = initWin(3, maxX - 1, maxY - 3, 0);
  WINDOW *res_win = initWin(maxY - 9, maxX/2 - 1, 6, 0);
  WINDOW *hist_win = initWin(maxY - 9, maxX/2, 6, (COLS/2) - 1);
  
  /* Initialize colors */
  init_pair(1, COLOR_GREEN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_CYAN, COLOR_BLACK);
  
  /* Print exit message on top right */
  char exitMSG[] = "Enter 'q' to exit";
  attron(A_BOLD | COLOR_PAIR(2));
  mvprintw(0, (COLS - strlen(exitMSG)), exitMSG);
  attroff(A_BOLD | COLOR_PAIR(2));
  
  /* Print messages for status window */
  wattron(status_win, A_BOLD | COLOR_PAIR(3));
  mvwprintw(status_win, 0, 2, "[ BatchDelete, a program by Joshua Harmon ]"); wattroff(status_win, A_BOLD | COLOR_PAIR(3));
  printStatus(status_win, query, path, status);
  
  /* Print history window */
  wattron(hist_win, A_BOLD | COLOR_PAIR(3));
  mvwprintw(hist_win, 0, 2, "[ History ]");
  wattroff(hist_win, A_BOLD | COLOR_PAIR(3));
  
  /* Print query window */
  printPrompt(query_win, prompt);
  mvwprintw(query_win, 1, 1, ">");
  
  /* Print results window */
  wattron(res_win, A_BOLD | COLOR_PAIR(3));
  mvwprintw(res_win, 0, 2, "[ Results ]");
  wattroff(res_win, A_BOLD | COLOR_PAIR(3));
  
  wnoutrefresh(query_win);
  wnoutrefresh(res_win);
  wnoutrefresh(status_win);
  wnoutrefresh(hist_win);
  doupdate();
  
  /* While user has not entered "q", continue running */
  while(input[0] != 'q') {
	if(mvgetstr(maxY - 2, 3, input) != ERR && inputHandler(input, 0))
	{
	  sprintf(status, "Read '%s' from input", input);
	  printStatus(status_win, query, path, status);
	  wnoutrefresh(status_win);
    
	} else {
	  sprintf(prompt, "Invalid input, try again. inputHandler: %d", inputHandler(input, 0));
	  printStatus(status_win, query, path, status);
	  printPrompt(query_win, prompt);
	  wnoutrefresh(status_win);
	  wnoutrefresh(query_win);
	}
	doupdate();
  }
  
  /* Free all memory */
  delwin(status_win);
  delwin(query_win);
  delwin(hist_win);
  delwin(res_win);
  endwin();
  return 0;
}

int validPath(const char *path) 
{
  struct stat stats;
  stat(path, &stats);
  if(S_ISDIR(stats.st_mode))
	return 1;
  return 0;
}

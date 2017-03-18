#include <iostream>
#include <map>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/ioctl.h>
#include <vector>
#include <ncurses.h>

#include "error.h"
#include "utils.h"
#include "terminal.h"
#include "terminal_ncurses.h"

bool term_change = false;

void sigh(int sig)
{
	// will always be SIGWINCH
	signal(SIGWINCH, sigh);

	term_change = true;
}

terminal_ncurses::terminal_ncurses(int verbosity_in, bool colors_in, std::string log_file_in) : terminal(verbosity_in, colors_in, log_file_in)
{
	bottom_win = line_win = top_win = NULL;
	bottom_n = top_n = 0;

        initscr();
	start_color();
        keypad(stdscr, TRUE);
        intrflush(stdscr, FALSE);
        noecho();
        nonl();
        refresh();
        nodelay(stdscr, FALSE);
        meta(stdscr, TRUE);     /* enable 8-bit input */
        idlok(stdscr, TRUE);    /* may give a little clunky screenredraw */
        idcok(stdscr, TRUE);    /* may give a little clunky screenredraw */
        leaveok(stdscr, FALSE);

	init_pair(C_WHITE, COLOR_WHITE, COLOR_BLACK);
	init_pair(C_CYAN, COLOR_CYAN, COLOR_BLACK);
	init_pair(C_MAGENTA, COLOR_MAGENTA, COLOR_BLACK);
	init_pair(C_BLUE, COLOR_BLUE, COLOR_BLACK);
	init_pair(C_YELLOW, COLOR_YELLOW, COLOR_BLACK);
	init_pair(C_GREEN, COLOR_GREEN, COLOR_BLACK);
	init_pair(C_RED, COLOR_RED, COLOR_BLACK);

	pthread_mutex_init(&lck, NULL);

	recreate_terminal();
}

terminal_ncurses::~terminal_ncurses()
{
	delwin(bottom_win);
	delwin(line_win);
	delwin(top_win);

	endwin();

	std::map<std::string, location_t *>::iterator it = sensor_map.begin();
	for(; it != sensor_map.end(); it++)
		delete it -> second;

	pthread_mutex_destroy(&lck);
}

void terminal_ncurses::allocate_space(int width, color_t c, std::string symbol)
{
	location_t *loc = new location_t;

	loc -> x = -1;
	loc -> y = -1;
	loc -> width = width;
	loc -> c = c;

	sensor_map.insert(std::pair<std::string, location_t *>(symbol, loc));
}

void terminal_ncurses::place_sensors()
{
	memset(line_usage, 0x00, sizeof line_usage);

	std::map<std::string, location_t *>::iterator it = sensor_map.begin();

	for(; it != sensor_map.end(); it++)
	{
		for(unsigned int line=0; line<256; line++)
		{
			unsigned int w = line_usage[line];
			if (w)
				w++;

			if (w + it -> second -> width <= max_x)
			{
				line_usage[line] += (line_usage[line] ? 1 : 0) + it -> second -> width;

				it -> second -> x = w;
				it -> second -> y = line;
				break;
			}
		}
	}
}

void terminal_ncurses::determine_terminal_size(unsigned int *max_y, unsigned int *max_x)
{
        struct winsize size;

        *max_x = *max_y = 0;

        if (ioctl(1, TIOCGWINSZ, &size) == 0)
        {
                *max_y = size.ws_row;
                *max_x = size.ws_col;
        }

        if (!*max_x || !*max_y)
        {
                char *dummy = getenv("COLUMNS");
                if (dummy)
                        *max_x = atoi(dummy);
                else
                        *max_x = 80;

                dummy = getenv("LINES");
                if (dummy)
                        *max_x = atoi(dummy);
                else
                        *max_x = 24;
        }
}

void terminal_ncurses::update_terminal()
{
	pthread_mutex_lock(&lck);

	wmove(line_win, 0, max_x - 1);
	wnoutrefresh(bottom_win);
	wnoutrefresh(top_win);
	doupdate();

	pthread_mutex_unlock(&lck);
}

void terminal_ncurses::recreate_terminal()
{
	term_change = false;

	determine_terminal_size(&max_y, &max_x);

	resizeterm(max_y, max_x);

        endwin();
        refresh(); /* <- as specified by ncurses faq, was: doupdate(); */

	place_sensors();

	create_windows();
}

void terminal_ncurses::create_windows()
{
	if (bottom_win)
		delwin(bottom_win);
	if (line_win)
		delwin(line_win);
	if (top_win)
		delwin(top_win);

	int last_n = max_y - 2;
	if (last_n > 255)
		last_n = 255;
	top_n = last_n;
	for(int index=0; index<last_n; index++)
	{
		if (line_usage[index] == 0)
		{
			top_n = index;
			break;
		}
	}

	bottom_n = max_y - (top_n + 1);

	top_win = newwin(top_n, max_x, 0, 0);

	line_win = newwin(1, max_x, top_n, 0);
	scrollok(line_win, false);
	draw_status();

	bottom_win = newwin(bottom_n, max_x, top_n + 1, 0);

	wnoutrefresh(line_win);

	scrollok(top_win, false);
	wmove(top_win, 0, 0);
	wclear(top_win);

	scrollok(bottom_win, true);
	wmove(bottom_win, 0, 0);
	wclear(bottom_win);

	doupdate();

	signal(SIGWINCH, sigh);
}

void terminal_ncurses::emit(int ll, std::string what)
{
	dolog(ll, what);

	pthread_mutex_lock(&lck);

	if (bottom_win)
	{
		if (term_change)
			recreate_terminal();

		if (ll <= verbosity)
			wprintw(bottom_win, "%s\n", what.c_str());
	}

	pthread_mutex_unlock(&lck);
}

void terminal_ncurses::put(std::string symbol, std::string what)
{
	std::map<std::string, location_t *>::iterator it = sensor_map.find(symbol);
	if (it == sensor_map.end())
		return;

	if (it -> second -> x == -1)
		return;
	if (it -> second -> y == -1)
		return;
	if (it -> second -> x >= max_x)
		return;
	if (it -> second -> y >= max_y)
		return;
	if (it -> second -> x + what.size() > max_x)
		return;

	int x = it -> second -> x;
	int y = it -> second -> y;
	int c = it -> second -> c;

	pthread_mutex_lock(&lck);

	if (top_win)
	{
		if (term_change)
			recreate_terminal();

		int attr = 0;

		if (colors)
		{
			if (c == C_BLUE)
				attr = A_BOLD | COLOR_PAIR(c);
			else
				attr = COLOR_PAIR(c);
		}

		wattron(top_win, attr);

		std::vector<std::string> parts = split_string(what, "|");
		if (parts.size() != 2 && parts.size() != 1)
			error_exit("String '%s' is not 1 or 2 parts", what.c_str());

		mvwprintw(top_win, y, x, "%s", parts.at(0).c_str());

		if (parts.size() == 2)
		{
			bool bold = false;

			std::string val = parts.at(1);
			if (val.substr(0, 1) == "!")
			{
				bold = true;
				val = val.substr(1);
			}

			wattron(top_win, A_REVERSE);
			if (bold)
				wattron(top_win, A_STANDOUT | A_BOLD);
			mvwprintw(top_win, y, x + parts.at(0).size(), "%s", val.c_str());
			if (bold)
				wattroff(top_win, A_STANDOUT | A_BOLD);
			wattroff(top_win, A_REVERSE);
		}

		wattroff(top_win, attr);
	}

	pthread_mutex_unlock(&lck);
}

void terminal_ncurses::draw_status()
{
	if (line_win)
	{
		wmove(line_win, 0, 0);

		for(unsigned int index=0; index<max_x; index++)
			waddch(line_win, ' ' | A_REVERSE);

		wattron(line_win, A_REVERSE);
		mvwprintw(line_win, 0, 0, last_status.c_str());
		wattroff(line_win, A_REVERSE);

		wnoutrefresh(line_win);
	}
}

void terminal_ncurses::set_status(std::string s)
{
	last_status = s;
	dolog(LL_DEBUG_SOME, s);

	pthread_mutex_lock(&lck);

	draw_status();

	pthread_mutex_unlock(&lck);
}

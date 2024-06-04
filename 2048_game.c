@ -1,77 +1,77 @@
/* Clone of the 2048 sliding tile puzzle game. (C) Wes Waugh 2014
 *
 * This program only works on Unix-like systems with curses. Link against
 * the curses library. You can pass the -lcurses flag to gcc at compile
 * time.
 *
 * This program is free software, licensed under the GPLv3. Check the
 * LICENSE file for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#define NROWS 4
#define NCOLS NROWS

static const char *usage =
	"2048: A sliding tile puzzle game\n\n"
	"Usage: %s [-r R] [-p P] [-s SEED] [-h]\n\n"
	"\t-r\tR\tRecord to file R\n"
	"\t-p\tP\tPlay back from file P\n"
	"\t-s\tSEED \tUse SEED for the random number generator\n"
	"\t-d\tDELAY\tDelay for DELAY ms when playing back\n"
	"\t-h\t\tShow this message\n";

typedef int tile;

struct game {
	int turns, score;
	tile board[NROWS][NCOLS];
};

static FILE *recfile = NULL, *playfile = NULL;
static int batch_mode;
static int delay_ms = 250;

// place_tile() returns 0 if it did place a tile and -1 if there is no open
// space.
int place_tile(struct game *game)
{
	// lboard is the "linear board" -- no need to distinguish rows/cols
	tile *lboard = (tile *)game->board;
	int i, num_zeros = 0;

	// Walk the board and count the number of empty tiles
	for (i = 0; i < NROWS * NCOLS; i++) {
		num_zeros += lboard[i] ? 0 : 1;
	}

	if (!num_zeros) {
		return -1;
	}

	// Choose the insertion point
	int loc = random() % num_zeros;

	// Find the insertion point and place the new tile
	for (i = 0; i < NROWS * NCOLS; i++) {
		if (!lboard[i] && !(loc--)) {
			lboard[i] = random() % 10 ? 1 : 2;
			return 0;
		}
	}
	assert(0);
}

void print_tile(int tile)
void print_tile(int tile)
{
	if (tile) {
		if (tile < 6)
@ -85,45 +85,22 @@ int place_tile(struct game *game)
	else {
		printw("   .");
	}
}




void print_game(const struct game *game)
{
    int r, c;
    move(0, 0);
    printw("Score: %6d  Turns: %4d", game->score, game->turns);
    for (r = 0; r < NROWS; r++) {
        for (c = 0; c < NCOLS; c++) {
            print_tileGUI(r, c, game->board[r][c]);
        }
    }

    refresh();
}
	int r, c;
	move(0, 0);
	printw("Score: %6d  Turns: %4d", game->score, game->turns);
	for (r = 0; r < NROWS; r++) {
		for (c = 0; c < NCOLS; c++) {
			move(r + 2, 5 * c);
			print_tile(game->board[r][c]);
		}
	}

	refresh();
}

int combine_left(struct game *game, tile row[NCOLS])
{
@ -173,18 +150,16 @@ void rotate_clockwise(struct game *game)

void move_left(struct game *game)
{
    int r, ret = 0;
    for (r = 0; r < NROWS; r++) {
        tile *row = &game->board[r][0];
        ret |= deflate_left(row);
        ret |= combine_left(game, row);
        ret |= deflate_left(row);
    }

    game->turns += ret;
}

	int r, ret = 0;
	for (r = 0; r < NROWS; r++) {
		tile *row = &game->board[r][0];
		ret |= deflate_left(row);
		ret |= combine_left(game, row);
		ret |= deflate_left(row);
	}

	game->turns += ret;
}

void move_right(struct game *game)
{
@ -344,40 +319,40 @@

		int key = get_input();
		switch (key) {
		case 'a': case KEY_LEFT: move_left(&game); break;
		case 's': case KEY_DOWN: move_down(&game); break;
		case 'w': case KEY_UP: move_up(&game); break;
		case 'd': case KEY_RIGHT: move_right(&game); break;
		case 'h': case KEY_LEFT: move_left(&game); break;
		case 'j': case KEY_DOWN: move_down(&game); break;
		case 'k': case KEY_UP: move_up(&game); break;
		case 'l': case KEY_RIGHT: move_right(&game); break;
		case 'q':
			exit_msg = "quit";
			goto end;
		}

		if (last_turn != game.turns) {
			place_tile(&game);
			record(key, &game);
		}
	}

lose:
	if (batch_mode) {
		return 0;
	}

	move(7, 0);
	printw("You lose! Press q to quit.");
	while (getch() != 'q');
end:
	if (batch_mode) {
		return 0;
	}

	endwin();
	printf("You %s after scoring %d points in %d turns, "
		"with largest tile %d\n",
		exit_msg, game.score, game.turns,
		1 << max_tile((tile *)game.board));
	return 0;
}


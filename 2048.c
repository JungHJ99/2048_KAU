#include "2048.h"


const char *GAME_RECORDS_FILE = "game_records.txt";

const char* get_mode_string(int mode) 
{
	switch (mode) 
	{
        	case 1: return "Normal";
        	case 2: return "Bomb";
        	case 3: return "Chance";
        	case 4: return "TimeAttack";
		case 5: return "To 2000 score";
		case 6: return "In 100, To 1000";
		default: return "Normal";
    	}
}

// 게임 정보를 파일에 기록하는 함수
void record_game_info(struct game_record game_info)
{	FILE *record_file = fopen(GAME_RECORDS_FILE, "a"); // 기록 파일을 쓰기 모드로 열기
    	if (record_file != NULL) 
	{
        	// 게임 정보를 파일에 기록
        	fprintf(record_file, "Mode: %s, Score: %d, Turns: %d, Time: %.2f seconds\n",
                	get_mode_string(game_info.mode),game_info.score, game_info.turns, game_info.elapsed_time);
        	fclose(record_file); // 파일 닫기
    	}
	else
	{
        	perror("Failed to open game records file");
    	}
}

// 저장된 모든 게임 기록을 파일에서 읽어와서 출력하는 함수
void show_all_game_records() 
{
    	FILE *record_file = fopen(GAME_RECORDS_FILE, "r"); // 기록 파일을 읽기 모드로 열기
    	if (record_file != NULL) 
	{
        	printf("All Game Records:\n");
        	char buffer[256]; // 한 줄씩 읽어올 버퍼

        	// 파일에서 한 줄씩 읽어와서 출력
        	while (fgets(buffer, sizeof(buffer), record_file) != NULL) 
		{
            		printf("%s", buffer);
        	}
        fclose(record_file); // 파일 닫기
    	}
	else
	{
        	perror("Failed to open game records file");
    	}
}

// 업적을 파일에 기록하는 함수
void record_achievement(int score)
{
	const char *achievement_file = "achievements.txt";
	FILE *record_achievement_file = fopen(achievement_file, "a");
	if (!record_achievement_file)
	{
		perror("Failed to open achievement file");
		return;
	}

	// 날짜 가져오기
	time_t t = time(NULL);
	struct tm *tm_info = localtime(&t);
	char date[20];
	strftime(date, sizeof(date), "%Y.%m.%d", tm_info);

	fprintf(record_achievement_file, "%d : %s\n", score, date); // 업적을 파일에 기록
	fclose(record_achievement_file);
}

// 업적을 확인하고 기록하는 함수
void check_and_record_achievements(int score)
{
	int milestones[] = {1000, 2048, 5000, 10000};
	const int num_milestones = sizeof(milestones) / sizeof(milestones[0]);
	char buffer[256];
	FILE *record_achievement_file = fopen("achievements.txt", "r"); // 업적 파일을 읽기 모드로 열기
	if (record_achievement_file)
	{
		while (fgets(buffer, sizeof(buffer), record_achievement_file))
		{
			int recorded_score;
			sscanf(buffer, "%d", &recorded_score); // 읽어온 줄에서 점수를 추출
			for (int i = 0; i < num_milestones; i++)
			{
				if (recorded_score == milestones[i])
				{
					milestones[i] = -1;
				}
			}
		}
		fclose(record_achievement_file);
	}
	for (int i = 0; i < num_milestones; i++)
	{
		if (milestones[i] != -1 && score >= milestones[i])
		{
			record_achievement(milestones[i]);
		}
	}
}

struct game prev_game; // 이전 상태를 저장하기 위한 구조체
int can_undo = 0; // undo 가능 여부를 나타내는 플래그

void copy_game_state(struct game *dest, const struct game *src)
{
	memcpy(dest, src, sizeof(struct game));
}

void save_game_state(struct game *game)
{
	copy_game_state(&prev_game, game);
	can_undo = 1;
}

void undo_game_state(struct game *game)
{
	if (can_undo)
	{
		copy_game_state(game, &prev_game);
		can_undo = 0;
	}
}

int place_tile(struct game *game, TileType tile_type)
{
	tile *lboard = (tile *)game->board;
	int i, num_zeros = 0;

	// 빈 포인트 개수 새기
	for (i = 0; i < NROWS * NCOLS; i++)
	{
		num_zeros += lboard[i] ? 0 : 1;
	}

	if (!num_zeros)
	{
		// 빈 포인트가 없을 때는 -1을 반환
		return -1;
	}

	// 타일 생성 포인트 선택
	int loc = random() % num_zeros;

	// 새로운 타일 생성
	for (i = 0; i < NROWS * NCOLS; i++)
	{
		if (!lboard[i] && !(loc--))
		{
			switch (tile_type)
			{
			case Number:
				lboard[i] = 1;
				return 0;
			case Bomb:
				lboard[i] = 15; // 폭탄 타일 (타일 넘버 15)
				return 0;
			case Chance:
				if (random() % 10 < 1)
				{
					lboard[i] = 2;
				}
				else if (random() % 10 == 9) // 찬스 등장 확률: 1/10
				{
					lboard[i] = 16; // 찬스 타일 (타일 넘버 16)
				}
				else
				{
					lboard[i] = 1;
				}
				return 0;
			}
		}
	}
	assert(0);
}

void print_tile(int tile) {
    	int pair;

    	if (tile)
	{
        	if (tile == 15) //폭탄 타일
		{
            		pair = COLOR_PAIR(8); // 새로운 색상 조합
            		attron(pair | A_BLINK); // 반짝이는 효과 추가
            		printw("   X");
            		attroff(pair | A_BLINK);
        	}
		else if (tile == 16) // 찬스 타일
            	{
			pair = COLOR_PAIR(9); // 또 다른 색상 조합
            		attron(pair | A_BOLD);
            		printw("   O");
            		attroff(pair | A_BOLD);
        	}
		else
		{
            		pair = COLOR_PAIR(1 + (tile % 6));
            		attron(pair);
            		if (tile > 10) attron(A_BLINK); // 값이 큰 타일에는 반짝이는 효과 추가
            		printw("%4d", 1 << tile);
            		attroff(pair | A_BLINK);
        	}
        attroff(A_BOLD);
	}
	else
 	{
        	printw("   .");
    	}
}


void print_game(const struct game* game)
{
	int r, c;
	move(0, 0);
	printw("Score: %6d  Turns: %4d", game->score, game->turns);

	struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time); // 현재 시간 얻어오기

	// 현재 시간과 시작 시간을 비교해서 경과된 시간 계산
	elapsed_time = (current_time.tv_sec - start_time.tv_sec) +
							(current_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;


	mvprintw(1,0,"Time: %.2f seconds", elapsed_time); // curse 라이브러리에 있는 print함수

	for (r = 0; r < NROWS; r++) 
	{
		for (c = 0; c < NCOLS; c++) 
		{
			move(r + 2, 5 * c);
			print_tile(game->board[r][c]);
		}
	}
  	refresh();
}

int combine_left(struct game *game, tile row[NCOLS])
{
	int c, did_combine = 0;
	for (c = 1; c < NCOLS; c++)
	{
		if (row[c])
		{
			if (row[c - 1] == 16 && row[c] == 16) // 찬스 타일과 찬스 타일이 만나면 2가 됨.
			{
				row[c - 1] = 2;
				row[c] = 0;
				game->score += 4; // 점수 계산
				did_combine = 1;
				mvprintw(10, 0, "Combined chance and chance to 2, score: %d\n", game->score);
			}
			else if (row[c - 1] == row[c])
			{
				row[c - 1]++;
				row[c] = 0;
				game->score += (1 << row[c - 1]); // 점수 계산 (2^(row[c-1]))
				did_combine = 1;
				mvprintw(11, 0, "Combined %d and %d to %d, score: %d\n", 
					(1 << (row[c - 1] - 1)), 
					(1 << (row[c - 1] - 1)),
					(1 << row[c - 1]), // 결과 값들이 지수로 표현되는 문제 해결
					game->score);
			}
			else if (row[c - 1] == 16 || row[c] == 16) // 찬스 타일과 숫자 타일이 만나면 무조건 합쳐짐
			{
				tile combined_num = row[c - 1] == 16 ? row[c] : row[c - 1];  // 숫자 타일
				row[c - 1] = combined_num + 1;
				row[c] = 0;
				game->score += (1 << (combined_num + 1)); // 점수 계산 (2^(combined_num + 1))
				did_combine = 1;
				mvprintw(12, 0, "Combined chance and %d to %d, score: %d\n", 1 << (combined_num), 1 << (combined_num + 1), game->score);
			}
		}
	}
	check_and_record_achievements(game->score);
	return did_combine;
}

int deflate_left(tile row[NCOLS])
{
	tile buf[NCOLS] = {0};
	tile *out = buf;
	int did_deflate = 0;
	int in;

	for (in = 0; in < NCOLS; in++)
	{
		if (row[in] != 0)
		{
			*out++ = row[in];
			did_deflate |= buf[in] != row[in];
		}
	}

	memcpy(row, buf, sizeof(buf));
	return did_deflate;
}

void rotate_clockwise(struct game *game)
{
	tile buf[NROWS][NCOLS];
	memcpy(buf, game->board, sizeof(game->board));

	int r, c;
	for (r = 0; r < NROWS; r++)
	{
		for (c = 0; c < NCOLS; c++)
		{
			game->board[r][c] = buf[NCOLS - c - 1][r];
		}
	}
}

void move_left(struct game *game)
{
	int r, ret = 0;
	for (r = 0; r < NROWS; r++)
	{
		tile *row = &game->board[r][0];
		ret |= deflate_left(row);
		ret |= combine_left(game, row);
		ret |= deflate_left(row);
	}
		game->turns += ret;
}

void move_right(struct game *game)
{
	rotate_clockwise(game);
	rotate_clockwise(game);
	move_left(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
}

void move_up(struct game *game)
{
	rotate_clockwise(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
	move_left(game);
	rotate_clockwise(game);
}

void move_down(struct game *game)
{
	rotate_clockwise(game);
	move_left(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
	rotate_clockwise(game);
}

int lose_game(struct game test_game)
{
	int start_turns = test_game.turns;
	move_left(&test_game);
	move_up(&test_game);
	move_down(&test_game);
	move_right(&test_game);
	return test_game.turns == start_turns;
}

void init_curses()
{
    int bg = 0;
    initscr(); // curses 모드 시작
    start_color(); 
    cbreak();
    noecho(); 
    keypad(stdscr, TRUE); 
    timeout(100); 
    curs_set(0); 

    bg = use_default_colors() == OK ? -1 : 0; 

    init_pair(1, COLOR_WHITE, COLOR_GREEN);
    init_pair(2, COLOR_WHITE, COLOR_CYAN); 
    init_pair(3, COLOR_WHITE, COLOR_MAGENTA); 
    init_pair(4, COLOR_WHITE, COLOR_BLUE); 
    init_pair(5, COLOR_WHITE, COLOR_YELLOW); 
    init_pair(6, COLOR_WHITE, COLOR_RED); 
    init_pair(7, COLOR_WHITE, bg);
    init_pair(8, COLOR_WHITE, COLOR_RED); // 폭탄 타일용 밝은 빨간색 배경
    init_pair(9, COLOR_WHITE, COLOR_BLUE); // 찬스 타일용 밝은 파란색 배경
}
int max_tile(const tile *lboard)
{
	int i, ret = 0;
	for (i = 0; i < NROWS * NCOLS; i++)
	{
		ret = max(ret, lboard[i]);
	}
	return ret;
}

FILE *fopen_or_die(const char *path, const char *mode)
{
	FILE *ret = fopen(path, mode);
	if (!ret)
	{
		perror(path);
		exit(EXIT_FAILURE);
	}
	return ret;
}

int get_input()
{
	if (playfile)
	{
		char *line = NULL;
		size_t len = 0;
		int ret = 'q';
		if (getline(&line, &len, playfile) > 0)
		{
			ret = line[strspn(line, " \t")];
		}
		free(line);
		if (!batch_mode)
			usleep(delay_ms * 1000);
		return ret;
	}
	else
	{
		return getch();
	}
}

void record(char key, const struct game *game)
{
	if (recfile)
	{
		fprintf(recfile, "%c:%d\n", key, game->score);
	}
}

int high_score = 0; // 최고 기록을 저장하는 전역 변수

const char *get_high_score_file_name(int game_mode)
{
	switch (game_mode)
	{
	case 2:
		return "high_score_bomb.txt";
	case 3:
		return "high_score_chance.txt";
	case 4:
		return "high_score_chance.txt";
	default:
		return "high_score_default.txt";
	}
}

void load_high_score(int game_mode)
{
	const char *file_name = get_high_score_file_name(game_mode);
	FILE *high_score_file = fopen(file_name, "r"); // 해당 파일을 읽기 모드로 열기
	if (high_score_file)
	{
		fscanf(high_score_file, "%d", &high_score); // 파일에서 최고 기록 읽어오기
		fclose(high_score_file); // 파일 닫기
	}
}

void save_high_score(int game_mode, int score)
{
	const char *file_name = get_high_score_file_name(game_mode);
	FILE *high_score_file = fopen(file_name, "w"); // 해당 파일을 쓰기 모드로 열기
	if (high_score_file)
	{
		fprintf(high_score_file, "%d", score); // 파일에 최고 기록 쓰기
		fclose(high_score_file); // 파일 닫기
	}
}

// 게임 상태 저장 함수
void save_game(struct game *game, const char *filename)
{
	FILE *fp = fopen(filename, "w");
	if (fp == NULL)
	{
		perror("Error opening file");
		return;
	}
	fprintf(fp, "%d %d\n", game->turns, game->score);
	for (int r = 0; r < NROWS; r++)
	{
		for (int c = 0; c < NCOLS; c++)
		{
			fprintf(fp, "%d ", game->board[r][c]);
		}
		fprintf(fp, "\n");
	}

	fclose(fp);
}

// 게임 상태 불러오기 함수
void load_game(struct game *game, const char *filename)
{
	FILE *fp = fopen(filename, "r");
	if (fp == NULL)
	{
		perror("Error opening file");
		return;
	}
	fscanf(fp, "%d %d\n", &game->turns, &game->score);
	for (int r = 0; r < NROWS; r++)
	{
		for (int c = 0; c < NCOLS; c++)
		{
			fscanf(fp, "%d ", &game->board[r][c]);
		}
	}
	fclose(fp);
}

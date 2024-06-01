#ifndef _2048_H
#define _2048_H

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

// 타일 종류
typedef enum
{
	Number,
	Bomb,
	Chance
} TileType;

typedef int tile;

// 게임 정보 구조체
struct game
{
	int turns, score;
	tile board[NROWS][NCOLS];
};

// 게임 기록 구조체
struct game_record {
        int score;
        int turns;
        double elapsed_time;
        int mode;
};

// 게임 모드를 문자열로 변환하는 함수
const char* get_mode_string(int mode);

// 게임 정보를 파일에 기록하는 함수
void record_game_info(struct game_record game_info);

// 저장된 모든 게임 기록을 출력하는 함수
void show_all_game_records();

// 업적을 파일에 기록하는 함수
void record_achievement(int score);

// 업적을 확인하고 기록하는 함수
void check_and_record_achievements(int score);

// 게임 상태 복사 함수
void copy_game_state(struct game *dest, const struct game *src);

// 게임 상태 저장 함수
void save_game_state(struct game *game);

// 게임 상태 복구 함수
void undo_game_state(struct game *game);

// 타일 생성 함수
int place_tile(struct game *game, TileType tile_type);

// 타일 출력 함수
void print_tile(int tile);

// 게임 상태 출력 함수
void print_game(const struct game *game);

// 왼쪽으로 합치기 함수
int combine_left(struct game *game, tile row[NCOLS]);

// 왼쪽으로 밀어내기 함수
int deflate_left(tile row[NCOLS]);

// 시계 방향으로 회전 함수
void rotate_clockwise(struct game *game);

// 왼쪽으로 이동 함수
void move_left(struct game *game);

// 오른쪽으로 이동 함수
void move_right(struct game *game);

// 위쪽으로 이동 함수
void move_up(struct game *game);

// 아래쪽으로 이동 함수
void move_down(struct game *game);

// 패배 조건 확인 함수
int lose_game(struct game test_game);

// 터미널 초기화 함수
void init_curses();

// 최대 타일 값 가져오기 함수
int max_tile(const tile *lboard);

// 파일 열기 함수 (오류 발생 시 종료)
FILE *fopen_or_die(const char *path, const char *mode);

// 입력 받기 함수
int get_input();

// 게임 기록 저장 함수
void record(char key, const struct game *game);

// 게임 모드에 따른 최고 기록 파일 이름 반환 함수
const char *get_high_score_file_name(int game_mode);

// 최고 기록 불러오기 함수
void load_high_score(int game_mode);

// 최고 기록 저장 함수
void save_high_score(int game_mode, int score);

// 게임 상태 저장 함수
void save_game(struct game *game, const char *filename);

// 게임 상태 불러오기 함수
void load_game(struct game *game, const char *filename);

// 전역 변수 선언
extern struct game prev_game;
extern int can_undo;
extern FILE *recfile;
extern FILE *playfile;
extern FILE *loadfile;
extern int batch_mode;
extern int delay_ms;
extern struct timespec start_time;
extern double elapsed_time;
extern int high_score;

// 전역 상수 선언
const char *GAME_RECORDS_FILE;

#endif
#include "2048.h"

// 전역 변수 정의
struct game prev_game;
FILE *recfile = NULL;
FILE *playfile = NULL;
FILE *loadfile = NULL;
int batch_mode;
int delay_ms = 250;
struct timespec start_time;
double elapsed_time = 0;

// 사용법 문자열
static const char *usage =
        "2048: A sliding tile puzzle game\n\n"
        "Usage: %s [-m M] [-r R] [-p P] [-s SEED] [-h]\n\n"
        "\t-m\tM\tGame mode M\n"
        "\t\t\t[1]: Normal mode\n"
        "\t\t\t[2]: Bomb mode (Start with a bomb tile 0 which moves but won't be combined) \n"
        "\t\t\t[3]: Chance mode (Start with a bomb tile 0 which moves but won't be combined) \n"
        "\t-r\tR\tRecord to file R\n"
        "\t-p\tP\tPlay back from file P\n"
        "\t-s\tSEED \tUse SEED for the random number generator\n"
        "\t-d\tDELAY\tDelay for DELAY ms when playing back\n"
        "\t-h\t\tShow this message\n";

int main(int argc, char **argv)
{
    	const char *exit_msg = "";
    	struct game game = {0};
  	struct game_record current_game_info;
	int last_turn = game.turns;

	time_t seed = time(NULL);
	int opt;
	int game_mode = 0;

	while ((opt = getopt(argc, argv, "hr:p:s:d:m:l:f")) != -1)
	{
		switch (opt)
		{
		case 'm': // 게임 모드를 args로 받아와서 설정
			game_mode = atoi(optarg);
			break;
		case 'f':
			show_all_game_records();
			return 0;
		case 'r':
			recfile = fopen_or_die(optarg, "w");
			break;
		case 'p':
			playfile = fopen_or_die(optarg, "r");
			break;
		case 's':
			seed = atoi(optarg);
			break;
		case 'd':
			delay_ms = atoi(optarg);
			break;
		case 'h':
			printf(usage, argv[0]);
			exit(EXIT_SUCCESS);
		case 'l':
			loadfile = fopen_or_die(optarg, "r"); // loadfile 변수에 파일 정보 저장
			break;
		default:
			fprintf(stderr, usage, argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	srandom(seed);
	load_high_score(game_mode);

	if (loadfile) // loadfile 변수가 NULL이 아닌 경우
	{
		load_game(&game, optarg); // 게임 상태 불러오기
	}
	else
	{
		place_tile(&game, Number);
		if (game_mode == 2) // 폭탄 모드
		{
			place_tile(&game, Bomb);
		}
		place_tile(&game, Number);
	}

  	clock_gettime(CLOCK_MONOTONIC, &start_time); // 게임이 시작하면 현재 시간을 얻어와서 저장.

	batch_mode = recfile && playfile;

	if (!batch_mode)
	{
		init_curses();
	}

	while (1)
	{
		if (!batch_mode)
		{
			print_game(&game);
		}

		if (lose_game(game)) 
    		{
			// lose_game 시 게임 중단
			struct timespec end_time;
			clock_gettime(CLOCK_MONOTONIC, &end_time);
			elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
							(end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
			exit_msg = "lost";
			goto lose;
		}

		last_turn = game.turns;

		int key = get_input();
		switch (key)
		{
			char file_name[256];
			case 'a':
			case KEY_LEFT:
				save_game_state(&game);
				move_left(&game);
				break;
			case 's':
			case KEY_DOWN:
				save_game_state(&game);
				move_down(&game);
				break;
			case 'w':
			case KEY_UP:
				save_game_state(&game);
				move_up(&game);
				break;
			case 'd':
			case KEY_RIGHT:
				save_game_state(&game);
				move_right(&game);
				break;
			case 'q':
				exit_msg = "quit";
				goto end;
			case 'u': // 이전 상태로 가기
				undo_game_state(&game);
                                break;
			case 'r': // 게임 재시작
				// 게임 초기화
				game.turns = 0;
				game.score = 0;
				memset(game.board, 0, sizeof(game.board));
				// 새로운 타일 생성
				place_tile(&game, Number);
				if(game_mode == 2)
				{
					place_tile(&game, Bomb);
				}
				place_tile(&game, Number);
				// 시간 초기화
				clock_gettime(CLOCK_MONOTONIC, &start_time);
				break;
			case 'l': // 게임 저장
				if (!batch_mode)
				{
					endwin(); // curses 모드 비활성화
					printf("Enter file name: ");
					fgets(file_name, sizeof(file_name), stdin);
					file_name[strcspn(file_name, "\n")] = 0;
					save_game(&game, file_name);
					init_curses(); // curses 모드 활성화
					goto end;
				}
				break;
		}

		if (key != 'u' && last_turn != game.turns) // 이전상태로 갈 때는 새 타일 생성 안되게 하기
		{
			if (game_mode == 3) // 찬스 모드
			{
				place_tile(&game, Chance);
			}
			else
			{
				place_tile(&game, Number);
			}
			record(key, &game);

			if (game.score > high_score) // 최고 기록을 갱신했다면?
			{
				high_score = game.score; // 최고 기록 갱신
				save_high_score(game_mode, high_score); // 해당 기록 파일에 저장
				if (!batch_mode)
				{
					move(8, 0);
					printw("New high score: %d\n", high_score); // 새로운 최고 기록 유저에게 알려주기
				}
			}
		}

		if (game_mode == 4) // 타임어택 모드
		{
			if (elapsed_time >= 120.00) // 120초가 되면 게임 종료
			{
				exit_msg = "timed out";
				goto end;
			}
		}

		if (game_mode == 5) // 2000점 빨리 얻는 모드
		{
			if (game.score >= 2000)
			{
				mvprintw(9, 0, "You reached 2000 points in %.2f seconds\nPress q to quit.", elapsed_time);
				while (getch() != 'q');	
				exit_msg = "won";
				goto end;
			}
		}

		if (game_mode == 6) // 100턴안에 1000점 만들기 모드
		{
			if (game.turns == 100)
			{
				if (game.score >= 1000)
				{
					exit_msg = "won";
					goto end;
				}
				else
				{
					exit_msg = "lost";
					goto lose;
				}
			}
		}
	}

	lose:
		if (batch_mode)
		{
			return 0;
		}

		move(7, 0);
		printw("You lose! Press q to quit.");
		while (getch() != 'q');
	end:
		// 게임이 종료되면 해당 게임의 정보를 구성합니다.
                current_game_info.score = game.score;
                current_game_info.turns = game.turns;
                current_game_info.elapsed_time = elapsed_time;
                current_game_info.mode = game_mode;
                // 해당 게임의 정보를 파일에 기록합니다.
                record_game_info(current_game_info);

		if (batch_mode)
		{
			return 0;
		}

	endwin();
	printf("You %s after scoring %d points in %d turns, "
				 "with largest tile %d\n",
				 exit_msg, game.score, game.turns,
				 1 << max_tile((tile *)game.board));

	if (game.score > high_score) // 게임 종료 시 최고 기록을 갱신했다면?
	{
		printf("Congratulations! New high score: %d\n", game.score); // 게임 종료 시 새로운 최고 기록 알림
	}
	else
	{
		printf("High score: %d\n", high_score); // 최고 기록 출력
	}
  	printf("Time played: %.2f seconds\n", elapsed_time); // 진행 시간 출력
	return 0;
}
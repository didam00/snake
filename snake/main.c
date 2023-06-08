#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <math.h>
#include <conio.h>
#include <time.h>
#include <mmsystem.h> // 소리를 넣고 싶었다

// #pragma는 강력한 지시자 중 하나다
// #pragma comment(lib, string)은 라이브러리를 링크해준다
#pragma comment(lib,"winmm.lib")

#define false 0
#define true 1	
#define null 0
#define MAX 99999999

// 키 코드
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77
#define ENTER 13
#define SPACE 32
#define ESC 27

// 화면 모드
#define MAIN_SCREEN 0
#define HEIGHT 20
#define WIDTH 20

// 컬러
#define BLACK 0
#define DARK_BLUE 1
#define DARK_GREEN 2
#define DARK_SKYBLUE 3
#define DARK_RED 4
#define DARK_PURPLE 5
#define DARK_YELLOW 6
#define GRAY 7
#define DARK_GRAY 8
#define BLUE 9
#define GREEN 10
#define SKYBLUE 11
#define RED 12
#define PURPLE 13
#define YELLOW 14
#define WHITE 15

// 블럭
#define BLANK 0
#define WALL 1

// 몸통
#define HEAD 10
#define BODY 11

// 아이템
#define APPLE 20
#define BANANA 21
#define FAKE_APPLE 22
#define SODA 23
#define MATCHA 24
#define SMOKE 25
#define RAINBOW_STAR 26
#define VITAMIN 27
#define WALL_MAKER 28
#define RANDOM_BOX 29
#define APPLE_SPREAD 30

// 레이어
#define BARRIER_LAYER 0
#define SNAKE_LAYER 1
#define ITEM_LAYER 2

char screen_mode = MAIN_SCREEN;
char view_map[HEIGHT][WIDTH]; // 실제로 맵을 그릴 때 쓰이는 배열
// 미리 연산을 하고 바꿔치기할 배열
// [0] = 벽 나타내는 곳
// [1] = 뱀 머리와 몸통 나타내는 곳
// [2] = 아이템 나타내는 곳
char map[3][HEIGHT][WIDTH];
const char ITEM_LIST[9] = {APPLE, BANANA, SODA, MATCHA, VITAMIN, WALL_MAKER, RAINBOW_STAR, APPLE_SPREAD, SMOKE};
const ITEM_LIST_LENGTH = sizeof(ITEM_LIST) / sizeof(char);
// snake_body[0]은 머리다
int snake_body[(HEIGHT - 1) * (WIDTH - 1)][2] = { {10, 9}, {10, 10}, {9, 10} };
int snake_len = 3;
int snake_speed = 125;
int score = 0;
int high_score = 0;
int eventAmount = 0;
char snake_dir = UP;
// key_dir은 스네이크가 맵 배열 상 보고있는 방향과 snake_dir의 불일치때문에 만들어졌다
// key_dir은 키보드 방향키에 의해 즉각적으로 할당되지만, snake_dir와 반대방향이면 할당되지 않는다. snake_dir은 일정 시간마다 key_dir의 값을 할당받는다
char key_dir = UP;
char keyCode = 0;
char systemText[32] = "";

clock_t alertTime = 0;

void init();
void viewMain();
void viewRanking();

void addBody(int, int, int);
void removeBody(int);
void color(int, int);
void gotoxy(int, int);
void goto2xy(int, int);
void blinkFrame(char);
void alert(char*);
void randCreateBlock(char, char);
void drawFrame();
void gameOver();
void gameLoop();

// timer는 계속 1씩 감소하고 0이 되면 
//int maxTimer = 500;
//int timer = 500;

void addBody(int x, int y, int index) { // 배열의 index번째에 좌표배열을 추가
	// 먼저 index번째 뒤의 요소들은 모두 뒤쪽으로 밀어야한다
	// 앞쪽에 있는 데이터를 한칸 뒤로 빼는 식으로 할 것이기때문에 뒤쪽부터 빼야 데이터의 손실을 막을 수 있다
	// 옮기려는 위치를 기준으로 snake_len(snake_len-1+1)에서부터 index뒤에까지 내려간다
	for (int i = snake_len; i > index; i--) {
		snake_body[i][0] = snake_body[i - 1][0];
		snake_body[i][1] = snake_body[i - 1][1];
	}
	snake_body[index][0] = x;
	snake_body[index][1] = y;

	// 언제나 길이를 잘 챙긴다
	snake_len++;
}

void removeBody(int index) { // 배열의 index번째에 좌표배열을 제거
	for (int i = index; i < snake_len; i++) {
		snake_body[i][0] = snake_body[i + 1][0];
		snake_body[i][1] = snake_body[i + 1][1];
	}

	snake_len--;
}

void color(int textColor, int backColor) {
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), col);
	SetConsoleTextAttribute(handle, (backColor << 4) + textColor);
}

void gotoxy(int x, int y) {
	COORD pos = {x, y};
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void goto2xy(int x, int y) {
	gotoxy(x * 2, y);
}

void alert(char* text) {
	goto2xy(WIDTH+2, HEIGHT-3);
	color(WHITE, BLACK);
	for (int i = 0; i < 16; i++) {
		printf("  ");
	}
	goto2xy(WIDTH+2, HEIGHT-3);
	printf("* %s", text);
	alertTime = clock();
}

void drawFrame() {
	goto2xy(0, 0);
	for (int i = 0; i < WIDTH; i++) {
		printf("▧");
	}

	for (int i = 1; i < HEIGHT-1; i++) {
		goto2xy(0, i);
		printf("▧");
	}
	
	for (int i = 1; i < HEIGHT-1; i++) {
		goto2xy(WIDTH-1, i);
		printf("▧");
	}

	goto2xy(0, HEIGHT-1);
	for (int i = 0; i < WIDTH; i++) {
		printf("▧");
	}
}

void blinkFrame(char col) {
	for (int i = 0; i < 2; i++) {
		color(col, BLACK);
		drawFrame();
		Sleep(150);
		color(DARK_BLUE, BLACK);
		drawFrame();
		Sleep(150);
	}
}

void randCreateBlock(char item, char layerPos) {
	int tempArr[(WIDTH - 2) * (HEIGHT - 2)][2] = {{0, 0}};
	int arrLen = 0;
	for (int i = 1; i < WIDTH - 1; i++) {
		for (int j = 1; j < HEIGHT - 1; j++) {
			if (map[0][j][i] + map[1][j][i] + map[2][j][i] == 0) { // 모든 레이어에서 공백인 곳
				tempArr[arrLen][0] = i; // x
				tempArr[arrLen][1] = j; // y
				arrLen++;
			}
		}
	}
	if (arrLen != 0) {
		int randomPos[2];
		memcpy(randomPos, tempArr[rand()%arrLen], sizeof(randomPos));
		map[layerPos][randomPos[1]][randomPos[0]] = item;
	}
}

void gameOver() {
	FILE* fp = NULL;

	int scores[6] = { 0 };
	int scores_len = sizeof(scores) / sizeof(int);
	int tmp;

	fopen_s(&fp, "ranking.txt", "r");

	for (int i = 0; i < scores_len-1; i++) {
		fscanf_s(fp, "%d", &scores[i]);
	}

	scores[5] = score;

	for (int i = 0; i < scores_len; i++) {
		for (int j = i + 1; j < scores_len; j++) {
			if (scores[i] < scores[j]) {
				tmp = scores[i];
				scores[i] = scores[j];
				scores[j] = tmp;
			}
		}
	}

	fclose(fp);

	fopen_s(&fp, "ranking.txt", "w");
	fprintf_s(fp, "%d\n%d\n%d\n%d\n%d", scores[0], scores[1], scores[2], scores[3], scores[4]);
	fclose(fp);

	// 깜빡임
	PlaySound(TEXT("collision.wav"), 0, SND_FILENAME | SND_ASYNC);
	blinkFrame(RED);

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH+20; j++) {
			goto2xy(j, i);
			printf("  ");
		}
		Sleep(50);
	}

	color(DARK_RED, BLACK);
	gotoxy((WIDTH + 20) - 8, HEIGHT/2 - 4);
	printf("Y O U  D I E D !");
	Sleep(250);

	color(RED, BLACK);
	gotoxy((WIDTH + 20) - 8, HEIGHT/2 - 4);
	printf("Y O U  D I E D !");

	Sleep(750);
	gotoxy((WIDTH + 20) - 12, HEIGHT/2 - 1);

	if (score > high_score) {
		high_score = score;
		color(YELLOW, BLACK);
		printf("SCORE: %d ★", score);
	}
	else {
		color(WHITE, BLACK);
		printf("SCORE: %d", score);
	}
	color(WHITE, BLACK);
	gotoxy((WIDTH + 20) - 12, HEIGHT/2 + 1);
	printf("BEST:  %d", high_score);

	Sleep(500);

	color(DARK_GRAY, BLACK);
	gotoxy((WIDTH + 20) - 11, HEIGHT - 2);
	printf("press 'R' key restart");

	clock_t startTime = clock(), blinkTime = 0;
	char isVisible = true;

	while (1) {
		blinkTime = clock();
		if (_kbhit()) {
			keyCode = _getch();
			if (keyCode == ESC || keyCode == 82 || keyCode == 114) {
				break;
			}
		}
		if (blinkTime - startTime > 500) {
			startTime = clock();
			if (isVisible) {
				gotoxy((WIDTH + 20) - 11, HEIGHT - 2);
				printf("                     ");
				isVisible = false;
			}
			else {
				gotoxy((WIDTH + 20) - 11, HEIGHT - 2);
				color(DARK_GRAY, BLACK);
				printf("press 'R' key restart");
				isVisible = true;
			}
		}
	}

	switch (keyCode) {
		case ESC: viewMain(); break;
		case 82:
		case 114:
			PlaySound(TEXT("move.wav"), 0, SND_FILENAME | SND_ASYNC);
			gameLoop();
	}
}

void clearMap() {
	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			view_map[i][j] = 99;
		}
	}
}

void gameLoop() {
  // 중첩 반복문의 사용때문에 따로 변수를 만들어서 해줘야한다
	char isGameOver = false;
	char isImmune = false;
	char isSmoke = false;
	int rainbow_index = 0;
	int rainbow_col = 0; // 계속해서 바뀐다
	clock_t moveTime = clock(), ranbowTime = clock(), immuneTime = clock(), smokeTime = clock(), curTime; // clock 선언
	sprintf_s(systemText, sizeof(systemText), "mode con cols=%d lines=%d", WIDTH*2+40, HEIGHT);
	system(systemText);
	eventAmount = 1;
	snake_len = 3;
	snake_dir = UP;
	snake_speed = 125;
	key_dir = UP;
	snake_body[0][0] = 10;
	snake_body[0][1] = 9;
	snake_body[1][0] = 10;
	snake_body[1][1] = 10;
	snake_body[2][0] = 9;
	snake_body[2][1] = 10;
	score = 0;
	system("cls");

	color(WHITE, BLACK);

	// 일단 모두 공백을 넣어주기
	for (int l = 0; l < 3; l++) {
		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				map[l][i][j] = BLANK;
				view_map[i][j] = BLANK;
				goto2xy(j, i);
				color(DARK_GRAY, BLACK);
				printf("·");
			}
		}
	}

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			view_map[i][j] = BLANK;
		}
	}

	// 배리어 그리기
	for (int i = 0; i < WIDTH; i++) {
		map[BARRIER_LAYER][0][i] = WALL;
	}

	for (int i = 0; i < HEIGHT; i++) {
		map[BARRIER_LAYER][i][0] = WALL;
	}

	for (int i = 0; i < HEIGHT; i++) {
		map[BARRIER_LAYER][i][WIDTH-1] = WALL;
	}

	for (int i = 0; i < WIDTH; i++) {
		map[BARRIER_LAYER][HEIGHT-1][i] = WALL;
	}

	// UI 그리기
	color(WHITE, BLACK);
	// 뒤에 스페이스를 넣어야 기호가 잘리지 않는다
	goto2xy(WIDTH, 0); printf("┌ ");
	goto2xy(WIDTH+19, 0); printf("┐ ");
	goto2xy(WIDTH, HEIGHT-1); printf("└ ");
	goto2xy(WIDTH+19, HEIGHT-1); printf("┘ ");
	for (int i = 1; i < HEIGHT-1; i++) {
		goto2xy(WIDTH, i);
		printf("│ ");
		goto2xy(WIDTH+19, i);
		printf("│ ");
	}
	goto2xy(WIDTH+1, 0);
	for (int i = 1; i < 19; i++) {
		printf("─ ");
	}
	goto2xy(WIDTH+1, HEIGHT-1);
	for (int i = 1; i < 19; i++) {
		printf("─ ");
	}

	randCreateBlock(APPLE, 2);

	while (1) {
		int snake_head[2] = {snake_body[0][0], snake_body[0][1]};
		curTime = clock(); // ms로 반환된다

		if (curTime - alertTime > 3000) { // 3초마다
			goto2xy(WIDTH + 2, HEIGHT - 3);
			color(BLACK, BLACK);
			for (int i = 0; i < 16; i++) {
				printf("  ");
			}
			alertTime = MAX;
		}

		if (isImmune && (curTime - immuneTime > 10000)) { // 10초가 넘으면
			isImmune = false;
			snake_speed -= 300;
			immuneTime = MAX;
		}

		if (isSmoke && (curTime - smokeTime > 20000)) {
			isSmoke = false;
			smokeTime = MAX;
			clearMap();
		}

		if (curTime - ranbowTime > 100) {
			ranbowTime = clock();
			switch (rainbow_index) {
				case 0 : rainbow_col = RED; break;
				case 1 : rainbow_col = YELLOW; break;
				case 2 : rainbow_col = GREEN; break;
				case 3 : rainbow_col = SKYBLUE; break;
				case 4 : rainbow_col = BLUE; break;
				case 5 : rainbow_col = PURPLE; break;
			}
			rainbow_index = rainbow_index >= 5 ? 0 : rainbow_index + 1;
		}

		if (_kbhit()) {
			keyCode = _getch();
			//if (keyCode == -32) {
			//	keyCode = _getch();
			//	snake_dir = keyCode; // 어차피 키코드의 UP, DOWN... 등과 공유하기 때문에 상관없다
			//}

			if (keyCode == UP || keyCode == RIGHT || keyCode == DOWN || keyCode == LEFT) {
				// 역방향 방지
				if (!((snake_dir == UP && keyCode == DOWN)
					 || (snake_dir == DOWN && keyCode == UP)
					 || (snake_dir == RIGHT && keyCode == LEFT)
					 || (snake_dir == LEFT && keyCode == RIGHT))) {
					key_dir = keyCode;
				}
			}
		}

		if (curTime - moveTime > 50000 / snake_speed) { // snake_speed가 높아질 수록 빨리 돈다
			moveTime = clock();

			switch (key_dir) {
				case UP:		addBody(snake_head[0], snake_head[1]-1, 0); break;
				case RIGHT: addBody(snake_head[0]+1, snake_head[1], 0); break;
				case DOWN:	addBody(snake_head[0], snake_head[1]+1, 0); break;
				case LEFT:	addBody(snake_head[0]-1, snake_head[1], 0); break;
			}

			snake_dir = key_dir;

			// 가장 오래된 몸통을 삭제한다
			//removeBody(snake_len - 1);
			snake_len--;

			score += snake_speed/25;
			PlaySound(TEXT("move.wav"), 0, SND_FILENAME | SND_ASYNC);
		}

		// 머리와 하트부분이 겹쳤을 때 길이가 증가한다
		if (map[ITEM_LAYER][snake_head[1]][snake_head[0]] != BLANK) {
			switch (map[ITEM_LAYER][snake_head[1]][snake_head[0]]) {
				case APPLE : 
					// 길이 증가 및 속도 증가
					snake_speed += 18;
					addBody(snake_body[snake_len][0], snake_body[snake_len][1], snake_len);
					score += snake_speed * 2;
					// 다른 곳에 하트 놓기
					randCreateBlock(APPLE, 2);
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC );
					alert("사과: 길이 1cm 증가");
					break;

				case FAKE_APPLE : 
					// 길이 증가 및 속도 증가
					snake_speed += 18;
					addBody(snake_body[snake_len][0], snake_body[snake_len][1], snake_len);
					score += snake_speed * 2;
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC );
					alert("가짜사과: 재생성X");
					break;

				case BANANA:
					addBody(snake_body[snake_len][0], snake_body[snake_len][1], snake_len);
					addBody(snake_body[snake_len-1][0], snake_body[snake_len-1][1], snake_len);
					snake_speed += 36;
					score += (snake_len * 20 + 100)*2;
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
					alert("바나나: 길이 2cm 증가");
					break;

				case SODA :
					snake_speed *= 1.25; // 5mm/s
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
					alert("소다: 속도 25% 증가");
					break;

				case MATCHA :
					snake_speed /= 2; // 5mm/s
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
					alert("녹차: 속도 50% 감소");
					break;

				case SMOKE :
					isSmoke = true;
					smokeTime = clock();
					clearMap();
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
					alert("연막: 20초간 흐려짐");
					break;

				case RAINBOW_STAR :
					isImmune = true;
					snake_speed += 300;
					immuneTime = clock();
					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
					alert("무지개별: 10초간 무적");
					break;
					
				case VITAMIN: {
						int temp_snake_len = snake_len; // 마지막 부분을 여러개 복사해야한다
						for(int i = 0; i < 4; i++)
							addBody(snake_body[temp_snake_len][0], snake_body[temp_snake_len][1], snake_len);
						PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
						alert("비타민: 길이만 4 증가");
						break;
					}

				case RANDOM_BOX: {
					char randItem = ITEM_LIST[rand()%ITEM_LIST_LENGTH];
					switch (randItem) {
						case WALL_MAKER: 
							for (int i = 0; i < 4; i++) {
								randCreateBlock(WALL, 0);
							}
							alert("벽생성기: 벽 4개 생성");
							break;

						case APPLE_SPREAD:
							for (int i = 0; i < 3; i++) {
								randCreateBlock(FAKE_APPLE, 2);
							}
							alert("사과폭탄: 가짜사과 3개 생성");
							break;

						default:
							randCreateBlock(randItem, 2);
							alert("랜덤박스");
					}

					PlaySound(TEXT("apple.wav"), 0, SND_FILENAME | SND_ASYNC);
					break;
				}
			}
			// 해당 위치에 아이템 제거
			map[ITEM_LAYER][snake_head[1]][snake_head[0]] = BLANK;
		}

		if (eventAmount <= snake_len/10) { // 길이가 10의 배수일 때마다 랜덤박스 소환
			randCreateBlock(RANDOM_BOX, 2);
			eventAmount++;
			alert("랜덤박스가 생성되었습니다");
		}

		for (int i = 0; i < HEIGHT; i++) {
			for (int j = 0; j < WIDTH; j++) {
				map[SNAKE_LAYER][i][j] = BLANK;
			}
		}

		// 머리와 블럭부분이 겹쳤을 때 종료한다
		if (map[BARRIER_LAYER][snake_head[1]][snake_head[0]] == WALL) {
			// 만약 무적일 경우에는 종료대신 벽에 막힌다
			if (isImmune) {
				removeBody(0); // 충돌된 머리를 제거한다
				snake_len++; // 따로 특정 좌표를 추가해주는 대신 마지막으로 사라진 몸통을 사용하기 위함
			}
			else {
				isGameOver = true;
			}
		}

		// 스네이크 머리 부분을 먼저
		// 항상 map[1][y][x] 형식임에 유의
		map[SNAKE_LAYER][snake_body[0][1]][snake_body[0][0]] = HEAD;
		// 스네이크의 몸통 위치가 담긴 배열을 이용
		for (int i = 1; i < snake_len; i++) {
			map[SNAKE_LAYER][snake_body[i][1]][snake_body[i][0]] = BODY;
			// 스네이크의 머리가 몸통과 부딪혔을 때 종료한다. (어차피 머리는 제외하고 돌리는 반복문이므로 머리인 경우를 따로 제외하진 않는다)
			if (snake_body[0][0] == snake_body[i][0] && snake_body[0][1] == snake_body[i][1]) {
				if (!isImmune) {
					isGameOver = true;
				}
			}
		}

		for (int l = 2; l >= 0; l--) { // 낮은 레이어일 수록 우선 순위가 높음
			for (int i = 0; i < HEIGHT; i++) {
				for (int j = 0; j < WIDTH; j++) {
					// 만약 미리 계산한 맵과 현재 맵의 특정 좌표 블럭이 다르다면
					if ((view_map[i][j] != map[l][i][j]) || (map[l][i][j] == RAINBOW_STAR) || (map[l][i][j] == HEAD) || (map[l][i][j] == BODY)) {
						int snake_col = 0; // 0인 경우엔 원래 색으로 된다
						if (isImmune) {
							snake_col = rainbow_col;
						}
						goto2xy(j, i);
						// 해당 좌표에 있는 정수를 블럭으로 변경
						switch (map[l][i][j]) {
							// clearMap을 호출할 때 비어있는 부분도 다시 색칠할 수 있게 하기 위해서 넣었다
							case 99: map[l][i][j] = BLANK; view_map[i][j] = map[l][i][j];
							// BLANK인 경우엔 다른 곳도 다 비어있을 때만 적용 (다 더했을 때 0일 것임)
							case BLANK: if(map[BARRIER_LAYER][i][j] + map[SNAKE_LAYER][i][j] + map[ITEM_LAYER][i][j] == 0) {
								color(DARK_GRAY, isSmoke ? GRAY : BLACK);
								printf("·");
								view_map[i][j] = map[l][i][j];
							} break;
							case WALL: color(isSmoke ? WHITE : DARK_BLUE, isSmoke ? WHITE : BLACK); printf("▧"); view_map[i][j] = map[l][i][j]; break;
							case HEAD:
								color(isSmoke ? WHITE : (snake_col == 0 ? GREEN : snake_col), isSmoke ? WHITE : BLACK);
								printf("▣"); view_map[i][j] = map[l][i][j]; break;
							case BODY:
								color(isSmoke ? WHITE : (snake_col == 0 ? DARK_GREEN : snake_col), isSmoke ? WHITE : BLACK);
								printf("▣"); view_map[i][j] = map[l][i][j]; break;
							case APPLE:	color(isSmoke ? WHITE : RED, isSmoke ? WHITE : BLACK); printf("♥"); view_map[i][j] = map[l][i][j]; break;
							case BANANA:	color(isSmoke ? WHITE : YELLOW, isSmoke ? WHITE : BLACK); printf("♥"); view_map[i][j] = map[l][i][j]; break;
							case SODA:	color(isSmoke ? WHITE : SKYBLUE, isSmoke ? WHITE : BLACK); printf("▥"); view_map[i][j] = map[l][i][j]; break;
							case MATCHA:	color(isSmoke ? WHITE : DARK_GREEN, isSmoke ? WHITE : BLACK); printf("▤"); view_map[i][j] = map[l][i][j]; break;
							case RAINBOW_STAR:	color(isSmoke ? WHITE : rainbow_col, isSmoke ? WHITE : BLACK); printf("★"); view_map[i][j] = map[l][i][j]; break;
							case VITAMIN:	color(isSmoke ? WHITE : DARK_YELLOW, isSmoke ? WHITE : BLACK); printf("▥"); view_map[i][j] = map[l][i][j]; break;
							case RANDOM_BOX:	color(isSmoke ? WHITE : DARK_YELLOW, isSmoke ? WHITE : BLACK); printf("▩"); view_map[i][j] = map[l][i][j]; break;
							case FAKE_APPLE:	color(isSmoke ? WHITE : RED, isSmoke ? WHITE : BLACK); printf("♡"); view_map[i][j] = map[l][i][j]; break;
							case SMOKE:	color(DARK_GRAY, isSmoke ? WHITE : BLACK); printf("※"); view_map[i][j] = map[l][i][j]; break;
						}
					}
				}
			}
		}

		// status
		goto2xy(WIDTH + 2, 2);
		color(BLACK, WHITE);
		printf("   SCORE   "); // 11칸
		gotoxy(WIDTH * 2 + 16, 2);
		color(WHITE, BLACK);
		printf("%d        ", score);
		
		goto2xy(WIDTH + 2, 4);
		color(BLACK, WHITE);
		printf(" HIGHSCORE ");
		gotoxy(WIDTH * 2 + 16, 4);
		color(WHITE, BLACK);
		printf("%d        ", high_score);

		goto2xy(WIDTH + 2, 6);
		color(BLACK, WHITE);
		printf("    LEN    ");
		gotoxy(WIDTH * 2 + 16, 6);
		color(WHITE, BLACK);
		printf("%dcm      ", snake_len);

		goto2xy(WIDTH + 2, 8);
		color(BLACK, WHITE);
		printf("   SPEED   ");
		gotoxy(WIDTH * 2 + 16, 8);
		color(WHITE, BLACK);
		printf("%dmm/s    ", snake_speed/5);

		if(isGameOver || keyCode == ESC) break;
	}

	gameOver();
}

void viewRanking(void) {
	FILE* fp = NULL;
	int scores[5] = {0};
	int scores_len = sizeof(scores) / sizeof(int);
	fopen_s(&fp, "ranking.txt", "r");
	for (int i = 0; i < scores_len; i++) {
		fscanf_s(fp, "%d", &scores[i]);
	}
	fclose(fp);

	system("cls");
	color(DARK_BLUE, BLACK);
	drawFrame();

	color(WHITE, BLACK);
	gotoxy(WIDTH - 9, HEIGHT / 2 - 6);
	printf("< R A N K I N G >");

	gotoxy(WIDTH - 5, HEIGHT / 2 - 2);
	color(YELLOW, BLACK);
	printf("1. %06d", scores[0]);
	gotoxy(WIDTH - 5, HEIGHT / 2 + 0);
	color(GRAY, BLACK);
	printf("2. %06d", scores[1]);
	gotoxy(WIDTH - 5, HEIGHT / 2 + 2);
	color(DARK_YELLOW, BLACK);
	printf("3. %06d", scores[2]);
	gotoxy(WIDTH - 5, HEIGHT / 2 + 4);
	color(WHITE, BLACK);
	printf("4. %06d", scores[3]);
	gotoxy(WIDTH - 5, HEIGHT / 2 + 6);
	printf("5. %06d", scores[4]);

	while (1) {
		if (_kbhit()) {
			keyCode = _getch();
			if (keyCode == ESC) break;
		}
	}

	viewMain();
}

void viewMain(void) {
	static int sel = 0;
	PlaySound(TEXT("move.wav"), 0, SND_FILENAME | SND_ASYNC);

	sprintf_s(systemText, sizeof(systemText), "mode con cols=%d lines=%d", WIDTH * 2, HEIGHT);
	system(systemText);

	system("cls");
	// 메인 화면 그리기
	gotoxy(WIDTH - 10, HEIGHT / 2 - 4);
	color(WHITE, BLACK);
	printf("S N A K E   G A M E");

	gotoxy(WIDTH - 4, HEIGHT / 2 + 1);
	color(sel == 0 ? WHITE : DARK_GRAY, BLACK);
	printf("<START>");

	gotoxy(WIDTH - 5, HEIGHT / 2 + 3);
	color(sel == 1 ? WHITE : DARK_GRAY, BLACK);
	printf("<RANKING>");

	gotoxy(WIDTH - 6, HEIGHT / 2 + 5);
	color(sel == 2 ? WHITE : DARK_GRAY, BLACK);
	printf("<QUIT GAME>");

	color(DARK_BLUE, BLACK);
	drawFrame();

	while (1) {
		if (_kbhit()) {
			PlaySound(TEXT("move.wav"), 0, SND_FILENAME | SND_ASYNC);
			keyCode = _getch();
			// 방향키는 다른 값과의 혼동을 방지하기 위해 -32와 해당 입력키 코드를 동시에 반환한다
			if (keyCode == -32) {
				keyCode = _getch();
				if (keyCode == DOWN) sel = sel != 2 ? sel + 1 : sel;
				else if (keyCode == UP) sel = sel != 0 ? sel - 1 : sel;

				gotoxy(WIDTH - 4, HEIGHT / 2 + 1);
				color(sel == 0 ? WHITE : DARK_GRAY, BLACK);
				printf("<START>");

				gotoxy(WIDTH - 5, HEIGHT / 2 + 3);
				color(sel == 1 ? WHITE : DARK_GRAY, BLACK);
				printf("<RANKING>");

				gotoxy(WIDTH - 6, HEIGHT / 2 + 5);
				color(sel == 2 ? WHITE : DARK_GRAY, BLACK);
				printf("<QUIT GAME>");
			}
			if (keyCode == ESC) exit(0);
			if (keyCode == ENTER || keyCode == SPACE) break;
		}
	}

	switch (sel) {
		case 0: gameLoop(); break;
		case 1: viewRanking(); break;
		case 2: exit(0);
	}
}

void init(void) {
	FILE *fp = NULL;
	fopen_s(&fp, "ranking.txt", "r");
	if (fp == NULL) {
		fopen_s(&fp, "ranking.txt", "w");
		fprintf_s(fp, "0\n0\n0\n0\n0");
	}
	fclose(fp);

	// random 함수가 계속 달라지게 해준다
	srand((unsigned)time(NULL));

	SetConsoleTitle(TEXT("SNAKE GAME"));

	// 커서 숨김
	CONSOLE_CURSOR_INFO cursorInfo = { 0, };
	cursorInfo.bVisible = 0;
	cursorInfo.dwSize = 1;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

int main(void) {
	init();

	viewMain();

	return 0;
}
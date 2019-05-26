/*
 ============================================================================
 Name        : 2048.c
 Author      : Maurits van der Schee
 Description : Console version of the game "2048" for GNU/Linux
 ============================================================================
 */

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

#define SIZE 4
uint32_t score = 0;
uint8_t scheme = 0;

/**
 * @brief 실행 인자값에 따른 실행모드 상수입니다
 */
#define EXECUTE_GAME_MODE            -1
#define EXECUTE_TEST_MODE            0
#define EXECUTE_COLOR_BLACKWHITE     1
#define EXECUTE_COLOR_BLUERED        2



/**
 * @author                          박소연 (pparksso0308@gmail.com)
 * @brief                           화면에 출력될 블록들의 색깔 스키마를 설정한다.
 * @param uint8_t value             블록들에 저장되어 있는 값
 * @param char * color              블록들의 색깔을 저장할 변수
 * @param size_t length             color 변수의 크기 설정
*/
void getColor(uint8_t value, char *color, size_t length) {

    uint8_t original[] = {8, 255, 1, 255, 2, 255, 3, 255, 4, 255, 5, 255, 6, 255, 7,
                          255, 9, 0, 10, 0, 11, 0, 12, 0, 13, 0, 14, 0, 255, 0, 255, 0};
    uint8_t blackwhite[] = {232, 255, 234, 255, 236, 255, 238, 255, 240, 255, 242, 255, 244, 255,
                            246, 0, 248, 0, 249, 0, 250, 0, 251, 0, 252, 0, 253, 0, 254, 0, 255, 0};
    uint8_t bluered[] = {235, 255, 63, 255, 57, 255, 93, 255, 129, 255, 165, 255, 201, 255, 200, 255,
                        199, 255, 198, 255, 197, 255, 196, 255, 196, 255, 196, 255, 196, 255, 196, 255};

    uint8_t *schemes[] = {original, blackwhite, bluered};
    uint8_t *background = schemes[scheme] + 0;
    uint8_t *foreground = schemes[scheme] + 1;

    if (value > 0)  {
        while (value--) {
            if (background + 2 < schemes[scheme] + sizeof(original)) {
                background += 2;
                foreground += 2;
            }
        }
    }

    snprintf(color, length, "\033[38;5;%d;48;5;%dm", *foreground, *background);
}



/**
 * @author                      박소연 (pparksso0308@gmail.com)
 * @brief                       화면에 블록의 값을 출력한다.
 * @param uint8_t board         화면에 출력할 게임판 정보
 * @param unit8_t x_index       게임판의 x_index 값
 * @param unit8_t y_index       게임판의 y_index 값
 */
void printValue(uint8_t board[SIZE][SIZE], uint8_t x_index, uint8_t y_index )
{
    if (board[x][y] != 0) {
        char s[8];
        snprintf(s, 8, "%u", (uint32_t) 1 << board[x][y]);

        uint8_t t = 7 - strlen(s);
        printf("%*s%s%*s", t - t / 2, "", s, t / 2, "");
    } else {
        printf("   ·   ");
    }
}

/**
 * @author                      박소연 (pparksso0308@gmail.com)
 * @brief                       화면에 게임판을 출력한다.
 * @param uint8_t board         화면에 출력할 게임판 정보
 */
void drawBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t x; 
    uint8_t y;
    char color[40];
    char reset[] = "\033[m";

    printf("\033[H");
    printf("2048.c %17d pts\n\n", score);

    for (y = 0; y < SIZE; y++) {
        for (x = 0; x < SIZE; x++) {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            printf("       ");
            printf("%s", reset);
        }
        printf("\n");

        for (x = 0; x < SIZE; x++) {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            printValue(board, int x, int y);
            printf("%s", reset);
        }
        printf("\n");
        for (x = 0; x < SIZE; x++) {
            getColor(board[x][y], color, 40);
            printf("%s", color);
            printf("       ");
            printf("%s", reset);
        }
        printf("\n");
    }
    printf("\n");
    printf("        ←,↑,→,↓ or q        \n");
    printf("\033[A"); // one line up
}

/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           블럭이 이동할 수 있는 위치를 찾아 반환하는 함수
 * @param unsigned int array[SIZE]  검사할 블럭이 속한 행
 * @param unsigned int x            검사할 블럭의 위치 정보
 * @param unsigned int stop         중복검사를 방지하기 위한 인덱스
 * @return unsigned int             블럭이 이동할 수 있는 위치를 찾으면 해당 위치를 반환 (t+1),
 *                                  블럭이 stop 에 의해 더이상 검사를 할 필요가 없는 경우 (t),
 *                                  블럭이 이동할 수 있는 위치가 없는 경우 원래 위치 반환(제자리) (x)
 */
uint8_t findTarget(uint8_t array[SIZE], uint8_t x, uint8_t stop) {
    uint8_t t;
    // if the position is already on the first, don't evaluate
    if (x == 0) {
        return x;
    }
    for (t = x - 1;; t--) {
        if (array[t] != 0) {
            if (array[t] != array[x]) {
                // merge is not possible, take next position
                return t + 1;
            }
            return t;
        } else {
            // we should not slide further, return this one
            if (t == stop) {
                return t;
            }
        }
    }
    // we did not find a
    return x;
}

/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           게임판의 블럭들을 이동하는 함수       
 *                                  이동 중 블럭끼리 merge되는 경우도 발생함
 * @param unsigned int board        게임판
 * @param unsigned int index        검사할 행의 인덱스
 * @return bool success             게임판의 한 행의 블럭들이 이동되었는지 여부
 *                                  하나의 블럭이라도 이동했다면 true 반환한다.             
 */
bool slideArray(uint8_t board[SIZE][SIZE], uint8_t index) {
    bool success = false;
    uint8_t x, t, stop = 0;

    for (x = 0; x < SIZE; x++) {
        if (board[index][x] != 0) {
            t = findTarget(board[index], x, stop);
            // if target is not original position, then move or merge
            if (t != x) {
                // if target is zero, this is a move
                if (board[index][t] == 0) {
                    board[index][t] = board[index][x];
                } 
                else if (board[index][t] == board[index][x]) {
                    // merge (increase power of two)
                    board[index][t]++;
                    // increase score
                    score += (uint32_t) 1 << board[index][t];
                    // set stop to avoid double merge
                    stop = t + 1;
                }
                else {
                    
                }
                board[index][x] = 0;
                success = true;
            }
        }
    }
    return success;
}

/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           게임판을 반시계 방향으로 90도 회전시키는 함수
 *                                  실제 구현은 4x4 배열의 원소들을 옮겨서 회전과 같은 효과를 냈다.
 * @param unsigned int board        게임판        
 */
void rotateBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t i, j, n = SIZE;
    uint8_t tmp;
    for (i = 0; i < n / 2; i++) {
        for (j = i; j < n - i - 1; j++) {
            tmp = board[i][j];
            board[i][j] = board[j][n - i - 1];
            board[j][n - i - 1] = board[n - i - 1][n - j - 1];
            board[n - i - 1][n - j - 1] = board[n - j - 1][i];
            board[n - j - 1][i] = tmp;
        }
    }
}
/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           게임판의 블럭들을 위로 이동하는 함수
 *                                  모든 move 함수들은 rotateBoard를 수행해서 방향을 맞추고,
 *                                  moveUp 함수를 호출해서 move작업을 수행한다.
 * @remark                          4x4 배열을 4x1 씩 나누어 4번의 slideArray 함수를 호출한다.
 * @param unsigned int board        게임판       
 * @return bool success             작업의 성공 여부
 */
bool moveUp(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    uint8_t x;
    for (x = 0; x < SIZE; x++) {
        success |= slideArray(board, x);
    }
    return success;
}

/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           게임판의 블럭들을 왼쪽으로 이동하는 함수
 *                                  반 시계 벙향으로 90도 게임판을 회전시키고,
 *                                  moveUp 이후에 반 시계 방향으로 270도 게임판을 회전시킨다.
 * @param unsigned int board        게임판       
 * @return bool success             작업의 성공 여부
 */
bool moveLeft(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           게임판의 블럭들을 왼쪽으로 이동하는 함수
 *                                  반 시계 벙향으로 180도 게임판을 회전시키고,
 *                                  moveUp 이후에 반 시계 방향으로 180도 게임판을 회전시킨다.
 * @param unsigned int board        게임판       
 * @return bool success             작업의 성공 여부
 */
bool moveDown(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    rotateBoard(board);
    return success;
}

/**
 * @author                          이원준 (21jun7654@gmail.com)
 * @brief                           게임판의 블럭들을 왼쪽으로 이동하는 함수
 *                                  반 시계 벙향으로 270도 게임판을 회전시키고,
 *                                  moveUp 이후에 반 시계 방향으로 90도 게임판을 회전시킨다.
 * @param unsigned int board        게임판       
 * @return bool success             작업의 성공 여부
 */
bool moveRight(uint8_t board[SIZE][SIZE]) {
    bool success;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    success = moveUp(board);
    rotateBoard(board);
    return success;
}

bool findPairDown(uint8_t board[SIZE][SIZE]) {
    bool success = false;
    uint8_t x, y;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE - 1; y++) {
            if (board[x][y] == board[x][y + 1]) return true;
        }
    }
    return success;
}

uint8_t countEmpty(uint8_t board[SIZE][SIZE]) {
    uint8_t x, y;
    uint8_t count = 0;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            if (board[x][y] == 0) {
                count++;
            }
        }
    }
    return count;
}

bool gameEnded(uint8_t board[SIZE][SIZE]) {
    bool ended = true;
    if (countEmpty(board) > 0) return false;
    if (findPairDown(board)) return false;
    rotateBoard(board);
    if (findPairDown(board)) ended = false;
    rotateBoard(board);
    rotateBoard(board);
    rotateBoard(board);
    return ended;
}
/**
 * @author                      박소연 (pparksso0308@gmail.com)
 * @brief                       게임 시작시 모든 블록의 값을 각각 랜덤하게 설정
 * @param uint8_t board         화면에 출력할 게임판 정보
 */
void addRandom(uint8_t board[SIZE][SIZE]) {
    static bool initialized = false;
    uint8_t x;
    uint8_t y;
    uint8_t r;
    uint8_t len = 0;
    uint8_t n;
    uint8_t list[SIZE * SIZE][2];

    if (!initialized) {
        srand(time(NULL));
        initialized = true; 
    }

    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            if (board[x][y] == 0) { 
                list[len][0] = x;
                list[len][1] = y;
                len++;
            }
        }
    }

    if (len > 0) {
        r = rand() % len;
        x = list[r][0];
        y = list[r][1];
        n = (rand() % 10) / 9 + 1;
        board[x][y] = n;
    }
}

// @todo 함수 설명 필요
void initBoard(uint8_t board[SIZE][SIZE]) {
    uint8_t x, y;
    for (x = 0; x < SIZE; x++) {
        for (y = 0; y < SIZE; y++) {
            board[x][y] = 0;
        }
    }
    addRandom(board);
    addRandom(board);
    drawBoard(board);
    score = 0;
}

void setBufferedInput(bool enable) {
    static bool enabled = true;
    static struct termios old;
    struct termios new;

    if (enable && !enabled) {
        // restore the former settings
        tcsetattr(STDIN_FILENO, TCSANOW, &old);
        // set the new state
        enabled = true;
    } else if (!enable && enabled) {
        // get the terminal settings for standard input
        tcgetattr(STDIN_FILENO, &new);
        // we want to keep the old setting to restore them at the end
        old = new;
        // disable canonical mode (buffered i/o) and local echo
        new.c_lflag &= (~ICANON & ~ECHO);
        // set the new settings immediately
        tcsetattr(STDIN_FILENO, TCSANOW, &new);
        // set the new state
        enabled = false;
    }
}

// @todo 함수 설명 필요
// @todo 함수 분할 필요
// @todo 주석 한글화 필요
int test() {
    uint8_t array[SIZE];
    uint8_t board[SIZE][SIZE];
    // these are exponents with base 2 (1=2 2=4 3=8)
    uint8_t data[] = {
            0, 0, 0, 1, 1, 0, 0, 0,
            0, 0, 1, 1, 2, 0, 0, 0,
            0, 1, 0, 1, 2, 0, 0, 0,
            1, 0, 0, 1, 2, 0, 0, 0,
            1, 0, 1, 0, 2, 0, 0, 0,
            1, 1, 1, 0, 2, 1, 0, 0,
            1, 0, 1, 1, 2, 1, 0, 0,
            1, 1, 0, 1, 2, 1, 0, 0,
            1, 1, 1, 1, 2, 2, 0, 0,
            2, 2, 1, 1, 3, 2, 0, 0,
            1, 1, 2, 2, 2, 3, 0, 0,
            3, 0, 1, 1, 3, 2, 0, 0,
            2, 0, 1, 1, 2, 2, 0, 0
    };
    uint8_t *in, *out;
    uint8_t t, tests;
    uint8_t i;
    bool success = true;

    tests = (sizeof(data) / sizeof(data[0])) / (2 * SIZE);
    for (t = 0; t < tests; t++) {
        in = data + t * 2 * SIZE;
        out = in + SIZE;
        for (i = 0; i < SIZE; i++) {
            array[i] = in[i];
            board[0][i] = in[i];
        }
        slideArray(board, 0);
        for (i = 0; i < SIZE; i++) {
            if (board[0][i] != out[i]) {
                success = false;
            }
        }
        if (success == false) {
            for (i = 0; i < SIZE; i++) {
                printf("%d ", in[i]);
            }
            printf("=> ");
            for (i = 0; i < SIZE; i++) {
                printf("%d ", array[i]);
            }
            printf("expected ");
            for (i = 0; i < SIZE; i++) {
                printf("%d ", in[i]);
            }
            printf("=> ");
            for (i = 0; i < SIZE; i++) {
                printf("%d ", out[i]);
            }
            printf("\n");
            break;
        }
    }
    if (success) {
        printf("All %u tests executed successfully\n", tests);
    }
    return !success;
}

void signal_callback_handler(int signum) {
    printf("         TERMINATED         \n");
    setBufferedInput(true);
    printf("\033[?25h\033[m");
    exit(signum);
}



/**
 * @author                      조유신 (cho8wola@sju.ac.kr)
 * @brief                       실행 파라미터에 따른 처리를 해주는 함수
 * @param int $argc             명령햅 옵션의 개수
 * @param int $argv             명령행 옵션의 문자열
 * @return EXCUTE_TEST_MODE     TEST모드로 실행되었을 경우만 리턴
 */
int getExecuteMode(int argc, char *argv[])
{
    if (argc == 2) {
        if ( strcmp(argv[1], "test") == 0 ) {
            printf("hello");
            return EXECUTE_TEST_MODE;
        }
        if ( strcmp(argv[1], "blackwhite") == 0) {
            scheme = EXECUTE_COLOR_BLACKWHITE;
        }
        if ( strcmp(argv[1], "bluered") == 0) {
            scheme = EXECUTE_COLOR_BLUERED;
        }
    }

    return EXECUTE_GAME_MODE;
}

/**
 * @author                      조유신 (cho8wola@sju.ac.kr)
 * @brief                       키 입력 이벤트를 처리하는 함수
 * @param unit8_t board         게임의 현재 상황을 나타내는 게임보드
 */
void KeyInputProcess(uint8_t board[][SIZE])
{
    char c;
    bool success;

    
    while (true) {
        c = getchar();
        if (c == -1) { // @todo -1에 대한 설명 명명된 상수로 대체할 예정
            puts("\nError! Cannot read keyboard input!");
            break;
        }
        switch (c) {
            case 97:    // 'a' 키
            case 104:    // 'h' 키
            case 68:    // 왼쪽 화살표
                success = moveLeft(board);
                break;
            case 100:    // 'd' 키
            case 108:    // 'l' 키
            case 67:    // 오른쪽 화살표
                success = moveRight(board);
                break;
            case 119:    // 'w' 키
            case 107:    // 'k' 키
            case 65:    // 위쪽 화살표
                success = moveUp(board);
                break;
            case 115:    // 's' 키
            case 106:    // 'j' 키
            case 66:    // 아래쪽 화살표
                success = moveDown(board);
                break;
            default:
                success = false;
        }
        if (success) {
            drawBoard(board);
            usleep(150000);
            addRandom(board);
            drawBoard(board);
            if (gameEnded(board)) {
                printf("         GAME OVER          \n");
                break;
            }
        }
        if (c == 'q') {
            printf("        QUIT? (y/n)         \n");
            c = getchar();
            if (c == 'y') {
                break;
            }
            drawBoard(board);
        }
        if (c == 'r') {
            printf("       RESTART? (y/n)       \n");
            c = getchar();
            if (c == 'y') {
                initBoard(board);
            }
            drawBoard(board);
        }
    }
}

/**
 * @author                      조유신 (cho8wola@sju.ac.kr)
 * @brief                       main함수
 * @param int argc              실행 파라미터의 개수
 * @param int argv              실행 파라미터 값들의 배열
 * @return EXIT_SUCCESS         정상적으로 종료되었을 경우 리턴
 */
int main(int argc, char *argv[]) {
    uint8_t board[SIZE][SIZE];

    if (getExecuteMode(argc, argv) == EXECUTE_TEST_MODE) { return test(); }

    printf("\033[?25l\033[2J");

    // @brief 컨트롤 C에 대한 이벤트를 받을 핸들러 등록
    signal(SIGINT, signal_callback_handler);

    initBoard(board);
    setBufferedInput(false);

    // @brief 게임의 실제 진행이 이루어지는 부분
    KeyInputProcess(board);
    
    setBufferedInput(true);

    printf("\033[?25h\033[m");

    return EXIT_SUCCESS;
}
#include <iostream>
#include <conio.h>
#include <windows.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <clocale>
#include <limits>
#include <algorithm> // Добавлено для max и min

using namespace std;

// Прототипы функций
void mainMenu(HANDLE h);
void playGame(HANDLE h);
void chooseDifficulty();
void enableCheats();
void showRules();
void resetGame(int& playerX, int& playerY, int& coins, int& health, int maze[15][50]);
void initLevel(int level, int& playerX, int& playerY, int& exitX, int& exitY, int maze[15][50]);
void updateStats(HANDLE h);
void printMaze(HANDLE h);
void setColor(int color);
void clearConsole();
void markAttackZone(int bossX, int bossY, int attackZone[15][50]);
void clearAttackZone(int attackZone[15][50]);

// Перечисления
enum MazeObject { HALL, WALL, COIN, ENEMY, BORDER, EXIT, BOSS, HEALTH_PICKUP, BACK_EXIT };
enum Color { DARKGREEN = 2, YELLOW = 14, RED = 12, BLUE = 9, WHITE = 15, DARKYELLOW = 6, DARKRED = 4, MAGENTA = 13, GREEN = 10 };
enum KeyCode { ENTER = 13, ESCAPE = 27, SPACE = 32, LEFT = 75, RIGHT = 77, UP = 72, DOWN = 80 };

// Константы
const int HEIGHT = 15;
const int WIDTH = 50;
const int TOTAL_LEVELS = 3;
const int COINS_FOR_BOSS = 20;
const int HEALTH_PICKUP_VALUE = 20;

// Структура для уровней сложности
struct Difficulty {
    int enemyCount;
    int enemyDamage;
    int healthPickups;
    int healthPickupsLevel3;
    int levelTransitionCost;
    int bossHealth;
    int bossDamage;
    int bossChargeDamage;
};

// Параметры сложности
Difficulty difficulties[] = {
    {5, 10, 0, 5, 5, 80, 30, 60},    // Мирно
    {10, 20, 0, 4, 10, 90, 40, 80},   // Легкий
    {15, 30, 5, 3, 15, 100, 50, 100}, // Средний
    {20, 40, 3, 2, 20, 120, 60, 120}, // Сложно
    {30, 50, 1, 1, 25, 150, 70, 140}  // Безумие
};

// Глобальные переменные
int currentDifficulty = 0;
bool cheatsEnabled = false;
int cheatMessageTimer = 0;
int currentLevel = 0;

int maze[HEIGHT][WIDTH] = { 0 };
int attackZone[HEIGHT][WIDTH] = { 0 };
int playerX = 2;
int playerY = 1;
int exitX = 12;
int exitY = 48;
int backExitX = 1;
int backExitY = 1;
int coins = 0;
int health = 100;
bool bossSpawned = false;
int bossX = -1;
int bossY = -1;
int bossHealth = 0;
int bossDamage = 0;
int bossMoveCounter = 0;
int bossChargeCounter = 0;
int bossWarningTimer = 0;
int bossAttackTimer = 0;
int coinWarningTimer = 0;
const int BOSS_CHARGE_MAX = 5;
const int BOSS_WARNING_DURATION = 4;
const int BOSS_ATTACK_DURATION = 1;
const int COIN_WARNING_DURATION = 3;

// Установка цвета
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Очистка консоли
void clearConsole() {
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    FillConsoleOutputAttribute(console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE, screen.dwSize.X * screen.dwSize.Y, topLeft, &written);
    SetConsoleCursorPosition(console, topLeft);
}

// Отметка зоны атаки босса
void markAttackZone(int bossX, int bossY, int attackZone[HEIGHT][WIDTH]) {
    for (int y = max(0, bossX - 2); y <= min(HEIGHT - 1, bossX + 2); y++) {
        for (int x = max(0, bossY - 2); x <= min(WIDTH - 1, bossY + 2); x++) {
            if (abs(y - bossX) + abs(x - bossY) <= 2 && maze[y][x] == MazeObject::HALL) {
                attackZone[y][x] = 1; // Безопасный доступ
            }
        }
    }
}

// Очистка зоны атаки
void clearAttackZone(int attackZone[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            attackZone[y][x] = 0;
        }
    }
}

// Отрисовка лабиринта
void printMaze(HANDLE h) {
    COORD topLeft = { 0, 0 };
    SetConsoleCursorPosition(h, topLeft);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (x == playerY && y == playerX) {
                setColor(Color::BLUE);
                cout << "@";
                continue;
            }
            if (attackZone[y][x] == 1) {
                setColor(Color::RED);
                cout << "*";
                continue;
            }
            switch (maze[y][x]) {
            case MazeObject::HALL: cout << " "; break;
            case MazeObject::WALL: setColor(Color::DARKGREEN); cout << "#"; break;
            case MazeObject::BORDER: setColor(Color::WHITE); cout << "#"; break;
            case MazeObject::COIN: setColor(Color::YELLOW); cout << "$"; break;
            case MazeObject::ENEMY: setColor(Color::RED); cout << "E"; break;
            case MazeObject::EXIT: setColor(Color::MAGENTA); cout << "X"; break;
            case MazeObject::BOSS: setColor(Color::MAGENTA); cout << "B"; break;
            case MazeObject::HEALTH_PICKUP: setColor(Color::GREEN); cout << "+"; break;
            case MazeObject::BACK_EXIT: setColor(Color::YELLOW); cout << "O"; break;
            }
        }
        if (y == 0) { setColor(Color::DARKYELLOW); cout << "  МОНЕТЫ: " << coins; }
        if (y == 1) { setColor(Color::DARKRED); cout << "  ЗДОРОВЬЕ: " << health; }
        if (y == 2) { setColor(Color::WHITE); cout << "  УРОВЕНЬ: " << (currentLevel + 1); }
        if (y == 3) { setColor(Color::BLUE); cout << "  Игрок: (" << playerX << ", " << playerY << ")  "; }
        if (y == 4) { setColor(Color::YELLOW); cout << "  Монет для перехода: " << difficulties[currentDifficulty].levelTransitionCost; }
        if (y == 5 && bossSpawned) { setColor(Color::DARKRED); cout << "  Здоровье босса: " << bossHealth; }
        if (y == 6 && bossSpawned) { setColor(Color::MAGENTA); cout << "  Зарядка босса: " << bossChargeCounter << "/" << BOSS_CHARGE_MAX; }
        if (y == 7 && bossWarningTimer > 0) { setColor(Color::RED); cout << "  Босс заряжается! Убегайте!"; bossWarningTimer--; }
        if (y == 8 && bossAttackTimer > 0) { setColor(Color::RED); cout << "  Босс атакует!"; bossAttackTimer--; }
        if (y == 9 && coinWarningTimer > 0) {
            setColor(Color::YELLOW);
            if (currentLevel == 1) cout << "  Недостаточно монет для босса!";
            else cout << "  Недостаточно монет!";
            coinWarningTimer--;
        }
        if (y == 10 && cheatsEnabled && cheatMessageTimer > 0) { setColor(Color::YELLOW); cout << "  Ах ты, читер! Даже враги в шоке! :D"; cheatMessageTimer--; }
        cout << endl;
    }
    setColor(Color::WHITE);
}

// Обновление статистики
void updateStats(HANDLE h) {
    COORD statsPos = { static_cast<SHORT>(WIDTH + 2), 0 };
    SetConsoleCursorPosition(h, statsPos);
    setColor(Color::DARKYELLOW); cout << "МОНЕТЫ: " << coins << "   ";
    statsPos.Y = 1; SetConsoleCursorPosition(h, statsPos);
    setColor(Color::DARKRED); cout << "ЗДОРОВЬЕ: " << health << "   ";
    statsPos.Y = 2; SetConsoleCursorPosition(h, statsPos);
    setColor(Color::WHITE); cout << "УРОВЕНЬ: " << (currentLevel + 1) << "   ";
    statsPos.Y = 3; SetConsoleCursorPosition(h, statsPos);
    setColor(Color::BLUE); cout << "Игрок: (" << playerX << ", " << playerY << ")  ";
    statsPos.Y = 4; SetConsoleCursorPosition(h, statsPos);
    setColor(Color::YELLOW); cout << "Монет для перехода: " << difficulties[currentDifficulty].levelTransitionCost << "   ";
    if (bossSpawned) {
        statsPos.Y = 5; SetConsoleCursorPosition(h, statsPos);
        setColor(Color::DARKRED); cout << "Здоровье босса: " << bossHealth << "   ";
        statsPos.Y = 6; SetConsoleCursorPosition(h, statsPos);
        setColor(Color::MAGENTA); cout << "Зарядка босса: " << bossChargeCounter << "/" << BOSS_CHARGE_MAX << "   ";
        if (bossWarningTimer > 0) {
            statsPos.Y = 7; SetConsoleCursorPosition(h, statsPos);
            setColor(Color::RED); cout << "Босс заряжается! Убегайте!   ";
        }
        else { statsPos.Y = 7; SetConsoleCursorPosition(h, statsPos); cout << "                              "; }
        if (bossAttackTimer > 0) {
            statsPos.Y = 8; SetConsoleCursorPosition(h, statsPos);
            setColor(Color::RED); cout << "Босс атакует!   ";
        }
        else { statsPos.Y = 8; SetConsoleCursorPosition(h, statsPos); cout << "                 "; }
    }
    if (coinWarningTimer > 0) {
        statsPos.Y = 9; SetConsoleCursorPosition(h, statsPos);
        setColor(Color::YELLOW);
        if (currentLevel == 1) cout << "Недостаточно монет для босса!   ";
        else cout << "Недостаточно монет!   ";
    }
    else { statsPos.Y = 9; SetConsoleCursorPosition(h, statsPos); cout << "                                 "; }
    if (cheatsEnabled && cheatMessageTimer > 0) {
        statsPos.Y = 10; SetConsoleCursorPosition(h, statsPos);
        setColor(Color::YELLOW); cout << "Ах ты, читер! Даже враги в шоке! :D   ";
    }
    else { statsPos.Y = 10; SetConsoleCursorPosition(h, statsPos); cout << "                                     "; }
    setColor(Color::WHITE);
}

// Инициализация уровня
void initLevel(int level, int& playerX, int& playerY, int& exitX, int& exitY, int maze[HEIGHT][WIDTH]) {
    for (int y = 0; y < HEIGHT; y++) for (int x = 0; x < WIDTH; x++) maze[y][x] = MazeObject::HALL;
    for (int x = 0; x < WIDTH; x++) { maze[0][x] = MazeObject::BORDER; maze[HEIGHT - 1][x] = MazeObject::BORDER; }
    for (int y = 0; y < HEIGHT; y++) { maze[y][0] = MazeObject::BORDER; maze[y][WIDTH - 1] = MazeObject::BORDER; }
    playerX = 2; playerY = 1;
    if (level > 0) { backExitX = 1; backExitY = 1; maze[backExitX][backExitY] = MazeObject::BACK_EXIT; }

    switch (level) {
    case 0:
        for (int x = 5; x <= 7; x++) { maze[2][x] = MazeObject::WALL; maze[6][x] = MazeObject::WALL; maze[10][x] = MazeObject::WALL; }
        for (int x = 15; x <= 17; x++) { maze[2][x] = MazeObject::WALL; maze[6][x] = MazeObject::WALL; maze[10][x] = MazeObject::WALL; }
        for (int x = 25; x <= 27; x++) { maze[2][x] = MazeObject::WALL; maze[6][x] = MazeObject::WALL; maze[10][x] = MazeObject::WALL; }
        for (int x = 35; x <= 37; x++) { maze[2][x] = MazeObject::WALL; maze[6][x] = MazeObject::WALL; maze[10][x] = MazeObject::WALL; }
        for (int x = 45; x <= 47; x++) { maze[2][x] = MazeObject::WALL; maze[6][x] = MazeObject::WALL; maze[10][x] = MazeObject::WALL; }
        maze[1][4] = MazeObject::COIN; maze[1][10] = MazeObject::COIN; maze[1][15] = MazeObject::COIN; maze[1][20] = MazeObject::COIN;
        maze[1][25] = MazeObject::COIN; maze[1][30] = MazeObject::COIN; maze[1][35] = MazeObject::COIN; maze[1][40] = MazeObject::COIN;
        maze[1][45] = MazeObject::COIN; maze[3][5] = MazeObject::COIN; maze[3][15] = MazeObject::COIN; maze[3][25] = MazeObject::COIN;
        maze[5][10] = MazeObject::COIN; maze[5][20] = MazeObject::COIN; maze[5][30] = MazeObject::COIN; maze[7][5] = MazeObject::COIN;
        maze[7][15] = MazeObject::COIN; maze[7][25] = MazeObject::COIN; maze[7][35] = MazeObject::COIN; maze[9][10] = MazeObject::COIN;
        maze[9][20] = MazeObject::COIN; maze[9][30] = MazeObject::COIN; maze[11][5] = MazeObject::COIN; maze[11][15] = MazeObject::COIN;
        maze[11][25] = MazeObject::COIN; maze[11][35] = MazeObject::COIN;
        exitX = 12; exitY = 48; maze[exitX][exitY] = MazeObject::EXIT;
        break;
    case 1:
        for (int y = 3; y <= 5; y++) { maze[y][10] = MazeObject::WALL; maze[y][20] = MazeObject::WALL; maze[y][30] = MazeObject::WALL; maze[y][40] = MazeObject::WALL; }
        for (int y = 8; y <= 10; y++) { maze[y][15] = MazeObject::WALL; maze[y][25] = MazeObject::WALL; maze[y][35] = MazeObject::WALL; }
        maze[2][5] = MazeObject::COIN; maze[2][15] = MazeObject::COIN; maze[2][25] = MazeObject::COIN; maze[2][35] = MazeObject::COIN;
        maze[2][45] = MazeObject::COIN; maze[4][10] = MazeObject::COIN; maze[4][20] = MazeObject::COIN; maze[4][30] = MazeObject::COIN;
        maze[4][40] = MazeObject::COIN; maze[6][5] = MazeObject::COIN; maze[6][15] = MazeObject::COIN; maze[6][25] = MazeObject::COIN;
        maze[6][35] = MazeObject::COIN; maze[8][10] = MazeObject::COIN; maze[8][20] = MazeObject::COIN; maze[8][30] = MazeObject::COIN;
        maze[10][5] = MazeObject::COIN; maze[10][15] = MazeObject::COIN; maze[10][25] = MazeObject::COIN; maze[10][35] = MazeObject::COIN;
        maze[12][10] = MazeObject::COIN; maze[12][20] = MazeObject::COIN; maze[12][30] = MazeObject::COIN; maze[12][40] = MazeObject::COIN;
        exitX = 12; exitY = 1; maze[exitX][exitY] = MazeObject::EXIT;
        break;
    case 2:
        for (int x = 5; x <= 45; x += 5) { maze[5][x] = MazeObject::WALL; maze[9][x] = MazeObject::WALL; }
        for (int y = 2; y <= 12; y += 2) { maze[y][5] = MazeObject::WALL; maze[y][45] = MazeObject::WALL; }
        maze[1][3] = MazeObject::COIN; maze[1][7] = MazeObject::COIN; maze[1][12] = MazeObject::COIN; maze[1][17] = MazeObject::COIN;
        maze[1][22] = MazeObject::COIN; maze[1][27] = MazeObject::COIN; maze[1][32] = MazeObject::COIN; maze[1][37] = MazeObject::COIN;
        maze[1][42] = MazeObject::COIN; maze[3][5] = MazeObject::COIN; maze[3][15] = MazeObject::COIN; maze[3][25] = MazeObject::COIN;
        maze[3][35] = MazeObject::COIN; maze[3][45] = MazeObject::COIN; maze[7][5] = MazeObject::COIN; maze[7][15] = MazeObject::COIN;
        maze[7][25] = MazeObject::COIN; maze[7][35] = MazeObject::COIN; maze[7][45] = MazeObject::COIN; maze[11][5] = MazeObject::COIN;
        maze[11][15] = MazeObject::COIN; maze[11][25] = MazeObject::COIN; maze[11][35] = MazeObject::COIN; maze[11][45] = MazeObject::COIN;
        exitX = 7; exitY = 25; maze[exitX][exitY] = MazeObject::EXIT;
        break;
    }

    int enemiesToPlace = difficulties[currentDifficulty].enemyCount;
    while (enemiesToPlace > 0) {
        int x = rand() % (WIDTH - 2) + 1;
        int y = rand() % (HEIGHT - 2) + 1;
        if (maze[y][x] == MazeObject::HALL && !(x == playerX && y == playerY) && !(x == backExitY && y == backExitX)) {
            maze[y][x] = MazeObject::ENEMY;
            enemiesToPlace--;
        }
    }

    int healthPickupsToPlace = (level == 2) ? difficulties[currentDifficulty].healthPickupsLevel3 : difficulties[currentDifficulty].healthPickups;
    while (healthPickupsToPlace > 0) {
        int x = rand() % (WIDTH - 2) + 1;
        int y = rand() % (HEIGHT - 2) + 1;
        if (maze[y][x] == MazeObject::HALL && !(x == playerX && y == playerY) && !(x == backExitY && y == backExitX) && maze[y][x] != MazeObject::ENEMY) {
            maze[y][x] = MazeObject::HEALTH_PICKUP;
            healthPickupsToPlace--;
        }
    }

    if (level == TOTAL_LEVELS - 1 && coins >= COINS_FOR_BOSS && !bossSpawned) {
        bossSpawned = true;
        bossHealth = difficulties[currentDifficulty].bossHealth;
        bossDamage = difficulties[currentDifficulty].bossDamage;
        bossChargeCounter = 0;
        bossWarningTimer = 0;

        bool bossPlaced = false;
        while (!bossPlaced) {
            bossX = rand() % (HEIGHT - 2) + 1;
            bossY = rand() % (WIDTH - 2) + 1;
            if (maze[bossX][bossY] == MazeObject::HALL && !(bossX == playerX && bossY == playerY) && !(bossX == backExitX && bossY == backExitY)) {
                maze[bossX][bossY] = MazeObject::BOSS;
                bossPlaced = true;
            }
        }
    }
}

// Сброс игры
void resetGame(int& playerX, int& playerY, int& coins, int& health, int maze[HEIGHT][WIDTH]) {
    coins = 0;
    health = 100;
    currentLevel = 0;
    bossSpawned = false;
    bossX = -1;
    bossY = -1;
    bossMoveCounter = 0;
    bossChargeCounter = 0;
    bossWarningTimer = 0;
    bossAttackTimer = 0;
    coinWarningTimer = 0;
    clearAttackZone(attackZone);
    initLevel(currentLevel, playerX, playerY, exitX, exitY, maze);
}

// Основная игровая функция
void playGame(HANDLE h) {
    resetGame(playerX, playerY, coins, health, maze);
    printMaze(h);

    bool gameOver = false;
    COORD oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
    COORD position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
    SetConsoleCursorPosition(h, position);
    setColor(Color::BLUE);
    cout << "@";

    while (!gameOver) {
        if (playerX == exitX && playerY == exitY) {
            int transitionCost = difficulties[currentDifficulty].levelTransitionCost;
            if (coins < transitionCost) {
                coinWarningTimer = COIN_WARNING_DURATION;
                printMaze(h); // Обновляем экран
                continue;
            }

            coins -= transitionCost;
            Beep(500, 200); Beep(600, 200); Beep(700, 200); Beep(800, 300);
            currentLevel++;

            if (currentLevel == 2 && coins < COINS_FOR_BOSS) {
                coinWarningTimer = COIN_WARNING_DURATION;
            }

            if (currentLevel < TOTAL_LEVELS) {
                if (currentDifficulty == 0 || currentDifficulty == 1) health = 100;
                initLevel(currentLevel, playerX, playerY, exitX, exitY, maze);
                printMaze(h);
                oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                SetConsoleCursorPosition(h, position);
                setColor(Color::BLUE);
                cout << "@";
                continue;
            }
            else {
                MessageBoxA(0, "Вы прошли все уровни! Поздравляем!", "ПОБЕДА!", 0);
                int answer = MessageBoxA(0, "Что хотите сделать?\n\nДа - Сыграть снова\nНет - Вернуться в главное меню\nОтмена - Выйти", "ИГРА ЗАВЕРШЕНА", MB_YESNOCANCEL);
                system("cls");
                if (answer == IDYES) {
                    resetGame(playerX, playerY, coins, health, maze);
                    printMaze(h);
                    oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                    position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                    SetConsoleCursorPosition(h, position);
                    setColor(Color::BLUE);
                    cout << "@";
                    continue;
                }
                else if (answer == IDNO) {
                    system("cls");
                    mainMenu(h);
                    return;
                }
                else {
                    exit(0);
                }
            }
        }

        if (playerX == backExitX && playerY == backExitY && currentLevel > 0) {
            Beep(400, 200);
            currentLevel--;
            initLevel(currentLevel, playerX, playerY, exitX, exitY, maze);
            printMaze(h);
            oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
            position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
            SetConsoleCursorPosition(h, position);
            setColor(Color::BLUE);
            cout << "@";
            continue;
        }

        if (_kbhit()) {
            int code = _getch();
            if (code == 224) code = _getch();

            int newX = playerX, newY = playerY;
            switch (code) {
            case KeyCode::LEFT: case 'a': case 'A': newY--; break;
            case KeyCode::RIGHT: case 'd': case 'D': newY++; break;
            case KeyCode::UP: case 'w': case 'W': newX--; break;
            case KeyCode::DOWN: case 's': case 'S': newX++; break;
            case KeyCode::ESCAPE: exit(0); break;
            default: continue;
            }

            if (newX >= 0 && newX < HEIGHT && newY >= 0 && newY < WIDTH) {
                if (maze[newX][newY] != MazeObject::WALL && maze[newX][newY] != MazeObject::BORDER) {
                    if (maze[newX][newY] == MazeObject::COIN) {
                        coins++;
                        maze[newX][newY] = MazeObject::HALL;
                        Beep(1000, 100);
                        COORD coinPos = { static_cast<SHORT>(newY), static_cast<SHORT>(newX) };
                        SetConsoleCursorPosition(h, coinPos);
                        cout << " ";
                        updateStats(h);
                    }
                    else if (maze[newX][newY] == MazeObject::HEALTH_PICKUP) {
                        health += HEALTH_PICKUP_VALUE;
                        if (health > 100) health = 100;
                        maze[newX][newY] = MazeObject::HALL;
                        Beep(800, 100);
                        COORD healthPos = { static_cast<SHORT>(newY), static_cast<SHORT>(newX) };
                        SetConsoleCursorPosition(h, healthPos);
                        cout << " ";
                        updateStats(h);
                    }
                    else if (maze[newX][newY] == MazeObject::ENEMY) {
                        health -= difficulties[currentDifficulty].enemyDamage;
                        maze[newX][newY] = MazeObject::HALL;
                        Beep(300, 200);
                        updateStats(h);
                        if (health <= 0) {
                            Beep(200, 500);
                            MessageBeep(MB_ICONEXCLAMATION);
                            int answer = MessageBoxA(0, "Здоровье закончилось!\n\nДа - Сыграть снова\nНет - Вернуться в главное меню\nОтмена - Выйти", "ПОРАЖЕНИЕ!", MB_YESNOCANCEL);
                            system("cls");
                            if (answer == IDYES) {
                                resetGame(playerX, playerY, coins, health, maze);
                                printMaze(h);
                                oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                                position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                                SetConsoleCursorPosition(h, position);
                                setColor(Color::BLUE);
                                cout << "@";
                                continue;
                            }
                            else if (answer == IDNO) {
                                system("cls");
                                mainMenu(h);
                                return;
                            }
                            else {
                                exit(0);
                            }
                        }
                    }
                    else if (maze[newX][newY] == MazeObject::BOSS) {
                        health -= bossDamage;
                        bossHealth -= 20;
                        Beep(200, 300);
                        if (bossHealth <= 0) {
                            maze[newX][newY] = MazeObject::HALL;
                            bossSpawned = false;
                            bossX = -1;
                            bossY = -1;
                            bossChargeCounter = 0;
                            bossWarningTimer = 0;
                            bossAttackTimer = 0;
                            clearAttackZone(attackZone);
                            Beep(500, 200); Beep(600, 200); Beep(700, 200);
                            MessageBoxA(0, "Вы победили босса! Продолжайте путь к выходу!", "ПОБЕДА НАД БОССОМ!", 0);
                        }
                        updateStats(h);
                        if (health <= 0) {
                            Beep(200, 500);
                            MessageBeep(MB_ICONEXCLAMATION);
                            int answer = MessageBoxA(0, "Здоровье закончилось!\n\nДа - Сыграть снова\nНет - Вернуться в главное меню\nОтмена - Выйти", "ПОРАЖЕНИЕ!", MB_YESNOCANCEL);
                            system("cls");
                            if (answer == IDYES) {
                                resetGame(playerX, playerY, coins, health, maze);
                                printMaze(h);
                                oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                                position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                                SetConsoleCursorPosition(h, position);
                                setColor(Color::BLUE);
                                cout << "@";
                                continue;
                            }
                            else if (answer == IDNO) {
                                system("cls");
                                mainMenu(h);
                                return;
                            }
                            else {
                                exit(0);
                            }
                        }
                    }
                    SetConsoleCursorPosition(h, oldPlayerPos);
                    cout << " ";
                    playerX = newX;
                    playerY = newY;
                    position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                    SetConsoleCursorPosition(h, position);
                    setColor(Color::BLUE);
                    cout << "@";
                    oldPlayerPos = position;
                    updateStats(h);
                }
            }
        }

        if (bossSpawned) {
            int dx = abs(playerX - bossX);
            int dy = abs(playerY - bossY);
            if ((dx <= 1 && dy <= 1) && (dx + dy > 0)) {
                health -= 3;
                Beep(400, 100);
                updateStats(h);
                if (health <= 0) {
                    Beep(200, 500);
                    MessageBeep(MB_ICONEXCLAMATION);
                    int answer = MessageBoxA(0, "Здоровье закончилось!\n\nДа - Сыграть снова\nНет - Вернуться в главное меню\nОтмена - Выйти", "ПОРАЖЕНИЕ!", MB_YESNOCANCEL);
                    system("cls");
                    if (answer == IDYES) {
                        resetGame(playerX, playerY, coins, health, maze);
                        printMaze(h);
                        oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                        position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                        SetConsoleCursorPosition(h, position);
                        setColor(Color::BLUE);
                        cout << "@";
                        continue;
                    }
                    else if (answer == IDNO) {
                        system("cls");
                        mainMenu(h);
                        return;
                    }
                    else {
                        exit(0);
                    }
                }
            }
        }

        COORD enemy_positions[100];
        int enemy_count = 0;
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (maze[y][x] == MazeObject::ENEMY || maze[y][x] == MazeObject::BOSS) {
                    enemy_positions[enemy_count].X = x;
                    enemy_positions[enemy_count].Y = y;
                    enemy_count++;
                }
            }
        }

        for (int i = 0; i < enemy_count; i++) {
            int newEnemyX = enemy_positions[i].Y;
            int newEnemyY = enemy_positions[i].X;
            bool isBoss = (maze[newEnemyX][newEnemyY] == MazeObject::BOSS);

            if (isBoss && bossSpawned) {
                bossMoveCounter++;
                if (bossMoveCounter % 3 != 0) continue;

                if (playerX < newEnemyX && newEnemyX > 0 && maze[newEnemyX - 1][newEnemyY] == MazeObject::HALL) newEnemyX--;
                else if (playerX > newEnemyX && newEnemyX < HEIGHT - 1 && maze[newEnemyX + 1][newEnemyY] == MazeObject::HALL) newEnemyX++;
                else if (playerY < newEnemyY && newEnemyY > 0 && maze[newEnemyX][newEnemyY - 1] == MazeObject::HALL) newEnemyY--;
                else if (playerY > newEnemyY && newEnemyY < WIDTH - 1 && maze[newEnemyX][newEnemyY + 1] == MazeObject::HALL) newEnemyY++;
            }
            else {
                int r = rand() % 4;
                if (r == 0 && newEnemyY > 0 && maze[newEnemyX][newEnemyY - 1] == MazeObject::HALL) newEnemyY--;
                else if (r == 1 && newEnemyY < WIDTH - 1 && maze[newEnemyX][newEnemyY + 1] == MazeObject::HALL) newEnemyY++;
                else if (r == 2 && newEnemyX > 0 && maze[newEnemyX - 1][newEnemyY] == MazeObject::HALL) newEnemyX--;
                else if (r == 3 && newEnemyX < HEIGHT - 1 && maze[newEnemyX + 1][newEnemyY] == MazeObject::HALL) newEnemyX++;
            }

            if (newEnemyX == playerX && newEnemyY == playerY) {
                if (isBoss) {
                    health -= bossDamage;
                    bossHealth -= 20;
                    Beep(200, 300);
                    if (bossHealth <= 0) {
                        maze[newEnemyX][newEnemyY] = MazeObject::HALL;
                        bossSpawned = false;
                        bossX = -1;
                        bossY = -1;
                        bossChargeCounter = 0;
                        bossWarningTimer = 0;
                        bossAttackTimer = 0;
                        clearAttackZone(attackZone);
                        Beep(500, 200); Beep(600, 200); Beep(700, 200);
                        MessageBoxA(0, "Вы победили босса! Продолжайте путь к выходу!", "ПОБЕДА НАД БОССОМ!", 0);
                    }
                }
                else {
                    health -= difficulties[currentDifficulty].enemyDamage;
                    Beep(300, 200);
                }
                updateStats(h);
                if (health <= 0) {
                    Beep(200, 500);
                    MessageBeep(MB_ICONEXCLAMATION);
                    int answer = MessageBoxA(0, "Здоровье закончилось!\n\nДа - Сыграть снова\nНет - Вернуться в главное меню\nОтмена - Выйти", "ПОРАЖЕНИЕ!", MB_YESNOCANCEL);
                    system("cls");
                    if (answer == IDYES) {
                        resetGame(playerX, playerY, coins, health, maze);
                        printMaze(h);
                        oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                        position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                        SetConsoleCursorPosition(h, position);
                        setColor(Color::BLUE);
                        cout << "@";
                        continue;
                    }
                    else if (answer == IDNO) {
                        system("cls");
                        mainMenu(h);
                        return;
                    }
                    else {
                        exit(0);
                    }
                }
                continue;
            }

            if (newEnemyX != enemy_positions[i].Y || newEnemyY != enemy_positions[i].X) {
                COORD temp = enemy_positions[i];
                SetConsoleCursorPosition(h, temp);
                cout << " ";
                maze[enemy_positions[i].Y][enemy_positions[i].X] = MazeObject::HALL;
                temp.X = newEnemyY; temp.Y = newEnemyX;
                SetConsoleCursorPosition(h, temp);
                if (isBoss) {
                    setColor(Color::MAGENTA); cout << "B";
                    bossX = newEnemyX; bossY = newEnemyY;
                    maze[newEnemyX][newEnemyY] = MazeObject::BOSS;
                }
                else {
                    setColor(Color::RED); cout << "E";
                    maze[newEnemyX][newEnemyY] = MazeObject::ENEMY;
                }
            }
        }

        if (bossSpawned) {
            bossChargeCounter++;
            if (bossChargeCounter == BOSS_CHARGE_MAX - 1) {
                bossWarningTimer = BOSS_WARNING_DURATION;
                clearAttackZone(attackZone);
                markAttackZone(bossX, bossY, attackZone);
            }
            if (bossChargeCounter >= BOSS_CHARGE_MAX) {
                int dx = abs(playerX - bossX);
                int dy = abs(playerY - bossY);
                if (dx <= 2 && dy <= 2) {
                    health -= difficulties[currentDifficulty].bossChargeDamage;
                    Beep(600, 150);
                    bossAttackTimer = BOSS_ATTACK_DURATION;
                    updateStats(h);
                    if (health <= 0) {
                        Beep(200, 500);
                        MessageBeep(MB_ICONEXCLAMATION);
                        int answer = MessageBoxA(0, "Здоровье закончилось!\n\nДа - Сыграть снова\nНет - Вернуться в главное меню\nОтмена - Выйти", "ПОРАЖЕНИЕ!", MB_YESNOCANCEL);
                        system("cls");
                        if (answer == IDYES) {
                            resetGame(playerX, playerY, coins, health, maze);
                            printMaze(h);
                            oldPlayerPos = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                            position = { static_cast<SHORT>(playerY), static_cast<SHORT>(playerX) };
                            SetConsoleCursorPosition(h, position);
                            setColor(Color::BLUE);
                            cout << "@";
                            continue;
                        }
                        else if (answer == IDNO) {
                            system("cls");
                            mainMenu(h);
                            return;
                        }
                        else {
                            exit(0);
                        }
                    }
                }
                bossChargeCounter = 0;
                clearAttackZone(attackZone);
            }
            printMaze(h);
            updateStats(h);
        }

        if (cheatsEnabled) {
            health = 9999;
            coins = 9999;
            updateStats(h);
            if (rand() % 50 == 0) cheatMessageTimer = 10;
        }

        Sleep(100);
    }
}

// Выбор сложности
void chooseDifficulty() {
    clearConsole();
    cout << "=== ВЫБОР УРОВНЯ СЛОЖНОСТИ ===\n\n";
    cout << "0. Назад\n";
    cout << "1. Мирно (переход: 5 монет)\n";
    cout << "2. Легкий (переход: 10 монет)\n";
    cout << "3. Средний (переход: 15 монет)\n";
    cout << "4. Сложно (переход: 20 монет)\n";
    cout << "5. Безумие (переход: 25 монет)\n";
    cout << "Ваш выбор: ";
    int choice;
    while (!(cin >> choice)) {
        cout << "Ошибка! Введите число.\n";
        cin.clear();
        cin.ignore(10000, '\n'); // Заменили numeric_limits
    }
    if (choice == 0) return;
    currentDifficulty = choice - 1;
    if (currentDifficulty < 0 || currentDifficulty > 4) {
        cout << "Неверный выбор, установлен уровень 'Мирно'.\n";
        currentDifficulty = 0;
    }
    cout << "Уровень сложности установлен. Нажмите любую клавишу...\n";
    _getch();
}

// Включение читов
void enableCheats() {
    clearConsole();
    cout << "=== ВКЛЮЧЕНИЕ ЧИТОВ ===\n\n";
    cout << "Включить читы? (y/n): ";
    char choice;
    cin >> choice;
    if (choice == 'y' || choice == 'Y') {
        cheatsEnabled = true;
        cout << "Читы включены!\n";
    }
    else {
        cheatsEnabled = false;
        cout << "Читы отключены.\n";
    }
    cout << "Нажмите любую клавишу...\n";
    _getch();
}

// Правила
void showRules() {
    clearConsole();
    cout << "=== ПРАВИЛА И УПРАВЛЕНИЕ ===\n\n";
    cout << "Правила:\n";
    cout << "- Цель — дойти до выхода (X), собирая монеты ($).\n";
    cout << "- Переход на новый уровень стоит монеты (5-25).\n";
    cout << "- Враги (E) наносят урон. Здоровье = 0 — проигрыш.\n";
    cout << "- '+' — аптечки, восстанавливают здоровье.\n";
    cout << "- На 3-м уровне при " << COINS_FOR_BOSS << " монетах появляется босс (B).\n";
    cout << "Управление:\n";
    cout << "- WASD или стрелки — движение.\n";
    cout << "- ESC — выход.\n\n";
    cout << "Нажмите любую клавишу...\n";
    _getch();
}

// Главное меню
void mainMenu(HANDLE h) {
    clearConsole();
    cout << "=== ГЛАВНОЕ МЕНЮ ===\n";
    cout << "1. Начать игру\n";
    cout << "2. Выбрать уровень сложности\n";
    cout << "3. Включить читы\n";
    cout << "4. Правила и управление\n";
    cout << "5. Выйти\n";
    cout << "Выберите опцию: ";

    int choice;
    while (true) {
        if (cin >> choice) {
            break;
        }
        else {
            cout << "Ошибка! Введите число от 1 до 5.\n";
            cin.clear();
            cin.ignore(10000, '\n'); // Заменили numeric_limits
            cout << "Выберите опцию: ";
        }
    }

    switch (choice) {
    case 1: playGame(h); break;
    case 2: chooseDifficulty(); mainMenu(h); break;
    case 3: enableCheats(); mainMenu(h); break;
    case 4: showRules(); mainMenu(h); break;
    case 5: exit(0);
    default:
        cout << "Неверный выбор. Нажмите любую клавишу...\n";
        _getch();
        mainMenu(h);
    }
}

// Точка входа
int main() {
    setlocale(LC_ALL, "Russian");
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    srand(static_cast<unsigned int>(time(0)));
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    mainMenu(h);
    CloseHandle(h);
    return 0;
}
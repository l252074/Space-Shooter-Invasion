#include "raylib.h"
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <string>
using namespace std;
const int MAX_ENEMIES = 100;
const int MAX_BULLETS = 100;
const int MAX_SCORES = 100;
const int screenWidth = 800;
const int screenHeight = 600;
const float PLAYER_SPEED = 6.0f;
const float PLAYER_Y = screenHeight - 50.0f;
float backgroundPosition = 0;
float backgroundSpeed = 2;
Texture2D background;
Texture2D enemy1;
Texture2D enemy2;
Texture2D player;
Texture2D bulletp;
Texture2D heart;
Texture2D bullete;
Texture2D bosss;
Sound explosionSound;
Sound playerHitSound;
Sound levelCompleteSound;
Sound playerDeadSound;
struct Enemy {
    Vector2 pos = { 0, 0 };
    bool alive = false;
    int type = 1;
};
struct Bullet {
    Vector2 pos = { 0, 0 };
    bool active = false;
    int ownerType = 0;
};
struct Boss {
    Vector2 pos = { 0, 0 };
    float speed = 0.0f;
    int health = 0;
    int damage = 1;
    bool active = false;
} boss;
Enemy enemies[MAX_ENEMIES];
Bullet pBullets[MAX_BULLETS];
Bullet eBullets[MAX_BULLETS];
int enemyCount = 0;
int rows = 3;
int cols = 6;
int level = 1;
int score = 0;
int lives = 3;
int highScore = 0;
int scoreList[MAX_SCORES];
int storedScoreCount = 0;
float enemySpeed = 1.0f;
float playerX = screenWidth * 0.5f;
float playerWidth = 0.0f;
float playerHeight = 0.0f;
float bulletWidth = 0.0f;
float bulletHeight = 0.0f;
float enemyBulletWidth = 8.0f;
float enemyBulletHeight = 16.0f;
bool isBossLevel = false;
bool prevAllEnemiesDead = false;
int menuIndex = 0;
const int STATE_MENU = 0;
const int STATE_INSTRUCTIONS = 1;
const int STATE_SCORES = 2;
const int STATE_HIGHSCORE = 3;
const int STATE_GAME = 4;
const int STATE_EXIT = 5;
int currentState = STATE_MENU;
bool ShowCongratulationsScreen();
void SaveScore(int s);
int convertToInt(const string& s);
void LoadScores();
void InitAll();
void SpawnEnemiesGrid(int r, int c);
void SpawnBoss();
void FirePlayerBullet();
int ChooseRandomAliveEnemy();
void FireEnemyBulletFromRandom();
void FireBossBullet();
void UpdatePlayerBullets();
void UpdateEnemyBullets();
void UpdateEnemiesHorizontal(float& direction);
void UpdateBoss();
void CheckPlayerBulletEnemyCollisions();
void CheckEnemyBulletPlayerCollisions();
bool AllEnemiesDead();
void GameReset() {
    level = 1;
    score = 0;
    lives = 3;
    playerX = screenWidth * 0.5f;
    enemySpeed = 1.0f;
    isBossLevel = false;
    prevAllEnemiesDead = false;
    menuIndex = 0;
    InitAll();
    SpawnEnemiesGrid(3, 6);
}
void UpdateBackground();
void DrawBackground();
void handleMenuInput() {
    if (IsKeyPressed(KEY_DOWN)) menuIndex++;
    if (IsKeyPressed(KEY_UP)) menuIndex--;
    if (menuIndex < 0) menuIndex = 4;
    if (menuIndex > 4) menuIndex = 0;
    if (IsKeyPressed(KEY_ENTER)) {
        if (menuIndex == 0) currentState = STATE_GAME;
        else if (menuIndex == 1) currentState = STATE_INSTRUCTIONS;
        else if (menuIndex == 2) currentState = STATE_SCORES;
        else if (menuIndex == 3) currentState = STATE_HIGHSCORE;
        else if (menuIndex == 4) currentState = STATE_EXIT;
    }
}
void drawMenuScreen() {
    BeginDrawing();
    DrawTexture(background, 0, 0, WHITE);
    DrawText("SPACE INVASION", screenWidth / 2 - 170, 60, 40, YELLOW);
    char options[5][20] = {
        "START GAME",
        "INSTRUCTIONS",
        "SCORES",
        "HIGH SCORE",
        "EXIT"
    };
    for (int i = 0; i < 5; i++) {
        Color color = (i == menuIndex) ? YELLOW : WHITE;
        DrawText(options[i], screenWidth / 2 - 120, 200 + i * 50, 30, color);
    }
    EndDrawing();
}

void drawInstructionsScreen() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("-> INSTRUCTIONS <-", screenWidth / 2 - 160, 60, 40, WHITE);
    DrawText("_To Move ->(A/D or Left/Right<-", screenWidth / 2 - 200, 200, 25, GREEN);
    DrawText("_SPACE to -> (S H O O T) <-", screenWidth / 2 - 200, 240, 25, GREEN);
    DrawText("_Backspace to return to MAIN MENU", screenWidth / 2 - 200, 320, 25, YELLOW);
    EndDrawing();
    if (IsKeyPressed(KEY_BACKSPACE)) {
        currentState = STATE_MENU;
        menuIndex = 0;
    }
}
void drawScoresScreen() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("PAST SCORES", screenWidth / 2 - 120, 40, 35, YELLOW);
    for (int i = 0; i < storedScoreCount; i++) {
        DrawText(TextFormat("%d", scoreList[i]), screenWidth / 2 - 20, 120 + i * 20, 20, WHITE);
    }
    DrawText("Press BACKSPACE to return", screenWidth / 2 - 160, screenHeight - 60, 20, GREEN);
    EndDrawing();

    if (IsKeyPressed(KEY_BACKSPACE)) {
        currentState = STATE_MENU;
        menuIndex = 0;
    }
}
void drawHighScoreScreen() {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText("HIGHEST SCORE", screenWidth / 2 - 150, 80, 40, YELLOW);
    DrawText(TextFormat("%d", highScore),
        screenWidth / 2 - 20, 180, 30, GREEN);
    DrawText("Press BACKSPACE to return",
        screenWidth / 2 - 160, screenHeight - 60, 20, GREEN);
    EndDrawing();
    if (IsKeyPressed(KEY_BACKSPACE)) {
        currentState = STATE_MENU;
        menuIndex = 0;
    }
}
int convertToInt(const string& s) {
    int number = 0;  
    bool isValid = true;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] < '0' || s[i] > '9') {  
            isValid = false;
            break;
        }
    }
    if (isValid && s.length() > 0) {
        number = 0;
        for (int i = 0; i < s.length(); i++) {
            number = number * 10 + (s[i] - '0');  
        }
    }return number;
}

void LoadScores() {
    storedScoreCount = 0;
    highScore = 0;
    ifstream in("highscore.txt");
    if (!in.is_open()) return;
    int value;
    while (in >> value && storedScoreCount < MAX_SCORES) {
        scoreList[storedScoreCount] = value;
        storedScoreCount++;
       if (value > highScore)
            highScore = value;
    }
    in.close();
}

void SaveScore(int s) {
   ofstream out("highscore.txt", ios::app);
    if (out.is_open()) {
        out << s << "\n";
    }
}

void InitAll() {
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i] = {};
    for (int i = 0; i < MAX_BULLETS; i++) {
        pBullets[i] = {};
        eBullets[i] = {};
    }
    boss = {};
}
void SpawnEnemiesGrid(int r, int c) {
    if (r < 1) r = 1;
    if (c < 1) c = 1;
    if (r * c > MAX_ENEMIES) {
     if (r > MAX_ENEMIES) r = MAX_ENEMIES; 
     c = MAX_ENEMIES / r;                   
 }
    rows = r;
    cols = c;
    enemyCount = r * c;
    const float gapX = 70;
    const float gapY = 50;
    const float startX = 80;
    const float startY = 60;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].alive = false;
        enemies[i].type = 1;
    }
    int specialTopRows = 0;
    if (level == 4) specialTopRows = 2;
    if (level == 5) specialTopRows = 3;
    int idx = 0;
    for (int r0 = 0; r0 < r; r0++) {
        for (int c0 = 0; c0 < c; c0++) {
            enemies[idx].pos.x = startX + c0 * gapX;
            enemies[idx].pos.y = startY + r0 * gapY;
            enemies[idx].alive = true;
            enemies[idx].type = (r0 < specialTopRows) ? 2 : 1;
            idx++;
        }
    }
}
void SpawnBoss() {
    boss.active = true;
    isBossLevel = true;
    boss.pos = { screenWidth / 2.0f, 120.0f };
    boss.health = 100;
    boss.speed = 3.5f;
    boss.damage = 1;
}
void FirePlayerBullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!pBullets[i].active) {
            pBullets[i].active = true;
            pBullets[i].ownerType = 0;
            pBullets[i].pos.x = playerX;
            pBullets[i].pos.y = PLAYER_Y - playerHeight / 2;
            break;
        }
    }
}
int ChooseRandomAliveEnemy() {
    int totalWeight = 0;
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].alive) {
            if (enemies[i].type == 2) totalWeight += 2;
            else totalWeight += 1;
        }
    }
    if (totalWeight == 0) return -1;
    int pick = GetRandomValue(0, totalWeight - 1);
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].alive) continue;
        int weight = (enemies[i].type == 2) ? 2 : 1;
        if (pick < weight)
            return i;
        pick -= weight;
    }return -1;
}

void FireEnemyBulletFromRandom() {
    int id = ChooseRandomAliveEnemy();
    if (id < 0) return;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!eBullets[i].active) {
            eBullets[i].active = true;
            eBullets[i].ownerType = enemies[id].type;
            eBullets[i].pos.x = enemies[id].pos.x;
            eBullets[i].pos.y = enemies[id].pos.y + 18;
            break;
        }
    }
}
void FireBossBullet() {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!eBullets[i].active) {
            eBullets[i].active = true;
            eBullets[i].ownerType = 3;
            eBullets[i].pos.x = boss.pos.x;
            eBullets[i].pos.y = boss.pos.y + 40;
            break;
        }
    }
}
void UpdatePlayerBullets() {
    const float speed = 8.0f;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (pBullets[i].active) {
            pBullets[i].pos.y -= speed;
            if (pBullets[i].pos.y < -10) pBullets[i].active = false;
        }
    }
}
void UpdateEnemyBullets() {
    float baseSpeed = (level >= 4) ? 8.0f : 4.0f;
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!eBullets[i].active) continue;
        float useSpeed = baseSpeed;
        if (eBullets[i].ownerType == 2) useSpeed += 2.0f;
        else if (eBullets[i].ownerType == 3) useSpeed += 4.0f;
        eBullets[i].pos.y += useSpeed;
        if (eBullets[i].pos.y > screenHeight + 10)
            eBullets[i].active = false;
    }
}
void UpdateEnemiesHorizontal(float& direction) {
    bool needReverse = false;
    float dx = enemySpeed * direction;
    for (int i = 0; i < enemyCount; i++) {
        if (!enemies[i].alive) continue;
        enemies[i].pos.x += dx;
        if (enemies[i].pos.x < 20 || enemies[i].pos.x > screenWidth - 40)
            needReverse = true;
    }
    if (needReverse) {
        direction *= -1;
        for (int i = 0; i < enemyCount; i++) {
            if (enemies[i].alive)
                enemies[i].pos.x += enemySpeed * direction;
        }
    }
}
void UpdateBoss() {
    if (!boss.active) return;
    boss.pos.x += boss.speed;
    if (boss.pos.x < 60) boss.speed = fabs(boss.speed);
    if (boss.pos.x > screenWidth - 60) boss.speed = -fabs(boss.speed);
    if (GetRandomValue(0, 100) < 6)
        FireBossBullet();
}
void CheckPlayerBulletEnemyCollisions() {
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!pBullets[b].active) continue;
        for (int e = 0; e < enemyCount; e++) {
            if (!enemies[e].alive) continue;
            float dx = fabs(pBullets[b].pos.x - enemies[e].pos.x);
            float dy = fabs(pBullets[b].pos.y - enemies[e].pos.y);
            if (dx < 28 && dy < 20) {
                PlaySound(explosionSound);
                pBullets[b].active = false;
                enemies[e].alive = false;
                score += (enemies[e].type == 2) ? 3 : 1;
                if (score > highScore) highScore = score;
                break;
            }
        }
    }
    if (boss.active) {
        for (int b = 0; b < MAX_BULLETS; b++) {
            if (!pBullets[b].active) continue;
            float dx = fabs(pBullets[b].pos.x - boss.pos.x);
            float dy = fabs(pBullets[b].pos.y - boss.pos.y);
            if (dx < 100 && dy < 60) {
                pBullets[b].active = false;
                boss.health-=2;
                PlaySound(explosionSound);

                if (boss.health <= 0) {
                    boss.active = false;
                    isBossLevel = false;
                    score += 100;
                    if (score > highScore) highScore = score;
                    SaveScore(score);
                    ShowCongratulationsScreen();
                    level = 1;
                    score = 0;
                    lives = 3;
                    playerX = screenWidth * 0.5f;
                    SpawnEnemiesGrid(3, 6);
                    enemySpeed = 1.0f;
                    InitAll();
                    currentState = STATE_MENU;
                }
            }
        }
    }
}
void CheckEnemyBulletPlayerCollisions() {
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!eBullets[b].active) continue;
        float dx = fabs(eBullets[b].pos.x - playerX);
        float dy = fabs(eBullets[b].pos.y - PLAYER_Y);
        if (dx < playerWidth / 2 && dy < playerHeight / 2) {
            PlaySound(playerHitSound);
            eBullets[b].active = false;
            lives--;
        }
    }
}
bool AllEnemiesDead() {
    for (int i = 0; i < enemyCount; i++)
        if (enemies[i].alive) return false;
    return true;
}
bool ShowCongratulationsScreen() {
    while (!WindowShouldClose()) {
        BeginDrawing();
        DrawTexture(background, 0, 0, WHITE);
        DrawText("CONGRATULATIONS!", screenWidth / 2 - 180, screenHeight / 2 - 40, 40, GOLD);
        DrawText("You defeated the boss!", screenWidth / 2 - 160, screenHeight / 2 + 20, 20, WHITE);
        DrawText("Press ENTER to restart\nESC to quit", screenWidth / 2 - 200, screenHeight / 2 + 60, 20, WHITE);
        EndDrawing();
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) return true;
        if (IsKeyPressed(KEY_ESCAPE)) return false;
    }
    return false;
}
void UpdateBackground() {
    backgroundPosition = backgroundPosition + backgroundSpeed;
    if (backgroundPosition >= (float)screenHeight) {
        backgroundPosition -= (float)screenHeight;
    }
}
void DrawBackground() {
    DrawTexture(background, 0, (int)backgroundPosition, WHITE);
    DrawTexture(background, 0, (int)(backgroundPosition - screenHeight), WHITE);
}
int main() {
    InitWindow(screenWidth, screenHeight, "Space Invasion");
    InitAudioDevice();
    background = LoadTexture("background2.png");
    enemy1 = LoadTexture("alien_2.png");
    enemy2 = LoadTexture("alien3.png");
    player = LoadTexture("spaceship41.png");
    bulletp = LoadTexture("blt1.png");
    heart = LoadTexture("lives.png");
    bullete = LoadTexture("xyz.png");
    bosss = LoadTexture("boss.png");
    explosionSound = LoadSound("shoot.wav");
    playerHitSound = LoadSound("hit.wav");
    levelCompleteSound = LoadSound("complete.wav");
    playerDeadSound = LoadSound("losing.wav");
    playerWidth = (float)player.width;
    playerHeight = (float)player.height;
    bulletWidth = (float)bulletp.width;
    bulletHeight = (float)bulletp.height;
    if (bullete.width > 0) {
        enemyBulletWidth = (float)bullete.width;
        enemyBulletHeight = (float)bullete.height;
    }
    SetTargetFPS(60);
    InitAll();
    LoadScores();
    float enemyDirection = 1.0f;
    // game loop
    while (!WindowShouldClose() && currentState != STATE_EXIT) {
        switch (currentState) {
        case STATE_MENU:
            handleMenuInput();
            drawMenuScreen();
            if (currentState == STATE_GAME) GameReset();
            break;
        case STATE_INSTRUCTIONS:
            drawInstructionsScreen();
            break;
        case STATE_SCORES:
            drawScoresScreen();
            break;
        case STATE_HIGHSCORE:
            drawHighScoreScreen();
            break;
        case STATE_GAME:
            if (IsKeyPressed(KEY_BACKSPACE)) {
                SaveScore(score);
                LoadScores();
                currentState = STATE_MENU;
                menuIndex = 0;
                break;
            }
            if (IsKeyPressed(KEY_B) && !isBossLevel) {
                level = 6;
                SpawnBoss();
                isBossLevel = true;
                playerX = screenWidth / 2;
                lives = 3;
            }
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) playerX -= PLAYER_SPEED;
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) playerX += PLAYER_SPEED;
            if (playerX < 20) playerX = 20;
            if (playerX > screenWidth - 20) playerX = screenWidth - 20;
            if (IsKeyPressed(KEY_SPACE)) FirePlayerBullet();
            UpdatePlayerBullets();
            UpdateEnemyBullets();
            if (!isBossLevel) {
                UpdateEnemiesHorizontal(enemyDirection);
                if (level >= 2 && GetRandomValue(0, 300) < 3)
                    FireEnemyBulletFromRandom();
            }
            else {
                UpdateBoss();
            }
            CheckPlayerBulletEnemyCollisions();
            CheckEnemyBulletPlayerCollisions();
            if (lives <= 0) {
                PlaySound(playerDeadSound);
                SaveScore(score);
                LoadScores();
                prevAllEnemiesDead = false;
                currentState = STATE_MENU;
                menuIndex = 0;
                break;
            }
            bool allDead = AllEnemiesDead();
            if (allDead && !prevAllEnemiesDead && !isBossLevel) {
                level++;
                PlaySound(levelCompleteSound);
                if (level <= 5) {
                    int nr = 3 + (level - 1);
                    int nc = 6 + (level - 1);
                    if (nr * nc > MAX_ENEMIES) nc = MAX_ENEMIES / nr;
                    if (nc < 1) nc = 1;
                    SpawnEnemiesGrid(nr, nc);
                    enemySpeed = 1.0f + (level - 1) * 0.5f;
                }
                else {
                    SpawnBoss();
                }
            }
            prevAllEnemiesDead = allDead;
            UpdateBackground();
            BeginDrawing();
            DrawBackground();
            DrawText(TextFormat("SCORE: %d", score), 10, 10, 20, WHITE);
            DrawText(TextFormat("HIGH SCORE: %d", highScore), screenWidth / 2 - 80, 10, 20, YELLOW);
            DrawText(TextFormat("LEVEL: %d", level), screenWidth - 120, 10, 20, WHITE);
            float heartScale = 0.3f;
            int heartSpacing = 5;
            int heartStartX = screenWidth / 2 + 120;
            for (int i = 0; i < lives; i++) {
                float x = heartStartX + i * (heart.width * heartScale + heartSpacing);
                DrawTextureEx(heart, { x, 10.0f }, 0.0f, heartScale, WHITE);
            }
            DrawTexture(player, playerX - playerWidth / 2, PLAYER_Y - playerHeight / 2, WHITE);
            if (!isBossLevel) {
                for (int i = 0; i < enemyCount; i++) {
                    if (!enemies[i].alive) continue;
                    Texture2D tex = (enemies[i].type == 2) ? enemy2 : enemy1;
                    DrawTexture(tex, (int)(enemies[i].pos.x - tex.width / 2),
                        (int)(enemies[i].pos.y - tex.height / 2), WHITE);
                }
            }
            if (boss.active) {
                float sx = 4.0f * ((float)enemy1.width / bosss.width);
                float sy = 2.5f * ((float)enemy1.height / bosss.height);
                DrawTextureEx(bosss,
                    { boss.pos.x - (bosss.width * sx) / 2, boss.pos.y - (bosss.height * sy) / 2 },
                    0.0f, sx, WHITE);
                float barW = 150.0f;
                float hpPct = boss.health / 100.0f;
                DrawRectangle((int)(boss.pos.x - barW / 2),
                    (int)(boss.pos.y - bosss.height * sy / 2 - 12),
                    (int)(barW * hpPct), 8, RED);
                DrawRectangleLines((int)(boss.pos.x - barW / 2),
                    (int)(boss.pos.y - bosss.height * sy / 2 - 12),
                    (int)barW, 8, RAYWHITE);
            }
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (pBullets[i].active)
                    DrawTexture(bulletp, (int)(pBullets[i].pos.x - bulletWidth / 2),
                        (int)(pBullets[i].pos.y - bulletHeight / 2), WHITE);
                if (eBullets[i].active)
                    DrawTexture(bullete, (int)(eBullets[i].pos.x - enemyBulletWidth / 2),
                        (int)(eBullets[i].pos.y - enemyBulletHeight / 2), WHITE);
            }

            EndDrawing();
            break;
        }
    }
SaveScore(score);
UnloadTexture(enemy1);
UnloadTexture(enemy2);
UnloadTexture(player);
UnloadTexture(bulletp);
UnloadTexture(heart);
UnloadTexture(background);
UnloadTexture(bullete);
UnloadTexture(bosss);
UnloadSound(explosionSound);
UnloadSound(playerHitSound);
UnloadSound(levelCompleteSound);
UnloadSound(playerDeadSound);
CloseAudioDevice();
CloseWindow();
return 0;
}

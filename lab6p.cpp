// ============================================================
//  StarCatcher - OpenGL 2D Game
//  Computer Graphics Project
// ============================================================

//  CONTROLS:
//    LEFT  Arrow / A  → Move player left
//    RIGHT Arrow / D  → Move player right
//    UP    Arrow / W  → Move player up
//    DOWN  Arrow / S  → Move player down
//    SPACE           → Start game / Restart
//    ESC             → Quit
//    Z               → Zoom In
//    X               → Zoom Out
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <mmsystem.h>
#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>


// ================= SOUNDS =================
void playMusic() {
    PlaySound(L"C:/Users/janab/Documents/CG Labs/lab6/audio.wav",
        NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
}

void playWinSound() {
    PlaySound(L"C:/Users/janab/Documents/CG Labs/win.wav",
        NULL, SND_FILENAME | SND_ASYNC);
}

void playLoseSound() {
    PlaySound(L"C:/Users/janab/Documents/CG Labs/lab6/lose.wav",
        NULL, SND_FILENAME | SND_ASYNC);
}


// Window
const int WINDOW_W = 800;
const int WINDOW_H = 600;

// Scenes
enum Scene {
    START_SCREEN,
    HOW_TO_PLAY,
    PLAYING,
    GAME_OVER
};

Scene currentScene = START_SCREEN;

// Player
float playerX = 400;
float playerY = 80;
float playerSize = 40;
float playerSpeed = 8;

// Stars
const int MAX_STARS = 3;

float starX[MAX_STARS];
float starY[MAX_STARS];
float starSpeed[MAX_STARS];
float starRotate[MAX_STARS];

// Game
int score = 0;
int gameTime = 60;

bool playerWon = false;
std::string loseReason = "";

float zoomLevel = 1.0f;

int lastSecond = 0;

GLuint backgroundTexture;

// NEW — Pause texture
GLuint pauseTexture;

// Pause activation
bool isPaused = false;


// BMP Loader
GLuint loadBMP(const char* filename) {

    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Cannot open image\n");
        return 0;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, file);

    if (header[0] != 'B' || header[1] != 'M') {
        printf("Not BMP\n");
        fclose(file);
        return 0;
    }

    int dataPos = *(int*)&header[0x0A];
    int width = *(int*)&header[0x12];
    int height = *(int*)&header[0x16];
    int bpp = *(short*)&header[0x1C];

    if (bpp != 24) {
        printf("Only 24-bit BMP supported\n");
        fclose(file);
        return 0;
    }

    int row_padded = (width * 3 + 3) & (~3);
    int size = row_padded * height;

    unsigned char* data = new unsigned char[size];
    unsigned char* rgb = new unsigned char[width * height * 3];

    fseek(file, dataPos, SEEK_SET);
    fread(data, sizeof(unsigned char), size, file);
    fclose(file);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {

            int bmpIndex = i * row_padded + j * 3;
            int rgbIndex = (i * width + j) * 3;

            rgb[rgbIndex + 0] = data[bmpIndex + 2];
            rgb[rgbIndex + 1] = data[bmpIndex + 1];
            rgb[rgbIndex + 2] = data[bmpIndex + 0];
        }
    }

    delete[] data;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
        width, height, 0,
        GL_RGB, GL_UNSIGNED_BYTE, rgb);

    delete[] rgb;

    return tex;
}


// TEXT
void drawText(float x, float y, std::string text) {

    glRasterPos2f(x, y);

    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

// Shadow
void drawTextShadow(float x, float y, std::string text) {

    glColor3f(0, 0, 0);
    glRasterPos2f(x + 2, y - 2);
    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glColor3f(1, 1, 1);
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}


// Bold text
void drawTextBold(float x, float y, std::string text, float r, float g, float b) {

    glColor3f(0, 0, 0);
    glRasterPos2f(x + 1, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2f(x - 1, y);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2f(x, y + 1);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2f(x, y - 1);
    for (char c : text) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (char c : text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}


// BACKGROUND
void drawBackground() {

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture);

    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);

    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(WINDOW_W, 0);
    glTexCoord2f(1, 1); glVertex2f(WINDOW_W, WINDOW_H);
    glTexCoord2f(0, 1); glVertex2f(0, WINDOW_H);

    glEnd();

    glDisable(GL_TEXTURE_2D);
}


// PAUSE IMAGE DISPLAY — NEW
void drawPauseScreen() {

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pauseTexture);

    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(200, 150);
    glTexCoord2f(1, 0); glVertex2f(600, 150);
    glTexCoord2f(1, 1); glVertex2f(600, 450);
    glTexCoord2f(0, 1); glVertex2f(200, 450);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}


// PLAYER
void drawPlayer() {

    glPushMatrix();

    glTranslatef(playerX, playerY, 0);

    // Thobe
    glColor3f(1, 1, 1);

    glBegin(GL_QUADS);

    glVertex2f(-20, -40);
    glVertex2f(20, -40);
    glVertex2f(20, 20);
    glVertex2f(-20, 20);

    glEnd();

    // Left arm
    glBegin(GL_QUADS);

    glVertex2f(-35, -30);
    glVertex2f(-20, -30);
    glVertex2f(-20, 10);
    glVertex2f(-35, 10);

    glEnd();

    // Right arm
    glBegin(GL_QUADS);

    glVertex2f(20, -30);
    glVertex2f(35, -30);
    glVertex2f(35, 10);
    glVertex2f(20, 10);

    glEnd();

    // Pocket
    glColor3f(0.6, 0.6, 0.6);

    glBegin(GL_LINE_LOOP);

    glVertex2f(5, 0);
    glVertex2f(12, 0);
    glVertex2f(12, -10);
    glVertex2f(5, -10);

    glEnd();

    // Head
    glColor3f(1.0, 0.8, 0.6);

    glBegin(GL_POLYGON);

    for (int i = 0; i < 100; i++) {

        float angle = 2 * 3.14159f * i / 100;

        glVertex2f(
            cos(angle) * 15,
            sin(angle) * 15 + 35
        );
    }

    glEnd();

    // SHEMAGH
    glPushMatrix();

    glTranslatef(0, 10, 0);

    glColor3f(1, 0, 0);

    glBegin(GL_QUADS);
    glVertex2f(-18, 44);
    glVertex2f(18, 44);
    glVertex2f(18, 36);
    glVertex2f(-18, 36);
    glEnd();

    // left side
    glBegin(GL_QUADS);
    glVertex2f(-18, 36);
    glVertex2f(-28, 28);
    glVertex2f(-22, 5);
    glVertex2f(-12, 28);
    glEnd();

    // right side
    glBegin(GL_QUADS);
    glVertex2f(18, 36);
    glVertex2f(28, 28);
    glVertex2f(22, 5);
    glVertex2f(12, 28);
    glEnd();

    glPopMatrix();

    // Agal
    glColor3f(0, 0, 0);

    glBegin(GL_QUADS);

    glVertex2f(-12, 52);
    glVertex2f(12, 52);
    glVertex2f(12, 56);
    glVertex2f(-12, 56);

    glEnd();

    // Eyes
    glColor3f(0, 0, 0);

    glPointSize(4);

    glBegin(GL_POINTS);

    glVertex2f(-5, 38);
    glVertex2f(5, 38);

    glEnd();

    // Smile
    glBegin(GL_LINE_STRIP);

    glVertex2f(-6, 30);
    glVertex2f(0, 26);
    glVertex2f(6, 30);

    glEnd();

    glPopMatrix();
}


// STARS
void resetStar(int i) {

    starX[i] = rand() % 760 + 20;

    starY[i] = WINDOW_H + rand() % 300;


    // Slow speed
    starSpeed[i] = 0.2f + static_cast<float>(rand() % 100) / 400.0f;

    starRotate[i] = 0;
}

// Draw star
void drawSingleStar(float x, float y, float angle) {

    glPushMatrix();

    glTranslatef(x, y, 0);

    glRotatef(angle, 0, 0, 1);

    glColor3f(1.0f, 1.0f, 0.0f);

    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(0, 0);

    for (int i = 0; i <= 10; i++) {

        float radius = (i % 2 == 0) ? 25 : 12;

        float a = i * 3.14159f / 5;

        glVertex2f(
            cos(a) * radius,
            sin(a) * radius
        );
    }

    glEnd();

    glPopMatrix();
}

// Draw all stars
void drawStars() {

    for (int i = 0; i < MAX_STARS; i++) {

        drawSingleStar(
            starX[i],
            starY[i],
            starRotate[i]
        );
    }
}

// Update stars movement
void updateStars() {

    for (int i = 0; i < MAX_STARS; i++) {

        starY[i] -= starSpeed[i];

        starRotate[i] += 1;

        if (starY[i] < 0) {
            resetStar(i);
        }
    }
}


const int MAX_BAD_STARS = 2;

float badStarX[MAX_BAD_STARS];
float badStarY[MAX_BAD_STARS];
float badStarSpeed[MAX_BAD_STARS];
float badStarRotate[MAX_BAD_STARS];

void resetBadStar(int i) {

    badStarX[i] = rand() % 760 + 20;

    badStarY[i] = WINDOW_H + rand() % 300;

    // Slow speed
    badStarSpeed[i] = 0.3f + static_cast<float>(rand() % 100) / 300.0f;

    badStarRotate[i] = 0;
}

void drawBadStar(float x, float y, float angle) {

    glPushMatrix();

    glTranslatef(x, y, 0);

    glRotatef(angle, 0, 0, 1);

    // RED
    glColor3f(1.0f, 0.1f, 0.1f);

    glBegin(GL_TRIANGLE_FAN);

    glVertex2f(0, 0);

    for (int i = 0; i <= 10; i++) {

        float radius = (i % 2 == 0) ? 25 : 10;

        float a = i * 3.14159f / 5;

        glVertex2f(
            cos(a) * radius,
            sin(a) * radius
        );
    }

    glEnd();

    glPopMatrix();
}

void drawBadStars() {

    for (int i = 0; i < MAX_BAD_STARS; i++) {

        drawBadStar(
            badStarX[i],
            badStarY[i],
            badStarRotate[i]
        );
    }
}

void updateBadStars() {

    for (int i = 0; i < MAX_BAD_STARS; i++) {

        badStarY[i] -= badStarSpeed[i];

        badStarRotate[i] += 1;

        if (badStarY[i] < 0) {
            resetBadStar(i);
        }
    }
}

void checkBadCollision() {

    for (int i = 0; i < MAX_BAD_STARS; i++) {

        float dx = playerX - badStarX[i];
        float dy = playerY - badStarY[i];

        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 55) {

            // instant lose
            playerWon = false;

            loseReason = "RED STAR";

            playLoseSound();

            currentScene = GAME_OVER;

            return;
        }
    }
}


// COLLISION
void checkCollision() {

    for (int i = 0; i < MAX_STARS; i++) {

        float dx = playerX - starX[i];

        float dy = playerY - starY[i];

        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 55) {

            score += 10;
            resetStar(i);

            if (score >= 500) {

                playerWon = true;

                playWinSound();

                currentScene = GAME_OVER;
            }
        }
    }
}


// START SCREEN
void drawStartScreen() {

    drawBackground();

    drawTextBold(270, 435, "STAR CATCHER", 1.0f, 0.85f, 0.2f);

    drawTextShadow(190, 360, "Catch falling stars before time runs out");

    drawTextShadow(310, 310, "CONTROLS");

    drawTextShadow(240, 280, "Arrow Keys → Move");
    drawTextShadow(240, 250, "Mouse → Follow Cursor");
    drawTextShadow(240, 220, "Z / X → Zoom");
    drawTextShadow(240, 190, "P → Pause");

    drawTextBold(260, 150, "PRESS SPACE TO START", 0.2f, 1.0f, 0.4f);
}
void drawHowToPlayScreen() {

    drawBackground();

    drawTextBold(300, 430, "HOW TO PLAY", 1.0f, 0.85f, 0.2f);

    drawTextShadow(220, 360, "Catch falling yellow stars");

    drawTextShadow(220, 320, "Avoid red stars");

    drawTextShadow(220, 280, "Move using Arrow Keys");

    drawTextShadow(220, 240, "Reach 500 score to win");

    drawTextBold(240, 160, "PRESS SPACE TO START", 0.2f, 1.0f, 0.4f);
}

// END SCREEN
void drawEndScreen() {

    drawBackground();

    if (playerWon) {

        drawTextBold(260, 440, "MISSION COMPLETE", 0.3f, 1.0f, 0.3f);

        drawTextShadow(260, 380, "You caught enough stars!");

        glPushMatrix();
        glTranslatef(400, 250, 0);
        glScalef(2, 2, 1);
        drawPlayer();
        glPopMatrix();

        drawTextBold(300, 180, "STAR MASTER", 1.0f, 0.85f, 0.2f);
    }
    else {

        drawTextBold(280, 440, "MISSION FAILED", 1.0f, 0.3f, 0.3f);

        if (loseReason == "TIME UP") {

            drawTextShadow(240, 380, "Time ran out!");

            drawTextShadow(170, 340, "The stars disappeared into the night...");
        }

        else if (loseReason == "RED STAR") {

            drawTextShadow(230, 380, "You touched the forbidden star!");

            drawTextShadow(210, 340, "One mistake ended the mission...");
        }

        drawTextBold(290, 250, "TRY AGAIN", 1.0f, 0.6f, 0.6f);
    }

    drawTextShadow(260, 120, "PRESS SPACE TO RESTART");
}


void display() {

    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Zoom transformation (safe)
    glTranslatef(WINDOW_W / 2, WINDOW_H / 2, 0);
    glScalef(zoomLevel, zoomLevel, 1);
    glTranslatef(-WINDOW_W / 2, -WINDOW_H / 2, 0);

    if (currentScene == START_SCREEN) {

        drawStartScreen();
    }
    else if (currentScene == HOW_TO_PLAY) {

        drawHowToPlayScreen();
    }
    else if (currentScene == PLAYING) {

        drawBackground();
        drawStars();
        drawBadStars();
        drawPlayer();

        glColor3f(1, 1, 1);
        drawText(20, 560, "Score: " + std::to_string(score));
        drawText(20, 530, "Time: " + std::to_string(gameTime));

        if (isPaused) {
            drawPauseScreen();
        }
    }
    else if (currentScene == GAME_OVER) {

        drawEndScreen();
    }

    glutSwapBuffers();
}


// GAME LOOP
void gameLoop() {

    if (isPaused) {
        glutPostRedisplay();
        return;
    }

    if (currentScene == PLAYING) {

        int now = glutGet(GLUT_ELAPSED_TIME) / 1000;

        if (now != lastSecond) {

            lastSecond = now;

            gameTime--;

            if (gameTime <= 0) {

                gameTime = 0;

                playerWon = false;

                loseReason = "TIME UP";

                playLoseSound();

                currentScene = GAME_OVER;
            }
        }

        updateStars();
        updateBadStars();

        checkCollision();
        checkBadCollision();
    }

    glutPostRedisplay();
}


// INPUT
void handleInput(unsigned char key, int x, int y) {

    if (key == ' ') {

        if (currentScene == START_SCREEN) {
            currentScene = HOW_TO_PLAY;
        }

        else if (currentScene == HOW_TO_PLAY) {

            score = 0;
            gameTime = 60;
            playerWon = false;
            isPaused = false;

            playerX = 400;
            playerY = 80;

            for (int i = 0; i < MAX_STARS; i++) {
                resetStar(i);
            }

            for (int i = 0; i < MAX_BAD_STARS; i++) {
                resetBadStar(i);
            }

            lastSecond = glutGet(GLUT_ELAPSED_TIME) / 1000;

            currentScene = PLAYING;

            playMusic();
        }
        else if (currentScene == GAME_OVER) {

            score = 0;
            gameTime = 60;
            playerWon = false;
            isPaused = false;

            playerX = 400;
            playerY = 80;

            for (int i = 0; i < MAX_STARS; i++) resetStar(i);
            for (int i = 0; i < MAX_BAD_STARS; i++) resetBadStar(i);

            lastSecond = glutGet(GLUT_ELAPSED_TIME) / 1000;

            currentScene = PLAYING;

            playMusic();
        }
    }

    if (key == 27) {
        exit(0);
    }

    if (key == 'p' || key == 'P') {
        isPaused = !isPaused;
        return;
    }



    // zoom FIX
    if (key == 'z') {
        zoomLevel += 0.1f;
        if (zoomLevel > 2.0f) zoomLevel = 2.0f;
        glutPostRedisplay();
    }

    if (key == 'x') {
        zoomLevel -= 0.1f;
        if (zoomLevel < 0.5f) zoomLevel = 0.5f;
        glutPostRedisplay();
    }

}


void specialKeys(int key, int x, int y) {

    if (currentScene != PLAYING || isPaused)
        return;

    if (key == GLUT_KEY_LEFT)
        playerX -= playerSpeed;

    if (key == GLUT_KEY_RIGHT)
        playerX += playerSpeed;

    if (key == GLUT_KEY_UP)
        playerY += playerSpeed;

    if (key == GLUT_KEY_DOWN)
        playerY -= playerSpeed;
}


void mouseMove(int x, int y)
{
    if (currentScene != PLAYING || isPaused) return;

    playerX = x;
    playerY = WINDOW_H - y;

    glutPostRedisplay();
}


// RESHAPE
void reshape(int w, int h) {

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);

    glLoadIdentity();

    gluOrtho2D(0, WINDOW_W, 0, WINDOW_H);

    glMatrixMode(GL_MODELVIEW);
}


// INIT
void init() {

    playMusic();
    glClearColor(0, 0, 0, 1);

    glEnable(GL_TEXTURE_2D);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    srand(time(0));

    backgroundTexture = loadBMP("C:/Users/janab/Documents/CG Labs/lab6/background.bmp");

    // NEW — Load Pause image
    pauseTexture = loadBMP("C:/Users/janab/Documents/CG Labs/lab6/pp (1).bmp");

    for (int i = 0; i < MAX_STARS; i++) {
        resetStar(i);
    }

    for (int i = 0; i < MAX_BAD_STARS; i++) {
        resetBadStar(i);
    }
}


// MAIN
int main(int argc, char** argv) {

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    glutInitWindowSize(WINDOW_W, WINDOW_H);

    glutCreateWindow("Star Catcher");

    init();
    glutPassiveMotionFunc(mouseMove);
    glutMotionFunc(mouseMove);

    glutDisplayFunc(display);

    glutIdleFunc(gameLoop);

    glutKeyboardFunc(handleInput);

    glutSpecialFunc(specialKeys);

    glutReshapeFunc(reshape);

    glutMainLoop();

    return 0;
}
#include "game.hpp"
#include <GL/glut.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>
#include <algorithm>
#include <iostream>

Game game;
auto game_start_time = std::chrono::steady_clock::now();
bool game_initialized = false;
bool game_paused = false;

// Estados do jogo
enum GameState {
    MENU_MAIN,
    GAME_PLAYING,
    GAME_PAUSED
};

GameState current_state = MENU_MAIN;
int menu_selection = 0; // 0 = Jogar, 1 = Sair
int pause_selection = 0; // 0 = Continuar, 1 = Reiniciar, 2 = Sair

// Declarações de funções
void init(void);
void drawBoard(void);
void drawGame(void); // Nova função
void drawNextPiecePanel();
void drawHoldPanel();
void drawStatsPanel();
void drawAchievementNotifications();
void drawComboEffects();
void drawRecyclingAnimation();
void drawTexturedBlock(float x, float y, TrashType type, float alpha = 1.0f, bool glow = false);
void drawRecycleBin(float x, float y, float r, float g, float b, float scale = 1.0f, bool animated = false);
void drawParticles();
void renderText(float x, float y, const std::string &text, void *font = GLUT_BITMAP_HELVETICA_12);
void transform(int key, int x, int y);
void options(unsigned char key, int x, int y);
void timer(int id);
void reshape(int width, int height);
void drawMainMenu();
void drawPauseMenu();
void drawControlsPanel();

void init(void)
{
    glClearColor(0.05, 0.05, 0.1, 0.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Game::loadTextures();

}

// Função para renderizar texto melhorada
void renderText(float x, float y, const std::string &text, void *font)
{
    glRasterPos2f(x, y);
    for (char c : text)
    {
        glutBitmapCharacter(font, c);
    }
}

// Função para desenhar um bloco com textura e efeitos
void drawTexturedBlock(float x, float y, TrashType type, float alpha, bool glow)
{
    Game::bindTexture(type);

    if (glow)
    {
        // Efeito de brilho
        static float glow_time = 0.0f;
        glow_time += 0.1f;
        float glow_intensity = 0.8f + 0.2f * sin(glow_time);
        glColor4f(glow_intensity, glow_intensity, glow_intensity, alpha);
    }
    else
    {
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
    }

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(x + 1, y);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(x + 1, y + 1);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(x, y + 1);
    glEnd();

    // Borda com efeito de profundidade
    glDisable(GL_TEXTURE_2D);

    // Borda clara (topo e esquerda)
    glColor4f(1.0f, 1.0f, 1.0f, alpha * 0.6f);
    glBegin(GL_LINES);
    glVertex2f(x, y + 1);
    glVertex2f(x + 1, y + 1);
    glVertex2f(x, y);
    glVertex2f(x, y + 1);
    glEnd();

    // Borda escura (baixo e direita)
    glColor4f(0.2f, 0.2f, 0.2f, alpha * 0.8f);
    glBegin(GL_LINES);
    glVertex2f(x + 1, y + 1);
    glVertex2f(x + 1, y);
    glVertex2f(x + 1, y);
    glVertex2f(x, y);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

// Função para desenhar lixeira melhorada com animações
void drawRecycleBin(float x, float y, float r, float g, float b, float scale, bool animated)
{
    glDisable(GL_TEXTURE_2D);

    if (animated)
    {
        static float bounce_time = 0.0f;
        bounce_time += 0.2f;
        scale *= 1.0f + 0.1f * sin(bounce_time);
        y += 0.2f * sin(bounce_time * 2);
    }

    // Sombra projetada
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
    glBegin(GL_POLYGON); 
    for (int i = 0; i < 20; i++)
    {
        float angle = i * 2.0f * 3.14159f / 20.0f;
        glVertex2f(x + 0.8f * scale * cos(angle), y - 0.2f + 0.1f * sin(angle));
    }
    glEnd();

    // Corpo principal com gradiente
    for (int i = 0; i < 10; i++)
    {
        float ratio = (float)i / 10.0f;
        float dark_factor = 1.0f - ratio * 0.3f;
        glColor3f(r * dark_factor, g * dark_factor, b * dark_factor);

        float bottom_width = 0.8f * scale;
        float top_width = 0.6f * scale;
        float segment_height = 1.5f * scale / 10.0f;

        glBegin(GL_QUADS);
        glVertex2f(x - bottom_width + ratio * (bottom_width - top_width), y + i * segment_height);
        glVertex2f(x + bottom_width - ratio * (bottom_width - top_width), y + i * segment_height);
        glVertex2f(x + bottom_width - (ratio + 0.1f) * (bottom_width - top_width), y + (i + 1) * segment_height);
        glVertex2f(x - bottom_width + (ratio + 0.1f) * (bottom_width - top_width), y + (i + 1) * segment_height);
        glEnd();
    }

    // Tampa com brilho
    glColor3f(r * 0.7f + 0.3f, g * 0.7f + 0.3f, b * 0.7f + 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(x - 0.9f * scale, y + 1.5f * scale);
    glVertex2f(x + 0.9f * scale, y + 1.5f * scale);
    glVertex2f(x + 0.8f * scale, y + 1.8f * scale);
    glVertex2f(x - 0.8f * scale, y + 1.8f * scale);
    glEnd();

    // Símbolo de reciclagem animado
    static float symbol_rotation = 0.0f;
    if (animated)
    {
        symbol_rotation += 2.0f;
    }

    glPushMatrix();
    glTranslatef(x, y + 0.9f * scale, 0);
    glRotatef(symbol_rotation, 0, 0, 1);

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 3; i++)
    {
        float angle = i * 120.0f * 3.14159f / 180.0f;
        float size = 0.25f * scale;
        glVertex2f(size * cos(angle), size * sin(angle));
        glVertex2f(size * 0.5f * cos(angle + 0.5f), size * 0.5f * sin(angle + 0.5f));
        glVertex2f(size * 0.5f * cos(angle - 0.5f), size * 0.5f * sin(angle - 0.5f));
    }
    glEnd();

    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
}

// Função para desenhar partículas melhoradas
void drawParticles()
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_POINT_SMOOTH);

    for (const auto &p : game.getParticles())
    {
        float r = game.getRGB(static_cast<Color>(p.type), 0);
        float g = game.getRGB(static_cast<Color>(p.type), 1);
        float b = game.getRGB(static_cast<Color>(p.type), 2);

        // Tamanho baseado na vida da partícula
        glPointSize(p.size * 10.0f * p.life);

        // Efeito de fade out
        glColor4f(r, g, b, p.life * 0.8f);

        glBegin(GL_POINTS);
        glVertex2f(p.x, p.y);
        glEnd();

        // Rastro da partícula
        glColor4f(r, g, b, p.life * 0.3f);
        glBegin(GL_LINES);
        glVertex2f(p.x, p.y);
        glVertex2f(p.x - p.vx * 0.5f, p.y - p.vy * 0.5f);
        glEnd();
    }

    glDisable(GL_POINT_SMOOTH);
    glEnable(GL_TEXTURE_2D);
}

// Nova função para desenhar apenas o jogo (sem gerenciar estados)
void drawGame()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, 1000, 700);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 25, 0, 20);

    // Desenhar fundo do jogo
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.04f, 0.08f);
    glVertex2f(0, 20);
    glVertex2f(25, 20);
    glColor3f(0.01f, 0.02f, 0.04f);
    glVertex2f(25, 0);
    glVertex2f(0, 0);
    glEnd();
    glEnable(GL_TEXTURE_2D);

    glColor3f(1.0f, 1.0f, 1.0f);

    // Desenhar tabuleiro principal
    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            if (game.isLineClearing() && y == game.getLineBeingCleared())
            {
                if (x >= game.getAnimationStep())
                {
                    TrashType type = game.getTrashType(x, y);
                    if (type != NONE)
                    {
                        drawTexturedBlock(x, y, type, 1.0f, true); 
                    }
                }
            }
            else if (game.getOccupied(x, y) ^ game.getCurrent(x, y))
            {
                TrashType type = game.getTrashType(x, y);
                if (type != NONE)
                {
                    bool is_current = game.getCurrent(x, y);
                    drawTexturedBlock(x, y, type, is_current ? 0.9f : 1.0f, is_current);
                }
            }
        }
    }

    // Desenhar células vazias
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.01f, 0.01f, 0.03f);
    for (int y = 0; y < 20; y++)
    {
        for (int x = 0; x < 10; x++)
        {
            if (!game.getOccupied(x, y) && !game.getCurrent(x, y))
            {
                glRectf(x, y, x + 1, y + 1);
            }
        }
    }

    // Grade opcional
    glColor4f(0.15f, 0.15f, 0.25f, 0.6f);
    glBegin(GL_LINES);
    for (int i = 0; i <= 10; i++)
    {
        glVertex2f(i, 0.0f);
        glVertex2f(i, 20.0f);
    }
    for (int i = 0; i <= 20; i++)
    {
        glVertex2f(0.0f, i);
        glVertex2f(10.0f, i);
    }
    glEnd();

    // Borda principal com efeito de neon
    glColor3f(0.0f, 1.0f, 0.5f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-0.1f, -0.1f);
    glVertex2f(10.1f, -0.1f);
    glVertex2f(10.1f, 20.1f);
    glVertex2f(-0.1f, 20.1f);
    glEnd();
    glLineWidth(1.0f);

    glEnable(GL_TEXTURE_2D);

    // UI lateral melhorada
    drawNextPiecePanel();
    drawHoldPanel();
    drawStatsPanel();
    drawControlsPanel();
    drawAchievementNotifications();

    // Efeitos especiais (só se não pausado)
    if (current_state == GAME_PLAYING)
    {
        drawParticles();
        drawComboEffects();

        // Animação de reciclagem
        if (game.isLineClearing())
        {
            drawRecyclingAnimation();
        }
    }
}

// Função principal de renderização corrigida
void drawBoard(void)
{
    switch (current_state)
    {
        case MENU_MAIN:
            drawMainMenu();
            break;
            
        case GAME_PLAYING:
            drawGame();
            glutSwapBuffers();
            break;
            
        case GAME_PAUSED:
            drawGame();
            
            // Desenhar menu de pausa sobre o jogo
            drawPauseMenu();
            glutSwapBuffers();
            break;
    }
}

// Função para desenhar o menu principal
void drawMainMenu()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    
    // Fundo do menu
    glBegin(GL_QUADS);
    glColor3f(0.02f, 0.04f, 0.08f);
    glVertex2f(0, 20);
    glVertex2f(25, 20);
    glColor3f(0.01f, 0.02f, 0.04f);
    glVertex2f(25, 0);
    glVertex2f(0, 0);
    glEnd();
    
    // Título do jogo
    glColor3f(0.0f, 1.0f, 0.5f);
    renderText(6.0f, 16.0f, "ECOTETRIS", GLUT_BITMAP_TIMES_ROMAN_24);
    glColor3f(0.0f, 0.8f, 0.4f);
    renderText(7.5f, 15.0f, "Reciclagem Sustentavel", GLUT_BITMAP_HELVETICA_18);
    
    // Opções do menu
    float menu_y = 12.0f;
    
    // Opção Jogar
    if (menu_selection == 0)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        renderText(10.0f, menu_y, "> JOGAR <", GLUT_BITMAP_HELVETICA_18);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(11.0f, menu_y, "JOGAR", GLUT_BITMAP_HELVETICA_18);
    }
    
    menu_y -= 2.0f;
    
    // Opção Sair
    if (menu_selection == 1)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        renderText(10.5f, menu_y, "> SAIR <", GLUT_BITMAP_HELVETICA_18);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(11.5f, menu_y, "SAIR", GLUT_BITMAP_HELVETICA_18);
    }
    
    // Instruções
    glColor3f(0.7f, 0.7f, 0.7f);
    renderText(7.0f, 7.0f, "Use as setas para navegar", GLUT_BITMAP_HELVETICA_12);
    renderText(8.0f, 6.0f, "ENTER para selecionar", GLUT_BITMAP_HELVETICA_12);
    
    // Créditos/Tema
    glColor3f(0.0f, 0.6f, 0.3f);
    renderText(4.0f, 3.0f, "Ajude o planeta separando o lixo corretamente!", GLUT_BITMAP_HELVETICA_12);
    renderText(6.0f, 2.0f, "Cada tipo de lixo tem sua cor especial", GLUT_BITMAP_HELVETICA_10);
    
    glutSwapBuffers();
}

// Função para desenhar o menu de pausa
void drawPauseMenu()
{
    // Escurecer tela
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(25, 0);
    glVertex2f(25, 20);
    glVertex2f(0, 20);
    glEnd();
    
    // Painel do menu
    glColor4f(0.1f, 0.1f, 0.2f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(8.0f, 8.0f);
    glVertex2f(17.0f, 8.0f);
    glVertex2f(17.0f, 15.0f);
    glVertex2f(8.0f, 15.0f);
    glEnd();
    
    // Borda do painel
    glColor3f(0.0f, 1.0f, 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(8.0f, 8.0f);
    glVertex2f(17.0f, 8.0f);
    glVertex2f(17.0f, 15.0f);
    glVertex2f(8.0f, 15.0f);
    glEnd();
    glLineWidth(1.0f);
    
    // Título
    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(10.5f, 14.0f, "PAUSADO", GLUT_BITMAP_HELVETICA_18);
    
    // Opções do menu de pausa
    float pause_y = 12.5f;
    
    // Continuar
    if (pause_selection == 0)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        renderText(9.0f, pause_y, "> CONTINUAR <", GLUT_BITMAP_HELVETICA_12);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(10.0f, pause_y, "CONTINUAR", GLUT_BITMAP_HELVETICA_12);
    }
    
    pause_y -= 1.0f;
    
    // Reiniciar
    if (pause_selection == 1)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        renderText(9.0f, pause_y, "> REINICIAR <", GLUT_BITMAP_HELVETICA_12);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(10.0f, pause_y, "REINICIAR", GLUT_BITMAP_HELVETICA_12);
    }
    
    pause_y -= 1.0f;
    
    // Sair
    if (pause_selection == 2)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        renderText(9.5f, pause_y, "> SAIR <", GLUT_BITMAP_HELVETICA_12);
    }
    else
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(10.5f, pause_y, "SAIR", GLUT_BITMAP_HELVETICA_12);
    }
    
    // Instruções
    glColor3f(0.7f, 0.7f, 0.7f);
    renderText(9.0f, 9.0f, "Setas + ENTER", GLUT_BITMAP_8_BY_13);
}

// Função para desenhar controles na tela
void drawControlsPanel()
{
    glDisable(GL_TEXTURE_2D);
    
    // Painel de controles (canto inferior direito)
    glBegin(GL_QUADS);
    glColor4f(0.05f, 0.08f, 0.12f, 0.9f);
    glVertex2f(18.5f, 0.5f);
    glVertex2f(24.5f, 0.5f);
    glColor4f(0.02f, 0.04f, 0.08f, 0.9f);
    glVertex2f(24.5f, 7.5f);
    glVertex2f(18.5f, 7.5f);
    glEnd();
    
    // Borda
    glColor3f(0.0f, 0.6f, 0.3f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(18.5f, 0.5f);
    glVertex2f(24.5f, 0.5f);
    glVertex2f(24.5f, 7.5f);
    glVertex2f(18.5f, 7.5f);
    glEnd();
    
    // Título
    glColor3f(0.0f, 1.0f, 0.5f);
    renderText(19.0f, 7.0f, "CONTROLES", GLUT_BITMAP_HELVETICA_12);
    
    glColor3f(1.0f, 1.0f, 1.0f);
    float y = 6.3f;
    
    renderText(19.0f, y, "(up) - Rotacionar", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "<- -> - Mover", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "(down) - Acelerar", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "Espaco - Drop", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "C - Hold", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "ESC - Pausar", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "R - Reiniciar", GLUT_BITMAP_8_BY_13);
    y -= 0.4f;
    renderText(19.0f, y, "Q - Sair", GLUT_BITMAP_8_BY_13);
    
    glEnable(GL_TEXTURE_2D);
}
void drawNextPiecePanel()
{
    glDisable(GL_TEXTURE_2D);

    // Painel com gradiente
    glBegin(GL_QUADS);
    glColor4f(0.1f, 0.15f, 0.2f, 0.9f);
    glVertex2f(12.0f, 16.0f);
    glVertex2f(17.0f, 16.0f);
    glColor4f(0.05f, 0.1f, 0.15f, 0.9f);
    glVertex2f(17.0f, 19.5f);
    glVertex2f(12.0f, 19.5f);
    glEnd();

    // Borda com efeito
    glColor3f(0.0f, 0.8f, 0.4f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(12.0f, 16.0f);
    glVertex2f(17.0f, 16.0f);
    glVertex2f(17.0f, 19.5f);
    glVertex2f(12.0f, 19.5f);
    glEnd();
    glLineWidth(1.0f);

    // Título
    glColor3f(1.0f, 1.0f,1.0f);
    renderText(13.0f, 19.0f, "PROXIMA", GLUT_BITMAP_HELVETICA_12);

    glEnable(GL_TEXTURE_2D);

    // Peça
    int shape = game.getNextShape();
    TrashType *types = game.getNextTrashTypes();

    float offset_x = 14.0f;
    float offset_y = 17.5f;
    float piece_scale = 0.6f;

    drawTexturedBlock(offset_x, offset_y, types[0], 0.9f);

    for (int i = 1; i < 6; i += 2)
    {
        float x = offset_x + shapes[shape][0][i - 1] * piece_scale;
        float y = offset_y + shapes[shape][0][i] * piece_scale;
        drawTexturedBlock(x, y, types[i / 2 + 1], 0.9f);
    }
}

void drawHoldPanel()
{
    glDisable(GL_TEXTURE_2D);

    // Painel
    bool can_hold = game.canHold();
    float alpha = can_hold ? 0.9f : 0.5f;

    glBegin(GL_QUADS);
    glColor4f(0.1f, 0.15f, 0.2f, alpha);
    glVertex2f(12.0f, 12.5f);
    glVertex2f(17.0f, 12.5f);
    glColor4f(0.05f, 0.1f, 0.15f, alpha);
    glVertex2f(17.0f, 15.5f);
    glVertex2f(12.0f, 15.5f);
    glEnd();

    // Borda
    glColor4f(can_hold ? 0.0f : 0.5f, can_hold ? 0.8f : 0.3f, can_hold ? 0.4f : 0.2f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(12.0f, 12.5f);
    glVertex2f(17.0f, 12.5f);
    glVertex2f(17.0f, 15.5f);
    glVertex2f(12.0f, 15.5f);
    glEnd();
    glLineWidth(1.0f);

    // Título
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    renderText(13.8f, 15.0f, "HOLD", GLUT_BITMAP_HELVETICA_12);
    renderText(12.2f, 14.5f, "Pressione C", GLUT_BITMAP_8_BY_13);

    glEnable(GL_TEXTURE_2D);

    // Peça guardada
    int hold_shape = game.getHoldShape();
    if (hold_shape != -1)
    {
        TrashType *types = game.getHoldTrashTypes();

        float offset_x = 14.0f;
        float offset_y = 13.5f;
        float piece_scale = 0.6f;

        drawTexturedBlock(offset_x, offset_y, types[0], alpha);

        for (int i = 1; i < 6; i += 2)
        {
            float x = offset_x + shapes[hold_shape][0][i - 1] * piece_scale;
            float y = offset_y + shapes[hold_shape][0][i] * piece_scale;
            drawTexturedBlock(x, y, types[i / 2 + 1], alpha);
        }
    }
}

void drawStatsPanel()
{
    glDisable(GL_TEXTURE_2D);

    // Painel principal
    glBegin(GL_QUADS);
    glColor4f(0.05f, 0.08f, 0.12f, 0.95f);
    glVertex2f(11.5f, 0.5f);
    glVertex2f(24.0f, 0.5f);
    glColor4f(0.02f, 0.04f, 0.08f, 0.95f);
    glVertex2f(24.0f, 12.0f);
    glVertex2f(11.5f, 12.0f);
    glEnd();

    // Borda
    glColor3f(0.0f, 0.6f, 0.3f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(11.5f, 0.5f);
    glVertex2f(24.0f, 0.5f);
    glVertex2f(24.0f, 12.0f);
    glVertex2f(11.5f, 12.0f);
    glEnd();
    glLineWidth(1.0f);

    // Título
    glColor3f(0.0f, 1.0f, 0.5f);
    renderText(16.0f, 11.5f, "ESTATISTICAS", GLUT_BITMAP_HELVETICA_18);

    glColor3f(1.0f, 1.0f, 1.0f);

    float y = 10.8f;
    std::stringstream ss;

    // Pontuação com animação
    static int displayed_score = 0;
    int target_score = game.getScore();
    if (displayed_score < target_score)
    {
        displayed_score += std::max(1, (target_score - displayed_score) / 10);
    }

    ss << "PONTOS: " << displayed_score;
    renderText(12.0f, y, ss.str(), GLUT_BITMAP_HELVETICA_12);
    y -= 0.6f;

    // Nível com barra de progresso
    ss.str("");
    ss << "NIVEL: " << game.getLevel();
    renderText(12.0f, y, ss.str());

    // Barra de progresso para próximo nível
    int lines_for_next = ((game.getLevel()) * 10) - game.getLinesCleared();
    float progress = 1.0f - (float)lines_for_next / 10.0f;

    glColor3f(0.2f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
    glVertex2f(18.0f, y - 0.1f);
    glVertex2f(23.0f, y - 0.1f);
    glVertex2f(23.0f, y + 0.3f);
    glVertex2f(18.0f, y + 0.3f);
    glEnd();

    glColor3f(0.0f, 1.0f, 0.5f);
    glBegin(GL_QUADS);
    glVertex2f(18.0f, y - 0.1f);
    glVertex2f(18.0f + 5.0f * progress, y - 0.1f);
    glVertex2f(18.0f + 5.0f * progress, y + 0.3f);
    glVertex2f(18.0f, y + 0.3f);
    glEnd();

    y -= 0.8f;
    glColor3f(1.0f, 1.0f, 1.0f);

    // Outras estatísticas
    ss.str("");
    ss << "LINHAS: " << game.getLinesCleared();
    renderText(12.0f, y, ss.str());
    y -= 0.5f;

    if (game.getComboCount() > 0)
    {
        glColor3f(1.0f, 1.0f, 0.0f);
        ss.str("");
        ss << "COMBO: " << game.getComboCount() << "x";
        renderText(12.0f, y, ss.str());
        y -= 0.5f;
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    // Título reciclagem
    y -= 0.3f;
    glColor3f(0.0f, 0.8f, 0.4f);
    renderText(12.0f, y, "RECICLADOS:", GLUT_BITMAP_HELVETICA_10);
    y -= 0.5f;

    // Estatísticas por tipo de lixo com ícones coloridos
    const char *trash_names[] = {"Papel", "Plastico", "Metal", "Vidro", "Organico"};
    const float trash_colors[][3] = {
        {0.3f, 0.5f, 1.0f}, // Azul
        {1.0f, 0.3f, 0.3f}, // Vermelho
        {1.0f, 0.8f, 0.2f}, // Amarelo
        {0.3f, 1.0f, 0.3f}, // Verde
        {0.8f, 0.5f, 0.2f}  // Marrom
    };

    for (int i = 0; i < 5; i++)
    {
        // Ícone colorido
        glColor3fv(trash_colors[i]);
        glBegin(GL_QUADS);
        glVertex2f(12.0f, y);
        glVertex2f(12.3f, y);
        glVertex2f(12.3f, y + 0.3f);
        glVertex2f(12.0f, y + 0.3f);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        ss.str("");
        ss << trash_names[i] << ": " << game.getRecycledCount(static_cast<TrashType>(i));
        renderText(12.5f, y, ss.str(), GLUT_BITMAP_8_BY_13);
        y -= 0.4f;
    }

    glEnable(GL_TEXTURE_2D);
}

void drawAchievementNotifications()
{
    // Implementação simplificada para evitar dependências
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    renderText(18.0f, 8.0f, "Conquistas: OK", GLUT_BITMAP_8_BY_13);
    glEnable(GL_TEXTURE_2D);
}

void drawComboEffects()
{
    if (game.getComboCount() > 1)
    {
        glDisable(GL_TEXTURE_2D);

        static float combo_glow = 0.0f;
        combo_glow += 0.1f;

        // Efeito de brilho ao redor do tabuleiro
        float glow_intensity = 0.5f + 0.3f * sin(combo_glow);
        glColor4f(1.0f, 1.0f, 0.0f, glow_intensity * 0.5f);

        glLineWidth(5.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.5f, -0.5f);
        glVertex2f(10.5f, -0.5f);
        glVertex2f(10.5f, 20.5f);
        glVertex2f(-0.5f, 20.5f);
        glEnd();
        glLineWidth(1.0f);

        // Texto de combo flutuante
        char combo_text[32];
        sprintf(combo_text, "COMBO x%d!", game.getComboCount());

        float text_y = 15.0f + 2.0f * sin(combo_glow * 0.5f);
        glColor4f(1.0f, 1.0f, 0.0f, glow_intensity);
        renderText(3.0f, text_y, combo_text, GLUT_BITMAP_HELVETICA_18);

        glEnable(GL_TEXTURE_2D);
    }
}

void drawRecyclingAnimation()
{
    TrashType type = game.getLineTrashType();
    float r, g, b;

    switch (type)
    {
    case PAPER:
        r = 0.3f;
        g = 0.5f;
        b = 1.0f;
        break;
    case PLASTIC:
        r = 1.0f;
        g = 0.3f;
        b = 0.3f;
        break;
    case METAL:
        r = 1.0f;
        g = 0.8f;
        b = 0.2f;
        break;
    case GLASS:
        r = 0.3f;
        g = 1.0f;
        b = 0.3f;
        break;
    case ORGANIC:
        r = 0.8f;
        g = 0.5f;
        b = 0.2f;
        break;
    }

    // Lixeira animada maior
    float scale = 1.5f + 0.2f * sin(game.getAnimationStep() * 0.8f);
    drawRecycleBin(20.0f, 10.0f, r, g, b, scale, true);

    // Trilha de partículas do bloco até a lixeira
    if (game.getAnimationStep() > 0 && game.getAnimationStep() <= 10)
    {
        float progress = (float)game.getAnimationStep() / 10.0f;

        // Bloco principal caindo
        float start_x = game.getLineBeingCleared() >= 0 ? 5.0f : 5.0f;
        float start_y = game.getLineBeingCleared();
        float end_x = 19.5f;
        float end_y = 11.0f;

        float current_x = start_x + (end_x - start_x) * progress;
        float current_y = start_y + (end_y - start_y) * progress;

        // Efeito de rastro
        glDisable(GL_TEXTURE_2D);
        glColor4f(r, g, b, 0.6f);
        glPointSize(8.0f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 10; i++)
        {
            float trail_progress = progress - i * 0.05f;
            if (trail_progress > 0)
            {
                float trail_x = start_x + (end_x - start_x) * trail_progress;
                float trail_y = start_y + (end_y - start_y) * trail_progress;
                glColor4f(r, g, b, 0.6f * trail_progress);
                glVertex2f(trail_x, trail_y);
            }
        }
        glEnd();
        glEnable(GL_TEXTURE_2D);

        // Bloco com rotação
        glPushMatrix();
        glTranslatef(current_x + 0.5f, current_y + 0.5f, 0);
        glRotatef(progress * 360.0f, 0, 0, 1);
        glTranslatef(-0.5f, -0.5f, 0);

        float alpha = 1.0f - progress * 0.3f;
        drawTexturedBlock(0, 0, type, alpha, true);

        glPopMatrix();

        // Efeito de impacto na lixeira
        if (progress > 0.8f)
        {
            glDisable(GL_TEXTURE_2D);
            float impact_intensity = (progress - 0.8f) * 5.0f;
            glColor4f(1.0f, 1.0f, 1.0f, impact_intensity * 0.8f);

            glPointSize(20.0f);
            glBegin(GL_POINTS);
            glVertex2f(end_x, end_y);
            glEnd();

            // Raios de impacto
            glBegin(GL_LINES);
            for (int i = 0; i < 8; i++)
            {
                float angle = i * 45.0f * 3.14159f / 180.0f;
                float ray_length = impact_intensity * 2.0f;
                glVertex2f(end_x, end_y);
                glVertex2f(end_x + cos(angle) * ray_length, end_y + sin(angle) * ray_length);
            }
            glEnd();
            glEnable(GL_TEXTURE_2D);
        }
    }

    // Texto informativo
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);
    std::string type_name = game.getTrashTypeName(type);
    renderText(18.0f, 8.0f, type_name + " Reciclado!", GLUT_BITMAP_HELVETICA_12);
    glEnable(GL_TEXTURE_2D);
}

// Controles de teclado atualizados
void transform(int key, int x, int y)
{
    switch (current_state)
    {
        case MENU_MAIN:
            switch (key)
            {
                case GLUT_KEY_UP:
                    menu_selection = (menu_selection - 1 + 2) % 2;
                    glutPostRedisplay();
                    break;
                case GLUT_KEY_DOWN:
                    menu_selection = (menu_selection + 1) % 2;
                    glutPostRedisplay();
                    break;
            }
            break;
            
        case GAME_PLAYING:
            if (game.getGameOver())
                return;

            switch (key)
            {
            case GLUT_KEY_UP:
                game.rotate();
                glutPostRedisplay();
                break;
            case GLUT_KEY_LEFT:
                game.translate(-1);
                glutPostRedisplay();
                break;
            case GLUT_KEY_RIGHT:
                game.translate(1);
                glutPostRedisplay();
                break;
            case GLUT_KEY_DOWN:
                game.moveDown();
                glutPostRedisplay();
                break;
            }
            break;
            
        case GAME_PAUSED:
            switch (key)
            {
                case GLUT_KEY_UP:
                    pause_selection = (pause_selection - 1 + 3) % 3;
                    glutPostRedisplay();
                    break;
                case GLUT_KEY_DOWN:
                    pause_selection = (pause_selection + 1) % 3;
                    glutPostRedisplay();
                    break;
            }
            break;
    }
}

void options(unsigned char key, int x, int y)
{
    switch (current_state)
    {
        case MENU_MAIN:
            switch (key)
            {
                case '\r': // ENTER
                case '\n':
                    if (menu_selection == 0) // Jogar
                    {
                        current_state = GAME_PLAYING;
                        game.restart();
                        game_start_time = std::chrono::steady_clock::now();
                        game_initialized = true;
                    }
                    else if (menu_selection == 1) // Sair
                    {
                        exit(0);
                    }
                    glutPostRedisplay();
                    break;
                case 27: // ESC
                    exit(0);
                    break;
            }
            break;
            
        case GAME_PLAYING:
            switch (key)
            {
            case 'q':
            case 'Q':
                current_state = MENU_MAIN;
                menu_selection = 0;
                glutPostRedisplay();
                break;

            case 'r':
            case 'R':
                game.restart();
                game_start_time = std::chrono::steady_clock::now();
                glutPostRedisplay();
                break;

            case 'c':
            case 'C':
                if (!game.getGameOver())
                {
                    game.holdPiece();
                    glutPostRedisplay();
                }
                break;

            case ' ': // Barra de espaço para drop rápido
                if (!game.getGameOver())
                {
                    while (!game.getGameOver() && !game.isLineClearing())
                    {
                        if (game.checkCollision(game.getCurrentX(), game.getCurrentY() - 1, game.getCurrentRotation()))
                        {
                            break;
                        }
                        game.moveDown();
                    }
                    glutPostRedisplay();
                }
                break;

            case 27: // ESC - Pausar
                if (!game.getGameOver())
                {
                    current_state = GAME_PAUSED;
                    pause_selection = 0;
                    glutPostRedisplay();
                }
                break;
            }
            break;
            
        case GAME_PAUSED:
            switch (key)
            {
                case '\r': // ENTER
                case '\n':
                    switch (pause_selection)
                    {
                        case 0: // Continuar
                            current_state = GAME_PLAYING;
                            break;
                        case 1: // Reiniciar
                            current_state = GAME_PLAYING;
                            game.restart();
                            game_start_time = std::chrono::steady_clock::now();
                            break;
                        case 2: // Sair
                            current_state = MENU_MAIN;
                            menu_selection = 0;
                            break;
                    }
                    glutPostRedisplay();
                    break;
                case 27: // ESC - Voltar ao jogo
                    current_state = GAME_PLAYING;
                    glutPostRedisplay();
                    break;
            }
            break;
    }
}

// Timer atualizado
void timer(int id)
{
    // Só executar lógica do jogo se estiver jogando
    if (current_state == GAME_PLAYING)
    {
        static auto last_time = std::chrono::steady_clock::now();
        auto current_time = std::chrono::steady_clock::now();
        auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time);

        game.update();

        if (game_initialized)
        {
            if (game.getGameOver())
            {
                game_initialized = false;
            }
            else
            {
                // Movimento automático baseado no nível
                static int drop_counter = 0;
                int drop_interval = (int)(30 * game.getDifficultyMultiplier());

                drop_counter++;
                if (drop_counter >= drop_interval)
                {
                    game.moveDown();
                    drop_counter = 0;
                }
            }
        }

        last_time = current_time;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, id + 1);
}

// Função para redimensionamento da janela
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 25, 0, 20);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA);
    glutInitWindowPosition(100, 50);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("EcoTetris - Reciclagem Sustentavel");

    init();

    glutDisplayFunc(drawBoard);
    glutSpecialFunc(transform);
    glutKeyboardFunc(options);
    glutReshapeFunc(reshape);
    glutTimerFunc(16, timer, 0);

    // Mensagem de boas-vindas atualizada
    std::cout << "=== EcoTetris - Reciclagem Sustentavel ===" << std::endl;
    std::cout << "Bem-vindo ao EcoTetris!" << std::endl;
    std::cout << "Use o menu para navegar pelas opções." << std::endl;
    std::cout << "Os controles aparecem na tela durante o jogo." << std::endl;
    std::cout << "=============================================" << std::endl;

    glutMainLoop();

    return 0;
}

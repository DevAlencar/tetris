#ifndef GAME_HPP
#define GAME_HPP

#include <vector>
#include <GL/glut.h>
#include <string>

//////////////////////////////////////////////
// Classe Space - Representa uma célula do tabuleiro
//////////////////////////////////////////////

// Enumeração dos tipos de lixo para reciclagem
enum TrashType {PAPER, PLASTIC, METAL, GLASS, ORGANIC, NONE};

class Space{
    public:
        bool isOccupied; // True se ocupado por uma peça congelada
        bool isCurrent;  // True se parte da peça em movimento
        TrashType trash_type; // Tipo de lixo para determinar a textura
        float red;       // Valores RGB para colorir a célula (backup)
        float green;
        float blue;
        int combo_id = -1; // ID para identificar combos
};

// Enumeração das cores disponíveis para as peças (agora representam tipos de lixo)
enum Color {paper, plastic, metal, glass, organic};

// Definição de todas as peças (tetrominos) e suas rotações
extern const int shapes[7][4][6];

//////////////////////////////////////////////
// Struct para efeitos de partículas
//////////////////////////////////////////////
struct Particle {
    float x, y;
    float vx, vy;
    float life;
    float size;
    TrashType type;
};

//////////////////////////////////////////////
// Classe Game - Gerencia a lógica principal do jogo
//////////////////////////////////////////////
class Game{
    public:
        Game();
        ~Game();

        // Métodos existentes
        std::vector< std::vector<Space> > getBoard();
        bool getGameOver() const;
        bool getOccupied(int x, int y) const;
        bool getCurrent(int x, int y) const;
        float getRed(int x, int y) const;
        float getGreen(int x, int y) const;
        float getBlue(int x, int y) const;
        TrashType getTrashType(int x, int y) const;
        
        void spawnFruits();
        void rotate();
        void translate(int direction);
        void moveDown();
        void setCurrent(int x, int y);
        void dropFruit();
        void restart();
        void update(); // Novo: atualiza partículas e efeitos

        // Métodos para o sistema de animação de reciclagem
        bool isLineClearing() const { return line_clearing; }
        int getAnimationStep() const { return animation_step; }
        int getLineBeingCleared() const { return line_being_cleared; }
        TrashType getLineTrashType() const { return line_trash_type; }
        
        // Métodos para texturas
        static void loadTextures();
        static void bindTexture(TrashType type);
        
        // Métodos adicionais para integração completa
        int getCurrentShape() const { return curr_shape; }
        int getCurrentRotation() const { return curr_rotation; }
        int getCurrentX() const { return curr_x; }
        int getCurrentY() const { return curr_y; }
        TrashType* getCurrentTrashTypes() const { return const_cast<TrashType*>(curr_trash_types); }
        
        // Métodos para save/load system
        void setScore(int s) { score = s; }
        void setLevel(int l) { level = l; }
        void setLinesCleared(int lc) { lines_cleared = lc; }
        void setComboCount(int cc) { combo_count = cc; }
        void setRecycledCount(TrashType type, int count) { recycled_count[type] = count; }
        void setCell(int x, int y, bool occupied, TrashType type, float r, float g, float b);
        void setCurrentPiece(int shape, int rotation, int x, int y, TrashType types[4]);
        void setNextPiece(int shape, TrashType types[4]);
        void setHoldPiece(int shape, TrashType types[4], bool can_hold_flag);
        void setGameOver(bool go) { game_over = go; }
        
        // Método para verificar colisão externamente
        bool checkCollision(int x, int y, int rotation);
        
        // Novos métodos para sistema de pontuação e níveis
        int getScore() const { return score; }
        int getLevel() const { return level; }
        int getLinesCleared() const { return lines_cleared; }
        int getComboCount() const { return combo_count; }
        float getDifficultyMultiplier() const;
        std::string getTrashTypeName(TrashType type) const;
        
        // Sistema de próxima peça
        int getNextShape() const { return next_shape; }
        TrashType* getNextTrashTypes() const { return const_cast<TrashType*>(next_trash_types); }
        
        // Sistema de hold (guardar peça)
        void holdPiece();
        int getHoldShape() const { return hold_shape; }
        TrashType* getHoldTrashTypes() const { return const_cast<TrashType*>(hold_trash_types); }
        bool canHold() const { return can_hold; }
        
        // Sistema de estatísticas
        int getRecycledCount(TrashType type) const { return recycled_count[type]; }
        
        // Sistema de partículas
        std::vector<Particle>& getParticles() { return particles; }
        void createRecycleEffect(int x, int y, TrashType type);
        
        // Método getRGB público
        float getRGB(Color color, int RGB) const;
        
    private:
        std::vector< std::vector<Space> > board;
        int curr_shape;
        int curr_rotation;
        int curr_x;
        int curr_y;
        TrashType curr_trash_types[4];
        bool game_over = false;
        
        // Sistema de pontuação e progressão
        int score = 0;
        int level = 1;
        int lines_cleared = 0;
        int combo_count = 0;
        int recycled_count[5] = {0, 0, 0, 0, 0}; // Contador para cada tipo de lixo
        
        // Sistema de próxima peça e hold
        int next_shape;
        TrashType next_trash_types[4];
        int hold_shape = -1;
        TrashType hold_trash_types[4];
        bool can_hold = true;
        
        // Variáveis para animação de reciclagem
        bool line_clearing = false;
        int animation_step = 0;
        int line_being_cleared = -1;
        TrashType line_trash_type = PAPER;
        
        // Sistema de partículas
        std::vector<Particle> particles;
        
        // IDs das texturas OpenGL
        static GLuint texture_ids[5];
        static bool textures_loaded;
        
        // Métodos existentes
        void updateActiveFruits();
        void clearPreviousFrame();
        void freezeCurrent();
        void clearLines();
        void checkRow();
        void deleteRow(int y);
        void checkFruits();
        void checkFruit(int x, int y);
        void deleteFruit(int x1, int y1, int x2, int y2, int x3, int y3);
        void shiftColumn(int x, int k, int diff);
        
        // Métodos para reciclagem melhorados
        bool isUniformLine(int y, TrashType &type);
        void advanceLineAnimation();
        void initLineAnimation(int y, TrashType type);
        TrashType getTrashTypeFromColor(float r, float g, float b);
        void checkMultipleLines(); // Novo: verifica múltiplas linhas simultâneas
        
        // Novos métodos para sistema de pontuação
        void updateScore(int lines_cleared, TrashType type, bool is_combo);
        void updateLevel();
        void generateNextPiece();
        void updateParticles();
        
        // Cores de backup
        const float colors[5][3] = {
            {0.0, 0.0, 1.0},    // PAPER - Azul
            {1.0, 0.0, 0.0},    // PLASTIC - Vermelho  
            {1.0, 1.0, 0.0},    // METAL - Amarelo
            {0.0, 1.0, 0.0},    // GLASS - Verde
            {0.5, 0.25, 0.0}    // ORGANIC - Marrom
        };
        
        // Pontuações base para cada tipo de lixo
        const int base_scores[5] = {100, 150, 200, 175, 125};
};

#endif // GAME_HPP

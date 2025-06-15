#include "game.hpp"
#include <time.h>
#include <stdlib.h>
#include <ostream>
#include <cmath>
#include <iostream>
#include <algorithm>
#define GL_CLAMP_TO_EDGE 0x812F
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// Definição de todas as peças (tetrominos) e suas rotações
const int shapes[7][4][6] = 
{
    {
        {-2, 0, -1, 0, 1, 0}, //I shape 
        {0, -2, 0, -1, 0, 1},
        {2, 0, 1, 0, -1, 0},
        {0, 2, 0, 1, 0, -1}
    },
    {
        {-1, -1, 0, -1, 1, 0}, //S shape
        {1, -1, 1, 0, 0, 1},
        {1, 1, 0, 1, -1, 0},
        {-1, 1, -1, 0, 0, -1}
    },
    {
        {-1, 1, 0, 1, 1, 0}, //reverse S shape
        {-1, -1, -1, 0, 0, 1},
        {1, -1, 0, -1, -1, 0},
        {1, 1, 1, 0, 0, -1}
    },
    {
        {-1, -1, -1, 0, 1, 0}, //L shape
        {1, -1, 0, -1, 0, 1},
        {1, 1, 1, 0, -1, 0},
        {-1, 1, 0, 1, 0, -1}
    },
    {
        {-1, 1, -1, 0, 1, 0}, //reverse L shape
        {-1, -1, 0, -1, 0, 1},
        {1, -1, 1, 0, -1, 0},
        {1, 1, 0, 1, 0, -1}
    },
    {
        {-1, 0, 0, -1, 1, 0}, //T shape
        {0, -1, 1, 0, 0, 1},
        {1, 0, 0, 1, -1, 0},
        {0, 1, -1, 0, 0, -1}
    },
    {
        {0, -1, -1, -1, -1, 0}, //square shape
        {0, -1, -1, -1, -1, 0},
        {0, -1, -1, -1, -1, 0},
        {0, -1, -1, -1, -1, 0}
    }
};

// Inicialização das variáveis estáticas
GLuint Game::texture_ids[5] = {0, 0, 0, 0, 0};
bool Game::textures_loaded = false;

Game::Game(){
    board.resize(10);
    for(int k = 0; k<10; k++){
        board[k].resize(20);
    }
    for(int y=0; y<20; y++){
        for(int x=0; x<10; x++){
            board[x][y].isOccupied = false;
            board[x][y].isCurrent = false;
            board[x][y].combo_id = -1;
        }
    }
    generateNextPiece();
    srand(time(NULL));
}

Game::~Game(){
    board.clear();
}

void Game::restart(){
    board.resize(10);
    for(int k = 0; k<10; k++){
        board[k].resize(20);
    }
    for(int y=0; y<20; y++){
        for(int x=0; x<10; x++){
            board[x][y].isOccupied = false;
            board[x][y].isCurrent = false;
            board[x][y].combo_id = -1;
        }
    }
    game_over = false;
    
    // Reset das variáveis de pontuação e progressão
    score = 0;
    level = 1;
    lines_cleared = 0;
    combo_count = 0;
    for(int i = 0; i < 5; i++) {
        recycled_count[i] = 0;
    }
    
    // Reset do sistema de hold
    hold_shape = -1;
    can_hold = true;
    
    // Reset das variáveis de animação
    line_clearing = false;
    animation_step = 0;
    line_being_cleared = -1;
    line_trash_type = PAPER;
    
    // Limpar partículas
    particles.clear();
    
    generateNextPiece();
    spawnFruits();
}

bool Game::getGameOver() const {
    return game_over; 
}

bool Game::getCurrent(int x, int y) const {
    return board[x][y].isCurrent;
}

bool Game::getOccupied(int x, int y) const {
    return board[x][y].isOccupied;
}

float Game::getRed(int x, int y) const {
    return board[x][y].red;
}

float Game::getGreen(int x, int y) const {
    return board[x][y].green;
}

float Game::getBlue(int x, int y) const {
    return board[x][y].blue;
}

float Game::getRGB(Color color, int RGB) const {
    if (color >= 0 && color < 5 && RGB >= 0 && RGB < 3) {
        return colors[color][RGB];
    }
    return 0.0f;
}

void Game::setCurrent(int x, int y){
    clearPreviousFrame();
    curr_x = x;
    curr_y = y;
    updateActiveFruits();
}

void Game::generateNextPiece() {
    next_shape = rand() % 7;
    TrashType trash_type = static_cast<TrashType>(rand() % 5);
    for(int i = 0; i < 4; i++){
        next_trash_types[i] = trash_type;
    }
}

void Game::spawnFruits(){
    if (line_clearing) return;
    
    // Usar a próxima peça gerada
    curr_shape = next_shape;
    for(int i = 0; i < 4; i++){
        curr_trash_types[i] = next_trash_types[i];
    }
    
    // Gerar nova próxima peça
    generateNextPiece();
    
    int rotation = rand() % 4;
    int position = (rand() % 5) + 2; 
    
    curr_rotation = rotation;
    if(checkCollision(position, 17, rotation)){
        game_over = true;
    }
    else{
        curr_x = position;
        curr_y = 17;
        can_hold = true; // Permite usar hold novamente
        updateActiveFruits();
    }
}

void Game::holdPiece() {
    if (!can_hold) return;
    
    clearPreviousFrame();
    
    if (hold_shape == -1) {
        // Primeira vez usando hold
        hold_shape = curr_shape;
        for(int i = 0; i < 4; i++){
            hold_trash_types[i] = curr_trash_types[i];
        }
        spawnFruits();
    } else {
        // Trocar peça atual com a guardada
        int temp_shape = hold_shape;
        TrashType temp_trash[4];
        for(int i = 0; i < 4; i++){
            temp_trash[i] = hold_trash_types[i];
        }
        
        hold_shape = curr_shape;
        for(int i = 0; i < 4; i++){
            hold_trash_types[i] = curr_trash_types[i];
        }
        
        curr_shape = temp_shape;
        for(int i = 0; i < 4; i++){
            curr_trash_types[i] = temp_trash[i];
        }
        
        // Reposicionar peça
        curr_rotation = 0;
        curr_x = 5;
        curr_y = 17;
        
        if(checkCollision(curr_x, curr_y, curr_rotation)){
            game_over = true;
        } else {
            updateActiveFruits();
        }
    }
    
    can_hold = false;
}

void Game::rotate(){
    int rotation;
    if(curr_rotation > 0){
        rotation = curr_rotation - 1; 
    }
    else{
        rotation = 3;
    }
    if(!(checkCollision(curr_x, curr_y, rotation))){
        clearPreviousFrame();
        curr_rotation = rotation;
        updateActiveFruits();
    }
}

void Game::translate(int direction){
    int new_x = curr_x + direction;
    if(!(checkCollision(new_x, curr_y, curr_rotation))){
        clearPreviousFrame();
        curr_x = new_x;
        updateActiveFruits();
    }
}

void Game::moveDown(){
    if (line_clearing) {
        advanceLineAnimation();
        return;
    }
    
    if(!(checkCollision(curr_x, curr_y - 1, curr_rotation))){
        clearPreviousFrame();
        curr_y -= 1;
        updateActiveFruits();
    }
    else{
        freezeCurrent();
        clearLines();
        spawnFruits();
    }
}

void Game::freezeCurrent(){
    board[curr_x][curr_y].isOccupied = true;
    board[curr_x][curr_y].isCurrent = false;
    board[curr_x][curr_y].trash_type = curr_trash_types[0];
    board[curr_x][curr_y].red = colors[curr_trash_types[0]][0];
    board[curr_x][curr_y].green = colors[curr_trash_types[0]][1];
    board[curr_x][curr_y].blue = colors[curr_trash_types[0]][2];
    
    int k = 1;
    for(int i = 1; i < 6; i+=2){
        int new_x = curr_x + shapes[curr_shape][curr_rotation][i-1];
        int new_y = curr_y + shapes[curr_shape][curr_rotation][i];
        
        board[new_x][new_y].isOccupied = true;
        board[new_x][new_y].isCurrent = false;
        board[new_x][new_y].trash_type = curr_trash_types[k];
        board[new_x][new_y].red = colors[curr_trash_types[k]][0];
        board[new_x][new_y].green = colors[curr_trash_types[k]][1];
        board[new_x][new_y].blue = colors[curr_trash_types[k]][2];
        k++;
    }
}

void Game::clearLines(){
    checkMultipleLines();
}

void Game::checkMultipleLines() {
    if (line_clearing) return;
    
    std::vector<int> lines_to_clear;
    std::vector<TrashType> line_types;
    
    for(int y = 0; y < 20; y++){
        bool full = true;
        for(int x = 0; x < 10; x++){
            if(!(board[x][y].isOccupied)){
                full = false;
                break;
            }
        }
        
        if(full){
            TrashType type;
            if (isUniformLine(y, type)) {
                lines_to_clear.push_back(y);
                line_types.push_back(type);
            } else {
                // Linha mista - remove imediatamente
                deleteRow(y);
                y--;
            }
        }
    }
    
    if (!lines_to_clear.empty()) {
        // Processar múltiplas linhas uniformes
        if (lines_to_clear.size() > 1) {
            combo_count++;
        } else {
            combo_count = 0;
        }
        
        // Iniciar animação para a primeira linha
        initLineAnimation(lines_to_clear[0], line_types[0]);
        
        // Calcular pontuação para todas as linhas
        for (size_t i = 0; i < lines_to_clear.size(); i++) {
            updateScore(1, line_types[i], i > 0 || combo_count > 0);
            recycled_count[line_types[i]]++;
            
            // Criar efeito de partículas
            for (int x = 0; x < 10; x++) {
                createRecycleEffect(x, lines_to_clear[i], line_types[i]);
            }
        }
        
        lines_cleared += lines_to_clear.size();
        updateLevel();
    } else {
        combo_count = 0;
    }
}

void Game::checkRow(){
    if (line_clearing) return;
    
    bool full;
    TrashType type;
    
    for(int y=0; y<20; y++){
        full = true;
        for(int x=0; x<10; x++){
            if(!(board[x][y].isOccupied)){
                full = false;
                break;
            }
        }
        if(full){
            if (isUniformLine(y, type)) {
                initLineAnimation(y, type);
                return;
            } else {
                deleteRow(y);
                y--;
            }
        }
    }
}

void Game::deleteRow(int y){
    for(int k = y; k<19; k++){
        for(int x = 0; x<10; x++){
            board[x][k].isOccupied = board[x][k+1].isOccupied;
            board[x][k].isCurrent = board[x][k+1].isCurrent;
            board[x][k].trash_type = board[x][k+1].trash_type;
            board[x][k].red = board[x][k+1].red;
            board[x][k].green = board[x][k+1].green;
            board[x][k].blue = board[x][k+1].blue;
            board[x][k].combo_id = board[x][k+1].combo_id;
        }
    }
    for(int x = 0; x<10; x++){
        board[x][19].isOccupied = false;
        board[x][19].isCurrent = false;
        board[x][19].combo_id = -1;
    }
}

void Game::clearPreviousFrame(){
    if(curr_x < 10 && curr_x > -1 && curr_y < 20 && curr_y > -1){
        board[curr_x][curr_y].isCurrent = false;
    }
    for(int i = 1; i < 6; i+=2){
        if(curr_x+shapes[curr_shape][curr_rotation][i-1] < 10 && 
            curr_x+shapes[curr_shape][curr_rotation][i-1] > -1 &&
            curr_y+shapes[curr_shape][curr_rotation][i] < 20 &&
            curr_y+shapes[curr_shape][curr_rotation][i] > -1){
                board[curr_x+shapes[curr_shape][curr_rotation][i-1]][curr_y+shapes[curr_shape][curr_rotation][i]].isCurrent = false;
            }
    }
}

bool Game::checkCollision(int x, int y, int rotation){
    int xpos[4] = {x, 
        x+shapes[curr_shape][rotation][0], 
        x+shapes[curr_shape][rotation][2], 
        x+shapes[curr_shape][rotation][4]
    };
    int ypos[4] = {y,
        y+shapes[curr_shape][rotation][1],
        y+shapes[curr_shape][rotation][3],
        y+shapes[curr_shape][rotation][5]
    };
    for(int i=0; i<4; i++){
        if(xpos[i] > 9 || xpos[i] < 0 || ypos[i] > 19 || ypos[i] < 0){
            return true;
        }
        if(board[xpos[i]][ypos[i]].isOccupied){
            return true;
        }
    }
    return false;
}

void Game::updateActiveFruits(){
    if(curr_x >= 0 && curr_x < 10 && curr_y >= 0 && curr_y < 20){
        board[curr_x][curr_y].isCurrent = true;
        if(!board[curr_x][curr_y].isOccupied){
            board[curr_x][curr_y].trash_type = curr_trash_types[0];
            board[curr_x][curr_y].red = colors[curr_trash_types[0]][0];
            board[curr_x][curr_y].green = colors[curr_trash_types[0]][1];
            board[curr_x][curr_y].blue = colors[curr_trash_types[0]][2];
        }
    }
    
    int k = 1;
    for(int i = 1; i < 6; i+=2){
        int new_x = curr_x + shapes[curr_shape][curr_rotation][i-1];
        int new_y = curr_y + shapes[curr_shape][curr_rotation][i];
        
        if(new_x >= 0 && new_x < 10 && new_y >= 0 && new_y < 20){
            board[new_x][new_y].isCurrent = true;
            if(!board[new_x][new_y].isOccupied){
                board[new_x][new_y].trash_type = curr_trash_types[k];
                board[new_x][new_y].red = colors[curr_trash_types[k]][0];
                board[new_x][new_y].green = colors[curr_trash_types[k]][1];
                board[new_x][new_y].blue = colors[curr_trash_types[k]][2];
            }
        }
        k++;
    }
}

void Game::deleteFruit(int x1, int y1, int x2, int y2, int x3, int y3){
    if(x1 == x2){
        for(int k = y1; k>3; k--){
            shiftColumn(x1, k, 3);
        }
    }
    else{
        for(int k = y1; k>0; k--){
            shiftColumn(x1, k, 1);
        }
        for(int k = y2; k>0; k--){
            shiftColumn(x2, k, 1);
        }
        for(int k = y3; k>0; k--){
            shiftColumn(x3, k, 1);
        }
    }
}

void Game::shiftColumn(int x, int k, int diff){
    board[x][k].isOccupied = board[x][k-diff].isOccupied;
    board[x][k].isCurrent = board[x][k-diff].isCurrent;
    board[x][k].trash_type = board[x][k-diff].trash_type;
    board[x][k].red = board[x][k-diff].red;
    board[x][k].green = board[x][k-diff].green;
    board[x][k].blue = board[x][k-diff].blue;
    board[x][k].combo_id = board[x][k-diff].combo_id;
}

void Game::dropFruit(){
    if(!(checkCollision(curr_x, curr_y, curr_rotation))){
        freezeCurrent();
        spawnFruits();
    }
    else{
        game_over = true;
    }
}

TrashType Game::getTrashType(int x, int y) const {
    if (x >= 0 && x < 10 && y >= 0 && y < 20) {
        return board[x][y].trash_type;
    }
    return NONE;
}

bool Game::isUniformLine(int y, TrashType &type) {
    if (!board[0][y].isOccupied) return false;
    
    type = board[0][y].trash_type;
    
    for (int x = 1; x < 10; x++) {
        if (!board[x][y].isOccupied) return false;
        if (board[x][y].trash_type != type) return false;
    }
    
    return true;
}

void Game::loadTextures() {
    if (textures_loaded) return;
    
    stbi_set_flip_vertically_on_load(true);
    glGenTextures(5, texture_ids);
    
    const char* texture_files[5] = {
        "textures/paper.png",
        "textures/plastic.png",
        "textures/metal.png",
        "textures/glass.png",
        "textures/organic.png"
    };
    
    for (int i = 0; i < 5; i++) {
        int width, height, channels;
        unsigned char* data = stbi_load(texture_files[i], &width, &height, &channels, 0);
        
        if (data) {
            glBindTexture(GL_TEXTURE_2D, texture_ids[i]);
            
            GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            stbi_image_free(data);
            std::cout << "Textura carregada: " << texture_files[i] << std::endl;
        } else {
            std::cout << "Erro ao carregar textura: " << texture_files[i] << std::endl;
        }
    }
    
    textures_loaded = true;
    std::cout << "Texturas carregadas com sucesso!" << std::endl;
}

void Game::bindTexture(TrashType type) {
    if (!textures_loaded) {
        loadTextures();
    }
    
    if (type >= 0 && type < 5) {
        glBindTexture(GL_TEXTURE_2D, texture_ids[type]);
    }
}

void Game::initLineAnimation(int y, TrashType type) {
    line_clearing = true;
    line_being_cleared = y;
    line_trash_type = type;
    animation_step = 0;
}

void Game::advanceLineAnimation() {
    if (!line_clearing) return;
    
    animation_step++;
    
    if (animation_step >= 10) {
        deleteRow(line_being_cleared);
        line_clearing = false;
        animation_step = 0;
        line_being_cleared = -1;
        line_trash_type = PAPER;
        
        checkRow();
    }
}

void Game::updateScore(int lines, TrashType type, bool is_combo) {
    int base_points = base_scores[type] * lines;
    
    // Multiplicador de nível
    float level_multiplier = 1.0f + (level - 1) * 0.1f;
    
    // Multiplicador de combo
    float combo_multiplier = 1.0f + combo_count * 0.2f;
    
    // Bônus por linha uniforme
    float uniform_bonus = 1.5f;
    
    int points = (int)(base_points * level_multiplier * combo_multiplier * uniform_bonus);
    
    if (is_combo) {
        points = (int)(points * 1.5f); // Bônus adicional para combos
    }
    
    score += points;
}

void Game::updateLevel() {
    int new_level = (lines_cleared / 10) + 1;
    if (new_level > level) {
        level = new_level;
        std::cout << "Nível " << level << " alcançado!" << std::endl;
    }
}

float Game::getDifficultyMultiplier() const {
    return std::max(0.1f, 1.0f - (level - 1) * 0.05f);
}

std::string Game::getTrashTypeName(TrashType type) const {
    switch (type) {
        case PAPER: return "Papel";
        case PLASTIC: return "Plástico";
        case METAL: return "Metal";
        case GLASS: return "Vidro";
        case ORGANIC: return "Orgânico";
        default: return "Desconhecido";
    }
}

void Game::createRecycleEffect(int x, int y, TrashType type) {
    for (int i = 0; i < 3; i++) {
        Particle p;
        p.x = x + 0.5f;
        p.y = y + 0.5f;
        p.vx = ((rand() % 200) - 100) / 100.0f;
        p.vy = ((rand() % 200) - 100) / 100.0f;
        p.life = 1.0f;
        p.size = 0.1f + (rand() % 20) / 100.0f;
        p.type = type;
        particles.push_back(p);
    }
}

void Game::updateParticles() {
    for (auto it = particles.begin(); it != particles.end();) {
        it->x += it->vx * 0.016f; // 60 FPS
        it->y += it->vy * 0.016f;
        it->life -= 0.016f;
        it->size *= 0.98f;
        
        if (it->life <= 0 || it->size <= 0.01f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::update() {
    updateParticles();
}

// Métodos adicionais para save/load system
void Game::setCell(int x, int y, bool occupied, TrashType type, float r, float g, float b) {
    if (x >= 0 && x < 10 && y >= 0 && y < 20) {
        board[x][y].isOccupied = occupied;
        board[x][y].trash_type = type;
        board[x][y].red = r;
        board[x][y].green = g;
        board[x][y].blue = b;
    }
}

void Game::setCurrentPiece(int shape, int rotation, int x, int y, TrashType types[4]) {
    curr_shape = shape;
    curr_rotation = rotation;
    curr_x = x;
    curr_y = y;
    for (int i = 0; i < 4; i++) {
        curr_trash_types[i] = types[i];
    }
}

void Game::setNextPiece(int shape, TrashType types[4]) {
    next_shape = shape;
    for (int i = 0; i < 4; i++) {
        next_trash_types[i] = types[i];
    }
}

void Game::setHoldPiece(int shape, TrashType types[4], bool can_hold_flag) {
    hold_shape = shape;
    for (int i = 0; i < 4; i++) {
        hold_trash_types[i] = types[i];
    }
    can_hold = can_hold_flag;
}

// Implementações vazias para métodos não utilizados
void Game::checkFruits() {}
void Game::checkFruit(int x, int y) {}
TrashType Game::getTrashTypeFromColor(float r, float g, float b) { return PAPER; }
std::vector< std::vector<Space> > Game::getBoard() { return board; }

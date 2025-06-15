#ifndef SAVE_SYSTEM_HPP
#define SAVE_SYSTEM_HPP

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm> // FIX: Incluído para std::sort
#include <ctime>     // FIX: Incluído para time_t
#include "game.hpp"
#include "achievements.hpp"

struct GameSettings {
    float master_volume = 1.0f;
    float sound_volume = 1.0f;
    float music_volume = 0.5f;
    bool sound_enabled = true;
    bool music_enabled = true;
    bool show_particles = true;
    bool show_grid = true;
    bool show_shadows = true;
    int difficulty_mode = 0; // 0=Normal, 1=Easy, 2=Hard
    bool auto_save = true;
    
    // Estatísticas globais
    int total_games_played = 0;
    int best_score = 0;
    int best_level = 0;
    int total_lines_cleared = 0;
    int total_time_played = 0; // em segundos
    
    // Estatísticas de reciclagem
    int lifetime_recycled[5] = {0, 0, 0, 0, 0};
};

struct GameSave {
    int score;
    int level;
    int lines_cleared;
    int combo_count;
    int recycled_count[5];
    int curr_shape;
    int curr_rotation;
    int curr_x;
    int curr_y;
    TrashType curr_trash_types[4];
    int next_shape;
    TrashType next_trash_types[4];
    int hold_shape;
    TrashType hold_trash_types[4];
    bool can_hold;
    bool game_over;
    
    // Estado do tabuleiro
    struct BoardCell {
        bool isOccupied;
        TrashType trash_type;
        float red, green, blue;
    } board[10][20];
    
    time_t save_time;
};

class SaveSystem {
public:
    static SaveSystem& getInstance();
    
    // Configurações
    bool loadSettings();
    bool saveSettings();
    GameSettings& getSettings() { return settings; }
    
    // Save/Load do jogo
    bool saveGame(const Game& game, const std::string& slot_name = "autosave");
    bool loadGame(Game& game, const std::string& slot_name = "autosave");
    bool hasSaveFile(const std::string& slot_name = "autosave");
    void deleteSaveFile(const std::string& slot_name = "autosave");
    
    // Estatísticas
    void updateStatistics(const Game& game, int game_duration);
    void resetStatistics();
    
    // High Scores
    struct HighScore {
        int score;
        int level;
        int lines;
        std::string date;
        int duration;
    };
    
    std::vector<HighScore> getHighScores();
    void addHighScore(int score, int level, int lines, int duration);
    
    // Conquistas
    bool saveAchievements();
    bool loadAchievements();

private:
    SaveSystem() = default;
    
    GameSettings settings;
    std::vector<HighScore> high_scores;
    
    std::string getSettingsPath() { return "ecotetris_settings.dat"; }
    std::string getSavePath(const std::string& slot) { return "ecotetris_save_" + slot + ".dat"; }
    std::string getHighScoresPath() { return "ecotetris_scores.dat"; }
    std::string getAchievementsPath() { return "ecotetris_achievements.dat"; }
    
    void loadHighScores();
    void saveHighScores();
};

SaveSystem& SaveSystem::getInstance() {
    static SaveSystem instance;
    return instance;
}

bool SaveSystem::loadSettings() {
    std::ifstream file(getSettingsPath(), std::ios::binary);
    if (!file.is_open()) {
        // Criar configurações padrão
        saveSettings();
        return true;
    }
    
    try {
        file.read(reinterpret_cast<char*>(&settings), sizeof(GameSettings));
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao carregar configurações: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool SaveSystem::saveSettings() {
    std::ofstream file(getSettingsPath(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao criar arquivo de configurações" << std::endl;
        return false;
    }
    
    try {
        file.write(reinterpret_cast<const char*>(&settings), sizeof(GameSettings));
        file.close();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Erro ao salvar configurações: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool SaveSystem::saveGame(const Game& game, const std::string& slot_name) {
    if (!settings.auto_save && slot_name == "autosave") {
        return true; // Auto-save desabilitado
    }
    
    std::ofstream file(getSavePath(slot_name), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Erro ao criar arquivo de save" << std::endl;
        return false;
    }
    
    try {
        GameSave save_data;
        
        // Copiar dados básicos do jogo
        save_data.score = game.getScore();
        save_data.level = game.getLevel();
        save_data.lines_cleared = game.getLinesCleared();
        save_data.combo_count = game.getComboCount();
        
        for (int i = 0; i < 5; i++) {
            save_data.recycled_count[i] = game.getRecycledCount(static_cast<TrashType>(i));
        }
        
        // Estado da peça atual
        save_data.curr_shape = game.getCurrentShape();
        save_data.curr_rotation = game.getCurrentRotation();
        save_data.curr_x = game.getCurrentX();
        save_data.curr_y = game.getCurrentY();
        
        // Copiar tipos de lixo da peça atual
        for (int i = 0; i < 4; i++) {
            save_data.curr_trash_types[i] = game.getCurrentTrashTypes()[i];
        }
        
        // Próxima peça
        save_data.next_shape = game.getNextShape();
        for (int i = 0; i < 4; i++) {
            save_data.next_trash_types[i] = game.getNextTrashTypes()[i];
        }
        
        // Peça em hold
        save_data.hold_shape = game.getHoldShape();
        for (int i = 0; i < 4; i++) {
            save_data.hold_trash_types[i] = game.getHoldTrashTypes()[i];
        }
        save_data.can_hold = game.canHold();
        save_data.game_over = game.getGameOver();
        
        // Estado do tabuleiro
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 20; y++) {
                save_data.board[x][y].isOccupied = game.getOccupied(x, y);
                save_data.board[x][y].trash_type = game.getTrashType(x, y);
                save_data.board[x][y].red = game.getRed(x, y);
                save_data.board[x][y].green = game.getGreen(x, y);
                save_data.board[x][y].blue = game.getBlue(x, y);
            }
        }
        
        save_data.save_time = time(nullptr);
        
        file.write(reinterpret_cast<const char*>(&save_data), sizeof(GameSave));
        file.close();
        
        std::cout << "Jogo salvo com sucesso em: " << slot_name << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro ao salvar jogo: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool SaveSystem::loadGame(Game& game, const std::string& slot_name) {
    std::ifstream file(getSavePath(slot_name), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Arquivo de save não encontrado: " << slot_name << std::endl;
        return false;
    }
    
    try {
        GameSave save_data;
        file.read(reinterpret_cast<char*>(&save_data), sizeof(GameSave));
        file.close();
        
        // Restaurar estado do jogo
        game.restart(); // Limpar estado atual
        
        // Definir valores básicos
        game.setScore(save_data.score);
        game.setLevel(save_data.level);
        game.setLinesCleared(save_data.lines_cleared);
        game.setComboCount(save_data.combo_count);
        
        for (int i = 0; i < 5; i++) {
            game.setRecycledCount(static_cast<TrashType>(i), save_data.recycled_count[i]);
        }
        
        // Restaurar estado do tabuleiro
        for (int x = 0; x < 10; x++) {
            for (int y = 0; y < 20; y++) {
                if (save_data.board[x][y].isOccupied) {
                    game.setCell(x, y, true, save_data.board[x][y].trash_type,
                               save_data.board[x][y].red, save_data.board[x][y].green, save_data.board[x][y].blue);
                }
            }
        }
        
        // Restaurar peças
        game.setCurrentPiece(save_data.curr_shape, save_data.curr_rotation, 
                           save_data.curr_x, save_data.curr_y, save_data.curr_trash_types);
        game.setNextPiece(save_data.next_shape, save_data.next_trash_types);
        game.setHoldPiece(save_data.hold_shape, save_data.hold_trash_types, save_data.can_hold);
        game.setGameOver(save_data.game_over);
        
        std::cout << "Jogo carregado com sucesso de: " << slot_name << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Erro ao carregar jogo: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

bool SaveSystem::hasSaveFile(const std::string& slot_name) {
    std::ifstream file(getSavePath(slot_name));
    bool exists = file.good();
    file.close();
    return exists;
}

void SaveSystem::deleteSaveFile(const std::string& slot_name) {
    remove(getSavePath(slot_name).c_str());
}

void SaveSystem::updateStatistics(const Game& game, int game_duration) {
    settings.total_games_played++;
    settings.total_time_played += game_duration;
    
    if (game.getScore() > settings.best_score) {
        settings.best_score = game.getScore();
    }
    
    if (game.getLevel() > settings.best_level) {
        settings.best_level = game.getLevel();
    }
    
    settings.total_lines_cleared += game.getLinesCleared();
    
    for (int i = 0; i < 5; i++) {
        settings.lifetime_recycled[i] += game.getRecycledCount(static_cast<TrashType>(i));
    }
    
    // Adicionar ao high score se qualificar
    addHighScore(game.getScore(), game.getLevel(), game.getLinesCleared(), game_duration);
    
    saveSettings();
}

void SaveSystem::addHighScore(int score, int level, int lines, int duration) {
    HighScore new_score;
    new_score.score = score;
    new_score.level = level;
    new_score.lines = lines;
    new_score.duration = duration;
    
    // Formatar data atual
    time_t now = time(0);
    char* dt = ctime(&now);
    new_score.date = std::string(dt).substr(0, 24); // Remove quebra de linha
    
    high_scores.push_back(new_score);
    
    // Ordenar por pontuação (maior primeiro)
    std::sort(high_scores.begin(), high_scores.end(), 
              [](const HighScore& a, const HighScore& b) {
                  return a.score > b.score;
              });
    
    // Manter apenas top 10
    if (high_scores.size() > 10) {
        high_scores.resize(10);
    }
    
    saveHighScores();
}

std::vector<SaveSystem::HighScore> SaveSystem::getHighScores() {
    if (high_scores.empty()) {
        loadHighScores();
    }
    return high_scores;
}

void SaveSystem::loadHighScores() {
    std::ifstream file(getHighScoresPath(), std::ios::binary);
    if (!file.is_open()) return;
    
    high_scores.clear();
    HighScore score;
    
    while (file.read(reinterpret_cast<char*>(&score), sizeof(HighScore))) {
        high_scores.push_back(score);
    }
    
    file.close();
}

void SaveSystem::saveHighScores() {
    std::ofstream file(getHighScoresPath(), std::ios::binary);
    if (!file.is_open()) return;
    
    for (const auto& score : high_scores) {
        file.write(reinterpret_cast<const char*>(&score), sizeof(HighScore));
    }
    
    file.close();
}

bool SaveSystem::saveAchievements() {
    std::ofstream file(getAchievementsPath(), std::ios::binary);
    if (!file.is_open()) return false;
    
    auto& achievements = AchievementManager::getInstance().getAllAchievements();
    
    size_t count = achievements.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
    
    for (const auto& achievement : achievements) {
        file.write(reinterpret_cast<const char*>(&achievement.id), sizeof(int));
        file.write(reinterpret_cast<const char*>(&achievement.unlocked), sizeof(bool));
    }
    
    file.close();
    return true;
}

bool SaveSystem::loadAchievements() {
    std::ifstream file(getAchievementsPath(), std::ios::binary);
    if (!file.is_open()) return false;
    
    size_t count;
    file.read(reinterpret_cast<char*>(&count), sizeof(size_t));
    
    auto& achievements = const_cast<std::vector<Achievement>&>(
        AchievementManager::getInstance().getAllAchievements());
    
    for (size_t i = 0; i < count; i++) {
        int id;
        bool unlocked;
        file.read(reinterpret_cast<char*>(&id), sizeof(int));
        file.read(reinterpret_cast<char*>(&unlocked), sizeof(bool));
        
        // Encontrar e atualizar conquista
        for (auto& achievement : achievements) {
            if (achievement.id == id) {
                achievement.unlocked = unlocked;
                break;
            }
        }
    }
    
    file.close();
    return true;
}

void SaveSystem::resetStatistics() {
    settings.total_games_played = 0;
    settings.best_score = 0;
    settings.best_level = 0;
    settings.total_lines_cleared = 0;
    settings.total_time_played = 0;
    
    for (int i = 0; i < 5; i++) {
        settings.lifetime_recycled[i] = 0;
    }
    
    high_scores.clear();
    saveSettings();
    saveHighScores();
}

#endif
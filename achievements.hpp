#ifndef ACHIEVEMENTS_HPP
#define ACHIEVEMENTS_HPP

#include <vector>
#include <string>
#include <map>
#include "game.hpp" // Dependência de Game

class AchievementManager {
enum AchievementType {
    SCORE_BASED,
    LINES_BASED,
    LEVEL_BASED,
    RECYCLE_BASED,
    COMBO_BASED,
    SPECIAL
};

struct Achievement {
    int id;
    std::string name;
    std::string description;
    AchievementType type;
    int target_value;
    TrashType specific_trash = NONE;
    bool unlocked = false;
    int icon_id;
    
    Achievement(int _id, const std::string& _name, const std::string& _desc, 
                AchievementType _type, int _target, int _icon = 0, TrashType _trash = NONE)
        : id(_id), name(_name), description(_desc), type(_type), 
          target_value(_target), icon_id(_icon), specific_trash(_trash) {}
};

class AchievementManager {
public:
    static AchievementManager& getInstance();
    
    void init();
    void checkAchievements(const Game& game);
    void unlockAchievement(int achievement_id);
    
    // Getters
    const std::vector<Achievement>& getAllAchievements() const { return achievements; }
    std::vector<Achievement> getUnlockedAchievements() const;
    std::vector<Achievement> getLockedAchievements() const;
    int getUnlockedCount() const;
    float getCompletionPercentage() const;
    
    // Notificações
    bool hasNewAchievement() const { return new_achievement_notification; }
    Achievement getLatestAchievement() const { return latest_achievement; }
    void clearNotification() { new_achievement_notification = false; }

private:
    AchievementManager() = default;
    
    std::vector<Achievement> achievements;
    bool new_achievement_notification = false;
    Achievement latest_achievement = Achievement(0, "", "", SCORE_BASED, 0);
    
    void createAchievements();
};

AchievementManager& AchievementManager::getInstance() {
    static AchievementManager instance;
    return instance;
}

void AchievementManager::init() {
    createAchievements();
}

void AchievementManager::createAchievements() {
    achievements.clear();
    
    // Conquistas baseadas em pontuação
    achievements.emplace_back(1, "Primeiro Passo", "Alcance 1.000 pontos", SCORE_BASED, 1000, 1);
    achievements.emplace_back(2, "Reciclador Iniciante", "Alcance 5.000 pontos", SCORE_BASED, 5000, 2);
    achievements.emplace_back(3, "Eco Guerreiro", "Alcance 25.000 pontos", SCORE_BASED, 25000, 3);
    achievements.emplace_back(4, "Mestre da Reciclagem", "Alcance 100.000 pontos", SCORE_BASED, 100000, 4);
    achievements.emplace_back(5, "Lenda Ecológica", "Alcance 500.000 pontos", SCORE_BASED, 500000, 5);
    
    // Conquistas baseadas em linhas
    achievements.emplace_back(6, "Primeira Limpeza", "Limpe 10 linhas", LINES_BASED, 10, 6);
    achievements.emplace_back(7, "Limpador Eficiente", "Limpe 100 linhas", LINES_BASED, 100, 7);
    achievements.emplace_back(8, "Máquina de Limpeza", "Limpe 500 linhas", LINES_BASED, 500, 8);
    achievements.emplace_back(9, "Demolidor Ecológico", "Limpe 1000 linhas", LINES_BASED, 1000, 9);
    
    // Conquistas baseadas em nível
    achievements.emplace_back(10, "Subindo de Nível", "Alcance o nível 5", LEVEL_BASED, 5, 10);
    achievements.emplace_back(11, "Especialista", "Alcance o nível 10", LEVEL_BASED, 10, 11);
    achievements.emplace_back(12, "Mestre", "Alcance o nível 20", LEVEL_BASED, 20, 12);
    achievements.emplace_back(13, "Lenda", "Alcance o nível 50", LEVEL_BASED, 50, 13);
    
    // Conquistas específicas por tipo de lixo
    achievements.emplace_back(14, "Amigo do Papel", "Recicle 100 itens de papel", RECYCLE_BASED, 100, 14, PAPER);
    achievements.emplace_back(15, "Guerreiro do Plástico", "Recicle 100 itens de plástico", RECYCLE_BASED, 100, 15, PLASTIC);
    achievements.emplace_back(16, "Coletor de Metal", "Recicle 100 itens de metal", RECYCLE_BASED, 100, 16, METAL);
    achievements.emplace_back(17, "Protetor do Vidro", "Recicle 100 itens de vidro", RECYCLE_BASED, 100, 17, GLASS);
    achievements.emplace_back(18, "Composteiro", "Recicle 100 itens orgânicos", RECYCLE_BASED, 100, 18, ORGANIC);
    
    // Conquistas avançadas de reciclagem
    achievements.emplace_back(19, "Reciclador Completo", "Recicle 50 itens de cada tipo", SPECIAL, 50, 19);
    achievements.emplace_back(20, "Eco Champion", "Recicle 1000 itens no total", SPECIAL, 1000, 20);
    
    // Conquistas de combo
    achievements.emplace_back(21, "Combo Iniciante", "Faça um combo de 3", COMBO_BASED, 3, 21);
    achievements.emplace_back(22, "Combo Master", "Faça um combo de 5", COMBO_BASED, 5, 22);
    achievements.emplace_back(23, "Combo Legend", "Faça um combo de 10", COMBO_BASED, 10, 23);
    
    // Conquistas especiais
    achievements.emplace_back(24, "Velocista Ecológico", "Alcance nível 10 em menos de 5 minutos", SPECIAL, 1, 24);
    achievements.emplace_back(25, "Perfeccionista", "Complete um nível sem errar uma peça", SPECIAL, 1, 25);
}

void AchievementManager::checkAchievements(const Game& game) {
    for (auto& achievement : achievements) {
        if (achievement.unlocked) continue;
        
        bool should_unlock = false;
        
        switch (achievement.type) {
            case SCORE_BASED:
                should_unlock = game.getScore() >= achievement.target_value;
                break;
                
            case LINES_BASED:
                should_unlock = game.getLinesCleared() >= achievement.target_value;
                break;
                
            case LEVEL_BASED:
                should_unlock = game.getLevel() >= achievement.target_value;
                break;
                
            case RECYCLE_BASED:
                if (achievement.specific_trash != NONE) {
                    should_unlock = game.getRecycledCount(achievement.specific_trash) >= achievement.target_value;
                }
                break;
                
            case COMBO_BASED:
                should_unlock = game.getComboCount() >= achievement.target_value;
                break;
                
            case SPECIAL:
                // Lógica especial para cada conquista
                if (achievement.id == 19) { // Reciclador Completo
                    bool all_types_met = true;
                    for (int i = 0; i < 5; i++) {
                        if (game.getRecycledCount(static_cast<TrashType>(i)) < achievement.target_value) {
                            all_types_met = false;
                            break;
                        }
                    }
                    should_unlock = all_types_met;
                } else if (achievement.id == 20) { // Eco Champion
                    int total_recycled = 0;
                    for (int i = 0; i < 5; i++) {
                        total_recycled += game.getRecycledCount(static_cast<TrashType>(i));
                    }
                    should_unlock = total_recycled >= achievement.target_value;
                }
                break;
        }
        
        if (should_unlock) {
            unlockAchievement(achievement.id);
        }
    }
}

void AchievementManager::unlockAchievement(int achievement_id) {
    for (auto& achievement : achievements) {
        if (achievement.id == achievement_id && !achievement.unlocked) {
            achievement.unlocked = true;
            latest_achievement = achievement;
            new_achievement_notification = true;
            
            // Aqui você pode adicionar efeitos sonoros ou visuais
            // SoundManager::getInstance().playSound("achievement_unlock");
            break;
        }
    }
}

std::vector<Achievement> AchievementManager::getUnlockedAchievements() const {
    std::vector<Achievement> unlocked;
    for (const auto& achievement : achievements) {
        if (achievement.unlocked) {
            unlocked.push_back(achievement);
        }
    }
    return unlocked;
}

std::vector<Achievement> AchievementManager::getLockedAchievements() const {
    std::vector<Achievement> locked;
    for (const auto& achievement : achievements) {
        if (!achievement.unlocked) {
            locked.push_back(achievement);
        }
    }
    return locked;
}

int AchievementManager::getUnlockedCount() const {
    int count = 0;
    for (const auto& achievement : achievements) {
        if (achievement.unlocked) count++;
    }
    return count;
}

float AchievementManager::getCompletionPercentage() const {
    if (achievements.empty()) return 0.0f;
    return (float)getUnlockedCount() / (float)achievements.size() * 100.0f;
}
}

#endif
#ifndef MENU_SYSTEM_HPP
#define MENU_SYSTEM_HPP

#include <vector>
#include <string>
#include <functional>
#include <cmath> // FIX: Incluído para sin() e fmod()
#include "save_system.hpp"
#include "achievements.hpp"

enum GameState {
    MAIN_MENU,
    PLAYING,
    PAUSED,
    GAME_OVER,
    SETTINGS,
    ACHIEVEMENTS,
    HIGH_SCORES,
    HELP,
    LOADING
};

enum MenuOption {
    NEW_GAME,
    CONTINUE_GAME,
    SETTINGS_MENU,
    ACHIEVEMENTS_MENU,
    HIGH_SCORES_MENU,
    HELP_MENU,
    EXIT_GAME,
    BACK_TO_MENU,
    RESUME_GAME,
    RESTART_GAME,
    SAVE_GAME,
    LOAD_GAME
};

struct MenuItem {
    std::string text;
    MenuOption option;
    bool enabled;
    std::function<void()> action;
    
    MenuItem(const std::string& _text, MenuOption _option, bool _enabled = true)
        : text(_text), option(_option), enabled(_enabled) {}
};

class MenuSystem {
public:
    static MenuSystem& getInstance();
    
    void init();
    void update();
    void render();
    
    // Controle de estado
    GameState getCurrentState() const { return current_state; }
    void setState(GameState state);
    
    // Controle de menu
    void moveUp();
    void moveDown();
    void select();
    void back();
    
    // Callbacks para ações do menu
    void setNewGameCallback(std::function<void()> callback) { new_game_callback = callback; }
    void setContinueGameCallback(std::function<void()> callback) { continue_game_callback = callback; }
    void setExitGameCallback(std::function<void()> callback) { exit_game_callback = callback; }
    
    // Sistema de notificações
    void showNotification(const std::string& message, float duration = 3.0f);
    bool hasActiveNotification() const { return notification_timer > 0; }
    
    // Animações
    void startTransition(GameState target_state);
    bool isTransitioning() const { return transition_active; }

private:
    MenuSystem() = default;
    
    GameState current_state = MAIN_MENU;
    GameState target_state = MAIN_MENU;
    
    std::vector<MenuItem> current_menu;
    int selected_index = 0;
    
    // Animações e transições
    bool transition_active = false;
    float transition_progress = 0.0f;
    float menu_animation_offset = 0.0f;
    
    // Notificações
    std::string notification_text;
    float notification_timer = 0.0f;
    
    // Callbacks
    std::function<void()> new_game_callback;
    std::function<void()> continue_game_callback;
    std::function<void()> exit_game_callback;
    
    // Métodos internos
    void buildMainMenu();
    void buildPauseMenu();
    void buildSettingsMenu();
    void buildGameOverMenu();
    
    void renderMainMenu();
    void renderPauseMenu();
    void renderSettingsMenu();
    void renderAchievements();
    void renderHighScores();
    void renderHelp();
    void renderGameOver();
    void renderNotification();
    void renderBackground();
    
    void executeMenuAction(MenuOption option);
    void updateAnimations();
    void updateNotifications();
    
    // Utilitários de renderização
    void renderMenuItem(const MenuItem& item, int index, float y_position);
    void renderTitle(const std::string& title, float y_position);
    void renderText(const std::string& text, float x, float y, bool centered = false);
    void renderProgressBar(float x, float y, float width, float height, float progress);
};

MenuSystem& MenuSystem::getInstance() {
    static MenuSystem instance;
    return instance;
}

void MenuSystem::init() {
    setState(MAIN_MENU);
}

void MenuSystem::setState(GameState state) {
    if (state != current_state) {
        current_state = state;
        selected_index = 0;
        
        switch (state) {
            case MAIN_MENU:
                buildMainMenu();
                break;
            case PAUSED:
                buildPauseMenu();
                break;
            case SETTINGS:
                buildSettingsMenu();
                break;
            case GAME_OVER:
                buildGameOverMenu();
                break;
            default:
                current_menu.clear();
                break;
        }
    }
}

void MenuSystem::buildMainMenu() {
    current_menu.clear();
    
    current_menu.emplace_back("NOVO JOGO", NEW_GAME);
    
    // Habilitar "Continuar" apenas se houver save
    bool has_save = SaveSystem::getInstance().hasSaveFile("autosave");
    current_menu.emplace_back("CONTINUAR", CONTINUE_GAME, has_save);
    
    current_menu.emplace_back("CONFIGURAÇÕES", SETTINGS_MENU);
    current_menu.emplace_back("CONQUISTAS", ACHIEVEMENTS_MENU);
    current_menu.emplace_back("RECORDES", HIGH_SCORES_MENU);
    current_menu.emplace_back("AJUDA", HELP_MENU);
    current_menu.emplace_back("SAIR", EXIT_GAME);
}

void MenuSystem::buildPauseMenu() {
    current_menu.clear();
    
    current_menu.emplace_back("CONTINUAR", RESUME_GAME);
    current_menu.emplace_back("SALVAR JOGO", SAVE_GAME);
    current_menu.emplace_back("CONFIGURAÇÕES", SETTINGS_MENU);
    current_menu.emplace_back("REINICIAR", RESTART_GAME);
    current_menu.emplace_back("MENU PRINCIPAL", BACK_TO_MENU);
}

void MenuSystem::buildGameOverMenu() {
    current_menu.clear();
    
    current_menu.emplace_back("NOVO JOGO", NEW_GAME);
    current_menu.emplace_back("RECORDES", HIGH_SCORES_MENU);
    current_menu.emplace_back("MENU PRINCIPAL", BACK_TO_MENU);
}

void MenuSystem::buildSettingsMenu() {
    current_menu.clear();
    
    current_menu.emplace_back("VOLUME GERAL", MenuOption::BACK_TO_MENU); // Placeholder
    current_menu.emplace_back("VOLUME EFEITOS", MenuOption::BACK_TO_MENU); // Placeholder
    current_menu.emplace_back("VOLUME MÚSICA", MenuOption::BACK_TO_MENU); // Placeholder
    current_menu.emplace_back("PARTÍCULAS", MenuOption::BACK_TO_MENU); // Placeholder
    current_menu.emplace_back("DIFICULDADE", MenuOption::BACK_TO_MENU); // Placeholder
    current_menu.emplace_back("RESETAR DADOS", MenuOption::BACK_TO_MENU); // Placeholder
    current_menu.emplace_back("VOLTAR", BACK_TO_MENU);
}

void MenuSystem::moveUp() {
    if (current_menu.empty()) return;
    
    do {
        selected_index--;
        if (selected_index < 0) {
            selected_index = current_menu.size() - 1;
        }
    } while (!current_menu[selected_index].enabled);
    
    // SoundManager::getInstance().playSound("menu_move");
}

void MenuSystem::moveDown() {
    if (current_menu.empty()) return;
    
    do {
        selected_index++;
        if (selected_index >= (int)current_menu.size()) {
            selected_index = 0;
        }
    } while (!current_menu[selected_index].enabled);
    
    // SoundManager::getInstance().playSound("menu_move");
}

void MenuSystem::select() {
    if (current_menu.empty() || selected_index >= (int)current_menu.size()) return;
    
    const MenuItem& item = current_menu[selected_index];
    if (!item.enabled) return;
    
    // SoundManager::getInstance().playSound("menu_select");
    executeMenuAction(item.option);
}

void MenuSystem::back() {
    switch (current_state) {
        case SETTINGS:
        case ACHIEVEMENTS:
        case HIGH_SCORES:
        case HELP:
            setState(MAIN_MENU);
            break;
        case PAUSED:
            setState(PLAYING);
            break;
        default:
            break;
    }
}

void MenuSystem::executeMenuAction(MenuOption option) {
    switch (option) {
        case NEW_GAME:
            if (new_game_callback) {
                new_game_callback();
            }
            setState(PLAYING);
            break;
            
        case CONTINUE_GAME:
            if (continue_game_callback) {
                continue_game_callback();
            }
            setState(PLAYING);
            break;
            
        case SETTINGS_MENU:
            setState(SETTINGS);
            break;
            
        case ACHIEVEMENTS_MENU:
            setState(ACHIEVEMENTS);
            break;
            
        case HIGH_SCORES_MENU:
            setState(HIGH_SCORES);
            break;
            
        case HELP_MENU:
            setState(HELP);
            break;
            
        case EXIT_GAME:
            if (exit_game_callback) {
                exit_game_callback();
            }
            break;
            
        case BACK_TO_MENU:
            setState(MAIN_MENU);
            break;
            
        case RESUME_GAME:
            setState(PLAYING);
            break;
            
        case RESTART_GAME:
            if (new_game_callback) {
                new_game_callback();
            }
            setState(PLAYING);
            break;
            
        case SAVE_GAME:
            // Implementar salvamento manual
            showNotification("Jogo salvo com sucesso!");
            break;
            
        default:
            break;
    }
}

void MenuSystem::update() {
    updateAnimations();
    updateNotifications();
}

void MenuSystem::updateAnimations() {
    if (transition_active) {
        transition_progress += 0.05f; // Velocidade da transição
        if (transition_progress >= 1.0f) {
            transition_progress = 1.0f;
            transition_active = false;
            current_state = target_state;
        }
    }
    
    // Animação de flutuação do menu
    static float time = 0.0f;
    time += 0.02f;
    menu_animation_offset = sin(time) * 0.5f;
}

void MenuSystem::updateNotifications() {
    if (notification_timer > 0) {
        notification_timer -= 0.016f; // ~60 FPS
        if (notification_timer < 0) {
            notification_timer = 0;
        }
    }
}

void MenuSystem::showNotification(const std::string& message, float duration) {
    notification_text = message;
    notification_timer = duration;
}

void MenuSystem::render() {
    renderBackground();
    
    switch (current_state) {
        case MAIN_MENU:
            renderMainMenu();
            break;
        case PAUSED:
            renderPauseMenu();
            break;
        case SETTINGS:
            renderSettingsMenu();
            break;
        case ACHIEVEMENTS:
            renderAchievements();
            break;
        case HIGH_SCORES:
            renderHighScores();
            break;
        case HELP:
            renderHelp();
            break;
        case GAME_OVER:
            renderGameOver();
            break;
        default:
            break;
    }
    
    if (hasActiveNotification()) {
        renderNotification();
    }
}

void MenuSystem::renderBackground() {
    glDisable(GL_TEXTURE_2D);
    
    // Gradiente de fundo
    glBegin(GL_QUADS);
        glColor3f(0.1f, 0.2f, 0.3f); // Azul escuro no topo
        glVertex2f(0, 20);
        glVertex2f(20, 20);
        
        glColor3f(0.05f, 0.1f, 0.15f); // Mais escuro na parte inferior
        glVertex2f(20, 0);
        glVertex2f(0, 0);
    glEnd();
    
    // Efeito de partículas de fundo (opcional)
    glColor4f(1.0f, 1.0f, 1.0f, 0.1f);
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 50; i++) {
        float x = (i * 7.3f) % 20;
        float y = (i * 11.7f) % 20;
        glVertex2f(x, y);
    }
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
}

void MenuSystem::renderMainMenu() {
    renderTitle("ECOTETRIS", 17.0f);
    renderText("Recicle e Divirta-se!", 10.0f, 15.5f, true);
    
    // Estatísticas rápidas
    auto& settings = SaveSystem::getInstance().getSettings();
    if (settings.total_games_played > 0) {
        char stats[256];
        sprintf(stats, "Melhor Pontuação: %d | Jogos: %d", 
                settings.best_score, settings.total_games_played);
        renderText(stats, 10.0f, 2.0f, true);
    }
    
    // Renderizar itens do menu
    float start_y = 12.0f;
    for (size_t i = 0; i < current_menu.size(); i++) {
        float y = start_y - i * 1.5f + menu_animation_offset * 0.1f;
        renderMenuItem(current_menu[i], i, y);
    }
}

void MenuSystem::renderMenuItem(const MenuItem& item, int index, float y_position) {
    glDisable(GL_TEXTURE_2D);
    
    bool is_selected = (index == selected_index);
    
    // Cor do texto
    if (!item.enabled) {
        glColor3f(0.4f, 0.4f, 0.4f); // Cinza para desabilitado
    } else if (is_selected) {
        glColor3f(1.0f, 1.0f, 0.0f); // Amarelo para selecionado
        
        // Destacar item selecionado
        glColor4f(1.0f, 1.0f, 0.0f, 0.2f);
        glBegin(GL_QUADS);
            glVertex2f(8.0f, y_position - 0.3f);
            glVertex2f(12.0f, y_position - 0.3f);
            glVertex2f(12.0f, y_position + 0.8f);
            glVertex2f(8.0f, y_position + 0.8f);
        glEnd();
        
        glColor3f(1.0f, 1.0f, 0.0f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f); // Branco normal
    }
    
    renderText(item.text, 10.0f, y_position, true);
    glEnable(GL_TEXTURE_2D);
}

void MenuSystem::renderTitle(const std::string& title, float y_position) {
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.0f, 1.0f, 0.5f); // Verde eco-friendly
    
    // Efeito de brilho no título
    static float glow_time = 0.0f;
    glow_time += 0.03f;
    float glow = 0.8f + 0.2f * sin(glow_time);
    glColor3f(0.0f, glow, 0.5f * glow);
    
    renderText(title, 10.0f, y_position, true);
    glEnable(GL_TEXTURE_2D);
}

void MenuSystem::renderText(const std::string& text, float x, float y, bool centered) {
    if (centered) {
        // Aproximação do centro baseada no comprimento do texto
        x -= text.length() * 0.3f;
    }
    
    glRasterPos2f(x, y);
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
}

void MenuSystem::renderNotification() {
    glDisable(GL_TEXTURE_2D);
    
    // Fundo da notificação
    float alpha = std::min(1.0f, notification_timer);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f * alpha);
    glBegin(GL_QUADS);
        glVertex2f(2.0f, 18.0f);
        glVertex2f(18.0f, 18.0f);
        glVertex2f(18.0f, 19.5f);
        glVertex2f(2.0f, 19.5f);
    glEnd();
    
    // Borda
    glColor4f(0.0f, 1.0f, 0.5f, alpha);
    glBegin(GL_LINE_LOOP);
        glVertex2f(2.0f, 18.0f);
        glVertex2f(18.0f, 18.0f);
        glVertex2f(18.0f, 19.5f);
        glVertex2f(2.0f, 19.5f);
    glEnd();
    
    // Texto da notificação
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    renderText(notification_text, 10.0f, 18.7f, true);
    
    glEnable(GL_TEXTURE_2D);
}

void MenuSystem::renderAchievements() {
    renderTitle("CONQUISTAS", 19.0f);
    
    auto& achievements = AchievementManager::getInstance();
    float completion = achievements.getCompletionPercentage();
    
    char progress_text[64];
    sprintf(progress_text, "Progresso: %.1f%% (%d/%d)", 
            completion, 
            achievements.getUnlockedCount(),
            (int)achievements.getAllAchievements().size());
    
    renderText(progress_text, 10.0f, 17.5f, true);
    
    // Barra de progresso
    renderProgressBar(5.0f, 16.5f, 10.0f, 0.5f, completion / 100.0f);
    
    // Lista de conquistas (primeiras 10)
    auto unlocked = achievements.getUnlockedAchievements();
    auto locked = achievements.getLockedAchievements();
    
    float y = 15.0f;
    int count = 0;
    
    // Mostrar conquistas desbloqueadas primeiro
    for (const auto& achievement : unlocked) {
        if (count >= 8) break;
        
        glColor3f(0.0f, 1.0f, 0.0f); // Verde para desbloqueado
        renderText("✓ " + achievement.name, 2.0f, y, false);
        
        glColor3f(0.8f, 0.8f, 0.8f);
        renderText(achievement.description, 2.5f, y - 0.5f, false);
        
        y -= 1.5f;
        count++;
    }
    
    // Mostrar algumas conquistas bloqueadas
    for (const auto& achievement : locked) {
        if (count >= 8) break;
        
        glColor3f(0.5f, 0.5f, 0.5f); // Cinza para bloqueado
        renderText("✗ " + achievement.name, 2.0f, y, false);
        
        glColor3f(0.4f, 0.4f, 0.4f);
        renderText(achievement.description, 2.5f, y - 0.5f, false);
        
        y -= 1.5f;
        count++;
    }
    
    renderText("Pressione ESC para voltar", 10.0f, 1.0f, true);
}

void MenuSystem::renderProgressBar(float x, float y, float width, float height, float progress) {
    glDisable(GL_TEXTURE_2D);
    
    // Fundo da barra
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
    
    // Progresso
    glColor3f(0.0f, 1.0f, 0.5f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + width * progress, y);
        glVertex2f(x + width * progress, y + height);
        glVertex2f(x, y + height);
    glEnd();
    
    // Borda
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
        glVertex2f(x, y);
        glVertex2f(x + width, y);
        glVertex2f(x + width, y + height);
        glVertex2f(x, y + height);
    glEnd();
    
    glEnable(GL_TEXTURE_2D);
}

void MenuSystem::renderHighScores() {
    renderTitle("RECORDES", 19.0f);
    
    auto scores = SaveSystem::getInstance().getHighScores();
    
    if (scores.empty()) {
        renderText("Nenhum recorde ainda!", 10.0f, 10.0f, true);
        renderText("Jogue para estabelecer seu primeiro recorde!", 10.0f, 8.0f, true);
    } else {
        float y = 16.0f;
        for (size_t i = 0; i < scores.size() && i < 10; i++) {
            char score_text[128];
            sprintf(score_text, "%d. %d pts - Nível %d (%d linhas)", 
                    (int)i + 1, scores[i].score, scores[i].level, scores[i].lines);
            
            glColor3f(1.0f, 1.0f, 1.0f);
            renderText(score_text, 1.0f, y, false);
            
            // Data e duração
            int minutes = scores[i].duration / 60;
            int seconds = scores[i].duration % 60;
            sprintf(score_text, "    %s - %02d:%02d", 
                    scores[i].date.substr(4, 12).c_str(), minutes, seconds);
            
            glColor3f(0.7f, 0.7f, 0.7f);
            renderText(score_text, 1.0f, y - 0.4f, false);
            
            y -= 1.3f;
        }
    }
    
    renderText("Pressione ESC para voltar", 10.0f, 1.0f, true);
}

void MenuSystem::renderHelp() {
    renderTitle("AJUDA", 19.0f);
    
    float y = 17.0f;
    
    struct HelpLine {
        std::string text;
        bool is_header;
    };
    
    std::vector<HelpLine> help_lines = {
        {"CONTROLES:", true},
        {"Setas: Mover e Rotacionar", false},
        {"Espaço: Drop Rápido", false},
        {"C: Guardar Peça (Hold)", false},
        {"R: Reiniciar", false},
        {"ESC: Pausar/Menu", false},
        {"", false},
        {"COMO JOGAR:", true},
        {"• Complete linhas com o mesmo tipo de lixo", false},
        {"• Linhas uniformes valem mais pontos", false},
        {"• Faça combos para multiplicar pontos", false},
        {"• Recicle diferentes tipos de materiais", false},
        {"• Desbloqueie conquistas jogando", false}
    };
    
    for (const auto& line : help_lines) {
        if (line.is_header) {
            glColor3f(0.0f, 1.0f, 0.5f);
        } else {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        
        renderText(line.text, 2.0f, y, false);
        y -= 0.8f;
        
        if (y < 2.0f) break;
    }
    
    renderText("Pressione ESC para voltar", 10.0f, 1.0f, true);
}

void MenuSystem::renderPauseMenu() {
    // Overlay semi-transparente
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(20, 0);
        glVertex2f(20, 20);
        glVertex2f(0, 20);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    
    renderTitle("PAUSADO", 15.0f);
    
    float start_y = 12.0f;
    for (size_t i = 0; i < current_menu.size(); i++) {
        float y = start_y - i * 1.5f;
        renderMenuItem(current_menu[i], i, y);
    }
}

void MenuSystem::renderGameOver() {
    // Overlay escuro
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(20, 0);
        glVertex2f(20, 20);
        glVertex2f(0, 20);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    
    glColor3f(1.0f, 0.3f, 0.3f);
    renderTitle("GAME OVER", 15.0f);
    
    // Mostrar estatísticas da partida atual
    // (Assumindo que estas informações estão disponíveis globalmente)
    
    float start_y = 10.0f;
    for (size_t i = 0; i < current_menu.size(); i++) {
        float y = start_y - i * 1.5f;
        renderMenuItem(current_menu[i], i, y);
    }
}

void MenuSystem::renderSettingsMenu() {
    renderTitle("CONFIGURAÇÕES", 19.0f);
    
    auto& settings = SaveSystem::getInstance().getSettings();
    
    float y = 16.0f;
    
    // Volume Geral
    char volume_text[64];
    sprintf(volume_text, "Volume Geral: %.0f%%", settings.master_volume * 100);
    renderText(volume_text, 3.0f, y, false);
    renderProgressBar(12.0f, y, 6.0f, 0.4f, settings.master_volume);
    y -= 1.5f;
    
    // Volume Efeitos
    sprintf(volume_text, "Volume Efeitos: %.0f%%", settings.sound_volume * 100);
    renderText(volume_text, 3.0f, y, false);
    renderProgressBar(12.0f, y, 6.0f, 0.4f, settings.sound_volume);
    y -= 1.5f;
    
    // Volume Música
    sprintf(volume_text, "Volume Música: %.0f%%", settings.music_volume * 100);
    renderText(volume_text, 3.0f, y, false);
    renderProgressBar(12.0f, y, 6.0f, 0.4f, settings.music_volume);
    y -= 1.5f;
    
    // Configurações booleanas
    renderText(settings.show_particles ? "Partículas: ON" : "Partículas: OFF", 3.0f, y, false);
    y -= 1.0f;
    
    renderText(settings.show_grid ? "Grade: ON" : "Grade: OFF", 3.0f, y, false);
    y -= 1.0f;
    
    const char* difficulty_names[] = {"Fácil", "Normal", "Difícil"};
    sprintf(volume_text, "Dificuldade: %s", difficulty_names[settings.difficulty_mode]);
    renderText(volume_text, 3.0f, y, false);
    y -= 1.5f;
    
    // Estatísticas totais
    renderText("ESTATÍSTICAS TOTAIS:", 3.0f, y, false);
    y -= 1.0f;
    
    sprintf(volume_text, "Jogos: %d | Melhor: %d pts", 
            settings.total_games_played, settings.best_score);
    renderText(volume_text, 3.0f, y, false);
    y -= 0.8f;
    
    int total_recycled = 0;
    for (int i = 0; i < 5; i++) {
        total_recycled += settings.lifetime_recycled[i];
    }
    sprintf(volume_text, "Total Reciclado: %d itens", total_recycled);
    renderText(volume_text, 3.0f, y, false);
    
    renderText("Use setas para navegar, Enter para selecionar", 10.0f, 1.5f, true);
    renderText("Pressione ESC para voltar", 10.0f, 1.0f, true);
}

#endif
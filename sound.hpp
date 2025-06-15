//sound.hpp
#ifndef SOUND_HPP
#define SOUND_HPP

#include <string>
#include <map>

class SoundManager {
public:
    static SoundManager& getInstance();
    
    void init();
    void cleanup();
    
    // Carregamento de sons
    bool loadSound(const std::string& name, const std::string& filename);
    bool loadMusic(const std::string& name, const std::string& filename);
    
    // Reprodução
    void playSound(const std::string& name, float volume = 1.0f);
    void playMusic(const std::string& name, bool loop = true, float volume = 0.5f);
    void stopMusic();
    
    // Controle de volume
    void setSoundVolume(float volume);
    void setMusicVolume(float volume);
    void setMasterVolume(float volume);
    
    // Estados
    bool isSoundEnabled() const { return sound_enabled; }
    bool isMusicEnabled() const { return music_enabled; }
    void toggleSound() { sound_enabled = !sound_enabled; }
    void toggleMusic() { music_enabled = !music_enabled; if(!music_enabled) stopMusic(); }

private:
    SoundManager() = default;
    ~SoundManager() = default;
    
    bool sound_enabled = true;
    bool music_enabled = true;
    float master_volume = 1.0f;
    float sound_volume = 1.0f;
    float music_volume = 0.5f;
    
    // Mapa de sons carregados (simulado - em implementação real usaria SDL_mixer ou FMOD)
    std::map<std::string, int> loaded_sounds;
    std::map<std::string, int> loaded_music;
};

// Implementação simplificada (para demonstração)
SoundManager& SoundManager::getInstance() {
    static SoundManager instance;
    return instance;
}

void SoundManager::init() {
    // Inicializar sistema de áudio
    // Em implementação real: Mix_OpenAudio, FMOD::System::create, etc.
    
    // Carregar sons do jogo
    loadSound("piece_move", "sounds/move.wav");
    loadSound("piece_rotate", "sounds/rotate.wav");
    loadSound("piece_drop", "sounds/drop.wav");
    loadSound("line_clear", "sounds/recycle.wav");
    loadSound("level_up", "sounds/level_up.wav");
    loadSound("game_over", "sounds/game_over.wav");
    loadSound("combo", "sounds/combo.wav");
    
    // Carregar música
    loadMusic("main_theme", "music/eco_theme.ogg");
    loadMusic("game_over_theme", "music/game_over.ogg");
}

void SoundManager::playSound(const std::string& name, float volume) {
    if (!sound_enabled) return;
    // Em implementação real: Mix_PlayChannel ou FMOD equivalente
    // printf("Playing sound: %s at volume %.2f\n", name.c_str(), volume * sound_volume * master_volume);
}

void SoundManager::playMusic(const std::string& name, bool loop, float volume) {
    if (!music_enabled) return;
    // Em implementação real: Mix_PlayMusic ou FMOD equivalente
    // printf("Playing music: %s\n", name.c_str());
}

bool SoundManager::loadSound(const std::string& name, const std::string& filename) {
    // Simulação de carregamento
    loaded_sounds[name] = 1; // Em implementação real: Mix_LoadWAV, etc.
    return true;
}

bool SoundManager::loadMusic(const std::string& name, const std::string& filename) {
    // Simulação de carregamento
    loaded_music[name] = 1; // Em implementação real: Mix_LoadMUS, etc.
    return true;
}

void SoundManager::stopMusic() {
    // Em implementação real: Mix_HaltMusic()
}

void SoundManager::setSoundVolume(float volume) {
    sound_volume = volume;
}

void SoundManager::setMusicVolume(float volume) {
    music_volume = volume;
}

void SoundManager::setMasterVolume(float volume) {
    master_volume = volume;
}

void SoundManager::cleanup() {
    // Limpar recursos de áudio
    loaded_sounds.clear();
    loaded_music.clear();
}

#endif
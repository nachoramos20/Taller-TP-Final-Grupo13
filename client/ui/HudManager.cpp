#include "HudManager.h"

#include "../config/ClientConfig.h"

HudManager::HudManager(SDL2pp::Renderer& renderer, GameAudioService* audio):
        _chat(std::make_unique<ChatWidget>(renderer, ClientConfig::instance().fonts.chat_font_path,
                                           ClientConfig::instance().fonts.chat_font_size)),
        _stats(std::make_unique<StatsPanel>(renderer, ClientConfig::instance().fonts.chat_font_path,
                                            ClientConfig::instance().fonts.medium_font_size,
                                            audio)),
        _inventory(std::make_unique<InventoryPanel>(
                renderer, ClientConfig::instance().fonts.chat_font_path,
                ClientConfig::instance().fonts.small_font_size, audio)),
        _pos_label(std::make_unique<PositionLabel>(
                renderer, ClientConfig::instance().fonts.chat_font_path,
                ClientConfig::instance().fonts.medium_font_size)) {}

void HudManager::render(int screen_w, int screen_h) {
    _chat->render(screen_w, screen_h);
    _stats->render(screen_w, screen_h);
    _inventory->render(screen_w, screen_h);
    _pos_label->render(screen_w, screen_h);
}

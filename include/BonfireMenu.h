#pragma once

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

// 前向声明
struct Bonfire;

class BonfireMenu {
public:
    static BonfireMenu* GetSingleton() {
        static BonfireMenu singleton;
        return &singleton;
    }

    void Open(const Bonfire& bonfire);
    void Close();

private:
    BonfireMenu() = default;
    
    // UI回调函数
    void OnOptionSelect(std::uint32_t index);
    
    // 传送相关函数
    void ShowTeleportMenu();
    void RegisterTeleportOption(const std::string& name, const Bonfire* target);
    void TeleportToBonfire(const Bonfire* target);
    
    // 存档相关函数
    void SaveGame();
    std::string GenerateSaveName() const;
    void ShowSaveNotification(bool success);

    // 其他功能函数
    void Rest();

    // 成员变量
    const Bonfire* currentBonfire{nullptr};
    RE::IMenu* menuInterface{nullptr};
    std::vector<const Bonfire*> teleportTargets;  // 存储可传送的目标篝火
};

// UI相关常量
constexpr const char* MENU_PATH = "BonfireMenu";
constexpr const char* MENU_NAME = "Bonfire Menu";

// 传送相关常量
constexpr float TELEPORT_HEIGHT = 128.0f;    // 传送时的上升高度

// 存档相关常量
constexpr const char* SAVE_PREFIX = "BONFIRE_";  // 存档前缀
constexpr const char* SAVE_SUCCESS_MSG = "篝火存档已创建";
constexpr const char* SAVE_FAILED_MSG = "存档失败";
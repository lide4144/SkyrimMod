#include "BonfireMenu.h"

void BonfireMenu::Open(const Bonfire& bonfire) {
    // 存储当前篝火引用
    currentBonfire = &bonfire;
    
    // 获取UI管理器
    auto ui = RE::UI::GetSingleton();
    if (!ui) return;

    // 创建菜单参数
    RE::UIMessageQueue::Message msg;
    msg.menu = MENU_NAME;
    msg.type = RE::UI_MESSAGE_TYPE::kShow;
    
    // 发送打开菜单消息
    ui->AddMessage(msg);
    
    // 注册菜单选项
    auto scaleformManager = RE::BSScaleformManager::GetSingleton();
    if (scaleformManager) {
        // 添加菜单选项
        RegisterMenuOption("休息", 0);
        RegisterMenuOption("传送", 1);
        RegisterMenuOption("存档", 2);
        RegisterMenuOption("离开", 3);
    }
}

void BonfireMenu::Close() {
    auto ui = RE::UI::GetSingleton();
    if (!ui) return;

    // 发送关闭菜单消息
    RE::UIMessageQueue::Message msg;
    msg.menu = MENU_NAME;
    msg.type = RE::UI_MESSAGE_TYPE::kHide;
    ui->AddMessage(msg);
    
    currentBonfire = nullptr;
    menuInterface = nullptr;
}

void BonfireMenu::OnOptionSelect(std::uint32_t index) {
    if (!currentBonfire) return;

    if (index < teleportTargets.size()) {
        // 处理传送选项
        TeleportToBonfire(teleportTargets[index]);
    } else {
        // 处理主菜单选项
        switch (index - teleportTargets.size()) {
            case 0: // 休息
                Rest();
                break;
            case 1: // 传送
                ShowTeleportMenu();
                break;
            case 2: // 存档
                SaveGame();
                break;
            case 3: // 离开
                Close();
                break;
        }
    }
}

// 休息功能
void BonfireMenu::Rest() {
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;

    // 恢复生命值
    player->RestoreActorValue(RE::ActorValue::kHealth, 999999.0f);
    
    // 播放休息动画
    player->PlayIdle(RE::TESForm::LookupByID<RE::TESIdleForm>(0x000000)); // 需要在Creation Kit中创建
    
    // 等待3秒
    SKSE::GetTaskInterface()->AddTask([this]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        Close();
    });
}

// 传送菜单
void BonfireMenu::ShowTeleportMenu() {
    // 清空之前的传送目标列表
    teleportTargets.clear();
    
    // 获取所有可传送的篝火
    for (const auto& bonfire : bonfires) {
        if (bonfire.isDiscovered && &bonfire != currentBonfire) {
            teleportTargets.push_back(&bonfire);
            RegisterTeleportOption(bonfire.name, &bonfire);
        }
    }
}

void BonfireMenu::RegisterTeleportOption(const std::string& name, const Bonfire* target) {
    if (!menuInterface) return;
    
    // 使用Scaleform注册菜单选项
    auto ui = RE::BSScaleformManager::GetSingleton();
    if (ui) {
        RE::GFxValue args[2];
        args[0].SetString(name.c_str());
        args[1].SetNumber(teleportTargets.size() - 1);
        
        menuInterface->Invoke("_root.addTeleportOption", nullptr, args, 2);
    }
}

void BonfireMenu::TeleportToBonfire(const Bonfire* target) {
    if (!target || !target->fireRef) return;
    
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player) return;
    
    // 获取目标位置并调整高度
    RE::NiPoint3 targetPos = target->fireRef->GetPosition();
    targetPos.z += TELEPORT_HEIGHT;  // 稍微升高一点，防止卡地形
    
    // 传送玩家到调整后的位置
    player->MoveTo(target->fireRef, targetPos, target->fireRef->GetAngle());
    
    Close();
}

// 存档功能
void BonfireMenu::SaveGame() {
    if (!currentBonfire) return;

    // 检查玩家状态
    auto player = RE::PlayerCharacter::GetSingleton();
    if (!player || player->IsInCombat()) {
        ShowSaveNotification(false);
        return;
    }

    // 尝试存档
    auto saveLoadManager = RE::BGSSaveLoadManager::GetSingleton();
    if (!saveLoadManager) {
        ShowSaveNotification(false);
        return;
    }

    // 生成存档名称并保存
    std::string saveName = GenerateSaveName();
    bool success = saveLoadManager->Save(saveName.c_str());
    
    ShowSaveNotification(success);
    if (success) {
        Close();
    }
}

std::string BonfireMenu::GenerateSaveName() const {
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    // 格式化时间字符串
    char timeStr[20];
    std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", std::localtime(&time));

    // 构建存档名称：前缀_位置_时间
    std::string locationName = currentBonfire->name;
    // 移除非法字符
    locationName.erase(std::remove_if(locationName.begin(), locationName.end(), 
        [](char c) { return !std::isalnum(c) && c != '_'; }), locationName.end());

    return fmt::format("{}{}_{}_{}", SAVE_PREFIX, locationName, 
        RE::PlayerCharacter::GetSingleton()->GetLevel(), timeStr);
}

void BonfireMenu::ShowSaveNotification(bool success) {
    RE::DebugNotification(success ? SAVE_SUCCESS_MSG : SAVE_FAILED_MSG);
} 
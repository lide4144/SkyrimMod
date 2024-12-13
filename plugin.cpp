#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"
#include "BonfireMenu.h"

// 篝火数据结构
struct Bonfire {
    RE::TESObjectREFR* fireRef;  // 篝火的引用
    std::string name;            // 篝火名称
    bool isDiscovered;           // 是否已发现
};

// 存储所有篝火的容器
std::vector<Bonfire> bonfires;

// 创建篝火的函数
void CreateBonfire(const RE::TESObjectREFR* playerRef, const char* name) {
    // 获取篝火基础物体(这里需要你在Creation Kit中创建一个篝火物体并获取其ID)
    RE::TESForm* bonfireBase = RE::TESForm::LookupByID(你在Creation Kit中创建的FormID);
    if (!bonfireBase) return;

    // 在玩家位置创建篝火
    RE::TESObjectREFR* newBonfire = playerRef->PlaceAtMe(bonfireBase, 1, false, false);
    if (!newBonfire) return;

    // 添加到篝火列表
    bonfires.push_back({newBonfire, name, true});
}

// 处理篝火激活事件
class BonfireActivateHandler : public RE::BSTEventSink<RE::TESActivateEvent> {
    RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* event, RE::BSTEventSource<RE::TESActivateEvent>*) override {
        if (!event) return RE::BSEventNotifyControl::kContinue;

        // 检查激活的物体是否是篝火
        auto it = std::find_if(bonfires.begin(), bonfires.end(),
            [event](const Bonfire& b) { return b.fireRef == event->objectActivated; });

        if (it != bonfires.end()) {
            // 显示篝火菜单
            ShowBonfireMenu(*it);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

// 修改ShowBonfireMenu函数的实现
void ShowBonfireMenu(const Bonfire& bonfire) {
    BonfireMenu::GetSingleton()->Open(bonfire);
}

// 插件加载入口
SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);

    // 注册事件处理
    auto eventHandler = new BonfireActivateHandler();
    RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(eventHandler);

    // 在游戏加载完成后创建初始篝火
    SKSE::GetMessagingInterface()->RegisterListener([](SKSE::MessagingInterface::Message* message) {
        if (message->type == SKSE::MessagingInterface::kDataLoaded) {
            auto player = RE::PlayerCharacter::GetSingleton();
            if (player) {
                CreateBonfire(player, "初始篝火");
            }
        }
    });

    return true;
}
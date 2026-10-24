#include "SurvivorManager.h"
#include <System/Scene.h>
#include "Game/Scene/PoittersPoint_Clear.h"

SurvivorManager& SurvivorManager::Instance()
{
    static SurvivorManager instance;
    return instance;
}

void SurvivorManager::Reset(int total)
{
    alive_ = total;
    winner_.clear();
}

void SurvivorManager::OnDeath(const std::string& name)
{
    if(alive_ > 0) alive_--;
    if(alive_ == 1 && winner_.empty()) {
        winner_ = name;
        Scene::Change(Scene::GetScene<PoittersPoint::PoittersPoint_Clear>());
    }
}

int SurvivorManager::GetAliveCount() const { return alive_; }

bool SurvivorManager::HasWinner() const { return !winner_.empty() && alive_ <= 1; }

const std::string& SurvivorManager::GetWinner() const { return winner_; }
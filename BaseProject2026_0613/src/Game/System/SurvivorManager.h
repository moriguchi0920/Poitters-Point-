#pragma once
#include <vector>
#include <string>

class SurvivorManager
{
public:
    static SurvivorManager& Instance();

    void               Reset(int total);
    void               OnDeath(const std::string& name);
    int                GetAliveCount() const;
    bool               HasWinner() const;
    const std::string& GetWinner() const;

private:
    SurvivorManager()  = default;
    int         alive_ = 0;
    std::string winner_;
};

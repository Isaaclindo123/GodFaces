#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

std::map<int, int> levelRank;

std::string getGodTier(int rank) {
    if (rank == 1) return "Ultra Omega God";
    if (rank <= 25) return "Omega God";
    if (rank <= 100) return "Ultra God";
    if (rank <= 250) return "Hiper God";
    if (rank <= 300) return "Super God";
    if (rank <= 500) return "The God";
    if (rank <= 1000) return "God";
    return "";
}

std::string getGodSprite(int rank) {
    if (rank == 1) return "brilliance.png";
    if (rank <= 25) return "wise.png";
    if (rank <= 100) return "genius.png";
    if (rank <= 250) return "mirthful.png";
    if (rank <= 300) return "perfected.png";
    if (rank <= 500) return "intellectual.png";
    if (rank <= 1000) return "god.png";
    return "";
}

void loadFromAPI() {
    std::thread([]() {
        auto res = web::WebRequest().getSync("https://api.aredl.net/v2/api/aredl/pack-tiers");

        if (!res.ok()) {
            log::error("Erro na requisição: {}", res.code());
            return;
        }

        auto jsonResult = res.json();
        if (!jsonResult) {
            log::error("Erro ao parsear JSON!");
            return;
        }

        auto& tiers = jsonResult.unwrap();
        if (!tiers.isArray()) return;

        for (auto& tier : tiers.asArray().unwrap()) {
            if (!tier["packs"].isArray()) continue;

            for (auto& pack : tier["packs"].asArray().unwrap()) {
                if (!pack["levels"].isArray()) continue;

                for (auto& lvl : pack["levels"].asArray().unwrap()) {
                    auto idRes = lvl["level_id"].asInt();
                    auto rankRes = lvl["position"].asInt();

                    if (idRes && rankRes) {
                        int id = idRes.unwrap();
                        int rank = rankRes.unwrap();

                        if (levelRank.find(id) == levelRank.end() || rank < levelRank[id]) {
                            levelRank[id] = rank;
                        }
                    }
                }
            }
        }

        log::info("AERDL carregada! {} níveis", levelRank.size());
    }).detach();
}

class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        int levelID = level->m_levelID;

        if (levelRank.count(levelID)) {
            int rank = levelRank[levelID];
            auto tier = getGodTier(rank);

            auto spriteName = getGodSprite(rank);
            auto spritePath = Mod::get()->getResourcesDir() / spriteName;
            auto sprite = CCSprite::create(spritePath.string().c_str());
            if (sprite) {
                sprite->setScale(0.5f);
                sprite->setPosition({200, 260});
                this->addChild(sprite, 10);
            }

            auto label = CCLabelBMFont::create(tier.c_str(), "goldFont.fnt");
            label->setScale(0.6f);
            label->setPosition({200, 215});
            this->addChild(label, 10);
        }

        return true;
    }
};

$on_mod(Loaded) {
    auto searchPath = Mod::get()->getResourcesDir().string();
    cocos2d::CCFileUtils::sharedFileUtils()->addSearchPath(searchPath.c_str());

    std::vector<std::string> incompatibleMods = {
        "ultrasoda.grandpa_demon_revived",
        "itzkiba.grandpa_demon"
    };

    for (auto& mod : incompatibleMods) {
        if (Loader::get()->isModLoaded(mod)) {
            std::string msg = "Conflito com: " + mod;
            FLAlertLayer::create("Incompatibility", msg.c_str(), "OK")->show();
            break;
        }
    }

    loadFromAPI();
}

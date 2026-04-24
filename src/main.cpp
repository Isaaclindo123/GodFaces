#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/utils/web.hpp>

using namespace geode::prelude;

// ==========================
// Armazena ranks
std::map<int, int> levelRank;

// ==========================
// Tier por rank
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

// ==========================
// Sprite por rank
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

// ==========================
// Carregar API AERDL (numa thread separada pra não travar o jogo)
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

        auto& data = jsonResult.unwrap();
        if (!data.isArray()) {
            log::error("JSON não é um array!");
            return;
        }

        for (auto& lvl : data.asArray().unwrap()) {
            auto idRes = lvl["id"].asInt();
            auto rankRes = lvl["rank"].asInt();
            if (idRes && rankRes) {
                levelRank[idRes.unwrap()] = rankRes.unwrap();
            }
        }

        log::info("AERDL carregada! {} níveis", levelRank.size());
    }).detach();
}

// ==========================
// Hook no Level Info
class $modify(MyLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool p1) {
        if (!LevelInfoLayer::init(level, p1)) return false;

        int levelID = level->m_levelID;

        if (levelRank.count(levelID)) {
            int rank = levelRank[levelID];

            auto tier = getGodTier(rank);

            auto label = CCLabelBMFont::create(
                tier.c_str(),
                "goldFont.fnt"
            );
            label->setScale(0.6f);
            label->setPosition({200, 80});
            this->addChild(label);

            auto sprite = CCSprite::create(getGodSprite(rank).c_str());
            if (sprite) {
                sprite->setScale(0.5f);
                sprite->setPosition({200, 130});
                this->addChild(sprite);
            }
        }

        return true;
    }
};

// ==========================
// Inicialização do mod
$on_mod(Loaded) {
    std::vector<std::string> incompatibleMods = {
        "ultrasoda.grandpa_demon_revived",
        "itzkiba.grandpa_demon"
    };

    for (auto& mod : incompatibleMods) {
        if (Loader::get()->isModLoaded(mod)) {
            std::string msg = "Conflito com: " + mod;
            FLAlertLayer::create(
                "Incompatibility",
                msg.c_str(),
                "OK"
            )->show();
            break;
        }
    }

    loadFromAPI();
}

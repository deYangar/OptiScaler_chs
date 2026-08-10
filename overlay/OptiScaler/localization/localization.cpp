#include "pch.h"
#include "localization.h"
#include "lang_en.h"
#include "lang_zh_cn.h"

void LocalizationManager::Init() {
    tables_[Language::English] = {};
    tables_[Language::ChineseSimplified] = {};
    InitEnglishTable(tables_[Language::English]);
    InitChineseSimplifiedTable(tables_[Language::ChineseSimplified]);
}

void LocalizationManager::SetLanguage(Language lang) {
    currentLang_ = lang;
}

const char* LocalizationManager::Get(LK key) const {
    auto langIt = tables_.find(currentLang_);
    if (langIt != tables_.end()) {
        auto keyIt = langIt->second.find(static_cast<int>(key));
        if (keyIt != langIt->second.end() && keyIt->second != nullptr && keyIt->second[0] != '\0') {
            return keyIt->second;
        }
    }
    // Fallback to English
    auto enIt = tables_.find(Language::English);
    if (enIt != tables_.end()) {
        auto keyIt = enIt->second.find(static_cast<int>(key));
        if (keyIt != enIt->second.end() && keyIt->second != nullptr) {
            return keyIt->second;
        }
    }
    return "[MISSING]";
}

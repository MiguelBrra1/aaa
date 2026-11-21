// (c) JasperEngine contributors
// SPDX-License-Identifier: MIT

#include "EditorLang.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <unordered_map>

using namespace jasperengine::editor;

namespace {

    struct EditorLangEntry { const char* english; const char* translated; };
    struct EditorLangTable { const char* code; const EditorLangEntry* entries; int count; };

#include "lang/EditorLangTables.inc"

    struct State {
        std::string code = "en";
        // English -> translation of the current language (views into the static tables)
        std::unordered_map<std::string_view, std::string_view> map;
        size_t minLength = 0;
        size_t maxLength = 0;
    };

    State& state(){
        static State s;
        return s;
    }

    // HoneyEngine: the product name the editor shows. Code, files and settings keep the
    // upstream name; only what is drawn changes. Paths are left alone so a folder on disk is
    // shown as it is really called.
    constexpr std::string_view kUpstreamName = "JasperEngine";
    constexpr std::string_view kProductName = "HoneyEngine";

    bool looksLikePath(std::string_view text){
        return text.find('/') != std::string_view::npos || text.find('\\') != std::string_view::npos;
    }

    // the rebranded text of `text`, kept for the editor's lifetime (the views stay valid)
    std::string_view rebranded(std::string_view text){
        static std::unordered_map<std::string, std::string> cache;
        auto it = cache.find(std::string(text));
        if (it != cache.end()) return it->second;
        std::string out(text);
        for (size_t at = out.find(kUpstreamName); at != std::string::npos; at = out.find(kUpstreamName, at + kProductName.size()))
            out.replace(at, kUpstreamName.size(), kProductName);
        if (cache.size() > 4096) cache.clear();   // bounded: titles and labels, not a log
        return cache.emplace(std::string(text), std::move(out)).first->second;
    }

    // a menu label with a trailing shortcut hint or ellipsis matches only as written; the
    // lookup is exact so user text that merely contains a word is never touched
    bool translateHook(const char* text, const char* text_end, const char** out_text, const char** out_text_end, void*){
        const State& s = state();
        const size_t length = (size_t)(text_end - text);
        std::string_view shown(text, length);
        bool replaced = false;
        if (!s.map.empty() && length >= s.minLength && length <= s.maxLength){
            auto it = s.map.find(shown);
            if (it != s.map.end()){ shown = it->second; replaced = true; }
        }
        if (shown.find(kUpstreamName) != std::string_view::npos && !looksLikePath(shown)){
            shown = rebranded(shown);
            replaced = true;
        }
        if (!replaced) return false;
        *out_text = shown.data();
        *out_text_end = shown.data() + shown.size();
        return true;
    }

    const EditorLangTable* findTable(const std::string& code){
        for (const EditorLangTable& table : kEditorLangTables)
            if (code == table.code) return &table;
        return nullptr;
    }

}

void EditorLang::set(const std::string& requested){
    State& s = state();
    const EditorLangTable* table = requested == "en" ? nullptr : findTable(requested);
    s.code = table ? requested : "en";
    s.map.clear();
    s.minLength = SIZE_MAX;
    s.maxLength = 0;
    ImGui::SetTextTranslateHook(&translateHook, nullptr);
    if (!table) return;
    s.map.reserve((size_t)table->count);
    for (int i = 0; i < table->count; i++){
        const std::string_view en(table->entries[i].english);
        s.map.emplace(en, std::string_view(table->entries[i].translated));
        s.minLength = std::min(s.minLength, en.size());
        s.maxLength = std::max(s.maxLength, en.size());
    }
}

const std::string& EditorLang::get(){
    return state().code;
}

std::vector<std::string> EditorLang::available(){
    std::vector<std::string> codes = {"en"};
    for (const EditorLangTable& table : kEditorLangTables) codes.push_back(table.code);
    return codes;
}

const char* EditorLang::displayName(const std::string& code){
    if (code == "en") return "English";
    if (code == "es") return "Espa\xc3\xb1ol";
    if (code == "pt") return "Portugu\xc3\xaas";
    if (code == "fr") return "Fran\xc3\xa7" "ais";
    return "?";
}

const char* EditorLang::tr(const char* english){
    const State& s = state();
    if (s.map.empty() || english == nullptr) return english;
    auto it = s.map.find(std::string_view(english));
    return it == s.map.end() ? english : it->second.data();   // (the hook rebrands what is drawn)
}

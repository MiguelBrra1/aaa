// (c) JasperEngine contributors
// SPDX-License-Identifier: MIT

#include "Localization.h"

#include "TitleArtLanguage.h"
#include "io/FileData.h"
#include "System.h"
#include "Log.h"

#include <algorithm>
#include <memory>
#include <unordered_map>

using namespace jasperengine;

namespace {

    struct Tables{
        std::vector<std::string> languages;                               // column order, first table first
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> text;  // key -> lang -> text
        std::vector<std::string> keys;                                    // first-seen order
        bool autoTried = false;
    };

    Tables& tables(){
        static Tables t;
        return t;
    }

    // one CSV record per call; false at the end of the text
    bool readRecord(const std::string& s, size_t& pos, std::vector<std::string>& out){
        out.clear();
        if (pos >= s.size()) return false;
        std::string cell;
        bool quoted = false;
        while (pos < s.size()){
            const char c = s[pos];
            if (quoted){
                if (c == '"'){
                    if (pos + 1 < s.size() && s[pos + 1] == '"'){ cell += '"'; pos += 2; continue; }
                    quoted = false; pos++; continue;
                }
                cell += c; pos++; continue;
            }
            if (c == '"' && cell.empty()){ quoted = true; pos++; continue; }
            if (c == ','){ out.push_back(cell); cell.clear(); pos++; continue; }
            if (c == '\r' || c == '\n'){
                pos++;
                if (c == '\r' && pos < s.size() && s[pos] == '\n') pos++;
                break;
            }
            cell += c; pos++;
        }
        out.push_back(cell);
        return true;
    }

    std::string trim(const std::string& s){
        size_t a = 0, b = s.size();
        while (a < b && (s[a] == ' ' || s[a] == '\t')) a++;
        while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t')) b--;
        return s.substr(a, b - a);
    }

    std::string unescape(const std::string& s){
        std::string out;
        out.reserve(s.size());
        for (size_t i = 0; i < s.size(); i++){
            if (s[i] == '\\' && i + 1 < s.size()){
                if (s[i + 1] == 'n'){ out += '\n'; i++; continue; }
                if (s[i + 1] == 't'){ out += '\t'; i++; continue; }
                if (s[i + 1] == '\\'){ out += '\\'; i++; continue; }
            }
            out += s[i];
        }
        return out;
    }

    const std::string* cell(const std::string& key, const std::string& language){
        const Tables& t = tables();
        auto row = t.text.find(key);
        if (row == t.text.end()) return nullptr;
        auto it = row->second.find(language);
        if (it == row->second.end() || it->second.empty()) return nullptr;
        return &it->second;
    }

}

bool Localization::loadFromString(const std::string& csvText, const std::string& source, std::string* error){
    ensureLoaded();     // localization.csv first, so an explicit table merges over it
    std::string csv = csvText;
    if (csv.size() >= 3 && (unsigned char)csv[0] == 0xEF && (unsigned char)csv[1] == 0xBB && (unsigned char)csv[2] == 0xBF)
        csv = csv.substr(3);

    size_t pos = 0;
    std::vector<std::string> header;
    // the header: the first record that is not empty
    while (readRecord(csv, pos, header)){
        if (!(header.size() == 1 && trim(header[0]).empty())) break;
    }
    if (header.size() < 2){
        if (error) *error = "'" + source + "' has no language columns (the first row is key,<lang>,<lang>...)";
        return false;
    }

    std::vector<std::string> columns;   // "" for a column to skip
    for (size_t i = 1; i < header.size(); i++){
        const std::string tag = TitleArtLanguage::canonicalize(trim(header[i]));
        if (!TitleArtLanguage::isValid(tag)){
            Log::warn("Localization: '%s' column '%s' is not a language code, skipped", source.c_str(), header[i].c_str());
            columns.push_back("");
            continue;
        }
        columns.push_back(tag);
    }

    Tables& t = tables();
    const std::string oldLanguage = getLanguage();
    for (const std::string& lang : columns)
        if (!lang.empty() && std::find(t.languages.begin(), t.languages.end(), lang) == t.languages.end())
            t.languages.push_back(lang);

    std::vector<std::string> row;
    while (readRecord(csv, pos, row)){
        if (row.empty()) continue;
        const std::string key = trim(row[0]);
        if (key.empty() || key[0] == '#') continue;
        auto& entry = t.text[key];
        if (entry.empty()) t.keys.push_back(key);
        for (size_t i = 1; i < row.size() && i - 1 < columns.size(); i++){
            if (columns[i - 1].empty()) continue;
            entry[columns[i - 1]] = unescape(row[i]);
        }
    }

    if (getLanguage() != oldLanguage) getOnLanguageChanged().call(getLanguage());
    return true;
}

bool Localization::load(const std::string& path, std::string* error){
    std::unique_ptr<FileData> file(FileData::newFile(path.c_str()));
    if (!file || file->length() == 0){
        if (error) *error = "'" + path + "' could not be read";
        return false;
    }
    return loadFromString(file->readString(), path, error);
}

void Localization::ensureLoaded(){
    Tables& t = tables();
    if (t.autoTried) return;
    t.autoTried = true;
    for (const char* path : {"lua://localization.csv", "localization.csv"}){
        std::unique_ptr<FileData> file(FileData::newFile(path));
        if (file && file->length() > 0){
            std::string error;
            if (!loadFromString(file->readString(), path, &error)) Log::warn("Localization: %s", error.c_str());
            return;
        }
    }
}

void Localization::clear(){
    tables() = Tables();
}

std::string Localization::getLanguage(){
    ensureLoaded();
    const Tables& t = tables();
    if (t.languages.empty()) return TitleArtLanguage::global();
    std::vector<std::string> preferred;
    try { preferred = System::instance().getPreferredLocales(); } catch (...) {}
    const std::string found = TitleArtLanguage::resolve(TitleArtLanguage::candidates("", preferred, ""), t.languages);
    return found.empty() ? t.languages.front() : found;
}

std::string Localization::get(const std::string& key){
    ensureLoaded();
    const Tables& t = tables();
    if (t.languages.empty()) return key;
    const std::string lang = getLanguage();
    if (const std::string* s = cell(key, lang)) return *s;
    const std::string def = TitleArtLanguage::match(TitleArtLanguage::projectDefault(), t.languages);
    if (!def.empty()) if (const std::string* s = cell(key, def)) return *s;
    for (const std::string& l : t.languages) if (const std::string* s = cell(key, l)) return *s;
    return key;
}

std::string Localization::getIn(const std::string& key, const std::string& language){
    ensureLoaded();
    const std::string lang = TitleArtLanguage::match(language, tables().languages);
    const std::string* s = lang.empty() ? nullptr : cell(key, lang);
    return s ? *s : std::string();
}

bool Localization::has(const std::string& key){
    ensureLoaded();
    return tables().text.count(key) > 0;
}

std::string Localization::substitute(const std::string& text, const std::map<std::string, std::string>& args){
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); i++){
        if (text[i] == '{'){
            const size_t end = text.find('}', i + 1);
            if (end != std::string::npos){
                auto it = args.find(text.substr(i + 1, end - i - 1));
                if (it != args.end()){ out += it->second; i = end; continue; }
            }
        }
        out += text[i];
    }
    return out;
}

std::string Localization::format(const std::string& key, const std::map<std::string, std::string>& args){
    return substitute(get(key), args);
}

bool Localization::setLanguage(const std::string& code, std::string* error){
    getOnLanguageChanged();     // the listener exists before the first change
    return TitleArtLanguage::setGlobal(code, error);
}

std::vector<std::string> Localization::getLanguages(){
    ensureLoaded();
    return tables().languages;
}

std::vector<std::string> Localization::getKeys(){
    ensureLoaded();
    return tables().keys;
}

FunctionSubscribe<void(const std::string&)>& Localization::getOnLanguageChanged(){
    static FunctionSubscribe<void(const std::string&)>* event = []{
        auto* e = new FunctionSubscribe<void(const std::string&)>();
        TitleArtLanguage::addListener([e](const std::string&){ e->call(Localization::getLanguage()); });
        return e;
    }();
    return *event;
}

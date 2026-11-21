// (c) JasperEngine contributors
// SPDX-License-Identifier: MIT

#ifndef LOCALIZATION_H
#define LOCALIZATION_H

// Localization: string tables for a game's text in any number of languages (Unity's
// Localization String Tables).
//
// - Tables are CSV files (UTF-8, comma separated, quotes allowed) whose first row is
//   `key,<lang>,<lang>,...` with BCP 47 codes, for example:
//
//       key,en,es,fr,pt-BR
//       menu.play,Play,Jugar,Jouer,Jogar
//       hud.coins,"Coins: {count}","Monedas: {count}","Pièces : {count}","Moedas: {count}"
//
//   `\n` in a cell is a line break. Empty rows and rows whose key starts with `#` are skipped.
// - `localization.csv` beside the game's scripts loads by itself on the first lookup; more
//   tables load with load() and merge (a later table wins for a repeated key).
// - The language is the one TitleArt uses (TitleArt.setLanguage and Localization.setLanguage
//   set the same global value): the set language, else the device's preferred locales, else
//   Project Settings > Localization's default language, else the tables' first language. A
//   language without a cell falls back to the default language, then the first language, then
//   the key itself, so a missing translation is visible but never empty.
// - get(key, args) replaces `{name}` with args' values (and `{1}`, `{2}`... with positional
//   values from Lua).
// - Thread-safety: main thread.

#include "Export.h"
#include "util/FunctionSubscribe.h"

#include <map>
#include <string>
#include <vector>

namespace jasperengine{

    class JASPERENGINE_API Localization{
    public:
        // loads (merges) a CSV table; false with `error` when the file cannot be read or has no
        // language column
        static bool load(const std::string& path, std::string* error = nullptr);
        // parses CSV text as load() does (tools and tests)
        static bool loadFromString(const std::string& csv, const std::string& source = "", std::string* error = nullptr);
        // forgets every table (the next lookup loads localization.csv again)
        static void clear();

        // the text of `key` in the current language, `key` itself when the tables lack it
        static std::string get(const std::string& key);
        // get(key) with `{name}` replaced by args[name]
        static std::string format(const std::string& key, const std::map<std::string, std::string>& args);
        // replaces `{name}` in `text` (unknown names stay as written)
        static std::string substitute(const std::string& text, const std::map<std::string, std::string>& args);
        static bool has(const std::string& key);
        // the text of `key` in a given language, without fallback ("" when absent)
        static std::string getIn(const std::string& key, const std::string& language);

        // sets the game's language (shared with TitleArt); "" goes back to the device's
        // language; false with `error` for an invalid tag
        static bool setLanguage(const std::string& code, std::string* error = nullptr);
        // the language lookups use now (one of getLanguages(), or "" without tables)
        static std::string getLanguage();
        // the languages the loaded tables have, in column order
        static std::vector<std::string> getLanguages();
        static std::vector<std::string> getKeys();

        // fires with the new code when the language changes
        static FunctionSubscribe<void(const std::string&)>& getOnLanguageChanged();

    private:
        static void ensureLoaded();
    };

}

#endif //LOCALIZATION_H

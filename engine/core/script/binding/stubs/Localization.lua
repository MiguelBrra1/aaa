---@meta _
-- Hand-written shapes of the Localization module (engine/core/script/binding/UtilClassesLua.cpp,
-- engine/core/util/Localization.h). Guide: docs/localization.md.

---Values that replace {name} (string keys) and {1}, {2}... (array part) in a translated text.
---@alias LocalizationArgs table<string|integer, any>

---String tables for a game's text in many languages. The tables are CSV files whose first row
---is `key,en,es,...`; `localization.csv` beside the scripts loads by itself.
---@class Localization
---@field onLanguageChanged FunctionSubscribe_V_S # fires with the new language code
Localization = {}

---The text of `key` in the game's language (the key itself when no table has it).
---`args` replaces `{name}` and `{1}`, `{2}`...
---@param key string
---@param args? LocalizationArgs
---@return string
function Localization.get(key, args) end

---Same as Localization.get.
---@param key string
---@param args? LocalizationArgs
---@return string
function Localization.format(key, args) end

---The text of `key` in one language, without falling back ("" when absent).
---@param key string
---@param language string
---@return string
function Localization.getIn(key, language) end

---@param key string
---@return boolean
function Localization.has(key) end

---Loads (merges) one more CSV table.
---@param path string
---@return boolean ok
---@return string? error
function Localization.load(path) end

---Forgets every table (the next lookup loads localization.csv again).
function Localization.clear() end

---Sets the game's language (shared with TitleArt.setLanguage); "" goes back to the device's
---language. An invalid code raises.
---@param code string
function Localization.setLanguage(code) end

---The language lookups use now.
---@return string
function Localization.getLanguage() end

---The languages of the loaded tables, in column order.
---@return string[]
function Localization.getLanguages() end

---Every key of the loaded tables.
---@return string[]
function Localization.getKeys() end

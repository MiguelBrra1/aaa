// (c) JasperEngine contributors
// SPDX-License-Identifier: MIT

#pragma once

// The editor's interface language (Editor Settings > General > Editor Language).
//
// English is the source: every label in the code stays English. Another language installs an
// ImGui display-text hook (libs/imgui JASPER_PATCHES.md, "Editor language") that shows the
// translation of each text its table has, matched exactly on the visible part of the label.
// IDs never change, so docking layouts, .ini files and shortcuts are the same in every language.
// Tables: editor/lang/<code>.tsv, compiled by editor/lang/gen_lang.py.
// The same hook shows the product name, HoneyEngine, wherever a label says JasperEngine (paths
// excepted), in every language.

#include <string>
#include <vector>

namespace jasperengine::editor {

    class EditorLang {
    public:
        // "en" (no translation) or a code with a table ("es"); an unknown code falls back to "en"
        static void set(const std::string& code);
        static const std::string& get();
        // the codes the editor can show, "en" first
        static std::vector<std::string> available();
        // the language's own name ("English", "Español")
        static const char* displayName(const std::string& code);
        // the translation of an English text in the current language, or the text itself
        static const char* tr(const char* english);
    };

}

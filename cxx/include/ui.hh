// SPDX-FileCopyrightText: (C) 2024 Thiago Kaique
// SPDX-License-Identifier: MIT
//
// ref: https://github.com/QTR-Modding/SKSE-Menu-Framework-3-Example/blob/master/include/UI.h
#pragma once

#include <SKSEMenuFramework.h>

namespace ui {
    void Register();

    namespace Example1 {
        static inline std::atomic<uint32_t> ADD_FORM_ID{ 0x0000f };  // Gold
        inline RE::TESBoundObject*          ADD_BOUND_OBJECT{};

        void           LookupForm();
        void __stdcall Render();
    }  // namespace Example1

    namespace Example2 {
        inline MENU_WINDOW ExampleWindow;

        void __stdcall Render();
        void __stdcall RenderWindow();
    }  // namespace Example2

    namespace Example3 {
        inline ImGuiMCP::ImGuiTextFilter* filter;  // NOTE: We don't use `unique_ptr`, to avoid crash unexpectedly.
        inline const char8_t*             CheckMark = u8"\uf00c";

        inline constexpr ImGuiMCP::ImGuiTableFlags flags =
            ImGuiMCP::ImGuiTableFlags_Resizable |
            ImGuiMCP::ImGuiTableFlags_Reorderable |
            ImGuiMCP::ImGuiTableFlags_Hidable |
            ImGuiMCP::ImGuiTableFlags_BordersOuter |
            ImGuiMCP::ImGuiTableFlags_BordersV |
            ImGuiMCP::ImGuiTableFlags_NoBordersInBody |
            ImGuiMCP::ImGuiTableFlags_ScrollY;

        void __stdcall Render();
    }  // namespace Example3

    namespace Example4 {
        inline const char8_t* TITLE_TEXT{ u8"This is an \uf2b4 Font Awesome usage example" };
        inline const char8_t* BUTTON_TEXT{ u8"\uf0e9 Umbrella" };
        inline const char8_t* BUTTON2_TEXT{ u8"\uf06e Eye" };

        void __stdcall Render();
    }  // namespace Example4

    namespace Example5 {
        void __stdcall     Render();
        void __stdcall     RenderOverlay();
        bool __stdcall     OnInput(RE::InputEvent* event);
        void __stdcall     RenderWindow();
        inline MENU_WINDOW NonPausingWindow;
    }  // namespace Example5

};  // namespace ui

namespace Configuration {
    constexpr const char* MOD_NAME = "MyPlugin(Rust bridge)";

    namespace Example1 {
        inline int Number = 1;
    }
    namespace Example2 {
        inline std::array<char, 256> Buffer{ '\0' };
        inline std::array<float, 4>  Color{};
    }  // namespace Example2
};  // namespace Configuration

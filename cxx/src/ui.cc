// SPDX-FileCopyrightText: (C) 2024 Thiago Kaique
// SPDX-License-Identifier: MIT
//
// ref: https://github.com/QTR-Modding/SKSE-Menu-Framework-3-Example/blob/master/src/UI.cpp
#include <ranges>

#include "SKSE/API.h"
#include "cmd_cheat/cmd_view.hh"
#include "item_cheat/item_view.hh"
#include "ui.hh"

void ui::Register() {
    if (!SKSEMenuFramework::IsInstalled()) {
        spdlog::info("Since 'SKSEMenuFramework' is not installed, the GUI settings cannot be used.");
        return;
    }

    SKSEMenuFramework::SetSection(std::string{ SKSE::GetPluginName() });

    SKSEMenuFramework::AddSectionItem("Item Cheat", item_cheat::view::Render);
    SKSEMenuFramework::AddSectionItem("Auto Command", cmd_cheat::view::Render);

    // SKSEMenuFramework::AddSectionItem("Folder Example/Example 2", Example2::Render);
    // ui::Example2::ExampleWindow = SKSEMenuFramework::AddWindow(Example2::RenderWindow, true);

    // Example3::filter = new ImGuiMCP::ImGuiTextFilter();
    // SKSEMenuFramework::AddSectionItem("Folder Example/Example 3", Example3::Render);

    // SKSEMenuFramework::AddSectionItem("Example 5", Example5::Render);
    // SKSEMenuFramework::AddHudElement(Example5::RenderOverlay);
    // SKSEMenuFramework::AddInputEvent(Example5::OnInput);
    // ui::Example5::NonPausingWindow = SKSEMenuFramework::AddWindow(Example5::RenderWindow, false);
}

void ui::Example1::LookupForm() {
    auto addForm = RE::TESForm::LookupByID(ADD_FORM_ID);
    if (addForm) {
        ADD_BOUND_OBJECT = addForm->As<RE::TESBoundObject>();
    } else {
        ADD_BOUND_OBJECT = nullptr;
    }
}

void __stdcall ui::Example1::Render() {
    ImGuiMCP::InputScalar("form id", ImGuiMCP::ImGuiDataType_U32, &ADD_FORM_ID,
        nullptr, nullptr, "%08X");

    if (ImGuiMCP::Button("Search")) {
        LookupForm();
    }

    if (ADD_BOUND_OBJECT) {
        ImGuiMCP::Text("How much %s would you like to add?",
            ADD_BOUND_OBJECT->GetName());
        ImGuiMCP::SliderInt("number", &Configuration::Example1::Number, 1, 100000);
        if (ImGuiMCP::Button("Add")) {
            auto player =
                RE::PlayerCharacter::GetSingleton()->As<RE::TESObjectREFR>();
            player->AddObjectToContainer(ADD_BOUND_OBJECT, nullptr,
                Configuration::Example1::Number, nullptr);
        }
    } else {
        ImGuiMCP::Text("Form not found");
    }
}

void __stdcall ui::Example2::Render() {
    if (ImGuiMCP::Button("Open Window")) {
        ExampleWindow->IsOpen = true;
    }

    ImGuiMCP::InputText("string", Configuration::Example2::Buffer.data(), Configuration::Example2::Buffer.size());
    ImGuiMCP::ColorEdit4("Color", Configuration::Example2::Color.data());

#pragma warning(push)
#pragma warning(disable: 4244)  // double -> float
#pragma warning(disable: 4267)  // size_t -> int
    std::array<float, 100> samples{};
    for (auto [n, v] : std::views::enumerate(samples)) {
        // NOLINTNEXTLINE(bugprone-narrowing-conversions))
        v = sinf(n * 0.2f + ImGuiMCP::GetTime() * 1.5f);
    }

    ImGuiMCP::PlotLines("Samples", samples.data(), samples.size());
#pragma warning(pop)

    ImGuiMCP::TextColored(ImGuiMCP::ImVec4(1, 1, 0, 1), "Important Stuff");
    ImGuiMCP::BeginChild("Scrolling");
    for (int n : std::views::iota(0, 50)) {
        ImGuiMCP::Text("%04d: Some text", n);
    }
    ImGuiMCP::EndChild();
}

void __stdcall ui::Example2::RenderWindow() {
    auto viewport = ImGuiMCP::GetMainViewport();

    auto center = ImGuiMCP::ImVec2Manager::Create();
    ImGuiMCP::ImGuiViewportManager::GetCenter(center, viewport);
    ImGuiMCP::SetNextWindowPos(*center, ImGuiMCP::ImGuiCond_Appearing,
        ImGuiMCP::ImVec2{ 0.5f, 0.5f });
    ImGuiMCP::ImVec2Manager::Destroy(center);
    ImGuiMCP::SetNextWindowSize(
        ImGuiMCP::ImVec2{ viewport->Size.x * 0.4f, viewport->Size.y * 0.4f },
        ImGuiMCP::ImGuiCond_Appearing);
    ImGuiMCP::Begin("My First Tool##MenuEntryFromMod", nullptr, ImGuiMCP::ImGuiWindowFlags_MenuBar);
    if (ImGuiMCP::BeginMenuBar()) {
        if (ImGuiMCP::BeginMenu("File")) {
            if (ImGuiMCP::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */
            }
            if (ImGuiMCP::MenuItem("Save", "Ctrl+S")) { /* Do stuff */
            }
            if (ImGuiMCP::MenuItem("Close", "Ctrl+W")) {
                ExampleWindow->IsOpen = false;
            }
            ImGuiMCP::EndMenu();
        }
        ImGuiMCP::EndMenuBar();
    }
    if (ImGuiMCP::Button("Close Window")) {
        ExampleWindow->IsOpen = false;
    }
    ImGuiMCP::End();
}

void __stdcall ui::Example3::Render() {
    if (Example3::filter) {
        Example3::filter->Draw();
    }

    if (ImGuiMCP::BeginTable("table1", 3, flags)) {
        ImGuiMCP::TableSetupColumn("One");
        ImGuiMCP::TableSetupColumn("Two");
        ImGuiMCP::TableSetupColumn("Three");
        ImGuiMCP::TableHeadersRow();

        for (int row = 0; row < 5; row++) {
            ImGuiMCP::TableNextRow();
            for (int column = 0; column < 3; column++) {
                ImGuiMCP::TableSetColumnIndex(column);
                std::array<char, 32> buf{};

                int fail_w = sprintf_s(buf.data(), buf.size(), "Hello %d,%d", column, row);
                if (fail_w && Example3::filter && Example3::filter->PassFilter(buf.data())) {
                    FontAwesome::PushSolid();
                    ImGuiMCP::Text(reinterpret_cast<const char*>(CheckMark));
                    FontAwesome::Pop();
                    ImGuiMCP::SameLine();
                }

                if (column == 2) {
                    ImGuiMCP::Button(buf.data(), ImGuiMCP::ImVec2(-FLT_MIN, 0.0f));
                } else {
                    ImGuiMCP::TextUnformatted(buf.data());
                }
            }
        }
        ImGuiMCP::EndTable();
    }
}

void __stdcall ui::Example4::Render() {
    FontAwesome::PushBrands();
    ImGuiMCP::Text(reinterpret_cast<const char*>(TITLE_TEXT));
    FontAwesome::Pop();

    FontAwesome::PushSolid();
    ImGuiMCP::Button(reinterpret_cast<const char*>(BUTTON_TEXT));
    FontAwesome::Pop();

    ImGuiMCP::SameLine();

    FontAwesome::PushRegular();
    ImGuiMCP::Button(reinterpret_cast<const char*>(BUTTON2_TEXT));
    FontAwesome::Pop();
}

void __stdcall ui::Example5::Render() {
    auto texture = SKSEMenuFramework::LoadTexture(
        "Data\\interface\\unlit-bomb.svg", { 100, 100 });
    ImGuiMCP::Text("Open non pausing menu: ");
    ImGuiMCP::SameLine();
    if (ImGuiMCP::ImageButton("Open non pausing menu##button", texture,
            { 100, 100 })) {
        NonPausingWindow->IsOpen = !NonPausingWindow->IsOpen;
    }
}

void __stdcall ui::Example5::RenderOverlay() {
    if (SKSEMenuFramework::IsAnyBlockingWindowOpened()) {
        return;
    }
    auto drawList = ImGuiMCP::GetForegroundDrawList();
    // ImVec2 center = ImGui::GetIO()->DisplaySize;
    // center.x *= 0.5;
    // center.y *= 0.5;
    // ImGui::ImDrawListManager::AddCircle(drawList, center, 100, IM_COL32(255, 0,
    // 0, 255), 100, 10);

    const char*      text = "Press B to toggle the info window";
    ImGuiMCP::ImVec2 textSize;
    ImGuiMCP::CalcTextSize(&textSize, text, 0, false, 0);
    auto textPos =
        ImGuiMCP::ImVec2(ImGuiMCP::GetIO()->DisplaySize.x - textSize.x - 20,
            20);  // 10px padding from edges
    ImGuiMCP::ImDrawListManager::AddText(drawList, textPos,
        IM_COL32(255, 255, 255, 255), text);
}

bool __stdcall ui::Example5::OnInput(RE::InputEvent* event) {
    bool blockThisUserInput = false;

    if (event->device == RE::INPUT_DEVICE::kKeyboard) {
        if (auto button = event->AsButtonEvent()) {
            if (button->GetIDCode() == RE::BSWin32KeyboardDevice::Key::kB &&
                button->IsDown()) {
                NonPausingWindow->IsOpen = !NonPausingWindow->IsOpen;
                blockThisUserInput = true;
            }
        }
    }

    return blockThisUserInput;
}

void __stdcall ui::Example5::RenderWindow() {
    auto viewport = ImGuiMCP::GetMainViewport();

    // Position window at top right corner
    auto windowSize =
        ImGuiMCP::ImVec2{ viewport->Size.x * 0.4f, viewport->Size.y * 0.4f };
    auto windowPos =
        ImGuiMCP::ImVec2{ viewport->Pos.x + viewport->Size.x - windowSize.x - 20,
            viewport->Pos.y + 20 };

    ImGuiMCP::SetNextWindowPos(windowPos, ImGuiMCP::ImGuiCond_Appearing, { 0, 0 });
    ImGuiMCP::SetNextWindowSize(windowSize, ImGuiMCP::ImGuiCond_Appearing);

    ImGuiMCP::Begin("My First Overlay Window##MenuEntityFromMod", nullptr,
        ImGuiMCP::ImGuiWindowFlags_NoCollapse);
    ImGuiMCP::Text("Chinese characters (requires chinese font): %s", u8"测试");

    // You can load only once if you want, but the texture loader creates a cache of your texture;
    auto texture = SKSEMenuFramework::LoadTexture(
        "Data\\interface\\unlit-bomb.svg", { 100, 100 });
    auto texture2 =
        SKSEMenuFramework::LoadTexture("Data\\interface\\screenshot.png");
    ImGuiMCP::Text("Image Display: ");
    ImGuiMCP::SameLine();
    ImGuiMCP::Image(texture, ImGuiMCP::ImVec2(100, 100));
    ImGuiMCP::Image(texture2, ImGuiMCP::ImVec2(640, 360));
    ImGuiMCP::End();
}

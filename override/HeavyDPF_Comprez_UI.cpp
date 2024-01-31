/** Copyright (c) AnClark 2024 - GPL-3.0-or-later */

#include "DistrhoUI.hpp"
#include "ResizeHandle.hpp"
#include "veramobd.hpp"
#include "wstdcolors.hpp"
#include "comprez_colors.hpp"

// Generated by tools/get_plugin_version.py
#include "version.hpp"

// For ImGuiPluginUI::OsOpenInShell()
#ifdef __WIN32
#include "windows.h"
#endif

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------------------------------------------------------
enum HeavyParams {ATTACK_RELEASE,RATIO,THRESHOLD,
};

class ImGuiPluginUI : public UI
{
    
    float fattack_release = 40.0f;
    float fratio = 1.0f;
    float fthreshold = 70.0f;

    // true:  wider ratio range [0, inf]
    // false: narrower ratio range [0, 10]
    bool fratio_range = false;

    bool fshow_about_window = false;

    ResizeHandle fResizeHandle;

    // ----------------------------------------------------------------------------------------------------------------

public:
   /**
      UI class constructor.
      The UI should be initialized to a default state that matches the plugin side.
    */
    ImGuiPluginUI()
        : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT),
          fResizeHandle(this)
    {
        setGeometryConstraints(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT, true);

        ImGuiIO& io(ImGui::GetIO());

        ImFontConfig fc;
        fc.FontDataOwnedByAtlas = true;
        fc.OversampleH = 1;
        fc.OversampleV = 1;
        fc.PixelSnapH = true;

        io.Fonts->AddFontFromMemoryCompressedTTF((void*)veramobd_compressed_data, veramobd_compressed_size, 16.0f * getScaleFactor(), &fc);
        io.Fonts->AddFontFromMemoryCompressedTTF((void*)veramobd_compressed_data, veramobd_compressed_size, 21.0f * getScaleFactor(), &fc);
        io.Fonts->AddFontFromMemoryCompressedTTF((void*)veramobd_compressed_data, veramobd_compressed_size, 11.0f * getScaleFactor(), &fc);
        io.Fonts->Build();
        io.FontDefault = io.Fonts->Fonts[1];

        fResizeHandle.hide();
    }

protected:
    // ----------------------------------------------------------------------------------------------------------------
    // DSP/Plugin Callbacks

   /**
      A parameter has changed on the plugin side.@n
      This is called by the host to inform the UI about parameter changes.
    */
    void parameterChanged(uint32_t index, float value) override
    {
        switch (index) {
            case ATTACK_RELEASE:
                fattack_release = value;
                break;
            case RATIO:
                fratio = value;
                break;
            case THRESHOLD:
                fthreshold = value;
                break;
            
            default: return;
        }
    
        repaint();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Widget Callbacks

   /**
      ImGui specific onDisplay function.
    */
    void onImGuiDisplay() override
    {
        const float width = getWidth();
        const float height = getHeight();
        const float margin = 0.0f;

        ImGui::SetNextWindowPos(ImVec2(margin, margin));
        ImGui::SetNextWindowSize(ImVec2(width - 2 * margin, height - 2 * margin));

        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

        style.Colors[ImGuiCol_MenuBarBg] = (ImVec4)ComprezTitleBg;        
        style.Colors[ImGuiCol_WindowBg] = (ImVec4)ComprezWindowBg;

        ImGuiIO& io(ImGui::GetIO());
        ImFont* defaultFont = ImGui::GetFont();
        ImFont* titleBarFont = io.Fonts->Fonts[2];
        ImFont* smallFont = io.Fonts->Fonts[3];

        // TODO: Set color intense when tuning Threshold. Less means brighter.
        auto intense = 0.0f;

        auto ThresholdActive  = ColorBright(Amber, intense);
        auto ThresholdHovered = ColorBright(AmberBr, intense);
        auto RatioActive      = ColorBright(Teal, intense);
        auto RatioHovered     = ColorBright(TealBr, intense);
        auto RangeSw          = ColorBright(WhiteDr, intense);
        auto RangeAct         = ColorBright(GreenDr, intense);
        auto RangeActHovered  = ColorBright(Green, intense);
        auto AttackReleaseActive   = ColorBright(Indigo, intense);
        auto AttackReleaseHovered  = ColorBright(IndigoBr, intense);

        auto scaleFactor = getScaleFactor();
        const float hundred = 100 * scaleFactor;
        const float toggleWidth  = 20 * scaleFactor;

        constexpr auto RATIO_MAX_VALUE_WIDE = 100.0f;
        constexpr auto RATIO_MAX_VALUE_NARROW = 10.0f;

        auto ratiostep = (fratio_range) ? 1.0f : 0.1f;
        auto thresholdstep = 1.0f;
        auto msstep = 2.0f;

        if (io.KeyShift)
        {
            ratiostep = (fratio_range) ? 0.1f : 0.01f;
            thresholdstep = 0.1f;
            msstep = 1.0f;
        }

        if (fratio > RATIO_MAX_VALUE_NARROW)
            fratio_range = true;

        ImGui::PushFont(titleBarFont);
        if (!fshow_about_window && ImGui::Begin("Comprez", nullptr, ImGuiWindowFlags_MenuBar + ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoCollapse + ImGuiWindowFlags_NoTitleBar))
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::PushStyleColor(ImGuiCol_Button,    style.Colors[ImGuiCol_MenuBarBg]);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  ColorBright(Amber, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ColorBright(Amber, -20.0f));

                if (ImGui::Button("Comprez"))
                    fshow_about_window = !fshow_about_window;

                ImGui::PopStyleColor(3);
                ImGui::EndMenuBar();
            }

            ImGui::Dummy(ImVec2(0.0f, 8.0f * scaleFactor));
            ImGui::PushFont(defaultFont);
            auto ImGuiKnob_Flags = ImGuiKnobFlags_DoubleClickReset + ImGuiKnobFlags_ValueTooltip + ImGuiKnobFlags_NoInput + ImGuiKnobFlags_ValueTooltipHideOnClick;
            //auto ImGuiKnob_FlagsDB = ImGuiKnob_Flags + ImGuiKnobFlags_dB;

            ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)ThresholdActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)ThresholdHovered);
            if (ImGuiKnobs::Knob("Threshold", &fthreshold, 0.0f, 200.0f, thresholdstep, "%.1f", ImGuiKnobVariant_SteppedTick, hundred, ImGuiKnob_Flags, 11))
            {
                if (ImGui::IsItemActivated())
                {
                    editParameter(THRESHOLD, true);
                    if (ImGui::IsMouseDoubleClicked(0))
                        fthreshold = 70.0f;
                }
                setParameterValue(THRESHOLD, fthreshold);
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 12);

            ImGui::BeginGroup();
            {
                ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)RatioActive);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)RatioHovered);
                auto flow_max = RATIO_MAX_VALUE_WIDE;
                auto flow_steps = 21;
                if (not fratio_range)
                {
                    flow_max = RATIO_MAX_VALUE_NARROW;
                    flow_steps = 11;
                }

                if (ImGuiKnobs::Knob("Ratio", &fratio, 0.0f, flow_max, ratiostep, "%.2f : 1", ImGuiKnobVariant_SteppedTick, hundred, ImGuiKnob_Flags, flow_steps))
                {
                    if (ImGui::IsItemActivated())
                    {
                        editParameter(RATIO, true);
                        if (ImGui::IsMouseDoubleClicked(0))
                            fratio = 1.0f;
                    }
                    setParameterValue(RATIO, fratio);
                }
                ImGui::PopStyleColor(2);
                ImGui::SameLine(0, 10);

                ImGui::BeginGroup();
                {
                    // Title text
                    ImGui::PushStyleColor(ImGuiCol_Text, TextClr);
                    CenterTextX("Range", toggleWidth);
                    ImGui::Dummy(ImVec2(0.0f, 20.0f) * scaleFactor);

                    // Range text
                    ImGui::PushFont(smallFont);
                    auto rangedef = (fratio_range) ? "100:1": "10:1";
                    CenterTextX(rangedef, toggleWidth);
                    ImGui::PopFont();

                    ImGui::PushFont(defaultFont);
                    ImGui::PopStyleColor();

                    // knob
                    ImGui::PushStyleColor(ImGuiCol_Text,            (ImVec4)RangeSw);

                    // inactive colors
                    ImGui::PushStyleColor(ImGuiCol_FrameBg,         (ImVec4)RangeAct);
                    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,  (ImVec4)RangeActHovered);

                    // active colors
                    ImGui::PushStyleColor(ImGuiCol_Button,          (ImVec4)RangeAct);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)RangeActHovered);

                    if (ImGui::Toggle("##Range", &fratio_range, ImGuiToggleFlags_Animated))
                    {
                        if (ImGui::IsItemActivated() && !fratio_range)
                        {
                            editParameter(RATIO, true);
                            fratio = std::min(fratio, RATIO_MAX_VALUE_NARROW);
                            setParameterValue(RATIO, fratio);
                        }
                    }
                    ImGui::PopStyleColor(5);

                    ImGui::PopFont();
                }
                ImGui::EndGroup();
            }
            ImGui::EndGroup();
            ImGui::SameLine(0, 20);

            ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)AttackReleaseActive);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)AttackReleaseHovered);
            constexpr auto flow_steps = 21;

            if (ImGuiKnobs::Knob("Atk/Rel", &fattack_release, -1000.0f, 1000.0f, msstep, "Attack/Release:\n%.0f ms", ImGuiKnobVariant_SteppedTick, hundred, ImGuiKnob_Flags, flow_steps))
            {
                if (ImGui::IsItemActivated())
                {
                    editParameter(ATTACK_RELEASE, true);
                    if (ImGui::IsMouseDoubleClicked(0))
                        fattack_release = 40.0f;
                }
                setParameterValue(ATTACK_RELEASE, fattack_release);
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine(0, 10);

            if (ImGui::IsItemDeactivated())
            {
                editParameter(ATTACK_RELEASE, false);
                editParameter(RATIO, false);
                editParameter(THRESHOLD, false);
            }

            ImGui::PopFont();
        }
        ImGui::PopFont();
        ImGui::End();

        if (fshow_about_window && ImGui::Begin("About Comprez", &fshow_about_window, ImGuiWindowFlags_NoResize + ImGuiWindowFlags_NoCollapse))
        {
            // TODO: Parse and show version and commit by Python script
            ImGui::Text("Comprez - v%d.%d.%d (commit %s)", VERSION_MAJOR, VERSION_MINOR, VERSION_BUGFIX, GIT_COMMIT_ID);
            ImGui::Text("Simple & High Performance Compressor Plugin");
            ImGui::Text("");
            ImGui::Text("Author:\n    AnClark Liu <clarklaw4701@qq.com>");
            ImGui::Text("DSP Engine:\n    Heavy Stereo Compressor by Joe White");
            ImGui::Text("Credits to:\n    Wasted Audio Team");
            ImGui::Text("");
            ImGui::Text("License:\n    GNU GPLv3");
            ImGui::Text("VST Plugin Interface Technology by Steinberg");
            ImGui::Text("");

            if (ImGui::Button("    OK    "))
                fshow_about_window = false;

            ImGui::SameLine(0, 20);
            if (ImGui::Button("  Official Website  "))
                OsOpenInShell("https://github.com/AnClark/Comprez");
        }
        ImGui::End();
    }

    // ----------------------------------------------------------------------------------------------------------------
    // Utilities

   /**
      Open a program / URL in current OS.
      Used to open plugin official website in "About" window.

      Reference: https://github.com/ocornut/imgui/issues/5510
    */
    void OsOpenInShell(const char* path)
    {
#ifdef _WIN32
        // Note: executable path must use backslashes!
        ::ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWDEFAULT);
#else
#if __APPLE__
        const char* open_executable = "open";
#else
        const char* open_executable = "xdg-open";
#endif
        char command[256];
        snprintf(command, 256, "%s \"%s\"", open_executable, path);
        system(command);
#endif
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImGuiPluginUI)
};

// --------------------------------------------------------------------------------------------------------------------

UI* createUI()
{
    return new ImGuiPluginUI();
}

// --------------------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#pragma once
#include <imgui.h>

namespace Theme
{
    // Colours used across all states
    inline ImVec4 Gold()       { return {1.00f, 0.84f, 0.00f, 1.00f}; }
    inline ImVec4 DarkGold()   { return {0.75f, 0.60f, 0.00f, 1.00f}; }
    inline ImVec4 Green()      { return {0.00f, 0.60f, 0.15f, 1.00f}; }
    inline ImVec4 DarkGreen()  { return {0.05f, 0.30f, 0.05f, 1.00f}; }
    inline ImVec4 Red()        { return {0.80f, 0.10f, 0.10f, 1.00f}; }
    inline ImVec4 LightText()  { return {0.95f, 0.90f, 0.75f, 1.00f}; }
    inline ImVec4 Felt()       { return {0.08f, 0.28f, 0.08f, 1.00f}; }
    inline ImVec4 Dark()       { return {0.06f, 0.06f, 0.06f, 1.00f}; }

    void apply();

    // Helpers
    void PushTitleStyle();
    void PopTitleStyle();
    void PushButtonGold();
    void PopButtonGold();
    void PushButtonRed();
    void PopButtonRed();
}

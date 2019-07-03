/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2019  natinusala

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <CrashFrame.hpp>
#include <Animations.hpp>
#include <Application.hpp>

#include <math.h>

CrashFrame::CrashFrame(string text)
{
    // Label
    this->label = new Label(LabelStyle::CRASH, text, true);
    this->label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->label->setParent(this);

    // Button
    this->button = new Button(ButtonStyle::CRASH, "OK");
    this->button->setParent(this);
    this->button->alpha = 0.0f;
    this->button->setClickListener([](View *view) { Application::quit(); });
    this->button->overrideTheme(&themeDark);
}

void CrashFrame::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx)
{
    // Background
    nvgFillColor(vg, RGB(0, 0, 0));
    nvgBeginPath(vg);
    nvgRect(vg, x, y, width, height);
    nvgFill(vg);

    // Scale
    float scale = (this->alpha + 2.0f) / 3.0f;
    nvgTranslate(vg, (1.0f-scale) * width * 0.5f, (1.0f-scale) * height * 0.5f);
    nvgScale(vg, scale, scale);

    // Label
    this->label->frame(ctx);

    // [!] box
    unsigned boxSize = style->CrashFrame.boxSize;
    nvgStrokeColor(vg, RGB(255, 255, 255));
    nvgStrokeWidth(vg, style->CrashFrame.boxStrokeWidth);
    nvgBeginPath(vg);
    nvgRect(vg, x + width / 2 - boxSize / 2, y + style->CrashFrame.boxSpacing, boxSize, boxSize);
    nvgStroke(vg);

    nvgFillColor(vg, RGB(255, 255, 255));

    nvgFontSize(vg, (float)style->CrashFrame.boxSize / 1.25f);
    nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgBeginPath(vg);
    nvgText(vg, x + width / 2, y + style->CrashFrame.boxSpacing + boxSize/2, "!", nullptr);
    nvgFill(vg);

    // End scale
    nvgResetTransform(vg);

    // Footer
    nvgBeginPath(vg);
    nvgRect(vg, x + style->SettingsFrame.separatorSpacing, y + height - style->SettingsFrame.footerHeight, width - style->SettingsFrame.separatorSpacing * 2, 1);
    nvgFill(vg);

    nvgFontSize(vg, style->SettingsFrame.footerTextSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgBeginPath(vg);
    nvgText(vg, x + style->SettingsFrame.separatorSpacing + style->SettingsFrame.footerTextSpacing, y + height - style->SettingsFrame.footerHeight/2, WINDOW_NAME, nullptr);

    // Button
    this->button->frame(ctx);

    //TODO: Hint
}

void CrashFrame::onShowAnimationEnd()
{
    this->button->show(nullptr);
}

View* CrashFrame::requestFocus(FocusDirection direction, View *oldFocus, bool fromUp)
{
    return this->button;
}

void CrashFrame::layout(NVGcontext* vg, Style *style, FontStash *stash)
{
    // Label
    this->label->setWidth(roundf((float)this->width * style->CrashFrame.labelWidth));
    this->label->layout(vg, style, stash);

    this->label->setBoundaries(
        this->x + this->width / 2 - this->label->getWidth() / 2,
        this->y + (this->height - style->SettingsFrame.footerHeight) / 2,
        this->label->getWidth(),
        this->label->getHeight()
    );

    // Button
    this->button->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth / 2,
        this->y + this->height - style->SettingsFrame.footerHeight - style->CrashFrame.boxSpacing - style->CrashFrame.buttonHeight,
        style->CrashFrame.buttonWidth,
        style->CrashFrame.buttonHeight
    );
    this->button->invalidate();
}

CrashFrame::~CrashFrame()
{
    delete this->label;
}
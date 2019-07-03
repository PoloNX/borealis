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

#include <List.hpp>
#include <Application.hpp>
#include <TableView.hpp>
#include <SelectView.hpp>

#include <Animations.hpp>

// TODO: Scrollbar

List::List() : BoxLayout(BoxLayoutOrientation::VERTICAL)
{
    Style *style = Application::getStyle();
    this->setMargins(style->List.marginTopBottom, style->List.marginLeftRight, style->List.marginTopBottom, style->List.marginLeftRight);
    this->setSpacing(style->List.spacing);
}

void List::customSpacing(View *current, View *next, int *spacing)
{
    // Don't add spacing to the first list item
    // if it doesn't have a sublabel and the second one is a
    // list item too
    // Or if the next item is a ListItemGroupSpacing
    if (ListItem *currentItem = dynamic_cast<ListItem*>(current))
    {
        if (ListItem *nextItem = dynamic_cast<ListItem*>(next))
        {
            if (!currentItem->hasSubLabel())
            {
                *spacing = 2;
                nextItem->setDrawTopSeparator(false);
            }
        }
        else if (dynamic_cast<ListItemGroupSpacing*>(next))
        {
            *spacing = 0;
        }
        else if (dynamic_cast<TableView*>(next))
        {
            *spacing /= 2;
        }
    }
    // TableView custom spacing
    else if (dynamic_cast<TableView*>(current))
    {
        *spacing /= 2;
    }
    // ListItemGroupSpacing custom spacing
    else if (dynamic_cast<ListItemGroupSpacing*>(current))
    {
        *spacing /= 2;
    }
}

View* List::defaultFocus(View *oldFocus)
{
    if (this->focusedIndex >= 0 && this->focusedIndex < this->children.size())
    {
        View *newFocus = this->children[this->focusedIndex]->view->requestFocus(FocusDirection::NONE, oldFocus);
        if (newFocus)
            return newFocus;
    }

    return BoxLayout::defaultFocus(oldFocus);
}

ListItem::ListItem(string label, string sublabel) : label(label)
{
    Style *style = Application::getStyle();

    this->setHeight(style->List.Item.height);
    this->setTextSize(style->List.Item.textSize);

    if (sublabel != "")
        this->sublabelView = new Label(LabelStyle::SUBLABEL, sublabel, true);
}

void ListItem::setIndented(bool indented)
{
    this->indented = indented;
}

void ListItem::setTextSize(unsigned textSize)
{
    this->textSize = textSize;
}

void ListItem::setParent(View *parent)
{
    View::setParent(parent);
    if (this->sublabelView)
        this->sublabelView->setParent(parent);
}

bool ListItem::onClick()
{
    if (this->clickListener)
        this->clickListener(this);

    return this->clickListener != nullptr;
}

void ListItem::setClickListener(EventListener listener)
{
    this->clickListener = listener;
}

void ListItem::layout(NVGcontext *vg, Style *style, FontStash *stash)
{
    if (this->sublabelView)
    {
        unsigned indent = style->List.Item.sublabelIndent;

        if (this->indented)
            indent += style->List.Item.indent;

        this->height = style->List.Item.height;
        this->sublabelView->setBoundaries(this->x + indent, this->y + this->height + style->List.Item.sublabelSpacing, this->width - indent * 2, 0);
        this->sublabelView->layout(vg, style, stash); // we must call layout directly
        this->height += this->sublabelView->getHeight() + style->List.Item.sublabelSpacing;
    }
}

void ListItem::getHighlightInsets(unsigned *top, unsigned *right, unsigned *bottom, unsigned *left)
{
    Style *style = Application::getStyle();
    View::getHighlightInsets(top, right, bottom, left);

    if (sublabelView)
        *bottom = -(sublabelView->getHeight() + style->List.Item.sublabelSpacing);

    if (indented)
        *left = -style->List.Item.indent;
}

void ListItem::resetValueAnimation()
{
    this->valueAnimation = 0.0f;

    menu_animation_ctx_tag tag = (uintptr_t) &this->valueAnimation;
    menu_animation_kill_by_tag(&tag);
}

void ListItem::setValue(string value, bool faint, bool animate)
{
    this->oldValue      = this->value;
    this->oldValueFaint = this->valueFaint;

    this->value         = value;
    this->valueFaint    = faint;

    this->resetValueAnimation();

    if (animate)
    {
        menu_animation_ctx_tag tag = (uintptr_t) &this->valueAnimation;
        menu_animation_ctx_entry_t entry;

        entry.cb            = [this](void *userdata) { this->resetValueAnimation(); };
        entry.duration      = LIST_ITEM_VALUE_ANIMATION_DURATION;
        entry.easing_enum   = EASING_IN_OUT_QUAD;
        entry.subject       = &this->valueAnimation;
        entry.tag           = tag;
        entry.target_value  = 1.0f;
        entry.tick          = nullptr;
        entry.userdata      = nullptr;

        menu_animation_push(&entry);
    }
}

void ListItem::setDrawTopSeparator(bool draw)
{
    this->drawTopSeparator = draw;
}

View* ListItem::requestFocus(FocusDirection direction, View *oldFocus, bool fromUp)
{
    if (this->collapseState != 1.0f)
        return nullptr;

    return this;
}

void ListItem::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, Style *style, FrameContext *ctx)
{
    unsigned baseHeight = this->height;

    if (this->indented)
    {
        x       += style->List.Item.indent;
        width   -= style->List.Item.indent;
    }

    // Sublabel
    if (this->sublabelView)
        this->sublabelView->frame(ctx);

    // Value
    nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
    nvgFontFaceId(vg, ctx->fontStash->regular);
    if (this->valueAnimation != 0.0f)
    {
        //Old value
        NVGcolor valueColor = a(this->oldValueFaint ? ctx->theme->listItemFaintValueColor : ctx->theme->listItemValueColor);
        valueColor.a *= (1.0f - this->valueAnimation);
        nvgFillColor(vg, valueColor);
        nvgFontSize(vg, style->List.Item.valueSize * (1.0f - this->valueAnimation));
        nvgBeginPath(vg);
        nvgText(vg, x + width - style->List.Item.padding, y + baseHeight/2, this->oldValue.c_str(), nullptr);

        //New value
        valueColor = a(this->valueFaint ? ctx->theme->listItemFaintValueColor : ctx->theme->listItemValueColor);
        valueColor.a *= this->valueAnimation;
        nvgFillColor(vg, valueColor);
        nvgFontSize(vg, style->List.Item.valueSize * this->valueAnimation);
        nvgBeginPath(vg);
        nvgText(vg, x + width - style->List.Item.padding, y + baseHeight/2, this->value.c_str(), nullptr);
    }
    else
    {
        nvgFillColor(vg, a(this->valueFaint ? ctx->theme->listItemFaintValueColor : ctx->theme->listItemValueColor));
        nvgFontSize(vg, style->List.Item.valueSize);
        nvgTextAlign(vg, NVG_ALIGN_RIGHT | NVG_ALIGN_MIDDLE);
        nvgFontFaceId(vg, ctx->fontStash->regular);
        nvgBeginPath(vg);
        nvgText(vg, x + width - style->List.Item.padding, y + baseHeight/2, this->value.c_str(), nullptr);
    }

    // Label
    nvgFillColor(vg, a(ctx->theme->textColor));
    nvgFontSize(vg, this->textSize);
    nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
    nvgFontFaceId(vg, ctx->fontStash->regular);
    nvgBeginPath(vg);
    nvgText(vg, x + style->List.Item.padding, y + baseHeight/2, this->label.c_str(), nullptr);

    // Separators
    // Offset by one to be hidden by highlight
    nvgFillColor(vg, a(ctx->theme->listItemSeparatorColor));

    // Top
    if (this->drawTopSeparator)
    {
        nvgBeginPath(vg);
        nvgRect(vg, x, y - 1, width, 1);
        nvgFill(vg);
    }

    // Bottom
    nvgBeginPath(vg);
    nvgRect(vg, x, y + 1 + baseHeight, width, 1);
    nvgFill(vg);
}

bool ListItem::hasSubLabel()
{
    return this->sublabelView;
}

string ListItem::getLabel()
{
    return this->label;
}

ListItem::~ListItem()
{
    if (this->sublabelView)
        delete this->sublabelView;

    this->resetValueAnimation();
}

ToggleListItem::ToggleListItem(string label, bool initialValue, string sublabel, ToggleListItemType type) : 
    ListItem(label, sublabel),
    toggleState(initialValue),
    type(type)
{
    this->updateValue();
}

string ToggleListItem::getOnValue()
{
    switch (this->type)
    {
        default:
        case ToggleListItemType::ON_OFF:
            return "ON";
        case ToggleListItemType::YES_NO:
            return "Yes";
    }
}

string ToggleListItem::getOffValue()
{
    switch (this->type)
    {
        default:
        case ToggleListItemType::ON_OFF:
            return "OFF";
        case ToggleListItemType::YES_NO:
            return "No";
    }
}

void ToggleListItem::updateValue()
{
    if (this->toggleState)
        this->setValue(this->getOnValue(), false);
    else
        this->setValue(getOffValue(), true);
}

bool ToggleListItem::onClick()
{

    this->toggleState = !this->toggleState;
    this->updateValue();

    ListItem::onClick();
    return true;
}

bool ToggleListItem::getToggleState()
{
    return this->toggleState;
}

ListItemGroupSpacing::ListItemGroupSpacing(bool separator) : Rectangle(nvgRGBA(0, 0, 0, 0))
{
    Theme *theme = Application::getTheme();

    if (separator)
        this->setColor(theme->listItemSeparatorColor);
}

SelectListItem::SelectListItem(string label, vector<string> values, unsigned selectedValue) :
    ListItem(label, ""),
    values(values),
    selectedValue(selectedValue)
{
    this->setValue(values[selectedValue], false, false);

    this->setClickListener([this](View *view){
        SelectListener selectListener = [this](int result){
            if (result == -1)
                return;

            this->setValue(this->values[result], false, false);
            this->selectedValue = result;
        };
        SelectView::open(this->getLabel(), this->values, selectListener, this->selectedValue);
    });
}
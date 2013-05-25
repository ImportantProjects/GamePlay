#include "TextBox.h"
#include "Game.h"

namespace gameplay
{

static bool space(char c) {
    return isspace(c);
}

TextBox::TextBox() : _lastKeypress(0), _fontSize(0), _caretImage(NULL), _passwordChar('*'), _inputMode(TEXT), _ctrlPressed(false)
{
}

TextBox::~TextBox()
{
}

TextBox* TextBox::create(const char* id, Theme::Style* style)
{
    GP_ASSERT(style);

    TextBox* textBox = new TextBox();
    if (id)
        textBox->_id = id;
    textBox->setStyle(style);

    return textBox;
}

TextBox* TextBox::create(Theme::Style* style, Properties* properties)
{
    TextBox* textBox = new TextBox();
    textBox->initialize(style, properties);

    return textBox;
}

void TextBox::initialize(Theme::Style* style, Properties* properties)
{
    GP_ASSERT(properties);

    Label::initialize(style, properties);
    _inputMode = getInputMode(properties->getString("inputMode"));
}

int TextBox::getLastKeypress()
{
    return _lastKeypress;
}

void TextBox::addListener(Control::Listener* listener, int eventFlags)
{
    if ((eventFlags & Control::Listener::VALUE_CHANGED) == Control::Listener::VALUE_CHANGED)
    {
        GP_ERROR("VALUE_CHANGED event is not applicable to TextBox.");
    }

    Control::addListener(listener, eventFlags);
}

bool TextBox::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{   
    switch (evt)
    {
    case Touch::TOUCH_PRESS: 
        if (x > _clipBounds.x && x <= _clipBounds.x + _clipBounds.width &&
                 y > _clipBounds.y && y <= _clipBounds.y + _clipBounds.height)
        {
            _contactIndex = (int) contactIndex;

            if (_state == NORMAL)
                Game::getInstance()->displayKeyboard(true);

            setCaretLocation(x, y);

            _state = ACTIVE;
            _dirty = true;
        }
        else
        {
            _contactIndex = INVALID_CONTACT_INDEX;
            _state = NORMAL;
            Game::getInstance()->displayKeyboard(false);
            _dirty = true;
            return false;
        }
        break;
    case Touch::TOUCH_MOVE:
        if (_state == ACTIVE &&
            x > _clipBounds.x && x <= _clipBounds.x + _clipBounds.width &&
            y > _clipBounds.y && y <= _clipBounds.y + _clipBounds.height)
        {
            setCaretLocation(x, y);
            _dirty = true;
        }
        break;
    case Touch::TOUCH_RELEASE:
        if (x > _clipBounds.x && x <= _clipBounds.x + _clipBounds.width &&
            y > _clipBounds.y && y <= _clipBounds.y + _clipBounds.height)
        {
            setCaretLocation(x, y);
            _state = FOCUS;
        }
        else
        {
            _state = NORMAL;
            Game::getInstance()->displayKeyboard(false);
        }
        _contactIndex = INVALID_CONTACT_INDEX;
        _dirty = true;
        break;
    }

    return _consumeInputEvents;
}

bool TextBox::keyEvent(Keyboard::KeyEvent evt, int key)
{
    switch (evt)
    {
        case Keyboard::KEY_PRESS:
        {
            switch (key)
            {
                case Keyboard::KEY_CTRL:
                {
                    _ctrlPressed = true;
                    break;
                }
                case Keyboard::KEY_HOME:
                {
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);
                    font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, 0,
                        textAlignment, true, rightToLeft);
                    _dirty = true;
                    break;
                }
                case Keyboard::KEY_END:
                {
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);
                    const std::string displayedText = getDisplayedText();
                    font->getLocationAtIndex(displayedText.c_str(), _textBounds, fontSize, &_caretLocation, displayedText.size(),
                        textAlignment, true, rightToLeft);
                    _dirty = true;
                    break;
                }
                case Keyboard::KEY_DELETE:
                {
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);

                    int textIndex = font->getIndexAtLocation(getDisplayedText().c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
                        textAlignment, true, rightToLeft);
                        
                    _text.erase(textIndex, 1);
                    font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, textIndex,
                        textAlignment, true, rightToLeft);
                    _dirty = true;
                    notifyListeners(Control::Listener::TEXT_CHANGED);
                    break;
                }
                case Keyboard::KEY_TAB:
                {
                    // Allow tab to move the focus forward.
                    return false;
                    break;
                }
                case Keyboard::KEY_LEFT_ARROW:
                {
                    const std::string displayedText = getDisplayedText();
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);

                    int textIndex = font->getIndexAtLocation(displayedText.c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
                        textAlignment, true, rightToLeft);
                    if (_ctrlPressed)
                    {
                        std::string::const_reverse_iterator it = std::find_if(displayedText.rend() - (textIndex - 1), displayedText.rend(), space);
                        textIndex = std::distance(displayedText.begin(), it.base());
                    }
                    else
                    {
                        --textIndex;
                    }
                    font->getLocationAtIndex(displayedText.c_str(), _textBounds, fontSize, &_caretLocation, textIndex,
                        textAlignment, true, rightToLeft);
                    _dirty = true;
                    break;
                }
                case Keyboard::KEY_RIGHT_ARROW:
                {
                    const std::string displayedText = getDisplayedText();
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);

                    int textIndex = font->getIndexAtLocation(displayedText.c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
                        textAlignment, true, rightToLeft);
                    if (_ctrlPressed)
                    {
                        std::string::const_iterator it = std::find_if(displayedText.begin() + (textIndex + 1), displayedText.end(), space);
                        textIndex = std::distance(displayedText.begin(), it);
                    }
                    else
                    {
                        ++textIndex;
                    }
                    font->getLocationAtIndex(displayedText.c_str(), _textBounds, fontSize, &_caretLocation, textIndex,
                        textAlignment, true, rightToLeft);
                    _dirty = true;
                    break;
                }
                case Keyboard::KEY_UP_ARROW:
                {
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);
                    _prevCaretLocation.set(_caretLocation);
                    _caretLocation.y -= fontSize;
                    int textIndex = font->getIndexAtLocation(getDisplayedText().c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
                        textAlignment, true, rightToLeft);
                    if (textIndex == -1)
                    {
                        _caretLocation.set(_prevCaretLocation);
                    }

                    _dirty = true;
                    break;
                }
                case Keyboard::KEY_DOWN_ARROW:
                {
                    Font* font = getFont(_state);
                    GP_ASSERT(font);
                    unsigned int fontSize = getFontSize(_state);
                    Font::Justify textAlignment = getTextAlignment(_state);
                    bool rightToLeft = getTextRightToLeft(_state);
                    _prevCaretLocation.set(_caretLocation);
                    _caretLocation.y += fontSize;
                    int textIndex = font->getIndexAtLocation(getDisplayedText().c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
                        textAlignment, true, rightToLeft);
                    if (textIndex == -1)
                    {
                        _caretLocation.set(_prevCaretLocation);
                    }

                    _dirty = true;
                    break;
                }
            }
            break;
        }

        case Keyboard::KEY_CHAR:
        {
            Font* font = getFont(_state);
            GP_ASSERT(font);
            unsigned int fontSize = getFontSize(_state);
            Font::Justify textAlignment = getTextAlignment(_state);
            bool rightToLeft = getTextRightToLeft(_state);

            int textIndex = font->getIndexAtLocation(getDisplayedText().c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
                textAlignment, true, rightToLeft);
            if (textIndex == -1)
            {
                textIndex = 0;
                font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, 0,
                    textAlignment, true, rightToLeft);
            }

            switch (key)
            {
                case Keyboard::KEY_BACKSPACE:
                {
                    if (textIndex > 0)
                    {
                        --textIndex;
                        _text.erase(textIndex, 1);
                        font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, textIndex,
                            textAlignment, true, rightToLeft);

                        _dirty = true;
                    }
                    break;
                }
                case Keyboard::KEY_RETURN:
                    // TODO: Handle line-break insertion correctly.
                    break;
                case Keyboard::KEY_ESCAPE:
                    break;
                case Keyboard::KEY_TAB:
                    // Allow tab to move the focus forward.
                    return false;
                    break;
                default:
                {
                    // Insert character into string.
                    _text.insert(textIndex, 1, (char)key);

                    // Get new location of caret.
                    font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, textIndex + 1,
                        textAlignment, true, rightToLeft);

                    if (key == ' ')
                    {
                        // If a space was entered, check that caret is still within bounds.
                        if (_caretLocation.x >= _textBounds.x + _textBounds.width ||
                            _caretLocation.y >= _textBounds.y + _textBounds.height)
                        {
                            // If not, undo the character insertion.
                            _text.erase(textIndex, 1);
                            font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, textIndex,
                                textAlignment, true, rightToLeft);

                            // No need to check again.
                            break;
                        }
                    }

                    // Always check that the text still fits within the clip region.
                    Rectangle textBounds;
                    font->measureText(getDisplayedText().c_str(), _textBounds, fontSize, &textBounds, textAlignment, true, true);
                    if (textBounds.x < _textBounds.x || textBounds.y < _textBounds.y ||
                        textBounds.width >= _textBounds.width || textBounds.height >= _textBounds.height)
                    {
                        // If not, undo the character insertion.
                        _text.erase(textIndex, 1);
                        font->getLocationAtIndex(getDisplayedText().c_str(), _textBounds, fontSize, &_caretLocation, textIndex,
                            textAlignment, true, rightToLeft);

                        // TextBox is not dirty.
                        break;
                    }
                
                    _dirty = true;
                    break;
                }
            
                break;
            }

            notifyListeners(Control::Listener::TEXT_CHANGED);
            break;
        }
        case Keyboard::KEY_RELEASE:
            switch (key)
            {
                case Keyboard::KEY_CTRL:
                {
                    _ctrlPressed = false;
                    break;
                }
            }
    }

    _lastKeypress = key;

    return _consumeInputEvents;
}

void TextBox::update(const Control* container, const Vector2& offset)
{
    Label::update(container, offset);

    _fontSize = getFontSize(_state);
    _caretImage = getImage("textCaret", _state);
}

void TextBox::drawImages(SpriteBatch* spriteBatch, const Rectangle& clip)
{
    if (_caretImage && (_state == ACTIVE || hasFocus()))
    {
        // Draw the cursor at its current location.
        const Rectangle& region = _caretImage->getRegion();
        if (!region.isEmpty())
        {
            GP_ASSERT(spriteBatch);
            const Theme::UVs uvs = _caretImage->getUVs();
            Vector4 color = _caretImage->getColor();
            color.w *= _opacity;

            spriteBatch->draw(_caretLocation.x - (region.width / 2.0f), _caretLocation.y, region.width, _fontSize, uvs.u1, uvs.v1, uvs.u2, uvs.v2, color, _viewportClipBounds);
        }
    }

    _dirty = false;
}

void TextBox::setCaretLocation(int x, int y)
{
    // Get index into string and cursor location from the latest touch location.
    _prevCaretLocation.set(_caretLocation);
    _caretLocation.set(x + _absoluteBounds.x,
                       y + _absoluteBounds.y);

    Font* font = getFont(_state);
    unsigned int fontSize = getFontSize(_state);
    Font::Justify textAlignment = getTextAlignment(_state);
    bool rightToLeft = getTextRightToLeft(_state);
    const std::string displayedText = getDisplayedText();

    int index = font->getIndexAtLocation(displayedText.c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
            textAlignment, true, rightToLeft);

    if (index == -1)
    {
        // Attempt to find the nearest valid caret location.
        Rectangle textBounds;
        font->measureText(displayedText.c_str(), _textBounds, fontSize, &textBounds, textAlignment, true, true);

        if (_caretLocation.x > textBounds.x + textBounds.width &&
            _caretLocation.y > textBounds.y + textBounds.height)
        {
            font->getLocationAtIndex(displayedText.c_str(), _textBounds, fontSize, &_caretLocation, (unsigned int)_text.length(),
                textAlignment, true, rightToLeft);
            return;
        }

        if (_caretLocation.x < textBounds.x)
        {
            _caretLocation.x = textBounds.x;
        }
        else if (_caretLocation.x > textBounds.x + textBounds.width)
        {
            _caretLocation.x = textBounds.x + textBounds.width;
        }

        if (_caretLocation.y < textBounds.y)
        {
            _caretLocation.y = textBounds.y;
        }
        else if (_caretLocation.y > textBounds.y + textBounds.height)
        {
            Font* font = getFont(_state);
            GP_ASSERT(font);
            unsigned int fontSize = getFontSize(_state);
            _caretLocation.y = textBounds.y + textBounds.height - fontSize;
        }

        index = font->getIndexAtLocation(displayedText.c_str(), _textBounds, fontSize, _caretLocation, &_caretLocation,
            textAlignment, true, rightToLeft);

        if (index == -1)
        {
            // We failed to find a valid location; just put the caret back to where it was.
            _caretLocation.set(_prevCaretLocation);
        }
    }
}

const char* TextBox::getType() const
{
    return "textBox";
}

void TextBox::setPasswordChar(char character)
{
    _passwordChar = character;
}

char TextBox::getPasswordChar() const
{
    return _passwordChar;
}

void TextBox::setInputMode(InputMode inputMode)
{
    _inputMode = inputMode;
}

TextBox::InputMode TextBox::getInputMode() const
{
    return _inputMode;
}

void TextBox::drawText(const Rectangle& clip)
{
    if (_text.size() <= 0)
        return;

    // Draw the text.
    if (_font)
    {
        const std::string displayedText = getDisplayedText();
        _font->start();
        _font->drawText(displayedText.c_str(), _textBounds, _textColor, getFontSize(_state), getTextAlignment(_state), true, getTextRightToLeft(_state), &_viewportClipBounds);
        _font->finish();
    }
}

TextBox::InputMode TextBox::getInputMode(const char* inputMode)
{
    if (!inputMode)
    {
        return TextBox::TEXT;
    }

    if (strcmp(inputMode, "TEXT") == 0)
    {
        return TextBox::TEXT;
    }
    else if (strcmp(inputMode, "PASSWORD") == 0)
    {
        return TextBox::PASSWORD;
    }
    else
    {
        GP_ERROR("Failed to get corresponding textbox inputmode for unsupported value '%s'.", inputMode);
    }

    // Default.
    return TextBox::TEXT;
}

std::string TextBox::getDisplayedText() const
{
    std::string displayedText;
    switch (_inputMode) {
        case PASSWORD:
            displayedText.insert((size_t)0, _text.length(), _passwordChar);
            break;

        case TEXT:
        default:
            displayedText = _text;
            break;
    }

    return displayedText;
}

}

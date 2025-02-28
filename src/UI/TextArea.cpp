#include <algorithm>
#include <sstream>
#include <SDL.h>
#include "../CrossPlatform.h"
#include "../Event/Mouse.h"
#include "../Game/Game.h"
#include "../Graphics/Font.h"
#include "../Graphics/Rect.h"
#include "../ResourceManager.h"
#include "../UI/TextArea.h"

namespace Falltergeist
{
    namespace UI
    {
        using Point = Graphics::Point;
        using Rect = Graphics::Rect;
        using Size = Graphics::Size;

        TextArea::TextArea(const Graphics::Point& pos) : Base(pos)
        {
            _timestampCreated = SDL_GetTicks();
        }

        TextArea::TextArea(int x, int y) : TextArea(Point(x, y))
        {
        }

        TextArea::TextArea(const std::string& text, const Graphics::Point& pos) : Base(pos)
        {
            _timestampCreated = SDL_GetTicks();
            setText(text);
        }

        TextArea::TextArea(const std::string& text, int x, int y) : TextArea(text, Point(x, y))
        {
        }

        TextArea::TextArea(const TextArea& textArea, Graphics::Point pos) : Base(pos)
        {
            _timestampCreated = textArea._timestampCreated;
            _text = textArea._text;
            _font = textArea._font;
            _color = textArea._color;
            _outlineColor = textArea._outlineColor;
            _backgroundColor = textArea._backgroundColor;
            _size = textArea._size;
            _horizontalAlign = textArea._horizontalAlign;
            _verticalAlign = textArea._verticalAlign;
            _wordWrap = textArea._wordWrap;
        }

        TextArea::~TextArea()
        {
        }

        void TextArea::_needUpdate(bool lines)
        {
            _changed = true;
            if (lines) {
                _lines.clear();
            }
        }

        void TextArea::appendText(const std::string& text)
        {
            _text += text;
            _needUpdate(true);
        }

        TextArea::HorizontalAlign TextArea::horizontalAlign() const
        {
            return _horizontalAlign;
        }

        void TextArea::setHorizontalAlign(HorizontalAlign align)
        {
            if (_horizontalAlign == align) {
                return;
            }
            _horizontalAlign = align;
            _needUpdate();
        }

        TextArea::VerticalAlign TextArea::verticalAlign() const
        {
            return _verticalAlign;
        }

        void TextArea::setVerticalAlign(VerticalAlign align)
        {
            if (_verticalAlign == align) {
                return;
            }
            _verticalAlign = align;
            _needUpdate();
        }

        void TextArea::setText(const std::string& text)
        {
            _text = text;
            _needUpdate(true);
        }

        Graphics::Font* TextArea::font()
        {
            if (!_font) {
                _font = ResourceManager::getInstance()->font();
            }
            return _font;
        }

        void TextArea::setFont(Graphics::Font* font) {
            _font = font;
            _needUpdate(true);
        }


        void TextArea::setFont(Graphics::Font *font, const Graphics::Color &color) {
            setFont(font);
            _color = color;
        }

        void TextArea::setFont(const std::string& fontName, const Graphics::Color &color) {
            setFont(ResourceManager::getInstance()->font(fontName));
            _color = color;
        }

        std::string TextArea::fontName() {
            return font()->filename();
        }

        void TextArea::setWordWrap(bool wordWrap) {
            if (_wordWrap == wordWrap) {
                return;
            }
            _wordWrap = wordWrap;
            _needUpdate(true);
        }

        bool TextArea::wordWrap() const {
            return _wordWrap;
        }

        void TextArea::setColor(const Graphics::Color &color)
        {
            _color = color;
        }

        void TextArea::setOutline(bool outline)
        {
            _outlineColor = outline ? _outlineColor.withAlpha(255) : _outlineColor.withAlpha(0);
        }

        bool TextArea::outline() const
        {
            return _outlineColor.alpha() != 0;
        }

        void TextArea::setOutlineColor(const Graphics::Color &color)
        {
            _outlineColor = color;
        }

        const Graphics::Color& TextArea::outlineColor() const
        {
            return _outlineColor;
        }

        int TextArea::lineOffset() const
        {
            return _lineOffset;
        }

        void TextArea::setLineOffset(int offset)
        {
            _lineOffset = offset;
            _needUpdate();
        }

        const Graphics::Size& TextArea::size() const
        {
            return _size.width() && _size.height() ? _size : _calculatedSize;
        }

        Graphics::Size TextArea::textSize()
        {
            _updateSymbols();
            return _calculatedSize;
        }

        int TextArea::numLines()
        {
            _updateLines();
            return static_cast<int>(_lines.size());
        }

        void TextArea::setSize(const Graphics::Size& size)
        {
            if (_size == size) {
                return;
            }
            // width affect line composition, so we need full update
            _needUpdate(_size.width() != size.width());
            _size = size;
        }

        void TextArea::setWidth(int width)
        {
            setSize({width, _size.height()});
        }

        // TODO: anyone is welcome to do this better..
        void TextArea::_updateSymbols()
        {
            if (!_changed) {
                return;
            }

            _symbols.clear();

            if (_text.empty())
            {
                _updateBuffers();
                _calculatedSize = Size(0, 0);
                _changed = false;
                return;
            }

            _updateLines();

            // at positive offset, skip number of first lines
            auto lineBegin = std::min(
                _lines.cbegin() + (_lineOffset > 0 ? _lineOffset : 0),
                _lines.cend()
            );
            auto lineEnd = _lines.cend();
            if (_size.height())
            {
                // calculate how much lines we can fit inside TextArea, taking vertical padding into account
                auto activeHeight = _size.height() - _paddingTopLeft.height() - _paddingBottomRight.height();

                if ((_lineOffset + ((activeHeight + font()->verticalGap()) / (font()->height() + font()->verticalGap()))) < (int)_lines.size())
                {
                    lineEnd = std::max(
                        std::min(
                            _lines.cbegin() + _lineOffset + ((activeHeight + font()->verticalGap()) / (font()->height() + font()->verticalGap())),
                            _lines.cend()
                        ),
                        _lines.cbegin()
                    );
                }
                else
                {
                    lineEnd = _lines.cend();
                }
            }

            auto numVisibleLines = static_cast<int>(std::distance(lineBegin, lineEnd));

            // Calculating textarea sizes if needed
            if (numVisibleLines > 0)
            {
                _calculatedSize.setWidth(std::max_element(lineBegin, lineEnd)->width);
                _calculatedSize.setHeight(numVisibleLines * font()->height() + (numVisibleLines - 1) * font()->verticalGap());
            }
            else
            {
                _calculatedSize = Size(0, 0);
            }

            // Alignment

            Point offset;
            // lines are generated with respect to horizontal padding only, so we should only change vertical;
            // on negative offset, add padding at the top, on positive - shift lines upwards
            offset.setY(_paddingTopLeft.height() + (font()->height() + font()->verticalGap()) * (- _lineOffset));
            // index of visible line
            size_t lineIdx = _lineOffset < 0 ? (size_t)(- _lineOffset) : 0;
            for (auto it = lineBegin; it != lineEnd; ++it, ++lineIdx)
            {
                auto& line = *it;
                offset.setX(0);
                if (_horizontalAlign != HorizontalAlign::LEFT)
                {
                    offset.setX((_size.width() ? _size.width() : _calculatedSize.width()) - line.width);
                    if (_horizontalAlign == HorizontalAlign::CENTER)
                    {
                        offset.rx() /= 2;
                    }
                }
                if (lineIdx < _customLineShifts.size())
                {
                    offset.rx() += _customLineShifts[lineIdx];
                }

                for (Graphics::TextSymbol symbol : line.symbols)
                {
                    symbol.position += offset;
                    // outline symbols
                    _symbols.push_back(symbol);
                }
            }
            _updateBuffers();
            _changed = false;
        }

        void TextArea::_updateLines()
        {
            // check if already generated
            if (!_lines.empty()) {
                return;
            }

            _lines.resize(1);

            // here we respect only horizontal padding in order to properly wrap lines; vertical is handled on higher level
            int x = _paddingTopLeft.width(),
                y = 0,
                wordWidth = 0,
                maxWidth = _size.width() ? (_size.width() - _paddingBottomRight.width()) : 0;

            // Parsing lines of text
            // Cutting lines when it is needed (\n or when exceeding _width)
            std::istringstream istream(_text);
            std::string word, part;
            auto aFont = font();

            // on first iteation, process only leading whitespaces
            while (!istream.eof() && isspace((int)istream.peek()))
            {
                part.push_back((char)istream.get());
            }
            do
            {
                if (word.size() > 0)
                {
                    // calculate word width
                    wordWidth = 0;
                    for (unsigned char ch : word)
                    {
                        wordWidth += aFont->glyphWidth(ch) + aFont->horizontalGap();
                    }
                    // switch to next line if word is too long
                    if (_wordWrap && maxWidth && (x + wordWidth) > maxWidth)
                    {
                        part.push_back('\n');
                    }
                    part += word;
                    // include trailing whitespaces
                    while (!istream.eof() && isspace((int)istream.peek()))
                    {
                        part.push_back((char)istream.get());
                    }
                }
                // place the part
                for (unsigned char ch : part)
                {
                    if (ch == ' ')
                    {
                        x += aFont->spaceWidth() + aFont->horizontalGap();
                    }

                    if (ch == '\n' || (_wordWrap && maxWidth && x >= maxWidth))
                    {
                        _lines.back().width = x;
                        x = 0;
                        y += aFont->height() + aFont->verticalGap();
                        _lines.emplace_back();
                    }

                    if (ch == ' ' || ch == '\n') {
                        continue;
                    }

                    Line& line = _lines.back();
                    Graphics::TextSymbol symbol {ch, {x, y}};
                    line.symbols.push_back(symbol);
                    x += aFont->glyphWidth(ch) + aFont->horizontalGap();
                    line.width = x;
                }
                part.clear();

            } while (istream >> word);
        }

        std::string TextArea::text() const
        {
            return _text;
        }

        unsigned int TextArea::timestampCreated() const
        {
            return _timestampCreated;
        }

        void TextArea::render(bool eggTransparency) {
            if (_changed) {
                _updateSymbols();
            }

            auto pos = position();

            _textArea.render(pos, font(), _color, _outlineColor);
        }

        TextArea& TextArea::operator<<(const std::string& text) {
            appendText(text);
            return *this;
        }

        TextArea& TextArea::operator<<(unsigned value) {
            appendText(std::to_string(value));
            return *this;
        }

        TextArea& TextArea::operator<<(signed value) {
            appendText(std::to_string(value));
            return *this;
        }

        TextArea& TextArea::operator=(const std::string& text)
        {
            setText(text);
            return *this;
        }

        TextArea& TextArea::operator=(unsigned value)
        {
            setText(std::to_string(value));
            return *this;
        }

        TextArea& TextArea::operator=(signed value)
        {
            setText(std::to_string(value));
            return *this;
        }

        TextArea& TextArea::operator+=(const std::string& text)
        {
            appendText(text);
            return *this;
        }

        TextArea& TextArea::operator+=(unsigned value)
        {
            appendText(std::to_string(value));
            return *this;
        }

        TextArea& TextArea::operator+=(signed value)
        {
            appendText(std::to_string(value));
            return *this;
        }

        const Graphics::Size& TextArea::paddingTopLeft() const
        {
            return _paddingTopLeft;
        }

        void TextArea::setPaddingTopLeft(const Graphics::Size& pad)
        {
            _needUpdate(_paddingTopLeft.width() != pad.width());
            _paddingTopLeft = pad;
        }

        const Graphics::Size& TextArea::paddingBottomRight() const
        {
            return _paddingBottomRight;
        }

        void TextArea::setPaddingBottomRight(const Graphics::Size& pad)
        {
            _needUpdate(_paddingBottomRight.width() != pad.width());
            _paddingBottomRight = pad;
        }

        void TextArea::setPadding(const Graphics::Size& topLeft, const Graphics::Size& bottomRight)
        {
            setPaddingTopLeft(topLeft);
            setPaddingBottomRight(bottomRight);
        }

        const std::vector<int>& TextArea::customLineShifts() const
        {
            return _customLineShifts;
        }

        void TextArea::setCustomLineShifts(std::vector<int> shifts)
        {
            _customLineShifts = shifts;
        }

        void TextArea::_updateBuffers()
        {
            std::vector<glm::vec2> vertices;
            std::vector<glm::vec2> UV;
            std::vector<unsigned int> indexes;

            int cnt = 0;
            auto tex = font()->texture();
            for ( auto symbol: _symbols )
            {
                auto textureX = static_cast<float>((symbol.chr % 16) * font()->width() + (symbol.chr % 16) * 2 + 1);
                auto textureY = static_cast<float>((symbol.chr / 16) * font()->height() + (symbol.chr / 16) * 2 + 1);

                Point drawPos = symbol.position;

                glm::vec2 vertex_up_left    = glm::vec2( (float)drawPos.x()-1.0, (float)drawPos.y()-1.0 );
                glm::vec2 vertex_up_right   = glm::vec2( (float)drawPos.x()+(float)font()->width()+1.0, (float)drawPos.y()-1.0 );
                glm::vec2 vertex_down_left  = glm::vec2( (float)drawPos.x()-1.0, (float)drawPos.y()+(float)font()->height()+1.0 );
                glm::vec2 vertex_down_right = glm::vec2( (float)drawPos.x()+(float)font()->width()+1.0, (float)drawPos.y()+(float)font()->height()+1.0 );

                vertices.push_back(vertex_up_left   );
                vertices.push_back(vertex_up_right  );
                vertices.push_back(vertex_down_left );
                vertices.push_back(vertex_down_right);

                glm::vec2 tex_up_left    = glm::vec2( (textureX-1.0)/(float)tex->size().width(), (textureY-1.0)/(float)tex->size().height() );
                glm::vec2 tex_up_right   = glm::vec2( (textureX+(float)font()->width()+1.0)/(float)tex->size().width(), (textureY-1.0)/(float)tex->size().height() );
                glm::vec2 tex_down_left  = glm::vec2( (textureX-1.0)/(float)tex->size().width(), (textureY+(float)font()->height()+1.0)/(float)tex->size().height() );
                glm::vec2 tex_down_right = glm::vec2( (textureX+(float)font()->width()+1.0)/(float)tex->size().width(), (textureY+(float)font()->height()+1.0)/(float)tex->size().height() );

                UV.push_back(tex_up_left   );
                UV.push_back(tex_up_right  );
                UV.push_back(tex_down_left );
                UV.push_back(tex_down_right);

                indexes.push_back(cnt*4);
                indexes.push_back(cnt*4+1);
                indexes.push_back(cnt*4+2);
                indexes.push_back(cnt*4+3);
                indexes.push_back(cnt*4+2);
                indexes.push_back(cnt*4+1);
                cnt++;
            }
            _textArea.updateBuffers(vertices,UV,indexes);
        }

        bool TextArea::opaque(const Graphics::Point &pos)
        {
            return Rect::inRect(pos, this->size());
        }
    }
}

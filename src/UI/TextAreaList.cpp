#include "../Graphics/Rect.h"
#include "../UI/TextArea.h"
#include "../UI/TextAreaList.h"

namespace Falltergeist
{
    namespace UI
    {
        using Size = Graphics::Size;

        TextAreaList::TextAreaList(const Graphics::Point &pos) : Falltergeist::UI::Base(pos)
        {
        }

        TextAreaList::~TextAreaList()
        {
        }

        void  TextAreaList::setSize(Graphics::Size size)
        {
            _size=size;
        }

        void TextAreaList::addArea(std::unique_ptr<TextArea> area)
        {
            _areas.push_back(std::move(area));
            _recalculatePositions();
        }

        const std::vector<std::unique_ptr<TextArea>> &TextAreaList::textAreas() const
        {
            return _areas;
        }

        bool TextAreaList::opaque(const Graphics::Point &pos)
        {
            return Graphics::Rect::inRect(pos, this->size());
        }

        void TextAreaList::render(bool eggTransparency)
        {
            for (unsigned int i = 0; i < _visibleCount; i++) {
                _areas.at(_areaIndex+i)->render(eggTransparency);
            }
        }

        void TextAreaList::scrollTo(int index)
        {
            _areaIndex=index;
            if (_areaIndex >= static_cast<int>(_areas.size())) {
                _areaIndex = static_cast<int>(_areas.size());
            }
            if (_areaIndex < 0) {
                _areaIndex = 0;
            }
            _recalculatePositions();
        }


        void TextAreaList::scrollDown(int count)
        {
            if (_areaIndex+count>=(int)_areas.size()) {
                return;
            }
            _areaIndex+=count;
            _recalculatePositions();
        }

        void TextAreaList::scrollUp(int count)
        {
            if (_areaIndex-count<0) {
                return;
            }
            _areaIndex-=count;
            _recalculatePositions();
        }

        void TextAreaList::_recalculatePositions()
        {
            _totalHeight = 0;
            _visibleCount = 0;
            for (unsigned int i = _areaIndex; i < _areas.size(); i++) {
                _areas.at(i)->setPosition(position() + Size(0, _totalHeight));
                _totalHeight+=_areas.at(i)->textSize().height()+_areas.at(i)->font()->verticalGap()*2;

                if (_totalHeight>_size.height()) {
                    break;
                }
                _visibleCount++;
            }
        }
    }
}

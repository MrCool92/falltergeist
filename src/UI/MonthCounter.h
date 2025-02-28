#pragma once

#include "../Graphics/Sprite.h"
#include "../UI/Base.h"

namespace Falltergeist
{
    namespace UI
    {
        class Image;

        class MonthCounter : public Base
        {
            public:

                enum Month
                {
                    JANUARY = 0,
                    FEBRUARY,
                    MARCH,
                    APRIL,
                    MAY,
                    JUNE,
                    JULY,
                    AUGUST,
                    SEPTEMBER,
                    OCTOBER,
                    NOVEMBER,
                    DECEMBER
                };

                MonthCounter(const Graphics::Point& pos = Graphics::Point(0, 0));

                MonthCounter(Month month, const Graphics::Point& pos = Graphics::Point(0, 0));

                Month month() const;

                void setMonth(Month month);

                bool opaque(const Graphics::Point &pos) override;

                void render(bool eggTransparency) override;

            private:
                Month _month;

                Graphics::Size _size;

                std::shared_ptr<Graphics::Sprite> _sprite;

                std::vector<Graphics::Rectangle> _rects;
        };
    }
}

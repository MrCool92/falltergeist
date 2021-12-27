#include <memory>
#include "../Audio/Mixer.h"
#include "../Event/Event.h"
#include "../Event/Mouse.h"
#include "../Game/Game.h"
#include "../UI/Image.h"
#include "../UI/Slider.h"

namespace Falltergeist
{
    namespace UI
    {
        Slider::Slider(const Point& pos, std::unique_ptr<Image> imageOn, std::unique_ptr<Image> imageOff) : Base(pos)
        {
            mouseDragHandler().add([=](Event::Mouse* event) { _onDrag(event); });
            mouseDownHandler().add([=](Event::Mouse* event) { _onLeftButtonDown(event); });
            mouseUpHandler().add([=](Event::Mouse* event) { _onLeftButtonUp(event); });
            this->imageOn = std::move(imageOn);
            this->imageOff = std::move(imageOff);
            _downSound = "sound/sfx/ib1p1xx1.acm";
            _upSound = "sound/sfx/ib1lu1x1.acm";
        }

        void Slider::handle(Event::Event* event)
        {
            if (auto mouseEvent = dynamic_cast<Event::Mouse*>(event)) {
                Point ofs = mouseEvent->position() - _position;

                bool opaque = imageOn->opaque(mouseEvent->position() - _offset);

                //if we are in slider coordinates and not on thumb (slider size = 218 + thumb size, thumb size = 21)
                if (ofs.x() > 0 && ofs.x() < 239 && ofs.y() > 0 && ofs.y() < imageOn->size().height() && !opaque)
                {
                    //on left button up only when not dragging thumb
                    if (mouseEvent->name() == "mouseup" && mouseEvent->leftButton() && !_drag)
                    {
                        ofs -= Point(10, 0); //~middle of thumb
                        if (ofs.x() < 0)
                        {
                            ofs.setX(0);
                        }
                        else if (ofs.y() > 218)
                        {
                            ofs.setX(218);
                        }
                        _offset.setX(ofs.x());
                        _value = ((maxValue() - minValue()) / 218.f) * (float)_offset.x();
                        emitEvent(std::make_unique<Event::Event>("change"), changeHandler());
                        Game::Game::getInstance()->mixer()->playACMSound(_downSound);
                        Game::Game::getInstance()->mixer()->playACMSound(_upSound);
                        return;
                    }
                }
            }
            //pass it to default handler
            Falltergeist::UI::Base::handle(event);
        }

        void Slider::_onDrag(Event::Mouse* event)
        {
            auto newOffset = _offset.x() + event->offset().x();
            if (newOffset <= 218 && newOffset >= 0)
            {
                _offset.setX(newOffset);
                _value = ((maxValue() - minValue())/218.f)*(float)offset().x();
                emitEvent(std::make_unique<Event::Event>("change"), changeHandler());
            }
        }

        void Slider::_onLeftButtonDown(Event::Mouse* event)
        {
            if (!_downSound.empty())
            {
                Game::Game::getInstance()->mixer()->playACMSound(_downSound);
            }
        }

        void Slider::_onLeftButtonUp(Event::Mouse* event)
        {
            if (!_upSound.empty())
            {
                Game::Game::getInstance()->mixer()->playACMSound(_upSound);
            }
        }

        double Slider::minValue() const
        {
            return _minValue;
        }

        void Slider::setMinValue(double value)
        {
            _minValue = value;
        }

        double Slider::maxValue() const
        {
            return _maxValue;
        }

        void Slider::setMaxValue(double value)
        {
            _maxValue = value;
        }

        double Slider::value() const
        {
            return _value;
        }

        void Slider::setValue(double value)
        {
            _value = value;
            _offset.setX(static_cast<int>((218.f/(maxValue() - minValue())) * _value));
            emitEvent(std::make_unique<Event::Event>("change"), changeHandler());
        }

        Event::Handler& Slider::changeHandler()
        {
            return _changeHandler;
        }

        void Slider::render(bool eggTransparency)
        {
            imageOn->setPosition(position());
            imageOff->setPosition(position());
            if (_drag) {
                imageOn->render(eggTransparency);
            } else {
                imageOff->render(eggTransparency);
            }
        }

        bool Slider::opaque(const Point &pos)
        {
            return imageOn->opaque(pos);
        }

        Size Slider::size() const
        {
            return imageOn->size();
        }
    }
}

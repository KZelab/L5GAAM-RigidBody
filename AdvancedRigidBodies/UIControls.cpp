#include "UIControls.hpp"
#include <sstream>
#include <iomanip>

UIControls::UIControls(sf::Font& f)
    : font(f),
      gravitySlider(f),
      restitutionSlider(f),
      frictionSlider(f),
      titleText(f),
      instructionsText(f),
      statsText(f) {
    backgroundPanel.setSize(sf::Vector2f(300.0f, 650.0f));
    backgroundPanel.setPosition(sf::Vector2f(10.0f, 10.0f));
    backgroundPanel.setFillColor(sf::Color(30, 30, 40, 230));
    backgroundPanel.setOutlineThickness(2.0f);
    backgroundPanel.setOutlineColor(sf::Color(100, 100, 120));

    titleText.setString("Physics Controls");
    titleText.setCharacterSize(20);
    titleText.setPosition(sf::Vector2f(20.0f, 20.0f));
    titleText.setFillColor(sf::Color::White);

    instructionsText.setCharacterSize(14);
    instructionsText.setPosition(sf::Vector2f(20.0f, 360.0f));
    instructionsText.setFillColor(sf::Color(200, 200, 200));
    instructionsText.setString(
        "Controls:\n"
        "Left Click: Add body\n"
        "Right Click: Drag bodies\n"
        "Space: Add 8 bodies\n"
        "C: Clear dynamic bodies\n"
        "G: Toggle gravity\n"
        "V: Toggle velocity vectors\n"
        "T: Toggle motion trails\n"
        "D: Toggle debug visualization\n\n"
        "Debug shows:\n"
        "- Contact points (red)\n"
        "- Collision normals (cyan)\n"
        "- Applied forces (orange)\n"
        "- Rotation indicators (white)"
    );

    statsText.setCharacterSize(14);
    statsText.setPosition(sf::Vector2f(20.0f, 590.0f));
    statsText.setFillColor(sf::Color(150, 255, 150));

    createSlider(gravitySlider, "Gravity", 20.0f, 60.0f, 0.0f, 1000.0f, 500.0f);
    createSlider(restitutionSlider, "Restitution", 20.0f, 140.0f, 0.0f, 1.0f, 0.6f);
    createSlider(frictionSlider, "Friction", 20.0f, 220.0f, 0.0f, 1.0f, 0.3f);
}

void UIControls::createSlider(Slider& slider, const std::string& labelText,
                               float x, float y, float minVal, float maxVal, float initialVal) {
    slider.minValue = minVal;
    slider.maxValue = maxVal;
    slider.currentValue = initialVal;

    slider.background.setSize(sf::Vector2f(260.0f, 4.0f));
    slider.background.setPosition(sf::Vector2f(x, y + 30.0f));
    slider.background.setFillColor(sf::Color(80, 80, 90));

    slider.handle.setSize(sf::Vector2f(12.0f, 20.0f));
    slider.handle.setFillColor(sf::Color(150, 150, 200));
    slider.handle.setOutlineThickness(1.0f);
    slider.handle.setOutlineColor(sf::Color::White);

    slider.label.setString(labelText);
    slider.label.setCharacterSize(14);
    slider.label.setPosition(sf::Vector2f(x, y));
    slider.label.setFillColor(sf::Color::White);

    slider.valueText.setCharacterSize(14);
    slider.valueText.setPosition(sf::Vector2f(x + 200.0f, y));
    slider.valueText.setFillColor(sf::Color(200, 255, 200));

    float ratio = (initialVal - minVal) / (maxVal - minVal);
    slider.handle.setPosition(sf::Vector2f(x + ratio * 260.0f - 6.0f, y + 22.0f));
}

void UIControls::updateSlider(Slider& slider, const sf::Vector2f& mousePos) {
    float x = slider.background.getPosition().x;
    float width = slider.background.getSize().x;
    float ratio = std::clamp((mousePos.x - x) / width, 0.0f, 1.0f);

    slider.currentValue = slider.minValue + ratio * (slider.maxValue - slider.minValue);
    slider.handle.setPosition(sf::Vector2f(x + ratio * width - 6.0f, slider.handle.getPosition().y));
}

float UIControls::getSliderValue(const Slider& slider) const {
    return slider.currentValue;
}

void UIControls::drawSlider(sf::RenderWindow& window, const Slider& slider) {
    window.draw(slider.background);
    window.draw(slider.handle);
    window.draw(slider.label);
    window.draw(slider.valueText);
}

void UIControls::update(float deltaTime) {
    std::ostringstream gravityStream;
    gravityStream << std::fixed << std::setprecision(0) << gravitySlider.currentValue;
    gravitySlider.valueText.setString(gravityStream.str());

    std::ostringstream restitutionStream;
    restitutionStream << std::fixed << std::setprecision(2) << restitutionSlider.currentValue;
    restitutionSlider.valueText.setString(restitutionStream.str());

    std::ostringstream frictionStream;
    frictionStream << std::fixed << std::setprecision(2) << frictionSlider.currentValue;
    frictionSlider.valueText.setString(frictionStream.str());

    if (gravityValue != gravitySlider.currentValue && onGravityChange) {
        gravityValue = gravitySlider.currentValue;
        onGravityChange(gravityValue);
    }

    if (restitutionValue != restitutionSlider.currentValue && onRestitutionChange) {
        restitutionValue = restitutionSlider.currentValue;
        onRestitutionChange(restitutionValue);
    }

    if (frictionValue != frictionSlider.currentValue && onFrictionChange) {
        frictionValue = frictionSlider.currentValue;
        onFrictionChange(frictionValue);
    }
}

void UIControls::updateStats(int bodyCount, float fps) {
    std::ostringstream statsStream;
    statsStream << "Bodies: " << bodyCount << "\n";
    statsStream << "FPS: " << std::fixed << std::setprecision(0) << fps;
    statsText.setString(statsStream.str());
}

void UIControls::draw(sf::RenderWindow& window) {
    window.draw(backgroundPanel);
    window.draw(titleText);
    window.draw(instructionsText);
    window.draw(statsText);

    drawSlider(window, gravitySlider);
    drawSlider(window, restitutionSlider);
    drawSlider(window, frictionSlider);
}

void UIControls::handleEvent(const std::optional<sf::Event>& event, const sf::Vector2f& mousePos) {
    if (!event) return;

    if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            auto checkSlider = [&](Slider& slider) {
                if (slider.handle.getGlobalBounds().contains(mousePos)) {
                    slider.isDragging = true;
                }
            };
            checkSlider(gravitySlider);
            checkSlider(restitutionSlider);
            checkSlider(frictionSlider);
        }
    }

    if (event->is<sf::Event::MouseButtonReleased>()) {
        gravitySlider.isDragging = false;
        restitutionSlider.isDragging = false;
        frictionSlider.isDragging = false;
    }

    if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
        sf::Vector2f pos(static_cast<float>(mouseMoved->position.x),
                        static_cast<float>(mouseMoved->position.y));

        if (gravitySlider.isDragging) updateSlider(gravitySlider, pos);
        if (restitutionSlider.isDragging) updateSlider(restitutionSlider, pos);
        if (frictionSlider.isDragging) updateSlider(frictionSlider, pos);
    }
}

bool UIControls::isMouseOverUI(const sf::Vector2f& mousePos) const {
    return backgroundPanel.getGlobalBounds().contains(mousePos);
}

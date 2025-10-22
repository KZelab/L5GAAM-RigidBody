#pragma once
#include <SFML/Graphics.hpp>
#include <functional>

class UIControls {
public:
    UIControls(sf::Font& font);

    void update(float deltaTime);
    void draw(sf::RenderWindow& window);
    void handleEvent(const std::optional<sf::Event>& event, const sf::Vector2f& mousePos);

    bool isMouseOverUI(const sf::Vector2f& mousePos) const;

    std::function<void(float)> onGravityChange;
    std::function<void(float)> onRestitutionChange;
    std::function<void(float)> onFrictionChange;

    float getGravity() const { return gravityValue; }
    float getRestitution() const { return restitutionValue; }
    float getFriction() const { return frictionValue; }

    bool showVelocityVectors = true;
    bool showMotionTrails = true;
    bool showDebugVisualization = false;

    void updateStats(int bodyCount, float fps);

private:
    struct Slider {
        sf::RectangleShape background;
        sf::RectangleShape handle;
        sf::Text label;
        sf::Text valueText;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float currentValue = 0.0f;
        bool isDragging = false;

        Slider(sf::Font& font) : label(font), valueText(font) {}
    };

    void createSlider(Slider& slider, const std::string& labelText,
                      float x, float y, float minVal, float maxVal, float initialVal);
    void updateSlider(Slider& slider, const sf::Vector2f& mousePos);
    void drawSlider(sf::RenderWindow& window, const Slider& slider);
    float getSliderValue(const Slider& slider) const;

    sf::Font& font;
    Slider gravitySlider;
    Slider restitutionSlider;
    Slider frictionSlider;

    sf::Text titleText;
    sf::Text instructionsText;
    sf::Text statsText;

    sf::RectangleShape backgroundPanel;

    float gravityValue = 500.0f;
    float restitutionValue = 0.6f;
    float frictionValue = 0.3f;
};

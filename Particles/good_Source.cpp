#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include "Source.h"
#include <iostream>

// Particle class
class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifespan;
    float size;

    Particle(sf::Vector2f startPos, sf::Vector2f startVel) {
        position = startPos;
        velocity = startVel;  // Use the provided velocity
        color = sf::Color::Yellow;
        lifespan = 11.0f;  // 11 seconds lifespan
        size = 7.0f;
    }

    void update(float deltaTime, float windowWidth, float windowHeight, sf::Vector2f wind) {
        // Apply wind
        velocity += wind * deltaTime;

        // Apply gravity 
        velocity.y += 60 * deltaTime;

        // Move particle
        position += velocity * deltaTime;

        // Bounce off the walls
        if (position.x <= 0) {
            position.x = 0;
            int randomvelocity = (rand() % 3) - 3;
            velocity.y *= randomvelocity + -0.2f;
            velocity.x *= randomvelocity + -0.2f;
            color = sf::Color::Blue;
        }
        if (position.x + size >= windowWidth) {
            position.x = windowWidth - size;
            int randomvelocity = (rand() % 3) - 3;
            velocity.y *= randomvelocity + -0.2f;
            velocity.x *= randomvelocity + -0.2f;
            color = sf::Color::Red;
        }
        if (position.y <= 0) {
            position.y = 0;
            int randomvelocity = (rand() % 3) - 3;
            velocity.y *= randomvelocity + -0.2f;
            velocity.x *= randomvelocity + -0.2f;
            color = sf::Color::Green;
        }
        if (position.y + size >= windowHeight) {
            position.y = windowHeight - size;
            int randomvelocity = (rand() % 3) - 3;
            velocity.y *= randomvelocity + -0.2f;
            velocity.x *= randomvelocity + -0.2f;
            color = sf::Color::White;
        }

        // Decrease lifespan
        lifespan -= deltaTime;
        color.a = static_cast<sf::Uint8>(std::max(0.0f, lifespan / 5.0f * 355));  // Fade out over time

        // Shrink particle over time
        size = std::max(0.0f, size - 0.4f * deltaTime);
    }

    bool isDead() {
        return lifespan <= 0;
    }
};

// ParticleSystem class
class ParticleSystem {
public:
    std::vector<Particle> particles;

    // Update the addParticle function to take both position and velocity
    void addParticle(sf::Vector2f position, sf::Vector2f velocity) {
        particles.push_back(Particle(position, velocity));
    }

    void update(float deltaTime, float windowWidth, float windowHeight, sf::Vector2f wind) {
        for (auto& particle : particles) {
            particle.update(deltaTime, windowWidth, windowHeight, wind);
        }
        // Remove dead particles
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](Particle& p) { return p.isDead(); }), particles.end());
    }

    void render(sf::RenderWindow& window) {
        for (auto& particle : particles) {
            sf::CircleShape shape(particle.size);
            shape.setPosition(particle.position);
            shape.setFillColor(particle.color);
            window.draw(shape);
        }
    }
};

// Function to create a vortex wind effect
sf::Vector2f vortexWind(sf::Vector2f particlePosition, sf::Vector2f vortexCenter, float strength) {
    sf::Vector2f direction = particlePosition - vortexCenter;  // Vector from vortex center to particle
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);  // Distance from vortex center

    if (distance == 0.0f) return sf::Vector2f(0.0f, 0.0f);  // Avoid division by zero

    // Normalize the direction vector
    sf::Vector2f normalizedDirection = direction / distance;

    // Rotate the normalized direction by 90 degrees to get the perpendicular vector (swirl effect)
    sf::Vector2f perpendicularWind(-normalizedDirection.y, normalizedDirection.x);

    // Apply strength to the perpendicular wind vector to create the vortex effect
    return perpendicularWind * strength;
}

// Function to create a light speed wind effect
sf::Vector2f lightSpeedWind(sf::Vector2f particlePosition, sf::Vector2f lightCenter, float strength) {
    // Vector from particle to the center of the screen
    sf::Vector2f direction = lightCenter - particlePosition;


    // Distance from the center (for scaling speed and fade effect)
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

    if (distance == 0.0f) return sf::Vector2f(0.0f, 0.0f);  // Avoid division by zero

    // Normalize the direction vector (so all particles move toward the center)
    sf::Vector2f normalizedDirection = direction / distance;

    // Slow down as they approach the center (the closer they are, the slower they move)
    float scaledStrength = strength * (distance / 50.0f);  // Adjust 50.0f for desired slowdown effect

    return normalizedDirection * scaledStrength;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Particle Simulation with Dynamic Wind");

    // Load a font (make sure you have a font file in your project folder)
    sf::Font font;
    if (!font.loadFromFile("arial.ttf")) {
        std::cout << "Failed to load font!" << std::endl;
        return -1;
    }

    ParticleSystem particleSystem;
    sf::Clock clock;

    // Create a text object
    sf::Text text;
    text.setFont(font);
    text.setString("Press 'P' for Particles 'Arrow' for Wind direction and 'L' for Zoom");
    text.setCharacterSize(20);  // Character size
    text.setFillColor(sf::Color::White);  // Initial color (opaque white)
    text.setPosition(5, 5);  // Position on the screen

    // Wind vector
    sf::Vector2f wind(0.0f, 0.0f);  // Initial wind is zero
    sf::Vector2f vortexCenter(400.0f, 300.0f);  // Center of the vortex at the middle of the window
    float vortexStrength = 0.2f;               // Vortex strength
    float lightStrength = 0.5f;               // Light Speed strength

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        /*
        // Get the elapsed time
        float elapsedTime = clock.getElapsedTime().asSeconds();
        float fadeDuration = 12.0f;  // Time (in seconds) for the text to fade out
        // If elapsed time is greater than 12 seconds, fade out
        if (elapsedTime < fadeDuration) {
            // Calculate how much time has passed as a percentage
            float fadeProgress = elapsedTime / fadeDuration;

            // Fade the text by reducing the alpha value over time
            sf::Uint8 alpha = static_cast<sf::Uint8>(255 * (1.0f - fadeProgress));  // From 255 to 0
            text.setFillColor(sf::Color(255, 255, 255, alpha));  // Set the new color with reduced alpha


        window.clear();
        window.draw(text);  // Draw the fading text
        window.display();
        }
        */
        // Get delta time
        float deltaTime = clock.restart().asSeconds();

        // Add particles where the mouse is clicked
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            particleSystem.addParticle(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)),
                sf::Vector2f(0, 0));  // Initial velocity is zero
        }

        // Control wind using arrow keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            wind.x -= 5.0f;   // Increase wind to the left
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            wind.x += 5.0f;  // Increase wind to the right
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            wind.y -= 5.0f;  // Increase wind upward
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            wind.y += 5.0f;  // Increase wind downward
        }

        // Add particle with random velocity when 'P' is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            unsigned int seed = static_cast<unsigned int>(time(NULL));
            srand(seed);  // Seed the random number generator

            // Generate a random offset in a small range for particle position
            int randomOffsetX = (rand() % 400) - 400;
            int randomOffsetY = (rand() % 300) - 300;

            // Generate random velocity within a certain range
            float randomVelocityX = ((rand() % 300) / 300.0f) * 300.0f - 300.0f;
            float randomVelocityY = ((rand() % 300) / 300.0f) * 300.0f - 300.0f;

            // Add particle with modified mouse position and random velocity
            particleSystem.addParticle(
                sf::Vector2f(static_cast<float>(400 + randomOffsetX), static_cast<float>(300 + randomOffsetY)),
                sf::Vector2f(randomVelocityX, randomVelocityY));
        }

        // Apply vortex wind when 'V' is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
            for (auto& particle : particleSystem.particles) {
                // Calculate vortex wind based on each particle's position
                sf::Vector2f vortexWindForce = vortexWind(particle.position, vortexCenter, vortexStrength);
                wind += vortexWindForce;  // Apply vortex wind to the particle system
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
            // Define the light center as the middle of the window
            sf::Vector2f lightCenter(window.getSize().x / 2.0f, window.getSize().y / 2.0f);

            for (auto& particle : particleSystem.particles) {
                sf::Vector2f lightWindForce = lightSpeedWind(particle.position, lightCenter, lightStrength);
                particle.velocity += lightWindForce;
            }
        }

        // Gradually reduce wind influence when keys are not pressed
        wind *= 0.99f;

        // Update the particle system
        particleSystem.update(deltaTime, window.getSize().x, window.getSize().y, wind);

        // Render the particles
        window.clear();
        particleSystem.render(window);
        window.display();
    }

    return 0;
}

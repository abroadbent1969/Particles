#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>
#include <iostream>

class Particle {
public:
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifespan;
    float size;

    Particle() : position(0, 0), velocity(0, 0), color(sf::Color::White), lifespan(0), size(1) {}

    Particle(sf::Vector2f startPos, sf::Vector2f startVel) {
        position = startPos;
        velocity = startVel;
        color = sf::Color::Magenta;
        lifespan = 11.0f;
        size = 7.0f;
    }

    void update(float deltaTime, float windowWidth, float windowHeight, sf::Vector2f wind) {
        velocity += wind * deltaTime; // Apply wind
        velocity.y += 50 * deltaTime; // Gravity effect
        position += velocity * deltaTime;

        // Handle collisions with window boundaries
        if (position.x <= 0) {
            position.x = 0;  // Prevent particle from going out of bounds
            velocity.x *= -1.5f;  // Bounce effect
            color = sf::Color::Blue;  // Change color on left boundary collision
        }
        if (position.x + size >= windowWidth) {
            position.x = windowWidth - size;  // Prevent particle from going out of bounds
            velocity.x *= -1.5f;  // Bounce effect
            color = sf::Color::Red;  // Change color on right boundary collision
        }
        if (position.y <= 0) {
            position.y = 0;  // Prevent particle from going out of bounds
            velocity.y *= -1.5f;  // Bounce effect
            color = sf::Color::Green;  // Change color on top boundary collision
        }
        if (position.y + size >= windowHeight) {
            position.y = windowHeight - size;  // Prevent particle from going out of bounds
            velocity.y *= -1.5f;  // Bounce effect
            color = sf::Color::White;  // Change color on bottom boundary collision
        }

        // Decrease lifespan and fade out
        lifespan -= deltaTime;
        color.a = static_cast<sf::Uint8>(std::max(0.0f, lifespan / 5.0f * 255));
        size = std::max(0.0f, size - 0.4f * deltaTime); // Shrink particle
    }

    bool isDead() {
        return lifespan <= 0;
    }
};

class ParticleSystem {
public:
    std::vector<Particle> particles;

    void addParticle(sf::Vector2f position, sf::Vector2f velocity) {
        particles.push_back(Particle(position, velocity));
    }

    void update(float deltaTime, float windowWidth, float windowHeight, sf::Vector2f wind) {
        for (auto& particle : particles) {
            particle.update(deltaTime, windowWidth, windowHeight, wind);
        }
        particles.erase(std::remove_if(particles.begin(), particles.end(), [](Particle& p) { return p.isDead(); }), particles.end());
    }

    void updateParticlesBasedOnAudio(const std::vector<float>& audioData) {
        for (size_t i = 0; i < particles.size(); ++i) {
            float scaleFactor = std::abs(audioData[i % audioData.size()]);  // Scale based on audio data
            particles[i].position += particles[i].velocity * scaleFactor;
            //particles[i].color.a = static_cast<sf::Uint8>(scaleFactor * 100);  // Modify alpha value
        }
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
    sf::Vector2f direction = particlePosition - vortexCenter;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (distance == 0.0f) return sf::Vector2f(0.0f, 0.0f);  // Avoid division by zero
    sf::Vector2f perpendicularWind(-direction.y / distance, direction.x / distance);
    return perpendicularWind * strength;
}

// Function to create a light speed wind effect
sf::Vector2f lightSpeedWind(sf::Vector2f particlePosition, sf::Vector2f lightCenter, float strength) {
    sf::Vector2f direction = lightCenter - particlePosition;
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (distance == 0.0f) return sf::Vector2f(0.0f, 0.0f);  // Avoid division by zero
    sf::Vector2f normalizedDirection = direction / distance;
    float scaledStrength = strength * (distance / 20.0f);  // Adjust for desired slowdown effect
    return normalizedDirection * scaledStrength;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Particle Simulation with Dynamic Wind");
    ParticleSystem particleSystem;
    sf::Clock clock;
    sf::Vector2f wind(0.0f, 0.0f);
    sf::Vector2f vortexCenter(400.0f, 300.0f);  // Center of vortex
    float vortexStrength = 0.2f;
    float lightStrength = 0.5f;

    // Load and play audio file
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("pronunciation_assessment.wav")) {
        return -1;
    }
    sf::Sound sound;
    sound.setBuffer(buffer);
    bool audioMode = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::M) {
                    audioMode = !audioMode;
                    if (audioMode) sound.play();
                    else sound.stop();
                }
            }
        }

        float deltaTime = clock.restart().asSeconds();

        // Add particles with left mouse click
        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            particleSystem.addParticle(sf::Vector2f(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)), sf::Vector2f(0, 0));
        }

        // Control wind using arrow keys
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) wind.x -= 5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) wind.x += 5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) wind.y -= 5.0f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) wind.y += 5.0f;

        // Add random particles when 'P' is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            particleSystem.addParticle(
                sf::Vector2f(static_cast<float>(rand() % 800), static_cast<float>(rand() % 600)),
                sf::Vector2f(static_cast<float>(rand() % 10 - 5), static_cast<float>(rand() % 10 - 5))
            );
        }

        // Apply vortex wind when 'V' is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
            for (auto& particle : particleSystem.particles) {
                sf::Vector2f vortexWindForce = vortexWind(particle.position, vortexCenter, vortexStrength);
                wind += vortexWindForce;
            }
        }

        // Apply light speed wind when 'L' is pressed
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)) {
            sf::Vector2f lightCenter(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
            for (auto& particle : particleSystem.particles) {
                sf::Vector2f lightWindForce = lightSpeedWind(particle.position, lightCenter, lightStrength);
                particle.velocity += lightWindForce;
            }
        }

        // Gradually reduce wind influence
        wind *= 0.99f;

        // Update particles
        particleSystem.update(deltaTime, window.getSize().x, window.getSize().y, wind);

        // Update particles based on audio data
        if (audioMode) {
            std::vector<float> audioData(particleSystem.particles.size(), 0.5f); // Fake audio data
            particleSystem.updateParticlesBasedOnAudio(audioData);  // Apply audio data
        }

        // Render particles
        window.clear();
        particleSystem.render(window);
        window.display();
    }

    return 0;
}

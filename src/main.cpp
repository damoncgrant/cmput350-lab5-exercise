#include <iostream>
#include <optional>
#include <vector>
#include <cmath>

#include <SFML/Graphics.hpp>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 800;
const int FPS_LIMIT = 30;

using Point2D = sf::Vector2f;

float distance(Point2D a, Point2D b) {
    return std::sqrt(std::pow((a.x - b.x), 2) + std::pow((a.y - b.y), 2));
}

// TODO: (Part 1) Define a function that samples a cubic Bezier curve at t in [0, 1].
Point2D getPoint(const std::vector<sf::Vector2f>& pts, float t) { 
    static std::vector<Point2D> tmp;
    tmp.clear();
    for (int x = 0; x < pts.size()-1; x++) {
        tmp.push_back(pts[x] * (1.0f-t) + pts[x+1] * t);
    }
    while (tmp.size() > 1) {
        for (int x = 0; x < tmp.size() - 1; x++) {
            tmp[x] = tmp[x] * (1.0f-t) + tmp[x+1] * t;
        }
        tmp.pop_back();
    }
    return tmp[0];
}

// TODO: (Part 2) Define a function that returns the curve's slope at t in [0, 1].
Point2D getSlope(const std::vector<sf::Vector2f>& pts, float t) { 
    const float c = 1 - t;
    // slope = 3(1-t)^2(p2-p1)+6(1-t)t(p3-p2)+3t^2(p4-p3)
    float slopeX = 3 * (c * c) * (pts[1].x - pts[0].x) + 
        6 * c * t * (pts[2].x - pts[1].x) + 3 * (t * t) * (pts[3].x - pts[2].x);
    float slopeY = 3 * (c * c) * (pts[1].y - pts[0].y) + 
        6 * c * t * (pts[2].y - pts[1].y) + 3 * (t * t) * (pts[3].y - pts[2].y);
    return Point2D{slopeX, slopeY}; 
}

// TODO: (Part 1) Store four control points for the curve.
std::vector<Point2D> points = {{50.0f, 50.0f}, {200.0f, 100.0f}, {500.0f, 100.0f}, {600.0f, 500.0f}};
// TODO: (Part 2) Track animation time for the square moving along the curve.
const float ANIMATION_TIME = 2.0f; // 2 seconds
// TODO: (Part 3) Track the index of the control point being dragged.
int currentPoint = -1;

void handleInput(sf::Window& window, bool& shouldQuit) {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            shouldQuit = true;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
            // TODO: (Part 3) On left-click, select the closest control point
            // using mouse->position and start dragging it.
            Point2D pos = static_cast<Point2D>(mouse->position);
            float dis = INFINITY;
            for (int i = 0; i < points.size(); ++i) {
                float currDis = distance(points[i], pos);
                if (currDis < dis) {
                    dis = currDis;
                    currentPoint = i;
                }
            }
            
        } else if (const auto* mouse = event->getIf<sf::Event::MouseButtonReleased>()) {
            // TODO: (Part 3) On left-button release, stop dragging.
            currentPoint = -1;
        } else if (const auto* mouse = event->getIf<sf::Event::MouseMoved>()) {
            // TODO: (Part 3) Move the selected control point to mouse->position.
            if (currentPoint >=0) {
                points[currentPoint] = static_cast<Point2D>(mouse->position);
            }
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Add)) {
                
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Subtract)) {
                if (points.size() > 4) {
                    // Remove 3 points
                    points.pop_back();
                    points.pop_back();
                    points.pop_back();
                }
            }
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    // ====== ====== ======
    // TODO: (Part 1) Sample GetPoint over t in [0, 1] and connect samples using the line-drawing
    // code from your project. Draw all four control points as circles after drawing the curve.
    // ====== ====== ======
    const int samples = 100;
    sf::VertexArray curve(sf::PrimitiveType::LineStrip, samples);

    for (int i = 0; i < samples; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(samples - 1);
        Point2D curr = getPoint(points, t);
        curve[i].position = curr;
        curve[i].color = sf::Color::Cyan;
    }
    window.draw(curve);

    for (int i = 0; i < points.size(); ++i) {
        Point2D curr = points[i];
        sf::CircleShape currPoint(15.0f);
        currPoint.setOrigin({10.0f, 10.0f});
        currPoint.setPosition(curr);
        currPoint.setFillColor(sf::Color::Yellow);
        window.draw(currPoint);
    }

    // ====== ====== ======
    // TODO: (Part 2) Draw a small square moving repeatedly along the curve.
    // Use GetSlope to orient it to the curve at each time step.
    // ====== ====== ======
    sf::RectangleShape square({20.0f, 20.0f});
    square.setOrigin({10.0f, 10.0f});
    square.setFillColor(sf::Color::Green);
    static float t = 0.f;
    static int direction = 1;

    t += direction / (ANIMATION_TIME * FPS_LIMIT);
    if (t >= 1) {
        t = 1.0f;
        direction = -1;
    } else if (t <= 0) {
        t = 0.f;
        direction = 1;
    }
    square.setPosition(getPoint(points, t));

    // Rotation
    Point2D slope = getSlope(points, t);
    float angle = std::atan2(slope.y, slope.x);
    square.setRotation(sf::radians(angle));

    window.draw(square);
    

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    sf::VertexArray line1(sf::PrimitiveType::Lines, 2);
    sf::VertexArray line2(sf::PrimitiveType::Lines, 2);
    line1[0].position = points[0];
    line1[0].color = sf::Color::White;
    line1[1].position = points[1];
    line1[1].color = sf::Color::White;

    line2[0].position = points[2];
    line2[0].color = sf::Color::White;
    line2[1].position = points[3];
    line2[1].color = sf::Color::White;

    window.draw(line1);
    window.draw(line2);

    // TODO: (Part 4) Draw all connected cubic Bezier segments and their handles.
    // ====== ====== ======

    // ====== ====== ======
    // TODO: (Bonus) Support multiple curves, a Galaga screen overlay at a 1:2 ratio, and exporting
    // curve points as C++ code for Project 1b.
    // ====== ====== ======

    window.display();
}

int main() {
    sf::RenderWindow window;

    try {
        // Initialize window
        window.create(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Bezier Curve Editor");
        window.setFramerateLimit(FPS_LIMIT);
        // Prevent key repeats.
        window.setKeyRepeatEnabled(false);

        bool shouldQuit = false;
        // Main game loop
        while (window.isOpen()) {
            handleInput(window, shouldQuit);
            if (shouldQuit) {
                break;
            }
            render(window);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}

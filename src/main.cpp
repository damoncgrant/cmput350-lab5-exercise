#include <algorithm>
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
                Point2D newPosition = static_cast<Point2D>(mouse->position);
                Point2D movement = newPosition - points[currentPoint];
                points[currentPoint] = newPosition;

                if (currentPoint > 0 &&
                    currentPoint + 1 < static_cast<int>(points.size()) &&
                    currentPoint % 3 == 0) {
                    points[currentPoint - 1] += movement;
                    points[currentPoint + 1] += movement;
                }
            }
            // TODO: (Part 4) Maintain matching slopes at shared endpoints.
            // When moving point 3, move point 5 without changing its distance
            // from point 4 (point numbers here start at 1).
            for (int sharedPoint = 3;
                 sharedPoint + 1 < static_cast<int>(points.size());
                 sharedPoint += 3) {
                const int incomingHandle = sharedPoint - 1;
                const int outgoingHandle = sharedPoint + 1;

                if (currentPoint != incomingHandle && currentPoint != outgoingHandle) {
                    continue;
                }

                const bool movingIncoming = currentPoint == incomingHandle;
                const int oppositeHandle = movingIncoming ? outgoingHandle : incomingHandle;
                const float oppositeDistance =
                    distance(points[oppositeHandle], points[sharedPoint]);
                Point2D tangent = movingIncoming
                    ? points[sharedPoint] - points[incomingHandle]
                    : points[outgoingHandle] - points[sharedPoint];
                const float tangentLength =
                    std::sqrt(tangent.x * tangent.x + tangent.y * tangent.y);

                if (tangentLength > 0.0f) {
                    Point2D direction = tangent / tangentLength;
                    points[oppositeHandle] = movingIncoming
                        ? points[sharedPoint] + direction * oppositeDistance
                        : points[sharedPoint] - direction * oppositeDistance;
                }
            }
        } else if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            // TODO: (Part 4) '+' adds three control points; '-' removes three,
            // keeping at least four points.
            const bool addPressed = key->code == sf::Keyboard::Key::Equal ||
                                    key->code == sf::Keyboard::Key::Add;
            const bool subtractPressed = key->code == sf::Keyboard::Key::Hyphen ||
                                         key->code == sf::Keyboard::Key::Subtract;

            if (addPressed) {
                std::vector<Point2D> lastSeg(points.end() - 4, points.end());
                Point2D slope = getSlope(lastSeg, 1.0f);
                float slopeLength = std::sqrt(slope.x * slope.x + slope.y * slope.y);
                Point2D lastPoint = points.back();

                if (slopeLength > 0.0f) {
                    Point2D direction = slope / slopeLength;
                    float spacing = 75.0f;

                    for (int i = 1; i <= 3; ++i) {
                        points.push_back(lastPoint + direction * (spacing * i));
                    }
                }
            } else if (subtractPressed) {
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
    int segCount = (points.size() - 1) / 3;

    for (int segIndex = 0; segIndex < segCount; ++segIndex) {
        int start = segIndex * 3;

        std::vector<Point2D> seg(
            points.begin() + start,
            points.begin() + start + 4
        );

        sf::VertexArray curve(sf::PrimitiveType::LineStrip, samples);

        for (int i = 0; i < samples; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(samples - 1);
            Point2D curr = getPoint(seg, t);
            curve[i].position = curr;
            curve[i].color = sf::Color::Cyan;
        }
        window.draw(curve);
    }
    
    // Drawing control points
    for (int i = 0; i < points.size(); ++i) {
        Point2D curr = points[i];
        sf::CircleShape currPoint(10.0f);
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
    if (t >= segCount) {
        t = static_cast<float>(segCount);
        direction = -1;
    } else if (t <= 0) {
        t = 0.f;
        direction = 1;
    }

    int squareSegIndex = std::min(static_cast<int>(t), segCount - 1);
    float localT = t - squareSegIndex;
    std::vector<Point2D> squareSegment(
        points.begin() + squareSegIndex * 3,
        points.begin() + squareSegIndex * 3 + 4
    );
    square.setPosition(getPoint(squareSegment, localT));

    // Rotation
    Point2D slope = getSlope(squareSegment, localT);
    float angle = std::atan2(slope.y, slope.x);
    square.setRotation(sf::radians(angle));

    window.draw(square);
    

    // ====== ====== ======
    // TODO: (Part 3) Draw control handles from point 1 to 2 and point 3 to 4.
    for (int segIndex = 0; segIndex < segCount; ++segIndex) {
        int start = segIndex * 3;
        sf::VertexArray handles(sf::PrimitiveType::Lines, 4);

        handles[0].position = points[start];
        handles[1].position = points[start + 1];
        handles[2].position = points[start + 2];
        handles[3].position = points[start + 3];

        for (std::size_t i = 0; i < handles.getVertexCount(); ++i) {
            handles[i].color = sf::Color::White;
        }

        window.draw(handles);
    }

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

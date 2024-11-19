#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include <iostream>
#include <filesystem>

struct Line {
    sf::Vector2f start;
    sf::Vector2f end;
};

bool isAdjacent(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float gridSize) {
    float dx = std::abs(pos1.x - pos2.x);
    float dy = std::abs(pos1.y - pos2.y);
    return (dx <= gridSize && dy <= gridSize) && (dx + dy != 0);
}

void removeLine(std::vector<Line>& lines, const sf::Vector2f& pos1, const sf::Vector2f& pos2) {
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        if ((it->start == pos1 && it->end == pos2) || (it->start == pos2 && it->end == pos1)) {
            lines.erase(it);
            break;
        }
    }
}

int main()
{
    // 创建一个窗口
    sf::RenderWindow window(sf::VideoMode(800, 600), "Grid Game");

    // 设置方格的大小
    const float gridSize = 50.0f;
    const float margin = 5.0f; // 三角形与方格边缘的距离
    const float triangleHeight = (std::sqrt(3.0f) / 2.0f) * (gridSize - 2 * margin); // 等边三角形的高度
    const float circleRadius = 10.0f; // 圆点的半径
    const float lineThickness = 5.0f; // 线条的厚度

    // 创建左上角方格内部的红色等边三角形
    sf::ConvexShape redTriangle;
    redTriangle.setPointCount(3);
    redTriangle.setPoint(0, sf::Vector2f(margin, gridSize - margin));
    redTriangle.setPoint(1, sf::Vector2f(gridSize / 2.0f, gridSize - margin - triangleHeight));
    redTriangle.setPoint(2, sf::Vector2f(gridSize - margin, gridSize - margin));
    redTriangle.setFillColor(sf::Color::Red);
    sf::Vector2f redTriangleCenter(gridSize / 2.0f, gridSize / 2.0f);

    bool isTriangleSelected = false;
    bool isCircleSelected = false;
    bool isCircleBlinking = false;
    sf::CircleShape blinkingCircle(circleRadius);
    blinkingCircle.setOrigin(circleRadius, circleRadius);
    blinkingCircle.setFillColor(sf::Color::Red);
    sf::Vector2f blinkingCircleCenter;
    std::vector<Line> lines;
    std::vector<sf::CircleShape> circles;
    sf::CircleShape selectedCircleBorder(circleRadius + 2);
    selectedCircleBorder.setOrigin(circleRadius + 2, circleRadius + 2);
    selectedCircleBorder.setFillColor(sf::Color::Transparent);
    selectedCircleBorder.setOutlineColor(sf::Color::Green);
    selectedCircleBorder.setOutlineThickness(2);
    bool isCircleBorderVisible = false;

    sf::Clock clock;
    float blinkTime = 0.5f; // 闪烁时间间隔

    int triangleCircleCount = 3; // 三角形中初始有3个圆点

    // 主循环
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                    if (!isTriangleSelected && !isCircleSelected)
                    {
                        if (redTriangle.getGlobalBounds().contains(mousePosF) && triangleCircleCount > 0)
                        {
                            isTriangleSelected = true;
                        }
                        else
                        {
                            for (auto it = circles.begin(); it != circles.end(); ++it)
                            {
                                if (it->getGlobalBounds().contains(mousePosF))
                                {
                                    isCircleSelected = true;
                                    selectedCircleBorder.setPosition(it->getPosition());
                                    isCircleBorderVisible = true;
                                    break;
                                }
                            }
                        }
                    }
                    else if (isTriangleSelected && !isCircleBlinking)
                    {
                        // 计算方格的中心
                        sf::Vector2f targetCenter(
                            std::floor(mousePosF.x / gridSize) * gridSize + gridSize / 2.0f,
                            std::floor(mousePosF.y / gridSize) * gridSize + gridSize / 2.0f
                        );

                        // 检查方格是否已有圆点
                        bool hasCircle = false;
                        for (const auto& circle : circles)
                        {
                            if (circle.getPosition() == targetCenter)
                            {
                                hasCircle = true;
                                break;
                            }
                        }

                        if (!hasCircle)
                        {
                            // 生成闪烁的红色圆点
                            blinkingCircle.setPosition(targetCenter);
                            blinkingCircleCenter = targetCenter;
                            isCircleBlinking = true;
                        }
                    }
                    else if (isCircleSelected && !isCircleBlinking)
                    {
                        // 计算方格的中心
                        sf::Vector2f targetCenter(
                            std::floor(mousePosF.x / gridSize) * gridSize + gridSize / 2.0f,
                            std::floor(mousePosF.y / gridSize) * gridSize + gridSize / 2.0f
                        );

                        // 检查方格是否已有圆点
                        bool hasCircle = false;
                        for (const auto& circle : circles)
                        {
                            if (circle.getPosition() == targetCenter)
                            {
                                hasCircle = true;
                                break;
                            }
                        }

                        // 检查目标方格是否在当前选中圆点的周围一圈内
                        if (!isAdjacent(selectedCircleBorder.getPosition(), targetCenter, gridSize))
                        {
                            continue; // 如果不在周围一圈内，忽略点击
                        }

                        if (!hasCircle)
                        {
                            // 生成闪烁的红色圆点
                            blinkingCircle.setPosition(targetCenter);
                            blinkingCircleCenter = targetCenter;
                            isCircleBlinking = true;
                        }
                    }
                    else if (isCircleBlinking)
                    {
                        if (blinkingCircle.getGlobalBounds().contains(mousePosF))
                        {
                            isCircleBlinking = false;

                            if (isTriangleSelected)
                            {
                                lines.push_back({ redTriangleCenter, blinkingCircleCenter });
                                isTriangleSelected = false;
                                triangleCircleCount--; // 减少三角形中的圆点数量
                            }
                            else if (isCircleSelected)
                            {
                                // 检查是否需要删除连接线段
                                bool lineRemoved = false;
                                for (auto it = lines.begin(); it != lines.end(); ++it) {
                                    if ((it->start == selectedCircleBorder.getPosition() && it->end == blinkingCircleCenter) ||
                                        (it->start == blinkingCircleCenter && it->end == selectedCircleBorder.getPosition())) {
                                        lines.erase(it);
                                        lineRemoved = true;
                                        break;
                                    }
                                }

                                if (!lineRemoved) {
                                    lines.push_back({ selectedCircleBorder.getPosition(), blinkingCircleCenter });
                                }

                                isCircleSelected = false;
                                isCircleBorderVisible = false;

                                // 删除原先的圆点
                                for (auto it = circles.begin(); it != circles.end(); ++it)
                                {
                                    if (it->getPosition() == selectedCircleBorder.getPosition())
                                    {
                                        circles.erase(it);
                                        break;
                                    }
                                }
                            }

                            // 保留新圆点
                            sf::CircleShape newCircle(circleRadius);
                            newCircle.setOrigin(circleRadius, circleRadius);
                            newCircle.setPosition(blinkingCircleCenter);
                            newCircle.setFillColor(sf::Color::Red);
                            circles.push_back(newCircle);
                        }
                    }
                }
                else if (event.mouseButton.button == sf::Mouse::Right)
                {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    sf::Vector2f mousePosF(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));

                    if (isCircleBlinking && blinkingCircle.getGlobalBounds().contains(mousePosF))
                    {
                        isCircleBlinking = false;
                    }
                }
            }
        }

        window.clear(sf::Color::White); // 清空窗口并设置背景色为白色

        // 画方格
        for (float y = 0; y < window.getSize().y; y += gridSize)
        {
            for (float x = 0; x < window.getSize().x; x += gridSize)
            {
                sf::RectangleShape rectangle(sf::Vector2f(gridSize, gridSize));
                rectangle.setPosition(x, y);
                rectangle.setOutlineColor(sf::Color::Black);
                rectangle.setOutlineThickness(1);
                rectangle.setFillColor(sf::Color::Transparent);
                window.draw(rectangle);
            }
        }

        // 绘制左上角方格内部的红色等边三角形
        window.draw(redTriangle);

        // 如果三角形被选中，绘制绿色边框
        if (isTriangleSelected)
        {
            sf::ConvexShape border;
            border.setPointCount(3);
            border.setPoint(0, sf::Vector2f(margin - 2, gridSize - margin + 2));
            border.setPoint(1, sf::Vector2f(gridSize / 2.0f, gridSize - margin - triangleHeight - 2));
            border.setPoint(2, sf::Vector2f(gridSize - margin + 2, gridSize - margin + 2));
            border.setFillColor(sf::Color::Transparent);
            border.setOutlineColor(sf::Color::Green);
            border.setOutlineThickness(2);
            window.draw(border);
        }

        // 绘制闪烁的红色圆点
        if (isCircleBlinking)
        {
            if (clock.getElapsedTime().asSeconds() > blinkTime)
            {
                blinkingCircle.setFillColor(blinkingCircle.getFillColor() == sf::Color::Red ? sf::Color::Transparent : sf::Color::Red);
                clock.restart();
            }
            window.draw(blinkingCircle);
        }

        // 绘制保留的圆点
        for (const auto& circle : circles)
        {
            window.draw(circle);
        }

        // 绘制选中圆点的绿色边框
        if (isCircleBorderVisible)
        {
            window.draw(selectedCircleBorder);
        }

        // 绘制连接线
        for (const auto& line : lines)
        {
            sf::Vector2f direction = line.end - line.start;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            sf::RectangleShape thickLine(sf::Vector2f(length, lineThickness));
            thickLine.setPosition(line.start);
            thickLine.setFillColor(sf::Color::Black);
            thickLine.setRotation(std::atan2(direction.y, direction.x) * 180 / 3.14159f);
            window.draw(thickLine);

            // 在线条交接处添加圆角
            sf::CircleShape joint(circleRadius / 2);
            joint.setOrigin(circleRadius / 2, circleRadius / 2);
            joint.setPosition(line.end);
            joint.setFillColor(sf::Color::Black);
            window.draw(joint);
        }

        window.display(); // 显示绘制的内容
    }

    return 0;
}

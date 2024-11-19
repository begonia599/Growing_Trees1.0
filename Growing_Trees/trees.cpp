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
    // ����һ������
    sf::RenderWindow window(sf::VideoMode(800, 600), "Grid Game");

    // ���÷���Ĵ�С
    const float gridSize = 50.0f;
    const float margin = 5.0f; // �������뷽���Ե�ľ���
    const float triangleHeight = (std::sqrt(3.0f) / 2.0f) * (gridSize - 2 * margin); // �ȱ������εĸ߶�
    const float circleRadius = 10.0f; // Բ��İ뾶
    const float lineThickness = 5.0f; // �����ĺ��

    // �������ϽǷ����ڲ��ĺ�ɫ�ȱ�������
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
    float blinkTime = 0.5f; // ��˸ʱ����

    int triangleCircleCount = 3; // �������г�ʼ��3��Բ��

    // ��ѭ��
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
                        // ���㷽�������
                        sf::Vector2f targetCenter(
                            std::floor(mousePosF.x / gridSize) * gridSize + gridSize / 2.0f,
                            std::floor(mousePosF.y / gridSize) * gridSize + gridSize / 2.0f
                        );

                        // ��鷽���Ƿ�����Բ��
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
                            // ������˸�ĺ�ɫԲ��
                            blinkingCircle.setPosition(targetCenter);
                            blinkingCircleCenter = targetCenter;
                            isCircleBlinking = true;
                        }
                    }
                    else if (isCircleSelected && !isCircleBlinking)
                    {
                        // ���㷽�������
                        sf::Vector2f targetCenter(
                            std::floor(mousePosF.x / gridSize) * gridSize + gridSize / 2.0f,
                            std::floor(mousePosF.y / gridSize) * gridSize + gridSize / 2.0f
                        );

                        // ��鷽���Ƿ�����Բ��
                        bool hasCircle = false;
                        for (const auto& circle : circles)
                        {
                            if (circle.getPosition() == targetCenter)
                            {
                                hasCircle = true;
                                break;
                            }
                        }

                        // ���Ŀ�귽���Ƿ��ڵ�ǰѡ��Բ�����ΧһȦ��
                        if (!isAdjacent(selectedCircleBorder.getPosition(), targetCenter, gridSize))
                        {
                            continue; // ���������ΧһȦ�ڣ����Ե��
                        }

                        if (!hasCircle)
                        {
                            // ������˸�ĺ�ɫԲ��
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
                                triangleCircleCount--; // �����������е�Բ������
                            }
                            else if (isCircleSelected)
                            {
                                // ����Ƿ���Ҫɾ�������߶�
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

                                // ɾ��ԭ�ȵ�Բ��
                                for (auto it = circles.begin(); it != circles.end(); ++it)
                                {
                                    if (it->getPosition() == selectedCircleBorder.getPosition())
                                    {
                                        circles.erase(it);
                                        break;
                                    }
                                }
                            }

                            // ������Բ��
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

        window.clear(sf::Color::White); // ��մ��ڲ����ñ���ɫΪ��ɫ

        // ������
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

        // �������ϽǷ����ڲ��ĺ�ɫ�ȱ�������
        window.draw(redTriangle);

        // ��������α�ѡ�У�������ɫ�߿�
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

        // ������˸�ĺ�ɫԲ��
        if (isCircleBlinking)
        {
            if (clock.getElapsedTime().asSeconds() > blinkTime)
            {
                blinkingCircle.setFillColor(blinkingCircle.getFillColor() == sf::Color::Red ? sf::Color::Transparent : sf::Color::Red);
                clock.restart();
            }
            window.draw(blinkingCircle);
        }

        // ���Ʊ�����Բ��
        for (const auto& circle : circles)
        {
            window.draw(circle);
        }

        // ����ѡ��Բ�����ɫ�߿�
        if (isCircleBorderVisible)
        {
            window.draw(selectedCircleBorder);
        }

        // ����������
        for (const auto& line : lines)
        {
            sf::Vector2f direction = line.end - line.start;
            float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
            sf::RectangleShape thickLine(sf::Vector2f(length, lineThickness));
            thickLine.setPosition(line.start);
            thickLine.setFillColor(sf::Color::Black);
            thickLine.setRotation(std::atan2(direction.y, direction.x) * 180 / 3.14159f);
            window.draw(thickLine);

            // ���������Ӵ����Բ��
            sf::CircleShape joint(circleRadius / 2);
            joint.setOrigin(circleRadius / 2, circleRadius / 2);
            joint.setPosition(line.end);
            joint.setFillColor(sf::Color::Black);
            window.draw(joint);
        }

        window.display(); // ��ʾ���Ƶ�����
    }

    return 0;
}

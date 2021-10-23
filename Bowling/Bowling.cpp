#include <iostream>
#include <vector>
#include <array>
#include <numeric>
#include <cassert>
#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <catch2/catch_reporter_tap.hpp>


class Game;

struct Frame
{
    enum class Type { Standard, Spare, Strike };
    
    int points(int rolls_left)
    {
        int rollsToScore = _type == Frame::Type::Standard ? 2 : 3;
        rollsToScore = std::min(rolls_left, rollsToScore);
        return std::accumulate(_rollPointer, _rollPointer + rollsToScore, 0);
    }
    int rolls_in_frame()
    {
        return _type == Frame::Type::Strike ? 1 : 2;
    }

    static Frame Create(Game& game, int* rollPointer);

    Type _type = Type::Standard;
    int* _rollPointer;
};

class Game
{
public:
    static constexpr int frames_count = 10;
    static constexpr int allPinsCount = 10;

    void roll(int points)
    {
        assert(points >= 0 && points <= allPinsCount);
        _points.push_back(points);
    }

    int score()
    {
        int score = 0;
        int* pointer = _points.data();
        std::array<Frame, frames_count> frames;
        for (auto& frame : frames)
        {
            int left = rolls_left(pointer);
            if (left <= 0) break;

            frame = Frame::Create(*this, pointer);
            score += frame.points(left);
            pointer += frame.rolls_in_frame();
        }
        return score;
    }

protected:
    int rolls_left(int* pointer)
    {
        int rolls_done = pointer - _points.data();
        int rolls_left = _points.size() - rolls_done;
        return rolls_left;
    }

    std::vector<int> _points;
};

Frame Frame::Create(Game& game, int* rollPointer)
{
    auto type = Frame::Type::Standard;
    if (*rollPointer == Game::allPinsCount)
        type = Frame::Type::Strike;
    else if (*rollPointer + *(rollPointer+1) == Game::allPinsCount)
        type = Frame::Type::Spare;

    return Frame{ type, rollPointer };
}

int TestScore(const std::vector<int>& rolls)
{
    Game g;
    for (auto& roll : rolls) g.roll(roll);
    return g.score();
}

TEST_CASE("Ensure points are correct", "[bowling]") {
    REQUIRE(TestScore({}) == 0);
    REQUIRE(TestScore({ 1, 1, 1, 1, 1 }) == 5);
    REQUIRE(TestScore({ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }) == 0);
    REQUIRE(TestScore({ 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10 }) == 300);
    REQUIRE(TestScore({ 10, 3, 7, 10, 10, 3, 5 }) == 89);
}

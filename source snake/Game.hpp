#pragma once
#include <algorithm>
#include <deque>
#include <random>
#include <stdexcept>
#include <vector>

namespace snake {
struct Cell {
    int x, y;
    bool operator==(Cell b) const { return x == b.x && y == b.y; }
};
enum class Direction { Up, Down, Left, Right };
enum class State { Playing, Paused, Lost, Won };
class Game {
    int width_, height_;
    std::mt19937 rng_;
    std::deque<Cell> body_;
    Cell food_{};
    Direction direction_ = Direction::Right, pending_ = Direction::Right;
    State state_ = State::Playing;
    int score_ = 0;
    int extraSpeed_ = 0;
    bool turned_ = false;
    void spawnFood() {
        std::vector<Cell> free;
        for (int y = 0; y < height_; ++y)
            for (int x = 0; x < width_; ++x)
                if (std::find(body_.begin(), body_.end(), Cell{x,y}) == body_.end()) free.push_back({x,y});
        if (free.empty()) { state_ = State::Won; return; }
        food_ = free[std::uniform_int_distribution<std::size_t>(0, free.size()-1)(rng_)];
    }
public:
    explicit Game(int width = 28, int height = 24, unsigned seed = std::random_device{}())
        : width_(width), height_(height), rng_(seed) {
        if (width < 4 || height < 2) throw std::invalid_argument("Board too small");
        reset();
    }
    void reset() {
        body_ = {{width_/2,height_/2},{width_/2-1,height_/2},{width_/2-2,height_/2}};
        direction_ = pending_ = Direction::Right;
        score_ = 0; extraSpeed_ = 0; turned_ = false; state_ = State::Playing; spawnFood();
    }
    void turn(Direction d) {
        if (state_ != State::Playing || turned_) return;
        if ((direction_ == Direction::Up && d == Direction::Down) ||
            (direction_ == Direction::Down && d == Direction::Up) ||
            (direction_ == Direction::Left && d == Direction::Right) ||
            (direction_ == Direction::Right && d == Direction::Left) || d == direction_) return;
        pending_ = d; turned_ = true;
    }
    void pause() {
        if (state_ == State::Playing) state_ = State::Paused;
        else if (state_ == State::Paused) state_ = State::Playing;
    }
    void step() {
        if (state_ != State::Playing) return;
        direction_ = pending_; turned_ = false;
        Cell next = body_.front();
        switch (direction_) {
        case Direction::Up: --next.y; break;
        case Direction::Down: ++next.y; break;
        case Direction::Left: --next.x; break;
        case Direction::Right: ++next.x; break;
        }
        const bool eating = next == food_;
        auto end = eating ? body_.end() : std::prev(body_.end());
        if (next.x < 0 || next.y < 0 || next.x >= width_ || next.y >= height_ ||
            std::find(body_.begin(), end, next) != end) { state_ = State::Lost; return; }
        body_.push_front(next);
        if (eating) { score_ += 10; spawnFood(); }
        else body_.pop_back();
    }
    const std::deque<Cell>& body() const { return body_; }
    Cell food() const { return food_; }
    State state() const { return state_; }
    int score() const { return score_; }
    void increaseSpeed() {
        if (state_ == State::Playing && speed() < 18) ++extraSpeed_;
    }
    int speed() const { return std::min(18, 6 + score_/30 + extraSpeed_); }
};
}


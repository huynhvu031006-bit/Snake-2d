#include <SFML/Graphics.hpp>
#include <Windows.h>
#include "Game.hpp"
#include "Scores.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>

bool snake::Scores::replace(const std::filesystem::path& temporary) {
    return MoveFileExW(temporary.c_str(), path_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
}

int runTests() {
    using namespace snake;
    int checks = 0;
    auto require = [&](bool ok, const char* name) {
        ++checks;
        if (!ok) throw std::runtime_error(name);
    };
    Game g(28,24,42);
    const auto start = g.body().front();
    g.turn(Direction::Left); g.step();
    require(g.body().front() == Cell{start.x+1,start.y}, "Cannot reverse");
    g.turn(Direction::Up); g.turn(Direction::Left); g.step();
    require(g.body().front() == Cell{start.x+1,start.y-1}, "Only one turn per step");
    g.pause(); const auto frozen = g.body(); g.step();
    require(g.body() == frozen && g.state() == State::Paused, "Pause freezes logic");
    g.pause();
    while (g.state() == State::Playing) g.step();
    require(g.state() == State::Lost, "Wall collision");
    g.reset(); require(g.score() == 0 && g.body().size() == 3 && g.state() == State::Playing, "Reset");
    g.increaseSpeed(); require(g.speed() == 7, "Manual speed increase");
    g.pause(); g.increaseSpeed(); require(g.speed() == 7, "No speed change while paused");
    g.pause();
    for (int i = 0; i < 30; ++i) g.increaseSpeed();
    require(g.speed() == 18, "Manual speed cap");
    g.reset(); require(g.speed() == 6, "Reset speed");
    bool ate = false, selfHit = false, won = false, tailMove = false;
    // Deterministic, public-API simulation exercises reachable states without test-only setters.
    for (unsigned seed = 0; seed < 400; ++seed) {
        Game sample(4,2,seed);
        sample.turn(Direction::Up); sample.step();
        const auto food = sample.food();
        require(std::find(sample.body().begin(), sample.body().end(), food) == sample.body().end(), "Food on free cell");
        for (int step = 0; step < 100 && sample.state() == State::Playing; ++step) {
            auto before = sample.body();
            int previous = sample.score();
            const auto head = before.front();
            // Follow a Hamiltonian cycle through every cell of the small board.
            Direction d = head.y == 0 ? (head.x == 3 ? Direction::Down : Direction::Right)
                                      : (head.x == 0 ? Direction::Up : Direction::Left);
            sample.turn(d); sample.step();
            if (sample.score() > previous) {
                ate = true;
                require(sample.body().size() == before.size()+1 && sample.score() == previous+10, "Eating grows and scores");
            }
            if (sample.state() == State::Playing && sample.body().front() == before.back()) tailMove = true;
            if (sample.state() == State::Lost && head.x > 0 && head.x < 3) selfHit = true;
            if (sample.state() == State::Won) won = true;
            require(sample.speed() == std::min(18, 6 + sample.score()/30), "Automatic speed every three meals");
        }
    }
    require(ate, "Eating covered"); require(won, "Full board victory covered");
    require(tailMove, "Moving into vacated tail covered");
    // Grow then fold into the neck/body on a larger board.
    for (unsigned seed = 0; seed < 2000 && !selfHit; ++seed) {
        Game sample(8,8,seed);
        for (int i = 0; i < 3 && sample.state() == State::Playing; ++i) sample.step();
        sample.turn(Direction::Down); sample.step(); sample.turn(Direction::Left); sample.step();
        auto head = sample.body().front(); sample.turn(Direction::Up); sample.step();
        if (sample.state() == State::Lost && head.y > 0) selfHit = true;
    }
    require(selfHit, "Self collision covered");
    auto path = std::filesystem::temp_directory_path() / ("snake-tests-" + std::to_string(GetCurrentProcessId()) + ".txt");
    { std::ofstream out(path); out << "bad\n-10\n20\n30junk\n"; }
    Scores scores(path); require(scores.values() == std::vector<int>{20}, "Invalid scores ignored");
    for (int i = 1; i <= 25; ++i) require(scores.add(i*10), "Save score");
    require(scores.add(0), "Zero-point completed round saved");
    Scores loaded(path);
    require(loaded.values().size() == 20 && loaded.values().front() == 0 && loaded.values()[1] == 250 && loaded.values().back() == 70, "Persistent twenty latest rounds, newest first");
    require(loaded.clear() && loaded.values().empty(), "Reset leaderboard");
    require(Scores(path).values().empty(), "Reset persists after reopening");
    require(loaded.add(10) && Scores(path).values() == std::vector<int>{10}, "Save after reset");
    std::filesystem::remove(path);
    std::cout << "PASS: " << checks << " checks\n";
    return 0;
}

int main(int argc, char** argv) {
    try {
        if (argc > 1 && std::string(argv[1]) == "--test") return runTests();
        using namespace snake;
        wchar_t executable[MAX_PATH]{};
        GetModuleFileNameW(nullptr, executable, MAX_PATH);
        auto dataDir = std::filesystem::path(executable).parent_path();
        wchar_t local[MAX_PATH]{};
        if (GetEnvironmentVariableW(L"LOCALAPPDATA", local, MAX_PATH)) dataDir = std::filesystem::path(local) / "Snake2D";
        std::filesystem::create_directories(dataDir);
        Scores scores(dataDir / "scores.txt");
        bool vietnamese = true;
        auto tr = [&](const wchar_t* en, const wchar_t* vi) { return sf::String(vietnamese ? vi : en); };
        Game game;
        sf::RenderWindow window(sf::VideoMode({1040,760}), "Snake 2D", sf::Style::Default);
        // Fixed logical coordinates keep rendering and mouse hit boxes aligned when resized.
        sf::View view(sf::FloatRect({0.f, 0.f}, {1040.f, 760.f}));
        auto resizeView = [&](sf::Vector2u size) {
            if (size.x == 0 || size.y == 0) return;
            constexpr float aspect = 1040.f / 760.f;
            const float actual = static_cast<float>(size.x) / size.y;
            if (actual > aspect) {
                const float width = aspect / actual;
                view.setViewport(sf::FloatRect({(1.f-width)/2.f, 0.f}, {width, 1.f}));
            } else {
                const float height = actual / aspect;
                view.setViewport(sf::FloatRect({0.f, (1.f-height)/2.f}, {1.f, height}));
            }
            window.setView(view);
        };
        resizeView(window.getSize());
        window.setFramerateLimit(120); window.setKeyRepeatEnabled(false);
        sf::Font font;
        wchar_t windowsPath[MAX_PATH]{}; GetWindowsDirectoryW(windowsPath, MAX_PATH);
        if (!font.openFromFile(std::filesystem::path(windowsPath) / "Fonts" / "arial.ttf")) {
            std::cerr << "Cannot load Windows Arial font\n"; return 1;
        }
        auto text = [&](const sf::String& value, float x, float y, unsigned size, sf::Color color = sf::Color(219,231,241)) {
            sf::Text label(font, value, size); label.setPosition({x,y}); label.setFillColor(color); window.draw(label);
        };
        auto box = [&](float x,float y,float w,float h,sf::Color color) {
            sf::RectangleShape shape({w,h}); shape.setPosition({x,y}); shape.setFillColor(color); window.draw(shape);
        };
        sf::Clock clock;
        constexpr double fixedDt = 1.0/120.0;
        double accumulator = 0, movement = 0;
        bool recorded = false, saveOk = true;
        bool inMenu = true;
        auto startGame = [&]() {
            game.reset(); inMenu = false; recorded = false;
            movement = accumulator = 0;
            clock.restart();
        };
        while (window.isOpen()) {
            double elapsed = std::min(0.25, static_cast<double>(clock.restart().asSeconds()));
            while (auto event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) window.close();
                if (auto resized = event->getIf<sf::Event::Resized>()) resizeView(resized->size);
                if (event->is<sf::Event::FocusLost>() && !inMenu && game.state() == State::Playing) { game.pause(); movement = 0; }
                if (auto mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouse->button == sf::Mouse::Button::Left && window.getSize().x > 0 && window.getSize().y > 0) {
                        const auto pos = window.mapPixelToCoords(mouse->position);
                        if (inMenu && pos.x >= 220 && pos.x < 520 && pos.y >= 420 && pos.y < 480) {
                            startGame(); elapsed = 0;
                        }
                        if (pos.x >= 742 && pos.x < 1018 && pos.y >= 526 && pos.y < 558)
                            saveOk = scores.clear();
                        if (!inMenu && pos.y >= 74 && pos.y < 112) {
                            if (pos.x >= 480 && pos.x < 590) game.increaseSpeed();
                            else if (pos.x >= 600 && pos.x < 714) {
                                game.pause(); movement = accumulator = 0; elapsed = 0;
                            }
                        }
                        if (pos.y >= 42 && pos.y < 80) {
                            if (pos.x >= 742 && pos.x < 864) vietnamese = false;
                            else if (pos.x >= 874 && pos.x < 1018) vietnamese = true;
                        }
                    }
                }
                if (auto key = event->getIf<sf::Event::KeyPressed>()) {
                    using K = sf::Keyboard::Key;
                    if (inMenu) {
                        if (key->code == K::Enter) { startGame(); elapsed = 0; }
                        else if (key->code == K::L) vietnamese = !vietnamese;

                        continue;
                    }
                    switch (key->code) {
                    case K::B: game.increaseSpeed(); break;
                    case K::L: vietnamese = !vietnamese; break;

                    case K::W: case K::Up: game.turn(Direction::Up); break;
                    case K::S: case K::Down: game.turn(Direction::Down); break;
                    case K::A: case K::Left: game.turn(Direction::Left); break;
                    case K::D: case K::Right: game.turn(Direction::Right); break;
                    case K::Escape: case K::Space: case K::P: game.pause(); movement = accumulator = 0; elapsed = 0; break;
                    case K::Enter: case K::R:
                        if (game.state() == State::Lost || game.state() == State::Won) { game.reset(); recorded = false; movement = accumulator = 0; }
                        break;
                    default: break;
                    }
                }
            }
            if (!window.isOpen()) break;
            accumulator += elapsed;
            while (accumulator >= fixedDt) {
                if (!inMenu && game.state() == State::Playing) {
                    movement += fixedDt;
                    if (movement >= 1.0/game.speed()) { movement -= 1.0/game.speed(); game.step(); }
                }
                accumulator -= fixedDt;
            }
            if (!inMenu && !recorded && (game.state() == State::Lost || game.state() == State::Won)) { saveOk = scores.add(game.score()); recorded = true; }
            window.clear(sf::Color(12,19,29));
            text(tr(L"LANGUAGE / L", L"NGÔN NGỮ / L"),742,14,16,sf::Color(129,155,173));
            box(742,42,122,38,!vietnamese ? sf::Color(45,120,91) : sf::Color(30,45,59));
            box(874,42,144,38,vietnamese ? sf::Color(45,120,91) : sf::Color(30,45,59));
            text(L"English",766,49,18);
            text(L"Tiếng Việt",894,49,18);
            text("S N A K E   /   2 D",32,22,30,sf::Color(98,235,170));
            text(tr(L"SCORE  ", L"ĐIỂM  ") + sf::String(std::to_string(game.score())),32,76,22);
            text(tr(L"SPEED  ", L"TỐC ĐỘ  ") + sf::String(std::to_string(game.speed())),300,76,22);
            if (!inMenu && (game.state() == State::Playing || game.state() == State::Paused)) {
                box(480,74,110,38,sf::Color(45,90,120));
                text(tr(L"Speed + / B", L"Tốc độ + / B"),487,84,16);
                box(600,74,114,38,sf::Color(45,120,91));
                text(game.state() == State::Paused ? tr(L"Resume / P", L"Tiếp tục / P") : tr(L"Pause / P", L"Tạm dừng / P"),605,84,16);
            }
            box(30,128,676,580,sf::Color(38,59,72));
            box(32,130,672,576,sf::Color(17,29,39));
            for (int y=0;y<24;++y) for (int x=0;x<28;++x)
                if ((x+y)%2 == 0) box(32.f+x*24,130.f+y*24,24,24,sf::Color(20,33,43));
            auto food = game.food();
            if (game.state() != State::Won) box(36.f+food.x*24,134.f+food.y*24,16,16,sf::Color(255,111,107));
            bool head = true;
            for (auto cell : game.body()) {
                box(33.f+cell.x*24,131.f+cell.y*24,22,22,head ? sf::Color(169,255,202) : sf::Color(67,196,139)); head = false;
            }
            text(tr(L"LEADERBOARD", L"BẢNG XẾP HẠNG"),742,135,23,sf::Color(98,235,170));
            text(tr(L"LAST 20 / NEWEST FIRST", L"20 LƯỢT / MỚI NHẤT TRƯỚC"),742,172,15,sf::Color(129,155,173));
            int rank = 0;
            for (int score : scores.values()) { ++rank; text(std::to_string(rank)+".    "+std::to_string(score),750,204.f+(rank-1)*16,14); }
            if (scores.values().empty()) text(tr(L"Your first score awaits.", L"Chưa có điểm. Hãy chơi nhé!"),742,215,18);
            box(742,526,276,32,sf::Color(120,53,60));
            text(tr(L"RESET LEADERBOARD", L"LÀM MỚI XẾP HẠNG"),775,532,17);
            text(tr(L"HOW TO PLAY", L"CÁCH CHƠI"),742,570,19,sf::Color(98,235,170));
            text(tr(L"WASD / Arrows: Move\nEsc / P / Space: Pause\nB: Increase speed\nEnter / R: Play again\nX: Close window", L"WASD / Mũi tên: Di chuyển\nEsc / P / Space: Tạm dừng\nB: Tăng tốc\nEnter / R: Chơi lại\nX: Đóng cửa sổ"),742,602,17);
            text(tr(L"Eat +10   |   Speed increases every 3 meals", L"Ăn mồi +10 điểm   |   Tăng tốc sau mỗi 3 mồi"),32,724,17,sf::Color(129,155,173));
            if (!saveOk) text(tr(L"Score save failed", L"Không lưu được điểm"),742,718,17,sf::Color(255,111,107));
            if (inMenu) {
                box(32,130,672,576,sf::Color(7,13,22,242));
                text(tr(L"WELCOME TO SNAKE", L"CHÀO MỪNG ĐẾN SNAKE"),130,270,30,sf::Color(98,235,170));
                text(tr(L"Choose your language, then start!", L"Chọn ngôn ngữ rồi bắt đầu nhé!"),175,330,22);
                box(220,420,300,60,sf::Color(45,120,91));
                text(tr(L"PLAY NOW", L"CHƠI NGAY"),285,433,28);
                text(tr(L"or press Enter", L"hoặc nhấn Enter"),280,500,20);
            }
            else if (game.state() != State::Playing) {
                box(32,130,672,576,sf::Color(7,13,22,220));
                const bool paused = game.state() == State::Paused;
                text(paused ? tr(L"PAUSED", L"TẠM DỪNG") : game.state() == State::Won ? tr(L"YOU WIN!", L"BẠN THẮNG!") : tr(L"GAME OVER", L"THUA RỒI!"),230,340,38,sf::Color(98,235,170));
                text(paused ? tr(L"Esc / P / Space to continue", L"Esc / P / Space để tiếp tục") : tr(L"Enter / R to play again", L"Enter / R để chơi lại"),235,400,21);
            }
            window.display();
        }
        return 0;
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}







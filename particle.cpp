#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>

const int WIDTH = 40;
const int HEIGHT = 20;
const int NUM_PARTICLES = 10;
const int STEPS = 100;

struct Particle {
    float x, y;
    float vx, vy;
};

std::mutex drawMutex;

void clearScreen() {
    // ANSI escape code to clear the screen
    std::cout << "\033[2J\033[H";
}

void draw(const std::vector<Particle>& particles) {
    std::vector<std::string> screen(HEIGHT, std::string(WIDTH, ' '));

    for (const auto& p : particles) {
        int x = static_cast<int>(p.x);
        int y = static_cast<int>(p.y);

        if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
            screen[y][x] = '*';
        }
    }

    clearScreen();

    for (const auto& line : screen) {
        std::cout << line << "\n";
    }
}

void updateParticles(std::vector<Particle>& particles, int start, int end) {
    for (int step = 0; step < STEPS; ++step) {
        {
            std::lock_guard<std::mutex> lock(drawMutex);
            for (int i = start; i < end; ++i) {
                particles[i].x += particles[i].vx;
                particles[i].y += particles[i].vy;

                // bounce off walls
                if (particles[i].x <= 0 || particles[i].x >= WIDTH - 1)
                    particles[i].vx *= -1;
                if (particles[i].y <= 0 || particles[i].y >= HEIGHT - 1)
                    particles[i].vy *= -1;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    std::vector<Particle> particles(NUM_PARTICLES);

    // Initialize particles with positions and random velocities
    for (auto& p : particles) {
        p.x = rand() % WIDTH;
        p.y = rand() % HEIGHT;
        p.vx = (rand() % 3 - 1); // -1, 0, or 1
        p.vy = (rand() % 3 - 1);
        if (p.vx == 0 && p.vy == 0) p.vx = 1;
    }

    std::vector<std::thread> threads;

    int chunkSize = NUM_PARTICLES / 2;

    for (int t = 0; t < 2; ++t) {
        int start = t * chunkSize;
        int end = (t == 1) ? NUM_PARTICLES : start + chunkSize;
        threads.emplace_back(updateParticles, std::ref(particles), start, end);
    }

    // Drawing loop
    for (int step = 0; step < STEPS; ++step) {
        {
            std::lock_guard<std::mutex> lock(drawMutex);
            draw(particles);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    for (auto& t : threads) {
        t.join();
    }

    return 0;
}

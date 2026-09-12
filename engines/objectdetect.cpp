#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <stdexcept>

// Enum representing the criticality level of an obstacle
enum class Criticality {
    SAFE,
    CAUTION,
    DANGER
};

// Struct holding configurable distance thresholds (in meters)
struct ThresholdConfig {
    double dangerDistance = 2.0;   // Distance < dangerDistance -> Danger
    double cautionDistance = 5.0;  // dangerDistance <= distance <= cautionDistance -> Caution
                                   // Distance > cautionDistance -> Safe

    bool isValid() const {
        return dangerDistance >= 0.0 && cautionDistance > dangerDistance;
    }
};

// ANSI Color Codes for terminal output
namespace Color {
    const std::string RED     = "\033[1;31m";
    const std::string YELLOW  = "\033[1;33m";
    const std::string GREEN   = "\033[1;32m";
    const std::string RESET   = "\033[0m";
    const std::string BOLD    = "\033[1m";
}

class ObstacleDetector {
private:
    ThresholdConfig thresholds;

public:
    explicit ObstacleDetector(ThresholdConfig config = ThresholdConfig{})
        : thresholds(config) {}

    // Update thresholds at runtime
    void setThresholds(double dangerDist, double cautionDist) {
        if (dangerDist < 0.0 || cautionDist <= dangerDist) {
            throw std::invalid_argument("caution distance must be greater than danger distance");
        }
        thresholds.dangerDistance = dangerDist;
        thresholds.cautionDistance = cautionDist;
    }

    const ThresholdConfig& getThresholds() const {
        return thresholds;
    }

    // Classify criticality based on measured distance
    Criticality evaluate(double distanceMeters) const {
        if (distanceMeters < thresholds.dangerDistance) {
            return Criticality::DANGER;
        } else if (distanceMeters <= thresholds.cautionDistance) {
            return Criticality::CAUTION;
        } else {
            return Criticality::SAFE;
        }
    }

    // Format output with colored status label
    void logReading(double distanceMeters) const {
        Criticality level = evaluate(distanceMeters);

        std::string statusTag;
        std::string alertMsg;

        switch (level) {
            case Criticality::DANGER:
                statusTag = Color::RED + "[ DANGER  ]" + Color::RESET;
                alertMsg  = "Obstacle critically close! Stop/divert.";
                break;
            case Criticality::CAUTION:
                statusTag = Color::YELLOW + "[ CAUTION ]" + Color::RESET;
                alertMsg  = "Obstacle approaching. Reduce speed.";
                break;
            case Criticality::SAFE:
                statusTag = Color::GREEN + "[  SAFE   ]" + Color::RESET;
                alertMsg  = "Path is clear.";
                break;
        }

        std::cout << statusTag 
                  << " Distance: " << std::fixed << std::setprecision(2) 
                  << std::setw(5) << distanceMeters << " m"
                  << " | Action: " << alertMsg 
                  << std::endl;
    }
};

// Mode 1: Simulated real-time sensor stream
void runSimulation(const ObstacleDetector& detector, int readingCount = 20) {
    std::cout << Color::BOLD << "\n--- Starting Real-Time Sensor Simulation ---\n" << Color::RESET;
    const ThresholdConfig& thresholds = detector.getThresholds();
    std::cout << "Thresholds: Danger < " << thresholds.dangerDistance
              << "m | Caution " << thresholds.dangerDistance << "m - "
              << thresholds.cautionDistance << "m | Safe > "
              << thresholds.cautionDistance << "m\n\n";

    std::mt19937 rng(std::random_device{}());
    // Simulate obstacle distances from 0.3m up to 8.0m
    std::uniform_real_distribution<double> distGenerator(0.3, 8.0);

    for (int i = 1; i <= readingCount; ++i) {
        double currentDistance = distGenerator(rng);
        std::cout << "#" << std::setw(2) << std::setfill('0') << i << " ";
        detector.logReading(currentDistance);

        // Simulate 500ms sensor sampling interval
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

// Mode 2: Manual user input for interactive testing
void runManualTest(const ObstacleDetector& detector) {
    std::cout << Color::BOLD << "\n--- Manual Distance Input Mode ---\n" << Color::RESET;
    std::cout << "Enter a distance in meters (or type a negative number to exit):\n";

    double distance = 0.0;
    while (true) {
        std::cout << "> ";
        if (!(std::cin >> distance)) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input. Please enter a numerical value.\n";
            continue;
        }

        if (distance < 0.0) {
            std::cout << "Exiting manual test mode.\n";
            break;
        }

        detector.logReading(distance);
    }
}

int main(int argc, char** argv) {
    // Configurable thresholds: < danger = Danger, <= caution = Caution, > caution = Safe
    ThresholdConfig config{2.0, 5.0};
    try {
        if (argc > 1) {
            config.dangerDistance = std::stod(argv[1]);
        }
        if (argc > 2) {
            config.cautionDistance = std::stod(argv[2]);
        }
    } catch (const std::exception&) {
        std::cerr << "Usage: objectdetect [danger_distance_m] [caution_distance_m]\n";
        return 1;
    }
    if (!config.isValid()) {
        std::cerr << "[ERROR] Caution distance must be greater than danger distance.\n";
        return 1;
    }
    ObstacleDetector detector(config);

    std::cout << Color::BOLD << "==========================================\n";
    std::cout << "     OBSTACLE DISTANCE DETECTION SYSTEM   \n";
    std::cout << "==========================================\n" << Color::RESET;
    std::cout << "Select mode:\n";
    std::cout << " 1. Run real-time simulation (sensor loop)\n";
    std::cout << " 2. Manual distance input\n";
    std::cout << "Enter choice (1 or 2): ";

    int choice = 1;
    if (std::cin >> choice) {
        if (choice == 1) {
            runSimulation(detector);
        } else {
            runManualTest(detector);
        }
    }

    return 0;
}
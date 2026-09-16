#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

const double PI = 3.14159265358979323846;
const double EARTH_RADIUS_METERS = 6371000.0;

double toRad(double deg) { return deg * (PI / 180.0); }
double toDeg(double rad) { return rad * (180.0 / PI); }

double getDistanceMeters(double lat1, double lon1, double lat2, double lon2) {
    double dLat = toRad(lat2 - lat1);
    double dLon = toRad(lon2 - lon1);
    double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0) +
               std::cos(toRad(lat1)) * std::cos(toRad(lat2)) *
               std::sin(dLon / 2.0) * std::sin(dLon / 2.0);
    return EARTH_RADIUS_METERS * (2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a)));
}

double getBearing(double lat1, double lon1, double lat2, double lon2) {
    double dLon = toRad(lon2 - lon1);
    double y = std::sin(dLon) * std::cos(toRad(lat2));
    double x = std::cos(toRad(lat1)) * std::sin(toRad(lat2)) -
               std::sin(toRad(lat1)) * std::cos(toRad(lat2)) * std::cos(dLon);
    double bearing = toDeg(std::atan2(y, x));
    return std::fmod(bearing + 360.0, 360.0);
}

std::string getCompassDirection(double bearing) {
    const std::string directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int index = static_cast<int>(std::round(bearing / 45.0)) % 8;
    return directions[index];
}

class AdaptiveNavigator {
private:
    double destLat, destLon;
    double lastLat, lastLon;
    bool hasLastPos = false;

    double baseStepLength;     // Initial/fallback step length (m)
    double currentStepLength;  // Dynamically adapted step length (m)
    const double alpha = 0.25; // Smoothing factor for moving average

public:
    AdaptiveNavigator(double dLat, double dLon, double defaultStep = 0.75)
        : destLat(dLat), destLon(dLon), baseStepLength(defaultStep), currentStepLength(defaultStep) {}

    // Adapt step length using hardware step sensor delta
    void adaptWithPedometer(double deltaDistance, int deltaSteps) {
        if (deltaSteps > 0 && deltaDistance > 1.0) {
            double measured = deltaDistance / deltaSteps;
            // Clamp to human limits (0.4m min shuffle, 1.6m max sprint stride)
            measured = std::max(0.4, std::min(1.6, measured));
            // Exponential moving average filter
            currentStepLength = (alpha * measured) + ((1.0 - alpha) * currentStepLength);
        }
    }

    // Adapt step length using GPS speed (m/s) if pedometer delta is unavailable
    void adaptWithSpeed(double speedMps) {
        if (speedMps > 0.3) {
            // Biomechanical scaling: stride length roughly scales with sqrt(speed)
            // Baseline 1.34 m/s (approx 4.8 km/h normal walk)
            double scaled = baseStepLength * std::sqrt(speedMps / 1.34);
            currentStepLength = std::max(0.4, std::min(1.6, scaled));
        }
    }

    struct NavResult {
        double distanceMeters;
        long long remainingSteps;
        double bearing;
        std::string direction;
        double adaptedStepLength;
    };

    NavResult update(double curLat, double curLon, double speedMps = -1.0, int deltaSteps = -1) {
        if (hasLastPos) {
            double deltaDist = getDistanceMeters(lastLat, lastLon, curLat, curLon);
            if (deltaSteps > 0) {
                adaptWithPedometer(deltaDist, deltaSteps);
            } else if (speedMps >= 0) {
                adaptWithSpeed(speedMps);
            }
        }

        lastLat = curLat;
        lastLon = curLon;
        hasLastPos = true;

        double distToDest = getDistanceMeters(curLat, curLon, destLat, destLon);
        double bearing = getBearing(curLat, curLon, destLat, destLon);
        long long remainingSteps = static_cast<long long>(std::round(distToDest / currentStepLength));

        return {distToDest, remainingSteps, bearing, getCompassDirection(bearing), currentStepLength};
    }
};

int main() {
    // Destination: (37.7749, -122.4194), Default Step: 0.70m
    AdaptiveNavigator nav(37.7749, -122.4194, 0.70);

    // Mock incoming GPS stream: {lat, lon, speed_mps, pedometer_steps_since_last_tick}
    double stream[3][4] = {
        {37.7700, -122.4100, 1.2, 0},   // Starting position
        {37.7710, -122.4120, 1.8, 170}, // User sped up (jogging)
        {37.7730, -122.4160, 2.5, 310}  // Running
    };

    for (int i = 0; i < 3; ++i) {
        auto res = nav.update(stream[i][0], stream[i][1], stream[i][2], static_cast<int>(stream[i][3]));
        std::cout << "[Tick " << i + 1 << "]\n"
                  << "Distance Left:  " << res.distanceMeters << " m\n"
                  << "Adaptive Step:  " << res.adaptedStepLength << " m\n"
                  << "Est. Steps Left:" << res.remainingSteps << "\n"
                  << "Direction:      " << res.direction << " (" << res.bearing << " deg)\n\n";
    }

    return 0;
}
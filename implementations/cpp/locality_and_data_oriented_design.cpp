/**
 * @file locality_and_data_oriented_design.cpp
 * @brief Reference implementations and validation for Data-Oriented Design (DOD) principles.
 *
 * Implements Array of Structures (AoS) vs Structure of Arrays (SoA) particle simulation,
 * struct alignment and padding verification, and cache line density calculations.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace dsa {

// --- 1. Struct Alignment & Padding Demonstrations ---

struct PoorlyAligned {
    char a;      // 1 byte + 7 bytes padding
    int64_t b;   // 8 bytes
    char c;      // 1 byte + 7 bytes padding
    int64_t d;   // 8 bytes
}; // Total: 32 bytes

struct WellAligned {
    int64_t b;   // 8 bytes
    int64_t d;   // 8 bytes
    char a;      // 1 byte
    char c;      // 1 byte + 6 bytes padding
}; // Total: 24 bytes

// --- 2. Array of Structures (AoS) ---

struct ParticleAoS {
    float x, y, z;
    float vx, vy, vz;
    uint32_t id;
    char metadata[36]; // Cold fields polluting cache line during physics updates
};

inline void update_particles_aos(std::vector<ParticleAoS>& particles, float dt) {
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
    }
}

// --- 3. Structure of Arrays (SoA) ---

struct ParticlesSoA {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    std::vector<float> vx;
    std::vector<float> vy;
    std::vector<float> vz;
    std::vector<uint32_t> id;

    void resize(size_t n) {
        x.resize(n);
        y.resize(n);
        z.resize(n);
        vx.resize(n);
        vy.resize(n);
        vz.resize(n);
        id.resize(n);
    }

    size_t size() const {
        return x.size();
    }
};

inline void update_particles_soa(ParticlesSoA& p, float dt) {
    size_t n = p.size();
    for (size_t i = 0; i < n; ++i) {
        p.x[i] += p.vx[i] * dt;
        p.y[i] += p.vy[i] * dt;
        p.z[i] += p.vz[i] * dt;
    }
}

/**
 * @brief Computes cache line efficiency percentage.
 * Useful payload bytes / Total bytes transferred per cache line.
 */
inline double calculate_cache_efficiency(size_t useful_bytes, size_t struct_size) {
    if (struct_size == 0) return 0.0;
    return (static_cast<double>(useful_bytes) / static_cast<double>(struct_size)) * 100.0;
}

} // namespace dsa

int main() {
    std::cout << "Running Locality and Data-Oriented Design verification..." << std::endl;

    // 1. Struct alignment assertions
    assert(sizeof(dsa::PoorlyAligned) == 32);
    assert(sizeof(dsa::WellAligned) == 24);
    assert(sizeof(dsa::WellAligned) < sizeof(dsa::PoorlyAligned));

    // 2. Cache line efficiency calculation
    // In AoS: useful fields are x, y, z, vx, vy, vz = 6 * 4 = 24 bytes
    double aos_efficiency = dsa::calculate_cache_efficiency(24, sizeof(dsa::ParticleAoS));
    assert(sizeof(dsa::ParticleAoS) == 64);
    assert(std::abs(aos_efficiency - (24.0 / 64.0 * 100.0)) < 1e-4);

    // 3. AoS vs SoA Numerical Equivalence Assertion
    const size_t N = 1000;
    const float dt = 0.016f; // ~60 FPS timestep

    std::vector<dsa::ParticleAoS> aos_data(N);
    dsa::ParticlesSoA soa_data;
    soa_data.resize(N);

    for (size_t i = 0; i < N; ++i) {
        float fx = static_cast<float>(i);
        float fy = static_cast<float>(i * 2);
        float fz = static_cast<float>(i * 3);
        float fvx = 1.5f;
        float fvy = -2.5f;
        float fvz = 0.5f;

        // Populate AoS
        aos_data[i].x = fx;
        aos_data[i].y = fy;
        aos_data[i].z = fz;
        aos_data[i].vx = fvx;
        aos_data[i].vy = fvy;
        aos_data[i].vz = fvz;
        aos_data[i].id = static_cast<uint32_t>(i);

        // Populate SoA
        soa_data.x[i] = fx;
        soa_data.y[i] = fy;
        soa_data.z[i] = fz;
        soa_data.vx[i] = fvx;
        soa_data.vy[i] = fvy;
        soa_data.vz[i] = fvz;
        soa_data.id[i] = static_cast<uint32_t>(i);
    }

    // Run updates
    dsa::update_particles_aos(aos_data, dt);
    dsa::update_particles_soa(soa_data, dt);

    // Verify exact equality across all coordinates
    for (size_t i = 0; i < N; ++i) {
        assert(std::abs(aos_data[i].x - soa_data.x[i]) < 1e-6f);
        assert(std::abs(aos_data[i].y - soa_data.y[i]) < 1e-6f);
        assert(std::abs(aos_data[i].z - soa_data.z[i]) < 1e-6f);
    }

    std::cout << "[PASS] Struct padding reduction verified (32B -> 24B, 25% savings)." << std::endl;
    std::cout << "[PASS] AoS vs SoA numerical results are strictly identical." << std::endl;
    std::cout << "All Data-Oriented Design assertions passed successfully!" << std::endl;
    return 0;
}

"""
Locality and Data-Oriented Design (DOD) Simulation and Testing.

Implements Array of Structures (AoS) vs Structure of Arrays (SoA) layout models,
cache density efficiency calculation, and unit tests validating numerical equivalence.
"""

import unittest
from typing import Dict, List


class ParticleAoS:
    def __init__(self, x: float, y: float, z: float, vx: float, vy: float, vz: float, metadata: str = ""):
        self.x = x
        self.y = y
        self.z = z
        self.vx = vx
        self.vy = vy
        self.vz = vz
        self.metadata = metadata


def update_particles_aos(particles: List[ParticleAoS], dt: float) -> None:
    for p in particles:
        p.x += p.vx * dt
        p.y += p.vy * dt
        p.z += p.vz * dt


class ParticlesSoA:
    def __init__(self, count: int):
        self.x = [0.0] * count
        self.y = [0.0] * count
        self.z = [0.0] * count
        self.vx = [0.0] * count
        self.vy = [0.0] * count
        self.vz = [0.0] * count

    def update(self, dt: float) -> None:
        for i in range(len(self.x)):
            self.x[i] += self.vx[i] * dt
            self.y[i] += self.vy[i] * dt
            self.z[i] += self.vz[i] * dt


def calculate_cache_efficiency(useful_bytes: int, total_struct_bytes: int) -> float:
    if total_struct_bytes == 0:
        return 0.0
    return (useful_bytes / total_struct_bytes) * 100.0


class TestLocalityAndDataOrientedDesign(unittest.TestCase):
    def test_aos_soa_equivalence(self):
        n = 500
        dt = 0.016
        aos_list: List[ParticleAoS] = []
        soa = ParticlesSoA(n)

        for i in range(n):
            fx, fy, fz = float(i), float(i * 2), float(i * 3)
            fvx, fvy, fvz = 1.25, -2.5, 0.75
            aos_list.append(ParticleAoS(fx, fy, fz, fvx, fvy, fvz, metadata="extra_cold_field"))
            soa.x[i] = fx
            soa.y[i] = fy
            soa.z[i] = fz
            soa.vx[i] = fvx
            soa.vy[i] = fvy
            soa.vz[i] = fvz

        update_particles_aos(aos_list, dt)
        soa.update(dt)

        for i in range(n):
            self.assertAlmostEqual(aos_list[i].x, soa.x[i], places=5)
            self.assertAlmostEqual(aos_list[i].y, soa.y[i], places=5)
            self.assertAlmostEqual(aos_list[i].z, soa.z[i], places=5)

    def test_cache_efficiency_calculation(self):
        # 24 useful bytes out of 64 byte cache line
        eff = calculate_cache_efficiency(24, 64)
        self.assertAlmostEqual(eff, 37.5)

        # 100% efficiency when whole line is payload
        eff_100 = calculate_cache_efficiency(64, 64)
        self.assertAlmostEqual(eff_100, 100.0)


if __name__ == "__main__":
    unittest.main()

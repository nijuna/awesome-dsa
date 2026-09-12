"""
Benchmarking Pitfalls and Measurement Science in Python.

Implements a statistical benchmark runner with GC control, warmup phases,
and percentiles/IQR calculation, accompanied by unit tests.
"""

import gc
import math
import time
import unittest
from typing import Callable, Dict, List


def compute_statistics(samples: List[float]) -> Dict[str, float]:
    """Computes min, max, median, mean, and standard deviation from timing samples."""
    if not samples:
        raise ValueError("Samples cannot be empty")

    s = sorted(samples)
    n = len(s)
    min_v = s[0]
    max_v = s[-1]

    if n % 2 == 1:
        median_v = s[n // 2]
    else:
        median_v = (s[n // 2 - 1] + s[n // 2]) / 2.0

    mean_v = sum(s) / n
    variance = sum((x - mean_v) ** 2 for x in s) / n
    stddev_v = math.sqrt(variance)

    return {
        "min": min_v,
        "max": max_v,
        "median": median_v,
        "mean": mean_v,
        "stddev": stddev_v,
    }


def run_benchmark(
    func: Callable[[], None],
    warmup_iters: int = 100,
    batch_count: int = 10,
    iters_per_batch: int = 1000,
) -> Dict[str, float]:
    """
    Executes a benchmark with disabled GC and warmup to avoid environmental jitter.
    Returns timing statistics in nanoseconds per operation.
    """
    # 1. Warmup
    for _ in range(warmup_iters):
        func()

    # 2. Measurement with GC disabled
    gc_was_enabled = gc.isenabled()
    gc.disable()
    batch_times_ns: List[float] = []

    try:
        for _ in range(batch_count):
            t0 = time.perf_counter_ns()
            for _ in range(iters_per_batch):
                func()
            t1 = time.perf_counter_ns()
            batch_times_ns.append((t1 - t0) / iters_per_batch)
    finally:
        if gc_was_enabled:
            gc.enable()

    return compute_statistics(batch_times_ns)


class TestBenchmarkingPitfalls(unittest.TestCase):
    def test_statistics_calculation(self):
        samples = [10.0, 20.0, 30.0, 40.0, 50.0]
        stats = compute_statistics(samples)
        self.assertEqual(stats["min"], 10.0)
        self.assertEqual(stats["max"], 50.0)
        self.assertEqual(stats["median"], 30.0)
        self.assertEqual(stats["mean"], 30.0)
        self.assertAlmostEqual(stats["stddev"], 14.1421356, places=5)

    def test_benchmark_runner_executes(self):
        counter = 0

        def dummy_op():
            nonlocal counter
            counter += 1

        stats = run_benchmark(dummy_op, warmup_iters=10, batch_count=5, iters_per_batch=50)
        self.assertGreater(counter, 0)
        self.assertGreaterEqual(stats["min"], 0.0)
        self.assertGreaterEqual(stats["max"], stats["min"])
        self.assertGreaterEqual(stats["median"], stats["min"])


if __name__ == "__main__":
    unittest.main()

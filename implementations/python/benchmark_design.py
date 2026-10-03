"""
Benchmark Design Reference Implementation
=========================================
Demonstrates rigorous scientific benchmarking principles:
1. Garbage Collection (GC) control during measurement
2. Warm-up phase to prime caches and JIT paths
3. Multi-sample statistical aggregation (mean, stddev, percentiles: p50, p90, p95, p99)
4. Outlier analysis and workload sensitivity
"""

from dataclasses import dataclass
from typing import Callable, List, Any
import bisect
import gc
import math
import time
import unittest


@dataclass
class BenchmarkStats:
    samples: int
    mean_ns: float
    median_ns: float
    stddev_ns: float
    min_ns: float
    max_ns: float
    p90_ns: float
    p95_ns: float
    p99_ns: float


class BenchmarkHarness:
    """Rigorous statistical benchmarking harness."""

    @staticmethod
    def run(func: Callable[[], Any], iterations: int = 1000, warmup: int = 100) -> BenchmarkStats:
        # 1. Warm-up Phase: prime CPU caches and JIT paths
        for _ in range(warmup):
            func()

        # 2. Measurement Phase with GC paused to prevent GC pauses from polluting latency percentiles
        timings_ns: List[int] = []
        gc_was_enabled = gc.isenabled()
        gc.collect()
        gc.disable()

        try:
            for _ in range(iterations):
                t_start = time.perf_counter_ns()
                func()
                t_end = time.perf_counter_ns()
                timings_ns.append(t_end - t_start)
        finally:
            if gc_was_enabled:
                gc.enable()

        # 3. Statistical Analysis
        timings_ns.sort()
        n = len(timings_ns)
        mean_val = sum(timings_ns) / n
        var = sum((t - mean_val) ** 2 for t in timings_ns) / n
        stddev_val = math.sqrt(var)

        def get_percentile(p: float) -> float:
            rank = (p / 100.0) * (n - 1)
            low = int(math.floor(rank))
            high = int(math.ceil(rank))
            weight = rank - low
            return timings_ns[low] * (1.0 - weight) + timings_ns[high] * weight

        return BenchmarkStats(
            samples=n,
            mean_ns=mean_val,
            median_ns=get_percentile(50.0),
            stddev_ns=stddev_val,
            min_ns=float(timings_ns[0]),
            max_ns=float(timings_ns[-1]),
            p90_ns=get_percentile(90.0),
            p95_ns=get_percentile(95.0),
            p99_ns=get_percentile(99.0),
        )


class TestBenchmarkDesign(unittest.TestCase):
    def test_harness_statistics(self):
        """Verify statistical sanity of harness calculations."""
        counter = [0]
        def work():
            counter[0] += 1

        stats = BenchmarkHarness.run(work, iterations=500, warmup=50)
        self.assertEqual(stats.samples, 500)
        self.assertGreaterEqual(stats.median_ns, 0.0)
        self.assertGreaterEqual(stats.p99_ns, stats.median_ns)
        self.assertGreaterEqual(stats.max_ns, stats.min_ns)

    def test_small_n_linear_vs_set(self):
        """Compare small N list scan vs set lookup."""
        small_n = 32
        lst = [i * 2 for i in range(small_n)]
        s = set(lst)
        target = 48

        def list_scan():
            return target in lst

        def set_lookup():
            return target in s

        stats_list = BenchmarkHarness.run(list_scan, iterations=1000, warmup=100)
        stats_set = BenchmarkHarness.run(set_lookup, iterations=1000, warmup=100)

        self.assertGreater(stats_list.samples, 0)
        self.assertGreater(stats_set.samples, 0)

    def test_large_n_bisect_vs_hash(self):
        """Compare large N binary search vs hash table."""
        large_n = 5000
        sorted_lst = [i * 3 for i in range(large_n)]
        s = set(sorted_lst)
        target = 7500

        def bin_search():
            idx = bisect.bisect_left(sorted_lst, target)
            return idx < len(sorted_lst) and sorted_lst[idx] == target

        def hash_lookup():
            return target in s

        stats_bin = BenchmarkHarness.run(bin_search, iterations=1000, warmup=100)
        stats_hash = BenchmarkHarness.run(hash_lookup, iterations=1000, warmup=100)

        self.assertGreater(stats_bin.samples, 0)
        self.assertGreater(stats_hash.samples, 0)


if __name__ == "__main__":
    unittest.main()

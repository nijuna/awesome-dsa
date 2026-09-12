"""
Fuzzing and Property-Based Testing in Python.

Implements property verification (Idempotence, Conservation, Monotonicity)
and an automated counterexample shrinker accompanied by unit tests.
"""

import random
import unittest
from typing import Callable, List


def is_sorted(arr: List[int]) -> bool:
    return all(arr[i - 1] <= arr[i] for i in range(1, len(arr)))


def is_permutation(a: List[int], b: List[int]) -> bool:
    return sorted(a) == sorted(b)


def verify_sort_idempotence(arr: List[int]) -> bool:
    first_pass = sorted(arr)
    second_pass = sorted(first_pass)
    return first_pass == second_pass


def shrink_counterexample(
    failing_input: List[int],
    property_holds: Callable[[List[int]], bool],
) -> List[int]:
    """Prunes and shrinks failing input list down to the minimal counterexample."""
    assert not property_holds(failing_input)
    current = list(failing_input)

    changed = True
    while changed:
        changed = False

        # 1. Prune single elements
        for i in range(len(current)):
            candidate = current[:i] + current[i + 1 :]
            if candidate and not property_holds(candidate):
                current = candidate
                changed = True
                break
        if changed:
            continue

        # 2. Shrink values toward zero
        for i in range(len(current)):
            if current[i] != 0:
                candidate = list(current)
                candidate[i] = candidate[i] // 2
                if not property_holds(candidate):
                    current = candidate
                    changed = True
                    break

    return current


class TestFuzzingAndPropertyTesting(unittest.TestCase):
    def test_randomized_sorting_properties(self):
        rng = random.Random(1337)
        for _ in range(500):
            length = rng.randint(0, 50)
            arr = [rng.randint(-100, 100) for _ in range(length)]

            self.assertTrue(verify_sort_idempotence(arr))
            sorted_arr = sorted(arr)
            self.assertTrue(is_sorted(sorted_arr))
            self.assertTrue(is_permutation(arr, sorted_arr))

    def test_automated_shrinker(self):
        # Property fails if list contains both 42 and 99
        def prop(arr: List[int]) -> bool:
            return not (42 in arr and 99 in arr)

        large_case = [1, 5, 42, 10, 20, 30, 99, 4, 8, 12, 15]
        self.assertFalse(prop(large_case))

        minimal = shrink_counterexample(large_case, prop)
        self.assertEqual(len(minimal), 2)
        self.assertEqual(set(minimal), {42, 99})


if __name__ == "__main__":
    unittest.main()

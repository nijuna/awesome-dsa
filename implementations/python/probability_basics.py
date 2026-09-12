"""
Discrete Probability Theory and Statistical Expectation Engine.

Provides discrete PMF evaluators, Bayes' theorem posterior calculator,
binomial and geometric distribution models, and linearity of expectation tests.
"""

import math
import random
import unittest
from typing import List


class DiscreteDistribution:
    """Represents a discrete random variable with values and probabilities."""

    def __init__(self, values: List[float], probabilities: List[float]):
        if len(values) != len(probabilities) or not values:
            raise ValueError("Values and probabilities must have identical non-zero length")
        if any(p < 0 for p in probabilities):
            raise ValueError("Probabilities must be non-negative")
        if not math.isclose(sum(probabilities), 1.0, rel_tol=1e-7):
            raise ValueError(f"Probabilities must sum to 1.0, got {sum(probabilities)}")

        self.values = values
        self.probabilities = probabilities

    def expectation(self) -> float:
        """Computes E[X] = sum x * P(X = x)."""
        return sum(x * p for x, p in zip(self.values, self.probabilities))

    def variance(self) -> float:
        """Computes Var(X) = E[X^2] - (E[X])^2."""
        mu = self.expectation()
        return sum(((x - mu) ** 2) * p for x, p in zip(self.values, self.probabilities))


def bayes_posterior(
    priors: List[float], likelihoods: List[float], target_index: int
) -> float:
    """Computes posterior probability P(B_target | A) via Bayes' Theorem."""
    if len(priors) != len(likelihoods) or not (0 <= target_index < len(priors)):
        raise ValueError("Invalid dimensions for Bayes calculation")

    total_prob_A = sum(p * l for p, l in zip(priors, likelihoods))
    if total_prob_A <= 0.0:
        raise ValueError("Total probability of conditioning event must be > 0")

    return (priors[target_index] * likelihoods[target_index]) / total_prob_A


def binomial_pmf(n: int, k: int, p: float) -> float:
    """Evaluates Binomial PMF: P(X = k) = C(n, k) * p^k * (1 - p)^(n - k)."""
    if k < 0 or k > n:
        return 0.0
    if not (0.0 <= p <= 1.0):
        raise ValueError("p must be in [0, 1]")
    return math.comb(n, k) * (p**k) * ((1.0 - p) ** (n - k))


def geometric_pmf(k: int, p: float) -> float:
    """Evaluates Geometric PMF: P(X = k) = (1 - p)^(k - 1) * p for k >= 1."""
    if k <= 0:
        return 0.0
    if not (0.0 < p <= 1.0):
        raise ValueError("p must be in (0, 1]")
    return ((1.0 - p) ** (k - 1)) * p


class TestProbabilityBasics(unittest.TestCase):
    def test_fair_die(self):
        vals = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0]
        probs = [1.0 / 6.0] * 6
        die = DiscreteDistribution(vals, probs)
        self.assertAlmostEqual(die.expectation(), 3.5, places=9)
        self.assertAlmostEqual(die.variance(), 35.0 / 12.0, places=9)

    def test_bayes_posterior(self):
        # Disease test: Prior = 0.001, Sensitivity = 0.99, False Positive Rate = 0.01
        priors = [0.001, 0.999]
        likelihoods = [0.99, 0.01]
        posterior = bayes_posterior(priors, likelihoods, 0)
        expected = 0.00099 / (0.00099 + 0.00999)
        self.assertAlmostEqual(posterior, expected, places=6)

    def test_binomial_pmf(self):
        self.assertAlmostEqual(binomial_pmf(4, 2, 0.5), 0.375, places=7)
        self.assertAlmostEqual(binomial_pmf(10, 0, 0.1), 0.9**10, places=7)

    def test_geometric_pmf(self):
        self.assertAlmostEqual(geometric_pmf(1, 0.2), 0.2, places=7)
        self.assertAlmostEqual(geometric_pmf(2, 0.2), 0.16, places=7)

    def test_linearity_of_expectation(self):
        random.seed(42)
        TRIALS = 100_000
        sum_d1 = 0
        sum_d2 = 0
        sum_total = 0
        for _ in range(TRIALS):
            d1 = random.randint(1, 6)
            d2 = random.randint(1, 6) if d1 % 2 == 0 else d1
            sum_d1 += d1
            sum_d2 += d2
            sum_total += (d1 + d2)

        mean_d1 = sum_d1 / TRIALS
        mean_d2 = sum_d2 / TRIALS
        mean_total = sum_total / TRIALS

        self.assertAlmostEqual(mean_total, mean_d1 + mean_d2, places=7)
        self.assertAlmostEqual(mean_total, 6.75, delta=0.05)


if __name__ == "__main__":
    unittest.main()

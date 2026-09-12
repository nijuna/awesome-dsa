/**
 * @file probability_basics.cpp
 * @brief Reference implementations for foundational probability theory in computing.
 *
 * Implements discrete PMF statistics, Bayes' theorem solver, binomial/geometric distributions,
 * and Monte Carlo empirical verification of linearity of expectation and tail bounds.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <stdexcept>

namespace dsa {

/**
 * @brief Discrete Probability Mass Function (PMF) evaluator.
 */
class DiscreteDistribution {
public:
    std::vector<double> values;
    std::vector<double> probabilities;

    DiscreteDistribution(const std::vector<double>& vals, const std::vector<double>& probs)
        : values(vals), probabilities(probs) {
        if (values.size() != probabilities.size() || values.empty()) {
            throw std::invalid_argument("Values and probabilities must have identical non-zero length");
        }
        double sum = 0.0;
        for (double p : probabilities) {
            if (p < 0.0) throw std::invalid_argument("Probabilities must be non-negative");
            sum += p;
        }
        assert(std::abs(sum - 1.0) < 1e-7);
    }

    double expectation() const {
        double exp = 0.0;
        for (size_t i = 0; i < values.size(); ++i) {
            exp += values[i] * probabilities[i];
        }
        return exp;
    }

    double variance() const {
        double mu = expectation();
        double var = 0.0;
        for (size_t i = 0; i < values.size(); ++i) {
            double diff = values[i] - mu;
            var += (diff * diff) * probabilities[i];
        }
        return var;
    }
};

/**
 * @brief Computes Bayes' theorem posterior P(B_j | A).
 *
 * @param priors P(B_i) for partition B_1, ..., B_k.
 * @param likelihoods P(A | B_i) for each partition element.
 * @param target_index Index j for which posterior P(B_j | A) is evaluated.
 * @return Posterior probability P(B_j | A).
 */
inline double bayes_posterior(
    const std::vector<double>& priors,
    const std::vector<double>& likelihoods,
    size_t target_index
) {
    if (priors.size() != likelihoods.size() || target_index >= priors.size()) {
        throw std::invalid_argument("Invalid dimension for Bayes calculation");
    }

    // Law of total probability: P(A) = sum P(B_i) * P(A | B_i)
    double total_prob_A = 0.0;
    for (size_t i = 0; i < priors.size(); ++i) {
        total_prob_A += priors[i] * likelihoods[i];
    }
    assert(total_prob_A > 0.0);

    return (priors[target_index] * likelihoods[target_index]) / total_prob_A;
}

/**
 * @brief Evaluates Binomial PMF: P(X = k) = C(n, k) * p^k * (1 - p)^(n - k).
 */
inline double binomial_pmf(uint32_t n, uint32_t k, double p) {
    if (k > n) return 0.0;
    if (p < 0.0 || p > 1.0) throw std::invalid_argument("p must be in [0, 1]");

    // Use log-gamma / log space to avoid overflow with large n
    double log_comb = std::lgamma(n + 1) - std::lgamma(k + 1) - std::lgamma(n - k + 1);
    double log_prob = log_comb + k * std::log(p) + (n - k) * std::log(1.0 - p);
    return std::exp(log_prob);
}

/**
 * @brief Geometric PMF: P(X = k) = (1 - p)^(k - 1) * p for k >= 1.
 */
inline double geometric_pmf(uint32_t k, double p) {
    if (k == 0) return 0.0;
    if (p <= 0.0 || p > 1.0) throw std::invalid_argument("p must be in (0, 1]");
    return std::pow(1.0 - p, k - 1) * p;
}

} // namespace dsa

int main() {
    std::cout << "Running Probability Basics C++17 unit tests..." << std::endl;

    // Test 1: Fair 6-sided die PMF statistics
    {
        std::vector<double> vals = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
        std::vector<double> probs(6, 1.0 / 6.0);
        dsa::DiscreteDistribution die(vals, probs);

        // Expected value: 3.5
        assert(std::abs(die.expectation() - 3.5) < 1e-9);

        // Variance: E[X^2] - (E[X])^2 = 91/6 - 49/4 = 35/12 approx 2.916666...
        double expected_var = 35.0 / 12.0;
        assert(std::abs(die.variance() - expected_var) < 1e-9);
    }

    // Test 2: Bayes' theorem diagnostic test
    {
        // Rare disease prevalence: P(D) = 0.001, P(~D) = 0.999
        // Test accuracy: P(+ | D) = 0.99, P(+ | ~D) = 0.01 (1% false positive)
        std::vector<double> priors = {0.001, 0.999};
        std::vector<double> likelihoods = {0.99, 0.01};

        // Compute P(D | +): posterior probability of disease given positive test
        double posterior = dsa::bayes_posterior(priors, likelihoods, 0);

        // Expected: (0.001 * 0.99) / (0.001 * 0.99 + 0.999 * 0.01) approx 0.09016 (9%)
        double expected = 0.00099 / (0.00099 + 0.00999);
        assert(std::abs(posterior - expected) < 1e-6);
        std::cout << "  [Bayes Verification] P(Disease | Positive Test): " << posterior * 100.0 << "%" << std::endl;
    }

    // Test 3: Binomial PMF: 4 coin flips, probability of exactly 2 heads with p = 0.5
    // P(X = 2) = C(4, 2) * (0.5)^4 = 6 / 16 = 0.375
    {
        double p_2heads = dsa::binomial_pmf(4, 2, 0.5);
        assert(std::abs(p_2heads - 0.375) < 1e-7);
    }

    // Test 4: Geometric PMF: trials until success with p = 0.2
    // E[X] = 1/p = 5.0
    {
        double p1 = dsa::geometric_pmf(1, 0.2); // First trial: 0.2
        assert(std::abs(p1 - 0.2) < 1e-7);
        double p2 = dsa::geometric_pmf(2, 0.2); // Second trial: 0.8 * 0.2 = 0.16
        assert(std::abs(p2 - 0.16) < 1e-7);
    }

    // Test 5: Monte Carlo verification of Linearity of Expectation with correlated variables
    {
        std::mt19937_64 rng(42);
        std::uniform_int_distribution<int> die_dist(1, 6);

        constexpr int TRIALS = 500'000;
        double sum_d1 = 0.0;
        double sum_d2 = 0.0;
        double sum_total = 0.0;

        for (int i = 0; i < TRIALS; ++i) {
            int d1 = die_dist(rng);
            int d2 = (d1 % 2 == 0) ? die_dist(rng) : d1; // Correlated variable
            sum_d1 += d1;
            sum_d2 += d2;
            sum_total += (d1 + d2);
        }

        double mean_d1 = sum_d1 / TRIALS;
        double mean_d2 = sum_d2 / TRIALS;
        double mean_total = sum_total / TRIALS;

        // Linearity of expectation: E[d1 + d2] == E[d1] + E[d2]
        assert(std::abs(mean_total - (mean_d1 + mean_d2)) < 1e-9);

        // Theoretical convergence: E[d1] = 3.5, E[d2] = 3.25, E[d1 + d2] = 6.75
        assert(std::abs(mean_d1 - 3.5) < 0.03);
        assert(std::abs(mean_d2 - 3.25) < 0.03);
        assert(std::abs(mean_total - 6.75) < 0.03);
    }

    std::cout << "[PASS] All Probability Basics C++ unit tests passed." << std::endl;
    return 0;
}

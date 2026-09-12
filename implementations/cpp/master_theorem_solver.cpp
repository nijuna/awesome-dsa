#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace dsa {

/**
 * @brief Master Theorem evaluation result.
 */
struct MasterResult {
    int case_num;
    double log_b_a;
    std::string theta_complexity;
};

/**
 * @brief Evaluates recurrences of the form T(n) = a * T(n / b) + Theta(n^d * log^k(n)).
 */
class MasterTheoremSolver {
public:
    static MasterResult solve(double a, double b, double d, int k = 0) {
        if (a < 1.0 || b <= 1.0) {
            throw std::invalid_argument("Master theorem requires a >= 1 and b > 1");
        }

        double log_b_a = std::log(a) / std::log(b);
        const double EPS = 1e-9;

        MasterResult res;
        res.log_b_a = log_b_a;

        if (log_b_a > d + EPS) {
            // Case 1: Leaves dominate
            res.case_num = 1;
            std::ostringstream ss;
            ss << "Theta(n^" << std::fixed << std::setprecision(3) << log_b_a << ")";
            res.theta_complexity = ss.str();
        } else if (std::abs(log_b_a - d) <= EPS) {
            // Case 2: Work evenly distributed across levels
            res.case_num = 2;
            std::ostringstream ss;
            ss << "Theta(n^" << d;
            if (k + 1 == 1) {
                ss << " * log n)";
            } else if (k + 1 > 1) {
                ss << " * log^" << (k + 1) << " n)";
            } else {
                ss << ")";
            }
            res.theta_complexity = ss.str();
        } else {
            // Case 3: Root dominates
            res.case_num = 3;
            std::ostringstream ss;
            ss << "Theta(n^" << d;
            if (k == 1) {
                ss << " * log n)";
            } else if (k > 1) {
                ss << " * log^" << k << " n)";
            } else {
                ss << ")";
            }
            res.theta_complexity = ss.str();
        }

        return res;
    }

    /**
     * @brief Solves characteristic exponent p for Akra-Bazzi uneven splits:
     * sum_{i=1}^m a_i * (b_i)^p = 1.
     */
    static double solve_akra_bazzi_p(const std::vector<std::pair<double, double>>& terms) {
        // terms: (a_i, b_i) where b_i in (0, 1)
        double low = -10.0, high = 10.0;
        for (int iter = 0; iter < 100; ++iter) {
            double mid = low + (high - low) / 2.0;
            double sum = 0.0;
            for (const auto& t : terms) {
                sum += t.first * std::pow(t.second, mid);
            }
            if (sum > 1.0) {
                low = mid;
            } else {
                high = mid;
            }
        }
        return (low + high) / 2.0;
    }
};

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Master Theorem Solver C++17 Verification..." << std::endl;

    // 1. Merge Sort: T(n) = 2T(n/2) + O(n) -> a=2, b=2, d=1, k=0 -> Case 2
    {
        auto res = MasterTheoremSolver::solve(2, 2, 1, 0);
        assert(res.case_num == 2);
        assert(std::abs(res.log_b_a - 1.0) < 1e-6);
    }

    // 2. Binary Search: T(n) = T(n/2) + O(1) -> a=1, b=2, d=0, k=0 -> Case 2
    {
        auto res = MasterTheoremSolver::solve(1, 2, 0, 0);
        assert(res.case_num == 2);
        assert(std::abs(res.log_b_a - 0.0) < 1e-6);
    }

    // 3. Strassen Matrix Mult: T(n) = 7T(n/2) + O(n^2) -> a=7, b=2, d=2 -> Case 1 (log2(7) ~ 2.807)
    {
        auto res = MasterTheoremSolver::solve(7, 2, 2, 0);
        assert(res.case_num == 1);
        assert(std::abs(res.log_b_a - 2.80735) < 1e-3);
    }

    // 4. Akra-Bazzi uneven split: T(n) = T(n/3) + T(2n/3) + O(n)
    // Terms: (1, 1/3) and (1, 2/3). Characteristic eq: (1/3)^p + (2/3)^p = 1 => p = 1.0
    {
        std::vector<std::pair<double, double>> terms = {{1.0, 1.0 / 3.0}, {1.0, 2.0 / 3.0}};
        double p = MasterTheoremSolver::solve_akra_bazzi_p(terms);
        assert(std::abs(p - 1.0) < 1e-4);
    }

    std::cout << "[PASSED] Master Theorem Solver C++17 All Tests Passed!" << std::endl;
    return 0;
}

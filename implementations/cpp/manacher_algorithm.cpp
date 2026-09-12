/**
 * @file manacher_algorithm.cpp
 * @brief Reference implementation of Manacher's Algorithm for linear-time palindrome analysis.
 *
 * Computes longest palindromic substring, all palindrome radii, and total palindromic
 * substring count in strictly O(n) time and O(n) auxiliary space.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cassert>
#include <numeric>

namespace dsa {

/**
 * @brief Computes Manacher's palindrome radii array P for transformed string.
 * Transformed string inserts '#' between all characters bookended by '^' and '$'.
 */
inline std::vector<int> manacher_radii(const std::string& s) {
    if (s.empty()) return {};

    // Transform string: "aba" -> "^#a#b#a#$"
    std::string t = "^";
    for (char c : s) {
        t += '#';
        t += c;
    }
    t += "#$";

    int n = static_cast<int>(t.size());
    std::vector<int> p(n, 0);
    int c = 0, r = 0;

    for (int i = 1; i < n - 1; ++i) {
        int i_mirror = 2 * c - i;
        if (i < r) {
            p[i] = std::min(r - i, p[i_mirror]);
        }

        // Expand palindrome centered at i using bookend sentinels
        while (t[i + p[i] + 1] == t[i - p[i] - 1]) {
            p[i]++;
        }

        // Update center and right boundary if current palindrome extends further
        if (i + p[i] > r) {
            c = i;
            r = i + p[i];
        }
    }

    return p;
}

/**
 * @brief Finds the longest palindromic substring in strictly O(n) time.
 */
inline std::string longest_palindromic_substring(const std::string& s) {
    if (s.empty()) return "";

    std::vector<int> p = manacher_radii(s);
    int max_len = 0;
    int center_idx = 0;

    for (size_t i = 1; i < p.size() - 1; ++i) {
        if (p[i] > max_len) {
            max_len = p[i];
            center_idx = static_cast<int>(i);
        }
    }

    // In transformed string, start index in original string is (center_idx - max_len) / 2
    int start_idx = (center_idx - max_len) / 2;
    return s.substr(start_idx, max_len);
}

/**
 * @brief Counts the total number of palindromic substrings in s in O(n) time.
 * In transformed string, a palindrome of radius P[i] contributes (P[i] + 1) / 2 palindromes.
 */
inline uint64_t count_palindromic_substrings(const std::string& s) {
    if (s.empty()) return 0;
    std::vector<int> p = manacher_radii(s);
    uint64_t total = 0;
    for (size_t i = 1; i < p.size() - 1; ++i) {
        total += (p[i] + 1) / 2;
    }
    return total;
}

} // namespace dsa

int main() {
    std::cout << "Running Manacher's Algorithm C++17 unit tests..." << std::endl;

    // Test 1: Odd-length palindrome
    {
        std::string s = "babad";
        std::string lps = dsa::longest_palindromic_substring(s);
        assert(lps == "bab" || lps == "aba");
    }

    // Test 2: Even-length palindrome
    {
        std::string s = "cbbd";
        std::string lps = dsa::longest_palindromic_substring(s);
        assert(lps == "bb");
    }

    // Test 3: Entire string is a palindrome
    {
        std::string s = "racecar";
        assert(dsa::longest_palindromic_substring(s) == "racecar");
        // Count palindromes in "racecar":
        // 7 single letters: r, a, c, e, c, a, r
        // 1 3-letter: cec
        // 1 5-letter: aceca
        // 1 7-letter: racecar
        // Total = 10
        assert(dsa::count_palindromic_substrings(s) == 10);
    }

    // Test 4: All identical characters
    {
        std::string s = "aaaa";
        assert(dsa::longest_palindromic_substring(s) == "aaaa");
        // For "aaaa", palindromes: 4 of len 1, 3 of len 2, 2 of len 3, 1 of len 4 = 10
        assert(dsa::count_palindromic_substrings(s) == 10);
    }

    // Test 5: Single character and empty string
    assert(dsa::longest_palindromic_substring("a") == "a");
    assert(dsa::count_palindromic_substrings("a") == 1);
    assert(dsa::longest_palindromic_substring("") == "");
    assert(dsa::count_palindromic_substrings("") == 0);

    // Test 6: Longest Palindromic Substring with multiple occurrences
    {
        std::string s = "abacdfgdcaba";
        // Palindromes include "aba" and "aba"
        std::string lps = dsa::longest_palindromic_substring(s);
        assert(lps == "aba");
    }

    std::cout << "[PASS] All Manacher's Algorithm C++ unit tests passed." << std::endl;
    return 0;
}

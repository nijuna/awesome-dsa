# Contributing to The DSA Handbook

Thank you for your interest in contributing to **The DSA Handbook**!

The DSA Handbook is an open-source, invariant-first reference manual and implementation engineering archive for data structures and algorithms. Unlike link-aggregators or flat solution dumps, every chapter in this handbook couples formal mathematical invariants and asymptotic proofs with physical CPU cache modeling, production systems case studies, and verified implementations in modern C++ and Python.

We welcome contributions from systems engineers, competitive programmers, researchers, and educators who share a commitment to technical precision.

---

## Contribution Categories

We accept contributions across four primary areas:
1. **Chapter Improvements**: Clarifying mental models, refining invariant proofs, expanding cache/hardware analysis, or improving systems case studies.
2. **Implementation Engineering**: Providing verified implementations, modernizing C++17/Python code, or adding memory-conscious implementations in Rust or Go.
3. **Test Suites & Verification**: Writing differential property-based tests, adversarial inputs, and fuzzers under `implementations/`.
4. **Errata & Link Integrity**: Correcting mathematical typos, fixing broken references, and updating citations to primary research literature.

---

## Editorial Standards

Every chapter in `docs/` must strictly adhere to the [**Style Guide**](STYLE-GUIDE.md) and fulfill the canonical **12-Section Architecture**:

1. **Why This Matters**: Concrete motivation, failure modes of naive approaches, and architectural context.
2. **Core Intuition & Visual Mental Model**: Conceptual explanation accompanied by at least one valid GitHub-rendered Mermaid diagram (`flowchart`, `graph`, or `sequenceDiagram`).
3. **Formal Invariants & Definitions**: Explicit mathematical invariants preserved before and after every operation.
4. **Operations, Mechanics & Step-by-Step Walkthrough**: Detailed operation mechanics with step-by-step state transitions.
5. **Asymptotic & Complexity Analysis**: Exact bounds for best, average, and worst-case time, auxiliary space, and amortized complexity with formal derivation.
6. **Memory Layout & CPU Cache Reality**: Analysis of pointer-chasing overhead, spatial/temporal locality, 64-byte cache line utilization, and memory fragmentation.
7. **Verified Python Implementation**: Clean, idiomatic, fully type-annotated (PEP 484) reference code.
8. **Verified Modern C++17 Implementation**: Cache-conscious, RAII-compliant code compiling cleanly under `-std=c++17 -Wall -Wextra -Werror -pedantic`.
9. **Edge Cases, Pitfalls & Adversarial Inputs**: Boundary conditions, overflow hazards, empty states, and adversarial sequences.
10. **Real-World Applications & Systems Case Studies**: Practical usage in operating system kernels, databases, storage engines, or distributed networks.
11. **Practice Problems & Categorized Exercises**: Curated problem sets mapped to platforms (LeetCode, CSES, Codeforces).
12. **References & Seminal Literature**: Formal academic citations to the original founding papers and textbooks.

---

## Code & Testing Requirements

All code submitted to `implementations/` must satisfy the following criteria:

* **Zero Compiler Warnings**: C++ code must compile with:
  ```bash
  g++ -std=c++17 -Wall -Wextra -Werror -pedantic -O3 -o solution main.cpp
  ```
* **Memory Safety**: No raw manual `new`/`delete` where smart pointers or contiguous vectors suffice; no memory leaks or undefined behavior under Valgrind / AddressSanitizer (`-fsanitize=address,undefined`).
* **Python Standards**: Python code must target Python 3.10+, pass type verification, and include unit tests using `unittest`.
* **Automated Verification**: Before submitting a PR, verify all Python unit and differential property tests pass:
  ```bash
  python3 -m unittest discover -s implementations/python -p "*.py"
  ```
* **Syntax Integrity**: Run the syntax validation script to verify that all Markdown files, Mermaid diagrams, and LaTeX math blocks render without errors:
  ```bash
  python3 scratch/validate_syntax.py
  ```

---

## Pull Request Workflow

1. **Fork the Repository**: Clone your fork locally.
2. **Create a Dedicated Branch**: Use a descriptive branch name reflecting your topic:
   ```bash
   git checkout -b topic/red-black-tree-audit
   ```
3. **Adhere to Conventional Commits**: We strictly enforce academic conventional commit messages:
   - `docs(module): refine loop invariant proof in binary search`
   - `feat(cpp): implement lock-free Chase-Lev work-stealing deque`
   - `test(python): add property fuzzing for interval tree queries`
   - `fix(complexity): correct worst-case recurrence bound for quickselect`
   - *Avoid colloquial or hype-laden commit messages.*
4. **Verify Locally**: Ensure syntax validation and all test suites pass with zero warnings or failures.
5. **Open a Pull Request**: Provide a concise summary of changes, the rationale for the edit, and confirmation that all test suites pass.

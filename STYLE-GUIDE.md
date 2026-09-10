# Awesome DSA Style Guide & Contribution Contract

To maintain the highest standards of clarity, visual intuition, and academic rigor, all content added to **Awesome DSA** must adhere to this style guide.

---

## 1. Typography & Mathematical Notation

* **Asymptotic Notation**: Always format Big-O, Omega, and Theta with LaTeX math blocks using dollar signs:
  * Good: $O(N \log N)$, $\Theta(V + E)$, $\Omega(1)$, $O(\alpha(N))$
  * Bad: `O(n log n)`, `O(N*log(N))`, `O(V+E)`
* **Variables & Dimensions**: Use uppercase $N$ for primary input size, $M$ for secondary dimensions, $V$ for vertices, and $E$ for edges.
* **Inline Code**: Use backticks for types, variables, and function names in text (e.g. `vector<int>`, `size_t`, `nullptr`).

---

## 2. Visuals & Diagrams

* **Mermaid Preferred**: Use GitHub-native Mermaid diagrams for state trees, graph traversals, and execution flows.
  ```mermaid
  graph TD
      A["Array [0..N-1]"] --> B["Left Child [2i+1]"]
      A --> C["Right Child [2i+2]"]
  ```
* **ASCII Art for Memory Layout**: For CPU cache lines, byte offsets, and memory alignment, clean ASCII art with fixed-width formatting is preferred:
  ```text
  +-------------------+-------------------+-------------------+
  | Cache Line (64B)  | Header (16B)      | Data Payload (48B)|
  +-------------------+-------------------+-------------------+
  ```

---

## 3. Code Standards

* **Python**:
  * Follow PEP 8.
  * Use clear type annotations (`def find_lca(u: int, v: int) -> int:`).
  * Prioritize clean, readable, self-documenting code over excessive one-liners.
* **C++**:
  * Modern C++ (C++17 or C++20).
  * Follow Google / LLVM styling conventions.
  * Use `size_t` for sizes and indices.
  * Avoid `using namespace std;` in header-like code; prefer explicit namespaces.

---

## 4. GitHub-Flavored Callouts (Alerts)

Use GitHub blockquote callouts purposefully:
* `> [!NOTE]` for contextual background or mathematical definition nuances.
* `> [!TIP]` for performance optimizations, constant factor reductions, or interview shortcuts.
* `> [!IMPORTANT]` for core invariants that must never be broken.
* `> [!WARNING]` for anti-patterns, cache-thrashing behaviors, or common traps.

---

## 5. File Naming & Linking

* **File Names**: Always use kebab-case (all lowercase with hyphens): e.g. `b-trees-and-b-plus-trees.md`.
* **Relative Links**: Always use explicit relative Markdown links with the `.md` extension so links resolve both in GitHub web UI and local tools like Obsidian and VS Code.
  * Example: `[B+ Trees](../05-trees-and-hierarchical-structures/b-trees-and-b-plus-trees.md)`

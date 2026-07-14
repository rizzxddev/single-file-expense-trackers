# Expense Tracker CLI 🪙

![Language](https://img.shields.io/badge/language-C-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)

A high-performance, interactive command-line interface (CLI) financial management application built natively in pure C. The system runs on a custom REPL (Read-Eval-Print Loop) architecture featuring advanced memory scaling, custom quote-aware string parsing, and immediate persistence layering.

## 🚀 Key Architectural Features

- **Quote-Aware Argument Tokenization:** Replaces generic whitespace split methods with a custom parser capable of handling encapsulated string arguments (`"Like This"`) for complex fields like tags and item descriptions.
- **Dynamic Memory Re-allocation:** Implements automated runtime dynamic array growth via heuristic heap calculations (`realloc`) to prevent artificial limits on the transaction log size.
- **Persistent Storage Layer:** Integrates a lightweight flat-file database using standard RFC CSV encoding rules. Data syncs immediately to the disk following mutating actions (`add`, `delete`).
- **Statistical Aggregation Metrics:** Offers sophisticated month-to-month localized reports, calculating total category expenditure weights, average trade sizes, and highest single expense limits.
- **Deterministic Record Querying:** Every financial transaction receives a self-incrementing Unique Identifier (UID) for explicit record tracking and mutation operations.

---

## 🛠️ Installation & Compilation

Ensure you have a C compiler installed (e.g., `gcc` or `clang`). 

### 1. Compilation
Compile the standalone source code using standard compilation vectors:
```bash
gcc main.c -o expense-tracker

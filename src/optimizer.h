#pragma once
// ─────────────────────────────────────────────────────────────────────────────
//  optimizer.h — Windows OS-level optimizations (native C++ version)
// ─────────────────────────────────────────────────────────────────────────────

#include <string>
#include <vector>

namespace optimizer {

struct Result {
    bool ok = false;
    std::string error;
    std::string mode;
};

Result applyOpt(const std::string& key, const std::string& mcExePath = "");
Result revertOpt(const std::string& key, const std::string& mcExePath = "");
std::vector<std::string> getApplied();
void restoreApplied(const std::vector<std::string>& keys, const std::string& mcExePath = "");
void cleanup();

// All known optimization keys
const std::vector<std::string>& allKeys();

} // namespace optimizer

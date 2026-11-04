#include "ProcessFactory.h"
#include <iostream>
#include <set>
#include <thread>
#include <vector>
int main() {
    Config c; c.minIns = 1; c.maxIns = 3;
    ProcessFactory f(c);
    std::cout << "named p02: " << (f.createNamed("p02") ? "created" : "null") << "\n";
    std::cout << "named p02 again: " << (f.createNamed("p02") ? "created" : "null") << "\n";
    std::cout << "named empty: " << (f.createNamed("") ? "created" : "null") << "\n";
    std::cout << "case differs P02: " << (f.createNamed("P02") ? "created" : "null") << "\n";
    auto a = f.createAuto(), b = f.createAuto(), d = f.createAuto();
    std::cout << "auto: " << a->name() << " " << b->name() << " " << d->name() << "  (p02 must be skipped)\n";
    // concurrency: 4 threads x 500 autos + a user thread grabbing names
    std::vector<std::thread> ts; std::mutex m; std::multiset<std::string> all;
    for (int t = 0; t < 4; ++t) ts.emplace_back([&] { for (int i = 0; i < 500; ++i) { auto p = f.createAuto(); std::lock_guard<std::mutex> lk(m); all.insert(p->name()); } });
    ts.emplace_back([&] { for (int i = 20; i < 400; i += 7) { auto p = f.createNamed("p" + std::to_string(i)); if (p) { std::lock_guard<std::mutex> lk(m); all.insert(p->name()); } } });
    for (auto& t : ts) t.join();
    std::set<std::string> uniq(all.begin(), all.end());
    std::cout << "concurrent: " << all.size() << " created, " << uniq.size() << " unique\n";
    auto big = f.createAuto(); (void)big;
    ProcessFactory g(c); for (int i = 0; i < 1239; ++i) g.createAuto();
    std::cout << "1240th auto name: " << g.createAuto()->name() << "\n";
}

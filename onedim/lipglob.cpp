/*
 * A simple branch-and-bound code with lischitzian bounds
 *
 * Created on April 21, 2018, 11:29 AM
 */

#include <cstdlib>
#include <iostream>
#include <vector>
#include <limits>
#include <testfuncs/onedim/univarbenchmarks.hpp>
#include <testfuncs/onedim/univarbenchmark.hpp>
#include <common/sgerrcheck.hpp>

constexpr double eps = 0.0001;

struct Flags {
    bool mCompIntervalBound = true;

    bool mDoReduct = false;
} flags;

void findGlobMin(const std::shared_ptr<UnivarBenchmark<double>> pbench, double& fbest, double& xbest, int& steps) {
    const double a = pbench->getBounds().first;
    const double b = pbench->getBounds().second;
    std::vector<Interval<double>> segments;
    segments.emplace_back(a, b);
    steps = 0;
    while (!segments.empty()) {
        try {
            const Interval<double> ci = segments.back();
            segments.pop_back();
            const double c = 0.5 * (ci.lb() + ci.rb());
            double fc = pbench->calcFunc(c);
            if (fc < fbest) {
                fbest = fc;
                xbest = c;
            }
            const Interval<double> dib = pbench->calcIntervalSymDiff(ci, 1);
            const double L = std::max(std::abs(dib.lb()), std::abs(dib.rb()));
            const double r = 0.5 * (ci.rb() - ci.lb());
            double lb = fc - L * r;
            if (flags.mCompIntervalBound) {
                const Interval<double> ib = pbench->calcInterval(ci);
                lb = std::max(ib.lb(), lb);
            }
            if (fbest - lb > eps) {
                if (flags.mDoReduct) {
                    const double d = (fc - fbest + eps)/L;
                    segments.emplace_back(ci.lb(), c - d);
                    segments.emplace_back(c + d, ci.rb());                                        
                } else {
                    segments.emplace_back(ci.lb(), c);
                    segments.emplace_back(c, ci.rb());                    
                }
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "Exception caught!\n";
            break;
        }
        steps++;
    }
}

void findMinForAllBenchMarks() {
    UnivarBenchmarks<double> bms;
    for (auto ptrBench : bms) {
        std::cout << "*************Testing benchmark**********" << std::endl;
        std::cout << *ptrBench;
        double fbest = std::numeric_limits<double>::max();
        double xbest = 0;
        int steps = 0;
        findGlobMin(ptrBench, fbest, xbest, steps);
        std::cout << "Found f = " << fbest << " at x = " << xbest << std::endl;
        std::cout << "Steps = " << steps << std::endl;
        std::cout << "****************************************" << std::endl << std::endl;
    }

}

/*
 * 
 */
int main(int argc, char** argv) {
    findMinForAllBenchMarks();
    return 0;
}


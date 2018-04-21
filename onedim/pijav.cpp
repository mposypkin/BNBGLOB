/*
 * A simple branch-and-bound code with lischitzian bounds
 */

/* 
 * Simple Pijavskiy solver
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

    bool mDoReduct = true;
} flags;

struct Segment {

    Segment(double a, double b, double A, double B) :
    mInterval(a, b), mA(A), mB(B) {
    }
    /**
     * Interval of the segment
     */
    Interval<double> mInterval;

    /**
     * Objective value at the left end
     */
    double mA;

    /**
     * Objective value at the right end
     */
    double mB;
};

void findGlobMin(const std::shared_ptr<UnivarBenchmark<double>> pbench, double& fbest, double& xbest, int& steps) {
    const double a = pbench->getBounds().first;
    const double b = pbench->getBounds().second;
    const double fa = pbench->calcFunc(a);
    const double fb = pbench->calcFunc(b);
    auto update = [&](double fn, double xn) {
        if (fn < fbest) {
            fbest = fn;
            xbest = xn;
        }
    };
    update(fa, a);
    update(fb, b);
    std::vector<Segment> segments;
    segments.emplace_back(a, b, fa, fb);
    steps = 0;
    while (!segments.empty()) {
        try {
            const Segment s = segments.back();
            segments.pop_back();
            const Interval<double> ci = s.mInterval;
            const Interval<double> dib = pbench->calcIntervalSymDiff(ci, 1);
            const double L = std::max(std::abs(dib.lb()), std::abs(dib.rb()));
            const double c = (s.mA - s.mB + L * (ci.lb() + ci.rb())) / (2. * L);
            if (!((ci.lb() <= c) && (c <= ci.rb()))) {
                std::cout << "========== wrong L ======== \n";
                std::cout << "L = " << L << ", interval " << ci << "\n";
                std::cout << "=========================== \n";
            }
            double lb = s.mA - L * (c - ci.lb());
 
            if (flags.mCompIntervalBound) {
                const Interval<double> ib = pbench->calcInterval(ci);
                const double ilb = ib.lb();
                lb = std::max(lb, ilb);
            }

            double fc = pbench->calcFunc(c);
            update(fc, c);
            if (fbest - lb > eps) {
                if (flags.mDoReduct) {
                    const double a = ci.lb() + (s.mA - fbest + eps) / L;
                    const double b = ci.rb() - (s.mB - fbest + eps) / L;
                    SG_ASSERT((a <= c) && (c <= b));
                    const double fa = pbench->calcFunc(a);
                    const double fb = pbench->calcFunc(b);
                    segments.emplace_back(a, c, fa, fc);
                    segments.emplace_back(c, b, fc, fb);
                } else {
                    segments.emplace_back(ci.lb(), c, s.mA, fc);
                    segments.emplace_back(c, ci.rb(), fc, s.mB);
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


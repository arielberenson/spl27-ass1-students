#include "SchedulingPolicy.h"
#include <vector>
#include <Request.h>
#include <algorithm>

std::vector<int> ArrivalOrderPolicy::choose(const std::vector<const Request*>& eligible, const std::vector<GPU>& gpus) const{
    std::vector<int> indices;

    for (std::size_t i = 0; i < eligible.size(); i++) {
        indices.push_back(i);
    }

    std::sort(indices.begin(), indices.end(),
        [&eligible](int a, int b) {

            const Request* r1 = eligible[a];
            const Request* r2 = eligible[b];

            if (r1->getAdmissionCycle() != r2->getAdmissionCycle()) {
                return r1->getAdmissionCycle() < r2->getAdmissionCycle();
            }

            return r1->getId() < r2->getId();
        }
    );

    return indices;
}
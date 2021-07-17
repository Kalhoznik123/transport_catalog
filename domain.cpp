#include "domain.h"

bool transport::Bus::operator<(const Bus& rhs) const {
    return name < rhs.name;
}

#include "ae.h"

// outputs random float between min and max
// uniformly distributed
float randomFloat(float min, float max)
{
    float random = ((float) rand()) / (float) RAND_MAX;

    // example : generate float between 1 and 3
    // generate (in your case) a float between 0 and (3-1)
    // then add 1 to get a float between 1  and 3
    float range = max - min;
    return (random*range) + min;
}
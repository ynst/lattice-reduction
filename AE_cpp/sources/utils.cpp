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

float randomFrechet (float uniform){
	return (1 * exp(log(-log(uniform))/-3) - 3);
}

void displayProgressBar(float progress){
	if (progress < 1.0) {
	    int barWidth = 70;

	    std::cout << "[";
	    int pos = barWidth * progress;
	    for (int i = 0; i < barWidth; ++i) {
	        if (i < pos) std::cout << "=";
	        else if (i == pos) std::cout << ">";
	        else std::cout << " ";
	    }
	    std::cout << "] " << int(progress * 100.0) << " %\r";
	    std::cout.flush();
	}
	std::cout << std::endl;
}
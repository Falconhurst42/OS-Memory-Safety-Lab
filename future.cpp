#include <future>
#include <iostream>
#include <functional>
#include <math.h>
#include <chrono>
#include <thread>
#include <vector>
#include <type_traits>
#include <utility>
#include "memSafe.h"

double bisection(std::function<double(double)> f, double a, double b, double precision_limit);

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type isPrime(T n);

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type isFragilePrime(T n, uint32_t base = 10);

std::vector<uint64_t> fragilePrimes(uint64_t upper = 10000000, uint64_t lower = 1);

std::vector<uint64_t> batchedFragilePrimes(uint64_t upper = 10000000, uint64_t lower = 1, uint64_t batch_count = 0);

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type primedIsPrime(std::vector<T> primes, T n);
template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type primedIsFragilePrime(std::vector<T> primes, T n, uint32_t base = 10);
std::vector<uint64_t> findPrimes(uint64_t max);
std::vector<uint64_t> primedFP(std::vector<uint64_t> primes, uint64_t upper = 10000000, uint64_t lower = 1);
std::vector<uint64_t> primedBFP(uint64_t upper = 10000000, uint64_t lower = 1, uint64_t batch_count = 0);

template<typename T>
void printVector(std::vector<T> v);

std::pair<std::chrono::nanoseconds, std::vector<uint64_t>> timeFPMethod(std::function<std::vector<uint64_t>(uint64_t,uint64_t)> method, uint64_t upper = 10000000, uint64_t lower = 1);
std::pair<std::chrono::nanoseconds, std::vector<uint64_t>> timeFPMethod(std::function<std::vector<uint64_t>(uint64_t,uint64_t,uint64_t)> method, uint64_t upper = 10000000, uint64_t lower = 1, uint64_t arg3 = 0);

double foo(double x) {
	return std::pow(x, 5) - x + 1;
}

int main() {
	/*std::future<double> bisect_future = std::async(bisection, foo, -3, 3, std::pow(10, -8));
	std::cout << "Stalling";
	for(int i = 0; i < 10; i++) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(10));
		std::cout << ".";
	}
	double result = bisect_future.get();
	std::cout << std::endl << "x = " << result  << ", f(x) = " << foo(result) << std::endl;*/

	const int LOWER = 1, UPPER = 10000000;
	//const int LOWER = 1, UPPER = 1000000000;
    auto start = std::chrono::steady_clock::now();

	printf("Finding all fragile primes from %d to %d:\n", LOWER, UPPER);
	for(double i = 0.2; i <= 3; i+=0.4) {
		int batches = std::pow(10, i);
		int batch_size = (UPPER - LOWER + 1) / batches;
		std::pair<std::chrono::nanoseconds, std::vector<uint64_t>> batched_results = timeFPMethod(batchedFragilePrimes, LOWER, UPPER, batches);
		std::cout << "With " << batches << " batches (batch size " << batch_size << "), the method took ";
		std::cout << batched_results.first.count() << "ns or " << std::chrono::duration_cast<std::chrono::milliseconds>(batched_results.first).count() << "ms or " << std::chrono::duration_cast<std::chrono::seconds>(batched_results.first).count() << "s or " << (double)(std::chrono::duration_cast<std::chrono::seconds>(batched_results.first).count())/60.0 << "m" << std::endl;
		std::cout << "Found " << batched_results.second.size() << std::endl;
		//printVector(batched_results.second);
	}

	/*printf("\n\nFinding all fragile primes from %d to %d with primed method:\n", LOWER, UPPER);
	for(double i = 0.6; i <= 3; i+=0.4) {
		int batches = std::pow(10, i);
		int batch_size = (UPPER - LOWER + 1) / batches;
		std::cout << "With " << batches << " batches (batch size " << batch_size << "), the method took ";
		std::pair<std::chrono::nanoseconds, std::vector<uint64_t>> batched_results = timeFPMethod(primedBFP, LOWER, UPPER, batches);
		std::cout << batched_results.first.count() << "ns or " << std::chrono::duration_cast<std::chrono::milliseconds>(batched_results.first).count() << "ms or " << std::chrono::duration_cast<std::chrono::seconds>(batched_results.first).count() << "s or " << (double)(std::chrono::duration_cast<std::chrono::seconds>(batched_results.first).count())/60.0 << "m" << std::endl;
		std::cout << "Found " << batched_results.second.size() << std::endl;
		//printVector(batched_results.second);
	}*/
	checkForLeaks();

    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << "s" << std::endl;
}

double bisection(std::function<double(double)> f, double a, double b, double precision_limit) {
	double c, f_a = f(a), f_b = f(b);
	while((b-a)/2 > precision_limit) {
		c = (a+b)/2;
		double f_c = f(c);
		if(f_c == 0) {
			break;
		} else if(f_a*f_c < 0) {
			b = c;
		} else {
			a = c;
		}
	}
	std::cout << "Complete" << std::endl;
	return c;
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type isPrime(T n) {
	T lim = std::sqrt(n);
	for(T i = 2; i < lim; i++) {
		if(n % i == 0) {
			return false;
		}
	}
	return true;
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type isFragilePrime(T n, uint32_t base) {
	double lbase = log(base);
	if(isPrime(n)) {
		uint8_t digits = std::ceil(std::log(n+1)/lbase);
		for(T i = 0; i < digits; i++) {
			T p = pow(base,i);
			uint8_t d = (n / p) % base;
			T temp = n - d*p;
			for(uint8_t add = 0; add < d; add++) {
				if(isPrime(temp + add*p)) {
					return false;
				}
			}for(uint8_t add = d+1; add < base; add++) {
				if(isPrime(temp + add*p)) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

std::vector<uint64_t> fragilePrimes(uint64_t upper, uint64_t lower) {
	std::vector<uint64_t> results;
	while(lower <= upper) {
		if(isFragilePrime(lower)) {
			//std::cout << "Found: " << lower << std::endl;
			results.push_back(lower);
		}
		lower++;
	}
	return results;
}

std::vector<uint64_t> batchedFragilePrimes(uint64_t upper, uint64_t lower, uint64_t batch_count) {
	// ensure upper >= lower
	if(upper < lower) {
		return batchedFragilePrimes(lower, upper, batch_count);
	}

	// set up batches
	uint64_t BATCH_SIZE;
	if(batch_count == 0) {
		BATCH_SIZE = 1000000;
	} else {
		BATCH_SIZE = (upper - lower) / batch_count;
	}
	const int BATCHES = (upper-lower)/BATCH_SIZE + 1;
	std::vector<std::future<std::vector<uint64_t>>> futureVec(BATCHES);

	// dispatch async batches
	for(int i = 0; i < BATCHES-1; i++) {
		futureVec[i] = std::async(fragilePrimes, lower + BATCH_SIZE - 1, lower);
		lower += BATCH_SIZE;
	}
	futureVec[BATCHES-1] = std::async(fragilePrimes, upper, lower);

	// collect batches
	std::vector<uint64_t> results;
	for(int i = 0; i < BATCHES; i++) {
		std::vector<uint64_t> batchResults = futureVec[i].get();
		results.insert(results.end(), batchResults.begin(), batchResults.end());
	}

	// return final results
	return results;
}

std::vector<uint64_t> findPrimes(uint64_t max) {
	std::vector<uint64_t> out;
	for(int i = 2; i <= max; i++) {
		bool found = false;
		uint64_t bound = sqrt(i);
		for(int j = 0; !found && j < out.size() && out[j] <= bound; j++) {
			found = (i % out[j] == 0);
		}
		if(!found) {
			out.push_back(i);
		}
	}
	return out;
}


template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type primedIsPrime(std::vector<T> primes, T n) {
	T maxCheck = ceil(sqrt(n));
	for(int i = 0; i < primes.size() && primes[i] <= maxCheck; i++) {
		if(n % primes[i] == 0) {
			return false;
		}
	}
	return true;
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type primedIsFragilePrime(std::vector<T> primes, T n, uint32_t base) {
	double lbase = log(base);
	if(primedIsPrime(primes, n)) {
		uint8_t digits = std::ceil(std::log(n+1)/lbase);
		for(T i = 0; i < digits; i++) {
			T p = pow(base,i);
			uint8_t d = (n / p) % base;
			T temp = n - d*p;
			for(uint8_t add = 0; add < d; add++) {
				if(primedIsPrime(primes, temp + add*p)) {
					return false;
				}
			}for(uint8_t add = d+1; add < base; add++) {
				if(primedIsPrime(primes, temp + add*p)) {
					return false;
				}
			}
		}
		return true;
	}
	return false;
}

std::vector<uint64_t> primedFP(std::vector<uint64_t> primes, uint64_t upper, uint64_t lower) {
	std::vector<uint64_t> results;
	while(lower <= upper) {
		if(primedIsFragilePrime(primes, lower)) {
			//std::cout << "Found: " << lower << std::endl;
			results.push_back(lower);
		}
		lower++;
	}
	return results;
}

std::vector<uint64_t> primedBFP(uint64_t upper, uint64_t lower, uint64_t batch_count) {
	// ensure upper >= lower
	if(upper < lower) {
		return batchedFragilePrimes(lower, upper, batch_count);
	}

	// get all relevant primes
	uint64_t maxCheck = ceil(sqrt(upper));
	std::vector<uint64_t> primes = findPrimes(maxCheck);

	// update lower
	uint64_t oldLower = lower;
	lower = std::max(lower, maxCheck);

	// set up batches
	uint64_t BATCH_SIZE;
	if(batch_count == 0) {
		BATCH_SIZE = 1000000;
	} else {
		BATCH_SIZE = (upper - lower) / batch_count;
	}
	const int BATCHES = (upper-lower)/BATCH_SIZE + 1;
	std::vector<std::future<std::vector<uint64_t>>> futureVec(BATCHES);

	// dispatch async batches
	std::vector<uint64_t> results;
	for(int i = 0; i < BATCHES-1; i++) {
		futureVec[i] = std::async(fragilePrimes, lower + BATCH_SIZE - 1, lower);
		lower += BATCH_SIZE;
	}
	futureVec[BATCHES-1] = std::async(fragilePrimes, upper, lower);

	// check pre-checked primes for fragiles
	for(int i = primes.size(); i >= 0 && primes[i] > oldLower; i--) {
		if(primedIsFragilePrime(primes, primes[i])) {
			results.push_back(primes[i]);
		}
	}

	// collect batches
	for(int i = 0; i < BATCHES; i++) {
		std::vector<uint64_t> batchResults = futureVec[i].get();
		results.insert(results.end(), batchResults.begin(), batchResults.end());
	}

	// return final results
	return results;
}

template<typename T>
void printVector(std::vector<T> v) {
	std::cout << "[";
	if(v.size() > 0) {
		for(typename std::vector<T>::iterator i = v.begin(); i < v.end()-1; i++) {
			std::cout << *i << ", ";
		}
		std::cout << *(v.end()-1);
	}
	std::cout << "]" << std::endl;
}

std::pair<std::chrono::nanoseconds, std::vector<uint64_t>> timeFPMethod(std::function<std::vector<uint64_t>(uint64_t,uint64_t)> method, uint64_t upper, uint64_t lower) {
	if(upper < lower) {
		return timeFPMethod(method, lower, upper);
	}
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::vector<uint64_t> results = method(upper, lower);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	return std::make_pair(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start), results);
}

std::pair<std::chrono::nanoseconds, std::vector<uint64_t>> timeFPMethod(std::function<std::vector<uint64_t>(uint64_t,uint64_t,uint64_t)> method, uint64_t upper, uint64_t lower, uint64_t arg3) {
	if(upper < lower) {
		return timeFPMethod(method, lower, upper, arg3);
	}
	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::vector<uint64_t> result = method(upper, lower, arg3);
	std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

	return std::make_pair(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start), result);
}

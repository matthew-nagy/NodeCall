#include <thread>
#include <mutex>
#include <atomic>


class Semaphore{
public:

	void incriment();

	void decriment();

	unsigned getCount();

	Semaphore(unsigned semCount);
private:
    std::mutex waitForIncrimentLock;//Used as the lock for a decriment on sem 0
	std::mutex blockingLock;		//Prevents two different semaphores from ever racing over locking waitForIncrimentLock
    std::mutex internalMutex;		//Locked for the duration of any method to force single-threadedness
    std::atomic_uint semCount;		//The internal track of the semaphores value
};
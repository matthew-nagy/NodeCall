#include "../header/semaphore.hpp"


//Internal mutex is locked on all operations meaning only one function can be running in a thread at once

void Semaphore::incriment() {
	std::unique_lock<std::mutex> lock(internalMutex);
	semCount += 1;
	//Notify any waiting thread
	blocker.notify_one();
}

void Semaphore::decriment() {
	std::unique_lock<std::mutex> lock(internalMutex);

	//Wait for a count
	while (semCount == 0)
		//blocks, but in a while loop just in case
		blocker.wait(lock);

	semCount -= 1;
}


unsigned Semaphore::getCount() {
	return semCount;
}

Semaphore::Semaphore(unsigned semCount) :
	semCount(semCount)
{}
#include "../header/semaphore.hpp"


//Internal mutex is locked on all operations meaning only one function can be running in a thread at once

void Semaphore::incriment() {
	internalMutex.lock();
	semCount += 1;

	if (semCount == 1) {//Newly unlocked
		//If there is a decriment waiting, unlock the inner lock and let it proceed
		waitForIncrimentLock.try_lock();
		waitForIncrimentLock.unlock();
	}

	internalMutex.unlock();
}

void Semaphore::decriment() {
	internalMutex.lock();

	if (semCount == 0) {
		//You are about to block, so unlock to let an incriment happen
		internalMutex.unlock();
		blockingLock.lock();
		//-----------------
		waitForIncrimentLock.lock();

		//In here you wait for an incriment to unlock it by incrimenting up from 0

		waitForIncrimentLock.lock();
		//-----------------
		blockingLock.unlock();
		//You are continuing, lock the internal mutex again
		internalMutex.lock();
		semCount -= 1;
		waitForIncrimentLock.unlock();
	}
	else {
		semCount -= 1;
	}

	internalMutex.unlock();
}


unsigned Semaphore::getCount() {
	return semCount;
}

Semaphore::Semaphore(unsigned semCount) :
	semCount(semCount)
{}
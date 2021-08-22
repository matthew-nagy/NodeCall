#pragma once
#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP
#include <thread>
#include <mutex>
#include <atomic>
#include<condition_variable>


class Semaphore {
public:

	void incriment();

	void decriment();

	unsigned getCount();

	Semaphore(unsigned semCount);
private:
	std::mutex internalMutex;		//Locked for the duration of any method to force single-threadedness
	std::condition_variable blocker;//Used to block threads on a decriment
	std::atomic_uint semCount;		//The internal track of the semaphores value
};
#endif
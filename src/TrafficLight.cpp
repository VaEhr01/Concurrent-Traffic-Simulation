#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.
    
    /* Create lock and wait for and receive new messages */
	std::unique_lock<std::mutex> ulck(_mutex);
	_condition.wait(ulck, [this] { return !_queue.empty(); });

	/* Pull the latest element and remove it from the queue */
	T msg = std::move(_queue.back());
	_queue.pop_back();
	return msg; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    /* apply lock */
	std::lock_guard<std::mutex> ulck(_mutex);
    _queue.clear(); //remove old informations

	/* Move into queue and send notification */
	_queue.push_back(std::move(msg));
	_condition.notify_one();
}


/* Implementation of class "TrafficLight" */ 
TrafficLight::TrafficLight()
{
    _queue = std::make_shared<MessageQueue<TrafficLightPhase>>();
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        /* sleep to avoid CPU load */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if(_queue->receive() == TrafficLightPhase::green){
            return; /* leave while loop */
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method ???cycleThroughPhases??? should be started in a thread when the public method ???simulate??? is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    /* generation of random number between 4 and 6 seconds */
	std::random_device rd;
	std::mt19937 eng(rd());
	std::uniform_int_distribution<> distr(4, 6);
    int cycle_duration = distr(eng);

    /* Save inital time of stop watch */
	auto last_time = std::chrono::system_clock::now();

    while(true){
        /* sleep for 1 sec between two cycles */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto timeSinceLastCall = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - last_time).count();
        if(timeSinceLastCall > cycle_duration){
            if(_currentPhase == TrafficLightPhase::red){
                _currentPhase = TrafficLightPhase::green;
            }else {
                _currentPhase = TrafficLightPhase::red;
            }

            /* send update method to queue */
            _queue->send(std::move(_currentPhase));

            /* Update last phase chance */
            last_time = std::chrono::system_clock::now();
        }
    }
}
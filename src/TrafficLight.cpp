#include <iostream>
#include <random>
#include <future>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    //The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> ulock(_qmtx);
    _cond.wait(ulock, [this]{return !_queue.empty();});

    T msg = std::move(_queue.back());
    _queue.pop_back();
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    std::lock_guard<std::mutex> lockGuard(_qmtx);
    _queue.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */
 
TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        auto msg = _msg_queue.receive();
        if (msg == TrafficLightPhase::green)
        {
            break;
        }
    }
    return;
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    //Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // Implemented the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    /* Init our random generation between 4 and 6 seconds */
    std::random_device rd;
    std::mt19937 eng(rd());
    std::uniform_int_distribution<> distr(4000, 6000);

    //cycle duration between traffic light toggle in ms
    double cycle_duration = distr(eng); //Duration of a single simulation cycle in seconds, is randomly chosen
    std::cout << "Traffic Light #" << _id << " Started " << cycle_duration  << std::endl;
    std::chrono::time_point<std::chrono::system_clock> lastUpdate;
    lastUpdate = std::chrono::system_clock::now();

    while(true)
    {
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();

        if(timeSinceLastUpdate >= cycle_duration)
        {

            //Toggle the phase
	    std::unique_lock<std::mutex> uLock(_mtx);
            if(_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
		std::cout << "Traffic Light #" << _id << "Green" << std::endl;
		uLock.unlock();
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
		std::cout << "Traffic Light #" << _id << " Red" << std::endl;
		uLock.unlock();
            }
            
            //send update to message queue
            auto msg = _currentPhase;
            _msg_queue.send(std::move(msg));

            //Reset the stop watch
            lastUpdate = std::chrono::system_clock::now();

            //Randomly choose the cycle duration for the next cycle
	    cycle_duration = distr(eng);
        }
    }
}


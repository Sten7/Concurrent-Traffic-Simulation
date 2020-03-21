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
    
    // Lock the mutex first
    std::unique_lock<std::mutex> lock(_mut);
    // Wait until an element is present
    _cond.wait(lock,[this] {return !_queue.empty();});

    // Remove element FIFO
    T e = std::move(_queue.front());
    _queue.pop_front();
    return e;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // Add msg to the queue under lock
    std::lock_guard<std::mutex> lock(_mut);
    _queue.push_back(std::move(msg));
    // Notify the client that the queue was changed
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true){
        if(_queue.receive()==green) return;
    }
}

TrafficLight::TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    // Launch cycleThroughPhases function in a thread
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.

    // Init stop watch
    auto lastUpdate = std::chrono::system_clock::now();
    // Set the next cycle duration, random 4000-6000 msecs
    long nextCycleDuration = std::rand() % 3000 + 4000;
    // Infinite loop
    while(true){
        // Wait before next iteration
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // Compute time difference between the stopwatch
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        // Check if the cycle duration reached the limit
        if(timeSinceLastUpdate>=nextCycleDuration){
            // Toggle the phase of the traffic light
            _currentPhase=_currentPhase==green?red:green;
            // Send an update method to the message queue
            _queue.send(std::move(_currentPhase));
            // Update the last update
            lastUpdate = std::chrono::system_clock::now();;
            // Generate next cycle length
            nextCycleDuration = std::rand() % 3000 + 4000;
        }
    }
}


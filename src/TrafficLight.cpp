#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _condition.wait(lock, [this] { return !_queue.empty(); });

    T result = std::move(_queue.back());
    _queue.pop_back();

    return result;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _queue.clear();
    _queue.emplace_back(msg);
    _condition.notify_one();
}

// Implementation of class TrafficLight
TrafficLight::TrafficLight() // constructor
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while (true)
    {
        if (_queue.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

void TrafficLight::toggleTrafficLight()
{
    if (_currentPhase == TrafficLightPhase::red)
    {
        _currentPhase = TrafficLightPhase::green;
    }
    else
    {
        _currentPhase = TrafficLightPhase::red;
    };
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    int delay = std::rand() % (6000 - 4000 + 1) + 4000; // (sufficiently) random number between 4 and 6
    auto timestamp = std::chrono::system_clock::now();
    std::chrono::milliseconds deltaT;

    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        deltaT = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - timestamp);
        if (deltaT >= std::chrono::milliseconds(delay))
        {
            toggleTrafficLight();
            _queue.send(std::move(_currentPhase));
            timestamp = std::chrono::system_clock::now();
        }
    };
}

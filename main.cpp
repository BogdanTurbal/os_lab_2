#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <semaphore>
#include <thread>

using namespace std;

const size_t NumPhilosophers = 5;

enum PhilosopherState {
    Thinking,
    Hungry,
    Eating
};

inline size_t LeftNeighbor(size_t index) {
    return (index + NumPhilosophers - 1) % NumPhilosophers;
}

inline size_t RightNeighbor(size_t index) {
    return (index + 1) % NumPhilosophers;
}

PhilosopherState philosophersState[NumPhilosophers];
mutex mtxCritical;
mutex mtxOutput;
binary_semaphore forks[NumPhilosophers]{
        binary_semaphore{0}, binary_semaphore{0},
        binary_semaphore{0}, binary_semaphore{0},
        binary_semaphore{0}
};

size_t RandomDuration(size_t min, size_t max) {
    static mt19937 generator(time(nullptr));
    return uniform_int_distribution<>(min, max)(generator);
}

void AttemptEat(size_t index) {
    if (philosophersState[index] == Hungry &&
        philosophersState[LeftNeighbor(index)] != Eating &&
        philosophersState[RightNeighbor(index)] != Eating) {
        philosophersState[index] = Eating;
        forks[index].release();
    }
}

void PhilosopherThink(size_t index) {
    size_t thinkTime = RandomDuration(400, 800);
    {
        lock_guard<mutex> lock(mtxOutput);
        cout << index << " thinking for " << thinkTime << "ms\n";
    }
    this_thread::sleep_for(chrono::milliseconds(thinkTime));
}

void GrabForks(size_t index) {
    {
        lock_guard<mutex> lock(mtxCritical);
        philosophersState[index] = Hungry;
        {
            lock_guard<mutex> outputLock(mtxOutput);
            cout << index << " hungry\n";
        }
        AttemptEat(index);
    }
    forks[index].acquire();
}

void PhilosopherEat(size_t index) {
    size_t eatTime = RandomDuration(400, 800);
    {
        lock_guard<mutex> lock(mtxOutput);
        cout << index << " eating for " << eatTime << "ms\n";
    }
    this_thread::sleep_for(chrono::milliseconds(eatTime));
}

void ReleaseForks(size_t index) {
    lock_guard<mutex> lock(mtxCritical);
    philosophersState[index] = Thinking;
    AttemptEat(LeftNeighbor(index));
    AttemptEat(RightNeighbor(index));
}

void PhilosopherLife(size_t index) {
    while (true) {
        PhilosopherThink(index);
        GrabForks(index);
        PhilosopherEat(index);
        ReleaseForks(index);
    }
}

int main() {
    cout << "Dining Philosophers Problem\n";

    thread t0([&] { PhilosopherLife(0); }); // [&] means every variable outside the lambda is captured by reference
    thread t1([&] { PhilosopherLife(1); });
    thread t2([&] { PhilosopherLife(2); });
    thread t3([&] { PhilosopherLife(3); });
    thread t4([&] { PhilosopherLife(4); });

    t0.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

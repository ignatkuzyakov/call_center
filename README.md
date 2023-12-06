# @call_center_emulator@

## Abbreviations and Terminology ##

* CC - Call Center (Call Processing Center)

* Subscriber A, also known as CgPN (Calling Party Number) - calling subscriber, call initiator

* Subscriber B, also known as CdPN (Called Party Number) - called subscriber

* PAC - Programmatically-Aided Complex (hardware and software complex)

* OWS - Operator Workstation, which is typically a PAC consisting of a computer with specialized software installed, as well as media processing equipment (headset + webcam)

* Call - a communication session, typically a voice call

* Call ID - call identifier, usually a string value that is a unique identifier for a call. It is desirable for this value to maintain its uniqueness over an extended period of time for the entire software complex.

* Release - call termination, call disconnect

* Release Cause - the reason for call termination, the reason for ending call processing. There are standards, such as Q.850, that describe all the standardized release causes.

* CDR - Call Detailed Record, an information record in a log that captures the details of a provided service. It is usually a text file or log with a fixed number of fields separated by a specific symbol (often a semicolon ";").

* CC is typically a software system that handles incoming calls from subscribers and distributes them to available operators. Examples include emergency services like 911 or customer support helplines for banks or mobile operators.

Operators use software that exchanges information with a central call handling service (which can be considered as a single server and application for simplicity), so the service has information about the availability and busy status of each operator.


## Prerequisite list ## 

```
sudo apt-get install libboost-all-dev g++-12 cmake
```
### [warn] Unfortunately you need install libboost-all-dev manualy (at least 1.74 version), FetchContent may cause linking error 

### (boost::log must be dynamically linked, however, BOOST_LOG_DYN_LINK does not solve this problem.)

### Source for this issue: https://www.boost.org/doc/libs/develop/libs/log/doc/html/log/installation/config.html

## Release build (for example without tests) ##

```
cmake -G "Unix Makefiles" -B build -DTESTING=OFF -DCMAKE_CXX_COMPILER=g++-12 -DCMAKE_BUILD_TYPE=Release .
```
```
cmake --build build
```

## Debug build (for example with tests) ##

```
cmake -G "Unix Makefiles" -B debug -DTESTING=ON -DCMAKE_CXX_COMPILER=g++-12 -DCMAKE_BUILD_TYPE=Debug .
```
```
cmake --build debug
```

## To run tests you can use ##

```
ctest --test-dir ./debug/tests/ && ctest --test-dir ./build/tests/
```
## Or just ##
```
./debug/tests/tests && ./build/tests/tests
```
## Run ##
From the source directory
```
sudo ./build/main/src/callCenter
```
```
./build/main/src/abonent
```
You can explicitly specify host, port, confi.json or number(for abonent)
```
sudo ./build/main/src/callCenter ./configure.json 0.0.0.0 81
```
```
./build/main/src/abonent 0.0.0.0 81 897362723
```
Run functional tests (which generates within -DTESTING=ON) temporarily for debug version

default host: 0.0.0.0, port: 81, it can be changed for your system in ./tests/tests_functional.cpp
```
sudo ./debug/tests/tests_functional
```
now you can manualy (TODO: automate log/journal analysis):
```
tail journal.txt
tail log.txt
```
## How it works ##

When a call comes in (HTTP REQ), first we validate the phone number. We check if the number is already in the queue. If it is not, we send a message saying "already in the queue" and close connection with Status = overload. Otherwise, the call is added to the queue and send CallID.

The queue is processed in a separate thread. Once a call is added to the call queue, we notify the handler that we are ready. The handler then checks if there is anyone in the queue, as there could be a spurious wakeup. It also checks if there is an available operator. If an available operator is found, we initiate the conversation.

We have a timer for the time spent in the queue. If the timer expires, we simply remove the subscriber from the queue with Status = timeout. However, if an operator becomes available before the timer expires, we extend the timer for a random duration to allow for the conversation.

The call center itself is implemented using an asynchronous server and utilizes the Boost library.

## Call Detail Record (CDR) ##

1. Call arrival timestamp
2. Incoming call identifie
3. Subscriber A number
4. Call completion timestamp
5. Call status (OK or error reason, e.g., timeout).
6. Operator response timestamp (if applicable, or empty value)
7. Operator identifier (empty value if the connection was not established)
8. Call duration (empty value if the connection was not established)

## What could be improved ##
* add encapsulation for notify interface
* change sync write method by future
* add automate log/journal analysis
* fix boost::log linking issue with FetchContent

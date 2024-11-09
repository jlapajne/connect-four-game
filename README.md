### Connect 4 game
Game utilizing server client architecture using websocketpp library. The library is attached as a submodule. Note that master branch of websocketpp library does not support c++20, but the develop branch dose, develop branch is cloned when cloning this repo.

Server side processes client messages asynchronously using asio thread pool, while client side message / tasks are processed sequentially.

##### Build:
Currently build system consists of Cmake (>3.2) and Ninja.
- cmake


##### Dependencies:
- websocketpp/develop
- asio
- sqlight3
- protobuf



##### TODOs:
- bug fixes,
- doxygen,
- asynchronous processing on client side,
- general code improvements and clean-up.
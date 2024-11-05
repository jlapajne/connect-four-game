add_library(server_lib
    ConnectFourGame.cpp
    ConnectFourGame.h
    
    Player.h
    Server.h
    ServerTypes.h
    ServerLogic.cpp
    Server.cpp
    GameStatistics.h
    
    PlayerManager.h
    PlayerManager.cpp

    RandomUtils.h
    RandomUtils.cpp
    
    GameManager.cpp
    GameManager.h

    Database.h
    Database.cpp
    )


    target_link_libraries(server_lib PUBLIC game_proto SQLite::SQLite3 Threads::Threads)

    add_executable(game_server main.cpp)
    target_link_libraries(game_server server_lib)
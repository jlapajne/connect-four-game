add_library(server
    main.cpp
    ConnectFourGame.cpp
    ConnectFourGame.h
    Player.h
    Server.h
    ServerLogic.h
    GameStatistics.h
    )


    target_link_libraries(server PUBLIC game_proto)
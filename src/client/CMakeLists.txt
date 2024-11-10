add_executable(client 
    Client.h
    Client.cpp
    ClientTypes.h
   
    IBot.h
    Bot.h
    Bot.cpp
    BotBase.h
    BotBase.cpp

   
    )

target_link_libraries(client server_lib game_proto)
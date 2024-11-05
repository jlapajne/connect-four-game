add_executable(client 
    client.cpp
    ClientTypes.h
    Bot.cpp
    Client.h
    )

target_link_libraries(client server_lib game_proto)
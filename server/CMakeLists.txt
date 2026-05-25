target_sources(argentum_grupo13_server
    PRIVATE
    main.cpp
    net/Acceptor.cpp
    net/ClientHandler.cpp
    net/ServerReceiverThread.cpp
    net/ServerSenderThread.cpp
    game/ServerGameLoop.cpp
    game/QueueMonitor.cpp
)

target_include_directories(argentum_grupo13_server
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_sources(argentum_grupo13_client
    PRIVATE
    main.cpp
    game/GameLoop.cpp
    render/Camera.cpp
    net/SenderThread.cpp
    net/ReceiverThread.cpp
)

target_include_directories(argentum_grupo13_client
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
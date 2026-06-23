# librería con todo el código del servidor (sin main)
add_library(argentum_grupo13_server_lib STATIC)

target_sources(argentum_grupo13_server_lib
    PRIVATE
    net/Acceptor.cpp
    net/ClientHandler.cpp
    net/ServerReceiverThread.cpp
    net/ServerProtocol.cpp
    net/ServerSenderThread.cpp
    game/ServerGameLoop.cpp
    game/commands/MovementCommands.cpp
    game/commands/SessionCommands.cpp
    game/commands/CombatCommands.cpp
    game/commands/InventoryCommands.cpp
    game/commands/MagicCommands.cpp
    game/commands/NpcCommands.cpp
    game/commands/ChatCommands.cpp
    game/commands/BankCommands.cpp
    game/commands/MerchantCommands.cpp
    game/commands/ClanCommands.cpp
    game/commands/DungeonCommands.cpp
    game/commands/ChatCheatCommands.cpp
    game/commands/CheatCommands.cpp
    game/PersistenceMonitor.cpp
    game/world/World.cpp
    game/world/WorldCollision.cpp
    game/world/WorldChat.cpp
    game/world/WorldPlayers.cpp
    game/world/WorldItems.cpp
    game/world/WorldClans.cpp
    game/world/WorldBank.cpp
    game/world/WorldNpcs.cpp
    game/world/WorldSpawner.cpp
    game/world/WorldSnapshot.cpp
    game/world/Mazmorra.cpp
    game/world/WorldBuilder.cpp
    game/QueueMonitor.cpp
    game/world/MapaBuilder.cpp
    game/Items.cpp
    game/Npc.cpp
    game/Equations.cpp
    game/config/GameConfig.cpp
    game/PersistenceThread.cpp
)

target_include_directories(argentum_grupo13_server_lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(argentum_grupo13_server_lib
    PUBLIC
    argentum_grupo13_common
)

# ejecutable del servidor
add_executable(argentum_grupo13_server)

target_sources(argentum_grupo13_server
    PRIVATE
    main.cpp
)

target_link_libraries(argentum_grupo13_server
    PUBLIC
    argentum_grupo13_server_lib
)
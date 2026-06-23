target_sources(argentum_grupo13_tests
    PRIVATE
    # .cpp files
    server_protocol_test.cpp
    client_protocol_test.cpp
    ..//client/net/ClientProtocol.cpp
    PUBLIC
    # .h files
    )

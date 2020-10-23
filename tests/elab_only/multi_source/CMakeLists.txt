add_executable(multi_source
        first.h
        first.cpp
        second.h
        second.cpp
        top.h
        top.cpp
        main.cpp
        )

svc_target(multi_source)

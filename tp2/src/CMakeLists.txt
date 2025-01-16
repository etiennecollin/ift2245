

add_library(scheduler_lib
    os.h
    os.c
    process.h
    process.c
    ready_queue.h
    ready_queue.c
    worker.h
    worker.c
)

add_executable(scheduler
    main.c
)

target_link_libraries(scheduler scheduler_lib Threads::Threads)
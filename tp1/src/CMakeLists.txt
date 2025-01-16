
add_executable(shell
        shell.c
        tokenizer.h
        tokenizer.c
        parser.h
        parser.c)

target_link_libraries(shell PRIVATE Threads::Threads)
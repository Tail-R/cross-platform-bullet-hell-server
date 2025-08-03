include(FetchContent)

FetchContent_Declare(
    shared
    GIT_REPOSITORY https://github.com/Tail-R/cross-platform-bullet-hell-shared.git
    GIT_TAG main
)

FetchContent_MakeAvailable(shared)
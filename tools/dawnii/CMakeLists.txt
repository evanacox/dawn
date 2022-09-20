add_dawn_executable(dawnii
        DEPENDS dawn::dawn absl::flags absl::flags_parse
        FILES
        ./src/main.cc)

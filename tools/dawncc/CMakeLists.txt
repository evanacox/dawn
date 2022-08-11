add_dawn_executable(dawncc
        DEPENDS dawn::dawn absl::flags absl::flags_parse
        FILES
        ./src/main.cc)

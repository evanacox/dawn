add_dawn_executable(dawnic
        DEPENDS dawn::dawn absl::flags absl::flags_parse
        FILES
        ./src/main.cc)

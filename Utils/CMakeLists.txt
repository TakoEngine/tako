add_executable(string_literal "string_literal.cpp" "Files.hpp")
#set_target_properties(string_literal PROPERTIES LINK_FLAGS "-sEXPORTED_FUNCTIONS=_malloc")

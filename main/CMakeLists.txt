# Warning! This code was automatically generated for projects
# without default 'CMakeLists.txt' file.

set(app_sources "app_main.c")
idf_component_register(SRCS ${app_sources} PRIV_REQUIRES system_include display communications IO)
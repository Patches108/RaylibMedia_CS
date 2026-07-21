cmake_minimum_required(VERSION 3.15)
project(raymedia_interop_stub LANGUAGES C)

add_library(raymedia SHARED raymedia_stub.c)
set_target_properties(raymedia PROPERTIES C_STANDARD 99)

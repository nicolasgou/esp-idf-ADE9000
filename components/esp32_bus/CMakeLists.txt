# Copyright (c) IASX Tecnologia S/A. All rights reserved.
# SPDX-License-Identifier: MIT

set(ROOT_PATH
    ${CMAKE_CURRENT_LIST_DIR}/../..
)#/home/nicolasgou/esp/esp-idf/PROJECTS/iasx_enmet_fw/

set(COMPONENTS_PATH
    ${ROOT_PATH}/components
)


file(GLOB_RECURSE COMPONENT_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/srcs/*.c
)

# Individual source file set method
# set(COMPONENT_SOURCES
#     ${CMAKE_CURRENT_LIST_DIR}   "iis3dwb_reg.c"
#                                 "iis3dwb_self_test.c"
#                                 #${COMPONENTS_PATH}/iis3dwb/iis3dwb_reg.c
# )

set(COMPONENT_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include
)


idf_component_register(SRCS ${COMPONENT_SOURCES}
                    INCLUDE_DIRS ${COMPONENT_INCLUDE_DIRS}
)





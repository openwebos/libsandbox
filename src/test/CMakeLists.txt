# @@@LICENSE
#
#      Copyright (c) 2010-2012 Hewlett-Packard Development Company, L.P.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@

set(libsandbox_test_NAME test-${CMAKE_PROJECT_NAME})
add_executable(${libsandbox_test_NAME} EXCLUDE_FROM_ALL testsandbox.cpp)
target_link_libraries(${libsandbox_test_NAME} ${CMAKE_PROJECT_NAME})

add_dependencies(${libsandbox_test_NAME} ${CMAKE_PROJECT_NAME})

if(WEBOS_CONFIG_BUILD_TESTS)
	set(all_flag ALL)
else()
	set(all_flag "")
endif()

add_custom_target(tests ${all_flag}
	DEPENDS ${libsandbox_test_NAME}
	COMMAND cp ${libsandbox_test_NAME} ${PROJECT_BINARY_DIR}
	COMMAND chmod u=rwx,g=rx,o=rx ${PROJECT_BINARY_DIR}/${libsandbox_test_NAME}
	COMMENT "Building ${libsandbox_test_NAME}")


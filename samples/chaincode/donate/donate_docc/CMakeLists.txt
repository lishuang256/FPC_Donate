cmake_minimum_required(VERSION 3.5.1)

set(SOURCE_FILES
    donate_docc.cpp
    donate_docc_json.cpp
    )

include($ENV{FPC_PATH}/ecc_enclave/enclave/CMakeLists-common-app-enclave.txt)


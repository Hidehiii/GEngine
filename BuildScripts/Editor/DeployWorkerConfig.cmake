if(NOT EXISTS "${GE_WORKER_OUTPUT}/Config.ini")
    file(COPY_FILE "${GE_WORKER_SOURCE}/Config.ini" "${GE_WORKER_OUTPUT}/Config.ini")
endif()

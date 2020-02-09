cmake_minimum_required(VERSION 3.1)

project(stratosphere)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(stratosphere STATIC
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/pm/pm_shell_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/pm/pm_dmnt_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/pm/pm_boot_mode_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/pm/pm_ams.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/pm/pm_info_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/settings/settings_fwdbg_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/spl/spl_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/spl/smc/spl_smc.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/ams/ams_environment.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/ams/ams_bpc.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/ams/ams_exosphere_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/ams/ams_emummc_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sm/smm_ams.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sm/sm_ams.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sm/sm_mitm_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sm/sm_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sm/sm_manager_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sm/sm_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/kvdb/kvdb_archive.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/kvdb/kvdb_file_key_value_store.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/result/result_on_assertion.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/util/ini.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/util/util_compression.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/util/util_tinymt.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/util/util_ini.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/util/lz4.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssrv/fssrv_storage_interface_adapter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssrv/fssrv_path_normalizer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssrv/fssrv_filesystem_interface_adapter.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/ldr/ldr_pm_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/ldr/ldr_ams.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/patcher/patcher_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/updater/updater_files.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/updater/updater_bis_management.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/updater/updater_bis_save.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/updater/updater_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/updater/updater_paths.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/cfg/cfg_sd_card.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/cfg/cfg_override.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/cfg/cfg_privileged_process.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/cfg/cfg_flags.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/cmif/sf_cmif_service_dispatch.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/cmif/sf_cmif_domain_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/cmif/sf_cmif_service_object_holder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/cmif/sf_cmif_domain_service_object.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/hipc/sf_hipc_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/hipc/sf_hipc_server_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/hipc/sf_hipc_server_session_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/hipc/sf_hipc_mitm_query_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/sf/hipc/sf_hipc_server_domain_session_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/dmnt/dmntcht.c
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/map/map_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/boot2/boot2_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/dd/dd_io_mappings.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssystem/fssystem_subdirectory_filesystem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssystem/fssystem_directory_savedata_filesystem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssystem/fssystem_utility.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fssystem/fssystem_directory_redirection_filesystem.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/hos/hos_version_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/hid/hid_api.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/impl/os_inter_process_event.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/impl/os_waitable_manager_impl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/impl/os_random_impl.os.horizon.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_interrupt_event.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_random.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_waitable_holder.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_event.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_waitable_manager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_process_handle.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_semaphore.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_message_queue.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/os/os_system_event.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fs/fs_path_utils.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fs/fs_file_storage.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/source/fs/fs_path_tool.cpp)

target_compile_definitions(stratosphere PUBLIC ATMOSPHERE_IS_STRATOSPHERE ATMOSPHERE_BOARD_NINTENDO_SWITCH)
target_include_directories(stratosphere
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libstratosphere/include
        ${CMAKE_CURRENT_LIST_DIR}/Atmosphere-libs/libvapours/include)
target_link_libraries(stratosphere switch::libnx)

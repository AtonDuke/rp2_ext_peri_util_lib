add_library(rp2_ext_peri_util_lib INTERFACE)

target_include_directories(rp2_ext_peri_util_lib INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(rp2_ext_peri_util_lib INTERFACE
    src/pi4ioe5v6408.c
    src/dac121s101.c
    src/ads7042.c
    src/fan_control.c
    src/at24cs0x.c
    src/serial_io_utils.c
    src/nv_struct.c
)

# Generate PIO header
pico_generate_pio_header(rp2_ext_peri_util_lib ${CMAKE_CURRENT_LIST_DIR}/src/adc_spi_master_unidir.pio)
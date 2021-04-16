FILE(REMOVE_RECURSE
  "CMakeFiles/man_page_gzips"
  "octoclock_firmware_burner.1.gz"
  "uhd_cal_rx_iq_balance.1.gz"
  "uhd_cal_tx_dc_offset.1.gz"
  "uhd_cal_tx_iq_balance.1.gz"
  "uhd_find_devices.1.gz"
  "uhd_image_loader.1.gz"
  "uhd_images_downloader.1.gz"
  "uhd_usrp_probe.1.gz"
  "usrp_n2xx_simple_net_burner.1.gz"
  "usrp_x3xx_fpga_burner.1.gz"
  "usrp2_card_burner.1.gz"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/man_page_gzips.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)

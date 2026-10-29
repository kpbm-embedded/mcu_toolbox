/* The app image (header + binary, made by mkimage.py) placed in the bootloader ELF at 0x10000. */
  .section .app_image, "a"
  .incbin "app.img"

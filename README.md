# Image Steganography in C
This project implements Image steganography using the Least Significant Bit(LSB) technique. It alows users to hide secret message inside BMP images and retrieve them later.

# Features
- Encode secret text inside an image
- Decode hidden message from image
- Uses LSB encoding technique
- Works with BMP image format
- Modular C program

# Technologies
- C programming
- File handling
- Bit manipulation
- Structures

# Compilation
- compile the program using GCC :
       gcc main.c encode.c decode.c -o stego  or gcc *.c -o stego
  
# usage
- Encode secret message into image
  ./stego -e input.bmp secret.txt output.bmp
  
- Decode hidden message from image
  ./stego -d stego_image.bmp decoded_output.txt



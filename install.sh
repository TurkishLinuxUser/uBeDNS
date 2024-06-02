#!/bin/bash

# Check if the file is run with Root permissions
if [ "$(id -u)" != "0" ]; then
    echo "Please run this file with root permissions: sudo sh install.sh"
    echo "If you want to see exactly what the file does, you can enter this command and read the comment lines: vim install.sh"
    # If you don't know how to exit this screen, just type :q! and click the big key (Enter) on the right on your keyboard :)
    echo "or open this file with your favorite text editor, copy all the codes and ask your favorite AI what they do."
    exit 1
fi

# Check if Curl is installed
if ! which curl &> /dev/null
then
    # Find package manager
    if command -v apt &> /dev/null
    then
        sudo apt update
        sudo apt install curl -y
    elif command -v pacman &> /dev/null
    then
        sudo pacman -Syu
        sudo pacman -S curl --noconfirm
    elif command -v dnf &> /dev/null
    then
        sudo dnf install curl -y
    elif command -v yum &> /dev/null
    then
        sudo yum install curl -y
    else
        echo "An unsupported package manager was detected or no package manager was found."
        exit 1
    fi
fi

# Download cJSON.h and ubedns files
curl -LJO https://github.com/TurkishLinuxUser/ubedns/raw/main/cJSON.h
curl -LJO https://github.com/TurkishLinuxUser/ubedns/raw/main/cJSON.c
curl -LJO https://github.com/TurkishLinuxUser/ubedns/raw/main/ubedns.c

#Compile cJSON.c and ubedns.c
gcc -o ubedns ubedns.c cJSON.c

# Delete cJSON.c and ubedns.c files
rm ubedns.c 
rm cJSON.c

# Move ubedns file to /usr/bin directory
sudo mv ubedns /usr/bin
sudo mv cJSON.h /usr/bin

# Make ubedns file executable
sudo chmod +x /usr/bin/ubedns

# Create a folder named ubedns in ~/.config/ and upload default_dns.json to ~/.config/ubedns
mkdir -p ~/.config/ubedns
curl -LJO https://github.com/TurkishLinuxUser/ubedns/raw/main/default_dns.json -o ~/.config/ubedns/default_dns.json


echo "Success! Just type “ubedns” to run it."
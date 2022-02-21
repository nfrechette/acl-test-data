#!/usr/bin/env bash

# Extract our command line arguments
COMPILER=$1

# Convert our GCC compiler into a list of packages it needs
if [[ $COMPILER == gcc9 ]]; then
    PACKAGES="g++-9 g++-9-multilib g++-multilib"
elif [[ $COMPILER == gcc10 ]]; then
    PACKAGES="g++-10 g++-10-multilib g++-multilib"
fi

# If using clang, add our apt source key
if [[ $COMPILER == clang* ]]; then
    curl -sSL "http://apt.llvm.org/llvm-snapshot.gpg.key" | sudo -E apt-key add - ;
fi

# Convert our clang compiler into a list of packages it needs and its source
if [[ $COMPILER == clang9 ]]; then
    PACKAGES="clang-9 libstdc++-5-dev libc6-dev-i386 g++-5-multilib g++-multilib"
    echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-9 main" | sudo tee -a /etc/apt/sources.list > /dev/null ;
elif [[ $COMPILER == clang10 ]]; then
    PACKAGES="clang-10 libstdc++-5-dev libc6-dev-i386 g++-5-multilib g++-multilib"
    echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main" | sudo tee -a /etc/apt/sources.list > /dev/null ;
elif [[ $COMPILER == clang11 ]]; then
    PACKAGES="clang-11 libstdc++-5-dev libc6-dev-i386 g++-5-multilib g++-multilib"
    echo "deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-11 main" | sudo tee -a /etc/apt/sources.list > /dev/null ;
fi

# Install the packages we need
sudo -E apt-add-repository -y "ppa:ubuntu-toolchain-r/test";
sudo -E apt-get -yq update;
sudo -E apt-get -yq --no-install-suggests --no-install-recommends --force-yes install $PACKAGES;

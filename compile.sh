#!/bin/bash

# brew install fftw
# sudo apt-get install libfftw3-dev

# m = 64*1024; s = 200 * 1024 * 1024; Math.floor(s/m)*m;

mkdir -p bin

# emcc -s NO_EXIT_RUNTIME=1 -sASSERTIONS=1 -sENVIRONMENT=shell -s ALLOW_MEMORY_GROWTH=0 -O3 -I. -I./src app.cpp src/*.c -o bin/btt.js
emcc -s NO_EXIT_RUNTIME=1 -sASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=0 -O3 \
  -I. \
  -I./src \
  -I./kissfft \
  -I./Chord-Detector-and-Chromagram/src \
  -DUSE_KISS_FFT \
  app.cpp \
  src/*.c \
  kissfft/*.c \
  Chord-Detector-and-Chromagram/src/*.cpp \
  -o bin/btt.js
# add to "lol;" to front of script
# sed -i '1s/^/importScripts/' bin/btt.js

# encode bin/btt.js as base64 and put into variable
b64=$(base64 -i bin/btt.wasm)
# trim newlines
# b64=$(echo $b64 | tr -d '\n')

cat header.js >bin/btt2.js
echo -n 'const wasmBinaryB64 = `' >>bin/btt2.js
echo -n "$b64" >>bin/btt2.js
echo '`;' >>bin/btt2.js
echo 'var wasmBinary = b64ToArrayBuffer(wasmBinaryB64);' >>bin/btt2.js
cat bin/btt.js >>bin/btt2.js
echo "export default Module;" >>bin/btt2.js

js-beautify -r bin/btt2.js

cp -R ./bin/* ../engine/audio/bin/

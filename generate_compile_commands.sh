arduino-cli compile --fqbn esp8266:esp8266:nodemcuv2 badmintion_feeder.ino --only-compilation-database --build-path ./build && ln -sf build/compile_commands.json

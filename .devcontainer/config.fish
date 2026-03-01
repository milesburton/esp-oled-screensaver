# Fish shell configuration for ESP8266-OLED-Experiment devcontainer

fish_add_path ~/.local/bin
fish_add_path ~/.platformio/penv/bin

alias build   'pio run -e esp8266_d1_mini'
alias upload  'pio run -e esp8266_d1_mini --target upload'
alias test    'pio test -e native'
alias monitor 'pio device monitor'
alias clean   'pio run --target clean'
alias check   'pre-commit run --all-files'

function fish_greeting
    set_color brblue
    echo '+---------------------------------------------------------+'
    echo '|          ESP8266-OLED-Experiment  dev container         |'
    echo '|   ESP8266 D1 Mini  *  128x64 OLED  *  WiFi  *  OTA     |'
    echo '+---------------------------------------------------------+'
    set_color normal
    echo ''
    echo '  build    pio run -e esp8266_d1_mini'
    echo '  upload   pio run -e esp8266_d1_mini --target upload'
    echo '  test     pio test -e native'
    echo '  monitor  pio device monitor'
    echo '  clean    pio run --target clean'
    echo '  check    pre-commit run --all-files'
    echo ''
end

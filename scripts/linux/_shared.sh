# Only use colors if stdout is a terminal
if [ -t 1 ]; then
    # Text colors
    BLACK="\033[0;30m"
    RED="\033[0;31m"
    GREEN="\033[0;32m"
    YELLOW="\033[0;33m"
    BLUE="\033[0;34m"
    MAGENTA="\033[0;35m"
    CYAN="\033[0;36m"
    WHITE="\033[0;37m"

    # Bright text colors
    BRIGHT_BLACK="\033[1;30m"
    BRIGHT_RED="\033[1;31m"
    BRIGHT_GREEN="\033[1;32m"
    BRIGHT_YELLOW="\033[1;33m"
    BRIGHT_BLUE="\033[1;34m"
    BRIGHT_MAGENTA="\033[1;35m"
    BRIGHT_CYAN="\033[1;36m"
    BRIGHT_WHITE="\033[1;37m"

    # Background colors
    BG_BLACK="\033[40m"
    BG_RED="\033[41m"
    BG_GREEN="\033[42m"
    BG_YELLOW="\033[43m"
    BG_BLUE="\033[44m"
    BG_MAGENTA="\033[45m"
    BG_CYAN="\033[46m"
    BG_WHITE="\033[47m"

    # Bright background colors
    BG_BRIGHT_BLACK="\033[100m"
    BG_BRIGHT_RED="\033[101m"
    BG_BRIGHT_GREEN="\033[102m"
    BG_BRIGHT_YELLOW="\033[103m"
    BG_BRIGHT_BLUE="\033[104m"
    BG_BRIGHT_MAGENTA="\033[105m"
    BG_BRIGHT_CYAN="\033[106m"
    BG_BRIGHT_WHITE="\033[107m"

    # Text styles
    BOLD="\033[1m"
    DIM="\033[2m"
    UNDERLINE="\033[4m"
    BLINK="\033[5m"
    REVERSE="\033[7m"
    HIDDEN="\033[8m"

    # Reset
    RESET="\033[0m"
else
    # No colors if not a terminal
    BLACK="" RED="" GREEN="" YELLOW="" BLUE="" MAGENTA="" CYAN="" WHITE=""
    BRIGHT_BLACK="" BRIGHT_RED="" BRIGHT_GREEN="" BRIGHT_YELLOW="" BRIGHT_BLUE=""
    BRIGHT_MAGENTA="" BRIGHT_CYAN="" BRIGHT_WHITE=""
    BG_BLACK="" BG_RED="" BG_GREEN="" BG_YELLOW="" BG_BLUE="" BG_MAGENTA="" BG_CYAN="" BG_WHITE=""
    BG_BRIGHT_BLACK="" BG_BRIGHT_RED="" BG_BRIGHT_GREEN="" BG_BRIGHT_YELLOW="" BG_BRIGHT_BLUE=""
    BG_BRIGHT_MAGENTA="" BG_BRIGHT_CYAN="" BG_BRIGHT_WHITE=""
    BOLD="" DIM="" UNDERLINE="" BLINK="" REVERSE="" HIDDEN="" RESET=""
fi

function info() { echo -e "${BOLD}${GREEN}INFO${RESET}  $1"; }
function warn() { echo -e "${BOLD}${YELLOW}WRN${RESET}   $1"; }
function error() { echo -e "${BOLD}${RED}ERR${RESET}   $1"; }

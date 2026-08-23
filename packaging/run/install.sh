#!/usr/bin/env sh
set -eu

APP_NAME="acer-nitro-sense"
SERVICE_NAME="acer-nitro-sense.service"
CONTROL_GROUP="acer-nitro-sense"
ROOT_DIR=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)

MODEL=""
ENABLE_SERVICE=1
START_SERVICE=1
UNINSTALL=0
TARGET_USER="${SUDO_USER:-${USER:-}}"

usage() {
    cat <<EOF
usage: sudo ./install.sh --model MODEL [options]

Options:
  --model MODEL          Required model id, for example acer-nitro-an517-51
  --list-models          List bundled model ids and exit
  --no-enable-service    Install files but do not enable the systemd service
  --no-start             Install files but do not start/restart the service
  --user USER            User to add to the control group, defaults to SUDO_USER
  --uninstall            Stop service and remove installed files
  -h, --help             Show this help
EOF
}

die() {
    printf 'error: %s\n' "$*" >&2
    exit 1
}

info() {
    printf '%s\n' "$*"
}

require_root() {
    if [ "$(id -u)" -ne 0 ]; then
        die "installer must run as root"
    fi
}

list_models() {
    for model in "$ROOT_DIR/usr/share/$APP_NAME/models/"*.json; do
        [ -e "$model" ] || continue
        basename "$model" .json
    done | sort
}

install_tree() {
    [ -d "$ROOT_DIR/usr" ] || die "release payload is missing usr/"

    if [ -d "$ROOT_DIR/usr/bin" ]; then
        install -d -m 0755 /usr/bin
        cp -a "$ROOT_DIR/usr/bin/." /usr/bin/
    fi

    if [ -d "$ROOT_DIR/usr/lib" ]; then
        install -d -m 0755 /usr/lib
        cp -a "$ROOT_DIR/usr/lib/." /usr/lib/
    fi

    install -d -m 0755 /usr/share
    cp -a "$ROOT_DIR/usr/share/$APP_NAME" /usr/share/
}

select_model() {
    model_path="/usr/share/$APP_NAME/models/$MODEL.json"
    [ -f "$model_path" ] || die "unknown model '$MODEL'. Run --list-models."

    install -d -m 0755 /etc/$APP_NAME
    ln -sfn "$model_path" /etc/$APP_NAME/model.json
    info "selected model: $MODEL"
}

configure_systemd() {
    if command -v systemd-sysusers >/dev/null 2>&1; then
        systemd-sysusers /usr/lib/sysusers.d/$APP_NAME.conf
    fi

    if command -v systemctl >/dev/null 2>&1; then
        systemctl daemon-reload
        if [ "$ENABLE_SERVICE" -eq 1 ]; then
            systemctl enable "$SERVICE_NAME"
        fi
        if [ "$START_SERVICE" -eq 1 ]; then
            systemctl restart "$SERVICE_NAME"
        fi
    fi
}

preflight_ec_notice() {
    if command -v modprobe >/dev/null 2>&1; then
        modprobe ec_sys 2>/dev/null || true
        modprobe acpi_ec 2>/dev/null || true
    fi

    if [ -e /sys/kernel/debug/ec/ec0/io ] || [ -e /dev/ec ]; then
        return 0
    fi

    if [ -r /sys/kernel/security/lockdown ] &&
        grep -Eq '\[(integrity|confidentiality)\]' /sys/kernel/security/lockdown; then
        info "warning: no ec_sys or /dev/ec backend is currently visible"
        info "warning: kernel lockdown is active, so direct EC I/O is likely blocked"
        info "warning: install/sign acpi_ec, disable lockdown, or boot a kernel with ec_sys"
    fi
}

add_user_to_group() {
    if [ -n "$TARGET_USER" ] && id "$TARGET_USER" >/dev/null 2>&1; then
        usermod -aG "$CONTROL_GROUP" "$TARGET_USER" || true
        info "added $TARGET_USER to $CONTROL_GROUP; log out and back in for control commands"
    fi
}

uninstall_app() {
    if command -v systemctl >/dev/null 2>&1; then
        systemctl disable --now "$SERVICE_NAME" 2>/dev/null || true
        systemctl daemon-reload
    fi

    rm -f /usr/bin/acer-nitro-sense /usr/bin/acer-nitro-sensed /usr/bin/ans
    rm -f /usr/lib/systemd/system/$SERVICE_NAME
    rm -f /usr/lib/sysusers.d/$APP_NAME.conf
    rm -f /usr/lib/systemd/system-sleep/$APP_NAME
    rm -rf "/usr/share/${APP_NAME:?}"
    rm -f /etc/$APP_NAME/model.json
    rmdir /etc/$APP_NAME 2>/dev/null || true

    info "removed installed $APP_NAME files"
    info "kept runtime state under /var/lib/$APP_NAME"
}

while [ "$#" -gt 0 ]; do
    case "$1" in
        --model)
            [ "$#" -ge 2 ] || die "--model requires a value"
            MODEL="$2"
            shift 2
            ;;
        --list-models)
            list_models
            exit 0
            ;;
        --no-enable-service)
            ENABLE_SERVICE=0
            shift
            ;;
        --no-start)
            START_SERVICE=0
            shift
            ;;
        --uninstall)
            UNINSTALL=1
            shift
            ;;
        --user)
            [ "$#" -ge 2 ] || die "--user requires a value"
            TARGET_USER="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            die "unknown argument: $1"
            ;;
    esac
done

require_root
if [ "$UNINSTALL" -eq 1 ]; then
    uninstall_app
    exit 0
fi

[ -n "$MODEL" ] || die "--model is required. Run --list-models."

install_tree
select_model
preflight_ec_notice
configure_systemd
add_user_to_group

info "installed $APP_NAME"
info "status: acer-nitro-sense status"
info "GNOME Shell extension is distributed separately"

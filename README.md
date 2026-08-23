<a href="https://shogun.ms" target="_blank" rel="noopener">
	<img src="https://cdn.shogun.ms/assets/branding/app-icon-256.svg" alt="Shogun app-icon" height="62"/>
</a>

---

# Acer Nitro Sense Linux

A lightweight Acer Nitro fan-control daemon and CLI for Linux.

It controls supported Acer Nitro laptops through model-specific EC registers and keeps the public surface focused: fan control, fan telemetry, Nitro profiles, CoolBoost-style turbo cooling, GPU temperature policy, power-source profiles, and validated keyboard backlight controls.

The first supported model is `Acer Nitro 5 AN517-51`.

## ✨ Features

- Reads CPU/GPU fan RPM and temperatures.
- Sets CPU, GPU, or all fan speeds from `1-100%`.
- Applies JSON presets.
- Switches fan mode between `auto`, `manual`, and `turbo`.
- Provides a CoolBoost-style turbo cooldown command.
- Switches Nitro profile between `quiet`, `balanced`, and `performance`.
- Optionally maps AC/battery state to Nitro profiles.
- Controls keyboard backlight brightness and timeout on validated models.
- Exposes local status for optional companion interfaces.

> [!NOTE]
> Not every Acer Sense feature has a stable Linux control path. Desktop sticky
keys, Windows/menu key locking, and TrueHarmony audio modes are intentionally
not implemented.

## 📦 Requirements

- Linux with systemd
- A supported Acer Nitro model profile
- EC access through one of:
  - `ec_sys`
  - `/dev/ec` from `acpi_ec`
  - direct EC I/O ports when the kernel allows it

> [!NOTE]
> On Fedora with Secure Boot, the practical route is usually a signed `acpi_ec`
module. See [EC access](docs/ec-access.md).

## 🚀 Installation

### Fedora Workstation, COPR Repository

```sh
sudo dnf install dnf-plugins-core
sudo dnf copr enable almighty-shogun/acer-nitro-sense-linux
sudo dnf install acer-nitro-sense-linux
```

### Fedora Atomic, COPR repository

Silverblue, Kinoite, Bazzite, Aurora, and other ostree-based variants layer the
same RPM. They do not all ship `dnf`, so add the COPR repository by writing its
repo file directly:

```sh
sudo curl -o /etc/yum.repos.d/almighty-shogun-acer-nitro-sense-linux.repo \
    "https://copr.fedorainfracloud.org/coprs/almighty-shogun/acer-nitro-sense-linux/repo/fedora-$(rpm -E %fedora)/almighty-shogun-acer-nitro-sense-linux-fedora-$(rpm -E %fedora).repo"
sudo rpm-ostree install acer-nitro-sense-linux
systemctl reboot
```

Updates arrive with your normal system upgrade: `sudo dnf upgrade` on Fedora Workstation, or `sudo rpm-ostree upgrade` followed by a reboot on Fedora Atomic.

### Debian/Ubuntu:

```sh
curl -LO https://github.com/Almighty-Shogun/acer-nitro-sense-linux/releases/latest/download/acer-nitro-sense-linux.deb
sudo apt install ./acer-nitro-sense-linux.deb
```

### Other distributions

```sh
curl -LO https://github.com/Almighty-Shogun/acer-nitro-sense-linux/releases/latest/download/acer-nitro-sense-linux.run
chmod +x acer-nitro-sense-linux.run
./acer-nitro-sense-linux.run --list-models
sudo ./acer-nitro-sense-linux.run --model acer-nitro-an517-51
```

Log out and back in after installation so your user session picks up the `acer-nitro-sense` group.

> [!NOTE]
> The generic `.run` installer refuses to run on atomic systems, because `/usr`
is read-only there. Use the COPR package through `rpm-ostree` instead.

The [GNOME Shell extension](https://github.com/Almighty-Shogun/acer-nitro-sense-linux-gnome-extension) is optional and distributed separately. Install the
daemon first, then install the extension if you want panel status and controls.

## 🧹 Uninstall
```sh
# Fedora
sudo dnf remove acer-nitro-sense-linux

# Fedora Atomic
sudo rpm-ostree uninstall acer-nitro-sense-linux

# Debian/Ubuntu
sudo apt remove acer-nitro-sense-linux

# Generic
sudo ./acer-nitro-sense-linux.run --uninstall
```

## 🔧 Building

```sh
meson setup build --prefix=/usr --sysconfdir=/etc
meson compile -C build
meson test -C build
```

Hardware validation tools live in `scripts/`:

```sh
scripts/hardware/validate-hardware.sh
scripts/hardware/discover-keyboard-backlight.sh
```

## 💻 Usage

Commands can either be invoked with `acer-nitro-sense` or the shorter alias `ans`.

| Name | Arguments                                                 | Description |
| --- |-----------------------------------------------------------| --- |
| `status` | `--json`, `--fahrenheit`                                  | Show current model, mode, RPM, temperatures, fan targets, and safety state. |
| `capabilities` | -                                                         | Show which features are available on the current machine. |
| `doctor` | -                                                         | Collect system, EC, sensor, and feature diagnostics. |
| `firmware-auto` | -                                                         | Let Acer firmware own live fan speed while the daemon keeps reporting status. |
| `auto` | -                                                         | Enable the daemon fan curve. |
| `set` | `[cpu\|gpu\|all] [1-100]`                                 | Set manual fan speed percentages. |
| `preset` | `quiet`, `balanced`, `performance`                        | Apply a configured fan preset. |
| `fan-mode` | `status`, `auto`, `manual`, `turbo`                       | Read or switch the Acer firmware fan mode. |
| `coolboost` | `status`, `[on\|off]`                                     | Toggle CoolBoost-style turbo cooling. |
| `profile` | `status`, `quiet`, `balanced`, `performance`              | Read or switch the Nitro profile. |
| `power-source` | `status`, `apply`, `auto [on\|off]`                       | Configure AC/battery profile switching. |
| `gpu-temp` | `status`, `live`, `auto`                                  | Configure GPU temperature wake policy. |
| `keyboard-backlight` | `status`, `set [0\|25\|50\|75\|100]`, `timeout [on\|off]` | Control keyboard backlight brightness and timeout. |
| `start` | -                                                         | Start the daemon service. Requires `sudo`. |
| `restart` | -                                                         | Restart the daemon service. Requires `sudo`. |
| `stop` | -                                                         | Ask the daemon to stop. |
